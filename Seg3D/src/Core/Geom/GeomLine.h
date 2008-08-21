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
 *  Line.h:  Line object
 *
 *  Written by:
 *   Steven G. Parker & David Weinstein
 *   Department of Computer Science
 *   University of Utah
 *   April 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

#ifndef SCI_Geom_Line_h
#define SCI_Geom_Line_h 1

#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/Material.h>

#include <Core/Geom/GeomObj.h>
#include <Core/Geometry/Point.h>
#include <Core/Thread/Mutex.h>

#include <vector>

#include <stdlib.h>	// For size_t

#include <Core/Geom/share.h>

namespace SCIRun {

using std::vector;

class SCISHARE GeomLine : public GeomObj
{
public:
  Point p1, p2;
  float line_width_;

  GeomLine(const Point& p1, const Point& p2);
  GeomLine(const GeomLine&);
  virtual ~GeomLine();
  virtual GeomObj* clone();

  virtual void get_bounds(BBox&);
  void setLineWidth(float val);

  virtual void draw(DrawInfoOpenGL*, Material*, double time);

  virtual void io(Piostream&);
  static PersistentTypeID type_id;
};

class SCISHARE GeomLines : public GeomObj
{
public:
  GeomLines();
  GeomLines(const GeomLines&);

  virtual ~GeomLines();
  virtual GeomObj* clone();

  void add(const Point &p0, const Point &p1);
  void add(const Point &p0, const MaterialHandle &c0,
	   const Point &p1, const MaterialHandle &c1);
  void add(const Point &p0, double cindex0,
	   const Point &p1, double cindex1);
  void add(const Point &p0, const MaterialHandle &c0, double cindex0,
	   const Point &p1, const MaterialHandle &c1, double cindex1);
  void setLineWidth(float val) { line_width_ = val; }

  unsigned int size() { return (int)points_.size() / 6; }

  virtual void get_bounds(BBox&);

  virtual void draw(DrawInfoOpenGL*, Material*, double time);
  
  virtual void io(Piostream&);
  static PersistentTypeID type_id;

protected:
  double line_width_;

  vector<float>         points_;
  vector<unsigned char> colors_;
  vector<float>         indices_;
};


class SCISHARE GeomTranspLines : public GeomLines
{
public:
  GeomTranspLines();
  GeomTranspLines(const GeomTranspLines&);

  virtual ~GeomTranspLines();
  virtual GeomObj* clone();

  void Sort();

  virtual void draw(DrawInfoOpenGL*, Material*, double time);
  
  virtual void io(Piostream&);
  static PersistentTypeID type_id;

protected:
  vector<unsigned int> xindices_;
  vector<unsigned int> yindices_;
  vector<unsigned int> zindices_;

  bool xreverse_;
  bool yreverse_;
  bool zreverse_;
};


// can generate "lit" streamlines this way

class SCISHARE TexGeomLines : public GeomObj
{
public:
  TexGeomLines();
  TexGeomLines(const TexGeomLines&);

  void add(const Point&, const Point&, double scale); // for one 
  void add(const Point&, const Vector&, const Colorub&); // for one 
  
  void batch_add(Array1<double>& t, Array1<Point>&); // adds a bunch
  // below is for when you also have color...
  void batch_add(Array1<double>& t, Array1<Point>&,Array1<Color>&);

  void CreateTangents(); // creates tangents from curve...

  void Sort();  // creates sorted lists...

  virtual ~TexGeomLines();
  virtual GeomObj* clone();
  virtual void get_bounds(BBox&);

  virtual void draw(DrawInfoOpenGL*, Material*, double time);
  
  virtual void io(Piostream&);
  static PersistentTypeID type_id;

protected:
  Mutex mutex_;

  Array1<Point>   points_;
  Array1<Vector>  tangents_;  // used in 1D illumination model...
  Array1<double>  times_;
  Array1<Colorub> colors_;    // only if you happen to have color...
  Array1<int>     sindex_;    // indices of the x, y and z sorted list

  double alpha_;              // 2D texture wi alpha grad/mag would be nice

  Array1<unsigned char> tmap1d_;  // 1D texture - should be in Viewer?
  int tmapid_;                    // id for this texture map
  int tex_per_seg_;               // 0 if batched...
};


class SCISHARE GeomCLineStrips : public GeomObj
{
public:
  GeomCLineStrips();
  GeomCLineStrips(const GeomCLineStrips&);
  virtual ~GeomCLineStrips();

  void add(const vector<Point> &p, const vector<MaterialHandle> &c);
  void add(const Point &p, const MaterialHandle c);
  void newline();
  void setLineWidth(float val) { line_width_ = val; }

  virtual GeomObj* clone();
  virtual void get_bounds(BBox&);

  virtual void draw(DrawInfoOpenGL*, Material*, double time);
  
  virtual void io(Piostream&);
  static PersistentTypeID type_id;
  virtual bool saveobj(std::ostream&, const string& format, GeomSave*);

protected:
  double line_width_;

  vector<vector<float> >         points_;
  vector<vector<unsigned char> > colors_;
};

} // End namespace SCIRun

#endif /* SCI_Geom_Line_h */
