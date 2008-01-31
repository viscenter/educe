//
// testGL.cpp
//
// testGL will create a GL windows which displays a colored
// triangle.  Hit escape once you see this window.  testGL
// will then print out the version of GL installed on this computer.
//
// Author: J. Davison de St. Germain
// Date:   Nov 7, 2007
//
// Copyright (c) 2007 - University of Utah
// SCI Institute
//

#include <windows.h>

#include <stdio.h>

#include <string>

#include "glew/include/GL/glew.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////

bool beVerbose = false;

//////////////////////////////////////////////////////////////////////////

void
printPFD( PIXELFORMATDESCRIPTOR & pfd )
{
  printf( "nSize = %d\n", pfd.nSize );
  printf( "nVersion = %d\n", pfd.nVersion );
  printf( "dwFlags = %d\n", pfd.dwFlags );
  printf( "iPixelType = %d\n", pfd.iPixelType );
  printf( "cColorBits = %d\n", pfd.cColorBits );
  printf( "cRedBits = %d\n", pfd.cRedBits );
  printf( "cRedShift = %d\n", pfd.cRedShift );
  printf( "cGreenBits = %d\n", pfd.cGreenBits );
  printf( "cGreenShift = %d\n", pfd.cGreenShift );
  printf( "cBlueBits = %d\n", pfd.cBlueBits );
  printf( "cBlueShift = %d\n", pfd.cBlueShift );
  printf( "cAlphaBits = %d\n", pfd.cAlphaBits );
  printf( "cAlphaShift = %d\n", pfd.cAlphaShift );
  printf( "cAccumBits = %d\n", pfd.cAccumBits );
  printf( "cAccumRedBits = %d\n", pfd.cAccumRedBits );
  printf( "cAccumGreenBits = %d\n", pfd.cAccumGreenBits );
  printf( "cAccumBlueBits = %d\n", pfd.cAccumBlueBits );
  printf( "cAccumAlphaBits = %d\n", pfd.cAccumAlphaBits );
  printf( "cDepthBits = %d\n", pfd.cDepthBits );
  printf( "cStencilBits = %d\n", pfd.cStencilBits );
  printf( "cAuxBuffers = %d\n", pfd.cAuxBuffers );
  printf( "iLayerType = %d\n", pfd.iLayerType );
  printf( "bReserved = %d\n", pfd.bReserved );
  printf( "dwLayerMask = %d\n", pfd.dwLayerMask );
  printf( "dwVisibleMask = %d\n", pfd.dwVisibleMask );
  printf( "dwDamageMask = %d\n", pfd.dwDamageMask );
}

HGLRC
create_context( HDC hdc )
{
  // describe and set the pixel format
  DWORD dwFlags = PFD_DRAW_TO_WINDOW | PFD_STEREO | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_GENERIC_ACCELERATED;

  PIXELFORMATDESCRIPTOR pfd1 = { 
    sizeof(PIXELFORMATDESCRIPTOR),  
    1,                     // version number 
    dwFlags,
    PFD_TYPE_RGBA,         // RGBA type 
    24, // color depth
//    8, 0, 8, 0, 8, 0,  // color bits  
    8, 0, 8, 0, 8, 0,  // color bits  
//    8, 0,  // alpha buffer 
    0, 0,  // alpha buffer                     // NOTE, if alpha buffer is set as the line above (8,0)
    0+0+0,// accumulation buffer               //       GL may (depends on your driver/laptop/etc?) error out
    0, 0, 0, 0,// accum bits                   //       or default back to GL 1.1
    32,  // 32-bit z-buffer 
    0,// no stencil buffer 
    0, // no auxiliary buffer 
    PFD_MAIN_PLANE,        // main layer 
    0,                     // reserved 
    0, 0, 0                // layer masks ignored 
  }; 
  if( beVerbose ) {
    printPFD( pfd1 );
  }

  PIXELFORMATDESCRIPTOR pfd;

  memset(&pfd, 0, sizeof(pfd));
  pfd.nSize        = sizeof(pfd);
  pfd.nVersion     = 1;
  pfd.dwFlags      = PFD_STEREO | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_GENERIC_ACCELERATED; 
  pfd.iPixelType   = PFD_TYPE_RGBA;
  pfd.cColorBits   = 32;

  if( beVerbose ) {
    printPFD( pfd );
  }

  int iPixelFormat = ChoosePixelFormat(hdc, &pfd1);
  if (iPixelFormat == 0) {
    printf( "error with choosepixelformat\n" );
  }
  
  if (!SetPixelFormat(hdc, iPixelFormat, &pfd1)) {
     printf( "error with setpixelformat\n" );
  }

  /* Get the actual pixel format */
  if (!DescribePixelFormat(hdc, iPixelFormat, sizeof(pfd1), &pfd1)) {
    printf( "error with DescribePixelFormat" );
  }

  HGLRC context;
  if ((context = wglCreateContext(hdc)) == 0) {
    printf( "error with wglCreateContext\n" );
  }

  static HGLRC first_context = NULL;
  if( (first_context = wglCreateContext(hdc)) == NULL) {
      printf( "error with wglCreateContext (first)\n" );
  } //else {
  //    printf("context has been created\n");
  //  }

  if (wglShareLists(first_context,context) == FALSE) {
    printf( "error with wglShareLists\n" );
  }

  wglMakeCurrent( hdc, context );

  return context;
}

void
printErrorString( DWORD err )
{
  LPTSTR s;
  if( FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                     NULL,  err,  0,  (LPTSTR)&s,  0,
                     NULL) == 0) {
    printf( "FormatMessage failed\n" );
  }
  else {
    printf( "error is %s\n", s );
  }
}

char    * GlClassName = "GL Test Window";
char      appname[20] = "davsapp";

#define CHECK_OPENGL_ERROR()                                             \
   {                                                                     \
    GLenum errCode;                                                      \
    errCode = glGetError();                                              \
    if( errCode != GL_NO_ERROR ) {                                       \
      printf( "OpenGL Error(%s - %d) %s\n", __FILE__, __LINE__, (const char*)gluErrorString(errCode) ); \
    }                                                                    \
  }

void
display( HWND hWnd )
{
  /* rotate a triangle around */
  glClear(GL_COLOR_BUFFER_BIT);

  CHECK_OPENGL_ERROR();
  glBegin(GL_TRIANGLES);
  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex2i(0,  1);
  glColor3f(0.0f, 1.0f, 0.0f);
  glVertex2i(-1, -1);
  glColor3f(0.0f, 0.0f, 1.0f);
  glVertex2i(1, -1);
  glEnd();
  glFlush();

  HDC hDC = GetDC(hWnd);
  SwapBuffers(hDC);
}


LONG WINAPI
WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
  static PAINTSTRUCT ps;


  switch(uMsg) {
  case WM_PAINT:
    display( hWnd );
    BeginPaint(hWnd, &ps);
    EndPaint(hWnd, &ps);
    return 0;

  case WM_SIZE:
    glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
    PostMessage(hWnd, WM_PAINT, 0, 0);
    return 0;

  case WM_CHAR:
    switch (wParam) {
    case 27:/* ESC key */
      PostQuitMessage(0);
      break;
    }
    return 0;

  case WM_CLOSE:
    PostQuitMessage(0);
    return 0;
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam); 
} 

HWND
CreateOpenGLWindow(char* title, int x, int y, int width, int height, 
                   BYTE type, DWORD flags)
{
  int         pf;
  HWND        hWnd;
  WNDCLASS    wc;
  HDC         hDC;

  //  PIXELFORMATDESCRIPTOR pfd;
  static HINSTANCE hInstance = 0;

  /* only register the window class once - use hInstance as a flag. */
  if (!hInstance) {
    hInstance = GetModuleHandle(NULL);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    //wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = (WNDPROC)WindowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(long);
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "OpenGL";

    if (!RegisterClass(&wc)) {
      MessageBox(NULL, "RegisterClass() failed:  "
                 "Cannot register window class.", "Error", MB_OK);
      return NULL;
    }
  }

  hWnd = CreateWindow("OpenGL", title, 0 /*WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN*/,
                      x, y, width, height, NULL, NULL, hInstance, NULL);

  if (hWnd == NULL) {
    MessageBox(NULL, "CreateWindow() failed:  Cannot create a window.",
               "Error", MB_OK);
    return NULL;
  }

  hDC = GetDC(hWnd);

  HGLRC context = create_context( hDC );

  ReleaseDC(hWnd, hDC);

  return hWnd;
}  // end CreateOpenGLWindow()

void
usage()
{
  printf( "\n" );
  printf( "Usage: testGL [options]\n" );
  printf( "\n" );
  printf( "   A window containing a rainbox triangle should appear.  Press escape\n" );
  printf( "   to kill the window, and the version of GL will be printed to the screen.\n" );
  printf( "\n" );
  printf( "   Options:\n" );
  printf( "\n" );
  printf( "       --v[erbose] - Be verbose.\n" );
  printf( "\n" );
  exit( 1 );
}

void
parse_args( int argc, char *argv[] )
{
  for( int cnt = 1; cnt < argc; cnt++ ) {
    string arg = argv[cnt];
    if( arg == "--verbose" || arg == "--v" ) {
      beVerbose = true;
    }
    else {
      usage();
    }
  }
}

int
main( int argc, char *argv[] )
{
  parse_args( argc, argv );

  printf( "\n" );
  printf( "Using GLEW %s\n", glewGetString(GLEW_VERSION) );
  HWND win = CreateOpenGLWindow("minimal", 0,400, 256, 256, PFD_TYPE_RGBA, 0);

  ShowWindow(win, SW_SHOW);
  UpdateWindow(win);

  glewInit();

  printf( "\n" );
  printf( "GL version is %s\n",  glGetString(GL_VERSION) );
  printf( "\n" );

  MSG msg;
  while(GetMessage(&msg, win, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;

} // end main()

