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
 *  GeomQuads.h: Quads
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   May 2003
 *
 *  Copyright (C) 2003 SCI Group
 */

#ifndef SCI_Geom_Quads_h
#define SCI_Geom_Quads_h 1

#include <Core/Geom/GeomObj.h>
#include <Core/Geom/Material.h>
#include <Core/Geometry/Point.h>

#include <vector>

#include <Core/Geom/share.h>

namespace SCIRun {

using std::vector;

class SCISHARE GeomFastQuads : public GeomObj
{
protected:
  vector<float> points_;
  vector<unsigned char> colors_;
  vector<float> indices_;
  vector<float> normals_;
  MaterialHandle material_;

public:
  GeomFastQuads();
  GeomFastQuads(const GeomFastQuads&);
  virtual ~GeomFastQuads();
  virtual GeomObj* clone();

  int size(void);
  void add(const Point &p0, const Point &p1,
	   const Point &p2, const Point &p3);
  void add(const Point &p0, const MaterialHandle &m0,
	   const Point &p1, const MaterialHandle &m1,
	   const Point &p2, const MaterialHandle &m2,
	   const Point &p3, const MaterialHandle &m3);
  void add(const Point &p0, double cindex0,
	   const Point &p1, double cindex1,
	   const Point &p2, double cindex2,
	   const Point &p3, double cindex3);
  void add(const Point &p0, const MaterialHandle &m0, double cindex0,
	   const Point &p1, const MaterialHandle &m1, double cindex1,
	   const Point &p2, const MaterialHandle &m2, double cindex2,
	   const Point &p3, const MaterialHandle &m3, double cindex3);

  void add(const Point &p0, const Vector &n0,
	   const Point &p1, const Vector &n1,
	   const Point &p2, const Vector &n2,
	   const Point &p3, const Vector &n3);
  void add(const Point &p0, const Vector &n0, const MaterialHandle &m0,
	   const Point &p1, const Vector &n1, const MaterialHandle &m1,
	   const Point &p2, const Vector &n2, const MaterialHandle &m2,
	   const Point &p3, const Vector &n3, const MaterialHandle &m3);
  void add(const Point &p0, const Vector &n0, double cindex0,
	   const Point &p1, const Vector &n1, double cindex1,
	   const Point &p2, const Vector &n2, double cindex2,
	   const Point &p3, const Vector &n3, double cindex3);
  void add(const Point &p0, const Vector &n0,
	   const MaterialHandle &m0, double cindex0,
	   const Point &p1, const Vector &n1,
	   const MaterialHandle &m1, double cindex1,
	   const Point &p2, const Vector &n2,
	   const MaterialHandle &m2, double cindex2,
	   const Point &p3, const Vector &n3,
	   const MaterialHandle &m3, double cindex3);

  void add( const vector< vector< pair<Point, Vector> > >& quadstrips );
  void add( const vector< vector< pair<Point, Vector> > >& quadstrips,
	    const MaterialHandle &mat );
  void add( const vector< vector< pair<Point, Vector> > >& quadstrips,
	    double cindex );
  void add( const vector< vector< pair<Point, Vector> > >& quadstrips,
	    const MaterialHandle &mat, double cindex );

  virtual void get_bounds(BBox& bb);

  virtual void draw(DrawInfoOpenGL*, Material*, double time);
  virtual void drawVertexData(DrawInfoOpenGL*);

  virtual void io(Piostream&);
  static PersistentTypeID type_id;
};


class SCISHARE GeomFastQuadsTwoSided : public GeomObj
{
protected:
  vector<float> points_;
  vector<unsigned char> colors_;
  vector<unsigned char> colors2_;  
  vector<float> indices_;
  vector<float> indices2_;
  vector<float> normals_;
  MaterialHandle material_;
  MaterialHandle material2_;
 
public:
  GeomFastQuadsTwoSided();
  GeomFastQuadsTwoSided(const GeomFastQuadsTwoSided&);
  virtual ~GeomFastQuadsTwoSided();
  virtual GeomObj* clone();

  int size(void);
  void add(const Point &p0, const Point &p1,
	   const Point &p2, const Point &p3);
  void add(const Point &p0, const MaterialHandle &m0, const MaterialHandle &k0,
	   const Point &p1, const MaterialHandle &m1, const MaterialHandle &k1,
	   const Point &p2, const MaterialHandle &m2, const MaterialHandle &k2,
	   const Point &p3, const MaterialHandle &m3, const MaterialHandle &k3);     
  void add(const Point &p0, double cindex0, double dindex0,
	   const Point &p1, double cindex1, double dindex1,
	   const Point &p2, double cindex2, double dindex2,
	   const Point &p3, double cindex3, double dindex3);
  void add(const Point &p0,
	   const MaterialHandle &m0, double cindex0,
	   const MaterialHandle &k0, double dindex0,
	   const Point &p1,
	   const MaterialHandle &m1, double cindex1,
	   const MaterialHandle &k1, double dindex1,
	   const Point &p2,
	   const MaterialHandle &m2, double cindex2,
	   const MaterialHandle &k2, double dindex2,
	   const Point &p3,
	   const MaterialHandle &m3, double cindex3,
	   const MaterialHandle &k3, double dindex3);

  void add(const Point &p0, const Vector &n0,
	   const Point &p1, const Vector &n1,
	   const Point &p2, const Vector &n2,
	   const Point &p3, const Vector &n3);
  void add(const Point &p0, const Vector &n0, double cindex0, double dindex0,
	   const Point &p1, const Vector &n1, double cindex1, double dindex1,
	   const Point &p2, const Vector &n2, double cindex2, double dindex2,
	   const Point &p3, const Vector &n3, double cindex3, double dindex3);
  void add(const Point &p0, const Vector &n0, const MaterialHandle &m0, const MaterialHandle &k0,
	   const Point &p1, const Vector &n1, const MaterialHandle &m1, const MaterialHandle &k1,
	   const Point &p2, const Vector &n2, const MaterialHandle &m2, const MaterialHandle &k2,
	   const Point &p3, const Vector &n3, const MaterialHandle &m3, const MaterialHandle &k3);
  void add(const Point &p0, const Vector &n0,
	   const MaterialHandle &m0, double cindex0,
	   const MaterialHandle &k0, double dindex0,
	   const Point &p1, const Vector &n1,
	   const MaterialHandle &m1, double cindex1,
	   const MaterialHandle &k1, double dindex1,
	   const Point &p2, const Vector &n2,
	   const MaterialHandle &m2, double cindex2,
	   const MaterialHandle &k2, double dindex2,
	   const Point &p3, const Vector &n3,
	   const MaterialHandle &m3, double cindex3,
	   const MaterialHandle &k3, double dindex3);

  virtual void get_bounds(BBox& bb);

  virtual void draw(DrawInfoOpenGL*, Material*, double time);
  virtual void drawVertexData(DrawInfoOpenGL*);

  virtual void io(Piostream&);
  static PersistentTypeID type_id;
};


class SCISHARE GeomTranspQuads : public GeomFastQuads
{
protected:
  vector<unsigned int> xlist_;
  vector<unsigned int> ylist_;
  vector<unsigned int> zlist_;
  bool xreverse_;
  bool yreverse_;
  bool zreverse_;

public:
  GeomTranspQuads();
  GeomTranspQuads(const GeomTranspQuads&);
  virtual ~GeomTranspQuads();
  virtual GeomObj* clone();

  void SortPolys();

  virtual void draw(DrawInfoOpenGL*, Material*, double time);

  virtual void io(Piostream&);
  static PersistentTypeID type_id;
};


class SCISHARE GeomTranspQuadsTwoSided : public GeomFastQuadsTwoSided
{
protected:
  vector<unsigned int> xlist_;
  vector<unsigned int> ylist_;
  vector<unsigned int> zlist_;
  bool xreverse_;
  bool yreverse_;
  bool zreverse_;

public:
  GeomTranspQuadsTwoSided();
  GeomTranspQuadsTwoSided(const GeomTranspQuadsTwoSided&);
  virtual ~GeomTranspQuadsTwoSided();
  virtual GeomObj* clone();

  void SortPolys();

  virtual void draw(DrawInfoOpenGL*, Material*, double time);

  virtual void io(Piostream&);
  static PersistentTypeID type_id;
};

} // End namespace SCIRun

#endif /* SCI_Geom_Quads_h */
