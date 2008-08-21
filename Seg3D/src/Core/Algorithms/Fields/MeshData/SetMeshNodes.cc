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

#include <Core/Algorithms/Fields/MeshData/SetMeshNodes.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool 
SetMeshNodesAlgo::
run(FieldHandle& input, MatrixHandle& matrix, FieldHandle& output)
				      
{
  algo_start("SetMeshNodes",true);

  if (input.get_rep() == 0)
  {
    algo_end(); error("No input source field");
    return (false);
  }
  
  if (matrix.get_rep() == 0)
  {
    algo_end(); error("No input source matrix");
    return (false);
  }

  if (!(input->has_virtual_interface()))
  {
    algo_end(); error("This algorithm requires a virtual field interface");
    return (false);  
  }

  VMesh::size_type numnodes = input->vmesh()->num_nodes();
  
  // try to see whether the matrix dimensions fit the mesh size
  if (!(matrix->nrows() == numnodes) ||
      !(matrix->ncols() == 3))
  {
    algo_end(); error("Matrix dimensions do not match any of the fields dimensions");
    return (false);
  }

  output = input->clone();
  output->copy_properties(input.get_rep());

  VMesh* mesh = output->vmesh();
  VMesh::size_type size = mesh->num_nodes();
  
  double* dataptr = matrix->get_data_pointer();

  Point p;
  Field::index_type k = 0;
  int cnt =0;
  for (VMesh::Node::index_type i=0; i<size; ++i)
  {
    p.x( dataptr[k  ]);
    p.y( dataptr[k+1]);
    p.z( dataptr[k+2]);
    k += 3;

    mesh->set_point(p,i);
    cnt++; if (cnt == 400) {cnt=0; update_progress(i,size); }
  }

  algo_end(); return (true);
}

} // namespace SCIRunAlgo
