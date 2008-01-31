//
//  For more information, please see: http://software.sci.utah.edu
//
//  The MIT License
//
//  Copyright (c) 2006 Scientific Computing and Imaging Institute,
//  University of Utah.
//
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//    File   : RectRegion.h
//    Author : McKay Davis
//    Date   : Tue Jun 27 13:08:13 2006

#ifndef SKINNER_RECTREGION_H
#define SKINNER_RECTREGION_H

#include <teem/air.h>

#include <Core/Skinner/share.h>

namespace SCIRun {
namespace Skinner {

struct SCISHARE RectRegion {
private:
  double            coords_[4];   // x1, y1, x2, y2

public:
  RectRegion();
  RectRegion(double x1, double y1, double x2, double y2);

  inline RectRegion(const RectRegion &copy) {
    memcpy(coords_, copy.coords_, sizeof(double)*4);
  }

  inline RectRegion &       operator=(const RectRegion &copy) {
    memcpy(coords_, copy.coords_, sizeof(double)*4);
    return *this;
  }

  inline double             x1() const { return coords_[0]; }
  inline double             y1() const { return coords_[1]; }
  inline double             x2() const { return coords_[2]; }
  inline double             y2() const { return coords_[3]; }
  inline double             width() const { return coords_[2]-coords_[0]; }
  inline double             height() const { return coords_[3]-coords_[1];}
  inline double &           operator[](unsigned int i) {return coords_[i];}
  inline const double &     operator[](unsigned int i) const {return coords_[i];}

  bool                      inside(double x, double y) const;
  bool                      valid() const;
  bool                      intersects(const RectRegion &region) const;
  RectRegion                operator+(const RectRegion &rhs) const;
  RectRegion                operator-(const RectRegion &rhs) const;
};


}
}

#endif
