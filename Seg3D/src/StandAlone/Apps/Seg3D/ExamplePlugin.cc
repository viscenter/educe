#include "GenericPlugin.h"

namespace SCIRun {

class ExamplePlugin : public GenericPlugin {
	public:
		virtual std::string get_menu_string() const {
			return "ExamplePlugin";
		}
		virtual int get_type() const {
			return PLUGIN_FILTER;
		}
};

extern "C" GenericPlugin* create() {
	return new ExamplePlugin;
}

extern "C" void destroy(GenericPlugin* g) {
	delete g;
}

}
