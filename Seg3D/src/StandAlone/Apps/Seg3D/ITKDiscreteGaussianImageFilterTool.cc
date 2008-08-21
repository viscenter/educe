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
//    File   : ITKDiscreteGaussianImageFilterTool.cc
//    Author : David Brayford
//    Date   : May 2008

#include <StandAlone/Apps/Seg3D/ITKDiscreteGaussianImageFilterTool.h>

#include <StandAlone/Apps/Seg3D/Seg3DFrame.h>
#include <StandAlone/Apps/Seg3D/BrushTool.h>
#include <StandAlone/Apps/Seg3D/Painter.h>
#include <StandAlone/Apps/Seg3D/VolumeOps.h>
#include <Core/Util/Assert.h>

#include <StandAlone/Apps/Seg3D/GuiCode/itkDiscreteGaussianImageFilter.h>
#include <StandAlone/Apps/Seg3D/Seg3DwxGuiUtils.h>
#include <wx/string.h>
#include <wx/variant.h>

namespace SCIRun {

ITKDiscreteGaussianImageFilterTool::
ITKDiscreteGaussianImageFilterTool(Painter *painter) :
  BaseTool("ITK Discrete Gaussian ImageFilter Tool"),
  painter_(painter),
  Variance_(painter->get_vars(), "itkDiscreteGaussianImageFilter::mVariance"),
  Max_Error_(painter->get_vars(), "itkDiscreteGaussianImageFilter::mMaxError"),
  filter_()
{
}


BaseTool::propagation_state_e 
ITKDiscreteGaussianImageFilterTool::process_event
(event_handle_t event)
{
  // check for the FinishEvent signal generated by the start button
  if (dynamic_cast<FinishEvent *>(event.get_rep())) {
    if (!painter_->check_for_active_data_volume("Discrete Gaussian filter"))
    {
      return STOP_E;
    }
    run_filter();
  }
 
  return CONTINUE_E;
}


void
ITKDiscreteGaussianImageFilterTool::run_filter()
{
  filter_.set_volume( painter_->copy_current_layer(" Discrete Gaussian") );

  // Set the filter parameters.
  const double variance = Variance_;
  const int max_kernel_width = Max_Error_;

  filter_->SetVariance(variance);
  if (max_kernel_width > 0)
  {
    filter_->SetMaximumKernelWidth(max_kernel_width);
  }

  painter_->set_status("Running discrete gaussian filter.");
  painter_->start_progress();
  filter_();
  painter_->finish_progress();

  painter_->current_volume_->reset_data_range();

  UndoHandle undo =
    new UndoReplaceLayer(painter_, "Undo Discrete Gaussian",
                         0, painter_->current_volume_, 0);
  painter_->push_undo(undo);

  // Redraw everything after completion.
  painter_->extract_all_window_slices();
  painter_->redraw_all();

  painter_->hide_tool_panel();
}


}
