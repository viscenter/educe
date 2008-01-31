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

#if !defined(Visualization_DataConversions_h)
#define Visualization_DataConversions_h

#include <Core/Datatypes/Color.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Tensor.h>

#include <sstream>
#include <iomanip>
#include <iostream>

#undef SCISHARE
#if defined(_WIN32) && !defined(BUILD_SCIRUN_STATIC)
#define SCISHARE __declspec(dllexport)
#else
#define SCISHARE
#endif

namespace SCIRun {

// Conversion templates.
template <class T>
SCISHARE bool
to_color( const T &v, Color &c )
{
  c = Color(fabs(v), fabs(v), fabs(v));
  return true;
}

template <class T>
SCISHARE bool
to_double( const T& in, double &out)
{
  out = (double) in;
  return true;
}


template <class T>
SCISHARE bool
to_vector( const T& in, Vector &out)
{
  return false;
}


template <class T>
SCISHARE bool
to_tensor( const T& in, Tensor &out)
{
  return false;
}

template <class T>
SCISHARE bool
to_buffer( const T &value, std::ostringstream &buffer)
{
  buffer << value;
  return true;
}


// Conversion template specialization.
template <>
SCISHARE bool to_color( const Vector &v, Color &c );

template <>
SCISHARE bool to_color( const Tensor &t, Color &c );

template <>
SCISHARE bool to_double(const Vector &in, double &out);

template <>
SCISHARE bool to_double(const Tensor &in, double &out);

template <>
SCISHARE bool to_double(const std::string &in, double &out);

template <>
SCISHARE bool to_vector(const Vector &in, Vector &out);

template <>
SCISHARE bool to_vector(const Tensor &in, Vector &out);

template <>
SCISHARE bool to_tensor(const Tensor &in, Tensor &out);

template <>
SCISHARE bool to_buffer(const unsigned char &value, std::ostringstream &buffer);

template <>
SCISHARE bool to_buffer(const char &value, std::ostringstream &buffer);

}

#endif
