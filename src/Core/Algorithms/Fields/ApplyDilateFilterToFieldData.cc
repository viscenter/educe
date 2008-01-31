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

#include <Core/Algorithms/Fields/ApplyDilateFilterToFieldData.h>

namespace SCIRunAlgo {

using namespace SCIRun;

template<class DATA> 
bool ApplyDilateFilterToFieldDataNodeV(ProgressReporter *pr, FieldHandle input, 
                    FieldHandle& output, FieldsAlgo::FilterSettings& settings);

template<class DATA> 
bool ApplyDilateFilterToFieldDataElemV(ProgressReporter *pr, FieldHandle input, 
                    FieldHandle& output, FieldsAlgo::FilterSettings& settings);



bool ApplyDilateFilterToFieldDataAlgo::ApplyDilateFilterToFieldData(ProgressReporter *pr, FieldHandle input, FieldHandle& output, FieldsAlgo::FilterSettings& settings)
{
  // Check whether we have an input field
  if (input.get_rep() == 0)
  {
    pr->error("ApplyDilateFilterToFieldData: No input field");
    return (false);
  }

  if (!(input->has_virtual_interface()))
  {
    pr->error("ApplyDilateFilterToFieldData: This algorithm requires a virtual field interface");
    return (false);  
  }

  // Figure out what the input type and output type have to be
  FieldInformation fi(input);
  
  if (fi.is_nonlinear())
  {
    pr->error("ApplyDilateFilterToFieldData: This function has not yet been defined for non-linear elements");
    return (false);
  }
  
  if (fi.is_nodata())
  {
    pr->error("ApplyDilateFilterToFieldData: There is no data defined in the input field");
    return (false);
  }
  
  if (!fi.is_scalar())
  {
    pr->error("ApplyDilateFilterToFieldData: The field data is not scalar data");
    return (false);  
  }
  
  if (!fi.has_virtual_interface())
  {
    pr->error("ApplyDilateFilterToFieldData: This algorithm requires the field to have a virtual interface");
    return (false);
  }
  
  if (fi.is_constantdata())
  {
    if (UseIntegerInterface(fi)) return (ApplyDilateFilterToFieldDataElemV<int>(pr,input,output,settings));
    if (UseScalarInterface(fi)) return (ApplyDilateFilterToFieldDataElemV<double>(pr,input,output,settings));
  }
  else if (fi.is_lineardata())
  {
    if (UseIntegerInterface(fi)) return (ApplyDilateFilterToFieldDataNodeV<int>(pr,input,output,settings));
    if (UseScalarInterface(fi)) return (ApplyDilateFilterToFieldDataNodeV<double>(pr,input,output,settings));  
  }

  return (false);
}


template <class DATA>
bool ApplyDilateFilterToFieldDataNodeV(ProgressReporter *pr, FieldHandle input, 
                      FieldHandle& output, FieldsAlgo::FilterSettings& settings)
{
  int n = settings.num_iterations_;

  output = input;
  output.detach();
  VMesh*  mesh = output->vmesh();
  mesh->synchronize(Mesh::NODE_NEIGHBORS_E|Mesh::FACES_E);
  input.detach();

  for (int p=0; p <n; p++)
  {
    VField* ifield = input->vfield();
    VField* ofield = output->vfield();
    
    VMesh::Node::size_type sz;  
    mesh->size(sz);

    VMesh::Node::array_type nodes;
    DATA val, nval;
    
    for(VMesh::Node::index_type i=0; i<sz; ++i)
    {
      mesh->get_neighbors(nodes,i);
      ifield->get_value(val,i);
      
      for (size_t j=0; j<nodes.size(); j++)
      {
        ifield->get_value(nval,nodes[j]);
        if (nval > val) val = nval;
      }
      ofield->set_value(val,i);
    }

    ifield->copy_values(ofield);
  }
  
  return (true);
}


template <class DATA>
bool ApplyDilateFilterToFieldDataElemV(ProgressReporter *pr, FieldHandle input, 
                      FieldHandle& output, FieldsAlgo::FilterSettings& settings)
{
  int n = settings.num_iterations_;

  output = input;
  output.detach();
  VMesh*  mesh = input->vmesh();
  mesh->synchronize(Mesh::ELEM_NEIGHBORS_E|Mesh::FACES_E);

  input.detach();

  for (int p=0; p <n; p++)
  {

    VField* ifield = input->vfield();
    VField* ofield = output->vfield();
    
    VMesh::Elem::size_type sz;  
    mesh->size(sz);

    VMesh::Elem::array_type nodes;
    DATA val, nval;
    
    for(VMesh::Elem::index_type i=0; i<sz; ++i)
    {
      mesh->get_neighbors(nodes,i);
      ifield->get_value(val,i);
      
      for (size_t j=0; j<nodes.size(); j++)
      {
        ifield->get_value(nval,nodes[j]);
        if (nval > val) val = nval;
      }
      ofield->set_value(val,i);
    }

    ifield->copy_values(ofield);
  }
  
  return (true);
}


} // End namespace SCIRunAlgo
