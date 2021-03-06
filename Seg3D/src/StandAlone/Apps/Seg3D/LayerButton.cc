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
//    File   : LayerButton.cc
//    Author : McKay Davis
//    Date   : Tue Sep 26 21:56:14 2006


#include <StandAlone/Apps/Seg3D/Painter.h>

namespace SCIRun {

LayerButton::LayerButton(Skinner::Variables *vars, Painter *painter) :
  Parent(vars),
  painter_(painter),
  layer_name_(vars, "LayerButton::name"),
  num_(vars, "LayerButton::num"),
  indent_(vars, "LayerButton::indent",0),
  background_color_(vars, "LayerButton::background_color"),
  color_(vars, "LayerButton::color", Skinner::Color(1., 1., 1., 1.)),
  layer_visible_(vars, "LayerButton::visible",1),
  active_render_target_(vars, "LayerButton::active_render_target", 0),
  color_is_selectable_(vars, "LayerButton::color_is_selectable", 0),
  volume_(0)
{
  REGISTER_CATCHER_TARGET(LayerButton::select);
}


LayerButton::~LayerButton() 
{
}


BaseTool::propagation_state_e
LayerButton::select(event_handle_t &signalh)
{
  painter_->current_volume_ = volume_;
  painter_->rebuild_layer_buttons();
  return CONTINUE_E;
}


BaseTool::propagation_state_e
LayerButton::update_from_gui(event_handle_t &signalh)
{
  if (!volume_.get_rep()) return STOP_E;
  volume_->name_ = layer_name_;
  volume_->tmp_visible_ = layer_visible_;
  return CONTINUE_E;
}

  
}
