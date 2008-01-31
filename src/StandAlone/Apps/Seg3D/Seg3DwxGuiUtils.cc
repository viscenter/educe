#include <stdio.h>
#include <iostream>
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/help.h>

#include <wx/filedlg.h>
#include <wx/tokenzr.h>
#include <wx/fs_zip.h>

#include <StandAlone/Apps/Seg3D/Seg3DwxGuiUtils.h>

//! converts wxString to std:string
std::string wx2std(const wxString& input, wxMBConv* conv)
{
    if (input.empty())
        return "";
    if (!conv)
        conv = wxConvCurrent;
    const wxWX2MBbuf buf(input.mb_str(*conv));
    // conversion may fail and return 0, which isn't a safe value to pass 
    // to std:string constructor
    if (!buf)
        return "";
    return std::string(buf);
}

//! converts std:string to wxString
wxString std2wx(const std::string& input, wxMBConv* conv)
{
    if (input.empty())
        return wxEmptyString;
    if (!conv)
        conv = wxConvCurrent;
    return wxString(input.c_str(), *conv);
}
