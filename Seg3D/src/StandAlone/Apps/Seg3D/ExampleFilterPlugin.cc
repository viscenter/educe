#include "FilterPlugin.h"

SET_PLUGIN_VERSION;

// g++ -I../../../../thirdparty.bin/3.1.0/Linux/gcc-4.1.3-64bit/include/InsightToolkit/Algorithms/ -I../../../../thirdparty.bin/3.1.0/Linux/gcc-4.1.3-64bit/include/InsightToolkit/Utilities/ -I../../../../thirdparty.bin/3.1.0/Linux/gcc-4.1.3-64bit/include/InsightToolkit/IO/ -I../../../../thirdparty.bin/3.1.0/Linux/gcc-4.1.3-64bit/include/InsightToolkit/BasicFilters/ -I../../../../thirdparty.bin/3.1.0/Linux/gcc-4.1.3-64bit/include/InsightToolkit/ -I../../../../thirdparty.bin/3.1.0/Linux/gcc-4.1.3-64bit/include/InsightToolkit/Utilities/vxl/core/ -I../../../../thirdparty.bin/3.1.0/Linux/gcc-4.1.3-64bit/include/InsightToolkit/Utilities/vxl/vcl/ -I../../../../thirdparty.bin/3.1.0/Linux/gcc-4.1.3-64bit/include/InsightToolkit/Common/ -I../../../../src/externals/glew/include/ -I../../../../src/externals/ `../../../../thirdparty.bin/3.1.0/Linux/gcc-4.1.3-64bit/bin/wx-config --cppflags` -I../../../ -I../../../../src/include/ -I../../../../bin/include/ -fPIC -shared -o ExampleFilterPlugin ExampleFilterPlugin.cc

namespace SCIRun {

class ExampleFilterPlugin : public FilterPlugin {
	public:
		virtual std::string get_menu_string() const {
			return "ExampleFilterPlugin";
		}
		virtual void menu_event() const {
			printf("Hello from ExampleFilterPlugin\n");
		}
		virtual propagation_state_e process_event(event_handle_t) const {
			return CONTINUE_E;
		}
	private:
		virtual void run_filter() const {	}
};

extern "C" GenericPlugin* create() {
	return (GenericPlugin*)new ExampleFilterPlugin;
}

extern "C" void destroy(GenericPlugin* g) {
	delete g;
}

}
