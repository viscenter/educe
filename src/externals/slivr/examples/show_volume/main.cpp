// -*- C++ -*- generated by wxGlade 0.4.1 on Tue Jul 10 11:33:23 2007

#include <wx/wx.h>
#include <wx/image.h>
#include "MainWindow.h"



class TestApp: public wxApp {
public:
    bool OnInit();
};

IMPLEMENT_APP(TestApp)

bool TestApp::OnInit()
{
    wxInitAllImageHandlers();
    MainWindow* main_window = new MainWindow(0, -1, wxT(""));
    SetTopWindow(main_window);
    main_window->Show();
    return true;
}
