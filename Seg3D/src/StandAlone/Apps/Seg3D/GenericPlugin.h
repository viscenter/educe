#ifndef SEG3D_GenericPlugin_h
#define SEG3D_GenericPlugin_h

#include <string>

// Bump when the GenericPlugin API is changed
#define PLUGIN_VERSION 00000001

#define CHECK_PLUGIN_VERSION extern "C" int check_plugin_version(int i){return i != PLUGIN_VERSION;}
typedef int check_plugin_version_t(int);

namespace SCIRun {

enum plugin_types {
	PLUGIN_FILTER, // segmentation
	PLUGIN_TOOL
};

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
