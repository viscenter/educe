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
 *  GeomQuads.cc: Fast Quads object
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   May 2003
 *
 *  Copyright (C) 2003 SCI Group
 */

#include <Core/Geom/GeomQuads.h>
#include <Core/Util/NotFinished.h>
#include <Core/Geom/GeomSave.h>
#include <Core/Geom/GeomTri.h>
#include <Core/Geometry/BBox.h>
#include <Core/Malloc/Allocator.h>
#include <Core/Persistent/PersistentSTL.h>
#include <iostream>
#include <functional>
#include <algorithm>

#include <stdio.h>

namespace SCIRun {

using namespace std;

static bool
pair_less(const pair<double, unsigned int> &a,
	  const pair<double, unsigned int> &b)
{
  return a.first < b.first;
}


Persistent* make_GeomTranspQuads()
{
    return scinew GeomTranspQuads;
}

PersistentTypeID GeomTranspQuads::type_id("GeomTranspQuads", "GeomFastQuads", make_GeomTranspQuads);


Persistent* make_GeomFastQuads()
{
    return scinew GeomFastQuads;
}

PersistentTypeID GeomFastQuads::type_id("GeomFastQuads", "GeomObj", make_GeomFastQuads);


GeomFastQuads::GeomFastQuads()
  : material_(0)
{
}

GeomFastQuads::GeomFastQuads(const GeomFastQuads& copy)
  : points_(copy.points_),
    colors_(copy.colors_),
    normals_(copy.normals_),
    material_(0)
{
}

GeomFastQuads::~GeomFastQuads()
{
}


GeomObj*
GeomFastQuads::clone()
{
  return scinew GeomFastQuads(*this);
}


int
GeomFastQuads::size()
{
  return points_.size() / 12;
}


void
GeomFastQuads::get_bounds(BBox& bb)
{
  for(unsigned int i=0;i<points_.size();i+=3)
  {
    bb.extend(Point(points_[i+0], points_[i+1], points_[i+2]));
  }
}


static unsigned char
COLOR_FTOB(double v)
{
  const int inter = (int)(v * 255 + 0.5);
  if (inter > 255) return 255;
  if (inter < 0) return 0;
  return (unsigned char)inter;
}


void
GeomFastQuads::add(const Point &p0, const Point &p1,
		   const Point &p2, const Point &p3)
{
  // Assume planar, use first three to compute normal.
  Vector n(Cross(p1-p0, p2-p0));
  if (n.length2() < (p1-p0).length2()*1e-5) n = Cross(p2-p1,p3-p1);
  add(p0, n, p1, n, p2, n, p3, n);
}


void
GeomFastQuads::add(const Point &p0, const MaterialHandle &m0,
		   const Point &p1, const MaterialHandle &m1,
		   const Point &p2, const MaterialHandle &m2,
		   const Point &p3, const MaterialHandle &m3)
{
  // Assume planar, use first three to compute normal.
  Vector n(Cross(p1-p0, p2-p0));
  if (n.length2() < (p1-p0).length2()*1e-5) n = Cross(p2-p1,p3-p1);
  add(p0, n, m0, p1, n, m1, p2, n, m2, p3, n, m3);
}


void
GeomFastQuads::add(const Point &p0, double i0,
		   const Point &p1, double i1,
		   const Point &p2, double i2,
		   const Point &p3, double i3)
{
  // Assume planar, use first three to compute normal.
  Vector n(Cross(p1-p0, p2-p0));
  if (n.length2() < (p1-p0).length2()*1e-5) n = Cross(p2-p1,p3-p1);
  add(p0, n, i0, p1, n, i1, p2, n, i2, p3, n, i3);
}


void
GeomFastQuads::add(const Point &p0, const MaterialHandle &m0, double i0,
		   const Point &p1, const MaterialHandle &m1, double i1,
		   const Point &p2, const MaterialHandle &m2, double i2,
		   const Point &p3, const MaterialHandle &m3, double i3)
{
  // Assume planar, use first three to compute normal.
  Vector n(Cross(p1-p0, p2-p0));
  if (n.length2() < (p1-p0).length2()*1e-5) n = Cross(p2-p1,p3-p1);
  add(p0, n, m0, i0, p1, n, m1, i1, p2, n, m2, i2, p3, n, m3, i3);
}


void
GeomFastQuads::add(const Point &p0, const Vector &n0,
		   const Point &p1, const Vector &n1,
		   const Point &p2, const Vector &n2,
		   const Point &p3, const Vector &n3)
{
  points_.push_back(p0.x());
  points_.push_back(p0.y());
  points_.push_back(p0.z());

  points_.push_back(p1.x());
  points_.push_back(p1.y());
  points_.push_back(p1.z());

  points_.push_back(p2.x());
  points_.push_back(p2.y());
  points_.push_back(p2.z());

  points_.push_back(p3.x());
  points_.push_back(p3.y());
  points_.push_back(p3.z());


  normals_.push_back(n0.x());
  normals_.push_back(n0.y());
  normals_.push_back(n0.z());

  normals_.push_back(n1.x());
  normals_.push_back(n1.y());
  normals_.push_back(n1.z());

  normals_.push_back(n2.x());
  normals_.push_back(n2.y());
  normals_.push_back(n2.z());

  normals_.push_back(n3.x());
  normals_.push_back(n3.y());
  normals_.push_back(n3.z());
}

void
GeomFastQuads::add(const Point &p0, const Vector &n0,
		   const MaterialHandle &m0,
		   const Point &p1, const Vector &n1,
		   const MaterialHandle &m1,
		   const Point &p2, const Vector &n2,
		   const MaterialHandle &m2,
		   const Point &p3, const Vector &n3,
		   const MaterialHandle &m3)
{
  add(p0, n0, p1, n1, p2, n2, p3, n3);

  colors_.push_back(COLOR_FTOB(m0->diffuse.r()));
  colors_.push_back(COLOR_FTOB(m0->diffuse.g()));
  colors_.push_back(COLOR_FTOB(m0->diffuse.b()));
  colors_.push_back(COLOR_FTOB(m0->transparency));

  colors_.push_back(COLOR_FTOB(m1->diffuse.r()));
  colors_.push_back(COLOR_FTOB(m1->diffuse.g()));
  colors_.push_back(COLOR_FTOB(m1->diffuse.b()));
  colors_.push_back(COLOR_FTOB(m1->transparency));

  colors_.push_back(COLOR_FTOB(m2->diffuse.r()));
  colors_.push_back(COLOR_FTOB(m2->diffuse.g()));
  colors_.push_back(COLOR_FTOB(m2->diffuse.b()));
  colors_.push_back(COLOR_FTOB(m2->transparency));

  colors_.push_back(COLOR_FTOB(m3->diffuse.r()));
  colors_.push_back(COLOR_FTOB(m3->diffuse.g()));
  colors_.push_back(COLOR_FTOB(m3->diffuse.b()));
  colors_.push_back(COLOR_FTOB(m3->transparency));

  material_ = m0;
}


void
GeomFastQuads::add(const Point &p0, const Vector &n0, double i0,
		   const Point &p1, const Vector &n1, double i1,
		   const Point &p2, const Vector &n2, double i2,
		   const Point &p3, const Vector &n3, double i3)
{
  add(p0, n0, p1, n1, p2, n2, p3, n3);

  indices_.push_back(i0);
  indices_.push_back(i1);
  indices_.push_back(i2);
  indices_.push_back(i3);
}


void
GeomFastQuads::add(const Point &p0, const Vector &n0,
		   const MaterialHandle &mat0, double i0,
		   const Point &p1, const Vector &n1,
		   const MaterialHandle &mat1, double i1,
		   const Point &p2, const Vector &n2,
		   const MaterialHandle &mat2, double i2,
		   const Point &p3, const Vector &n3,
		   const MaterialHandle &mat3, double i3)
{
  add(p0, n0, mat0, p1, n1, mat1, p2, n2, mat2, p3, n3, mat3);

  indices_.push_back(i0);
  indices_.push_back(i1);
  indices_.push_back(i2);
  indices_.push_back(i3);
}


void
GeomFastQuads::add( const vector< vector< pair<Point, Vector> > >&
		    quadstrips )
{
  for( unsigned int i=0; i<quadstrips.size(); i++ ) {

    const vector< pair<Point, Vector> >& quadstrip = quadstrips[i];

    for( unsigned int j=0; j<quadstrip.size()-2; j+=2 ) {

      add(quadstrip[j  ].first, quadstrip[j  ].second,
	  quadstrip[j+1].first, quadstrip[j+1].second,
	  quadstrip[j+3].first, quadstrip[j+3].second,
	  quadstrip[j+2].first, quadstrip[j+2].second);
    }
  }
}


void
GeomFastQuads::add( const vector< vector< pair<Point, Vector> > >&
		    quadstrips,
		    const MaterialHandle &mat )
{
  for( unsigned int i=0; i<quadstrips.size(); i++ ) {

    const vector< pair<Point, Vector> >& quadstrip = quadstrips[i];

    for( unsigned int j=0; j<quadstrip.size()-2; j+=2 ) {

      add( quadstrip[j  ].first, quadstrip[j  ].second, mat,
	   quadstrip[j+1].first, quadstrip[j+1].second, mat,
	   quadstrip[j+3].first, quadstrip[j+3].second, mat,
	   quadstrip[j+2].first, quadstrip[j+2].second, mat );
    }
  }
}


void
GeomFastQuads::add( const vector< vector< pair<Point, Vector> > >&
		    quadstrips,
		    const MaterialHandle &mat,
		    double cindex )
{
  for( unsigned int i=0; i<quadstrips.size(); i++ ) {

    const vector< pair<Point, Vector> >& quadstrip = quadstrips[i];

    for( unsigned int j=0; j<quadstrip.size()-2; j+=2 ) {

      add(quadstrip[j  ].first, quadstrip[j  ].second, mat, cindex,
	  quadstrip[j+1].first, quadstrip[j+1].second, mat, cindex,
	  quadstrip[j+3].first, quadstrip[j+3].second, mat, cindex,
	  quadstrip[j+2].first, quadstrip[j+2].second, mat, cindex);
    }
  }
}


void
GeomFastQuads::add( const vector< vector< pair<Point, Vector> > >&
		    quadstrips,
		    double cindex )
{
  for( unsigned int i=0; i<quadstrips.size(); i++ ) {

    const vector< pair<Point, Vector> >& quadstrip = quadstrips[i];

    for( unsigned int j=0; j<quadstrip.size()-2; j+=2 ) {

      add(quadstrip[j  ].first, quadstrip[j  ].second, cindex,
	  quadstrip[j+1].first, quadstrip[j+1].second, cindex,
	  quadstrip[j+3].first, quadstrip[j+3].second, cindex,
	  quadstrip[j+2].first, quadstrip[j+2].second, cindex);
    }
  }
}


#define GEOMFASTQUADS_VERSION 1

void GeomFastQuads::io(Piostream& stream)
{

    stream.begin_class("GeomFastQuads", GEOMFASTQUADS_VERSION);
    Pio(stream, points_);
    Pio(stream, colors_);
    Pio(stream, indices_);
    Pio(stream, normals_);
    stream.end_class();
}


GeomTranspQuads::GeomTranspQuads()
  : xreverse_(false),
    yreverse_(false),
    zreverse_(false)
{
}

GeomTranspQuads::GeomTranspQuads(const GeomTranspQuads& copy)
  : GeomFastQuads(copy),
    xlist_(copy.xlist_),
    ylist_(copy.ylist_),
    zlist_(copy.zlist_),
    xreverse_(copy.xreverse_),
    yreverse_(copy.yreverse_),
    zreverse_(copy.zreverse_)
{
}

GeomTranspQuads::~GeomTranspQuads()
{
}


GeomObj* GeomTranspQuads::clone()
{
    return scinew GeomTranspQuads(*this);
}

void
GeomTranspQuads::SortPolys()
{
  const unsigned int vsize = points_.size() / 12;
  if (xlist_.size() == vsize*4) return;

  xreverse_ = false;
  yreverse_ = false;
  zreverse_ = false;

  vector<pair<float, unsigned int> > tmp(vsize);
  unsigned int i;

  for (i = 0; i < vsize;i++)
  {
    tmp[i].first = points_[i*12+0] + points_[i*12+3] +
      points_[i*12+6] + points_[i*12+9];
    tmp[i].second = i*4;
  }
  std::sort(tmp.begin(), tmp.end(), pair_less);

  xlist_.resize(vsize*4);
  for (i=0; i < vsize; i++)
  {
    xlist_[i*4+0] = tmp[i].second + 0;
    xlist_[i*4+1] = tmp[i].second + 1;
    xlist_[i*4+2] = tmp[i].second + 2;
    xlist_[i*4+3] = tmp[i].second + 3;
  }

  for (i = 0; i < vsize;i++)
  {
    tmp[i].first = points_[i*12+1] + points_[i*12+4] +
      points_[i*12+7] + points_[i*12+10];
    tmp[i].second = i*4;
  }
  std::sort(tmp.begin(), tmp.end(), pair_less);

  ylist_.resize(vsize*4);
  for (i=0; i < vsize; i++)
  {
    ylist_[i*4+0] = tmp[i].second + 0;
    ylist_[i*4+1] = tmp[i].second + 1;
    ylist_[i*4+2] = tmp[i].second + 2;
    ylist_[i*4+3] = tmp[i].second + 3;
  }

  for (i = 0; i < vsize;i++)
  {
    tmp[i].first = points_[i*12+2] + points_[i*12+5] +
      points_[i*12+8] + points_[i*12+11];
    tmp[i].second = i*4;
  }
  std::sort(tmp.begin(), tmp.end(), pair_less);

  zlist_.resize(vsize*4);
  for (i=0; i < vsize; i++)
  {
    zlist_[i*4+0] = tmp[i].second + 0;
    zlist_[i*4+1] = tmp[i].second + 1;
    zlist_[i*4+2] = tmp[i].second + 2;
    zlist_[i*4+3] = tmp[i].second + 3;
  }
}


#define GEOMTRANSPQUADS_VERSION 1

void GeomTranspQuads::io(Piostream& stream)
{
  stream.begin_class("GeomTranspQuads", GEOMTRANSPQUADS_VERSION);
  GeomFastQuads::io(stream);
  stream.end_class();
}



Persistent* make_GeomFastQuadsTwoSided()
{
    return scinew GeomFastQuadsTwoSided;
}

PersistentTypeID GeomFastQuadsTwoSided::type_id("GeomFastQuadsTwoSided", "GeomObj", make_GeomFastQuadsTwoSided);


Persistent* make_GeomTranspQuadsTwoSided()
{
    return scinew GeomTranspQuadsTwoSided;
}

PersistentTypeID GeomTranspQuadsTwoSided::type_id("GeomTranspQuadsTwoSided", "GeomObj", make_GeomTranspQuadsTwoSided);


GeomFastQuadsTwoSided::GeomFastQuadsTwoSided()
  : material_(0), material2_(0)
{
}

GeomFastQuadsTwoSided::GeomFastQuadsTwoSided(const GeomFastQuadsTwoSided& copy)
  : points_(copy.points_),
    colors_(copy.colors_),
    colors2_(copy.colors2_),
    normals_(copy.normals_),
    material_(copy.material_),
    material2_(copy.material2_)
{
}

GeomFastQuadsTwoSided::~GeomFastQuadsTwoSided()
{
}


GeomObj*
GeomFastQuadsTwoSided::clone()
{
  return scinew GeomFastQuadsTwoSided(*this);
}


int
GeomFastQuadsTwoSided::size()
{
  return points_.size() / 12;
}


void
GeomFastQuadsTwoSided::get_bounds(BBox& bb)
{
  for(unsigned int i=0;i<points_.size();i+=3)
  {
    bb.extend(Point(points_[i+0], points_[i+1], points_[i+2]));
  }
}


void
GeomFastQuadsTwoSided::add(const Point &p0, const Point &p1,
			   const Point &p2, const Point &p3)
{
  // Assume planar, use first three to compute normal.
  Vector n(Cross(p1-p0, p2-p0));
  add(p0, n, p1, n, p2, n, p3, n);
}


void
GeomFastQuadsTwoSided::add(const Point &p0, const MaterialHandle &m0, const MaterialHandle &k0,
			   const Point &p1, const MaterialHandle &m1, const MaterialHandle &k1,
			   const Point &p2, const MaterialHandle &m2, const MaterialHandle &k2,
			   const Point &p3, const MaterialHandle &m3, const MaterialHandle &k3)
{
  // Assume planar, use first three to compute normal.
  Vector n(Cross(p1-p0, p2-p0));
  add(p0, n, m0, k0, p1, n, m1, k1, p2, n, m2, k2, p3, n, m3, k3);
}


void
GeomFastQuadsTwoSided::add(const Point &p0, double i0, double j0,
			   const Point &p1, double i1, double j1,
			   const Point &p2, double i2, double j2,
			   const Point &p3, double i3, double j3)
{
  // Assume planar, use first three to compute normal.
  Vector n(Cross(p1-p0, p2-p0));
  add(p0, n, i0, j0, p1, n, i1, j1, p2, n, i2, j2, p3, n, i3, j3);
}


void
GeomFastQuadsTwoSided::add(const Point &p0,
			   const MaterialHandle &m0, double i0,
			   const MaterialHandle &k0, double j0,
			   const Point &p1,
			   const MaterialHandle &m1, double i1,
			   const MaterialHandle &k1, double j1,
			   const Point &p2,
			   const MaterialHandle &m2, double i2,
			   const MaterialHandle &k2, double j2,
			   const Point &p3,
			   const MaterialHandle &m3, double i3,
			   const MaterialHandle &k3, double j3 )
{
  // Assume planar, use first three to compute normal.
  Vector n(Cross(p1-p0, p2-p0));
  add(p0, n, m0, i0, k0, j0, p1, n, m1, i1, k1, j1,
      p2, n, m2, i2, k2, j2, p3, n, m3, i3, k3, j3);
}


void
GeomFastQuadsTwoSided::add(const Point &p0, const Vector &n0,
                           const Point &p1, const Vector &n1,
                           const Point &p2, const Vector &n2,
                           const Point &p3, const Vector &n3)
{
  points_.push_back(p0.x());
  points_.push_back(p0.y());
  points_.push_back(p0.z());

  points_.push_back(p1.x());
  points_.push_back(p1.y());
  points_.push_back(p1.z());

  points_.push_back(p2.x());
  points_.push_back(p2.y());
  points_.push_back(p2.z());

  points_.push_back(p3.x());
  points_.push_back(p3.y());
  points_.push_back(p3.z());


  normals_.push_back(n0.x());
  normals_.push_back(n0.y());
  normals_.push_back(n0.z());

  normals_.push_back(n1.x());
  normals_.push_back(n1.y());
  normals_.push_back(n1.z());

  normals_.push_back(n2.x());
  normals_.push_back(n2.y());
  normals_.push_back(n2.z());

  normals_.push_back(n3.x());
  normals_.push_back(n3.y());
  normals_.push_back(n3.z());
}


void
GeomFastQuadsTwoSided::add(const Point &p0, const Vector &n0,
			   const MaterialHandle &m0, const MaterialHandle &k0,
			   const Point &p1, const Vector &n1,
			   const MaterialHandle &m1, const MaterialHandle &k1,
			   const Point &p2, const Vector &n2,
			   const MaterialHandle &m2, const MaterialHandle &k2,
			   const Point &p3, const Vector &n3,
			   const MaterialHandle &m3, const MaterialHandle &k3)
{
  add(p0, n0, p1, n1, p2, n2, p3, n3);

  colors_.push_back(COLOR_FTOB(m0->diffuse.r()));
  colors_.push_back(COLOR_FTOB(m0->diffuse.g()));
  colors_.push_back(COLOR_FTOB(m0->diffuse.b()));
  colors_.push_back(COLOR_FTOB(m0->transparency));

  colors_.push_back(COLOR_FTOB(m1->diffuse.r()));
  colors_.push_back(COLOR_FTOB(m1->diffuse.g()));
  colors_.push_back(COLOR_FTOB(m1->diffuse.b()));
  colors_.push_back(COLOR_FTOB(m1->transparency));

  colors_.push_back(COLOR_FTOB(m2->diffuse.r()));
  colors_.push_back(COLOR_FTOB(m2->diffuse.g()));
  colors_.push_back(COLOR_FTOB(m2->diffuse.b()));
  colors_.push_back(COLOR_FTOB(m2->transparency));

  colors_.push_back(COLOR_FTOB(m3->diffuse.r()));
  colors_.push_back(COLOR_FTOB(m3->diffuse.g()));
  colors_.push_back(COLOR_FTOB(m3->diffuse.b()));
  colors_.push_back(COLOR_FTOB(m3->transparency));

  colors2_.push_back(COLOR_FTOB(k0->diffuse.r()));
  colors2_.push_back(COLOR_FTOB(k0->diffuse.g()));
  colors2_.push_back(COLOR_FTOB(k0->diffuse.b()));
  colors2_.push_back(COLOR_FTOB(k0->transparency));

  colors2_.push_back(COLOR_FTOB(k1->diffuse.r()));
  colors2_.push_back(COLOR_FTOB(k1->diffuse.g()));
  colors2_.push_back(COLOR_FTOB(k1->diffuse.b()));
  colors2_.push_back(COLOR_FTOB(k1->transparency));

  colors2_.push_back(COLOR_FTOB(k2->diffuse.r()));
  colors2_.push_back(COLOR_FTOB(k2->diffuse.g()));
  colors2_.push_back(COLOR_FTOB(k2->diffuse.b()));
  colors2_.push_back(COLOR_FTOB(k2->transparency));

  colors2_.push_back(COLOR_FTOB(k3->diffuse.r()));
  colors2_.push_back(COLOR_FTOB(k3->diffuse.g()));
  colors2_.push_back(COLOR_FTOB(k3->diffuse.b()));
  colors2_.push_back(COLOR_FTOB(k3->transparency));

  material_  = m0;
  material2_ = k0;
}



void
GeomFastQuadsTwoSided::add(const Point &p0, const Vector &n0, double i0, double j0,
			   const Point &p1, const Vector &n1, double i1, double j1,
			   const Point &p2, const Vector &n2, double i2, double j2,
			   const Point &p3, const Vector &n3, double i3, double j3)
{
  add(p0, n0, p1, n1, p2, n2, p3, n3);

  indices_.push_back(i0);
  indices_.push_back(i1);
  indices_.push_back(i2);
  indices_.push_back(i3);

  indices2_.push_back(j0);
  indices2_.push_back(j1);
  indices2_.push_back(j2);
  indices2_.push_back(j3);
}

void
GeomFastQuadsTwoSided::add(const Point &p0, const Vector &n0,
			   const MaterialHandle &m0, double i0,
			   const MaterialHandle &k0, double j0,
			   const Point &p1, const Vector &n1,
			   const MaterialHandle &m1, double i1,
			   const MaterialHandle &k1, double j1,
			   const Point &p2, const Vector &n2,
			   const MaterialHandle &m2, double i2,
			   const MaterialHandle &k2, double j2,
			   const Point &p3, const Vector &n3,
			   const MaterialHandle &m3, double i3,
			   const MaterialHandle &k3, double j3)
{
  add(p0, n0, m0, k0, p1, n1, m1, k1, p2, n2, m2, k2, p3, n3, m3, k3);

  indices_.push_back(i0);
  indices_.push_back(i1);
  indices_.push_back(i2);
  indices_.push_back(i3);

  indices2_.push_back(j0);
  indices2_.push_back(j1);
  indices2_.push_back(j2);
  indices2_.push_back(j3);
}

#define GEOMFASTQUADSTWOSIDED_VERSION 1

void GeomFastQuadsTwoSided::io(Piostream& stream)
{
    stream.begin_class("GeomFastQuadsTwoSided", GEOMFASTQUADSTWOSIDED_VERSION);
    Pio(stream, points_);
    Pio(stream, colors_);
    Pio(stream, colors2_);
    Pio(stream, indices_);
    Pio(stream, indices2_);
    Pio(stream, normals_);
    stream.end_class();
}



GeomTranspQuadsTwoSided::GeomTranspQuadsTwoSided()
  : xreverse_(false),
    yreverse_(false),
    zreverse_(false)
{
}

GeomTranspQuadsTwoSided::GeomTranspQuadsTwoSided(const GeomTranspQuadsTwoSided& copy)
  : GeomFastQuadsTwoSided(copy),
    xlist_(copy.xlist_),
    ylist_(copy.ylist_),
    zlist_(copy.zlist_),
    xreverse_(copy.xreverse_),
    yreverse_(copy.yreverse_),
    zreverse_(copy.zreverse_)
{
}

GeomTranspQuadsTwoSided::~GeomTranspQuadsTwoSided()
{
}


GeomObj* GeomTranspQuadsTwoSided::clone()
{
    return scinew GeomTranspQuadsTwoSided(*this);
}

void
GeomTranspQuadsTwoSided::SortPolys()
{
  const unsigned int vsize = points_.size() / 12;
  if (xlist_.size() == vsize*4) return;

  xreverse_ = false;
  yreverse_ = false;
  zreverse_ = false;

  vector<pair<float, unsigned int> > tmp(vsize);
  unsigned int i;

  for (i = 0; i < vsize;i++)
  {
    tmp[i].first = points_[i*12+0] + points_[i*12+3] +
      points_[i*12+6] + points_[i*12+9];
    tmp[i].second = i*4;
  }
  std::sort(tmp.begin(), tmp.end(), pair_less);

  xlist_.resize(vsize*4);
  for (i=0; i < vsize; i++)
  {
    xlist_[i*4+0] = tmp[i].second + 0;
    xlist_[i*4+1] = tmp[i].second + 1;
    xlist_[i*4+2] = tmp[i].second + 2;
    xlist_[i*4+3] = tmp[i].second + 3;
  }

  for (i = 0; i < vsize;i++)
  {
    tmp[i].first = points_[i*12+1] + points_[i*12+4] +
      points_[i*12+7] + points_[i*12+10];
    tmp[i].second = i*4;
  }
  std::sort(tmp.begin(), tmp.end(), pair_less);

  ylist_.resize(vsize*4);
  for (i=0; i < vsize; i++)
  {
    ylist_[i*4+0] = tmp[i].second + 0;
    ylist_[i*4+1] = tmp[i].second + 1;
    ylist_[i*4+2] = tmp[i].second + 2;
    ylist_[i*4+3] = tmp[i].second + 3;
  }

  for (i = 0; i < vsize;i++)
  {
    tmp[i].first = points_[i*12+2] + points_[i*12+5] +
      points_[i*12+8] + points_[i*12+11];
    tmp[i].second = i*4;
  }
  std::sort(tmp.begin(), tmp.end(), pair_less);

  zlist_.resize(vsize*4);
  for (i=0; i < vsize; i++)
  {
    zlist_[i*4+0] = tmp[i].second + 0;
    zlist_[i*4+1] = tmp[i].second + 1;
    zlist_[i*4+2] = tmp[i].second + 2;
    zlist_[i*4+3] = tmp[i].second + 3;
  }
}

#define GEOMTRANSPQUADSTWOSIDED_VERSION 1

void GeomTranspQuadsTwoSided::io(Piostream& stream)
{
  stream.begin_class("GeomFastQuadsTwoSided", GEOMTRANSPQUADSTWOSIDED_VERSION);
  GeomFastQuadsTwoSided::io(stream);
  stream.end_class();
}

} // End namespace SCIRun

