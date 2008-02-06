#ifndef SEG3D_GenericPlugin_h
#define SEG3D_GenericPlugin_h

#include <string>

enum plugin_types {
	PLUGIN_FILTER, // segmentation
	PLUGIN_TOOL
};

namespace SCIRun {

class GenericPlugin {
	public:
		GenericPlugin() {}
		virtual ~GenericPlugin() {}

		virtual std::string get_menu_string() const = 0;
		virtual int get_type() const = 0;
};

typedef GenericPlugin* create_t();
typedef void destroy_t(GenericPlugin*);

}

#endif // SEG3D_GenericPlugin_h
