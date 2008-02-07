#include "GenericPlugin.h"
#include <iostream>

SET_PLUGIN_VERSION;

namespace SCIRun {

class ExamplePlugin : public GenericPlugin {
	public:
		virtual std::string get_menu_string() const {
			return "ExamplePlugin";
		}
		virtual void menu_event() const {
			printf("Hello from ExamplePlugin\n");
		}
};

extern "C" GenericPlugin* create() {
	return new ExamplePlugin;
}

extern "C" void destroy(GenericPlugin* g) {
	delete g;
}

}
