//
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  
//    File   : main.cc
//    Author : McKay Davis
//    Date   : Tue May 30 21:38:23 MDT 2006


#include <Core/Util/Environment.h>
#include <Core/Skinner/XMLIO.h>
#include <Core/Skinner/Skinner.h>
#include <Core/Skinner/Window.h>
#include <StandAlone/Apps/Seg3D/Seg3DFrame.h>
#include <StandAlone/Apps/Seg3D/Painter.h>
#include <StandAlone/Apps/Seg3D/config.h>
#include <StandAlone/Apps/Seg3D/plugins.h>

#include <Core/Events/EventManager.h>

#include <sgi_stl_warnings_off.h>
#include <iostream>
#include <sgi_stl_warnings_on.h>

#include <sci_defs/wx_defs.h>
#include <sci_defs/x11_defs.h>

using namespace SCIRun;

#include <wx/wx.h>
#include <wx/splash.h>
#include <wx/help.h>
#include <wx/stdpaths.h>
#include <wx/xrc/xmlres.h>
#include <wx/fs_zip.h>

class Seg3D : public wxApp 
{
public:
  // Called on application startup
  virtual bool OnInit();
  virtual int OnExit();
  Skinner::GLWindow *skinner_window_;

  Thread *event_manager_thread_;
};


DECLARE_APP(Seg3D)
IMPLEMENT_APP_CONSOLE(Seg3D)


bool
Seg3D::OnInit()
{
  // Look for the data directory in the current directory and the install path.
  const char *datapath = "./data";
  if (validDir(SYSDATAPATH))
  {
    datapath = SYSDATAPATH;
  }
#ifndef __APPLE__
  else if (!validDir(datapath))
  {
    cout << "Seg3D Error: Could not locate Seg3D data directory.  Exiting.\n";
    exit(0);
  }
#endif

#ifndef _DEBUG 
  // Turn off splash screen on debug - it's too hard to debug with the
  // splash screen that won't go anywhere in the way.
  wxString splashScreenPath;
  splashScreenPath = std2wx(datapath + string("/splash-seg3d.png"));
  
#if defined(__APPLE__)
  wxStandardPathsCF * paths = new wxStandardPathsCF();
  splashScreenPath = paths->GetResourcesDir() + wxT("/splash-seg3d.png");
#endif
    
  wxImage::AddHandler( new wxPNGHandler );
  wxBitmap bitmap;            
  if (bitmap.LoadFile(splashScreenPath, wxBITMAP_TYPE_PNG))
  {
    new wxSplashScreen(bitmap,
                       wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT,
                       4000, 0, -1, wxDefaultPosition, wxDefaultSize,
                       wxSIMPLE_BORDER|wxFRAME_NO_TASKBAR);
  }
  wxYield();
#endif // _DEBUG
	
  // TODO: May be bad cast somewhere or another.
  create_sci_environment(0, (char *)argv[0]);  

  // Need to make the frame before the Painter (since Painter's tools
  // will refer to.
  wxXmlResource::Get()->InitAllHandlers();
  wxFileSystem::AddHandler(new wxZipFSHandler);
  wxString xrcPath = std2wx(datapath + string("/Seg3D.xrc"));

#if defined(__APPLE__)
  wxStandardPathsCF * paths2 = new wxStandardPathsCF();
  xrcPath = paths2->GetResourcesDir() + wxT("/Seg3D.xrc");
#endif
  
  wxXmlResource::Get()->Load(xrcPath);

  Seg3DFrame* frame  = 
    new Seg3DFrame("Seg3D", NULL, _T("Seg3D"),
                   wxDefaultPosition, wxSize(900+PANEL_WIDTH, 750));

  Painter::global_seg3dframe_pointer_ = frame;
  Skinner::XMLIO::register_maker<Painter>();
  Skinner::Skinner *skinner = Skinner::load_default_skin(datapath);
  if (!skinner) {
    cerr << "Errors encounted loading default skin.\n";
    return false;
  }

  Skinner::GLWindow *skinner_window = skinner->get_windows()[0];
  
  skinner_window_ = skinner_window;
  // Attach the skinner window to it's opengl context.
  skinner_window->attach_to_context(frame->getContext());

  EventManager *em = new EventManager();
  event_manager_thread_ = new Thread(em, "Event Manager");

#if defined(__APPLE__)
  frame->Move(wxDefaultPosition);
  Painter::ThrowSkinnerSignal("Painter::Autoview");
#endif
    
  return true;
}


int
Seg3D::OnExit()
{
  delete skinner_window_;
  EventManager::add_event( new QuitEvent() );
  event_manager_thread_->join();
  return wxApp::OnExit();
}


