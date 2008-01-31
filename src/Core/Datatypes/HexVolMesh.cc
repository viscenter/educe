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

#include <Core/Datatypes/HexVolMesh.h>
#include <Core/Datatypes/VUnstructuredMesh.h>

//! Only include this class if we included HexVol Support
#if (SCIRUN_HEXVOL_SUPPORT > 0)

namespace SCIRun {

//! This is the virtual interface to the curve mesh
//! This class lives besides the real mesh class for now and solely profides
//! an interface. In the future however when dynamic compilation is gone
//! this should be put into the HexVolMesh class.
template<class MESH> class VHexVolMesh;

//! This class is not exposed to the general interface and only the VMesh class
//! is accessed by other classes.
template<class MESH>
class VHexVolMesh : public VUnstructuredMesh<MESH> {
public:
  virtual bool is_hexvolmesh()         { return (true); }

  //! constructor and descructor
  VHexVolMesh(MESH* mesh) : VUnstructuredMesh<MESH>(mesh) {}
  virtual ~VHexVolMesh() {}
    
  virtual void get_nodes(VMesh::Node::array_type& nodes, 
                         VMesh::Edge::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes, 
                         VMesh::Face::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes,
                         VMesh::Cell::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes,
                         VMesh::Elem::index_type i) const;
  virtual void get_nodes(VMesh::Node::array_type& nodes, 
                         VMesh::DElem::index_type i) const;

  virtual void get_enodes(VMesh::ENode::array_type& edges,
                          VMesh::Edge::index_type i) const;
  virtual void get_enodes(VMesh::ENode::array_type& edges,
                          VMesh::Face::index_type i) const;
  virtual void get_enodes(VMesh::ENode::array_type& edges, 
                          VMesh::Cell::index_type i) const;
  virtual void get_enodes(VMesh::ENode::array_type& edges, 
                          VMesh::Elem::index_type i) const;
  virtual void get_enodes(VMesh::ENode::array_type& edges, 
                          VMesh::DElem::index_type i) const;
                                            
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::Face::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::Cell::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::Elem::index_type i) const;
  virtual void get_edges(VMesh::Edge::array_type& edges,
                         VMesh::DElem::index_type i) const;

  virtual void get_faces(VMesh::Face::array_type& faces,
                         VMesh::Cell::index_type i) const;
  virtual void get_faces(VMesh::Face::array_type& faces,
                         VMesh::Elem::index_type i) const;
  virtual void get_faces(VMesh::Face::array_type& faces,
                         VMesh::DElem::index_type i) const;

  virtual void get_cells(VMesh::Cell::array_type& cells, 
                         VMesh::Node::index_type i) const;  
  virtual void get_cells(VMesh::Cell::array_type& cells,
                         VMesh::Edge::index_type i) const;  
  virtual void get_cells(VMesh::Cell::array_type& cells,
                         VMesh::Face::index_type i) const;  
  virtual void get_cells(VMesh::Cell::array_type& cells, 
                         VMesh::Elem::index_type i) const;  
  virtual void get_cells(VMesh::Cell::array_type& cells, 
                         VMesh::DElem::index_type i) const;  
  
  virtual void get_elems(VMesh::Elem::array_type& elems,
                         VMesh::Node::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems,
                         VMesh::Edge::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems, 
                         VMesh::Face::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems, 
                         VMesh::Cell::index_type i) const;
  virtual void get_elems(VMesh::Elem::array_type& elems, 
                         VMesh::DElem::index_type i) const;

  virtual void get_delems(VMesh::DElem::array_type& delems, 
                          VMesh::Face::index_type i) const;
  virtual void get_delems(VMesh::DElem::array_type& delems,
                          VMesh::Cell::index_type i) const;
  virtual void get_delems(VMesh::DElem::array_type& delems,
                          VMesh::Elem::index_type i) const;

};

//! Functions for creating the virtual interface for specific mesh types
//! These are similar to compare maker and only serve to instantiate the class

//! Currently there are only 3 variations of this mesh available
//! 1) linear interpolation
//! 2) quadratic interpolation
//! 3) cubic interpolation

//! Add the LINEAR virtual interface and the meshid for creating it 

//! Create virtual interface 
VMesh* CreateVHexVolMesh(HexVolMesh<HexTrilinearLgn<Point> >* mesh)
{
  return scinew VHexVolMesh<HexVolMesh<HexTrilinearLgn<Point> > >(mesh);
}

//! Register class maker, so we can instantiate it
static MeshTypeID HexVolMesh_MeshID1(HexVolMesh<HexTrilinearLgn<Point> >::type_name(-1),
                  HexVolMesh<HexTrilinearLgn<Point> >::mesh_maker);
                  
                  
//! Add the QUADRATIC virtual interface and the meshid for creating it                  
#if (SCIRUN_QUADRATIC_SUPPORT > 0)

//! Create virtual interface 
VMesh* CreateVHexVolMesh(HexVolMesh<HexTriquadraticLgn<Point> >* mesh)
{
  return scinew VHexVolMesh<HexVolMesh<HexTriquadraticLgn<Point> > >(mesh);
}

//! Register class maker, so we can instantiate it
static MeshTypeID HexVolMesh_MeshID2(HexVolMesh<HexTriquadraticLgn<Point> >::type_name(-1),
                  HexVolMesh<HexTriquadraticLgn<Point> >::mesh_maker);
#endif


//! Add the CUBIC virtual interface and the meshid for creating it                  
#if (SCIRUN_CUBIC_SUPPORT > 0)

//! Create virtual interface 
VMesh* CreateVHexVolMesh(HexVolMesh<HexTricubicHmt<Point> >* mesh)
{
  return scinew VHexVolMesh<HexVolMesh<HexTricubicHmt<Point> > >(mesh);
}

//! Register class maker, so we can instantiate it
static MeshTypeID HexVolMesh_MeshID3(HexVolMesh<HexTricubicHmt<Point> >::type_name(-1),
                  HexVolMesh<HexTricubicHmt<Point> >::mesh_maker);
#endif

template <class MESH>
void
VHexVolMesh<MESH>::get_nodes(VMesh::Node::array_type &nodes,
                             VMesh::Edge::index_type idx) const
{
  this->mesh_->get_nodes_from_edge(nodes,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_nodes(VMesh::Node::array_type &nodes,
                             VMesh::Face::index_type idx) const
{
  this->mesh_->get_nodes_from_face(nodes,idx);
}


template <class MESH>
void
VHexVolMesh<MESH>::get_nodes(VMesh::Node::array_type &nodes,
                             VMesh::Cell::index_type idx) const
{
  this->mesh_->get_nodes_from_cell(nodes,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_nodes(VMesh::Node::array_type &nodes,
                             VMesh::Elem::index_type idx) const
{
  this->mesh_->get_nodes_from_cell(nodes,idx);
}


template <class MESH>
void
VHexVolMesh<MESH>::get_nodes(VMesh::Node::array_type &nodes,
                             VMesh::DElem::index_type idx) const
{
  this->mesh_->get_nodes_from_face(nodes,idx);
}


template <class MESH>
void
VHexVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::Edge::index_type idx) const
{
  enodes.resize(1); enodes[0] = VMesh::ENode::index_type(idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::Face::index_type idx) const
{
  this->mesh_->get_edges_from_face(enodes,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::DElem::index_type idx) const
{
  this->mesh_->get_edges_from_face(enodes,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::Cell::index_type idx) const
{
  this->mesh_->get_edges_from_cell(enodes,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_enodes(VMesh::ENode::array_type &enodes,
                             VMesh::Elem::index_type idx) const
{
  this->mesh_->get_edges_from_cell(enodes,idx);
}



template <class MESH>
void
VHexVolMesh<MESH>::get_edges(VMesh::Edge::array_type &edges,
                             VMesh::Face::index_type idx) const
{
  this->mesh_->get_edges_from_face(edges,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_edges(VMesh::Edge::array_type &edges,
                             VMesh::DElem::index_type idx) const
{
  this->mesh_->get_edges_from_face(edges,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_edges(VMesh::Edge::array_type &edges,
                             VMesh::Cell::index_type idx) const
{
  this->mesh_->get_edges_from_cell(edges,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_edges(VMesh::Edge::array_type &edges,
                             VMesh::Elem::index_type idx) const
{
  this->mesh_->get_edges_from_cell(edges,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_faces(VMesh::Face::array_type &faces,
                             VMesh::Cell::index_type idx) const
{
  this->mesh_->get_faces_from_cell(faces,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_faces(VMesh::Face::array_type &faces,
                             VMesh::Elem::index_type idx) const
{
  this->mesh_->get_faces_from_cell(faces,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_faces(VMesh::Face::array_type &faces,
                             VMesh::DElem::index_type idx) const
{
  faces.resize(1); faces[0] = static_cast<VMesh::Face::index_type>(idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_cells(VMesh::Cell::array_type &cells,
                             VMesh::Node::index_type idx) const
{
  this->mesh_->get_cells_from_node(cells,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_cells(VMesh::Cell::array_type &cells,
                             VMesh::Edge::index_type idx) const
{
  this->mesh_->get_cells_from_edge(cells,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_cells(VMesh::Cell::array_type &cells,
                             VMesh::Face::index_type idx) const
{
  this->mesh_->get_cells_from_face(cells,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_cells(VMesh::Cell::array_type &cells,
                             VMesh::Elem::index_type idx) const
{
  cells.resize(1); cells[0] = static_cast<VMesh::Cell::index_type>(idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_cells(VMesh::Cell::array_type &cells,
                             VMesh::DElem::index_type idx) const
{
  this->mesh_->get_cells_from_face(cells,idx);
}


template <class MESH>
void
VHexVolMesh<MESH>::get_elems(VMesh::Elem::array_type &elems,
                             VMesh::Node::index_type idx) const
{
  this->mesh_->get_cells_from_node(elems,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_elems(VMesh::Elem::array_type &elems,
                             VMesh::Edge::index_type idx) const
{
  this->mesh_->get_cells_from_edge(elems,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_elems(VMesh::Elem::array_type &elems,
                             VMesh::Face::index_type idx) const
{
  this->mesh_->get_cells_from_face(elems,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_elems(VMesh::Elem::array_type &elems,
                             VMesh::Cell::index_type idx) const
{
  elems.resize(1); elems[0] = static_cast<VMesh::Elem::index_type>(idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_elems(VMesh::Elem::array_type &elems,
                             VMesh::DElem::index_type idx) const
{
  this->mesh_->get_cells_from_face(elems,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_delems(VMesh::DElem::array_type &delems,
                              VMesh::Cell::index_type idx) const
{
  this->mesh_->get_faces_from_cell(delems,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_delems(VMesh::DElem::array_type &delems,
                              VMesh::Elem::index_type idx) const
{
  this->mesh_->get_faces_from_cell(delems,idx);
}

template <class MESH>
void
VHexVolMesh<MESH>::get_delems(VMesh::DElem::array_type &delems,
                              VMesh::Face::index_type idx) const
{
  delems.resize(1); delems[0] = static_cast<VMesh::DElem::index_type>(idx);
}

} // namespace SCIRun

#endif
