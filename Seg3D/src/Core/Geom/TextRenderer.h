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
 *  TextRenderer.h
 *
 *  Written by:
 *   McKay Davis
 *   School of Computing
 *   University of Utah
 *   January, 2006
 *
 *  Copyright (C) 2006 SCI Group
 */

#ifndef SCIRun_Dataflow_Modules_Render_TextRenderer_h
#define SCIRun_Dataflow_Modules_Render_TextRenderer_h

#include <Core/Geom/FreeType.h>
#include <Core/Geom/View.h>
#include <Core/Geometry/BBox.h>
#include <string>
#include <map>
#include <set>

using namespace std;

#include <Core/Geom/share.h>
namespace SCIRun {

class TextureObj;
class SCISHARE TextRenderer {
public:
  TextRenderer(FreeTypeFace *face);
  ~TextRenderer();

  enum flags_e { N  = 1,
                 E  = 2, 
                 S  = 3,
                 W  = 4,
                 NE = 5,
                 SE = 6,
                 SW = 7,
                 NW = 8,
                 C  = 9,
                 ANCHOR_MASK = 15, 
                 VERTICAL = 16,
                 SHADOW = 32,
                 REVERSE = 64,
                 EXTRUDED = 128,
                 CURSOR = 256};

  int                   width(const string &text, int flags = 0);
  int                   height(const string &text, int flags = 0);

  void                  set_color(float, float, float, float);
  void                  set_color(float color[4]);

  void                  set_shadow_color(float, float, float, float);
  void                  set_shadow_color(float color[4]);

  void                  set_shadow_offset(int x, int y);

  void                  set_default_flags(int);

  void			render(const string &,
                               float x, float y, 
                               int flags);

  // rendering in a 3d scene.
  void			render(const string &,
                               float x, float y, float z, 
                               const View &view, double sf, int flags);

  double                find_scale_factor(const vector<Point> &p, 
					  const View &v) const;
  
  void                  set_cursor_position(unsigned int pos);
  
private:
  // Info for glyphs rendered to texture
  struct GlyphInfo {
#ifdef HAVE_FREETYPE
    FT_Glyph_Metrics    ft_metrics_;
    FT_UInt             index_;
    TextureObj *        texture_;
#endif
    float               tex_coords_[8];
  };

  typedef map<wchar_t, GlyphInfo *> GlyphMap;
  
  // Info for rendering glyphs from texture to screen
  struct GlyphLayoutInfo {
    GlyphInfo *         glyph_info_;
    Point               vertices_[4];
    float               color_[4];
  };

  typedef vector<GlyphLayoutInfo*> GlyphLayoutVector;


  struct TextLayoutInfo {
    string              text_;
    GlyphLayoutVector   layout_vector_;
    BBox                bbox_;
    Vector              offset_;
    Point               anchor_;
    Vector              left_;
    Vector              up_;
    int                 flags_;
    unsigned int        total_glyphs_;    
  };

  
  FreeTypeFace *	face_;
  GlyphMap              glyphs_;

  //  TextLayoutInfo        layout_;
  map<string, map<int, TextLayoutInfo> > layouts_;

  //  LayoutVector          layout_;
  float                 color_[4];
  float                 shadow_color_[4];
  pair<int, int>        shadow_offset_;  
  unsigned int          cursor_position_;

  // Textures of all glyphs
  typedef set<TextureObj *> TextureSet;
  TextureSet            textures_;

  // Texture to render next glyph to
  TextureObj *          texture_;

  // Info on where to render next glyph
  int                   x_;
  int                   y_;
  int                   height_;

  GlyphInfo *           render_glyph_to_texture(const wchar_t &);
  void                  render_string_glyphs_to_texture(const string &);
  void                  layout_text(TextLayoutInfo &,
				    bool clamp_vals = false);
  void                  move_text_to_anchor(TextLayoutInfo &, int);

};
  
}


#endif
