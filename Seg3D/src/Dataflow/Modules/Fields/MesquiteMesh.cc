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
#include <Dataflow/Modules/Fields/MesquiteMesh.h>
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRun {

 // MesquiteMesh constructor 
MesquiteMesh::MesquiteMesh( ProgressReporter* pr, FieldHandle field, 
     std::string isomethod, double isovalue)
{
  //make sure that we are given a field
  if( field.get_rep() == 0 )
  {
    return;
  }

  // Keep a lock on the field until this object is destroyed
  field_handle_ = field;
  // A pointer generates faster access than a handle, so for most calls use
  // this pointer, which is guaranteed because we have a handle as well
  field_ = field->vfield();
  // Similarly it is faster to have a pointer to the mesh than a handle.
  // Hence we maintain the handle.
  mesh_ = field->vmesh();
 
  pr_ = pr;
  
    //get the nodes and elements of this field
  mesh_->size( numNodes );
  mesh_->size( numElements ); 
  mesh_->begin(niterBegin); 
  mesh_->end(niterEnd);
  mesh_->begin(eiterBegin); 
  mesh_->end(eiterEnd);

    //setup some needed information storage vectors for MESQUITE
  count_ = 0;
  anticipated_iterations_ = 3*numNodes;
  
  bytes_.resize(numNodes);
  fixed_.resize(numNodes);  
  size_t i;
  for(i = 0; i < ((size_t) numNodes); ++i )
  {
    bytes_[i] = 0;
    fixed_[i] = false;
  }

  if (field_->basis_order() == 0)
  {
    VMesh::Elem::iterator it, eit;
    VMesh::Node::array_type nodes;
    double val;
    double isoval(isovalue);
    mesh_->begin(it);
    mesh_->end(eit);
    
    if (isomethod == "GreaterThan")
    {    
      while (it != eit)
      {
        field_->get_value(val,*it);
        if (isoval < val)
        {
          mesh_->get_nodes(nodes,*it);
          for (size_t p =0; p<nodes.size();p++)
          {
            fixed_[nodes[p]] = true;
          }
        }
        ++it;
      }
    }
    else if (isomethod == "LessThan")
    {
      while (it != eit)
      {
        field_->get_value(val,*it);
        if (isoval > val)
        {
          mesh_->get_nodes(nodes,*it);
          for (size_t p =0; p<nodes.size();p++)
          {
            fixed_[nodes[p]] = true;
          }
        }
        ++it;
      }      
    }
    else if (isomethod == "Equal")
    {
      while (it != eit)
      {
        field_->get_value(val,*it);
        if (isoval == val)
        {
          mesh_->get_nodes(nodes,*it);
          for (size_t p =0; p<nodes.size();p++)
          {
            fixed_[nodes[p]] = true;
          }
          ++it;
        }
      }
    }
  }
  else if (field_->basis_order() == 1)
  {
    VMesh::Node::iterator it, eit;
    double val;
    double isoval(isovalue);
    mesh_->begin(it);
    mesh_->end(eit);  
  
    if (isomethod == "GreaterThan")
    {
      while (it != eit)
      {
        field_->get_value(val,*it);
        if (isoval > val)
        {
          fixed_[(*it)] = true;
        }
        ++it;
      }        
    }
    else if (isomethod == "LessThan")
    {
      while (it != eit)
      {
        field_->get_value(val,*it);
        if (isoval < val)
        {
          fixed_[(*it)] = true;
        }
        ++it;
      }        
    }
    else if (isomethod == "Equal")
    {
      while (it != eit)
      {
        field_->get_value(val,*it);
        if (isoval == val)
        {
          fixed_[(*it)] = true;
        }
        ++it;
      }    
    }
  }
  
  // Find boundary nodes
  
  VMesh::Elem::iterator eit,eit_end;
  VMesh::Elem::index_type nit;
  VMesh::DElem::array_type delems;
  VMesh::Node::array_type nodes; 
  
  mesh_->synchronize(SCIRun::Mesh::DELEMS_E|SCIRun::Mesh::ELEM_NEIGHBORS_E);
  
  mesh_->begin(eit);
  mesh_->end(eit_end);
  
  while (eit !=eit_end)
  {
    mesh_->get_delems(delems,*eit);
    for (size_t p=0; p<delems.size(); p++)
    {
      if (!(mesh_->get_neighbor(nit,*eit,delems[p])))
      {
        mesh_->get_nodes(nodes,delems[p]);
        for (size_t p=0; p<nodes.size();p++)
        {
          fixed_[nodes[p]] = true;
        }
      }
    }
    ++eit;
  }
  
  FieldInformation fi(field);
  
  triExists=false;
  quadExists=false;
  tetExists=false;
  hexExists=false;

  if(fi.is_tetvolmesh() )
  {
    mesh_->synchronize(SCIRun::Mesh::NODE_NEIGHBORS_E);
    tetExists=true;
    myDimension = 3;
  }
  else if (fi.is_trisurfmesh())
  {
    mesh_->synchronize(SCIRun::Mesh::NODE_NEIGHBORS_E);
    triExists=true;
    myDimension = 2; 
  }
  else if (fi.is_hexvolmesh())
  {  
    // unfortunately in HexVolMesh get_elems has been implemented depending
    // on edges information. We should look at this issue at some point
    mesh_->synchronize(SCIRun::Mesh::EDGES_E); 
    mesh_->synchronize(SCIRun::Mesh::NODE_NEIGHBORS_E);   
    hexExists=true;
    myDimension = 3; 
  }
  else if (fi.is_quadsurfmesh())
  {
    mesh_->synchronize(SCIRun::Mesh::NODE_NEIGHBORS_E);
    quadExists=true;
    myDimension = 2;
  }
  else
  {
    return;
  }
}


  //destructor
MesquiteMesh::~MesquiteMesh()
{
}

  
  /*! We always pass in nodes in with three coordinates.  This may change
    in the future if we want to do smoothing in a parametric space, but
    !for now, we are always in three-dimensions. */
int
MesquiteMesh::get_geometric_dimension(Mesquite::MsqError &/*err*/)
{
  return 3;
}


  //! Returns the number of verticies for the entity.

size_t
MesquiteMesh::get_total_vertex_count(Mesquite::MsqError &/*err*/) const
{
  return (size_t) numNodes;
}

  
  //! Returns the number of elements for the entity.

size_t
MesquiteMesh::get_total_element_count(Mesquite::MsqError &/*err*/) const
{
  return (size_t) numElements;
}


  //! Fills array with handles to all vertices in the mesh.

void
MesquiteMesh::get_all_vertices( vector<Mesquite::Mesh::VertexHandle> &vertices,
                                       Mesquite::MsqError &/*err*/)
{
  vertices.clear();
  VMesh::Node::iterator node_iter = niterBegin;

  while( node_iter != niterEnd )
  {
    unsigned int temp_id = *node_iter;
    VertexHandle temp_v_handle = (void*)temp_id;
    
    vertices.push_back( temp_v_handle );
    ++node_iter;
  }
}


  //! Fills array with handles to all elements in the mesh.

void
MesquiteMesh::get_all_elements( vector<Mesquite::Mesh::ElementHandle> &elements,      
                                       Mesquite::MsqError &/*err*/ )
{
  elements.clear();
  VMesh::Elem::iterator elem_iter = eiterBegin;  

  while( elem_iter != eiterEnd )
  {
    unsigned int temp_id = *elem_iter;
    ElementHandle temp_e_handle = (void*)temp_id;
    
    elements.push_back( temp_e_handle );
    ++elem_iter;
  }
}

  
//! Returns a pointer to an iterator that iterates over the
//! set of all vertices in this mesh.  The calling code should
//! delete the returned iterator when it is finished with it.
//! If vertices are added or removed from the Mesh after obtaining
//! an iterator, the behavior of that iterator is undefined.

Mesquite::VertexIterator*
MesquiteMesh::vertex_iterator(Mesquite::MsqError &/*err*/)
{
  return new MesquiteMesh::VertexIterator(this);
}

  
//! Returns a pointer to an iterator that iterates over the
//! set of all top-level elements in this mesh.  The calling code should
//! delete the returned iterator when it is finished with it.
//! If elements are added or removed from the Mesh after obtaining
//! an iterator, the behavior of that iterator is undefined.

Mesquite::ElementIterator*
MesquiteMesh::element_iterator(Mesquite::MsqError &/*err*/)
{
  return new MesquiteMesh::ElementIterator(this);
}


//! Returns true or false, indicating whether the vertex
//! is allowed to moved.
//! Note that this is a read-only
//! property; this flag can't be modified by users of the
//! Mesquite::Mesh interface.

void
MesquiteMesh::vertices_get_fixed_flag(
  const Mesquite::Mesh::VertexHandle vert_array[], 
  bool fixed_flag_array[],
  size_t num_vtx, 
  Mesquite::MsqError &err )
{
  size_t i;
  for( i = 0; i < num_vtx; ++i )
  {
    unsigned long node_id = (unsigned long)vert_array[i];
    fixed_flag_array[i] = fixed_[node_id];
  }
}

  
//! Get location of a vertex

void
MesquiteMesh::vertices_get_coordinates(
    const Mesquite::Mesh::VertexHandle vert_array[],
    Mesquite::MsqVertex* coordinates,
    size_t num_vtx,
    Mesquite::MsqError &err)
{
  size_t i;
  for( i = 0; i < num_vtx; ++i )
  {
      //set coordinates to the vertex's position.
    Point p;
    VMesh::Node::index_type node_id = (unsigned long) vert_array[i];
    mesh_->get_point( p, node_id );    
    coordinates[i].set( p.x(), p.y(), p.z() );
  }
}

  
//! Set the location of a vertex.

void
MesquiteMesh::vertex_set_coordinates(
    VertexHandle vertex,
    const Mesquite::Vector3D &coordinates,
    Mesquite::MsqError &err)
{
  Point p;
  VMesh::Node::index_type node_id = (unsigned long) vertex;
  mesh_->get_point( p, node_id );
  p.x( coordinates[0] );
  p.y( coordinates[1] );
  p.z( coordinates[2] );
  mesh_->set_point( p, node_id );
  
  update_progress();
}


//! Each vertex has a byte-sized flag that can be used to store
//! flags.  This byte's value is neither set nor used by the mesh
//! implementation.  It is intended to be used by Mesquite algorithms.
//! Until a vertex's byte has been explicitly set, its value is 0.
//! Cubit stores the byte on the TDMesquiteFlag associated with the
//! node.

void
MesquiteMesh::vertex_set_byte( VertexHandle vertex,
                                      unsigned char byte,
                                      Mesquite::MsqError &err)
{
  unsigned long idx = (unsigned long)vertex;
  bytes_[idx] = byte;
}


//! Set the byte for a given array of vertices.

void
MesquiteMesh::vertices_set_byte (
  const VertexHandle *vert_array,
  const unsigned char *byte_array,
  size_t array_size,
  Mesquite::MsqError &err)
{
    //loop over the given vertices and call vertex_set_byte(...).
  size_t i = 0;
  for( i = 0; i < array_size; ++i )
  {
    vertex_set_byte( vert_array[i], byte_array[i], err );
  }
}

  
//! Retrieve the byte value for the specified vertex or vertices.
//! The byte value is 0 if it has not yet been set via one of the
//! *_set_byte() functions.

void
MesquiteMesh::vertex_get_byte(VertexHandle vertex,
                                     unsigned char *byte,
                                     Mesquite::MsqError &err)
{
  unsigned long idx = (unsigned long)vertex;
  *byte = bytes_[idx];
}


//! get the bytes associated with the vertices in a given array.

void
MesquiteMesh::vertices_get_byte(
  const VertexHandle *vertex_array,
  unsigned char *byte_array,
  size_t array_size,
  Mesquite::MsqError &err)
{
    //loop over the given nodes and call vertex_get_byte(...)
  size_t i = 0;
  for( i = 0; i < array_size; ++i )
  {
    vertex_get_byte( vertex_array[i], &byte_array[i], err );
  }
}

  
//! Gets the elements attached to this vertex.

void
MesquiteMesh::vertices_get_attached_elements(
    const VertexHandle* vertex_array,
    size_t num_vertex,
    msq_std::vector<ElementHandle>& elements,
    msq_std::vector<size_t>& offsets,
    Mesquite::MsqError& err )
{
  size_t i = 0, j = 0;
  size_t offset_counter = 0;
  ElementHandle temp_e_handle;

  elements.clear();
  offsets.clear();
  
  for( i = 0; i < num_vertex; ++i )
  {
    offsets.push_back(offset_counter);
    VMesh::Elem::array_type attached_elems;
    VMesh::Node::index_type this_node = (unsigned long)vertex_array[i];

    mesh_->get_elems( attached_elems, this_node );
    for( j = 0; j < attached_elems.size(); ++j ) 
    {
      unsigned int temp_id = attached_elems[j];
      temp_e_handle = (void*)temp_id;
      
      elements.push_back(temp_e_handle);
      ++offset_counter;
    }
  }
  offsets.push_back( offset_counter );
}

  
/*! \brief  
  Returns the vertices that are part of the topological definition of each
  element in the "elem_handles" array.  
  
  When this function is called, the
  following must be true:
  -# "elem_handles" points at an array of "num_elems" element handles.
  -# "vert_handles" points at an array of size "sizeof_vert_handles"
  -# "csr_data" points at an array of size "sizeof_csr_data"
  -# "csr_offsets" points at an array of size "num_elems+1"
      
  When this function returns, adjacency information will be stored
  in csr format:
  -# "vert_handles" stores handles to all vertices found in one
  or more of the elements.  Each vertex appears only
  once in "vert_handles", even if it is in multiple elements.
  -# "sizeof_vert_handles" is set to the number of vertex
  handles placed into "vert_handles".
  -# "sizeof_csr_data" is set to the total number of vertex uses (for
  example, sizeof_csr_data = 6 in the case of 2 TRIANGLES, even if
  the two triangles share some vertices).
  -# "csr_offsets" is filled such that csr_offset[i] indicates the location
  of entity i's first adjacency in "csr_data".  The number of vertices
  in element i is equal to csr_offsets[i+1] - csr_offsets[i].  For this
  reason, csr_offsets[num_elems] is set to the new value of
  "sizeof_csr_data".
  -# "csr_data" stores integer offsets which give the location of
  each adjacency in the "vert_handles" array.

  As an example of how to use this data, you can get the handle of the first
  vertex in element #3 like this:
  \code VertexHandle vh = vert_handles[ csr_data[ csr_offsets[3] ] ] \endcode

  and the second vertex of element #3 like this:
  \code VertexHandle vh = vert_handles[ csr_data[ csr_offsets[3]+1 ] ] \endcode
*/

void
MesquiteMesh::elements_get_attached_vertices(
  const Mesquite::Mesh::ElementHandle *elem_handles,
  size_t num_elems,
  vector<Mesquite::Mesh::VertexHandle>& vert_handles,
  vector<size_t> &offsets,
  Mesquite::MsqError &err)
{  
  vert_handles.clear();
  offsets.clear();

    // Check for zero element case.  
  if( num_elems == 0 )
  {
    return;
  }   
    
    //get a list of all nodes that are in these elements (the elements
    // in the list will not necessarily be unique).
  size_t i, j;
  size_t offset_counter = 0;

  VMesh::Node::array_type nodes;
  
  for( i = 0; i < ((size_t) num_elems); ++i )
  {
    offsets.push_back( offset_counter );
    
    VMesh::Elem::index_type elem_id = (unsigned long)elem_handles[i];
       
    mesh_->get_nodes( nodes, elem_id );
    
    VertexHandle temp_v_handle = NULL;
      //loop over the vertices, to add them to the given array.
    for( j = 0; j < nodes.size(); ++j )
    {
      unsigned int temp_id = nodes[j];
      temp_v_handle = (void*)temp_id;

      vert_handles.push_back( temp_v_handle );
      ++offset_counter;
    }
  }
  offsets.push_back(offset_counter);
}


//! Returns the topologies of the given entities.  The "entity_topologies"
//! array must be at least "num_elements" in size.

void
MesquiteMesh::elements_get_topologies(
  const ElementHandle *element_handle_array,
  Mesquite::EntityTopology *element_topologies,
  size_t num_elements,
  Mesquite::MsqError &err)
{
    //NOTE: this function assumes a homogenous mesh type for the entire mesh.
    //  If hybrid mesh types are allowed, this function will need to be 
    //  modified to remove this assumption...
  
  for ( ; num_elements--; )
  {
    if( tetExists )
    {
      element_topologies[num_elements] = Mesquite::TETRAHEDRON;
    }
    else if( hexExists )
    {
      element_topologies[num_elements] = Mesquite::HEXAHEDRON;
    }
    else if( quadExists )
    {
      element_topologies[num_elements] = Mesquite::QUADRILATERAL;
    }
    else if( triExists )
    {
      element_topologies[num_elements] = Mesquite::TRIANGLE;
    }
    else
    {
      MSQ_SETERR(err)("Type not recognized.", Mesquite::MsqError::UNSUPPORTED_ELEMENT);
      return;
    }
  }
}


//! Tells the mesh that the client is finished with a given
//! entity handle.  

void
MesquiteMesh::release_entity_handles(
  const EntityHandle */*handle_array*/,
  size_t /*num_handles*/,
  Mesquite::MsqError &/*err*/)
{
    // Do nothing...
}
  

//! Instead of deleting a Mesh when you think you are done,
//! call release().  In simple cases, the implementation could
//! just call the destructor.  More sophisticated implementations
//! may want to keep the Mesh object to live longer than Mesquite
//! is using it.

void
MesquiteMesh::release()
{
    // We allocate on the stack, so don't delete this...
}

  //***************   Start of Iterator functions ******************

// ********* VertexIterator functions ********
//constructor

MesquiteMesh::VertexIterator::VertexIterator( MesquiteMesh* mesh_ptr )
{
  meshPtr = mesh_ptr;
  restart();
}


//! Moves the iterator back to the first entity in the list.

void
MesquiteMesh::VertexIterator::restart()
{
  node_iter_ = meshPtr->niterBegin;
}

        
//! *iterator. Return the handle currently being pointed at by the iterator.

Mesquite::Mesh::EntityHandle
MesquiteMesh::VertexIterator::operator*() const
{
  node_iter_t ni = node_iter_;
  unsigned int i = *ni;  
  void *p = (void*)i; 
  if(!is_at_end())
      return reinterpret_cast<Mesquite::Mesh::EntityHandle>(p);
  return 0;
}


//! ++iterator

void
MesquiteMesh::VertexIterator::operator++()
{
  ++node_iter_;
}


//! iterator++

void
MesquiteMesh::VertexIterator::operator++(int)
{
  ++node_iter_;
}


//! Returns false until the iterator has been advanced PAST the last entity.
//! Once is_at_end() returns true, *iterator returns 0.

bool
MesquiteMesh::VertexIterator::is_at_end() const
{
  if( node_iter_ == meshPtr->niterEnd )
  {
    return true;
  }
  return false;
}


// ********* ElementIterator functions ********
//constructor

MesquiteMesh::ElementIterator::ElementIterator( MesquiteMesh* mesh_ptr )
{
  meshPtr=mesh_ptr;
  restart();
}


//! Moves the iterator back to the first entity in the list.

void
MesquiteMesh::ElementIterator::restart()
{
  elem_iter_ = meshPtr->eiterBegin;
}


//! *iterator.  Return the handle currently being pointed at by the iterator.

Mesquite::Mesh::EntityHandle
MesquiteMesh::ElementIterator::operator*() const
{
  elem_iter_t ei = elem_iter_;
  unsigned int i = *ei;  
  void *p = (void*)i; 

  if(!is_at_end())
      return reinterpret_cast<Mesquite::Mesh::EntityHandle>(p);
  return 0;
}


//! ++iterator

void
MesquiteMesh::ElementIterator::operator++()
{
  ++elem_iter_;
}


//! iterator++

void
MesquiteMesh::ElementIterator::operator++(int)
{
  ++elem_iter_;
}


//! Returns false until the iterator has been advanced PAST the last entity.
//! Once is_at_end() returns true, *iterator returns 0.

bool
MesquiteMesh::ElementIterator::is_at_end() const
{
  if( elem_iter_ == meshPtr->eiterEnd )
  {
    return true;
  }
  return false;
}
    
} // end namespace SCIRun

