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

#include <Core/Algorithms/Fields/CalculateIsInsideField.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool CalculateIsInsideFieldAlgo::CalculateIsInsideField(ProgressReporter *pr, 
        FieldHandle input, FieldHandle& output, FieldHandle objfield, 
        double newval, double defval, std::string output_type)
{
  if (input.get_rep() == 0)
  {
    pr->error("CalculateIsInsideField: No input field");
    return (false);
  }

  if (objfield.get_rep() == 0)
  {
    pr->error("CalculateIsInsideField: No object field");
    return (false);
  }

  // no precompiled version available, so compile one

  FieldInformation fi(input);
  FieldInformation fo(input);
  FieldInformation fobj(objfield);
  
  if ((!fi.has_virtual_interface())||(!fobj.has_virtual_interface()))
  {
    pr->error("CalculateIsInsideField: This algorithm requires both fields to have a virtual interface");
    return (false);
  }
  
  if (fi.is_nonlinear())
  {
    pr->error("CalculateIsInsideField: This function has not yet been defined for non-linear elements");
    return (false);
  }
    
  if (output_type != "same as input")
  {  
    fo.set_data_type(output_type);
  }
 
  if (fo.is_vector()) fo.make_double();
  if (fo.is_tensor()) fo.make_double();
  fo.make_constantdata();
  
  
  bool clean_field = false;
  if (output.get_rep() == 0)
  {
    output = CreateField(fo,input->mesh());
    
    if (output.get_rep() == 0)
    {
      pr->error("CalculateIsInsideField: Could not create output field");
      return(false);  
    }
    clean_field = true;
  }
  
  // For the moment we calculate everything in doubles
  
  VMesh* omesh = output->vmesh();
  VMesh* objmesh = objfield->vmesh();
  VField* ofield = output->vfield();

  VMesh::size_type vsz = ofield->num_values();
  if (clean_field == true)
  {
    for(VMesh::index_type i=0; i<vsz; i++) ofield->set_value(defval,i);
  }
  
  objmesh->synchronize(Mesh::LOCATE_E);
  omesh->synchronize(Mesh::LOCATE_E);

  VMesh::Elem::size_type sz;
  VMesh::Node::size_type nsz;
  omesh->size(sz);
  
  VMesh::Node::array_type nodes;
  VMesh::Elem::index_type cidx;

  for(VMesh::Elem::index_type i=0; i<sz;i++)
  {
    Point p;
    bool is_inside = false;  
    
    omesh->get_nodes(nodes,i);
    
    for (size_t r=0; r< nodes.size(); r++)
    {
      omesh->get_center(p,nodes[r]);
      
      if (objmesh->locate(cidx,p))
      {
        is_inside = true;
        break;
      }
    }

    omesh->get_center(p,i);
    if (objmesh->locate(cidx,p)) is_inside = true;
    
    if (is_inside) ofield->set_value(newval,i);
  }

  objmesh->size(nsz);
  objmesh->size(sz);
  
  for(VMesh::Node::index_type i=0; i<nsz;i++)
  {
    Point p;
    objmesh->get_center(p,i);
    if (omesh->locate(cidx,p)) ofield->set_value(newval,cidx);
  }

  for(VMesh::Elem::index_type i=0; i<sz;i++)
  {
    Point p;
    objmesh->get_center(p,i);
    if (omesh->locate(cidx,p)) ofield->set_value(newval,cidx);
  }
  
  return (true);  
}

} // End namespace SCIRunAlgo
