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

#ifndef DATAFLOW_MODULES_FIELDS_MESQUITE_MESH_H
#define DATAFLOW_MODULES_FIELDS_MESQUITE_MESH_H 1

#include <Core/Datatypes/Field.h>
#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <Core/Datatypes/HexVolMesh.h>
#include <Core/Datatypes/TetVolMesh.h>
#include <MeshInterface.hpp>
#include <MeshImpl.hpp>
#include <MsqError.hpp>
#include <MsqVertex.hpp>

namespace SCIRun {

class MesquiteMesh : public Mesquite::Mesh
{
public:
  MesquiteMesh( ProgressReporter* pr, FieldHandle fieldh, 
       std::string isomethod = "", double isovalue = 0.0 );
  
public:
  virtual ~MesquiteMesh();
  
//************ Operations on entire mesh ****************
    //! Returns whether this mesh lies in a 2D or 3D coordinate system.
  virtual int get_geometric_dimension( Mesquite::MsqError &err );
  
    //! Returns the number of entities of the indicated type.
  virtual size_t get_total_vertex_count( Mesquite::MsqError &err ) const;
  virtual size_t get_total_element_count( Mesquite::MsqError &err ) const;
  
    //! Fills vector with handles to all vertices/elements in the mesh.
  virtual void get_all_vertices(
      vector<Mesquite::Mesh::VertexHandle> &vertices,
    Mesquite::MsqError &err);
  
  virtual void get_all_elements(
    vector<Mesquite::Mesh::ElementHandle> &elements,
    Mesquite::MsqError &err);
  
    //! Returns a pointer to an iterator that iterates over the
    //! set of all vertices in this mesh.  The calling code should
    //! delete the returned iterator when it is finished with it.
    //! If vertices are added or removed from the Mesh after obtaining
    //! an iterator, the behavior of that iterator is undefined.
  virtual Mesquite::VertexIterator* vertex_iterator(
    Mesquite::MsqError &err);
  
    //! Returns a pointer to an iterator that iterates over the
    //! set of all top-level elements in this mesh.  The calling code should
    //! delete the returned iterator when it is finished with it.
    //! If elements are added or removed from the Mesh after obtaining
    //! an iterator, the behavior of that iterator is undefined.
  virtual Mesquite::ElementIterator* element_iterator(
    Mesquite::MsqError &err);
  
  
  virtual void vertices_get_fixed_flag(
    const Mesquite::Mesh::VertexHandle vert_array[], 
    bool fixed_flag_array[],
    size_t num_vtx, 
    Mesquite::MsqError &err );
  
    //! Get/set location of a vertex (vertices)
  virtual void vertices_get_coordinates(
    const Mesquite::Mesh::VertexHandle vert_array[],
    Mesquite::MsqVertex* coordinates,
    size_t num_vtx,
    Mesquite::MsqError &err);
  
  virtual void vertex_set_coordinates(
    Mesquite::Mesh::VertexHandle vertex,
    const Mesquite::Vector3D &coordinates,
    Mesquite::MsqError &err);
  
    //! Each vertex has a byte-sized flag that can be used to store
    //! flags.  This byte's value is neither set nor used by the mesh
    //! implementation.  It is intended to be used by Mesquite algorithms.
    //! Until a vertex's byte has been explicitly set, its value is 0.
  virtual void vertex_set_byte (
    Mesquite::Mesh::VertexHandle vertex,
    unsigned char byte,
    Mesquite::MsqError &err);
  
  virtual void vertices_set_byte (
    const Mesquite::Mesh::VertexHandle *vert_array,
    const unsigned char *byte_array,
    size_t array_size,
    Mesquite::MsqError &err);
  
    //! Retrieve the byte value for the specified vertex or vertices.
    //! The byte value is 0 if it has not yet been set via one of the
    //! *_set_byte() functions.
  virtual void vertex_get_byte(
    const Mesquite::Mesh::VertexHandle vertex,
    unsigned char *byte,
    Mesquite::MsqError &err);
  
  virtual void vertices_get_byte(
    const Mesquite::Mesh::VertexHandle *vertex_array,
    unsigned char *byte_array,
    size_t array_size,
    Mesquite::MsqError &err);
  
// //**************** Vertex Topology *****************    
//     //! Gets the number of elements attached to this vertex.
//     //! Useful to determine how large the "elem_array" parameter
//     //! of the vertex_get_attached_elements() function must be.
    //! Gets the elements attached to this vertex.
  
  virtual void vertices_get_attached_elements(
    const Mesquite::Mesh::VertexHandle* vertex_array,
    size_t num_vertex,
    vector<Mesquite::Mesh::ElementHandle>& elements,
    vector<size_t> &offsets,
    Mesquite::MsqError &err);
 
  virtual void elements_get_attached_vertices(
    const Mesquite::Mesh::ElementHandle *elem_handles,
    size_t num_elems,
    vector<Mesquite::Mesh::VertexHandle>& vert_handles,
    vector<size_t> &offsets,
    Mesquite::MsqError &err);
  
    //! Returns the topologies of the given entities.  The "entity_topologies"
    //! array must be at least "num_elements" in size.
  virtual void elements_get_topologies(
    const Mesquite::Mesh::ElementHandle *element_handle_array,
    Mesquite::EntityTopology *element_topologies,
    size_t num_elements,
    Mesquite::MsqError &err);


    //TAGs (not implemented yet)
  virtual Mesquite::TagHandle tag_create(
    const string& /*tag_name*/,
    Mesquite::Mesh::TagType /*type*/,
    unsigned /*length*/,
    const void* /*default_value*/,
    Mesquite::MsqError &err)
    {MSQ_SETERR(err)("Function not yet implemented.\n",Mesquite::MsqError::NOT_IMPLEMENTED); return NULL;}
  
  virtual void tag_delete(
    Mesquite::TagHandle /*handle*/,
    Mesquite::MsqError& err ) 
    {MSQ_SETERR(err)("Function not yet implemented.\n",Mesquite::MsqError::NOT_IMPLEMENTED);}
  
  virtual Mesquite::TagHandle tag_get(
    const string& /*name*/, 
    Mesquite::MsqError& err ){
    MSQ_SETERR(err)("Function not yet implemented.\n",Mesquite::MsqError::NOT_IMPLEMENTED); return NULL;}
  
  virtual void tag_properties(
    Mesquite::TagHandle /*handle*/,
    string& /*name_out*/,
    Mesquite::Mesh::TagType& /*type_out*/,
    unsigned& /*length_out*/,
    Mesquite::MsqError& err )
    {MSQ_SETERR(err)("Function not yet implemented.\n",Mesquite::MsqError::NOT_IMPLEMENTED);}

  virtual void tag_set_element_data(
    Mesquite::TagHandle /*handle*/,
    size_t /*num_elems*/,
    const Mesquite::Mesh::ElementHandle* /*elem_array*/,
    const void* /*tag_data*/,
    Mesquite::MsqError& err )
    {MSQ_SETERR(err)("Function not yet implemented.\n",Mesquite::MsqError::NOT_IMPLEMENTED);}

   virtual void tag_set_vertex_data (
     Mesquite::TagHandle /*handle*/,
     size_t /*num_elems*/,
     const Mesquite::Mesh::VertexHandle* /*node_array*/,
     const void* /*tag_data*/,
     Mesquite::MsqError& err )
    {MSQ_SETERR(err)("Function not yet implemented.\n",Mesquite::MsqError::NOT_IMPLEMENTED);}
  
  virtual void tag_get_element_data(
    Mesquite::TagHandle /*handle*/,
    size_t /*num_elems*/,
    const Mesquite::Mesh::ElementHandle* /*elem_array*/,
    void* /*tag_data*/,
    Mesquite::MsqError& err )
    {MSQ_SETERR(err)("Function not yet implemented.\n",Mesquite::MsqError::NOT_IMPLEMENTED);}
  
   virtual void tag_get_vertex_data (
     Mesquite::TagHandle /*handle*/,
     size_t /*num_elems*/,
     const Mesquite::Mesh::VertexHandle* /*node_array*/,
     void* /*tag_data*/,
     Mesquite::MsqError& err )
    {MSQ_SETERR(err)("Function not yet implemented.\n",Mesquite::MsqError::NOT_IMPLEMENTED);}
  
    //END TAGS (NOT IMPLEMENTED YET)
  
//**************** Memory Management ****************
    //! Tells the mesh that the client is finished with a given
    //! entity handle.  
  virtual void release_entity_handles(
    const Mesquite::Mesh::EntityHandle *handle_array,
    size_t num_handles,
    Mesquite::MsqError &err);
  
    //! Instead of deleting a Mesh when you think you are done,
    //! call release().  In simple cases, the implementation could
    //! just call the destructor.  More sophisticated implementations
    //! may want to keep the Mesh object to live longer than Mesquite
    //! is using it.
  virtual void release();

  int get_num_nodes()
    {return numNodes;}
  int get_num_elements()
    {return numElements;}
 
  void update_progress()
      {
        double temp;
        count_++;
        if  ( count_%200 == 0 )
        {
          if( count_ > anticipated_iterations_ )
          {
            count_ = 3*anticipated_iterations_ / 4 ;
            temp = 0.75;
          }
          else
          {
            temp = (double)count_/(double)anticipated_iterations_;
          }
          
          pr_->update_progress( temp );
        }   
      }
  
private:
  // Always maintain a lock to the object we are working with so another
  // thread cannot delete it. 
  FieldHandle   field_handle_;
  // Pointer for faster acccess
  VField*        field_;
  // Pointer to the mesh for faster access
  VMesh*         mesh_;
  // Pointer to where error messages should go
  ProgressReporter* pr_;

  int anticipated_iterations_;
  int count_;
  
  vector<unsigned char> bytes_;
  vector<bool> fixed_;
  
  VMesh::Node::iterator niterBegin;
  VMesh::Node::iterator niterEnd;
  VMesh::Elem::iterator eiterBegin;
  VMesh::Elem::iterator eiterEnd;

  VMesh::Node::size_type numNodes;
  VMesh::Elem::size_type numElements;

    //booleans about what element types we have
  bool triExists;
  bool quadExists;
  bool tetExists;
  bool hexExists;
  
  unsigned char myDimension;

    // Iterator definitions
  class VertexIterator : public Mesquite::EntityIterator
  {
  public:
    VertexIterator (MesquiteMesh* mesh_ptr);
    
    virtual ~VertexIterator()
      {}
    
      //! Moves the iterator back to the first
      //! entity in the list.
    virtual void restart();
    
      //! *iterator.  Return the handle currently
      //! being pointed at by the iterator.
    virtual Mesquite::Mesh::EntityHandle operator*() const;
    
      //! ++iterator
    virtual void operator++();
      //! iterator++
    virtual void operator++(int);
    
      //! Returns false until the iterator has
      //! been advanced PAST the last entity.
      //! Once is_at_end() returns true, *iterator
      //! returns 0.
    virtual bool is_at_end() const;

private:

    typedef VMesh::Node::iterator node_iter_t;   
    node_iter_t node_iter_;
    
    MesquiteMesh* meshPtr;
  };
  
  class ElementIterator : public Mesquite::EntityIterator
  {
  public:
    ElementIterator(MesquiteMesh* mesh_ptr);
    
    virtual ~ElementIterator()
      {}
    
      //! Moves the iterator back to the first
      //! entity in the list.
    virtual void restart();
    
      //! *iterator.  Return the handle currently
      //! being pointed at by the iterator.
    virtual Mesquite::Mesh::EntityHandle operator*() const;
    
      //! ++iterator
    virtual void operator++();
      //! iterator++
    virtual void operator++(int);
    
      //! Returns false until the iterator has
      //! been advanced PAST the last entity.
      //! Once is_at_end() returns true, *iterator
      //! returns 0.
    virtual bool is_at_end() const;

  private:
    typedef VMesh::Elem::iterator elem_iter_t;   
    elem_iter_t elem_iter_;

    MesquiteMesh* meshPtr;
  };
};
    
} // end namespace SCIRun

#endif //has file been included
