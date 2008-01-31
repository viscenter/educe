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

#include <Core/Algorithms/Fields/SetFieldNodes.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool SetFieldNodesAlgo::SetFieldNodes(ProgressReporter *pr,
				      FieldHandle& field_input_handle,
				      FieldHandle& field_output_handle,
				      MatrixHandle& matrix_input_handle)
{
  if (field_input_handle.get_rep() == 0)
  {
    pr->error("SetFieldNodes: No input source field");
    return (false);
  }
  
  if (matrix_input_handle.get_rep() == 0)
  {
    pr->error("SetFieldNodes: No input source matrix");
    return (false);
  }

  if (!(field_input_handle->has_virtual_interface()))
  {
    pr->error("SetFieldNodes: This algorithm requires a virtual field interface");
    return (false);  
  }

  // Get the virtual interface
  VMesh* imesh = field_input_handle->vmesh();
  VMesh::size_type numnodes = imesh->num_nodes();
  
  // try to see whether the matrix dimensions fit the mesh size
  if (!matrix_input_handle->nrows() == numnodes ||
      !matrix_input_handle->ncols() == 3)
  {
    pr->error("SetFieldNodes: Matrix dimensions do not match any of the fields dimensions");
    return (false);
  }

  field_output_handle = field_input_handle->clone();
  field_output_handle->thaw();
  field_output_handle->copy_properties(field_input_handle.get_rep());

  if ((field_output_handle.get_rep() == 0) ||
      (!(field_output_handle->has_virtual_interface())))
  {
    pr->error("SetFieldData: Could not create output field and output interface");
    return (false);  
  }  


  VMesh* mesh = field_output_handle->vmesh();
  VMesh::size_type size = mesh->num_nodes();
  
  double* dataptr = matrix_input_handle->get_data_pointer();

  Point p;
  int k = 0;
  for (VMesh::Node::index_type i=0; i<size; ++i)
  {
    p.x( dataptr[k  ]);
    p.y( dataptr[k+1]);
    p.z( dataptr[k+2]);
    k += 3;

    mesh->set_point(p,i);
  }

  return (true);
}

} // namespace SCIRunAlgo
