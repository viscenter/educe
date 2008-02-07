#ifndef SEG3D_GenericPlugin_h
#define SEG3D_GenericPlugin_h

#include <string>

// Bump when the plugin API is changed
#define PLUGIN_VERSION 00000003

#define SET_PLUGIN_VERSION extern "C" const int plugin_version = PLUGIN_VERSION
#define EXPORT_CREATE_FUNCTION(T) extern "C" \
	GenericPlugin* create() { \
	return new T; \
	}
#define EXPORT_DELETE_FUNCTION(T) extern "C" \
	void destroy(GenericPlugin* g) { \
	delete g; \
	}

namespace SCIRun {

enum plugin_types {
	PLUGIN_FILTER, // segmentation
	PLUGIN_TOOL,
	PLUGIN_GENERIC
};

class GenericPlugin {
	public:
		GenericPlugin() {}
		virtual ~GenericPlugin() {}

		virtual std::string get_menu_string() const = 0;
		virtual int get_type() const { return PLUGIN_GENERIC; }
		virtual void menu_event( ) const = 0; 
};

typedef GenericPlugin* create_t();
typedef void destroy_t(GenericPlugin*);

}

#endif // SEG3D_GenericPlugin_h
