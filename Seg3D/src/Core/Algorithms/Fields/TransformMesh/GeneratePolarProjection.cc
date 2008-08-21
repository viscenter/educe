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

#include <Core/Algorithms/Fields/TransformMesh/GeneratePolarProjection.h>

#include <Core/Geometry/Transform.h>

#include <Core/Datatypes/VField.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/FieldInformation.h>


namespace SCIRunAlgo {

using namespace SCIRun;

bool 
GeneratePolarProjectionAlgo::
run(FieldHandle input, FieldHandle& output)
{
  // Mark that we are starting the algorithm, but do not report progress
  algo_start("GeneratePolarProjection",false);
  
  // Handle: the function get_rep() returns the pointer contained in the handle
  if (input.get_rep() == 0)
  {
    // If we encounter a null pointer we return an error message and return to
    // the program to deal with this error. 
    error("No input field");
    algo_end(); return (false);
  }

  if (!(input->has_virtual_interface()))
  {
    error("This algorithm needs a virtual interface to function");
    algo_end(); return (false);
  }

  FieldInformation fi(input);
  fi.make_unstructuredmesh();
  
  output = CreateField(fi);

  Transform t;
  
  Point origin = get_point("origin");
  Vector axis = get_vector("direction");

  axis.normalize();
  
  Vector v1, v2;
  axis.find_orthogonal(v1,v2);
  v1.normalize();
  v2.normalize();
  t.load_basis(origin,v1,v2,axis);
  
  VMesh* omesh = output->vmesh();
  VMesh* imesh = input->vmesh();
  VMesh::size_type num_nodes = imesh->num_nodes();

  Point p;
  
  for (VMesh::Node::index_type idx=0; idx<num_nodes; idx++)
  {
    imesh->get_center(p,idx);
    p = t.unproject(p);
    
    Vector d(p.x(),p.y(),0.0); d.normalize();
    double len = p.asVector().length();
    Point r(d.x()*len,d.y()*len,0.0);
    
    r = t.project(r);
    omesh->add_point(r);
  }
    
  omesh->resize_elems(imesh->num_elems());    
  omesh->copy_elems(imesh);  
        
  VField* ifield = input->vfield();
  VField* ofield = output->vfield();
  
  ofield->resize_values();
  ofield->copy_values(ifield);
        
  //! Copy properties of the property manager
	output->copy_properties(input.get_rep());
   
  // Success:
  algo_end(); return (true);
}

} // End namespace SCIRunAlgo
