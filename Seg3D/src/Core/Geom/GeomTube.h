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
 *  GeomTube.h: Tube object
 *
 *  Written by:
 *   Han-Wei Shen
 *   Department of Computer Science
 *   University of Utah
 *   Oct 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

#ifndef SCI_Geom_Tube_h 
#define SCI_Geom_Tube_h 1 

#include <Core/Geom/GeomVertexPrim.h>

class SinCosTable;

namespace SCIRun {

class GeomTube : public GeomVertexPrim
{
public:
  GeomTube(int nu=8); 
  GeomTube(const GeomTube&); 
  virtual ~GeomTube(); 

  virtual GeomObj* clone(); 
  virtual void get_bounds(BBox&); 
  
  void add(GeomVertex*, double, const Vector&);

  virtual void draw(DrawInfoOpenGL*, Material*, double time); 

  virtual void io(Piostream&);
  static PersistentTypeID type_id;

private:
  int nu;
  Array1<Vector> directions;
  Array1<double> radii;
  void make_circle(int which, Array1<Point>& circle,
                   const SinCosTable& tab); 
};

} // End namespace SCIRun

#endif /*SCI_Geom_Tube_h */
