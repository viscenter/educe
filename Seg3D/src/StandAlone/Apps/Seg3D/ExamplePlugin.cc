#include "GenericPlugin.h"

namespace SCIRun {

class ExamplePlugin : public GenericPlugin {
	public:
		virtual std::string get_menu_string() const {
			return "ExamplePlugin";
		}
};

extern "C" GenericPlugin* create() {
	return new ExamplePlugin;
}

extern "C" void destroy(GenericPlugin* g) {
	delete g;
}

}
