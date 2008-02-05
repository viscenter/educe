//    File   : plugins.h
//    Author : Ryan Baumann
//    Date   : Tue Feb  5 16:17:59 EST 2008

#ifndef SEG3D_plugins_h
#define SEG3D_plugins_h

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/help.h>

#include <vector>

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include <iostream>

namespace fs = boost::filesystem;

namespace SCIRun {

		std::vector<fs::path> ScanPlugins(void);
		void LoadPlugins(wxMenu * menu);

}

#endif // SEG3D_plugins_h
