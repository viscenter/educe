//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2006 Scientific Computing and Imaging Institute,
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
//    File   : KeyToolSelectorTool.cc
//    Author : McKay Davis
//    Date   : Sun Oct 15 11:43:45 2006

#include <StandAlone/Apps/Seg3D/KeyToolSelectorTool.h>
#include <StandAlone/Apps/Seg3D/Painter.h>
#include <StandAlone/Apps/Seg3D/AutoviewTool.h>
#include <StandAlone/Apps/Seg3D/BrushTool.h>
#include <StandAlone/Apps/Seg3D/CLUTLevelsTool.h>
#include <StandAlone/Apps/Seg3D/CropTool.h>
#include <StandAlone/Apps/Seg3D/FloodfillTool.h>
#include <StandAlone/Apps/Seg3D/PanTool.h>
#include <StandAlone/Apps/Seg3D/ProbeTool.h>
#include <StandAlone/Apps/Seg3D/StatisticsTool.h>
#include <StandAlone/Apps/Seg3D/ZoomTool.h>

namespace SCIRun {


KeyToolSelectorTool::KeyToolSelectorTool(Painter *painter) :
  KeyTool("Painter KeyToolSelectorTool"),
  painter_(painter),
  tm_(painter->tm_)
{
}
  

KeyToolSelectorTool::~KeyToolSelectorTool()
{
}

BaseTool::propagation_state_e
KeyToolSelectorTool::key_press(string, int keyval,
                               unsigned int, unsigned int)
{
  if (!painter_->cur_window_) return STOP_E;
  SliceWindow &window = *painter_->cur_window_;
  if (sci_getenv_p("SCI_DEBUG"))
    cerr << "keyval: " << keyval << std::endl;
  event_handle_t event;
  switch (keyval) {
  case SCIRun_equal:    window.zoom_in(event); break;
  case SCIRun_minus:    window.zoom_out(event); break;
  case SCIRun_comma:    window.move_slice(-1); break;
  case SCIRun_period:   window.move_slice(1); break;

  case SCIRun_Left:     painter_->current_layer_up();break;
  case SCIRun_Right:    painter_->current_layer_down();break;
  case SCIRun_Up:       window.move_slice(1);break;
  case SCIRun_Down:     window.move_slice(-1);break;

#if 0
  case SCIRun_c:        painter_->CopyLayer(event); break;
  case SCIRun_x:        painter_->DeleteLayer(event); break;
  case SCIRun_v:        painter_->NewLayer(event);break;
  case SCIRun_f:        
  if (painter_->cur_window_) {
    painter_->cur_window_->autoview(painter_->current_volume_);
  } break;

  // Reset CLUT
  case SCIRun_r: { 
    if (painter_->current_volume_.get_rep()) {
      painter_->current_volume_->reset_clut();
    } 
  } break;

  case SCIRun_p:        painter_->opacity_up();break;
  case SCIRun_o:        painter_->opacity_down();break;

  case SCIRun_u:
    if (painter_->current_volume_.get_rep()) {
      painter_->current_volume_->colormap_ = 
        Max(0,painter_->current_volume_->colormap_-1);
      painter_->set_all_slices_tex_dirty();
      painter_->redraw_all();
    } break;
  case SCIRun_i:
    if (painter_->current_volume_.get_rep()) {
      painter_->current_volume_->colormap_ = 
        Min(int(painter_->colormaps_.size()), 
            painter_->current_volume_->colormap_+1);
      painter_->set_all_slices_tex_dirty();
      painter_->redraw_all();
    } break;    
#endif
  }

  return CONTINUE_E;
}  


} // End namespace SCIRun
