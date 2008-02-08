#ifndef SEG3D_FilterPlugin_h
#define SEG3D_FilterPlugin_h

#include <StandAlone/Apps/Seg3D/GenericPlugin.h>
#include <StandAlone/Apps/Seg3D/Painter.h>
#include <Core/Events/BaseTool.h>

namespace SCIRun {

	class FilterPlugin : public virtual BaseTool, public virtual GenericPlugin {
		protected:
			Painter *	painter_;	
		public:
			FilterPlugin(Painter * ip = NULL) : painter_(ip) {}
			virtual BaseTool::propagation_state_e process_event(event_handle_t) { printf("Processing event inside FilterPlugin\n"); return CONTINUE_E; }
			virtual int get_type() const { return PLUGIN_FILTER; }
			virtual void set_painter(Painter * ip) { painter_ = ip; }
			virtual std::string get_menu_string() const { return "FilterPlugin"; }
			virtual void menu_event() const { printf("TPME\n"); }

		private:
			virtual void run_filter() const { printf("Running FilterPlugin\n"); }
	};

}

#endif // SEG3D_FilterPlugin_h
