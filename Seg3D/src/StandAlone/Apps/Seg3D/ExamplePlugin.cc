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

EXPORT_CREATE_FUNCTION(GenericPlugin, ExamplePlugin)
EXPORT_DELETE_FUNCTION(GenericPlugin, ExamplePlugin)

}
