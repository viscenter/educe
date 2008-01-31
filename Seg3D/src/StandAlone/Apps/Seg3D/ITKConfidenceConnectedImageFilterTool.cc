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
//    File   : ITKConfidenceConnectedImageFilterTool.cc
//    Author : McKay Davis
//    Date   : Tue Sep 26 18:44:34 2006

#include <StandAlone/Apps/Seg3D/Painter.h>

#include <StandAlone/Apps/Seg3D/ITKConfidenceConnectedImageFilterTool.h>
#include <sci_gl.h>


namespace SCIRun {

ITKConfidenceConnectedImageFilterTool::
ITKConfidenceConnectedImageFilterTool(Painter *painter) :
  BaseTool("ITK Confidence Connected\nImage Filter"),
  PointerTool("ITK Confidence Connected\nImage Filter"),
  painter_(painter),
  seeds_(),
  volume_(0),
  prefix_("ITKConfidenceConnectedImageFilterTool::"),
  numberOfIterations_(painter_->get_vars(), prefix_+"numberOfIterations"),
  multiplier_(painter_->get_vars(), prefix_+"multiplier"),
  replaceValue_(painter_->get_vars(), prefix_+"replaceValue"),
  initialNeighborhoodRadius_(painter_->get_vars(), 
                             prefix_+"initialNeighborhoodRadius"),
  filter_(),
  seed_lock_("ITKConfidence Connected Seeds")
{
}


void
ITKConfidenceConnectedImageFilterTool::run_filter()
{
  if (!volume_.get_rep()) 
    return;


  for (size_t i = 0; i < seeds_.size(); ++i) {
    if (!volume_->index_valid(seeds_[i])) {
      painter_->set_status("Invalid seed point, please place seeds again.");
      return;
    }
  }

  typedef itk::ConfidenceConnectedImageFilter
    < ITKImageFloat3D, ITKImageLabel3D > FilterType;


  for (size_t i = 0; i < seeds_.size(); ++i) {
    FilterType::IndexType seed_point;
    for(unsigned int j = 0; j < seed_point.GetIndexDimension(); j++) {
      seed_point[j] = seeds_[i][j+1];
    }
    filter_->AddSeed(seed_point);
  }

  string status = "Confidence Connected Filter running with " +
    to_string(seeds_.size()) + " seed point";
  if (seeds_.size() > 1) status += "s";
  status += ".";
  painter_->set_status(status);

  filter_->SetNumberOfIterations(numberOfIterations_);
  filter_->SetMultiplier(multiplier_);
  filter_->SetReplaceValue(replaceValue_);
  filter_->SetInitialNeighborhoodRadius(initialNeighborhoodRadius_);

  event_handle_t event = 0;
  painter_->CreateLabelVolume(event);
  filter_.set_volume(painter_->current_volume_);

  painter_->start_progress();
  filter_.start(volume_->nrrd_handle_);
  painter_->finish_progress();

  volume_->set_dirty();
  painter_->extract_all_window_slices();
  painter_->redraw_all();

  painter_->hide_tool_panel();
}




BaseTool::propagation_state_e
ITKConfidenceConnectedImageFilterTool::pointer_down
(int b, int x, int y, unsigned int m, int t)
{
  if (b == 1 && !m) {
    if (!volume_.get_rep()) {
      volume_ = painter_->current_volume_;
    }

    seed_lock_.lock();
    if (volume_ != painter_->current_volume_) {
      seeds_.clear();
    }
    seed_lock_.unlock();

    if (volume_.get_rep()) {
      seed_lock_.lock();
      vector<int> newseed = volume_->world_to_index(painter_->pointer_pos_);
      if (volume_->index_valid(newseed)) {
        seeds_.push_back(newseed);
      }
      seed_lock_.unlock();
      painter_->redraw_all();
      return STOP_E;
    }
  }

  return CONTINUE_E;;
}

BaseTool::propagation_state_e
ITKConfidenceConnectedImageFilterTool::pointer_up
(int b, int x, int y, unsigned int m, int t)
{
  return CONTINUE_E;
}

BaseTool::propagation_state_e
ITKConfidenceConnectedImageFilterTool::pointer_motion
(int b, int x, int y, unsigned int m, int t)
{
  if (!volume_.get_rep()) {
    return CONTINUE_E;
  }

  if (b == 1 && !m) {
    seed_lock_.lock();
    vector<int> newseed = volume_->world_to_index(painter_->pointer_pos_);
    if (volume_->index_valid(newseed)) {
      seeds_.back() = newseed;
    }
    seed_lock_.unlock();
    painter_->redraw_all();
    return STOP_E;
  }
  return CONTINUE_E;

}



BaseTool::propagation_state_e 
ITKConfidenceConnectedImageFilterTool::process_event(event_handle_t event)
{
  RedrawSliceWindowEvent *redraw = 
    dynamic_cast<RedrawSliceWindowEvent *>(event.get_rep());
  if (redraw) {
    draw_gl(redraw->get_window());
  }

  if (dynamic_cast<SetLayerEvent *>(event.get_rep())) {
    seed_lock_.lock();
    seeds_.clear();
    seed_lock_.unlock();
    painter_->redraw_all();
  }

  if (dynamic_cast<FinishEvent *>(event.get_rep())) {
    if (!painter_->check_for_active_data_volume("Confidence connected filter"))
    {
      return STOP_E;
    }

    if (seeds_.empty())
    {
      painter_->set_status("Confidence Connected Filter requires one or more seed points.");
      return STOP_E;
    }

    run_filter();

    if (filter_.stopped()) {
      return CONTINUE_E;
    }
  }

  return CONTINUE_E;
}
  

void
ITKConfidenceConnectedImageFilterTool::draw_gl(SliceWindow &window)
{
  if (!volume_.get_rep()) return;

  seed_lock_.lock();
  for (size_t i = 0; i < seeds_.size(); ++i) {
    draw_seed(seeds_[i],window);
  }
  seed_lock_.unlock();
}


void
ITKConfidenceConnectedImageFilterTool::draw_seed(seed_t &seed,
                                                 SliceWindow &window)
{
  if (!volume_->index_valid(seed)) return;
  vector<double> index(seed.size());
  index[0] = seed[0];
  for( unsigned int sIndex = 1; sIndex < index.size(); ++sIndex ) {
    index[ sIndex ] = seed[sIndex] + 0.5;
  }

  Vector left = window.x_dir();
  Vector up = window.y_dir();
  Point center = volume_->index_to_point(index);
  Point p;

  vector<int> pidx = volume_->world_to_index(center);
  vector<int> widx = volume_->world_to_index(window.center_);
  bool in = (pidx[window.axis_+1] == widx[window.axis_+1]);
 
  double scale = in ? 7.0 : 4.0;

  //  double one = 100.0 / window.zoom_; // world space units per one pixel
  double units = window.zoom_ / 100.0;  // Pixels per world space unit
  double units_over_2 = units/2.0;
  double e = units_over_2 + Clamp( units_over_2, scale, Max(units, scale));

  for (int pass = 0; pass < 3; ++pass) {
    glLineWidth(Max(scale - pass*2.0, 1.0));
    if (pass == 0)
      glColor4d(0.0, 0.0, 0.0, 1.0);
    else if (pass == 1)
      glColor4d(in ? 0.0 : 0.0, in ? 1.0 : 0.6, 0.0, 1.0);
    else
      glColor4d(in ? 0.5 : 0.5, in ? 1.0 : 0.9, 0.5, 1.0);

    glBegin(GL_LINES);    
    p = center + units_over_2 * up;
    glVertex3dv(&p(0));
    p = center + e * up;
    glVertex3dv(&p(0));
    
    p = center - units_over_2 * up;
    glVertex3dv(&p(0));
    p = center - e * up;
    glVertex3dv(&p(0));
    
    p = center + units_over_2 * left;
    glVertex3dv(&p(0));
    p = center + e * left;
    glVertex3dv(&p(0));
    
    p = center - units_over_2 * left;
    glVertex3dv(&p(0));
    p = center - e * left;
    glVertex3dv(&p(0));
    glEnd();
    CHECK_OPENGL_ERROR();
  }

  glLineWidth(1.0);
} // end draw_seed()

}

