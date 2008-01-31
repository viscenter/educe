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

#include <Core/Algorithms/Fields/GetMeshQualityField.h>

namespace SCIRunAlgo {

bool
GetMeshQualityFieldAlgo::GetMeshQualityField(ProgressReporter *pr,FieldHandle input, FieldHandle& output, std::string metric)
{
  if (input.get_rep() == 0)
  {
    pr->error("GetMeshQualityField: No input field");
    return (false);
  }

  FieldInformation fi(input);
  fi.make_double();
  fi.make_constantdata();
  
  output = CreateField(fi,input->mesh());
  
  if (output.get_rep() == 0)
  {
    pr->error("GetMeshQualityField: Could not create output field");
    return (false);
  }
  
  VField* ofield = output->vfield();
  VMesh*  imesh  = input->vmesh();
  
  if (metric == "scaled_jacobian")
  {
    VMesh::Elem::size_type num_values = imesh->num_elems();
    for (VMesh::Elem::index_type j=0; j<num_values; j++)
    {
      ofield->set_value(imesh->scaled_jacobian_metric(j),j);
    }
  }
  else if (metric == "jacobian")
  {
    VMesh::Elem::size_type num_values = imesh->num_elems();
    for (VMesh::Elem::index_type j=0; j<num_values; j++)
    {
      ofield->set_value(imesh->jacobian_metric(j),j);
    }  
  }
  else if (metric == "volume")
  {
    VMesh::Elem::size_type num_values = imesh->num_elems();
    for (VMesh::Elem::index_type j=0; j<num_values; j++)
    {
      ofield->set_value(imesh->volume_metric(j),j);
    }  
  }

  return (true);
}


}
