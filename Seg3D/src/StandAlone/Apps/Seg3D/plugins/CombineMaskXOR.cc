#include <StandAlone/Apps/Seg3D/FilterPlugin.h>
 
#include <sci_defs/insight_defs.h>

#include <string>
#include <Core/Events/BaseTool.h>
#include <Core/Datatypes/ITKDatatype.h>
#include <StandAlone/Apps/Seg3D/VolumeOps.h>
#include <StandAlone/Apps/Seg3D/VolumeFilter.h>
#include <itkImageToImageFilter.h>
#include <itkCommand.h>
#include <itkOtsuThresholdImageFilter.h>

SET_PLUGIN_VERSION;

namespace SCIRun {

class ScrollFilterPlugin : public FilterPlugin {
	public:
		virtual std::string get_menu_string() const {
			return "Combine Labels with Logical XOR";
		}
		virtual void menu_event() const {
			// printf("Hello from ExampleFilterPlugin\n");
		}
		BaseTool::propagation_state_e process_event(event_handle_t event) /*const*/ {
			if(dynamic_cast<FinishEvent *>(event.get_rep())) {
				if (!painter_->check_for_active_label_volume("Combine mask with logical XOR"))
				{
					return STOP_E;
				}

				if (!(painter_->mask_volume_.get_rep()))
				{
					painter_->set_status("Combine mask with logical XOR requires a mask to be selected.");
					return STOP_E;
				}

				run_filter();
			}

			return CONTINUE_E;
		}
	private:
		virtual void run_filter() const {	
			painter_->volume_lock_.lock();

			NrrdDataHandle mnrrd = painter_->mask_volume_->nrrd_handle_;
			const label_type mlabel = painter_->mask_volume_->label_;
			painter_->set_status("Using " + painter_->mask_volume_->name_ + " as mask.");

			NrrdDataHandle cnrrd = painter_->current_volume_->nrrd_handle_;
			const label_type clabel = painter_->current_volume_->label_;

			// Make a new child volume.
			const string name = painter_->current_volume_->name_ + " XOR " + painter_->mask_volume_->name_;
			painter_->create_new_label(painter_->current_volume_, name);
			NrrdDataHandle dnrrd = painter_->current_volume_->nrrd_handle_;
			const label_type dlabel = painter_->current_volume_->label_;

			painter_->rebuild_layer_buttons();
			painter_->volume_lock_.unlock();

			// Copy what was in the current volume into the new volume.
			VolumeOps::bit_xor(dnrrd, dlabel, cnrrd, clabel, mnrrd, mlabel);

			painter_->extract_all_window_slices();
			painter_->redraw_all();

			painter_->hide_tool_panel();
		}
};

EXPORT_CREATE_FUNCTION(FilterPlugin, ScrollFilterPlugin)
EXPORT_DELETE_FUNCTION(FilterPlugin, ScrollFilterPlugin)

}
