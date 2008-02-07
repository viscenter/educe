#ifndef SEG3D_FilterPlugin_h
#define SEG3D_FilterPlugin_h

#include <StandAlone/Apps/Seg3D/GenericPlugin.h>
#include <StandAlone/Apps/Seg3D/Painter.h>
#include <Core/Events/BaseTool.h>

namespace SCIRun {

	class FilterPlugin : public virtual GenericPlugin, public virtual BaseTool {
		public:
			virtual propagation_state_e process_event(event_handle_t) const = 0;
			virtual int get_type() const { return PLUGIN_FILTER; }
		private:
			virtual void run_filter() const = 0;
			Painter *	painter_;	
	};

}

#endif // SEG3D_FilterPlugin_h
