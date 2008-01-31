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

#include <Core/Algorithms/Fields/GetFieldNodes.h>
#include <Core/Datatypes/DenseMatrix.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool GetFieldNodesAlgo::GetFieldNodes(ProgressReporter *pr,
				      FieldHandle& field_input_handle,
				      MatrixHandle& matrix_output_handle)
{

  if (field_input_handle.get_rep() == 0)
  {
    pr->error("GetFieldNodes: No input source field");
    return (false);
  }
  
  if (!(field_input_handle->has_virtual_interface()))
  {
    pr->error("GetFieldNodes: This algorithm requires a virtual field interface");
    return (false);  
  }
  
  VMesh* mesh = field_input_handle->vmesh();
  VMesh::size_type size = mesh->num_nodes();
  
  matrix_output_handle = scinew DenseMatrix(size,3);

  if (matrix_output_handle.get_rep() == 0)
  {
    pr->error("GetFieldNodes: Could not allocate output matrix");
    return (false);
  }

  double* dataptr = matrix_output_handle->get_data_pointer();

  Point p;
  int k = 0;
  for (VMesh::Node::index_type i=0; i<size; ++i)
  {
    mesh->get_center(p,i);
    dataptr[k] = p.x();
    dataptr[k+1] = p.y();
    dataptr[k+2] = p.z();
    k += 3;
  }
  return (true);
}

} // namespace SCIRunAlgo
