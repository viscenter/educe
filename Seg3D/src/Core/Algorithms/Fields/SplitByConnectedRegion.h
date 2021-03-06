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

#ifndef CORE_ALGORITHMS_FIELDS_SPLITBYCONNECTEDREGION_H
#define CORE_ALGORITHMS_FIELDS_SPLITBYCONNECTEDREGION_H 1

// The following include file will include all tools needed for doing 
// dynamic compilation and will include all the standard dataflow types
#include <Core/Algorithms/Util/DynamicAlgo.h>

//! for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SCISHARE SplitByConnectedRegionAlgo : public DynamicAlgoBase
{
public:
  virtual bool SplitByConnectedRegion(ProgressReporter *pr, FieldHandle input, std::vector<FieldHandle>& output);
};

template <class FSRC>
class SplitByConnectedRegionAlgoT : public SplitByConnectedRegionAlgo
{
public:
  virtual bool SplitByConnectedRegion(ProgressReporter *pr, FieldHandle input, std::vector<FieldHandle>& output);
};


template <class FSRC>
bool SplitByConnectedRegionAlgoT<FSRC>::SplitByConnectedRegion(ProgressReporter *pr, FieldHandle input, std::vector<FieldHandle>& output)
{

  FSRC *ifield = dynamic_cast<FSRC *>(input.get_rep());
  if (ifield == 0)
  { 
    pr->error("SplitByConnectedRegion: Could not obtain input field");
    return (false);
  }

  typename FSRC::mesh_handle_type imesh = ifield->get_typed_mesh();
  if (imesh == 0)
  {
    pr->error("SplitByConnectedRegion: No mesh associated with input field");
    return (false);
  }

  Field::index_type k = 0;

  typename FSRC::mesh_type::Node::size_type numnodes; 
  typename FSRC::mesh_type::Elem::size_type numelems; 
  imesh->size(numnodes);
  imesh->size(numelems);
  
  typename FSRC::mesh_type::Elem::iterator bi, ei;
  imesh->begin(bi); imesh->end(ei);

  Field::size_type surfsize = static_cast<Field::size_type>(pow(numelems, 2.0 / 3.0));
  vector<typename FSRC::mesh_type::Elem::index_type> buffer(0);
  buffer.reserve(surfsize);
  
  imesh->synchronize(Mesh::ELEM_NEIGHBORS_E|Mesh::DELEMS_E); 

  vector<Field::index_type> elemmap(numelems, 0);
  vector<Field::index_type> nodemap(numnodes, 0);
  vector<Field::index_type> renumber(numnodes,0);
  vector<short> visited(numelems, 0);

  typename FSRC::mesh_type::Node::array_type nnodes;
  typename FSRC::mesh_type::Elem::array_type neighbors;

  while (bi != ei)
  {
    // if list of elements to process is empty ad the next one
    if (buffer.size() == 0)
    {
      if(visited[*bi] == 0) { buffer.push_back(*bi); k++; }
    }
    
    if (buffer.size() > 0)
    {
      for (size_t i=0; i< buffer.size(); i++)
      {
        Field::index_type j = static_cast<Field::index_type>(buffer[i]);
        if (visited[j] > 0) { continue; }
        visited[j] = 1;
        
        imesh->get_nodes(nnodes,buffer[i]);
        for (size_t q=0; q<nnodes.size(); q++)
        {
          imesh->get_elems(neighbors,nnodes[q]);
          for (size_t p=0; p<neighbors.size(); p++)
          {
            if(visited[static_cast<unsigned int>(neighbors[p])] == 0)
            {
              buffer.push_back(neighbors[p]);
              visited[static_cast<unsigned int>(neighbors[p])] = -1;            
            }
          }
        }
         
        if (j >= elemmap.size()) elemmap.resize(j+1);
        
        elemmap[j] = k;
        for (int p=0;p<nnodes.size();p++)
        {
          if (nnodes[p] >= static_cast<Field::index_type>(nodemap.size())) 
            nodemap.resize(static_cast<size_t>(nnodes[p])+1);      
          nodemap[nnodes[p]] = k;
        }
      }
      buffer.clear();
    }
    ++bi;
  }

  
  output.resize(k);
  for (int p=0;p<k;p++)
  {
    FSRC* ofield;
    typename FSRC::mesh_handle_type omesh;

    Field::size_type nn = 0;
    Field::size_type ne = 0;
    
    for (Field::index_type q=0;q<numnodes;q++) if (nodemap[q] == p+1) nn++;
    for (Field::index_type q=0;q<numelems;q++) if (elemmap[q] == p+1) ne++;
    
    omesh = scinew typename FSRC::mesh_type();
    if (omesh == 0)
    {
      pr->error("SplitByConnectedRegion: Could not create output field");
      return (false);
    }

    omesh->node_reserve(nn);
    omesh->elem_reserve(ne);

    ofield = scinew FSRC(omesh);
    if (ofield == 0)
    {
      pr->error("SplitByConnectedRegion: Could not create output field");
      return (false);  
    }      
    
    output[p] = dynamic_cast<Field*>(ofield);

    Point point;
    for (int q=0;q<numnodes;q++) 
    {  
      if (nodemap[q] == p+1) 
      {
        imesh->get_center(point,static_cast<typename FSRC::mesh_type::Node::index_type>(q));
        renumber[q] = static_cast<unsigned int>(omesh->add_point(point));
      }
    }
    
    typename FSRC::mesh_type::Node::array_type elemnodes;
    for (int q=0;q<numelems;q++) 
    {  
      if (elemmap[q] == p+1) 
      {   
        imesh->get_nodes(elemnodes, static_cast<typename FSRC::mesh_type::Elem::index_type>(q));
        for (size_t r=0; r< elemnodes.size(); r++)
        {
          elemnodes[r] = static_cast<typename FSRC::mesh_type::Node::index_type>(renumber[elemnodes[r]]);
        }
        omesh->add_elem(elemnodes);
      }
    }
    
    ofield->resize_fdata();
    typename FSRC::value_type val;
    typename FSRC::mesh_type::Elem::iterator ei;
    typename FSRC::mesh_type::Node::iterator ni;


    if (ifield->basis_order() == 1)
    {
      omesh->begin(ni);
      for (Field::index_type q=0;q<numnodes;q++) 
      {  
        if (nodemap[q] == p+1) 
        {
          ifield->value(val,static_cast<typename FSRC::mesh_type::Node::index_type>(q));
          ofield->set_value(val,(*ni));
          ++ni;
        }
      }
    }
    
    if (ifield->basis_order() == 0)
    {
      omesh->begin(ei);    
      for (Field::index_type q=0;q<numelems;q++) 
      {  
        if (elemmap[q] == p+1) 
        {   
          ifield->value(val,static_cast<typename FSRC::mesh_type::Elem::index_type>(q));
          ofield->set_value(val,(*ei));
          ++ei;
        }
      }    
    }

    output[p]->copy_properties(input.get_rep());
  }
  
  // Success:
  return (true);
}

} // end namespace SCIRunAlgo

#endif 

