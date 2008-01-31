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
 *  Transform.cc: ?
 *
 *  Written by:
 *   Author ?
 *   Department of Computer Science
 *   University of Utah
 *   Date ?
 *
 *  Copyright (C) 199? SCI Group
 */

#include <Core/Util/TypeDescription.h>
#include <Core/Geometry/Transform.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Tensor.h>

namespace SCIRun {

using namespace std;

Persistent* transform_maker() {
  return new Transform();
}

// initialize the static member type_id
PersistentTypeID Transform::type_id("Transform", "Persistent", 
                                    transform_maker);

Transform::Transform() :
  SLIVR::Transform()
{
}

Transform::Transform(const Transform& copy) :
  SLIVR::Transform(copy)
{
}

Transform::Transform(const SLIVR::Transform& copy) :
  SLIVR::Transform(copy)
{
}

Transform::Transform(const Point& p, const Vector& i, 
                     const Vector& j, const Vector& k):
  SLIVR::Transform(p, i, j, k)
{
}

Transform::~Transform()
{
}

Transform&
Transform::operator=(const Transform& copy)
{
  SLIVR::Transform::operator=(copy);
  return *this;
}

const int TRANSFORM_VERSION = 1;

void 
Transform::io(Piostream& stream) {
  
  stream.begin_class("Transform", TRANSFORM_VERSION);
  for (int i=0; i<4; i++) {
    for (int j=0; j<4; j++){
      if (stream.reading()) {
	double tmp;
	Pio(stream, tmp);
	set_mat_val(i, j, tmp);
	Pio(stream, tmp);
	set_imat_val(i, j, tmp);
      } else {
	double tmp = get_mat_val(i, j);
	Pio(stream, tmp);
	tmp = get_imat_val(i, j);
	Pio(stream, tmp);
      }
    }
  }
  int iv = inv_valid();
  Pio(stream, iv);

  if (stream.reading()) {
    set_inv_valid(iv);
  }
  stream.end_class();
}

void
Pio_old(Piostream& stream, Transform& obj) {
  stream.begin_cheap_delim();
  for (int i=0; i<4; i++) {
    for (int j=0; j<4; j++) {
      if (stream.reading()) {
	double tmp;
	Pio(stream, tmp);
	obj.set_mat_val(i, j, tmp);
	Pio(stream, tmp);
	obj.set_imat_val(i, j, tmp);
      } else {
	double tmp = obj.get_mat_val(i, j);
	Pio(stream, tmp);
	tmp = obj.get_imat_val(i, j);
	Pio(stream, tmp);
      }
    }
  }
  int iv = obj.inv_valid();
  Pio(stream, iv);
  
  if (stream.reading()) {
    obj.set_inv_valid(iv);
  }
  stream.end_cheap_delim();
}

void
Pio(Piostream& stream, Transform*& obj)
{
  SCIRun::Persistent* pobj=obj;
  stream.io(pobj, Transform::type_id);
  if(stream.reading()) {
    obj=(Transform*)pobj;
  }
}

const string& 
Transform::get_h_file_path() {
  static const string path(TypeDescription::cc_to_h(__FILE__));
  return path;
}

const TypeDescription*
get_type_description(Transform*)
{
  static TypeDescription* td = 0;
  if(!td){
    td = scinew TypeDescription("Transform", Transform::get_h_file_path(), 
                                "SCIRun");
  }
  return td;
}


Tensor
operator*(const Transform &t, const Tensor &d)
{
  double result[9];
  
  double mat[16];
  t.get(mat);

  for (int k=0; k<9; k++) result[k] = 0.0;
  
  for (int k=0; k<3; k++)
  {
    for(int i=0;i<3;i++) 
    {
      for(int j=0;j<3;j++) 
      {
        result[i+3*k] += mat[4*i + j] * d.mat_[j][k];
      }
    }
  }

  return (SCIRun::Tensor(result[0],result[1],result[2],result[4],result[5],result[8]));
}


Tensor
operator*(const Tensor &d, const Transform &t)
{
  double result[9];
  
  double mat[16];
  t.get(mat);

  for (int k=0; k<9; k++) result[k] = 0.0;
  
  for (int k=0; k<3; k++)
  {
    for(int i=0;i<3;i++) 
    {
      for(int j=0;j<3;j++) 
      {
        result[i+3*k] += mat[4*j + i] * d.mat_[j][k];
      }
    }
  }
  
  return (SCIRun::Tensor(result[0],result[1],result[2],result[4],result[5],result[8]));
}

} // namespace SCIRun
