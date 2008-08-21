/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2004 Scientific Computing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

/*
 *  TkOpenGLContext.h:
 *
 *  Written by:
 *   McKay Davis
 *   December 2004
 */


#ifndef SCIRun_Core_2d_TkOpenGLContext_h
#define SCIRun_Core_2d_TkOpenGLContext_h

#include <sci_gl.h>
#include <sci_glx.h>
#include <stdio.h>
#include <tk.h>

#ifdef _WIN32
#  include <tkWinInt.h>
#  include <tkWinPort.h>
#  include <X11\XUtil.h>
#endif

#include <Core/Thread/Mutex.h>
#include <Core/Geom/OpenGLContext.h>

#include <Dataflow/GuiInterface/share.h>
namespace SCIRun {

class SCISHARE TkOpenGLContext : public OpenGLContext 
{
public:
  TkOpenGLContext(const string &, int visualid=0, 
		  int width=640, int height = 480);
  virtual ~TkOpenGLContext();

private:  
  void                  make_x11_gl_context(const string &, int, int, int);
  void                  make_win32_gl_context(const string &, int, int, int);
public:
#ifdef _WIN32
  Window tkglCallback(Tk_Window tkwin, Window parent);
#endif
  static string		        listvisuals();
  virtual bool			make_current();
  virtual void			release();
  virtual int       width();
  virtual int       height();
  virtual void			swap();
  virtual void			lock();
  virtual void			unlock();
  virtual bool      has_shaders();
  
private:
  #ifdef _WIN32
  const char*           ReportCapabilities();
  #endif

  static vector<int>	valid_visuals_;
  int			visualid_;
  int			screen_number_;
public:
  Display *		display_; /* X's token for the window's display. */
private:
  Window		x11_win_;
public:
  Tk_Window		tkwin_;
private:
  Tk_Window		mainwin_;
#ifndef _WIN32
  GLXContext		context_;
#else
  HDC                   hDC_;
  HGLRC                 context_;
  HWND                  hWND_;
#endif
  XVisualInfo*		vi_;
  Colormap		colormap_;
public:
  string		id_;
};

} // End namespace SCIRun

#endif // SCIRun_Core_2d_OpenGLContext_h
