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
 *  Freetype.h: Interface to Freetype 2 library
 *
 *  Written by:
 *   McKay Davis
 *   Department of Computer Science
 *   University of Utah
 *   October 2004
 *
 *  Copyright (C) 2004 Scientific Computing and Imaging Institute
 */

#ifndef SCIRun_src_Core_Geom_Freetype_h
#define SCIRun_src_Core_Geom_Freetype_h 1

#include <sci_defs/ogl_defs.h>  // For HAVE_FREETYPE

#ifdef HAVE_FREETYPE
#  include <ft2build.h>
#  include <freetype/freetype.h>
#  include <freetype/ftglyph.h>

#  if FREETYPE_MAJOR == 2
#    if (FREETYPE_MINOR == 0) || ((FREETYPE_MINOR == 1) && (FREETYPE_PATCH < 3))
#      define FT_KERNING_DEFAULT ft_kerning_default
#      define FT_RENDER_MODE_NORMAL ft_render_mode_normal
#      define FT_PIXEL_MODE_GRAY ft_pixel_mode_grays
#    endif
#  endif
#endif

#include <Core/Geometry/Point.h>
#include <Core/Geometry/BBox.h>
#include <Core/Geom/share.h>

#include <string>
#include <vector>

namespace SCIRun {

using std::string;
using std::vector;

class FreeTypeFace;

class SCISHARE FreeTypeLibrary {
public:
  FreeTypeLibrary();
  virtual ~FreeTypeLibrary();
  FreeTypeFace *	load_face(string);
#ifdef HAVE_FREETYPE
  FT_Library		library_;
#endif
};


class SCISHARE FreeTypeFace {
public:
  FreeTypeFace(FreeTypeLibrary *, string);
  virtual ~FreeTypeFace();
  bool			has_kerning_p();
  bool			scalable_p();
  void			set_dpi(unsigned int, unsigned int);
  void			set_points(double points);
  string		get_family_name();
  string		get_style_name();
  string		get_filename();
#ifdef HAVE_FREETYPE
  FT_Face		ft_face_;
#endif
  //private:
  double		points_;
  unsigned int		x_dpi_;
  unsigned int		y_dpi_;
  FreeTypeLibrary *	library_;
  string		filename_;
};


class SCISHARE FreeTypeGlyph {
public:
  FreeTypeGlyph();
  virtual ~FreeTypeGlyph();
#ifdef HAVE_FREETYPE
  FT_Glyph		glyph_;
  FT_UInt		index_;
  Point			position_;
  FT_Vector		ft_position();
#endif
};

typedef vector<FreeTypeGlyph *> FreeTypeGlyphs;

class SCISHARE FreeTypeText {
public:
  FreeTypeText(string text, FreeTypeFace *face, Point *pos = 0);
  virtual ~FreeTypeText();
  enum anchor_e { n, e, s, w, ne, se, sw, nw, c};
  
private:
  string		text_;
  FreeTypeFace *	face_;
  FreeTypeGlyphs	glyphs_;
  Point			position_;
  anchor_e		anchor_;
  
public:
  FreeTypeFace *	get_face();
  void			layout();
  void			render(int, int, unsigned char *);
  void			get_bounds(BBox &);
  void			set_anchor(anchor_e anchor) { anchor_ = anchor; }
  void			set_position(const Point &pt);
#ifdef HAVE_FREETYPE
  FT_Vector		ft_position(); 
#endif
};
  
 
 
} // End namespace SCIRun


#endif 

