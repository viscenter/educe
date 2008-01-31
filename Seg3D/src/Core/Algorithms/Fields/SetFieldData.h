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


#ifndef CORE_ALGORITHMS_FIELDS_SETFIELDDATA_H
#define CORE_ALGORITHMS_FIELDS_SETFIELDDATA_H 1

// The following include file will include all tools needed for doing 
// dynamic compilation and will include all the standard dataflow types
#include <Core/Algorithms/Util/DynamicAlgo.h>
#include <Core/Algorithms/Util/VirtualAlgo.h>

//! for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SCISHARE SetFieldDataAlgo : public SCIRun::DynamicAlgoBase
{
  public:
    // This function is now pure virtual
    bool SetFieldData(SCIRun::ProgressReporter *pr,
                      SCIRun::FieldHandle& field_input_handle, 
                      SCIRun::FieldHandle& field_output_handle, 
                      SCIRun::MatrixHandle& matrix_input_handle, 
                      bool keepscalartype);  

    bool SetFieldData(SCIRun::ProgressReporter *pr,
			      SCIRun::FieldHandle& field_input_handle,
			      SCIRun::FieldHandle& field_output_handle,
			      SCIRun::NrrdDataHandle& nrrddata_input_handle,
			      bool keepscalartype);  
            
            
    template <class T>
    bool SetFieldDataV(SCIRun::FieldHandle ouput,
		       NrrdDataHandle nrrd,
		       std::string output_type);
};


template <class T>
bool
SetFieldDataAlgo::SetFieldDataV(SCIRun::FieldHandle output, NrrdDataHandle nrrd
    , std::string output_datatype)
{
  VField* vfield = output->vfield();
  T* data = reinterpret_cast<T*>(nrrd->nrrd_->data);
  VMesh::size_type num_values = vfield->num_values();
  
  if (output_datatype == "Scalar")
  {
    vfield->set_values(data,num_values);
  }
  else if (output_datatype == "Vector")
  {
    VMesh::index_type k = 0;
    for(VMesh::index_type idx=0; idx<num_values; idx++)
    {
      Vector v(data[k],data[k+1],data[k+2]); k += 3;
      vfield->set_value(v,idx); 
    }
  }
  else if (output_datatype == "Tensor6")
  {
    VMesh::index_type k = 0;
    for(VMesh::index_type idx=0; idx<num_values; idx++)
    {
      Tensor tensor(data[k],data[k+1],data[k+2],data[k+3],data[k+4],data[k+5]);
      k += 6;
      vfield->set_value(tensor,idx); 
    }
  }
  else if (output_datatype == "Tensor7")
  {
    VMesh::index_type k = 0;
    for(VMesh::index_type idx=0; idx<num_values; idx++)
    {
      Tensor tensor(data[k+1],data[k+2],data[k+3],data[k+4],data[k+5],data[k+6]);
      k += 7;
      vfield->set_value(tensor,idx); 
    }
  }
  else if (output_datatype == "Tensor9")
  {
    VMesh::index_type k = 0;
    for(VMesh::index_type idx=0; idx<num_values; idx++)
    {
      Tensor tensor(data[k],data[k+1],data[k+2],data[k+4],data[k+5],data[k+8]);
      k += 9;
      vfield->set_value(tensor,idx); 
    }
  }
  return (true);      
}


} // end namespace SCIRunAlgo

#endif

