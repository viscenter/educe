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
//    File   : ITKThresholdSegmentationLevelSetImageFilterTool.cc
//    Author : McKay Davis
//    Date   : Sat Oct 14 14:52:24 2006

#include <StandAlone/Apps/Seg3D/ITKThresholdSegmentationLevelSetImageFilterTool.h>

#include <StandAlone/Apps/Seg3D/Seg3DFrame.h>
#include <StandAlone/Apps/Seg3D/BrushTool.h>
#include <StandAlone/Apps/Seg3D/Painter.h>
#include <StandAlone/Apps/Seg3D/VolumeOps.h>
#include <Core/Util/Assert.h>


namespace SCIRun {

ITKThresholdSegmentationLevelSetImageFilterTool::
ITKThresholdSegmentationLevelSetImageFilterTool(Painter *painter) :
  BaseTool("ITKThresholdSegmentationLevelSetImageFilterTool::"),
  painter_(painter),
  seed_volume_(0),
  LowerThreshold_(painter->get_vars(), get_name()+"LowerThreshold"),  
  UpperThreshold_(painter->get_vars(), get_name()+"UpperThreshold"),
  filter_(),
  started_(false)
{
  Skinner::Var<bool> ready(painter_->get_vars(), "Filter::ready", 0);
  ready = 0;
}

BaseTool::propagation_state_e 
ITKThresholdSegmentationLevelSetImageFilterTool::process_event
(event_handle_t event)
{
  if (dynamic_cast<SetLayerEvent *>(event.get_rep())) {
    if (!(painter_->current_volume_.get_rep() &&
          painter_->current_volume_->label_))
    {
      painter_->set_status("No active label volume to set as seed volume.");
      return STOP_E;
    }
    seed_volume_ = painter_->current_volume_;
    Skinner::Var<bool> ready(painter_->get_vars(), "Filter::ready", 0);
    ready = 1;
    painter_->redraw_all();
    return STOP_E;
  }

  if (dynamic_cast<SetDataLayerEvent *>(event.get_rep())) 
  {
     
	  if (!(painter_->current_volume_.get_rep() &&
          painter_->current_volume_->label_))
    {
      painter_->set_status("No active label volume to set as data volume.");
      return STOP_E;
    }

    if (painter_->current_volume_ == seed_volume_) 
	{
      painter_->set_status("Cannot use same volumes for source and seed.");
      painter_->redraw_all();
      return STOP_E;
    }
	else
		source_volume_ = painter_->current_volume_;

    
    if (!seed_volume_.get_rep()) 
	{
      painter_->set_status("No seed volume set.");
      painter_->redraw_all();
      return STOP_E;
    }
  }

  if (dynamic_cast<FinishEvent *>(event.get_rep())) {
/*    if (painter_->current_volume_ == seed_volume_) {
      painter_->set_status("Cannot use same volumes for source and seed.");
      painter_->redraw_all();
      return STOP_E;
    }
    
    if (!seed_volume_.get_rep()) {
      painter_->set_status("No seed volume set.");
      painter_->redraw_all();
      return STOP_E;
    }
*/
    run_filter();

    if (filter_.stopped()) {
      return CONTINUE_E;
    }

  }

  if (dynamic_cast<QuitEvent *>(event.get_rep())) {
    return QUIT_AND_CONTINUE_E;
  }
 
  return CONTINUE_E;
}


#define SetFilterVarMacro(name, type) \
  filter_->Set##name(painter_->get_vars()->get_##type(get_name()+#name));

void
ITKThresholdSegmentationLevelSetImageFilterTool::set_vars()
{
  SetFilterVarMacro(CurvatureScaling, double);
  SetFilterVarMacro(PropagationScaling, double);
  SetFilterVarMacro(EdgeWeight, double);
  SetFilterVarMacro(NumberOfIterations, int);
  SetFilterVarMacro(MaximumRMSError, double);
  SetFilterVarMacro(IsoSurfaceValue, double);
  SetFilterVarMacro(SmoothingIterations,int);
  SetFilterVarMacro(SmoothingTimeStep, double);
  SetFilterVarMacro(SmoothingConductance, double);
  if (painter_->get_vars()->get_bool(get_name()+"ReverseExpansionDirection")) 
    filter_->ReverseExpansionDirectionOn();
  else 
    filter_->ReverseExpansionDirectionOff();
}
  

void
ITKThresholdSegmentationLevelSetImageFilterTool::run_filter()
{
  if (started_) {
  
    set_vars();
    filter_->ManualReinitializationOn();
    filter_->Modified();
    filter_.update();
    
    return;
  }
  started_ = true;

  painter_->volume_lock_.lock();
  NrrdVolumeHandle cur_layer = painter_->current_volume_;
  painter_->current_volume_ = seed_volume_;
  NrrdVolumeHandle new_layer = painter_->copy_current_layer("LevelSet");
  new_layer->change_type_from_bit_to_float(1000.0);

  pair<double, double> meandev = VolumeOps::nrrd_mean_and_deviation
    (cur_layer->nrrd_handle_, new_layer->nrrd_handle_);

  set_vars();
  ITKDatatypeHandle fimageh = cur_layer->get_itk_image();
  FeatureImg *fimage = dynamic_cast<FeatureImg *>(fimageh->data_.GetPointer());

  double factor = 2.5;
  LowerThreshold_ = meandev.first - factor * meandev.second;
  UpperThreshold_ = meandev.first + factor * meandev.second;
  painter_->set_status("Threshold min: " + to_string(LowerThreshold_) + 
                       " Threshold max: " + to_string(UpperThreshold_));
  filter_->SetLowerThreshold(LowerThreshold_);
  filter_->SetUpperThreshold(UpperThreshold_);    
  filter_->SetFeatureImage(fimage);
  filter_.set_volume(new_layer);
  painter_->volume_lock_.unlock();
  filter_.start();

  painter_->volume_lock_.lock();
  new_layer->change_type_from_float_to_bit();
  painter_->volume_lock_.unlock();

  painter_->hide_tool_panel();
}


}
