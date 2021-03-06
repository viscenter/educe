#ifndef SCIRun_ConvertFieldDataFromIndicesToTensors_H
#define SCIRun_ConvertFieldDataFromIndicesToTensors_H
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


#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <Core/Geometry/Tensor.h>

namespace SCIRun {

// just get the first point from the field
class ConvertFieldDataFromIndicesToTensorsAlgo : public DynamicAlgoBase
{
public:
  virtual FieldHandle execute(FieldHandle srcH) = 0;

  //! support the dynamically compiled algorithm concept
 static CompileInfoHandle get_compile_info(const TypeDescription *field_src_td,
				           const string &field_dst_name);
};


template <class FSRC, class FDST>
class ConvertFieldDataFromIndicesToTensorsAlgoT : public ConvertFieldDataFromIndicesToTensorsAlgo
{
public:
  //! virtual interface. 
  virtual FieldHandle execute(FieldHandle srcH);
};

template <class FSRC, class FDST>
FieldHandle
ConvertFieldDataFromIndicesToTensorsAlgoT<FSRC, FDST>::execute(FieldHandle srcH)
{
  FSRC *src = dynamic_cast<FSRC *>(srcH.get_rep());
  FDST *dst = scinew FDST(src->get_typed_mesh());
  vector<pair<string, Tensor> > conds;
  src->get_property("conductivity_table", conds);
  typename FSRC::fdata_type::iterator in = src->fdata().begin();
  typename FDST::fdata_type::iterator out = dst->fdata().begin();
  typename FSRC::fdata_type::iterator end = src->fdata().end();
  while (in != end) {
    *out = conds[(int)(*in)].second;
    ++in; ++out;
  }
  return dst;
}
} // End namespace BioPSE

#endif
