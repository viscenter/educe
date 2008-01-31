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
 *  DataConversions.cc: Data converters for rendering alogrithms
 *
 *  Written by:
 *   Allen R. Sanderson
 *   SCI Institute
 *   University of Utah
 *   April 2007
 *
 *  Copyright (C) 2007 SCI Institute
 */

#include <Core/Algorithms/Visualization/DataConversions.h>

namespace SCIRun {

template <>
bool
to_color( const Vector &v, Color &c )
{
  c = Color(fabs(v.x()), fabs(v.y()), fabs(v.z()));
  return true;
}

template <>
bool
to_color( const Tensor &t, Color &c )
{
  Tensor tt = t;

  Vector e1, e2, e3;
  tt.get_eigenvectors(e1, e2, e3);
      
  e1.safe_normalize();
  double rr = fabs(e1.x());
  double gg = fabs(e1.y());
  double bb = fabs(e1.z());
  
  Color rgb1(rr, gg, bb);
  HSVColor hsv(rgb1);
  hsv[1] = 0.7;
  hsv[2] = 1.0;
  
  c = Color(hsv);
  return true;
}

// Conversion template specialization.
template <>
bool
to_double( const Vector &in, double &out)
{
  out = in.length();
  return true;
}

template <>
bool
to_double( const Tensor &in, double &out)
{
  Tensor t = in;
  double v1, v2, v3;
  t.get_eigenvalues(v1, v2, v3);
  out = Vector(v1, v2, v3).length();
  return false;
}

template <>
bool
to_double( const std::string &/*in*/, double &/*out*/)
{
  return false;
}

template <>
bool
to_vector( const Vector &in, Vector &out)
{
  out = in;
  return true;
}

template <>
bool
to_vector( const Tensor &in, Vector &out)
{
  Tensor t = in;
  double v1, v2, v3;
  t.get_eigenvalues(v1, v2, v3);
  out = Vector(v1, v2, v3);
  return true;
}

template <>
bool
to_tensor( const Tensor &in, Tensor &out)
{
  out = in;
  return true;
}

template <>
bool
to_buffer( const unsigned char &value, std::ostringstream &buffer)
{
  buffer << (int) value;
  return true;
}

template <>
bool
to_buffer( const char &value, std::ostringstream &buffer)
{
  buffer << (int) value;
  return true;
}

}
