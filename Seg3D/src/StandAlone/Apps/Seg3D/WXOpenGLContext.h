#ifndef CORE_EVENTS_WIN32EVENTSPAWNER_H
#define CORE_EVENTS_WIN32EVENTSPAWNER_H

#include <Core/Geom/OpenGLContext.h>

#include <sci_gl.h>

#include <wx/wx.h>
#include <wx/glcanvas.h>

#if !wxUSE_GLCANVAS
#error Need WX GL Canvas!
#endif

#include <StandAlone/Apps/Seg3D/share.h>

namespace SCIRun {

class WXOpenGLContext : public wxGLCanvas, public OpenGLContext
{
public:
  WXOpenGLContext(const string& target,
                  wxWindow *parent, wxWindowID id = wxID_ANY,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize, long style = 0,
                  const wxString& name = wxT("WXOpenGLContext"));

  ~WXOpenGLContext();

  // To make compliant with the existing OpenGLContext.
  virtual bool			make_current();
  virtual void			release();
  virtual int			width();
  virtual int			height();
  virtual void			swap();
  virtual void                  lock();
  virtual void                  unlock();

  // Called when Seg3D wants to shut the window down - it (currently)
  // only has access to the GL portion.
  void closeWindow();

protected:
  void OnPaint(wxPaintEvent& event);
  void OnSize(wxSizeEvent& event);
  void OnEraseBackground(wxEraseEvent& event);
  void OnMousePress(wxMouseEvent& event);
  void OnMouseRelease(wxMouseEvent& event);
  void OnMouseMove(wxMouseEvent& event);
  void OnMouseWheelMove(wxMouseEvent& event);
  void OnMouseEnter(wxMouseEvent& event);
  void OnMouseLeave(wxMouseEvent& event);
  void OnChar(wxKeyEvent &event);

  unsigned int ModifierKeyToSkinner(wxMouseEvent& event);
  unsigned int MouseDownToSkinner(wxMouseEvent& event, unsigned int mods);
  unsigned int MouseUpToSkinner(wxMouseEvent& event, unsigned int mods);
  unsigned int KeyToSkinner(int wxkeycode);

private:
  wxFrame* parentWindow;
  std::string target;
  DECLARE_EVENT_TABLE();
};


}
#endif
