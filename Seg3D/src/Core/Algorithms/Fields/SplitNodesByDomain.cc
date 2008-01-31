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

#include <Core/Algorithms/Fields/SplitNodesByDomain.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool SplitNodesByDomainAlgo::split_nodes_by_domain( FieldHandle input, FieldHandle& output)
{
  if (input.get_rep() == 0)
  {
    error("SplitNodesByDomain: No input field");
    return (false);
  }

  FieldInformation fi(input), fo(input); 
  if (fi.is_nonlinear())
  {
    error("SplitNodesByDomain: This function has not yet been defined for non-linear elements.");
    return (false);
  }

  if (!(fi.is_constantdata()))
  {
    error("SplitNodesByDomain: This function only works for data located at the elements");
    return (false);
  }

  if (!(fi.is_unstructuredmesh()))
  {
    error("SplitNodesByDomain: This function only works for unstructured meshes");
    return (false);
  }

  fo.make_unstructuredmesh();

  VField *field = input->vfield();
  VMesh  *mesh  = input->vmesh();
  
  if (field == 0 || mesh == 0)
  {
    error("SplitNodesByDomain: No input field");
    return (false);
  }

  VMesh::Elem::iterator bei, eei;

  VMesh::Node::iterator bni, eni;
  
  VMesh::size_type num_elems = mesh->num_elems();
  VMesh::size_type num_nodes = mesh->num_nodes();


  std::vector<VMesh::Node::index_type> idxarray(num_nodes);
  std::vector<bool> newidxarray(num_nodes);
  VMesh::Node::array_type nodes;
  VMesh::Node::array_type newnodes;
    
  double val, minval;
  double eval;
  Field::index_type idx;


  output = CreateField(fo);
  if (output.get_rep() == 0)
  {
    error("SplitNodesByDomain: Could not create output field");
    return(false); 
  }
 
  VField* ofield = output->vfield();
  VMesh *omesh = output->vmesh();

  if (ofield == 0 || omesh == 0)
  {
    error("SplitNodesByDomain: Could not create output field");
    return(false); 
  }

  std::vector<double> newdata(num_elems);
  
  omesh->elem_reserve(num_elems); // exact number
  omesh->node_reserve(num_nodes); // minimum number of nodes
  
  mesh->begin(bei);
  mesh->end(eei);
  
  if (bei != eei) field->get_value(minval,*(bei));
  while (bei != eei)  
  {
    field->get_value(val,*(bei));
    if (val < minval) minval = val;
    ++bei; 
  }
  
  Field::index_type k = 0;
  
  while(1)
  {
    val = minval;
  
    for (size_type p =0; p<num_nodes; p++) newidxarray[p] = true;

    mesh->begin(bei); mesh->end(eei);
    mesh->get_nodes(newnodes,*(bei));

    while (bei != eei)
    {
      field->get_value(eval,*(bei));
      if (eval == val)
      {
        mesh->get_nodes(nodes,*(bei));
        for (size_t p=0; p< nodes.size(); p++)
        {
          idx = nodes[p];
          if (newidxarray[idx])
          {
            Point pt;
            mesh->get_center(pt,nodes[p]);
            idxarray[idx] = omesh->add_point(pt);
            newidxarray[idx] = false;
          }
          newnodes[p] = idxarray[idx];
        }
        omesh->add_elem(newnodes);
        newdata[k] = eval; k++;
      }
      ++bei;
    }

    eval = val;
    bool foundminval = false;
    
    mesh->begin(bei);
    mesh->end(eei);
    while (bei != eei)
    {
      field->value(eval,*(bei));
      if (eval > val)
      {
        if (foundminval)
        {
          if (eval < minval) minval = eval;
        }
        else
        {
          minval = eval;
          foundminval = true;
        }
      }
      ++bei;
    }


    if (minval > val)
    {
      val = minval;
    }
    else
    {
      break;
    }
  }
  
  omesh->begin(bei); omesh->end(eei);
  ofield->resize_values();
  k = 0;
  while (bei != eei)
  {
    ofield->set_value(newdata[k],(*bei));
    ++bei;
    k++;
  }
  
  return(true);
}

} // namespace SCIRunAlgo

