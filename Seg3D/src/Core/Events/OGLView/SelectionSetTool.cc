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
//    File   : SelectionSetTool.cc
//    Author : Martin Cole
//    Date   : Mon Sep 18 10:04:12 2006

#include <Core/Events/EventManager.h>
#include <Core/Events/DataManager.h>
#include <Core/Geom/GeomSwitch.h>
#include <Core/Geom/GeomColorMap.h>
#include <Core/Events/OGLView/SelectionSetTool.h>
#include <Core/Events/OGLView/SelectionTargetEvent.h>
#include <Core/Algorithms/Fields/ClipAtIndeces.h>

#if defined(HAVE_PYTHON)
#include <Core/Algorithms/Interface/Visualization/ShowFieldAlg.h>
#endif

namespace SCIRun {


SelectionSetTool::SelectionSetTool(string name, SSTInterface *ssti) :
  BaseTool(name),
  mode_(FACES_E),
  sel_fld_(0),
  sel_fld_id_(0),
  ssti_(ssti),
#if defined(HAVE_PYTHON)
  params_(new ShowFieldAlg()),
#endif
  def_material_(new Material(Color(0.3, 0.7, 0.3))),
  text_material_(new Material(Color(0.25, 0.25, 0.45)))
{
  float ramp[1024];
  float m = 1./1024.;
  for (int i = 0; i < 1024; i += 4) {
    ramp[i] = m * i;
    ramp[i+1] = 0.0;
    ramp[i+2] = 0.0;
    ramp[i+3] = 0.9;
  }
  color_map_ = new ColorMap(ramp);
}


SelectionSetTool::~SelectionSetTool()
{
}


BaseTool::propagation_state_e
SelectionSetTool::process_event(event_handle_t e)
{
  SelectionTargetEvent *st =
    dynamic_cast<SelectionTargetEvent*>(e.get_rep());
  if (st) {
    sel_fld_ = st->get_selection_target();
    sel_fld_id_ = st->get_selection_id();
    return STOP_E;
  }
  return CONTINUE_E;
}


void
SelectionSetTool::delete_faces()
{
  typedef TriSurfMesh<TriLinearLgn<Point> > TSMesh;

  if (! sel_fld_.get_rep()) return;
  sel_fld_->lock.lock();

  // Turn this call into a general algorithm, but for now assume trisurf.
  MeshHandle mb = sel_fld_->mesh();
  TSMesh *tsm = dynamic_cast<TSMesh *>(mb.get_rep());
  if (!tsm) {
    cerr << "Error:: not a TriSurf in SelectionSetTool::delete_faces"
	 << endl;
  }
  vector<unsigned int> &sfaces = ssti_->get_selection_set();

  vector<int> faces;
  vector<unsigned int>::iterator si = sfaces.begin();
  while (si != sfaces.end()) {
    faces.push_back(*si++);
  }

  bool altered = false;
  // Remove last index first.
  sort(faces.begin(), faces.end());
  vector<int>::reverse_iterator iter  = faces.rbegin();
  while (iter != faces.rend()) {
    int face = *iter++;
    altered |= tsm->remove_face(face);
    cout << "removed face " << face << endl;
  }

  // Clear the selection set.
  sfaces.clear();
  ssti_->set_selection_set_visible(false);
  sel_fld_->lock.unlock();

  // Notify the data manager that this model has changed.
  CommandEvent *c = new CommandEvent();
  c->set_command("selection field modified");
  event_handle_t event = c;
  EventManager::add_event(event);
}


void
SelectionSetTool::add_face()
{
  typedef TriSurfMesh<TriLinearLgn<Point> > TSMesh;

  if (! sel_fld_.get_rep()) return;
  sel_fld_->lock.lock();

  // Turn this call into a general algorithm, but for now assume trisurf.
  MeshHandle mb = sel_fld_->mesh();
  TSMesh *tsm = dynamic_cast<TSMesh *>(mb.get_rep());
  if (!tsm) {
    cerr << "Error:: not a TriSurf in SelectionSetTool::delete_faces"
	 << endl;
  }
  vector<unsigned int> &snodes = ssti_->get_selection_set();
  if (snodes.size() < 3) {
    cerr << "Error: must select 3 nodes to add a face" << endl;
  }

  vector<int> nodes;
  vector<unsigned int>::iterator si = snodes.begin();
  int n0 = *si++;
  int n1 = *si++;
  int n2 = *si++;

  tsm->add_triangle(n0, n1, n2);

  //clear the selection set.
  snodes.erase(snodes.begin(), si);
  if (snodes.size() == 0) {
    ssti_->set_selection_set_visible(false);
  }
  sel_fld_->lock.unlock();

  //notify the data manager that this model has changed.
  CommandEvent *c = new CommandEvent();
  c->set_command("selection field modified");
  event_handle_t event = c;
  EventManager::add_event(event);
}


void
SelectionSetTool::render_selection_set()
{
  //create a new field with the selection items in it;
  if (! sel_fld_.get_rep()) { return; }
  if (! ssti_->get_selection_set().size()) {
    ssti_->set_selection_geom(GeomHandle(0));
    return;
  }
#if defined(HAVE_PYTHON)
  //  params_->defaults(); // won't reset the colors we already set
  DataManager *dm = DataManager::get_dm();
  size_t sel_vis = 0;
  switch (mode_) {

  case NODES_E:
    params_->set_p_nodes_on(0);
    params_->set_p_text_on(1);
    params_->set_p_text_show_faces(0);
    params_->set_p_text_show_nodes(1);
    params_->set_p_text_fontsize(4);
    params_->set_p_nodes_scale(1.0);
    sel_vis = dm->add_field(clip_nodes(sel_fld_, ssti_->get_selection_set()));
    break;
  case EDGES_E:
    params_->set_p_edges_on(1);
    break;
  default:
  case FACES_E:
    params_->set_p_faces_on(1);
    sel_vis = dm->add_field(clip_faces(sel_fld_, ssti_->get_selection_set()));
    break;
  case CELLS_E:

    break;
  };

  if (! sel_vis) {
    cerr << "Error: render_field failed." << endl;
    return;
  }

  vector<size_t> gobjs = params_->execute(sel_vis, 0);

  GeomHandle gmat;
  GeomHandle geom;
  string name;
  switch (mode_) {

  case NODES_E:
    {
      DataManager::geom_info_t gi = dm->get_geom(gobjs[3]);
      gmat = new GeomMaterial(gi.first, text_material_);
      geom = new GeomSwitch(new GeomColorMap(gmat, color_map_));
      name = params_->get_p_text_backface_cull() ? 
	"Culled Text Data" : "Text Data";
    }
    break;
  case EDGES_E:
    {
      DataManager::geom_info_t gi = dm->get_geom(gobjs[1]);
      gmat = new GeomMaterial(gi.first, def_material_);
      geom = new GeomSwitch(new GeomColorMap(gmat, color_map_));
      name = params_->get_p_edges_transparency() ? 
	"Transparent Edges" : "Edges";
    }
    break;
  default:
  case FACES_E:
    {
      DataManager::geom_info_t gi = dm->get_geom(gobjs[2]);
      gmat = new GeomMaterial(gi.first, def_material_);
      geom = new GeomSwitch(new GeomColorMap(gmat, color_map_));
      name = params_->get_p_faces_transparency() ? 
	"Transparent Faces" : "Faces";
    }
    break;
  };
  ssti_->set_selection_geom(geom);
#endif
}


} // namespace SCIRun
