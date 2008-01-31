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
 *  TkOpenGLContext.cc:
 *
 *  Written by:
 *   McKay Davis
 *   December 2004
 *
 */

#include <Core/Containers/StringUtil.h>
#include <Core/Datatypes/Color.h>
#include <Core/Exceptions/InternalError.h>
#include <Core/Malloc/Allocator.h>
#include <Core/Math/MiscMath.h> // for SWAP
#include <Core/Thread/Thread.h>
#include <Core/Util/Assert.h>

#include <Dataflow/GuiInterface/GuiInterface.h>
#include <Dataflow/GuiInterface/TCLInterface.h>
#include <Dataflow/GuiInterface/TclObj.h>
#include <Dataflow/GuiInterface/TCLTask.h>
#include <Dataflow/GuiInterface/TkOpenGLContext.h>

#include <sci_gl.h>
#include <sci_glx.h>

#include <iostream>
#include <set>

#ifdef _WIN32
#  include <Core/Geom/Win32OpenGLContext.h> // reuse the same 
#  include <tkWinInt.h>
#  include <tkIntPlatDecls.h>
#  include <windows.h>
#  include <sstream>
#  include <process.h>
#endif

using namespace SCIRun;
using namespace std;
  
extern "C" Tcl_Interp* the_interp;

vector<int> TkOpenGLContext::valid_visuals_ = vector<int>();

#if (TK_MAJOR_VERSION>=8 && TK_MINOR_VERSION>=4)
#  define HAVE_TK_SETCLASSPROCS
/* pointer to Tk_SetClassProcs function in the stub table */
#endif

#ifndef _WIN32
  static GLXContext first_context = NULL;
#else
  static HGLRC first_context = NULL;
#endif

#ifdef _WIN32
HINSTANCE dllHINSTANCE=0;

void
PrintErr(char* func_name)
{
  LPVOID lpMsgBuf;
  DWORD dw = GetLastError(); 
  
  if (dw) {
    FormatMessage(
		  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		  FORMAT_MESSAGE_FROM_SYSTEM,
		  NULL,
		  dw,
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  (LPTSTR) &lpMsgBuf,
		  0, NULL );
    
    fprintf(stderr, 
	    "%s failed with error %ld: %s", 
	    func_name, dw, (char*)lpMsgBuf); 
    LocalFree(lpMsgBuf);
  }
  SetLastError(0);
}

const char* TkOpenGLContext::ReportCapabilities()
{
  if (!this->hDC_)
    {
      return "no device context";
    }

  const char *glVendor = (const char *) glGetString(GL_VENDOR);
  const char *glRenderer = (const char *) glGetString(GL_RENDERER);
  const char *glVersion = (const char *) glGetString(GL_VERSION);
  const char *glExtensions = (const char *) glGetString(GL_EXTENSIONS);

  GLboolean stereo_enabled;

  glGetBooleanv(GL_STEREO, &stereo_enabled);

  ostringstream strm;
  if (glVendor) {
    strm << "OpenGL vendor string:  " << glVendor << endl;
    strm << "OpenGL renderer string:  " << glRenderer << endl;
    strm << "OpenGL version string:  " << glVersion << endl;
#if 0
    strm << "OpenGL extensions:  " << glExtensions << endl;
#endif
  }
  else {
    strm << "    Invalid OpenGL context\n";
  }

#if 0
  int pixelFormat = GetPixelFormat(this->hDC_);
  PIXELFORMATDESCRIPTOR pfd;

  DescribePixelFormat(this->hDC_, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);


  strm << "PixelFormat Descriptor:" << endl;
  strm << "depth:  " << static_cast<int>(pfd.cDepthBits) << endl;
  if (pfd.cColorBits <= 8)
    {
      strm << "class:  PseudoColor" << endl;
    } 
  else
    {
      strm << "class:  TrueColor" << endl;
    }
  strm << "buffer size:  " << static_cast<int>(pfd.cColorBits) << endl;
  strm << "level:  " << static_cast<int>(pfd.bReserved) << endl;
  if (pfd.iPixelType == PFD_TYPE_RGBA)
    {
    strm << "renderType:  rgba" << endl;
    }
  else
    {
    strm <<"renderType:  ci" << endl;
    }
  if (pfd.dwFlags & PFD_DOUBLEBUFFER) {
    strm << "double buffer:  True" << endl;
  } else {
    strm << "double buffer:  False" << endl;
  }
  if (pfd.dwFlags & PFD_STEREO) {
    strm << "stereo:  True" << endl;  
  } else {
    strm << "stereo:  False" << endl;
  }
  if (pfd.dwFlags & PFD_GENERIC_FORMAT) {
    strm << "hardware acceleration:  False" << endl; 
  } else {
    strm << "hardware acceleration:  True" << endl; 
  }
  strm << "rgba:  redSize=" << static_cast<int>(pfd.cRedBits) << " greenSize=" << static_cast<int>(pfd.cGreenBits) << "blueSize=" << static_cast<int>(pfd.cBlueBits) << "alphaSize=" << static_cast<int>(pfd.cAlphaBits) << endl;
  strm << "aux buffers:  " << static_cast<int>(pfd.cAuxBuffers)<< endl;
  strm << "depth size:  " << static_cast<int>(pfd.cDepthBits) << endl;
  strm << "stencil size:  " << static_cast<int>(pfd.cStencilBits) << endl;
  strm << "accum:  redSize=" << static_cast<int>(pfd.cAccumRedBits) << " greenSize=" << static_cast<int>(pfd.cAccumGreenBits) << "blueSize=" << static_cast<int>(pfd.cAccumBlueBits) << "alphaSize=" << static_cast<int>(pfd.cAccumAlphaBits) << endl;
#endif
  strm << " GL stereo: " << (stereo_enabled ? "enabled" : "disabled") << endl;
  strm << ends;
  
  cerr << strm.str().c_str();
  return 0;
}

char *tkGlClassName = "TkGL";
bool tkGlClassInitialized = false;
WNDCLASS tkGlClass;
WNDPROC tkWinChildProc=NULL;


// win32 event loop
LRESULT CALLBACK 
WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

  LONG result;
  TkOpenGLContext *tkCxt = (TkOpenGLContext*) GetWindowLong(hWnd, 0);
  WNDCLASS childClass;

  switch (msg) {
  case WM_WINDOWPOSCHANGED:
    /* Should be processed by DefWindowProc, otherwise a double buffered
       context is not properly resized when the corresponding window is resized.*/
    result = TRUE;
    break;
    
  case WM_NCCREATE:
    result = TRUE;
    break;

  case WM_DESTROY:
    result = TRUE;
#if 0
    // delete the tkCxt in OpenGL.cc destructor
    if (tkCxt)
      delete tkCxt;
#endif
    break;
    
  default:
    {
      if (tkWinChildProc == NULL) {
	GetClassInfo(Tk_GetHINSTANCE(),TK_WIN_CHILD_CLASS_NAME,
		     &childClass);
	tkWinChildProc = childClass.lpfnWndProc;
      }
      result = tkWinChildProc(hWnd, msg, wParam, lParam);
    }
  }
  Tcl_ServiceAll();
  return result;
}

static Window
TkGLMakeWindow(Tk_Window tkwin, Window parent, ClientData data)
{
  TkOpenGLContext *tkCxt = (TkOpenGLContext*) data;
  return tkCxt->tkglCallback(tkwin, parent);
}

Window
TkOpenGLContext::tkglCallback(Tk_Window tkwin, Window parent) 
{
  HWND parentWin;
  int style;
  HINSTANCE hInstance;

  vi_ = new XVisualInfo();
  display_ = Tk_Display(tkwin_);

  hInstance = Tk_GetHINSTANCE();

  // next register our own window class.... 
  
  if (!tkGlClassInitialized) {
    tkGlClassInitialized = true;
    tkGlClass.style = CS_HREDRAW | CS_VREDRAW;// | CS_OWNDC;
    tkGlClass.cbClsExtra = 0;
    tkGlClass.cbWndExtra = sizeof(long); /* To save TkCxt */
    tkGlClass.hInstance = dllHINSTANCE;
    tkGlClass.hbrBackground = NULL;
    tkGlClass.lpszMenuName = NULL;
    //tkGlClass.lpszClassName = TK_WIN_CHILD_CLASS_NAME;
    //tkGlClass.lpfnWndProc = TkWinChildProc;
    tkGlClass.lpszClassName = tkGlClassName;
    tkGlClass.lpfnWndProc = WndProc;
    tkGlClass.hIcon = NULL;
    tkGlClass.hCursor = NULL;

    if (RegisterClass(&tkGlClass) == 0)
      PrintErr("MakeWindow RegisterClass");
  }

  /*
   * Create the window, then ensure that it is at the top of the
   * stacking order.
   */

  int x = Tk_X(tkwin), y = Tk_Y(tkwin), width = Tk_Width(tkwin), height = Tk_Height(tkwin);
  if (width == 0 || height == 0) {
    style = WS_POPUP;
    parentWin = 0;
  }
  else {
    style = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    parentWin = Tk_GetHWND(parent);
  }

  hWND_ = CreateWindow(tkGlClassName, "SCIRun GL Viewer Screen",
			      style, x, y, width, height,
			      parentWin, NULL, dllHINSTANCE, 
			      NULL);
  PrintErr("CreateWindow");

  SetWindowLong(hWND_, 0, (LONG) this);

  if (width != 0 && height != 0)
    SetWindowPos(hWND_, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

  hDC_ = GetDC(hWND_);

  context_ = Win32OpenGLContext::create_context(hDC_);

  make_current();
  ReportCapabilities();

  if (!context_) throw scinew InternalError("Cannot create WGL Context", __FILE__, __LINE__);
  
  /* Just for portability, define the simplest visinfo */
  vi_->visual = DefaultVisual(display_, DefaultScreen(display_));   
  vi_->depth = vi_->visual->bits_per_rgb;
  /*
  * find a colormap
  */

  screen_number_ = Tk_ScreenNumber(tkwin_);

  colormap_ = DefaultColormap(display_, screen_number_);

  int result = Tk_SetWindowVisual(tkwin_, vi_->visual, 
				  vi_->depth,colormap_ );
  if (result != 1) throw scinew InternalError("Cannot set Tk Window Visual", 
					      __FILE__, __LINE__);

  SelectPalette(hDC_, ((TkWinColormap *)colormap_)->palette, TRUE);
  RealizePalette(hDC_);
  //visualid_ = iPixelFormat;

//   cerr << "after create window\n";
//   PrintErr("MakeWindow setwindowpos");
//   cerr << "after set window pos\n";
  Window win = Tk_AttachHWND(tkwin_, hWND_);
  XMapWindow(display_, win);
  return win;

}

#endif // _WIN32


TkOpenGLContext::TkOpenGLContext(const string &id, int visualid, 
				 int width, int height)
  : OpenGLContext()
{
#ifdef _WIN32
  make_win32_gl_context(id, visualid, width, height);
#else
  make_x11_gl_context(id, visualid, width, height);
#endif
}


void
TkOpenGLContext::make_x11_gl_context(const string &id, int visualid, 
                                     int width, int height)
{
#ifndef _WIN32
  visualid_ = visualid;
  id_ = id;

  mainwin_ = Tk_MainWindow(the_interp);
  if (!mainwin_) 
    throw scinew InternalError("Cannot find main Tk window",__FILE__,__LINE__);

  display_ = Tk_Display(mainwin_);
  if (!display_) 
    throw scinew InternalError("Cannot find X Display", __FILE__, __LINE__);

  screen_number_ = Tk_ScreenNumber(mainwin_);
  x11_win_  = 0;
  context_  = 0;
  vi_       = 0;

  if (valid_visuals_.empty())
    listvisuals();
  if (visualid < 0 || visualid >= (int)valid_visuals_.size())
    {
      cerr << "Bad visual id, does not exist.\n";
      visualid_ = 0;
    } else {
      visualid_ = valid_visuals_[visualid];
    }

  if (visualid_)
  {
    int n;
    XVisualInfo temp_vi;
    temp_vi.visualid = visualid_;
    vi_ = XGetVisualInfo(display_, VisualIDMask, &temp_vi, &n);
    if(!vi_ || n!=1) {
      throw 
        scinew InternalError("Cannot find Visual ID #"+to_string(visualid_), 
                             __FILE__, __LINE__);
    }
  }

  if (!vi_) 
    throw scinew InternalError("Cannot find Visual", __FILE__, __LINE__);

  colormap_ = XCreateColormap(display_, Tk_WindowId(mainwin_), 
			      vi_->visual, AllocNone);


  tkwin_ = Tk_CreateWindowFromPath(the_interp, mainwin_, 
				   ccast_unsafe(id),
				   (char *) NULL);

  if (!tkwin_) 
    throw scinew InternalError("Cannot create Tk Window", __FILE__, __LINE__);



  Tk_GeometryRequest(tkwin_, width, height);
  int result = Tk_SetWindowVisual(tkwin_, vi_->visual, vi_->depth, colormap_);
  if (result != 1) 
    throw scinew InternalError("Tk_SetWindowVisual failed",__FILE__,__LINE__);

  Tk_MakeWindowExist(tkwin_);
  if (Tk_WindowId(tkwin_) == 0) {
    throw InternalError("Tk_MakeWindowExist failed", __FILE__, __LINE__);
  }

  x11_win_ = Tk_WindowId(tkwin_);
  if (!x11_win_) 
    throw scinew InternalError("Cannot get Tk X11 window ID",__FILE__,__LINE__);

  XSync(display_, False);


  if (!first_context) {
    first_context = glXCreateContext(display_, vi_, 0, 1);
  }
  context_ = glXCreateContext(display_, vi_, first_context, 1);
  if (!context_) 
    throw scinew InternalError("Cannot create GLX Context",__FILE__,__LINE__);
#endif
}



void
TkOpenGLContext::make_win32_gl_context(const string &id, int visualid, 
                                       int width, int height)
{
#ifdef _WIN32
  visualid_ = visualid;
  id_ = id;

  mainwin_ = Tk_MainWindow(the_interp);
  if (!mainwin_) 
    throw scinew InternalError("Cannot find main Tk window",__FILE__,__LINE__);

  display_ = Tk_Display(mainwin_);
  if (!display_) 
    throw scinew InternalError("Cannot find X Display", __FILE__, __LINE__);

  screen_number_ = Tk_ScreenNumber(mainwin_);
  x11_win_  = 0;
  context_  = 0;
  vi_       = 0;

  if (valid_visuals_.empty())
    listvisuals();
  if (visualid < 0 || visualid >= (int)valid_visuals_.size())
    {
      visualid_ = 0;
    } else {
      visualid_ = valid_visuals_[visualid];
    }

  if (visualid_) {

    int n;
    XVisualInfo temp_vi;
    temp_vi.visualid = visualid_;
    vi_ = XGetVisualInfo(display_, VisualIDMask, &temp_vi, &n);
    if(!vi_ || n!=1) {
      throw 
        scinew InternalError("Cannot find Visual ID #"+to_string(visualid_), 
                             __FILE__, __LINE__);
    }
  }

  tkwin_ = Tk_CreateWindowFromPath(the_interp, mainwin_, 
				   ccast_unsafe(id),
				   (char *) NULL);

  if (!tkwin_) 
    throw scinew InternalError("Cannot create Tk Window", __FILE__, __LINE__);

#  ifdef HAVE_TK_SETCLASSPROCS
  Tk_ClassProcs *procsPtr;
  procsPtr = (Tk_ClassProcs*) Tcl_Alloc(sizeof(Tk_ClassProcs));
  procsPtr->size             = sizeof(Tk_ClassProcs);
#ifdef _WIN32
  procsPtr->createProc       = TkGLMakeWindow;
#endif
  procsPtr->worldChangedProc = NULL;
  procsPtr->modalProc        = NULL;
  Tk_SetClassProcs(tkwin_,procsPtr,(ClientData)this); 
#  else
//   TkClassProcs *procsPtr;
//   Tk_FakeWin *winPtr = (Tk_FakeWin*)(tkwin_);
    
//   procsPtr = (TkClassProcs*)Tcl_Alloc(sizeof(TkClassProcs));
//   procsPtr->createProc     = TkGLMakeWindow;
//   procsPtr->geometryProc   = NULL;
//   procsPtr->modalProc      = NULL;
//   winPtr->dummy17 = (char*)procsPtr;
//   winPtr->dummy18 = (ClientData)this;
#  endif
  Tk_GeometryRequest(tkwin_, width, height);
  Tk_ResizeWindow(tkwin_, width, height);

  Tk_MakeWindowExist(tkwin_);
  if (Tk_WindowId(tkwin_) == 0) {
    throw InternalError("Tk_MakeWindowExist failed", __FILE__, __LINE__);
  }

  x11_win_ = Tk_WindowId(tkwin_);
  if (!x11_win_) 
    throw scinew InternalError("Cannot get Tk X11 window ID", __FILE__, __LINE__);

  XSync(display_, False);

  release();
#endif
}



TkOpenGLContext::~TkOpenGLContext()
{
//   lock();
//   release();
// #ifdef _WIN32
//   if (context_ != first_context)
//     wglDeleteContext(context_);

//   // Remove the TkGL Context from the window so it won't delete it in
//   // the callback.
//   SetWindowLong(hWND_, 0, (LONG) 0);
//   DestroyWindow(hWND_);
// #else // X11
//   glXDestroyContext(display_, context_);
//   XSync(display_, False);
// #endif
//   unlock();
}

void
TkOpenGLContext::lock()
{
  TCLTask::lock();
}

void
TkOpenGLContext::unlock()
{
  TCLTask::unlock();
}

bool
TkOpenGLContext::make_current()
{
  ASSERT(context_);

  bool result = true;

#ifdef _WIN32
  HGLRC current = wglGetCurrentContext();

  if (current != context_) {
    result = wglMakeCurrent(hDC_,context_);
    if (!result)
      PrintErr("wglMakeCurrent");
  }
#else  // X11
  GuiInterface::getSingleton()->pause();
  result = glXMakeCurrent(display_, x11_win_, context_);
  GuiInterface::getSingleton()->unpause();
  if (!result)
  {
    std::cerr << "GL context: " << id_ << " failed make current.\n";
  }
#endif

  return result;
}


void
TkOpenGLContext::release()
{
#ifdef _WIN32
  if (wglGetCurrentContext() != NULL)
    wglMakeCurrent(NULL,NULL);
  PrintErr("TkOpenGLContext::release()");
#else // X11
  GuiInterface::getSingleton()->pause();
  glXMakeCurrent(display_, None, NULL);
  GuiInterface::getSingleton()->unpause();
#endif
}


int
TkOpenGLContext::width()
{
  return Tk_Width(tkwin_);
}


int
TkOpenGLContext::height()
{
  return Tk_Height(tkwin_);
}


void
TkOpenGLContext::swap()
{  
  // TODO:  Thread safe?
#ifdef _WIN32
  SwapBuffers(hDC_);
#else // X11
  glXSwapBuffers(display_, x11_win_);
#endif
}



#define GETCONFIG(attrib) \
if(glXGetConfig(display, &vinfo[i], attrib, &value) != 0){\
  cerr << "Error getting attribute: " << #attrib << std::endl; \
  TCLTask::unlock(); \
  return string(""); \
}


string
TkOpenGLContext::listvisuals()
{
#ifdef _WIN32
  valid_visuals_.clear();
  return string("");
#else // X11
  TCLTask::lock();
  Tk_Window topwin=Tk_MainWindow(the_interp);
  if(!topwin)
  {
    cerr << "Unable to locate main window!\n";
    TCLTask::unlock();
    return string("");
  }

  Display *display = Tk_Display(topwin);
  int screen = Tk_ScreenNumber(topwin);
  valid_visuals_.clear();
  vector<string> visualtags;
  vector<int> scores;
  int nvis;
  XVisualInfo* vinfo=XGetVisualInfo(display, 0, NULL, &nvis);
  if(!vinfo)
  {
    cerr << "XGetVisualInfo failed";
    TCLTask::unlock();
    return string("");
  }

  for (int i=0;i<nvis;i++)
  {
    int score=0;
    int value;
    GETCONFIG(GLX_USE_GL);
    if(!value)
      continue;
    GETCONFIG(GLX_RGBA);
    if(!value)
      continue;
    GETCONFIG(GLX_LEVEL);
    if(value != 0)
      continue;
    if(vinfo[i].screen != screen)
      continue;
    char buf[20];
    sprintf(buf, "id=%02x, ", (unsigned int)(vinfo[i].visualid));
    valid_visuals_.push_back(vinfo[i].visualid);
    string tag(buf);
    GETCONFIG(GLX_DOUBLEBUFFER);
    if (value)
    {
      score+=200;
      tag += "double, ";
    }
    else
    {
      tag += "single, ";
    }
    GETCONFIG(GLX_STEREO);
    if (value)
    {
      score+=1;
      tag += "stereo, ";
    }
    tag += "rgba=";
    GETCONFIG(GLX_RED_SIZE);
    tag+=to_string(value)+":";
    score+=value;
    GETCONFIG(GLX_GREEN_SIZE);
    tag+=to_string(value)+":";
    score+=value;
    GETCONFIG(GLX_BLUE_SIZE);
    tag+=to_string(value)+":";
    score+=value;
    GETCONFIG(GLX_ALPHA_SIZE);
    tag+=to_string(value);
    score+=value;
    GETCONFIG(GLX_DEPTH_SIZE);
    tag += ", depth=" + to_string(value);
    score+=value*5;
    GETCONFIG(GLX_STENCIL_SIZE);
    score += value * 2;
    tag += ", stencil="+to_string(value);
    tag += ", accum=";
    GETCONFIG(GLX_ACCUM_RED_SIZE);
    tag += to_string(value) + ":";
    GETCONFIG(GLX_ACCUM_GREEN_SIZE);
    tag += to_string(value) + ":";
    GETCONFIG(GLX_ACCUM_BLUE_SIZE);
    tag += to_string(value) + ":";
    GETCONFIG(GLX_ACCUM_ALPHA_SIZE);
    tag += to_string(value);
#ifdef __sgi
    tag += ", samples=";
    GETCONFIG(GLX_SAMPLES_SGIS);
    if(value)
      score+=50;
    tag += to_string(value);
#endif

    tag += ", score=" + to_string(score);
    
    visualtags.push_back(tag);
    scores.push_back(score);
  }
  for(int i=0; i < int(scores.size())-1; i++)
  {
    for(int j=i+1; j < int(scores.size()); j++)
    {
      if(scores[i] < scores[j])
      {
	SWAP(scores[i], scores[j]);
	SWAP(visualtags[i], visualtags[j]);
	SWAP(valid_visuals_[i], valid_visuals_[j]);
      }
    }
  }
  string ret_val;
  for (unsigned int k = 0; k < visualtags.size(); ++k) {
    ret_val = ret_val + "{" + visualtags[k] +"} ";
  }
  TCLTask::unlock();
  return ret_val;
#endif
}

#ifdef _WIN32
BOOL WINAPI DllMain(HINSTANCE hinstance, DWORD reason, LPVOID reserved)
{
  switch (reason) {
  case DLL_PROCESS_ATTACH:
    dllHINSTANCE = hinstance; break;
  default: break;
  }
  return TRUE;
}
#endif
