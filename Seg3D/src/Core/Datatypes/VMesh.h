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


#ifndef CORE_DATATYPES_VMESH_H
#define CORE_DATATYPES_VMESH_H

#include <Core/Geometry/Transform.h>

#include <Core/Datatypes/Mesh.h>
#include <Core/Containers/StackBasedVector.h>
//! Incude needed for Windows: declares SCISHARE
#include <Core/Datatypes/share.h>

namespace SCIRun {

class BBox;
class VMesh;
class Transform;
class TypeDescription;

typedef LockingHandle<VMesh> VMeshHandle;


class SCISHARE VMesh {
public:

  //! VIRTUAL INTERFACE
  
  //! typedefs for Node, Edge, Face and Cell
  typedef Mesh::index_type                        index_type;
  typedef double                                  weight_type;
  typedef Mesh::size_type                         size_type;
  typedef std::vector<index_type>                 array_type;
  typedef std::vector<Point>                      points_type;
  typedef vector<size_type>                       dimension_type;
  typedef StackVector<double,3>                   coords_type;
  typedef vector<StackVector<double,3> >          coords_array_type;
  typedef vector<vector<StackVector<double,3> > > coords_array2_type;
  typedef StackVector<Point,3>                    dpoints_type;
  typedef unsigned int                            mask_type;
  
  //! A dual vector will store the first entries on the stack,
  //! if extra space is needed it will create vector to store
  //! data on. This wya most operations are done directly from
  //! the stack, only in less common situations memory is allocated
  //! and is used to store data. Here we allow 12 spaces to be on the
  //! stack which should be enough.
  typedef StackBasedVector<index_type,12>         index_array_type;
  typedef StackBasedVector<double,12>             weight_array_type;
  
  //! Virtual indices, iterators, and arrays
  class Node { 
    public:
      typedef VNodeIterator<VMesh::index_type>   iterator;
      typedef VNodeIndex<VMesh::index_type>      index_type;
      typedef VNodeIndex<VMesh::size_type>       size_type;
      typedef StackBasedVector<index_type,8>     array_type;
  };

  typedef vector<Node::array_type>                nodes_array_type;

  class ENode { 
    public:
      typedef VENodeIterator<VMesh::index_type>   iterator;
      typedef VENodeIndex<VMesh::index_type>      index_type;
      typedef VENodeIndex<VMesh::size_type>       size_type;
      typedef StackBasedVector<index_type,12>     array_type;
  };

  class Edge { 
    public:
      typedef VEdgeIterator<VMesh::index_type>   iterator;
      typedef VEdgeIndex<VMesh::index_type>      index_type;
      typedef VEdgeIndex<VMesh::size_type>       size_type;
      typedef StackBasedVector<index_type,12>    array_type;
  };
  
  class Face { 
    public:
      typedef VFaceIterator<VMesh::index_type>   iterator;    
      typedef VFaceIndex<VMesh::index_type>      index_type;
      typedef VFaceIndex<VMesh::size_type>       size_type;
      typedef StackBasedVector<index_type,12>    array_type;
  }; 
  
  class Cell { 
    public:
      typedef VCellIterator<VMesh::index_type>   iterator;
      typedef VCellIndex<VMesh::index_type>      index_type;
      typedef VCellIndex<VMesh::size_type>       size_type;
      typedef StackBasedVector<index_type,12>    array_type;
  };
  
  class Elem { 
    public:
      typedef VElemIterator<VMesh::index_type>   iterator;
      typedef VElemIndex<VMesh::index_type>      index_type;
      typedef VElemIndex<VMesh::size_type>       size_type;
      typedef StackBasedVector<index_type,12>    array_type;
  };
  
  class DElem { 
    public:
      typedef VDElemIterator<VMesh::index_type>  iterator;    
      typedef VDElemIndex<VMesh::index_type>     index_type;
      typedef VDElemIndex<VMesh::size_type>      size_type;
      typedef StackBasedVector<index_type,12>    array_type;
 };

  // All information needed to do one interpolation inside an element
  class ElemInterpolate {
    public:
      typedef VMesh::index_type   index_type;      
      typedef VMesh::size_type    size_type;      
      int                         basis_order; // which order was created
      index_type                  elem_index; // which element
      StackVector<index_type,8>   node_index; // nodes that span the element
      StackVector<index_type,12>  edge_index; // edges that span the element
      size_type                   num_hderivs; // number of derivatives per node
      StackBasedVector<double,64> weights;     // weights for given points and derivatives
  };
    
  typedef vector<ElemInterpolate> MultiElemInterpolate;  
    
  // All information needed to do one interpolation inside
  // one element to determine the gradient.    
  class ElemGradient {
  public:
      typedef VMesh::index_type    index_type;      
      typedef VMesh::size_type     size_type;      
      int                          basis_order; // which order was created
      index_type                   elem_index; // which element
      StackVector<index_type,8>    node_index; // nodes that span the element
      StackVector<index_type,12>   edge_index; // edges that span the element
      size_type                    num_hderivs; // Number of derivatives per point
      StackBasedVector<double,64>  weights;    // weights for given points

      size_type                    num_derivs; // Number of derivatives in topology   
      StackVector<double,9>        inverse_jacobian; // Inverse jacobian, for local to global tranformation
      coords_type                  coords;
  };  

  typedef vector<ElemGradient> MultiElemGradient;  
    
  //! instantiate the element information
  //! as these are protected the various derived constructors can fill
  //! these out.
  VMesh() :
    ref_cnt(0),
    basis_order_(0),
    dimension_(0),
    has_normals_(false),
    is_editable_(false),
    is_regular_(false),
    is_structured_(false),
    num_nodes_per_elem_(0),
    num_enodes_per_elem_(0),
    num_edges_per_elem_(0),
    num_faces_per_elem_(0),
    num_nodes_per_face_(0),
    num_edges_per_face_(0)
  {
  }

  virtual ~VMesh() {}

  //! Get the mesh associated with the virtual interface
  //! These functions will return the respective mesh or vmesh
  //! class.
  virtual LockingHandle<Mesh> mesh();
  inline  VMesh* vmesh() { return (this); }

  //! iterators for the virtual topology indices. These are not strickly needed
  //! but make the concepts in line with previous version. All iterators now
  //! go from 0 to number of elements, using consecutive unique numbers   
  inline void begin(Node::iterator &it) const
    { it = 0; }
  inline void begin(ENode::iterator &it) const
    { it = 0; }
  inline void begin(Edge::iterator &it) const
    { it = 0; }
  inline void begin(Face::iterator &it) const
    { it = 0; }
  inline void begin(Cell::iterator &it) const
    { it = 0; }
  inline void begin(Elem::iterator &it) const
    { it = 0; }
  inline void begin(DElem::iterator &it) const
    { it = 0; }

  inline void end(Node::iterator &it) const
    { Node::size_type s; size(s); it = static_cast<index_type>(s); }
  inline void end(ENode::iterator &it) const
    { Node::size_type s; size(s); it = static_cast<index_type>(s); }
  inline void end(Edge::iterator &it) const
    { Edge::size_type s; size(s); it = static_cast<index_type>(s); }
  inline void end(Face::iterator &it) const
    { Face::size_type s; size(s); it = static_cast<index_type>(s); }
  inline void end(Cell::iterator &it) const
    { Cell::size_type s; size(s); it = static_cast<index_type>(s); }
  inline void end(Elem::iterator &it) const
    { Elem::size_type s; size(s); it = static_cast<index_type>(s); }
  inline void end(DElem::iterator &it) const
    { DElem::size_type s; size(s); it = static_cast<index_type>(s); }

  
  //! Get the number of elements in the mesh of the specified type
  //! Note: that for any size other then the number of nodes or
  //! elements, one has to synchronize that part of the mesh.
  virtual void size(Node::size_type& size) const;
  virtual void size(ENode::size_type& size) const;
  virtual void size(Edge::size_type& size) const;
  virtual void size(Face::size_type& size) const;
  virtual void size(Cell::size_type& size) const;
  virtual void size(Elem::size_type& size) const;
  virtual void size(DElem::size_type& size) const;
  
  
  //! We have been using the num_#type#() in other functions as well to
  //! determine the number of nodes, edges etc. These are just shortcuts to
  //! make porgrams more readable
  inline size_t num_nodes() const
    { Node::index_type s; size(s); return(static_cast<size_t>(s)); }
  inline size_t num_enodes() const
    { ENode::index_type s; size(s); return(static_cast<size_t>(s)); }
  inline size_t num_edges() const
    { Edge::index_type s; size(s); return(static_cast<size_t>(s)); }
  inline size_t num_faces() const
    { Face::index_type s; size(s); return(static_cast<size_t>(s)); }
  inline size_t num_cells() const
    { Cell::index_type s; size(s); return(static_cast<size_t>(s)); }
  inline size_t num_elems() const
    { Elem::index_type s; size(s); return(static_cast<size_t>(s)); }
  inline size_t num_delems() const
    { DElem::index_type s; size(s); return(static_cast<size_t>(s)); }  
  
  
  //! Topological functions: note that currently most meshes have an incomplete
  //! set implemented. Currently each mesh has:
  //! Getting cell, face, edge indices from node indices
  //! Getting the indices of the elements that are topologically building up the
  //! the element: e.g. one can derive faces from a cell index but not YET 
  //! vice versa. 
  
  //! Get the nodes that make up an element
  //! Depending on the geometry not every function may be available
  virtual void get_nodes(Node::array_type& nodes, Node::index_type i) const;
  virtual void get_nodes(Node::array_type& nodes, Edge::index_type i) const;
  virtual void get_nodes(Node::array_type& nodes, Face::index_type i) const;
  virtual void get_nodes(Node::array_type& nodes, Cell::index_type i) const;
  virtual void get_nodes(Node::array_type& nodes, Elem::index_type i) const;
  virtual void get_nodes(Node::array_type& nodes, DElem::index_type i) const;


  //! Get the nodes that make up an element
  //! Depending on the geometry not every function may be available
  virtual void get_enodes(ENode::array_type& nodes, Node::index_type i) const;
  virtual void get_enodes(ENode::array_type& nodes, Edge::index_type i) const;
  virtual void get_enodes(ENode::array_type& nodes, Face::index_type i) const;
  virtual void get_enodes(ENode::array_type& nodes, Cell::index_type i) const;
  virtual void get_enodes(ENode::array_type& nodes, Elem::index_type i) const;
  virtual void get_enodes(ENode::array_type& nodes, DElem::index_type i) const;
  

  //! Get the edges that make up an element
  //! or get the edges that contain certain nodes
  //! Depending on the geometry not every function may be available
  virtual void get_edges(Edge::array_type& edges, Node::index_type i) const;
  virtual void get_edges(Edge::array_type& edges, Edge::index_type i) const;
  virtual void get_edges(Edge::array_type& edges, Face::index_type i) const;
  virtual void get_edges(Edge::array_type& edges, Cell::index_type i) const;
  virtual void get_edges(Edge::array_type& edges, Elem::index_type i) const;
  virtual void get_edges(Edge::array_type& edges, DElem::index_type i) const;


  //! Get the faces that make up an element
  //! or get the faces that contain certain nodes or edges
  //! Depending on the geometry not every function may be available
  virtual void get_faces(Face::array_type& faces, Node::index_type i) const;
  virtual void get_faces(Face::array_type& faces, Edge::index_type i) const;
  virtual void get_faces(Face::array_type& faces, Face::index_type i) const;
  virtual void get_faces(Face::array_type& faces, Cell::index_type i) const;
  virtual void get_faces(Face::array_type& faces, Elem::index_type i) const;
  virtual void get_faces(Face::array_type& faces, DElem::index_type i) const;

  //! Get the cell index that contains the specified component
  //! Depending on the geometry not every function may be available
  virtual void get_cells(Cell::array_type& cells, Node::index_type i) const;
  virtual void get_cells(Cell::array_type& cells, Edge::index_type i) const;
  virtual void get_cells(Cell::array_type& cells, Face::index_type i) const;
  virtual void get_cells(Cell::array_type& cells, Cell::index_type i) const;
  virtual void get_cells(Cell::array_type& cells, Elem::index_type i) const;
  virtual void get_cells(Cell::array_type& cells, DElem::index_type i) const;

  //! Get the element index that contains the specified component
  //! Depending on the geometry not every function may be available
  virtual void get_elems(Elem::array_type& elems, Node::index_type i) const;
  virtual void get_elems(Elem::array_type& elems, Edge::index_type i) const;
  virtual void get_elems(Elem::array_type& elems, Face::index_type i) const;
  virtual void get_elems(Elem::array_type& elems, Cell::index_type i) const;
  virtual void get_elems(Elem::array_type& elems, Elem::index_type i) const;
  virtual void get_elems(Elem::array_type& elems, DElem::index_type i) const;

  //! Get the derived element index that contains the specified component
  //! Depending on the geometry not every function may be available
  virtual void get_delems(DElem::array_type& delems, Node::index_type i) const;
  virtual void get_delems(DElem::array_type& delems, Edge::index_type i) const;
  virtual void get_delems(DElem::array_type& delems, Face::index_type i) const;
  virtual void get_delems(DElem::array_type& delems, Cell::index_type i) const;
  virtual void get_delems(DElem::array_type& delems, Elem::index_type i) const;
  virtual void get_delems(DElem::array_type& delems, DElem::index_type i) const;

  //! Get the topology index from the vertex indices
  virtual bool get_elem(Elem::index_type& elem, Node::array_type& nodes) const;
  virtual bool get_delem(DElem::index_type& delem, Node::array_type& nodes) const;
  virtual bool get_cell(Cell::index_type& cell, Node::array_type& nodes) const;
  virtual bool get_face(Face::index_type& face, Node::array_type& nodes) const;
  virtual bool get_edge(Edge::index_type& edge, Node::array_type& nodes) const;

  //! Get the center of a certain mesh element
  virtual void get_center(Point &point, Node::index_type i) const;
  virtual void get_center(Point &point, ENode::index_type i) const;
  virtual void get_center(Point &point, Edge::index_type i) const;
  virtual void get_center(Point &point, Face::index_type i) const;
  virtual void get_center(Point &point, Cell::index_type i) const;
  virtual void get_center(Point &point, Elem::index_type i) const;
  virtual void get_center(Point &point, DElem::index_type i) const;

  //! Get the centers of a series of nodes
  virtual void get_centers(Point* points, Node::array_type& array) const;
  virtual void get_centers(Point* points, Elem::array_type& array) const;

  inline void get_centers(points_type &points, Node::array_type& array) const
    { points.resize(array.size()); get_centers(&(points[0]),array); }
  inline void get_centers(points_type &points, Elem::array_type& array) const
    { points.resize(array.size()); get_centers(&(points[0]),array); }


  //! Get the geometrical sizes of the mesh elements
  inline  double get_size(Node::index_type i) const 
  { return (0.0); }
  inline  double get_size(ENode::index_type i) const
  { return (0.0); }
  virtual double get_size(Edge::index_type i) const;
  virtual double get_size(Face::index_type i) const;
  virtual double get_size(Cell::index_type i) const;
  virtual double get_size(Elem::index_type i) const;
  virtual double get_size(DElem::index_type i) const;

  //! alternative ways to get the size values
  inline double get_length(Edge::index_type i) const
  { return (get_size(i)); }
  inline double get_area(Face::index_type i) const
  { return (get_size(i)); }
  inline double get_volume(Cell::index_type i) const
  { return (get_size(i)); }

  inline double get_length(Elem::index_type i) const
  { return (get_size(i)); }
  inline double get_area(Elem::index_type i) const
  { return (get_size(i)); }
  inline double get_volume(Elem::index_type i) const
  { return (get_size(i)); }
    
  //! Specialized functions to get weights for the interpolation
  //! One should use these instead of get_weights
  
  virtual void get_interpolate_weights(const Point& p, 
                                       ElemInterpolate& ei, 
                                       int basis_order) const;
  
  virtual void get_interpolate_weights(const coords_type& coords, 
                                       Elem::index_type elem, 
                                       ElemInterpolate& ei,
                                       int basis_order) const;

  virtual void get_minterpolate_weights(const vector<Point>& p, 
                                        MultiElemInterpolate& ei, 
                                        int basis_order) const;

  virtual void get_minterpolate_weights(const vector<coords_type>& coords, 
                                        Elem::index_type elem, 
                                        MultiElemInterpolate& ei,
                                        int basis_order) const;
  virtual void get_gradient_weights(const Point& p, 
                                    ElemGradient& ei, 
                                    int basis_order) const;
  
  virtual void get_gradient_weights(const coords_type& coords, 
                                    Elem::index_type elem, 
                                    ElemGradient& ei,
                                    int basis_order) const;

  virtual void get_mgradient_weights(const vector<Point>& p, 
                                     MultiElemGradient& ei, 
                                     int basis_order) const;

  virtual void get_mgradient_weights(const vector<coords_type>& coords, 
                                    Elem::index_type elem, 
                                    MultiElemGradient& eg,
                                    int basis_order) const;

  virtual void get_weights(const coords_type& coords, 
                           vector<double>& weights,
                           int basis_order) const;                                 

  virtual void get_derivate_weights(const coords_type& coords, 
                           vector<double>& weights,
                           int basis_order) const;

  virtual void get_gaussian_scheme(vector<VMesh::coords_type>& coords, 
                                   vector<double>& weights, int order) const;
                                   
  virtual void get_regular_scheme(vector<VMesh::coords_type>& coords, 
                                  vector<double>& weights, int order) const;

///////////////

  //! Locate where a position is in  the mesh
  //! The node version finds the closest node
  //! The element version find the element that contains the point
  virtual bool locate(VMesh::Node::index_type &i, const Point &point) const;
  virtual bool locate(VMesh::Elem::index_type &i, const Point &point) const;
  virtual bool locate(VMesh::Elem::index_type &i, 
                      VMesh::coords_type &coords, const Point& point) const;

  virtual void mlocate(vector<Node::index_type> &i, 
                       const vector<Point> &point) const;
  virtual void mlocate(vector<Elem::index_type> &i, 
                       const vector<Point> &point) const;


  //! Find the closest point on a surface or a curve
  virtual bool find_closest_node(double& dist,
                                 Point& result,
                                 VMesh::Node::index_type &i, 
                                 const Point &point) const; 

  virtual bool find_closest_node(double& dist,
                                 Point& result,
                                 VMesh::Node::index_type &i, 
                                 const Point &point,
                                 double maxdist) const; 

  inline bool find_closest_node(Point& result,
                                VMesh::Node::index_type& i,
                                const Point &point)
    { double dist; return(find_closest_node(dist,result,i,point));}

  virtual bool find_closest_nodes(vector<VMesh::Node::index_type>& nodes,
                                  const Point& p, double maxdist) const;

  virtual bool find_closest_elem(double &dist,
                                 Point &result,
                                 VMesh::coords_type &coords,
                                 VMesh::Elem::index_type &i, 
                                 const Point &point) const; 

  virtual bool find_closest_elem(double &dist,
                                 Point &result,
                                 VMesh::coords_type &coords,
                                 VMesh::Elem::index_type &i, 
                                 const Point &point,
                                 double maxdist) const; 
                                 
  inline  bool find_closest_elem(double &dist,
                                 Point &result,
                                 VMesh::Elem::index_type &i, 
                                 const Point &point) const
  { 
    VMesh::coords_type coords; 
    return(find_closest_elem(dist,result,coords,i,point));
  }

  inline  bool find_closest_elem(Point &result,
                                 VMesh::Elem::index_type &i, 
                                 const Point &point) const
  { 
    double dist;
    VMesh::coords_type coords; 
    return(find_closest_elem(dist,result,coords,i,point));
  }

  inline  bool find_closest_elem(VMesh::coords_type &coords,
                                 VMesh::Elem::index_type &i, 
                                 const Point &point) const
  { 
    double dist;
    Point result;
    return(find_closest_elem(dist,result,coords,i,point));
  }


  // TODO: Need to reformulate this one, closest element can have multiple 
  // intersection points
  virtual bool find_closest_elems(double& dist,
                                  Point& result,
                                  VMesh::Elem::array_type &i, 
                                  const Point &point) const; 

  //! Find the coordinates of a point in a certain element
  virtual bool get_coords(coords_type& coords, 
                                const Point &point, Elem::index_type i) const;
  
  //! Interpolate from local coordinates to global coordinates
  virtual void interpolate(Point &p, 
                         const coords_type& coords, Elem::index_type i) const;

  virtual void minterpolate(vector<Point> &p, 
                            const vector<coords_type>& coords, 
                            Elem::index_type i) const;

  //! Interpolate from local coordinates to a derivative in local coordinates  
  virtual void derivate(dpoints_type &p, 
                         const coords_type& coords, Elem::index_type i) const;
  
  virtual void get_normal(Vector &result, coords_type& coords, 
                                 Elem::index_type eidx, DElem::index_type fidx) const;
  
  inline void get_normal(Vector &result, coords_type& coords, 
                                 Elem::index_type eidx) const
    { get_normal(result,coords,eidx,0); }
    
  //! Set and get a node location.
  //! Node set is only available for editable meshes
    
  virtual void get_random_point(Point &p, 
                                Elem::index_type i,FieldRNG &rng) const;
                                
  inline  void get_point(Point &point, Node::index_type i) const
    { get_center(point,i); }
  inline  void get_point(Point &point, ENode::index_type i) const
    { get_center(point,i); }
    
  virtual void set_point(const Point &point, Node::index_type i);
  virtual void set_point(const Point &point, ENode::index_type i);
  
  // Only for irregular data
  virtual Point* get_points_pointer() const;
  // Only for unstructured data
  virtual VMesh::index_type* get_elems_pointer() const;
  
  inline void copy_nodes(VMesh* imesh, Node::index_type i, 
                          Node::index_type o,Node::size_type size)
  {
    Point* ipoint = imesh->get_points_pointer();
    Point* opoint = get_points_pointer();
    for (index_type j=0; j<size; j++,i++,o++ ) opoint[o] = ipoint[i];
  }

  inline void copy_nodes(VMesh* imesh)
  {
    size_type size = imesh->num_nodes();
    resize_nodes(size);
    Point* ipoint = imesh->get_points_pointer();
    Point* opoint = get_points_pointer();
    for (index_type j=0; j<size; j++) opoint[j] = ipoint[j];
  }
  
  inline void copy_elems(VMesh* imesh, Elem::index_type i, 
                          Elem::index_type o,Elem::size_type size,
                          Elem::size_type offset)
  {
    VMesh::index_type* ielem = imesh->get_elems_pointer();
    VMesh::index_type* oelem  = get_elems_pointer();
    index_type ii = i*num_nodes_per_elem_;
    index_type oo = o*num_nodes_per_elem_;
    size_type  ss = size*num_nodes_per_elem_;
    for (index_type j=0; j <ss; j++,ii++,oo++) oelem[oo] = ielem[ii]+offset;
  }

  inline void copy_elems(VMesh* imesh)
  {
    VMesh::index_type* ielem = imesh->get_elems_pointer();
    VMesh::index_type* oelem  = get_elems_pointer();
    //index_type ii = 0;
    //index_type oo = 0;
    size_type  ss = num_elems()*num_nodes_per_elem_;
    for (index_type j=0; j <ss; j++) oelem[j] = ielem[j];
  }
  
  //! Preallocate memory for better performance
  virtual void node_reserve(size_t size);
  inline  void reserve_nodes(size_t size) 
    { node_reserve(size); }
  
  virtual void elem_reserve(size_t size);
  inline  void reserve_elems(size_t size) 
    { elem_reserve(size); }
  
  virtual void resize_nodes(size_t size);
  virtual void resize_elems(size_t size);
  inline  void resize_points(size_t size) { resize_nodes(size); }

  //! Add a node to a mesh
  virtual void add_node(const Point &point,Node::index_type &i);
  virtual void add_enode(const Point &point,ENode::index_type &i);
  
  //! alternative calls
  inline void set_node(const Point &point, Node::index_type i)
    { set_point(point,i); }
  inline void set_enode(const Point &point, ENode::index_type i)
    { set_point(point,i); }
    
  inline Node::index_type add_node(const Point& point) 
    { Node::index_type idx; add_node(point,idx); return (idx); }
  
  inline void get_node(Point &point, Node::index_type i)
    { get_point(point,i); }
  inline void get_enode(Point &point, ENode::index_type i)
    { get_point(point,i); }
  
  //! Do not use this one as it is not clear whether it is a 
  //! element node or edge node  
  inline VMesh::Node::index_type add_point(const Point& point) 
    { Node::index_type idx; add_node(point,idx); return (idx); }
    
    
    
  //! Add an element to a mesh
  virtual void add_elem(const Node::array_type &nodes,Elem::index_type &i);
  
  //! Alternative calls for add_elem
  inline VMesh::Elem::index_type add_elem(const Node::array_type nodes)
    { Elem::index_type idx; add_elem(nodes,idx); return (idx); }
  
  
  //! Currently only available for tetrahedra, triangles and curves
  virtual void insert_node_into_elem(Elem::array_type& newelems, 
                                     Node::index_type& newnode,
                                     Elem::index_type  elem,
                                     Point& point);
  
  //! Get the neighbors of a node or an element
  virtual bool get_neighbor(Elem::index_type &neighbor, 
                      Elem::index_type from, DElem::index_type delem) const;
  virtual bool get_neighbors(Elem::array_type &elems, 
                         Elem::index_type i, DElem::index_type delem) const;
  virtual void get_neighbors(Elem::array_type &elems, 
                             Elem::index_type i) const;
  virtual void get_neighbors(Node::array_type &nodes, 
                             Node::index_type i) const;

  //! Draw non linear elements
  virtual void pwl_approx_edge(coords_array_type &coords, 
                               Elem::index_type ci, unsigned int which_edge, 
                               unsigned int div_per_unit) const;
  virtual void pwl_approx_face(coords_array2_type &coords, 
                               Elem::index_type ci, unsigned int which_face, 
                               unsigned int div_per_unit) const;

  //! Get node normals, needed for visualization
  virtual void get_normal(Vector& norm,Node::index_type i) const;

  //! Get the dimensions of the mesh.
  //! This function will replace get_dim()
  virtual void get_dimensions(dimension_type& dim);

  virtual void get_elem_dimensions(dimension_type& dim);

  //! The following functions are intended so one can do the local to global
  //! transformation efficiently. As the transformation matrix is a constant for
  //! certain meshes, it is precomputed and his function looks up the precomputed
  //! jacobians, while for others it depends on the element and it is computed
  //! on the fly. To assure that the fastest method is used, use these functions. 

  //! Get the determinant of the jacobian matrix
  //! Coords determine where the determinant needs o be evaluated
  //! Generally LatVol, ImageMesh, TriMesh, TetMesh have a jacobian that
  //! is independen of the local coordinate system, however HexVol, QuadSurf,
  //! and PrismVol have values that depend on the local position within the 
  //! element
  virtual double det_jacobian(const coords_type& coords,
                              Elem::index_type idx) const; 

  //! Get the jacobian of the local to global transformation
  //! Note J needs to be a least an array of 9 values. 
  //! Coords and idx again specify the element and the position in
  //! local coordinates.
  virtual void jacobian(const coords_type& coords,
                        Elem::index_type idx,
                        double* J) const; 

  //! Get the inverse jacobian matrix. This gives as side product the
  //! determinant of the inverse matrix.
  virtual double inverse_jacobian(const coords_type& coords,
                                   Elem::index_type idx,
                                   double* Ji) const;


  //! Element Quality metrics:
  
  virtual double scaled_jacobian_metric(const Elem::index_type idx) const;
  virtual double jacobian_metric(const Elem::index_type idx) const;
  inline double volume_metric(const Elem::index_type idx) const
    { return(get_volume(idx)); }
  

  //! Direct access to get weights functions in basis functions
  //! These four are for interpolation
  inline  void get_constant_weights(coords_type& coords, vector<double>& weights)
    { weights.resize(1); weights[0] = 1.0; }
  virtual void get_linear_weights(coords_type& coords, vector<double>& weights);
  virtual void get_quadratic_weights(coords_type& coords, vector<double>& weights);
  virtual void get_cubic_weights(coords_type& coords, vector<double>& weights);

  //! These four are for computating gradients
  inline  void get_constant_derivate_weights(coords_type& coords, vector<double>& weights)
    { weights.resize(1); weights[0] = 0.0; }
  virtual void get_linear_derivate_weights(coords_type& coords, vector<double>& weights);
  virtual void get_quadratic_derivate_weights(coords_type& coords, vector<double>& weights);
  virtual void get_cubic_derivate_weights(coords_type& coords, vector<double>& weights);

  //! Rerouting of some of the virtual mesh function calls
  
  virtual BBox get_bounding_box() const;
  virtual bool synchronize(unsigned int sync); 
  virtual bool unsynchronize(unsigned int sync);
  
  // Only use this function when this is the only code that uses this mesh
  virtual bool clear_synchronization();
  
  // Transform a full field, this one works on the full field
  virtual void transform(const Transform &t);
  
  //! Get the transform from a regular field
  virtual Transform get_transform() const;
  //! Set the transform of a regular field
  virtual void set_transform(const Transform &t);
  
  virtual void get_canonical_transform(Transform &t);
  //! Get the epsilon for doing numerical computations
  //! This one is generally 1e-7*length diagonal of the bounding box
  virtual double get_epsilon() const;

  //! check the type of mesh
  virtual bool is_pointcloudmesh()     { return (false); }
  virtual bool is_curvemesh()          { return (false); }
  virtual bool is_scanlinemesh()       { return (false); }
  virtual bool is_structcurvemesh()    { return (false); }
  virtual bool is_trisurfmesh()        { return (false); }
  virtual bool is_quadsurfmesh()       { return (false); }
  virtual bool is_imagemesh()          { return (false); }
  virtual bool is_structquadsurfmesh() { return (false); }
  virtual bool is_tetvolmesh()         { return (false); }
  virtual bool is_prismvolmesh()       { return (false); }
  virtual bool is_hexvolmesh()         { return (false); }
  virtual bool is_latvolmesh()         { return (false); }
  virtual bool is_structhexvolmesh()   { return (false); }

  inline bool is_constantmesh()        { return (basis_order_ == 0); }
  inline bool is_linearmesh()          { return (basis_order_ == 1); }
  inline bool is_quadraticmesh()       { return (basis_order_ == 2); }
  inline bool is_cubicmesh()           { return (basis_order_ == 3); }
  inline bool is_nonlinearmesh()          { return (basis_order_ > 1); }
  

  //----------------------------------------------------------------------

  //! Used for local conversion of vector types
  //! At some point this function should go away
  template <class VEC1, class VEC2>
  inline void convert_vector(VEC1& v1, VEC2 v2) const
  {
    v1.resize(v2.size());
    for (size_t p=0; p < v2.size(); p++) v1[p] = static_cast<typename VEC1::value_type>(v2[p]);
  }

  //! Inline calls to information that is constant for a mesh and does not
  //! change for a mesh. These properties are stored directly in the vmesh
  //! data structure and hence we can replace them by simple inline calls.

  inline int basis_order()
    { return (basis_order_); }

  inline int dimensionality()
    { return (dimension_); }
    
  inline bool is_point()
    { return (dimension_ == 0); }

  inline bool is_line()
    { return (dimension_ == 1); }

  inline bool is_surface()
    { return (dimension_ == 2); }

  inline bool is_volume()
    { return (dimension_ == 3); }

  inline unsigned int num_nodes_per_elem()
    { return (num_nodes_per_elem_); }

  inline unsigned int num_enodes_per_elem()
    { return (num_enodes_per_elem_); }

  inline unsigned int num_edges_per_elem()
    { return (num_edges_per_elem_); }

  inline unsigned int num_faces_per_elem()
    { return (num_faces_per_elem_); }

  inline unsigned int num_nodes_per_face()
    { return (num_nodes_per_face_); }

  inline unsigned int num_nodes_per_edge()
    { return (2); }

  inline unsigned int num_edges_per_face()
    { return (num_edges_per_face_); }
    
  inline unsigned int num_gradients_per_node()
    { return (num_gradients_per_node_); }
    
  inline bool has_normals()
    { return (has_normals_); }

  inline bool is_editable()
    { return (is_editable_); }

  inline bool is_regularmesh()
    { return (is_regular_); }

  inline bool is_irregularmesh()
    { return (!is_regular_); }

  inline bool is_structuredmesh()
    { return (is_structured_); }

  inline bool is_unstructuredmesh()
    { return (!is_structured_); }

  inline bool has_virtual_interface()
    { return (true); }

  inline size_type get_ni() const 
    { return ni_; }
  inline size_type get_nj() const 
    { return nj_; }
  inline size_type get_nk() const 
    { return nk_; }

  inline unsigned int generation() const
    { return (generation_); }

  inline void to_index(Node::index_type& idx,index_type i, index_type j, index_type k)
  {
    idx = Node::index_type(i+ni_*j+ni_*nj_*k);
  }

  inline void to_index(Node::index_type& idx,index_type i, index_type j)
  {
    idx = Node::index_type(i+ni_*j);
  }
  
  inline void to_index(Elem::index_type& idx,index_type i, index_type j, index_type k)
  {
    idx = Elem::index_type(i+(ni_-1)*j+(ni_-1)*(nj_-1)*k);
  }

  inline void to_index(Elem::index_type& idx,index_type i, index_type j)
  {
    idx = Elem::index_type(i+(ni_-1)*j);
  }    

  inline void to_index(Cell::index_type& idx,index_type i, index_type j, index_type k)
  {
    idx = Cell::index_type(i+(ni_-1)*j+(ni_-1)*(nj_-1)*k);
  }

  inline void to_index(Face::index_type& idx,index_type i, index_type j)
  {
    idx = Face::index_type(i+(ni_-1)*j);
  }    

  inline bool is_pnt_element()
    { return (is_pointcloudmesh()); }
    
  inline bool is_crv_element()
    { return (is_structcurvemesh()||is_curvemesh()||is_scanlinemesh()); }

  inline bool is_tri_element()
    { return (is_trisurfmesh()); }
    
  inline bool is_quad_element()
    { return (is_structquadsurfmesh()||is_quadsurfmesh()||is_imagemesh()); }
    
  inline bool is_tet_element()
    { return (is_tetvolmesh()); }
    
  inline bool is_prism_element()
    { return (is_prismvolmesh()); }
    
  inline bool is_hex_element()
    { return (is_structhexvolmesh()||is_hexvolmesh()||is_latvolmesh()); }

  inline void get_element_vertices(coords_array_type& coords)
    { coords = unit_vertices_; }
    
  inline void get_element_edges(nodes_array_type& edges)
    { edges = unit_edges_; }
 
  inline void get_element_center(coords_type& coords)
    { coords = unit_center_; }
 
  inline double get_element_size()
    { return (element_size_); }
 
public:
  int ref_cnt;

protected:
  int basis_order_;
  int dimension_;
  
  bool has_normals_;
  bool is_editable_;
  bool is_regular_;
  bool is_structured_;

  unsigned int num_nodes_per_elem_;
  unsigned int num_enodes_per_elem_;
  unsigned int num_edges_per_elem_;
  unsigned int num_faces_per_elem_;
  unsigned int num_nodes_per_face_;
  unsigned int num_edges_per_face_;  
  unsigned int num_gradients_per_node_;
 
  double    element_size_;
  
  coords_array_type       unit_vertices_;
  nodes_array_type        unit_edges_;
  coords_type             unit_center_;
          
  size_type ni_;
  size_type nj_;
  size_type nk_;

  unsigned int generation_;
};

} // end namespace SCIRun

#endif // Datatypes_Mesh_h
