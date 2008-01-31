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


/*
 *  CollectPointClouds.cc:  Collect a (time) series of point clouds
 *  turing them into a set of curves.
 *
 *  Written by:
 *   Allen R. Sanderson
 *   SCI Institute
 *   University of Utah
 *   July 2007
 *
 *  Copyright (C) 2007 SCI Institute
 */

#if !defined(CollectPointClouds_h)
#define CollectPointClouds_h

#include <Core/Algorithms/Util/DynamicAlgo.h>
#include <sci_hash_map.h>

//! for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRun {

class SCISHARE CollectPointCloudsAlgo : public DynamicAlgoBase
{
public:
  virtual bool CollectPointClouds(ProgressReporter *pr,
				  FieldHandle& field_input_handle,
				  FieldHandle& field_output_handle,
				  int num_fields,
				  int num_nodes,
				  unsigned int& count);
};


template <class IFIELD, class OFIELD>
class CollectPointCloudsAlgoT : public CollectPointCloudsAlgo
{
public:
  virtual bool CollectPointClouds(ProgressReporter *pr,
				  FieldHandle& field_input_handle,
				  FieldHandle& field_output_handle,
				  int num_fields,
				  int num_nodes,
				  unsigned int& count);
};


// Since the algorithm is templated, we need to define its implemenation in the
// header file.

template <class IFIELD, class OFIELD>
bool CollectPointCloudsAlgoT<IFIELD, OFIELD>::
CollectPointClouds(ProgressReporter *pr,
		   FieldHandle& field_input_handle,
		   FieldHandle& field_output_handle,
		   int num_fields,
		   int num_nodes,
		   unsigned int& count)
{
  IFIELD *iField = dynamic_cast<IFIELD *>(field_input_handle.get_rep());

  if (iField == 0)
  {
    pr->error("Could not obtain input field");
    return (false);
  }

  typename IFIELD::mesh_handle_type iMesh = iField->get_typed_mesh();
  if (iMesh == 0)
  {
    pr->error("No mesh associated with the input field");
    return (false);
  }
  
  // Get the number of nodes in the input mesh
  typename IFIELD::mesh_type::Node::size_type inumnodes;
  iMesh->size(inumnodes);
  
  if( inumnodes != num_nodes )
  {
    pr->error("The number of nodes does not match the current node list");
    return(false);
  }

  iMesh->synchronize(Mesh::NODES_E);

  OFIELD *oField;
  typename OFIELD::mesh_handle_type oMesh;

  if( count && field_output_handle.get_rep())
  {
    oField = dynamic_cast<OFIELD *> (field_output_handle->clone());
    oField->thaw();

    field_output_handle = oField;

    oMesh = oField->get_typed_mesh();

    if (oMesh == 0)
    {
      pr->error("No mesh associated with the output field");
      return (false);
    }
  }
  else
  {
    oMesh = scinew typename OFIELD::mesh_type();
    
    if (oMesh == 0)
    {
      pr->error("Could not create the output mesh");
      return (false);
    }
  }

  // Check to see if nodes and edges need to be removed and data advanced.
  if( count >= num_fields )
  {
    while( count >= num_fields )
    {
      oMesh->delete_nodes(0, inumnodes);

      // data access methods are not editable except for resizing.
      typename OFIELD::fdata_type::iterator bod = oField->fdata().begin();
      typename OFIELD::fdata_type::iterator nod = bod + inumnodes;
      typename OFIELD::fdata_type::iterator eod = oField->fdata().end() -
	inumnodes;
    
      // Move the data one step to the front - this is done because the
      while (bod != eod)
      {
	*bod = *nod;
	++bod;
	++nod;
      }

      if( count > 1 )
      {
	oMesh->delete_edges( (typename IFIELD::mesh_type::Edge::index_type) 
			     (count-2)*inumnodes,
			     (typename IFIELD::mesh_type::Edge::index_type) 
			     (count-1)*inumnodes);
      }

      --count;
    }
  }

  // Define iterators over the nodes
  typename IFIELD::mesh_type::Node::iterator bn, en;
  typename IFIELD::mesh_type::Node::index_type current, last;
  iMesh->begin(bn); // get begin iterator
  iMesh->end(en);   // get end iterator

  // Iterate over all nodes and add each node to the curve.
  while (bn != en)
  {
    Point point;
    iMesh->get_center(point, *bn);
    current = oMesh->add_node(point);
    ++bn;

    if( count && count < num_fields )
    {
      last = current - inumnodes;
      oMesh->add_edge( last, current );
    }
  }

  // Have the new mesh now make a new field if needed.
  if( count == 0 )
  {
    oField = scinew OFIELD(oMesh);

    if (oField == 0)
    {
      pr->error("Could not create the output field");
      return (false);  
    }

    field_output_handle = oField;
    field_output_handle->copy_properties(field_input_handle.get_rep());
  }

  // Make sure fdata matches the size of the number of nodes
  oField->resize_fdata();

  // Add the new data to the back
  typename OFIELD::fdata_type::iterator bod = oField->fdata().begin() +
    (count * inumnodes);
  typename IFIELD::fdata_type::iterator bid = iField->fdata().begin();
  typename IFIELD::fdata_type::iterator eid = iField->fdata().end();

  while (bid != eid)
  {
    *bod = *bid;
    ++bod;
    ++bid;
  }

  ++count;

  // Success:
  return (true);
}

} // end namespace SCIRunAlgo

#endif 

