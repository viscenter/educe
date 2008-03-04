
#include <StandAlone/Apps/Seg3D/FloodFillCopyTool.h>
#include <StandAlone/Apps/Seg3D/Seg3DFrame.h>
#include <StandAlone/Apps/Seg3D/Painter.h>
#include <StandAlone/Apps/Seg3D/VolumeOps.h>
#include <Core/Util/Assert.h>

namespace SCIRun {


FloodFillCopyTool::FloodFillCopyTool(Painter *painter)
  : SeedTool("FloodFillCopyTool::", painter)
{
}


void
FloodFillCopyTool::run_filter()
{
  if (!painter_->check_for_active_label_volume("Hole fill filter"))
  {
    return;
  }

  if (seeds_.empty())
  {
    painter_->set_status("Flood fill copy requires at least one seed point.");
    return;
  }

  painter_->volume_lock_.lock();

  // Save off the source.
  NrrdVolumeHandle source_volume = painter_->current_volume_;

  vector<vector<int> > iseeds;
  convert_seeds_to_indices(iseeds, source_volume);

  if (iseeds.empty())
  {
    painter_->set_status("All of the seed points were outside the volume.");
    painter_->volume_lock_.unlock();
    return;
  }

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

  // TODO:  This is inefficient but easy to implement.  It shouldn't be
  // hard to redo this without the two inverts (just changes the inside
  // test in flood_fill.

  painter_->start_progress();

  // Invert the source so that the selected thing is a hole.
  VolumeOps::bit_invert(source_volume->nrrd_handle_,
                        source_volume->label_,
                        source_volume->nrrd_handle_,
                        source_volume->label_);

  // Flood fill the source 'hole' into the destination.
  BrushFloodFill::flood_fill(painter_,
                             painter_->current_volume_->nrrd_handle_,
                             painter_->current_volume_->label_,
                             source_volume->nrrd_handle_,
                             source_volume->label_,
                             mnrrd, mlabel, false, iseeds);

  painter_->update_progress(90);

  // Reinvert the source back to it's original.
  VolumeOps::bit_invert(source_volume->nrrd_handle_,
                        source_volume->label_,
                        source_volume->nrrd_handle_,
                        source_volume->label_);

  painter_->finish_progress();

  painter_->extract_all_window_slices();
  painter_->redraw_all();

  painter_->hide_tool_panel();
}


}
