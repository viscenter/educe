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

#include <Core/Algorithms/Fields/TransformMeshWithTransform.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool TransformMeshWithTransformAlgo::TransformMeshWithTransform(ProgressReporter *pr, FieldHandle input, FieldHandle& output, Transform& transform, bool rotate_data)
{
  if (input.get_rep() == 0)
  {
    pr->error("TransformMeshWithTransform: No input field");
    return (false);
  }

  if (!(input->has_virtual_interface()))
  {
    pr->error("TransformMeshWithTransform: This algorithm requires a virtual field interface");
    return (false);  
  }
  
  FieldInformation fi(input);

  output = input;
  output.detach();
  output->mesh_detach();
  
  VMesh* mesh = output->vmesh();
  VField* field = output->vfield();
  
  mesh->transform(transform);

	output->copy_properties(input.get_rep()); 
  
  if (fi.is_vector() || fi.is_tensor())
  {
    if (rotate_data == true)
    {
      if (fi.is_vector())
      {
        VMesh::size_type sz = field->num_values();
        for (VMesh::index_type i=0; i < sz; i++)
        {
          Vector v;
          field->get_value(v,i);
          v = transform*v;
          field->set_value(v,i);
        }
      }
      if (fi.is_tensor())
      {
        VMesh::size_type sz = field->num_values();
        for (VMesh::index_type i=0; i < sz; i++)
        {
          Tensor v;
          field->get_value(v,i);
          v = transform*v*transform;
          field->set_value(v,i);
        }
      }    
    }
  }
  
  output->copy_properties(input.get_rep());
  return (true);
}

} // End namespace SCIRunAlgo
