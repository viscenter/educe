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


//    File   : StreamLineIntergrators.h
//    Author : Allen R. Sanderson
//    Date   : July 2006


#ifndef CORE_ALGORITHMS_FIELDS_STREAMLINES_STREAMLINEINTEGRATORS_H
#define CORE_ALGORITHMS_FIELDS_STREAMLINES_STREAMLINEINTEGRATORS_H 1

#include <string>
#include <vector>

#include <Core/Datatypes/Field.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>

#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace std;
using namespace SCIRun;

class SCISHARE StreamLineIntegrators
{
public:
  void FindAdamsBashforth();
  void FindAdamsMoulton();
  void FindHeun();
  void FindRK4();
  void FindRKF();

  int ComputeRKFTerms(Vector v[6],       // storage for terms
		      const Point &p,    // previous point
		      double s );        // current step size

  void integrate( unsigned int method );

protected:
  inline bool interpolate( const Point &p, Vector &v);

public:
  Point seed_;                         // initial point
  double tolerance2_;                  // square error tolerance
  double step_size_;                    // initial step size
  unsigned int max_steps_;              // max number of steps
  VField* vfield_;     // the field

  vector<Point> nodes_;                // storage for points
};

} // End namespace SCIRun

#endif 

