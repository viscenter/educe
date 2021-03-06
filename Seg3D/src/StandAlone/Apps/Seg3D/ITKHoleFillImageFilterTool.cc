
#include <StandAlone/Apps/Seg3D/ITKHoleFillImageFilterTool.h>
#include <StandAlone/Apps/Seg3D/Seg3DFrame.h>
#include <StandAlone/Apps/Seg3D/Painter.h>
#include <StandAlone/Apps/Seg3D/VolumeOps.h>
#include <Core/Util/Assert.h>

namespace SCIRun {


ITKHoleFillImageFilterTool::ITKHoleFillImageFilterTool(Painter *painter)
  : SeedTool("ITKHoleFillImageFilterTool::", painter)
{
}


void
ITKHoleFillImageFilterTool::run_filter()
{
  if (!painter_->check_for_active_label_volume("Hole fill filter"))
  {
    return;
  }

  painter_->volume_lock_.lock();

  // Save off the source.
  NrrdVolumeHandle source_volume = painter_->current_volume_;

  // Make a new label volume
  const string name = painter_->current_volume_->name_ + " Threshold";
  painter_->create_new_label(painter_->current_volume_, name);

  painter_->rebuild_layer_buttons();
  painter_->volume_lock_.unlock();

  NrrdDataHandle mnrrd;
  unsigned int mlabel = 0;
  if (painter_->mask_volume_.get_rep())
  {
    mnrrd = painter_->mask_volume_->nrrd_handle_;
    mlabel = painter_->mask_volume_->label_;
  }

  vector<vector<int> > iseeds;
  convert_seeds_to_indices(iseeds, source_volume);

  // Set a default seed if none are set.
  if (iseeds.empty())
  {
    vector<int> iseed;
    iseed.push_back(0);
    iseed.push_back(0);
    iseed.push_back(0);
    iseed.push_back(0);
    iseeds.push_back(iseed);
  }

  painter_->start_progress();

  // Flood fill the outer area.
  BrushFloodFill::flood_fill(painter_,
                             painter_->current_volume_->nrrd_handle_,
                             painter_->current_volume_->label_,
                             source_volume->nrrd_handle_,
                             source_volume->label_,
                             mnrrd, mlabel, false, iseeds);

  painter_->update_progress(80);

  // Invert the flood fill area.
  VolumeOps::bit_invert(painter_->current_volume_->nrrd_handle_,
                        painter_->current_volume_->label_,
                        painter_->current_volume_->nrrd_handle_,
                        painter_->current_volume_->label_);
  
  painter_->update_progress(90);

  // Logical or with the source.
  VolumeOps::bit_or(painter_->current_volume_->nrrd_handle_,
                    painter_->current_volume_->label_,
                    painter_->current_volume_->nrrd_handle_,
                    painter_->current_volume_->label_,
                    source_volume->nrrd_handle_,
                    source_volume->label_);

  painter_->finish_progress();

  painter_->extract_all_window_slices();
  painter_->redraw_all();

  painter_->hide_tool_panel();
}


}
