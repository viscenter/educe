#include <stdio.h>
#include <iostream>
#include <wx/wx.h>
#include <wx/glcanvas.h>


//! converts wxString to std:string
std::string wx2std(const wxString& input, wxMBConv* conv = NULL);

//! converts std:string to wxString
wxString std2wx(const std::string& input, wxMBConv* conv = NULL);
