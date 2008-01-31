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
 * GeomGlyph.cc: Glyph objects
 *
 *  Written by:
 *   Allen R. Sanderson
 *   Scientific Computing & Imaging Inst
 *   University of Utah
 *   March 2007
 *
 *  Copyright (C) 2007 SCI Group
 */

#include <Core/Geom/GeomGlyph.h>

namespace SCIRun {

using namespace std;

// GeomGlyph
Persistent* make_GeomGlyph()
{
  return scinew GeomGlyph;
}

PersistentTypeID GeomGlyph::type_id("GeomGlyph", "GeomObj", make_GeomGlyph);

GeomGlyph::GeomGlyph() : GeomFastQuads()
{
}

GeomGlyph::GeomGlyph(const GeomGlyph& copy)
: GeomFastQuads(copy)
{
}

GeomGlyph::~GeomGlyph()
{
}

GeomObj* GeomGlyph::clone()
{
  return scinew GeomGlyph(*this);
}


// GeomTranspGlyph
Persistent* make_GeomTranspGlyph()
{
  return scinew GeomTranspGlyph;
}

PersistentTypeID GeomTranspGlyph::type_id("GeomTranspGlyph", "GeomObj", make_GeomTranspGlyph);

GeomTranspGlyph::GeomTranspGlyph() : GeomTranspQuads()
{
}

GeomTranspGlyph::GeomTranspGlyph(const GeomTranspGlyph& copy)
: GeomTranspQuads(copy)
{
}

GeomTranspGlyph::~GeomTranspGlyph()
{
}

GeomObj* GeomTranspGlyph::clone()
{
  return scinew GeomTranspGlyph(*this);
}


// GeomGlyphBase
void GeomGlyphBase::gen_transforms( const Point& center,
				    const Vector& normal,
				    Transform& trans,
				    Transform& rotate )
{
  Vector axis = normal;

  axis.normalize();
  
  Vector z(0,0,1), zrotaxis;

  if(Abs(axis.x())+Abs(axis.y()) < 1.e-5) {
    // Only in x-z plane...
    zrotaxis = Vector(0,-1,0);
  }
  else
  {
    zrotaxis = Cross(axis, z);
    zrotaxis.normalize();
  }

  double cangle = Dot(z, axis);
  double zrotangle = -Acos(cangle);

  trans.pre_translate((Vector) center);
  trans.post_rotate(zrotangle, zrotaxis);

  rotate.post_rotate(zrotangle, zrotaxis);
}


void GeomGlyphBase::gen_transforms( const Point& center,
				    const Transform& transform,
				    Transform& trans,
				    Transform& rotate )
{
  trans = transform;
  rotate = transform;

  trans.pre_translate((Vector) center);
}


// Sphere
void GeomGlyphBase::add_sphere(const Point& center, double radius,
			      int nu, int nv, int half)
{
  vector< QuadStrip > quadstrips;

  gen_ellipsoid(center, Vector(0,0,1), radius, nu, nv, half, quadstrips);

  add_strip( quadstrips );
}

void GeomGlyphBase::add_sphere(const Point& center, double radius,
			      const MaterialHandle &mat,
			      int nu, int nv, int half)
{
  vector< QuadStrip > quadstrips;

  gen_ellipsoid(center, Vector(0,0,1), radius, nu, nv, half, quadstrips);

  add_strip( quadstrips, mat );
}


void GeomGlyphBase::add_sphere(const Point& center, double radius,
			      const double cindex,
			      int nu, int nv, int half)
{
  vector< QuadStrip > quadstrips;

  gen_ellipsoid(center, Vector(0,0,1), radius, nu, nv, half, quadstrips);

  add_strip( quadstrips, cindex );
}

void GeomGlyphBase::add_sphere(const Point& center, double radius,
			      const MaterialHandle &mat,
			      const double cindex,
			      int nu, int nv, int half)
{
  vector< QuadStrip > quadstrips;

  gen_ellipsoid(center, Vector(0,0,1), radius, nu, nv, half, quadstrips);

  add_strip( quadstrips, mat, cindex );
}

} // End namespace SCIRun




