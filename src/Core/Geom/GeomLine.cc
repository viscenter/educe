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
 *  GeomLine.cc:  Line object
 *
 *  Written by:
 *   Steven G. Parker & David Weinstein
 *   Department of Computer Science
 *   University of Utah
 *   April 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

#ifdef _WIN32
#pragma warning(disable:4291) // quiet the visual C++ compiler
#endif

#include <Core/Geom/GeomLine.h>
#include <Core/Util/NotFinished.h>
#include <Core/Geometry/BBox.h>
#include <Core/Persistent/PersistentSTL.h>
#include <iostream>
#include <algorithm>

#include <stdlib.h>

namespace SCIRun {

using std::cerr;
using std::ostream;


Persistent* make_GeomLine()
{
    return new GeomLine(Point(0,0,0), Point(1,1,1));
}

PersistentTypeID GeomLine::type_id("GeomLine", "GeomObj", make_GeomLine);

GeomLine::GeomLine(const Point& p1, const Point& p2) : 
  GeomObj(), 
  p1(p1), 
  p2(p2),
  line_width_(0.0)
{
}

GeomLine::GeomLine(const GeomLine& copy) : 
  GeomObj(), 
  p1(copy.p1), 
  p2(copy.p2),
  line_width_(0.0)
{
}

GeomLine::~GeomLine()
{
}

GeomObj* GeomLine::clone()
{    return new GeomLine(*this);
}

void GeomLine::get_bounds(BBox& bb)
{
  bb.extend(p1);
  bb.extend(p2);
}

void
GeomLine::setLineWidth(float val) 
{
  line_width_ = val;
}

#define GEOMLINE_VERSION 1

void GeomLine::io(Piostream& stream)
{

  stream.begin_class("GeomLine", GEOMLINE_VERSION);
  GeomObj::io(stream);
  Pio(stream, p1);
  Pio(stream, p2);
  stream.end_class();
}

Persistent* make_GeomLines()
{
  return new GeomLines();
}

PersistentTypeID GeomLines::type_id("GeomLines", "GeomObj", make_GeomLines);

GeomLines::GeomLines()
  : line_width_(0.0)
{
}

GeomLines::GeomLines(const GeomLines& copy)
  : line_width_(copy.line_width_),
    points_(copy.points_),
    colors_(copy.colors_),
    indices_(copy.indices_)
{
}

GeomLines::~GeomLines()
{
}

GeomObj* GeomLines::clone()
{
  return new GeomLines(*this);
}

void GeomLines::get_bounds(BBox& bb)
{
  for(unsigned int i=0;i<points_.size();i+=3)
  {
    bb.extend(Point(points_[i+0], points_[i+1], points_[i+2]));
  }
}

#define GEOMLINES_VERSION 1

void GeomLines::io(Piostream& stream)
{

  stream.begin_class("GeomLines", GEOMLINES_VERSION);
  GeomObj::io(stream);
  Pio(stream, line_width_);
  Pio(stream, points_);
  Pio(stream, colors_);
  Pio(stream, indices_);
  stream.end_class();
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
GeomLines::add(const Point& p1, const Point& p2)
{
  points_.push_back(p1.x());
  points_.push_back(p1.y());
  points_.push_back(p1.z());
  points_.push_back(p2.x());
  points_.push_back(p2.y());
  points_.push_back(p2.z());
}


void
GeomLines::add(const Point& p1, const MaterialHandle &c1,
	       const Point& p2, const MaterialHandle &c2)
{
  add(p1, p2);

  const unsigned char r0 = COLOR_FTOB(c1->diffuse.r());
  const unsigned char g0 = COLOR_FTOB(c1->diffuse.g());
  const unsigned char b0 = COLOR_FTOB(c1->diffuse.b());
  const unsigned char a0 = COLOR_FTOB(c1->transparency);

  colors_.push_back(r0);
  colors_.push_back(g0);
  colors_.push_back(b0);
  colors_.push_back(a0);

  const unsigned char r1 = COLOR_FTOB(c2->diffuse.r());
  const unsigned char g1 = COLOR_FTOB(c2->diffuse.g());
  const unsigned char b1 = COLOR_FTOB(c2->diffuse.b());
  const unsigned char a1 = COLOR_FTOB(c2->transparency);

  colors_.push_back(r1);
  colors_.push_back(g1);
  colors_.push_back(b1);
  colors_.push_back(a1);
}


void
GeomLines::add(const Point& p0, double cindex0,
	       const Point& p1, double cindex1)
{
  add(p0, p1);
  
  indices_.push_back(cindex0);
  indices_.push_back(cindex1);
}


void
GeomLines::add(const Point& p0, const MaterialHandle &c0, double cindex0,
	       const Point& p1, const MaterialHandle &c1, double cindex1)
{
  add(p0, c0, p1, c1);
  
  indices_.push_back(cindex0);
  indices_.push_back(cindex1);
}




Persistent* make_GeomTranspLines()
{
  return new GeomTranspLines();
}

PersistentTypeID GeomTranspLines::type_id("GeomTranspLines", "GeomLines",
					  make_GeomTranspLines);

GeomTranspLines::GeomTranspLines()
  : GeomLines(),
    xreverse_(false),
    yreverse_(false),
    zreverse_(false)
{
}

GeomTranspLines::GeomTranspLines(const GeomTranspLines& copy)
  : GeomLines(copy),
    xindices_(copy.xindices_),
    yindices_(copy.yindices_),
    zindices_(copy.zindices_),
    xreverse_(copy.xreverse_),
    yreverse_(copy.yreverse_),
    zreverse_(copy.zreverse_)
{
}

GeomTranspLines::~GeomTranspLines()
{
}

GeomObj* GeomTranspLines::clone()
{
  return new GeomTranspLines(*this);
}

#define GEOMLINES_VERSION 1

void GeomTranspLines::io(Piostream& stream)
{
  stream.begin_class("GeomTranspLines", GEOMLINES_VERSION);
  GeomLines::io(stream);
  stream.end_class();
}

static bool
pair_less(const pair<float, unsigned int> &a,
	  const pair<float, unsigned int> &b)
{
  return a.first < b.first;
}
 

void
GeomTranspLines::Sort()
{
  const unsigned int vsize = points_.size() / 6;
  if (xindices_.size() == vsize*2) return;
  
  xreverse_ = false;
  yreverse_ = false;
  zreverse_ = false;

  vector<pair<float, unsigned int> > tmp(vsize);
  unsigned int i;

  for (i = 0; i < vsize;i++)
  {
    tmp[i].first = points_[i*6+0] + points_[i*6+3];
    tmp[i].second = i*6;
  }
  std::sort(tmp.begin(), tmp.end(), pair_less);

  xindices_.resize(vsize*2);
  for (i=0; i < vsize; i++)
  {
    xindices_[i*2+0] = tmp[i].second / 3;
    xindices_[i*2+1] = tmp[i].second / 3 + 1;
  }

  for (i = 0; i < vsize;i++)
  {
    tmp[i].first = points_[i*6+1] + points_[i*6+4];
    tmp[i].second = i*6;
  }
  std::sort(tmp.begin(), tmp.end(), pair_less);

  yindices_.resize(vsize*2);
  for (i=0; i < vsize; i++)
  {
    yindices_[i*2+0] = tmp[i].second / 3;
    yindices_[i*2+1] = tmp[i].second / 3 + 1;
  }

  for (i = 0; i < vsize;i++)
  {
    tmp[i].first = points_[i*6+2] + points_[i*6+5];
    tmp[i].second = i*6;
  }
  std::sort(tmp.begin(), tmp.end(), pair_less);

  zindices_.resize(vsize*2);
  for (i=0; i < vsize; i++)
  {
    zindices_[i*2+0] = tmp[i].second / 3;
    zindices_[i*2+1] = tmp[i].second / 3 + 1;
  }

}


// for lit streamlines
Persistent* make_TexGeomLines()
{
  return new TexGeomLines();
}

PersistentTypeID TexGeomLines::type_id("TexGeomLines", "GeomObj", make_TexGeomLines);

TexGeomLines::TexGeomLines()
  : mutex_("TexGeomLines mutex"),
    alpha_(1.0),
    tmapid_(0),
    tex_per_seg_(1)
{
}

TexGeomLines::TexGeomLines(const TexGeomLines& copy)
  : mutex_("TexGeomLines mutex"), points_(copy.points_)
{
}

TexGeomLines::~TexGeomLines()
{
}

GeomObj* TexGeomLines::clone()
{
  return new TexGeomLines(*this);
}

void TexGeomLines::get_bounds(BBox& bb)
{
  for(int i=0; i<points_.size(); i++)
    bb.extend(points_[i]);
}

#define TexGeomLines_VERSION 1

void TexGeomLines::io(Piostream& stream)
{
  stream.begin_class("TexGeomLines", TexGeomLines_VERSION);
  GeomObj::io(stream);
  Pio(stream, points_);
  stream.end_class();
}

// this is used by the hedgehog...

void TexGeomLines::add(const Point& p1, const Point& p2,double scale)
{
  points_.add(p1);
  points_.add(p2);
  
  tangents_.add((p2-p1)*scale);
} 

void TexGeomLines::add(const Point& p1, const Vector& dir, const Colorub& c) {
  points_.add(p1);
  points_.add(p1+dir);

  Vector v(dir);
  v.normalize();
  tangents_.add(v);
  colors_.add(c);
}

// this is used by the streamline module...

void TexGeomLines::batch_add(Array1<double>&, Array1<Point>& ps)
{
  tex_per_seg_ = 0;  // this is not the hedgehog...
  int pstart = points_.size();
  int tstart = tangents_.size();

  points_.grow(2*(ps.size()-1));
  tangents_.grow(2*(ps.size()-1));  // ignore times for now...

  int i;
  for(i=0;i<ps.size()-1;i++) {// forward differences to get tangents...
    Vector v = ps[i+1]-ps[i];
    v.normalize();

    tangents_[tstart++] = v; // vector is set...
    points_[pstart++] = ps[i];
    if (i) { // only store it once...
      tangents_[tstart++] = v; // duplicate it otherwise
      points_[pstart++] = ps[i];
    }
  }
  tangents_[tstart] = tangents_[tstart-1]; // duplicate last guy...
  points_[pstart] = ps[i]; // last point...

}

void TexGeomLines::batch_add(Array1<double>&, Array1<Point>& ps,
			     Array1<Color>& cs)
{
  tex_per_seg_ = 0;  // this is not the hedgehog...
  int pstart = points_.size();
  int tstart = tangents_.size();
  int cstart = colors_.size();

  //  cerr << "Adding with colors...\n";

  points_.grow(2*(ps.size()-1));
  tangents_.grow(2*(ps.size()-1));
  colors_.grow(2*(ps.size()-1));

  int i;
  for(i=0;i<ps.size()-1;i++)
  {// forward differences to get tangents...
    Vector v = ps[i+1]-ps[i];
    v.normalize();

    tangents_[tstart++] = v; // vector is set...
    points_[pstart++] = ps[i];
    colors_[cstart++] = Colorub(cs[i]);
    if (i)
    { // only store it once...
      tangents_[tstart++] = v; // duplicate it otherwise
      points_[pstart++] = ps[i];
      colors_[cstart++] = Colorub(cs[i]);
    }
  }
  tangents_[tstart] = tangents_[tstart-1]; // duplicate last guy...
  points_[pstart] = ps[i]; // last point...
  colors_[cstart] = Colorub(cs[i]);
}


// this code sorts in three axis...
struct SortHelper {
  static Point* points__array;
  int id; // id for this guy...
};

Point* SortHelper::points__array=0;

int CompX(const void* e1, const void* e2)
{
  SortHelper *a = (SortHelper*)e1;
  SortHelper *b = (SortHelper*)e2;

  if (SortHelper::points__array[a->id].x() >
      SortHelper::points__array[b->id].x())
    return 1;
  if (SortHelper::points__array[a->id].x() <
      SortHelper::points__array[b->id].x())
    return -1;

  return 0; // they are equal...
}

int CompY(const void* e1, const void* e2)
{
  SortHelper *a = (SortHelper*)e1;
  SortHelper *b = (SortHelper*)e2;

  if (SortHelper::points__array[a->id].y() >
      SortHelper::points__array[b->id].y())
    return 1;
  if (SortHelper::points__array[a->id].y() <
      SortHelper::points__array[b->id].y())
    return -1;

  return 0; // they are equal...
}

int CompZ(const void* e1, const void* e2)
{
  SortHelper *a = (SortHelper*)e1;
  SortHelper *b = (SortHelper*)e2;

  if (SortHelper::points__array[a->id].z() >
      SortHelper::points__array[b->id].z())
    return 1;
  if (SortHelper::points__array[a->id].z() <
      SortHelper::points__array[b->id].z())
    return -1;

  return 0; // they are equal...
}


void TexGeomLines::Sort()
{
  const unsigned int vsize = points_.size()/2;
  if ((unsigned int)sindex_.size() == vsize*3) return;
  

  SortHelper::points__array = &points_[0];
  
  Array1<SortHelper> help; // list for help stuff...
  
  unsigned int realsize = points_.size()/2;
  unsigned int imul = 2;

  sindex_.resize(3*realsize); // resize the array...

  help.resize(realsize);

  unsigned int i;
  for(i=0; i<realsize; i++)
  {
    help[i].id = imul*i;  // start it in order...
  }

  qsort(&help[0],help.size(),sizeof(SortHelper),CompX);
  //	int (*) (const void*,const void*)CompX);

  // now dump these ids..

  for(i=0; i<realsize; i++)
  {
    sindex_[i] = help[i].id;
    help[i].id = imul*i;  // reset for next list...
  }
  
  qsort(&help[0],help.size(),sizeof(SortHelper),CompZ);

  unsigned int j;
  for(j=0; j<realsize; j++,i++)
  {
    sindex_[i] = help[j].id;
    help[j].id=imul*j;
  }

  qsort(&help[0],help.size(),sizeof(SortHelper),CompY);

  for(j=0; j<realsize; j++,i++)
  {
    sindex_[i] = help[j].id;
  }
}


Persistent* make_GeomCLineStrips()
{
  return new GeomCLineStrips();
}

PersistentTypeID GeomCLineStrips::type_id("GeomCLineStrips", "GeomObj", make_GeomCLineStrips);

GeomCLineStrips::GeomCLineStrips()
  : line_width_(0.0)
{
}

GeomCLineStrips::GeomCLineStrips(const GeomCLineStrips& copy)
  : line_width_(copy.line_width_),
    points_(copy.points_),
    colors_(copy.colors_)
{
}

GeomCLineStrips::~GeomCLineStrips()
{
}

GeomObj* GeomCLineStrips::clone()
{
  return new GeomCLineStrips(*this);
}

void GeomCLineStrips::get_bounds(BBox& bb)
{
  for(unsigned int s = 0; s < points_.size(); s++) {
    for (unsigned int i = 0; i < points_[s].size(); i+=3) {
      bb.extend(Point(points_[s][i+0], points_[s][i+1], points_[s][i+2]));
    }
  }
}

#define GEOMCLINESTRIPS_VERSION 1

void GeomCLineStrips::io(Piostream& stream)
{

  stream.begin_class("GeomCLineStrips", GEOMCLINESTRIPS_VERSION);
  GeomObj::io(stream);
  Pio(stream, line_width_);
  Pio(stream, points_);
  Pio(stream, colors_);
  stream.end_class();
}

bool GeomCLineStrips::saveobj(ostream&, const string&, GeomSave*)
{
#if 0
  NOT_FINISHED("GeomCLineStrips::saveobj");
  return false;
#else
  return true;
#endif
}


void
GeomCLineStrips::add(const vector<Point> &p, 
		     const vector<MaterialHandle> &c)
{
  points_.push_back(vector<float>());
  colors_.push_back(vector<unsigned char>());

  ASSERT(p.size() == c.size());
  for (unsigned int i = 0; i < p.size(); i++)
  {
    add(p[i], c[i]);
  }
}

void
GeomCLineStrips::add(const Point &p, 
		     const MaterialHandle c)
{
  if (points_.empty()) points_.push_back(vector<float>());
  if (colors_.empty()) colors_.push_back(vector<unsigned char>());
  
  points_.back().push_back(p.x());
  points_.back().push_back(p.y());
  points_.back().push_back(p.z());

  colors_.back().push_back(COLOR_FTOB(c->diffuse.r()));
  colors_.back().push_back(COLOR_FTOB(c->diffuse.g()));
  colors_.back().push_back(COLOR_FTOB(c->diffuse.b()));
  colors_.back().push_back(COLOR_FTOB(c->transparency));
}

void
GeomCLineStrips::newline()
{
  points_.push_back(vector<float>());
  colors_.push_back(vector<unsigned char>());
}




} // End namespace SCIRun

