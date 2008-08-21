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

#include <Core/Datatypes/VFData.h>


#define VFDATA_ACCESS_DEFINITION(type) \
void VFData::get_value(type &val, VMesh::index_type idx) const \
{ ASSERTFAIL("VFData interface has no virtual function implementation for get_value"); } \
\
void VFData::set_value(const type &val, VMesh::index_type idx) \
{ ASSERTFAIL("VFData interface has no virtual function implementation for set_value"); } \
\
void VFData::get_evalue(type &val, VMesh::index_type idx) const \
{ ASSERTFAIL("VFData interface has no virtual function implementation for get_evalue"); } \
\
void  VFData::set_evalue(const type &val, VMesh::index_type idx) \
{ ASSERTFAIL("VFData interface has no virtual function implementation for set_evalue"); } \
\
void VFData::get_values(type *ptr, VMesh::size_type sz, VMesh::size_type offset) const \
{ ASSERTFAIL("VFData interface has no virtual function implementation for get_values"); } \
\
void VFData::set_values(const type *ptr, VMesh::size_type sz, VMesh::size_type offset) \
{ ASSERTFAIL("VFData interface has no virtual function implementation for set_values"); } \
\
void VFData::get_evalues(type *ptr, VMesh::size_type sz, VMesh::size_type offset) const \
{ ASSERTFAIL("VFData interface has no virtual function implementation for get_evalues"); } \
\
void VFData::set_evalues(const type *ptr, VMesh::size_type sz, VMesh::size_type offset) \
{ ASSERTFAIL("VFData interface has no virtual function implementation for set_evalues"); } \
\
void VFData::set_all_values(const type &val) \
{ ASSERTFAIL("VFData interface has no virtual function implementation for set_all_values"); } \
\
void VFData::get_weighted_value(type &val, VMesh::index_type* idx, VMesh::weight_type* w, VMesh::size_type sz) const \
{ ASSERTFAIL("VFData interface has no virtual function implementation for get_weighted_values"); } \
\
void VFData::get_weighted_evalue(type &val, VMesh::index_type* idx, VMesh::weight_type* w, VMesh::size_type sz) const \
{ ASSERTFAIL("VFData interface has no virtual function implementation for get_weighted_values"); } \
\
void VFData::get_values(type *ptr, VMesh::Node::array_type& nodes ) const \
{ ASSERTFAIL("VFData interface has no virtual function implementation for get_values"); } \
\
void VFData::get_values(type *ptr, VMesh::Elem::array_type& elems ) const \
{ ASSERTFAIL("VFData interface has no virtual function implementation for get_values"); } \
\
void VFData::set_values(const type *ptr, VMesh::Node::array_type& nodes ) \
{ ASSERTFAIL("VFData interface has no virtual function implementation for set_values"); } \
\
void VFData::set_values(const type *ptr, VMesh::Elem::array_type& elems ) \
{ ASSERTFAIL("VFData interface has no virtual function implementation for set_values"); } \


#define VFDATA_ACCESS_DEFINITION2(type) \
void VFData::interpolate(type &val, VMesh::ElemInterpolate &interp, type defval) const \
{ ASSERTFAIL("VFData interface has no virtual function implementation for interpolate"); } \
\
void VFData::minterpolate(vector<type> &val, VMesh::MultiElemInterpolate &interp, type defval) const \
{ ASSERTFAIL("VFData interface has no virtual function implementation for minterpolate"); } \
\
void VFData::gradient(StackVector<type,3> &val, VMesh::ElemGradient &interp) const \
{ ASSERTFAIL("VFData interface has no virtual function implementation for gradient"); } \
\
void VFData::mgradient(vector<StackVector<type,3> > &val, VMesh::MultiElemGradient &interp) const \
{ ASSERTFAIL("VFData interface has no virtual function implementation for mgradient"); } \
\

namespace SCIRun {

VMesh::size_type 
VFData::fdata_size() const
{
  ASSERTFAIL("VFData interface has no virtual function implementation for fdata_size");
}

VMesh::size_type 
VFData::efdata_size() const
{
  ASSERTFAIL("VFData interface has no virtual function implementation for efdata_size");
}

void* 
VFData::fdata_pointer() const
{
  ASSERTFAIL("VFData interface has no virtual function implementation for fdata_pointer");
}

void* 
VFData::efdata_pointer() const
{
  ASSERTFAIL("VFData interface has no virtual function implementation for efdata_pointer");
}

void
VFData::resize_fdata(VMesh::dimension_type dim)
{
  ASSERTFAIL("VFData interface has no virtual function implementation for resize_fdata");
}

void
VFData::resize_efdata(VMesh::dimension_type dim)
{
  ASSERTFAIL("VFData interface has no virtual function implementation for resize_efdata");
}

bool
VFData::min(double& val,VMesh::index_type& idx) const
{
  ASSERTFAIL("VFData interface has no virtual function implementation for min");
}

bool
VFData::max(double& val,VMesh::index_type& idx) const
{
  ASSERTFAIL("VFData interface has no virtual function implementation for min");
}

bool
VFData::minmax(double& min,VMesh::index_type& idxmin,
               double& max,VMesh::index_type& idxmax) const
{
  ASSERTFAIL("VFData interface has no virtual function implementation for minmax");
}


void 
VFData::copy_value(VFData* fdata, VMesh::index_type vidx, VMesh::index_type idx)
{
  ASSERTFAIL("VFData interface has no virtual function implementation for copy_value");
}
 
void 
VFData::copy_evalue(VFData* fdata, VMesh::index_type vidx, VMesh::index_type idx)
{
  ASSERTFAIL("VFData interface has no virtual function implementation for copy_evalue");
}  

void 
VFData::copy_values(VFData* fdata, VMesh::index_type vidx, VMesh::index_type idx, VMesh::size_type num)
{
  ASSERTFAIL("VFData interface has no virtual function implementation for copy_values");
}
 
void 
VFData::copy_evalues(VFData* fdata, VMesh::index_type vidx, VMesh::index_type idx, VMesh::size_type num)
{
  ASSERTFAIL("VFData interface has no virtual function implementation for copy_evalues");
}  

void 
VFData::copy_weighted_value(VFData* fdata, VMesh::index_type* vidx, VMesh::weight_type* vw, VMesh::size_type sz, VMesh::index_type idx)
{
  ASSERTFAIL("VFData interface has no virtual function implementation for copy_weighted_value");
}

void 
VFData::copy_weighted_evalue(VFData* fdata, VMesh::index_type* vidx, VMesh::weight_type* vw, VMesh::size_type sz, VMesh::index_type idx)
{
  ASSERTFAIL("VFData interface has no virtual function implementation for copy_weighted_evalue");
}

void 
VFData::copy_values(VFData* fdata)
{
  ASSERTFAIL("VFData interface has no virtual function implementation for copy_values");
}
  
void 
VFData::copy_evalues(VFData* fdata)
{
  ASSERTFAIL("VFData interface has no virtual function implementation for copy_evalues");
}

VMesh::size_type 
VFData::size()
{
  return (0);
}

VFDATA_ACCESS_DEFINITION(double)
VFDATA_ACCESS_DEFINITION(Vector)
VFDATA_ACCESS_DEFINITION(Tensor)
VFDATA_ACCESS_DEFINITION(char)
VFDATA_ACCESS_DEFINITION(unsigned char)
VFDATA_ACCESS_DEFINITION(short)
VFDATA_ACCESS_DEFINITION(unsigned short)
VFDATA_ACCESS_DEFINITION(int)
VFDATA_ACCESS_DEFINITION(unsigned int)
VFDATA_ACCESS_DEFINITION(long long)
VFDATA_ACCESS_DEFINITION(unsigned long long)
VFDATA_ACCESS_DEFINITION(float)

VFDATA_ACCESS_DEFINITION2(int)
VFDATA_ACCESS_DEFINITION2(float)
VFDATA_ACCESS_DEFINITION2(double)
VFDATA_ACCESS_DEFINITION2(Vector)
VFDATA_ACCESS_DEFINITION2(Tensor)


}

