#ifndef SEG3D_GenericPlugin_h
#define SEG3D_GenericPlugin_h

#include <string>

// Bump when the GenericPlugin API is changed
#define PLUGIN_VERSION 00000002

#define SET_PLUGIN_VERSION extern "C" const int plugin_version = PLUGIN_VERSION

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
		virtual void menu_event( ) const = 0; 
};

typedef GenericPlugin* create_t();
typedef void destroy_t(GenericPlugin*);

}

#endif // SEG3D_GenericPlugin_h
