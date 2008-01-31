//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  License for the specific language governing rights and limitations under
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
//    File   : api.h
//    Author : Martin Cole
//    Date   : Wed Sep 26 13:07:13 2007


//swig -v -o sr_py_wrap.cc -DSCISHARE -I<path to build>/include -I<path to>SCIRun/src -python -c++ sr_py.i

#include <string>
#include <vector>

namespace SCIRun {

class CallbackOpenGLContext;
class KeyEvent;
class PointerEvent;
class TMNotifyEvent;
class CommandEvent;
class OpenGLViewer;

using std::string;
using std::vector;

size_t load_field(string fname);
//bool show_field(size_t fld_id);
bool toggle_field_visibility(size_t fld_id);
void init_sr_py(char**environment);
void terminate();
void test_function(string f1, string f2, string f3);
bool tetgen_2surf(string f1, string f2, string out);

//! runs a viewer in its own thread.
void run_viewer_thread(CallbackOpenGLContext *ogl, string id = "");

//! The following 2 functions are for use when the gui needs to own the thread 
//! that the viewer runs in.
//! Creates a viewer, returning the id for that viewer.
size_t create_viewer(CallbackOpenGLContext *ogl, string id = "");
//! Tell the viewer to update its contents.
bool update_viewer(size_t v_id);

size_t create_cm2view(CallbackOpenGLContext *ogl, string pid, string id);
bool set_cm2view_notify_id(size_t v_id, string id);
bool update_cm2view(size_t v_id);
bool add_rectangle_to_cm2view(size_t v_id);
bool add_triangle_to_cm2view(size_t v_id);


void add_pointer_event(PointerEvent *p, string target = "");
void add_key_event(KeyEvent *k, string target = "");
bool send_scene(size_t scene, string target = ""); 

void add_tm_notify_event(TMNotifyEvent *t);
void add_command_event(CommandEvent *c);
void selection_target_changed(size_t fid);

size_t  read_at_index(size_t * array, size_t index);

}
