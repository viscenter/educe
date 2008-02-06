//    File   : plugins.h
//    Author : Ryan Baumann
//    Date   : Tue Feb  5 16:17:59 EST 2008

#ifndef SEG3D_plugins_h
#define SEG3D_plugins_h

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/help.h>

#include <vector>
#include <string>
#include <map>

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include <iostream>

#include <StandAlone/Apps/Seg3D/GenericPlugin.h>

namespace fs = boost::filesystem;

namespace SCIRun {

class Plugin {
	private:
		fs::path fullpath;
		void * dlpointer;
	public:
		GenericPlugin * gp;
		Plugin() {}
		Plugin(fs::path fp, void * dlp) { fullpath = fp; dlpointer = dlp; }
		// SetPluginInstance(GenericPlugin * instance) { gp = instance; }
};

class PluginManager {
	private:
		std::vector<fs::path> ScanPlugins(std::string directory);
		std::map<int, Plugin> plugins;
		int curwxid;
	public:
		PluginManager(int wxid) { curwxid = wxid; }
		void LoadPlugins(wxMenu * menu);
};

}

#endif // SEG3D_plugins_h
