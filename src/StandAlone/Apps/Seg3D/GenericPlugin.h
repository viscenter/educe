#ifndef SEG3D_GenericPlugin_h
#define SEG3D_GenericPlugin_h

#include <string>

// Bump when the plugin API is changed
#define PLUGIN_VERSION 00000004

#define SET_PLUGIN_VERSION extern "C" const int plugin_version = PLUGIN_VERSION
#define EXPORT_CREATE_FUNCTION(I,T) extern "C" \
	I* create() { \
	return new T; \
	}
#define EXPORT_DELETE_FUNCTION(I,T) extern "C" \
	void destroy(I* g) { \
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
		GenericPlugin() { printf("GenericPlugin constructor called\n"); }
		virtual ~GenericPlugin() { printf("GenericPlugin destructor called\n"); }

		virtual std::string get_menu_string() const = 0;
		virtual int get_type() const { return PLUGIN_GENERIC; }
		virtual void menu_event( ) const = 0; 
};

typedef void* create_t();
typedef void destroy_t(GenericPlugin*);

}

#endif // SEG3D_GenericPlugin_h
