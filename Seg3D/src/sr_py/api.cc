//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  
//    File   : api.cc
//    Author : Martin Cole
//    Date   : Mon Aug  7 15:21:20 2006


#include <sr_py/api.h>
#include <Core/Init/init.h>
#include <Core/Persistent/Pstreams.h>
#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <Core/Util/ProgressReporter.h>
#include <Core/Util/Environment.h>
#include <Core/Geom/CallbackOpenGLContext.h>
#include <Core/Events/DataManager.h>
#include <Core/Events/EventManager.h>
#include <Core/Events/BaseEvent.h>
#include <Core/Events/SceneGraphEvent.h>
#include <Core/Events/OGLView/SelectionTargetEvent.h>
#include <Core/Events/OGLView/OpenGLViewer.h>
#include <Core/Events/CM2View/CM2View.h>
#include <Core/Datatypes/Field.h>
#include <main/sci_version.h>
#include <iostream>
#include <sstream>

#if defined(HAVE_TETGEN)
#include <Core/Basis/Constant.h>
#include <Core/Basis/TetLinearLgn.h>
#include <Core/Basis/TriLinearLgn.h>
#include <Core/Datatypes/TetVolMesh.h>
#include <Core/Datatypes/TriSurfMesh.h>
#include <Core/Datatypes/GenericField.h>
#include <Dataflow/Modules/Fields/InterfaceWithTetGen.h>
#endif


namespace SCIRun {

using std::cerr;
using std::endl;

void 
init_sr_py(char**environment)
{
  static bool init_done = false;
  if (!init_done) {
    create_sci_environment(environment, 0);
    sci_putenv("SCIRUN_VERSION", SCIRUN_VERSION);
    sci_putenv("SCIRUN_RCFILE_SUBVERSION", SCIRUN_RCFILE_SUBVERSION);

    SCIRunInit();

    //! create the EventManager thread.
    static Thread* emt = 0;
    if (! emt) {
      emt = scinew Thread(new EventManager(), "Event Manager Thread");
      emt->detach();
    }

    init_done = true;
    //cerr << "otf:" << sci_getenv("SCIRUN_ON_THE_FLY_LIBS_DIR") << endl;
  }
}

void 
test_function(string f1, string f2, string f3)
{
  cerr << "f1: " << f1 << endl;
  cerr << "f2: " << f2 << endl;
  cerr << "f3: " << f3 << endl;
}

bool 
tetgen_2surf(string f1, string f2, string outfile)
{
#if defined(HAVE_TETGEN)
  DataManager *dm = DataManager::get_dm();
  size_t f1_id = dm->load_field(f1);
  if (f1_id == 0) {
    cerr << "Error opening file: "<< f1 << endl;
    return false;
  }
  size_t f2_id = dm->load_field(f2);
  if (f2_id == 0) {
    cerr << "Error opening file: "<< f2 << endl;
    return false;
  }

  FieldHandle outer = dm->get_field(f1_id);
  FieldHandle inner = dm->get_field(f2_id);

  //cerr << "mesh scalar?: " << inner->is_scalar() << endl;
  tetgenio in, out;

  unsigned idx = 0;
  unsigned fidx = 0;

  // indices start from 0.
  in.firstnumber = 0;
  in.mesh_dim = 3;
  ProgressReporter rep;
  
  //! Add the info for the outer surface, or tetvol to be refined.
  const TypeDescription *ostd = outer->get_type_description();
  const string tcn("TGSurfaceTGIO");
  CompileInfoHandle ci = TGSurfaceTGIOAlgo::get_compile_info(ostd, tcn);
  //Handle<TGSurfaceTGIOAlgo> algo;
  DynamicAlgoHandle dalgo;
  if (!DynamicCompilation::compile(ci, dalgo)) {
    //error("Could not get TetGen/SCIRun converter algorithm");
    cerr << "Could not get TetGen/SCIRun converter algorithm" << endl;
    return false;
  }
  int marker = -10;
  TGSurfaceTGIOAlgo* algo = (TGSurfaceTGIOAlgo*)dalgo.get_rep();
  //TGSurfaceTGIOAlgo* algo = dynamic_cast<TGSurfaceTGIOAlgo*>(dalgo.get_rep());
  if (!algo) {
    return false;
  }
  algo->to_tetgenio(&rep, outer, idx, fidx, marker, in);


  // Interior surface.
  marker *= 2;
  algo->to_tetgenio(&rep, inner, idx, fidx, marker, in);


  //update_progress(.2);
  // Save files for later debugging.
  string filename = string(sci_getenv("SCIRUN_TMP_DIR")) + "/tgIN";
  in.save_nodes((char*)filename.c_str());
  in.save_poly((char*)filename.c_str());

  string tg_cmmd = "pqa300.0AAz";
  // Create the new mesh.
  tetrahedralize((char*)tg_cmmd.c_str(), &in, &out); 
  FieldHandle tetvol_out;
  //update_progress(.9);
  // Convert to a SCIRun TetVol.
  tetvol_out = algo->to_tetvol(out);
  //update_progress(1.0);
  if (tetvol_out.get_rep() == 0) { return false; }
  BinaryPiostream out_stream(outfile, Piostream::Write);
  Pio(out_stream, tetvol_out);
#endif
  return true;
}


void 
add_pointer_event(PointerEvent *pe, string target)
{
  PointerEvent *p = new PointerEvent();
  p->ref_cnt = 1; // workaround for assert in Datatype operator ==
  *p = *pe;
  p->set_target(target);
  event_handle_t event = p;
  EventManager::add_event(event);
}

void 
add_key_event(KeyEvent *ke, string target)
{
  KeyEvent *k = new KeyEvent();
  k->ref_cnt = 1; // workaround for assert in Datatype operator ==
  *k = *ke;
  k->set_target(target);
  event_handle_t event = k;
  EventManager::add_event(event);
}

void 
add_tm_notify_event(TMNotifyEvent *te)
{
  TMNotifyEvent *t = new TMNotifyEvent();
  t->ref_cnt = 1; // workaround for assert in Datatype operator ==
  *t = *te;

  event_handle_t event = t;
  EventManager::add_event(event);
}

void 
add_command_event(CommandEvent *ce)
{
  CommandEvent *c = new CommandEvent();
  c->ref_cnt = 1; // workaround for assert in Datatype operator ==
  *c = *ce;
  event_handle_t event = c;
  EventManager::add_event(event);
}

void 
selection_target_changed(size_t fid)
{
  DataManager *dm = DataManager::get_dm();
  cerr << "the selection target id is: " << fid << endl;
  dm->set_selection_target(fid);
  FieldHandle fld = dm->get_field(fid);

  SelectionTargetEvent *t = new SelectionTargetEvent();
  t->set_selection_target(fld);
  t->set_selection_id(fid);

  event_handle_t event = t;
  EventManager::add_event(event);
}

void terminate() 
{
  // send a NULL event to terminate...
  event_handle_t event = new QuitEvent();
  EventManager::add_event(event);
}

void 
run_viewer_thread(CallbackOpenGLContext *ogl, string id) 
{
  CallbackOpenGLContext *c = new CallbackOpenGLContext();
  *c = *ogl;
  //cerr << "starting viewer thread with: " << c << endl;
  
  OpenGLViewer *v = new OpenGLViewer(c, id);
  
  string tid = "Viewer Thread:" + id;
  Thread *vt = scinew Thread(v, tid.c_str());
  vt->detach(); // runs until thread exits.
}

size_t
create_viewer(CallbackOpenGLContext *ogl, string id) 
{
  DataManager *dm = DataManager::get_dm();
  CallbackOpenGLContext *c = new CallbackOpenGLContext();
  *c = *ogl;
  return dm->add_view(new OpenGLViewer(c, id));
}

bool
update_viewer(size_t id)
{
  DataManager *dm = DataManager::get_dm();
  OpenGLViewer *v = dynamic_cast<OpenGLViewer*>(dm->get_view(id));
  if (v) {
    v->update();
    return true;
  }
  return false;
}

size_t
create_cm2view(CallbackOpenGLContext *ogl, string parent_id, string id) 
{
  DataManager *dm = DataManager::get_dm();
  CallbackOpenGLContext *c = new CallbackOpenGLContext();
  *c = *ogl;
  return dm->add_view(new CM2View(c, parent_id, id));
}

bool
update_cm2view(size_t v_id)
{
  DataManager *dm = DataManager::get_dm();
  CM2View *v = dynamic_cast<CM2View*>(dm->get_view(v_id));
  if (v) {
    v->update();
    return true;
  }
  return false;
}

bool
add_rectangle_to_cm2view(size_t v_id)
{
  DataManager *dm = DataManager::get_dm();
  CM2View *v = dynamic_cast<CM2View*>(dm->get_view(v_id));
  if (v) {
    v->add_rectangle_widget();
    return true;
  }
  return false;
}

bool
add_triangle_to_cm2view(size_t v_id)
{
  DataManager *dm = DataManager::get_dm();
  CM2View *v = dynamic_cast<CM2View*>(dm->get_view(v_id));
  if (v) {
    v->add_triangle_widget();
    return true;
  }
  return false;
}

bool
set_cm2view_notify_id(size_t v_id, string id)
{
  DataManager *dm = DataManager::get_dm();
  CM2View *v = dynamic_cast<CM2View*>(dm->get_view(v_id));
  if (v) {
    v->set_notify_id(id);
    return true;
  }
  return false;
}

size_t 
load_field(string fname)
{
  DataManager *dm = DataManager::get_dm();
  return dm->load_field(fname);
}

bool 
toggle_field_visibility(size_t fld_id) 
{
  //  DataManager *dm = DataManager::get_dm();
  //toggle for all GeomObjs' generated for this field
  ostringstream str;
  str << "-" << fld_id << "-";

  SceneGraphEvent* sge = new SceneGraphEvent(0, str.str());
  sge->set_toggle_visibility();
  event_handle_t event = sge;
  EventManager::add_event(event);
  return true;
}

//read_at_index takes an array and returns the value at index. Needed for 
// finding the result of module with multiple output ports.
size_t 
read_at_index(size_t * array, size_t index)
{
  return array[index];
}

bool 
send_scene(size_t s, string target) 
{
  DataManager *dm = DataManager::get_dm();

  DataManager::geom_info_t gi = dm->get_geom(s);
  GeomHandle gh = gi.first;
  if (! gh.get_rep()) return false;

  SceneGraphEvent* sge = new SceneGraphEvent(gh, gi.second, target);
  event_handle_t event = sge;
  EventManager::add_event(event);
  return true;
}

}
