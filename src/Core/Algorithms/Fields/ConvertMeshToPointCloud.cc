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

// Get all the class definitions. 
#include <Core/Algorithms/Fields/ConvertMeshToPointCloud.h>


namespace SCIRunAlgo {

using namespace SCIRun;

// This is the actual algorithm
bool ConvertMeshToPointCloudAlgo::ConvertMeshToPointCloud(ProgressReporter *pr, FieldHandle input, FieldHandle& output,bool datalocation)
{
  
  // check whether we have an input handle
  if (input.get_rep() == 0)
  {
    pr->error("ConvertMeshToPointCloud: No input field");
    return (false);
  }

  // Get the information of the type of the mesh
  FieldInformation fi(input);
  FieldInformation fo(input);

  if (fi.is_nonlinear())
  {
    pr->error("ConvertMeshToPointCloud: This function has not yet been defined for non-linear elements");
    return (false);
  }

  // If output is already a pointcloud, return as we already succeeded.
  if (fo.is_pointcloud())
  {
    output = input;
    return (true);
  }
  
  // Alter type description to become a pointcloud
  fo.make_pointcloudmesh();
  fo.make_constantdata();
  
  // If input is nodata, output should be node data
  if (fi.is_nodata()) fo.make_nodata();
  // If we extract locations of nodes (no data locations), constant data needs
  // to be voided as we cannot store it anywhere
  if (!datalocation && fi.is_constantdata()) fo.make_nodata();
  
  // Create the output field
  output = CreateField(fo);
  
  // If it fails return an error
  if (output.get_rep() == 0)
  {
    pr->error("ConvertMeshToPointCloud: Could not create output field");
    return (false);
  }

  // Get the virtual interface classes
  VField* ifield = input->vfield();
  VMesh*  imesh =  input->vmesh();  
  VField* ofield = output->vfield();
  VMesh*  omesh =  output->vmesh();
  
  if (!datalocation)
  {
    // get the number of nodes in the input mesh
    VMesh::size_type num_nodes = imesh->num_nodes();
    // reserve space to put nodes (performance)
    omesh->reserve_nodes(num_nodes);
    
    // Copy all the nodes
    Point pnt;
    for(VMesh::Node::index_type p=0; p < num_nodes; p++)
    {
      imesh->get_point(pnt,p);
      omesh->add_point(pnt);
    }
  
    // Resize the array that stores the values (we know the number of nodes and
    // elements now).
    ofield->resize_values();

    // If data is linear copy over the data
    if (ifield->is_lineardata())
    {
      // Copy all the values
      ofield->copy_values(ifield);
    }
  }
  else
  {
    if (ifield->basis_order() == 0)
    {
      // Get number of elements
      VMesh::size_type num_elems = imesh->num_elems();
      omesh->reserve_nodes(num_elems);
      
      // Copy over the data locations 
      Point pnt;
      for(VMesh::Elem::index_type p=0; p < num_elems; p++)
      {
        imesh->get_center(pnt,p);
        omesh->add_point(pnt);
      }
      
      // Copy the data values
      ofield->resize_values();
      ofield->copy_values(ifield);    
    }
    else if(ifield->basis_order() == 1)
    {
      // Get number of nodes
      VMesh::size_type num_nodes = imesh->num_nodes();
      omesh->reserve_nodes(num_nodes);
      
      // Copy over the data locations 
      Point pnt;
      for(VMesh::Node::index_type p=0; p < num_nodes; p++)
      {
        imesh->get_point(pnt,p);
        omesh->add_point(pnt);
      }
      
      // Copy the data values
      ofield->resize_values();
      ofield->copy_values(ifield);        
    }
  }

  return (true);
}

} // End namespace SCIRunAlgo

