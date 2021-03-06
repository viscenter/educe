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

#if !defined(SCI_GL_H)
#define SCI_GL_H

#include <sci_defs/ogl_defs.h>
// Uncomment this line to turn on verbose OPENGL ERROR checking.
//#define OGL_DBG 1
#include <slivr/gldefs.h>

#if defined(_WIN32)

/*
 * GLEW does not include <windows.h> to avoid name space pollution.
 * GL needs GLAPI and GLAPIENTRY, GLU needs APIENTRY, CALLBACK, and wchar_t
 * defined properly.
 */
/* <windef.h> */
#ifndef APIENTRY
#define GLEW_APIENTRY_DEFINED
#  if defined(__CYGWIN__) || defined(__MINGW32__)
#    define APIENTRY __stdcall
#  elif (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#    define APIENTRY __stdcall
#  else
#    define APIENTRY
#  endif
#endif

#ifndef GLAPI
#  if defined(__CYGWIN__) || defined(__MINGW32__)
#    define GLAPI extern
#  endif
#endif

/* <winnt.h> */
#ifndef CALLBACK
#define GLEW_CALLBACK_DEFINED
#  if defined(__CYGWIN__) || defined(__MINGW32__)
#    define CALLBACK __attribute__ ((__stdcall__))
#  elif (defined(_M_MRX000) || defined(_M_IX86) || defined(_M_ALPHA) || defined(_M_PPC)) && !defined(MIDL_PASS)
#    define CALLBACK __stdcall
#  else
#    define CALLBACK
#  endif
#endif

/* <wingdi.h> and <winnt.h> */
#ifndef WINGDIAPI
#  define GLEW_WINGDIAPI_DEFINED
#  define WINGDIAPI __declspec(dllimport)
#endif

/* <ctype.h> */
#if defined(_MSC_VER) && !defined(_WCHAR_T_DEFINED)
typedef unsigned short wchar_t;
#  define _WCHAR_T_DEFINED
#endif

/* <stddef.h> */
#if !defined(_W64)
#  if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#    define _W64 __w64
#  else
#    define _W64
#  endif
#endif

#ifndef GLAPI
#  if defined(__CYGWIN__) || defined(__MINGW32__)
#    define GLAPI extern
#  else
#    define GLAPI WINGDIAPI
#  endif
#endif

#ifndef GLAPIENTRY
#  define GLAPIENTRY APIENTRY
#endif
#undef SCISHARE
#define SCISHARE __declspec(dllimport)
#else // ! _WIN32
#define SCISHARE
#ifndef GLAPIENTRY
  #define GLAPIENTRY
#endif

#ifndef GLAPI
  #define GLAPI
#endif

#endif // _WIN32

#include <GL/glew.h>

#ifdef __cplusplus
  extern "C" {
#endif
  extern SCISHARE int sci_glew_init();
#ifdef __cplusplus
  }
#endif

#ifdef _WIN32
   typedef unsigned int uint;
#endif /* _WIN32 */

#endif  /* #define SCI_GL_H */
