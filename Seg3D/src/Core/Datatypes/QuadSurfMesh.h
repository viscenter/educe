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

#ifndef CORE_DATATYPES_QUADSURFMESH_H
#define CORE_DATATYPES_QUADSURFMESH_H 1

//! Include what kind of support we want to have
//! Need to fix this and couple it sci-defs
#include <Core/Datatypes/MeshSupport.h>

#include <Core/Containers/StackVector.h>
#include <Core/Containers/LockingHandle.h>
#include <Core/Containers/Handle.h>

#include <Core/Geometry/BBox.h>
#include <Core/Geometry/CompGeom.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Transform.h>
#include <Core/Geometry/Vector.h>

#include <Core/Basis/Locate.h>
#include <Core/Basis/QuadBilinearLgn.h>
#include <Core/Basis/QuadBiquadraticLgn.h>
#include <Core/Basis/QuadBicubicHmt.h>

#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/FieldIterator.h>
#include <Core/Datatypes/FieldRNG.h>
#include <Core/Datatypes/SearchGrid.h>
#include <Core/Datatypes/VMesh.h>

#include <Core/Math/MinMax.h>

//! Incude needed for Windows: declares SCISHARE
#include <Core/Datatypes/share.h>

namespace SCIRun {

using std::pair;
using std::string;
using std::vector;

/////////////////////////////////////////////////////
// Declarations for virtual interface

template <class Basis> class QuadSurfMesh;

//! make sure any other mesh other than the preinstantiate ones
//! returns no virtual interface. Altering this behaviour will allow
//! for dynamically compiling the interfae if needed.
template<class MESH>
VMesh* CreateVQuadSurfMesh(MESH* mesh) { return (0); }

#if (SCIRUN_QUADSURF_SUPPORT > 0)
//! These declarations are needed for a combined dynamic compilation as
//! as well as virtual functions solution.
//! Declare that these can be found in a library that is already
//! precompiled. So dynamic compilation will not instantiate them again.
SCISHARE VMesh* CreateVQuadSurfMesh(QuadSurfMesh<QuadBilinearLgn<Point> >* mesh);
#if (SCIRUN_QUADRATIC_SUPPORT > 0)
SCISHARE VMesh* CreateVQuadSurfMesh(QuadSurfMesh<QuadBiquadraticLgn<Point> >* mesh);
#endif
#if (SCIRUN_CUBIC_SUPPORT > 0)
SCISHARE VMesh* CreateVQuadSurfMesh(QuadSurfMesh<QuadBicubicHmt<Point> >* mesh);
#endif

#endif
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////
// Declarations for QuadSurfMesh class


template <class Basis>
class QuadSurfMesh : public Mesh
{

//! Make sure the virtual interface has access
template<class MESH> friend class VQuadSurfMesh;
template<class MESH> friend class VMeshShared;
template<class MESH> friend class VUnstructuredMesh;


public:
  // Types that change depending on 32 or 64bits
  typedef SCIRun::index_type                 under_type;
  typedef SCIRun::index_type                 index_type;
  typedef SCIRun::size_type                  size_type;
  typedef SCIRun::mask_type                  mask_type;

  typedef LockingHandle<QuadSurfMesh<Basis> > handle_type;
  typedef Basis                         basis_type;

  //! Index and Iterator types required for Mesh Concept.
  struct Node {
    typedef NodeIndex<under_type>       index_type;
    typedef NodeIterator<under_type>    iterator;
    typedef NodeIndex<under_type>       size_type;
    typedef StackVector<index_type, 4>  array_type;
  };

  struct Edge {
    typedef EdgeIndex<under_type>       index_type;
    typedef EdgeIterator<under_type>    iterator;
    typedef EdgeIndex<under_type>       size_type;
    typedef vector<index_type>          array_type;
  };

  struct Face {
    typedef FaceIndex<under_type>       index_type;
    typedef FaceIterator<under_type>    iterator;
    typedef FaceIndex<under_type>       size_type;
    typedef vector<index_type>          array_type;
  };

  struct Cell {
    typedef CellIndex<under_type>       index_type;
    typedef CellIterator<under_type>    iterator;
    typedef CellIndex<under_type>       size_type;
    typedef vector<index_type>          array_type;
  };

  //! Elem refers to the most complex topological object
  //! DElem refers to object just below Elem in the topological hierarchy
  typedef Face Elem;
  typedef Edge DElem;

  //! Somehow the information of how to interpolate inside an element
  //! ended up in a separate class, as they need to share information
  //! this construction was created to transfer data. 
  //! Hopefully in the future this class will disappear again.
  friend class ElemData;

  class ElemData
  {
  public:
    typedef typename QuadSurfMesh<Basis>::index_type  index_type;

    ElemData(const QuadSurfMesh<Basis>& msh, const index_type ind) :
      mesh_(msh),
      index_(ind)
    {
      //! Linear and Constant Basis never use edges_
      if (basis_type::polynomial_order() > 1) 
      {
        mesh_.get_edges_from_face(edges_, index_);
      }
    }

    // the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const 
    {
      return mesh_.faces_[index_ * 4];
    }
    inline
    index_type node1_index() const 
    {
      return mesh_.faces_[index_ * 4 + 1];
    }
    inline
    index_type node2_index() const 
    {
      return mesh_.faces_[index_ * 4 + 2];
    }
    inline
    index_type node3_index() const 
    {
      return mesh_.faces_[index_ * 4 + 3];
    }

    //! changed the indexing as we now use unique indices
    inline
    index_type edge0_index() const 
    {
      return edges_[0];
    }
    inline
    index_type edge1_index() const 
    {
      return edges_[1];
    }
    inline
    index_type edge2_index() const 
    {
      return edges_[2];
    }
    inline
    index_type edge3_index() const 
    {
      return edges_[3];
    }

    inline
    const Point &node0() const 
    {
      return mesh_.points_[node0_index()];
    }
    inline
    const Point &node1() const 
    {
      return mesh_.points_[node1_index()];
    }
    inline
    const Point &node2() const 
    {
      return mesh_.points_[node2_index()];
    }
    inline
    const Point &node3() const 
    {
      return mesh_.points_[node3_index()];
    }

  private:
    //! reference to the mesh
    const QuadSurfMesh<Basis>        &mesh_;
    //! copy of element index
    const index_type                 index_;
    //! need edges for quadratic meshes    
    typename Edge::array_type        edges_;
   };

  //////////////////////////////////////////////////////////////////
  
  //! Construct a new mesh
  QuadSurfMesh();
  
  //! Copy a mesh, needed for detaching the mesh from a field   
  QuadSurfMesh(const QuadSurfMesh &copy);
  
  //! Clone function for detaching the mesh and automatically generating
  //! a new version if needed.    
  virtual QuadSurfMesh *clone() { return new QuadSurfMesh(*this); }

  //! Destructor 
  virtual ~QuadSurfMesh();

  //! Access point to virtual interface
  virtual VMesh* vmesh() {  return vmesh_.get_rep(); }

  //! This one should go at some point, should be reroute throught the
  //! virtual interface
  virtual int basis_order() { return (basis_.polynomial_order()); }

  //! Topological dimension  
  virtual int dimensionality() const { return 2; }

  //! What kind of mesh is this 
  //! structured = no connectivity data
  //! regular    = no node location data
  virtual int topology_geometry() const 
    { return (Mesh::UNSTRUCTURED | Mesh::IRREGULAR); }

  //! Get the bounding box of the field    
  virtual BBox get_bounding_box() const;

  //! Transform a field (transform all nodes using this transformation matrix)  
  virtual void transform(const Transform &t);

  //! Check whether mesh can be altered by adding nodes or elements
  virtual bool is_editable() const { return (true); }

  //! Has this mesh normals.
  virtual bool has_normals() const { return (true); }

  //! Compute tables for doing topology, these need to be synchronized
  //! before doing a lot of operations.
  virtual bool synchronize(mask_type mask);
  virtual bool unsynchronize(mask_type mask);

  //! Get the basis class.  
  Basis& get_basis() { return basis_; }

  //! begin/end iterators   
  void begin(typename Node::iterator &) const;
  void begin(typename Edge::iterator &) const;
  void begin(typename Face::iterator &) const;
  void begin(typename Cell::iterator &) const;

  void end(typename Node::iterator &) const;
  void end(typename Edge::iterator &) const;
  void end(typename Face::iterator &) const;
  void end(typename Cell::iterator &) const;

  void size(typename Node::size_type &) const;
  void size(typename Edge::size_type &) const;
  void size(typename Face::size_type &) const;
  void size(typename Cell::size_type &) const;

  //! These are here to convert indices to unsigned int
  //! counters. Some how the decision was made to use multi
  //! dimensional indices in some fields, these functions
  //! should deal with different pointer types.
  //! Use the virtual interface to avoid all this non sense.
  void to_index(typename Node::index_type &index, index_type i) const 
    { index = i; }
  void to_index(typename Edge::index_type &index, index_type i) const 
    { index = i; }
  void to_index(typename Face::index_type &index, index_type i) const 
    { index = i; }
  void to_index(typename Cell::index_type &index, index_type i) const 
    { index = i; }

  //! Get the child elements of the given index.
  void get_nodes(typename Node::array_type &array, 
                 typename Node::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_nodes(typename Node::array_type &array, 
                 typename Edge::index_type idx) const
    { get_nodes_from_edge(array,idx); }
  void get_nodes(typename Node::array_type &array, 
                 typename Face::index_type idx) const
    { get_nodes_from_face(array,idx); }
  void get_nodes(typename Node::array_type &array, 
                 typename Cell::index_type idx) const
    { ASSERTFAIL("QuadSurfMesh: get_nodes has not been implemented for cells"); }

  void get_edges(typename Edge::array_type &array, 
                 typename Node::index_type idx) const
    { get_edges_from_node(array,idx); }
  void get_edges(typename Edge::array_type &array, 
                 typename Edge::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_edges(typename Edge::array_type &array, 
                 typename Face::index_type idx) const
    { get_edges_from_face(array,idx); }
  void get_edges(typename Edge::array_type &array, 
                 typename Cell::index_type idx) const
    { ASSERTFAIL("QuadSurfMesh: get_edges has not been implemented for cells"); }

  void get_faces(typename Face::array_type &array, 
                 typename Node::index_type idx) const
    { get_faces_from_node(array,idx); }
  void get_faces(typename Face::array_type &array, 
                 typename Edge::index_type idx) const
    { get_faces_from_edge(array,idx); }
  void get_faces(typename Face::array_type &array, 
                 typename Face::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_faces(typename Face::array_type &array, 
                 typename Cell::index_type idx) const
    { ASSERTFAIL("QuadSurfMesh: get_faces has not been implemented for cells"); }

  void get_cells(typename Cell::array_type &array, 
                 typename Node::index_type idx) const
    { ASSERTFAIL("QuadSurfMesh: get_cells has not been implemented"); }
  void get_cells(typename Cell::array_type &array, 
                 typename Edge::index_type idx) const
    { ASSERTFAIL("QuadSurfMesh: get_cells has not been implemented"); }
  void get_cells(typename Cell::array_type &array, 
                 typename Face::index_type idx) const
    { ASSERTFAIL("QuadSurfMesh: get_cells has not been implemented"); }
  void get_cells(typename Cell::array_type &array, 
                 typename Cell::index_type idx) const
    { ASSERTFAIL("QuadSurfMesh: get_cells has not been implemented"); }

  void get_elems(typename Elem::array_type &array, 
                 typename Node::index_type idx) const
    { get_faces_from_node(array,idx); }
  void get_elems(typename Elem::array_type &array, 
                 typename Edge::index_type idx) const
    { get_faces_from_edge(array,idx); }
  void get_elems(typename Elem::array_type &array, 
                 typename Face::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_elems(typename Face::array_type &array, 
                 typename Cell::index_type idx) const
    { ASSERTFAIL("QuadSurfMesh: get_elems has not been implemented for cells"); }

  void get_delems(typename DElem::array_type &array, 
                  typename Node::index_type idx) const
    { get_edges_from_node(array,idx); }
  void get_delems(typename DElem::array_type &array, 
                  typename Edge::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_delems(typename DElem::array_type &array, 
                  typename Face::index_type idx) const
    { get_edges_from_face(array,idx); }
  void get_delems(typename DElem::array_type &array, 
                  typename Cell::index_type idx) const
    { ASSERTFAIL("QuadSurfMesh: get_delems has not been implemented for cells"); }

  //! Generate the list of points that make up a sufficiently accurate
  //! piecewise linear approximation of an edge.
  template<class VECTOR, class INDEX>
  void pwl_approx_edge(vector<VECTOR > &coords,
                       INDEX ci,
                       unsigned int which_edge,
                       unsigned int div_per_unit) const
  {
    basis_.approx_edge(which_edge, div_per_unit, coords);
  }

  //! Generate the list of points that make up a sufficiently accurate
  //! piecewise linear approximation of an face.
  template<class VECTOR, class INDEX>
  void pwl_approx_face(vector<vector<VECTOR > > &coords,
                       INDEX ci,
                       unsigned int which_face,
                       unsigned int div_per_unit) const
  {
    basis_.approx_face(which_face, div_per_unit, coords);
  }  

  //! get the center point (in object space) of an element  
  void get_center(Point &result, typename Node::index_type idx) const
    { get_node_center(result, idx); }
  void get_center(Point &result, typename Edge::index_type idx) const
    { get_edge_center(result, idx); }
  void get_center(Point &result, typename Face::index_type idx) const
    { get_face_center(result, idx); }
  void get_center(Point &result, typename Cell::index_type idx) const
    { ASSERTFAIL("QuadSurfMesh: get_cneter has not been implemented for cells"); }

  //! Get the size of an elemnt (length, area, volume)
  double get_size(typename Node::index_type /*idx*/) const 
    { return 0.0; }
  double get_size(typename Edge::index_type idx) const
    {
      typename Node::array_type arr;
      get_nodes_from_edge(arr, idx);
      return (point(arr[0]).asVector() - point(arr[1]).asVector()).length();
    }
  double get_size(typename Face::index_type idx) const
    {
      typename Node::array_type ra;
      get_nodes_from_face(ra,idx);
      const Point &p0 = point(ra[0]);
      const Point &p1 = point(ra[1]);
      const Point &p2 = point(ra[2]);
      const Point &p3 = point(ra[3]);
      return ((Cross(p0-p1,p2-p1)).length()+(Cross(p2-p3,p0-p3)).length()+
              (Cross(p3-p0,p1-p0)).length()+(Cross(p1-p2,p3-p2)).length())*0.25;
    }
  double get_size(typename Cell::index_type idx) const 
    { return 0.0; }
    
  //! More specific names for get_size    
  double get_length(typename Edge::index_type idx) const 
    { return get_size(idx); }
  double get_area(typename Face::index_type idx) const 
    { return get_size(idx); }
  double get_volume(typename Cell::index_type idx) const 
    { return 0.0; }


  //! Get neighbors of an element or a node
  
  //! THIS ONE IS FLAWED AS IN 3D SPACE MULTIPLE EDGES CAN CONNECTED THROUGH
  //! ONE EDGE
  bool get_neighbor(typename Elem::index_type &neighbor,
                    typename Elem::index_type elem,
                    typename DElem::index_type delem) const
    { return(get_elem_neighbor(neighbor,elem,delem)); } 

  //! These are more general implementations                      
  void get_neighbors(vector<typename Node::index_type> &array,
                     typename Node::index_type node) const
    { get_node_neighbors(array,node); }
  bool get_neighbors(vector<typename Elem::index_type> &array,
                     typename Elem::index_type elem,
                     typename DElem::index_type delem) const 
    { return(get_elem_neighbors(array,elem,delem)); }                     
  void get_neighbors(typename Elem::array_type &array,
                     typename Elem::index_type elem) const
    { get_elem_neighbors(array,elem); }
    
    
  //! Locate a point in a mesh, find which is the closest node
  bool locate(typename Node::index_type &loc, const Point &p) const;
  bool locate(typename Edge::index_type &loc, const Point &p) const;
  bool locate(typename Face::index_type &loc, const Point &p) const;
  bool locate(typename Cell::index_type &loc, const Point &p) const
    { ASSERTFAIL("QuadSurfMesh: locate(cells) is not supported."); }

  //! These should become obsolete soon, they do not follow the concept
  //! of the basis functions....
  int get_weights(const Point &p, typename Node::array_type &l, double *w);
  int get_weights(const Point & , typename Edge::array_type & , double * )
    { ASSERTFAIL("QuadSurfMesh: get_weights(edges) is not supported."); }
  int get_weights(const Point &p, typename Face::array_type &l, double *w);
  int get_weights(const Point & , typename Cell::array_type & , double * )
    { ASSERTFAIL("QuadSurfMesh: get_weights(cells) is not supported."); }

  //! Access the nodes of the mesh
  void get_point(Point &p, typename Node::index_type i) const 
    { p = points_[i]; }
  void set_point(const Point &p, typename Node::index_type i)
    { points_[i] = p; }

  void get_random_point(Point &, typename Elem::index_type, FieldRNG &rng) const;
  
  //! Normals for visualizations
  void get_normal(Vector &n, typename Node::index_type i) const 
  { 
    ASSERTMSG(synchronized_ & Mesh::NORMALS_E,
	      "Must call synchronize NORMALS_E on QuadSurfMesh first");   
    n = normals_[i]; 
  }

  //! Get the normals at the outside of the element
  template<class VECTOR, class INDEX1, class INDEX2>
  void get_normal(Vector &result, VECTOR &coords,
                  INDEX1 eidx,INDEX2 fidx)
  {
/*
 * THE ISSUE IS WHAT THE NORMAL IS AT THE NODE, THIS FUNCTION TAKES AN ELEMENT
 * INDEX AS WEL, HENCE IT SHOULD RETURN THE NORMAL OF THAT FACE, NOT AN AVERAGED
 * ONE OVER NEIGHBORING FACES

    if (basis_.polynomial_order() < 2) 
    {
      ASSERTMSG(synchronized_ & Mesh::NORMALS_E, "Must call synchronize NORMALS_E on TriSurfMesh first");

      typename Node::array_type arr(3);
      get_nodes_from_face(arr, eidx);

      const double c0_0 = fabs(coords[0]);
      const double c1_0 = fabs(coords[1]);
      const double c0_1 = fabs(coords[0] - 1.0L);
      const double c1_1 = fabs(coords[1] - 1.0L);

      if (c0_0 < 1e-7 && c1_0 < 1e-7) {
        // arr[0]
        result = normals_[arr[0]];
        return;
      } else if (c0_1 < 1e-7 && c1_0 < 1e-7) {
        // arr[1]
        result = normals_[arr[1]];
        return;
      } else if (c0_1 < 1e-7 && c1_1 < 1e-7) {
        // arr[2]
        result = normals_[arr[2]];
        return;
      } else if (c0_0 < 1e-7 && c1_1 < 1e-7) {
        // arr[3]
        result = normals_[arr[3]];
        return;
      }
    }
*/

    ElemData ed(*this, eidx);
    vector<Point> Jv;
    basis_.derivate(coords, ed, Jv);
    result = Cross(Jv[0].asVector(), Jv[1].asVector());
    result.normalize();
  }

  //! Add a new node to the mesh
  typename Node::index_type add_point(const Point &p);
  typename Node::index_type add_node(const Point &p)
    { return(add_point(p)); }

  //! Add a new element to the mesh
  template <class ARRAY>
  typename Elem::index_type add_elem(ARRAY a)
  {
    ASSERTMSG(a.size() == 4, "Tried to add non-quad element.");
    ASSERTMSG(order_face_nodes(a[0],a[1],a[2],a[3]), "add_elem: element that is being added is invalid");

    faces_.push_back(static_cast<typename Node::index_type>(a[0]));
    faces_.push_back(static_cast<typename Node::index_type>(a[1]));
    faces_.push_back(static_cast<typename Node::index_type>(a[2]));
    faces_.push_back(static_cast<typename Node::index_type>(a[3]));
    return static_cast<typename Elem::index_type>((static_cast<index_type>(faces_.size()) - 1) >> 2);
  }

  void node_reserve(size_type s) { points_.reserve(static_cast<size_t>(s)); }
  void elem_reserve(size_type s) { faces_.reserve(static_cast<size_t>(s*4)); }
  void resize_nodes(size_type s) { points_.resize(static_cast<size_t>(s)); }
  void resize_elems(size_type s) { faces_.resize(static_cast<size_t>(s*4)); }


  //! Get the local coordinates for a certain point within an element
  //! This function uses a couple of newton iterations to find the local
  //! coordinate of a point
  template<class VECTOR, class INDEX>
  bool get_coords(VECTOR &coords, const Point &p, INDEX idx) const
  {
    ElemData ed(*this, idx);
    return basis_.get_coords(coords, p, ed);
  }

  //! Find the location in the global coordinate system for a local coordinate
  //! This function is the opposite of get_coords.
  template<class VECTOR, class INDEX>
  void interpolate(Point &pt, const VECTOR &coords, INDEX idx) const
  {
    ElemData ed(*this, idx);
    pt = basis_.interpolate(coords, ed);
  }

  //! Interpolate the derivate of the function, This infact will return the
  //! jacobian of the local to global coordinate transformation. This function
  //! is mainly intended for the non linear elements
  template<class VECTOR1, class INDEX, class VECTOR2>
  void derivate(const VECTOR1 &coords, INDEX idx, VECTOR2 &J) const
  {
    ElemData ed(*this, idx);
    basis_.derivate(coords, ed, J);
  }

  //! Get the determinant of the jacobian, which is the local volume of an element
  //! and is intended to help with the integration of functions over an element.
  template<class VECTOR, class INDEX>
  double det_jacobian(const VECTOR& coords, INDEX idx) const
  {
    double J[9];
    jacobian(coords,idx,J);
    return (DetMatrix3x3(J));
  }

  //! Get the jacobian of the transformation. In case one wants the non inverted
  //! version of this matrix. This is currentl here for completeness of the 
  //! interface
  template<class VECTOR, class INDEX>
  void jacobian(const VECTOR& coords, INDEX idx, double* J) const
  {
    StackVector<Point,2> Jv;
    ElemData ed(*this,idx);
    basis_.derivate(coords,ed,Jv);
    Vector Jv2 = Cross(Jv[0].asVector(),Jv[1].asVector());
    Jv2.normalize();
    J[0] = Jv[0].x();
    J[1] = Jv[0].y();
    J[2] = Jv[0].z();
    J[3] = Jv[1].x();
    J[4] = Jv[1].y();
    J[5] = Jv[1].z();
    J[6] = Jv2.x();
    J[7] = Jv2.y();
    J[8] = Jv2.z();
  }

  //! Get the inverse jacobian of the transformation. This one is needed to 
  //! translate local gradients into global gradients. Hence it is crucial for
  //! calculating gradients of fields, or constructing finite elements.             
  template<class VECTOR, class INDEX>                
  double inverse_jacobian(const VECTOR& coords, INDEX idx, double* Ji) const
  {
    StackVector<Point,2> Jv;
    ElemData ed(*this,idx);
    basis_.derivate(coords,ed,Jv);
    double J[9];
    Vector Jv2 = Cross(Jv[0].asVector(),Jv[1].asVector());
    Jv2.normalize();
    J[0] = Jv[0].x();
    J[1] = Jv[0].y();
    J[2] = Jv[0].z();
    J[3] = Jv[1].x();
    J[4] = Jv[1].y();
    J[5] = Jv[1].z();
    J[6] = Jv2.x();
    J[7] = Jv2.y();
    J[8] = Jv2.z();
    
    return (InverseMatrix3x3(J,Ji));
  }


  template<class INDEX>
  double scaled_jacobian_metric(INDEX idx) const
  {
    StackVector<Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;

    basis_.derivate(basis_.unit_center,ed,Jv);
    Jv.resize(3); 
    Vector v = Cross(Jv[0].asVector(),Jv[1].asVector()); v.normalize();
    Jv[2] = v.asPoint();
    double min_jacobian = ScaledDetMatrix3P(Jv);
    
    size_t num_vertices = basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      basis_.derivate(basis_.unit_vertices[j],ed,Jv);
      Jv.resize(3); 
      v = Cross(Jv[0].asVector(),Jv[1].asVector()); v.normalize();
      Jv[2] = v.asPoint();
      temp = ScaledDetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }
      
    return (min_jacobian);
  }


  template<class INDEX>
  double jacobian_metric(INDEX idx) const
  {
    StackVector<Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;

    basis_.derivate(basis_.unit_center,ed,Jv);
    Jv.resize(3); 
    Vector v = Cross(Jv[0].asVector(),Jv[1].asVector()); v.normalize();
    Jv[2] = v.asPoint();
    double min_jacobian = DetMatrix3P(Jv);
    
    size_t num_vertices = basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      basis_.derivate(basis_.unit_vertices[j],ed,Jv);
      Jv.resize(3); 
      v = Cross(Jv[0].asVector(),Jv[1].asVector()); v.normalize();
      Jv[2] = v.asPoint();
      temp = DetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }
      
    return (min_jacobian);
  }

  //! This function will find the closest element and the location on that
  //! element that is the closest

  template <class INDEX>
  double find_closest_elem(Point &result, INDEX &face, const Point &p) const
  {
    // Walking the grid like this works really well if we're near the
    // surface.  It's degenerately bad if for example the point is
    // placed in the center of a sphere (because then we still have to
    // test all the faces, but with the grid overhead and triangle
    // duplication as well).
    ASSERTMSG(synchronized_ & LOCATE_E,
              "QuadSurfMesh::find_closest_elem requires synchronize(LOCATE_E).")

    // Convert to grid coordinates.
    index_type oi, oj, ok;
    grid_->unsafe_locate(oi, oj, ok, p);

    // Clamp to closest point on the grid.
    oi = Max(Min(oi, grid_->get_ni()-1), static_cast<index_type>(0));
    oj = Max(Min(oj, grid_->get_nj()-1), static_cast<index_type>(0));
    ok = Max(Min(ok, grid_->get_nk()-1), static_cast<index_type>(0));

    index_type bi, ei, bj, ej, bk, ek;
    bi = ei = oi;
    bj = ej = oj;
    bk = ek = ok;
    
    double dmin = DBL_MAX;
    bool found;
    do {
      const index_type bii = Max(bi, static_cast<index_type>(0));
      const index_type eii = Min(ei, grid_->get_ni()-1);
      const index_type bjj = Max(bj, static_cast<index_type>(0));
      const index_type ejj = Min(ej, grid_->get_nj()-1);
      const index_type bkk = Max(bk, static_cast<index_type>(0));
      const index_type ekk = Min(ek, grid_->get_nk()-1);
      found = false;
      for (index_type i = bii; i <= eii; i++)
      {
        for (index_type j = bjj; j <= ejj; j++)
        {
          for (index_type k = bkk; k <= ekk; k++)
          {
            if (i == bi || i == ei || j == bj || j == ej || k == bk || k == ek)
            {
              if (grid_->min_distance_squared(p, i, j, k) < dmin)
              {
                found = true;
                const list<index_type> *candidates;
                grid_->lookup_ijk(candidates, i, j, k);

                list<index_type>::const_iterator iter = candidates->begin();
                while (iter != candidates->end())
                {
                  Point rtmp;
                  index_type idx = *iter * 4;
                  est_closest_point_on_quad(rtmp, p,
                                       points_[faces_[idx  ]],
                                       points_[faces_[idx+1]],
                                       points_[faces_[idx+2]],
                                       points_[faces_[idx+3]]);
                  const double dtmp = (p - rtmp).length2();
                  if (dtmp < dmin)
                  {
                    result = rtmp;
                    face = INDEX(*iter);
                    dmin = dtmp;
                  }
                  ++iter;
                }
              }
            }
          }
        }
      }
      bi--;ei++;
      bj--;ej++;
      bk--;ek++;
    } 
    while (found) ;

    // As we computed an estimate, we use the Newton's method in the basis functions
    // compute a more refined solution. This function may slow down computation.
    // This piece of code will calculate the coordinates in the local element framework
    // (the newton's method of finding a minimum), then it will project this back
    // THIS CODE SHOULD BE FURTHER OPTIMIZED

    vector<double> coords;
    ElemData ed(*this,face);
    basis_.get_coords(coords,result,ed);
   
    result = basis_.interpolate(coords,ed);
    dmin = (result-p).length2();

    return sqrt(dmin);
  }


  template <class ARRAY>
  double find_closest_elems(Point &result, ARRAY &faces, const Point &p) const
  {
    // Walking the grid like this works really well if we're near the
    // surface.  It's degenerately bad if for example the point is
    // placed in the center of a sphere (because then we still have to
    // test all the faces, but with the grid overhead and triangle
    // duplication as well).
    ASSERTMSG(synchronized_ & LOCATE_E,
              "QuadSurfMesh::find_closest_elem requires synchronize(LOCATE_E).")

    // Convert to grid coordinates.
    index_type oi, oj, ok;
    grid_->unsafe_locate(oi, oj, ok, p);

    // Clamp to closest point on the grid.
    oi = Max(Min(oi, grid_->get_ni()-1), static_cast<index_type>(0));
    oj = Max(Min(oj, grid_->get_nj()-1), static_cast<index_type>(0));
    ok = Max(Min(ok, grid_->get_nk()-1), static_cast<index_type>(0));

    index_type bi, ei, bj, ej, bk, ek;
    bi = ei = oi;
    bj = ej = oj;
    bk = ek = ok;
    
    double dmin = DBL_MAX;
    bool found;
    do {
      const index_type bii = Max(bi, static_cast<index_type>(0));
      const index_type eii = Min(ei, grid_->get_ni()-1);
      const index_type bjj = Max(bj, static_cast<index_type>(0));
      const index_type ejj = Min(ej, grid_->get_nj()-1);
      const index_type bkk = Max(bk, static_cast<index_type>(0));
      const index_type ekk = Min(ek, grid_->get_nk()-1);
      found = false;
      for (index_type i = bii; i <= eii; i++)
      {
        for (index_type j = bjj; j <= ejj; j++)
        {
          for (index_type k = bkk; k <= ekk; k++)
          {
            if (i == bi || i == ei || j == bj || j == ej || k == bk || k == ek)
            {
              if (grid_->min_distance_squared(p, i, j, k) < dmin)
              {
                found = true;
                const list<index_type> *candidates;
                grid_->lookup_ijk(candidates, i, j, k);

                list<index_type>::const_iterator iter = candidates->begin();
                while (iter != candidates->end())
                {
                  Point rtmp;
                  index_type idx = *iter * 4;
                  est_closest_point_on_quad(rtmp, p,
                                       points_[faces_[idx  ]],
                                       points_[faces_[idx+1]],
                                       points_[faces_[idx+2]],
                                       points_[faces_[idx+3]]);
                  const double dtmp = (p - rtmp).length2();
                  if (dtmp < dmin - MIN_ELEMENT_VAL)
                  {
                    faces.clear();
                    result = rtmp;
                    faces.push_back(*iter);
                    dmin = dtmp;
                  }
                  else if (dtmp < dmin + MIN_ELEMENT_VAL)
                  {
                    faces.push_back(typename ARRAY::value_type(*iter));
                  }
                  ++iter;
                }
              }
            }
          }
        }
      }
      bi--;ei++;
      bj--;ej++;
      bk--;ek++;
    } 
    while (found) ;


    // As we computed an estimate, we use the Newton's method in the basis functions
    // compute a more refined solution. This function may slow down computation.
    // This piece of code will calculate the coordinates in the local element framework
    // (the newton's method of finding a minimum), then it will project this back
    // THIS CODE SHOULD BE FURTHER OPTIMIZED

    if (faces.size() == 1)
    {
      // if the number of faces is more then one the point we found is located
      // on the node or on the edge, which means the estimate is correct.
      vector<double> coords;
      ElemData ed(*this,faces[0]);
      basis_.get_coords(coords,result,ed);
      result = basis_.interpolate(coords,ed);
      dmin = (result-p).length2();
    }
    
    return sqrt(dmin);
  }

  double get_epsilon() const
    { return (epsilon_); }

  ///////////////////////////////////////////////////
  // STATIC VARIABLES AND FUNCTIONS
  
  //! Export this class using the old Pio system
  virtual void io(Piostream&);

  //! This ID is created as soon as this class will be instantiated  
  static PersistentTypeID quadsurfmesh_typeid;
  
  //! Core functionality for getting the name of a templated mesh class
  static  const string type_name(int n = -1);
  
  //! Type description, used for finding names of the mesh class for
  //! dynamic compilation purposes. Soem of this should be obsolete   
  virtual const TypeDescription *get_type_description() const;
  static const TypeDescription* node_type_description();
  static const TypeDescription* edge_type_description();
  static const TypeDescription* face_type_description();
  static const TypeDescription* cell_type_description();
  static const TypeDescription* elem_type_description()
    { return face_type_description(); }

  //! This function returns a maker for Pio.
  static Persistent *maker() { return scinew QuadSurfMesh<Basis>(); }
  //! This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return scinew QuadSurfMesh<Basis>(); }


  //////////////////////////////////////////////////////////////////
  // Mesh specific functions (these are not implemented in every mesh)
 
  bool inside4_p(typename Face::index_type i, const Point &p) const;

  // Extra functionality needed by this specific geometry.
  typename Node::index_type add_find_point(const Point &p,
                                           double err = 1.0e-3);
  typename Elem::index_type add_quad(typename Node::index_type a,
                            typename Node::index_type b,
                            typename Node::index_type c,
                            typename Node::index_type d);
  typename Elem::index_type add_quad(const Point &p0, const Point &p1,
                const Point &p2, const Point &p3);
 
protected:

  //////////////////////////////////////////////////////////////
  // These functions are templates and are used to define the
  // dynamic compilation interface and the virtual interface
  // as they both use different datatypes as indices and arrays
  // the following functions have been templated and are inlined
  // at the places where they are needed.
  //
  // Secondly these templates allow for the use of the stack vector
  // as well as the STL vector. When an algorithm supports non linear
  // functions an STL vector is a better choice, in the other cases
  // often a StackVector is enough (The latter improves performance).

     
  template<class ARRAY, class INDEX>
  inline void get_nodes_from_edge(ARRAY& array, INDEX i) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "QuadSurfMesh: Must call synchronize EDGES_E on QuadSurfMesh first");
    static int table[4][2] = { {0, 1}, {1, 2}, {2, 3}, {3, 0} };

    const index_type idx = edges_[i];
    const index_type off = idx % 4;
    const index_type node = idx - off;
    array.resize(2);
    array[0] = static_cast<typename ARRAY::value_type>(faces_[node + table[off][0]]);
    array[1] = static_cast<typename ARRAY::value_type>(faces_[node + table[off][1]]);
  }
  
  
  template<class ARRAY, class INDEX>
  inline void get_nodes_from_face(ARRAY& array, INDEX idx) const
  {  
    array.resize(4);
    array[0] = static_cast<typename ARRAY::value_type>(faces_[idx * 4 + 0]);
    array[1] = static_cast<typename ARRAY::value_type>(faces_[idx * 4 + 1]);
    array[2] = static_cast<typename ARRAY::value_type>(faces_[idx * 4 + 2]);
    array[3] = static_cast<typename ARRAY::value_type>(faces_[idx * 4 + 3]);
    order_face_nodes(array[0],array[1],array[2],array[3]);
  }

  template<class ARRAY, class INDEX>
  inline void get_nodes_from_elem(ARRAY& array, INDEX idx) const  
  {
    get_nodes_from_face(array,idx);
  }
  
  template<class ARRAY, class INDEX>
  inline void get_edges_from_face(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "QuadSurfMesh: Must call synchronize EDGES_E on QuadSurfMesh first");

    array.clear();

    index_type i;
    for (i=0; i < 4; i++)
    {
      const index_type a = idx * 4 + i;
      const index_type b = a - a % 4 + (a+1) % 4;
      index_type j = static_cast<index_type>(edges_.size()-1);
      for (; j >= 0; j--)
      {
        const index_type c = edges_[j];
        const index_type d = c - c % 4 + (c+1) % 4;
        if (faces_[a] == faces_[c] && faces_[b] == faces_[d] ||
            faces_[a] == faces_[d] && faces_[b] == faces_[c])
        {
          array.push_back(static_cast<typename ARRAY::value_type>(j));
          break;
        }
      }
    }
  }


  template<class ARRAY, class INDEX>
  inline void get_edges_from_elem(ARRAY& array, INDEX idx) const
  {
    get_edges_from_face(array,idx);
  }

  
  template <class ARRAY, class INDEX>
  inline void get_faces_from_node(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
	      "QuadSurfMesh: Must call synchronize NODE_NEIGHBORS_E on QuadSurfMesh first");
    array.resize(node_neighbors_[idx].size());    
    for (size_t i = 0; i < node_neighbors_[idx].size(); ++i)
      array[i] = static_cast<typename ARRAY::value_type>(node_neighbors_[idx][i]);
  }
  
 
  template <class ARRAY, class INDEX>
  inline void get_faces_from_edge(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on QuadSurfMesh first");

    // Find the two nodes that make up the edge
    typename Node::array_type nodes;
    get_nodes_from_edge(nodes,idx);
    
    // Get the table of faces that are connected to the two nodes
    const vector<typename Face::index_type>& faces1  = node_neighbors_[nodes[0]];
    const vector<typename Face::index_type>& faces2  = node_neighbors_[nodes[1]];
    
    // clear array
    array.clear();
    // conservative estimate
    array.reserve(2);
    
    // Find a face that is in both tables and is not the element for which
    // we are looking for a neighbor
    size_t fs1 = faces1.size();
    size_t fs2 = faces2.size();
    for (size_t i=0; i<fs1; i++)
    {
      for (size_t j=0; j<fs2; j++)
      {
        if (faces1[i] == faces2[j])
        {
          // found one that meets criteria
          // There could be more, however this function call
          // does not deal with that case
          array.push_back(static_cast<typename ARRAY::value_type>(faces1[i]));
        }    
      }
    }
  }


  template<class ARRAY, class INDEX>
  inline void get_edges_from_node(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on QuadSurfMesh first");
    
    // Get the table of faces that are connected to the two nodes
    const vector<typename Face::index_type>& faces  = node_neighbors_[idx];
    array.clear();
    
    typename Edge::index_type edge;
    for (index_type i=0; i<static_cast<size_type>(faces.size()); i++)
    {
      for (index_type j=0; j<4; j++)
      {
        edge = halfedge_to_edge_[faces[i]*4+j]; 
        size_t k=0;
        for (; k<array.size(); k++)
          if (array[k] == edge) break;
        if (k == array.size()) array.push_back(edge);
      }
    }
  }

  //! This function has been rewritten to allow for non manifold surfaces to be 
  //! handled ok.
  template <class INDEX1, class INDEX2>
  bool get_elem_neighbor(INDEX1 &neighbor, INDEX1 elem, INDEX2 delem) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on QuadSurfMesh first");

    // Find the two nodes that make up the edge
    typename Node::array_type nodes;
    get_nodes_from_edge(nodes,delem);
    
    // Get the table of faces that are connected to the two nodes
    const vector<typename Face::index_type>& faces1  = node_neighbors_[nodes[0]];
    const vector<typename Face::index_type>& faces2  = node_neighbors_[nodes[1]];
    
    // Find a face that is in both tables and is not the element for which
    // we are looking for a neighbor
    size_t fs1 = faces1.size();
    size_t fs2 = faces2.size();
    for (size_t i=0; i<fs1; i++)
    {
      if (faces1[i] != elem)
      {
        for (size_t j=0; j<fs2; j++)
        {
          if (faces1[i] == faces2[j])
          {
            // found one that meets criteria
            // There could be more, however this function call
            // does not deal with that case
            neighbor = INDEX1(faces1[i]);
            return (true);
          }    
        }
      }
    }
    return (false);
  }

  template <class ARRAY,class INDEX1, class INDEX2>
  bool get_elem_neighbors(ARRAY &array, INDEX1 elem, INDEX2 delem) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on QuadSurfMesh first");

    // Find the two nodes that make up the edge
    typename Node::array_type nodes;
    get_nodes_from_edge(nodes,delem);
    
    // Get the table of faces that are connected to the two nodes
    const vector<typename Face::index_type>& faces1  = node_neighbors_[nodes[0]];
    const vector<typename Face::index_type>& faces2  = node_neighbors_[nodes[1]];
    
    // clear array
    array.clear();
    
    // Find a face that is in both tables and is not the element for which
    // we are looking for a neighbor
    size_t fs1 = faces1.size();
    size_t fs2 = faces2.size();
    for (size_t i=0; i<fs1; i++)
    {
      if (faces1[i] != elem)
      {
        for (size_t j=0; j<fs2; j++)
        {
          if (faces1[i] == faces2[j])
          {
            // found one that meets criteria
            // There could be more, however this function call
            // does not deal with that case
            array.push_back(static_cast<typename ARRAY::value_type>(faces1[i]));
          }    
        }
      }
    }
    return (array.size() > 0);
  }

  template <class ARRAY, class INDEX>
  void get_elem_neighbors(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on QuadSurfMesh first");
    
    typename Edge::array_type edges;
    get_edges_from_face(edges, idx);
    
    array.clear();
    array.reserve(edges.size());
    
    for (index_type i=0; i<static_cast<index_type>(edges.size()); i++)
    {
      typename ARRAY::value_type nbor;
      if (get_elem_neighbor(nbor, idx, i))
      {
        array.push_back(nbor);
      }
    }
  }


  template <class ARRAY, class INDEX>
  void get_node_neighbors(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on QuadSurfMesh first");
    // clear old contents
    array.clear();

    // Get all the neighboring elements
    typename Node::array_type nodes;
    const vector<typename Face::index_type>& faces  = node_neighbors_[idx]; 
    // Make a conservative estimate of the number of node neighbors
    array.reserve(2*faces.size());
     
    for (index_type i=0;i<static_cast<index_type>(faces.size());i++)
    {
      index_type base = faces[i]*4;
      for (index_type j=0;j<4;j++)
      {
        if (faces_[base+j] ==  idx) continue;
        size_t k=0;
        for (;k<array.size();k++)
          if (static_cast<typename ARRAY::value_type>(faces_[base+j]) == array[k]) break;
        if (k==array.size()) 
          array.push_back(static_cast<typename ARRAY::value_type>(faces_[base+j]));  
      }
    }
  }

  template <class INDEX>
  inline bool locate_node(INDEX &loc, const Point &p) const
  {
    typename Node::iterator bi, ei;
    begin(bi);
    end(ei);
    loc = 0;

    bool found = false;
    double mindist = 0.0;
    while (bi != ei)
    {
      const Point &center = point(*bi);
      const double dist = (p - center).length2();
      if (!found || dist < mindist)
      {
        loc = static_cast<INDEX>(*bi);
        mindist = dist;
        found = true;
      }
      ++bi;
    }
    return (found);
  }


  template <class INDEX>
  inline bool locate_edge(INDEX &loc, const Point &p) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "Must call synchronize EDGES_E on QuadSurfMesh first");

    typename Edge::iterator bi, ei;
    typename Node::array_type nodes;
    begin(bi);
    end(ei);
    loc = 0;

    bool found = false;
    double mindist = 0.0;
    while (bi != ei)
    {
      get_nodes(nodes,*bi);
      const double dist = distance_to_line2(p, points_[nodes[0]],
                                            points_[nodes[1]],epsilon_);
      if (!found || dist < mindist)
      {
        loc = static_cast<INDEX>(*bi);
        mindist = dist;
        found = true;
      }
      ++bi;
    }
    return (found);
  }


  template <class INDEX>
  inline bool locate_elem(INDEX &face, const Point &p) const
  {
    if (basis_.polynomial_order() > 1) return elem_locate(face, *this, p);

    ASSERTMSG(synchronized_ & Mesh::LOCATE_E,
              "QuadSurfMesh:: requires synchronization.");

    const list<index_type> *candidates;
    if (grid_.get_rep() == 0) return (false);
    
    if (grid_->lookup(candidates, p))
    {
      list<index_type>::const_iterator iter = candidates->begin();	
      list<index_type>::const_iterator iter_end = candidates->end();	
      while (iter != iter_end)
      {
        if (inside4_p(typename Face::index_type(*iter), p))
        {
          face = static_cast<INDEX>(*iter);
          return (true);
        }
        ++iter;
      }
    }
    return (false);
  }

  template <class INDEX>
  inline void get_node_center(Point &p, INDEX idx) const
  {
    p = points_[idx];     
  }

  template <class INDEX>
  inline void get_edge_center(Point &result, INDEX idx) const
  {
    typename Node::array_type arr;
    get_nodes_from_edge(arr, idx);
    result = points_[arr[0]];
    result.asVector() += points_[arr[1]].asVector();
    result.asVector() *= 0.5;
  }


  template <class INDEX>
  inline void get_face_center(Point &p, INDEX idx) const
  {
    // NEED TO OPTIMIZE THIS ONE
    typename Node::array_type nodes;
    get_nodes_from_face(nodes, idx);
    ASSERT(nodes.size() == 4);
    typename Node::array_type::iterator nai = nodes.begin();
    p = points_[*nai];
    ++nai;
    while (nai != nodes.end())
    {
      const Point &pp = points_[*nai];
      p.asVector() += pp.asVector();
      ++nai;
    }
    p.asVector() *= (1.0 / 4.0);
  }


  const Point &point(typename Node::index_type i) const { return points_[i]; }

  // These require the synchronize_lock_ to be held before calling.
  void                  compute_edges();
  void                  compute_normals();
  void                  compute_node_neighbors();
  void                  compute_grid();
  void                  compute_epsilon();

  template <class NODE>
  bool order_face_nodes(NODE& n1,NODE& n2, NODE& n3, NODE& n4) const
  {
    // Check for degenerate or misformed face
    // Opposite faces cannot be equal
    if ((n1 == n3)||(n2==n4)) return (false);

    // Face must have three unique identifiers otherwise it was condition
    // n1==n3 || n2==n4 would be met.
    
    if (n1==n2)
    {
      if (n3==n4) return (false); // this is a line not a face
      NODE t;
      // shift one position to left
      t = n1; n1 = n2; n2 = n3; n3 = n4; n4 = t; 
      return (true);
    }
    else if (n2 == n3)
    {
      if (n1==n4) return (false); // this is a line not a face
      NODE t;
      // shift two positions to left
      t = n1; n1 = n3; n3 = t; t = n2; n2 = n4; n4 = t;
      return (true);
    }
    else if (n3 == n4)
    {
      NODE t;
      // shift one positions to right
      t = n4; n4 = n3; n3 = n2; n2 = n1; n1 = t;    
      return (true);
    }
    else if (n4 == n1)
    {
      // proper order
      return (true);
    }
    else
    {
      if ((n1 < n2)&&(n1 < n3)&&(n1 < n4))
      {
        // proper order
        return (true);
      }
      else if ((n2 < n3)&&(n2 < n4))
      {
        NODE t;
        // shift one position to left
        t = n1; n1 = n2; n2 = n3; n3 = n4; n4 = t; 
        return (true);    
      }
      else if (n3 < n4)
      {
        NODE t;
        // shift two positions to left
        t = n1; n1 = n3; n3 = t; t = n2; n2 = n4; n4 = t;
        return (true);    
      }
      else
      {
        NODE t;
        // shift one positions to right
        t = n4; n4 = n3; n3 = n2; n2 = n1; n1 = t;    
        return (true);    
      }
    }
  }


  index_type next(index_type i) { return ((i%4)==3) ? (i-3) : (i+1); }
  index_type prev(index_type i) { return ((i%4)==0) ? (i+3) : (i-1); }

  //! array with all the points
  vector<Point>                         points_;
  //! array with the four nodes that make up a face
  vector<index_type>                    faces_;
  
  //! FOR EDGE -> NODES
  //! array with information from edge number (unique ones) to the node numbers 
  //! this one refers to the first node
  vector<index_type>                    edges_;
  //! FOR FACES -> EDGE NUMBER
  //! array with information from halfedge (computed directly from face) to the edge number
  vector<index_type>                    halfedge_to_edge_;  // halfedge->edge map
  
  typedef vector<vector<typename Elem::index_type> > NodeNeighborMap;
  NodeNeighborMap                       node_neighbors_;  
  
  vector<Vector>                        normals_; //! normalized per node
  LockingHandle<SearchGridConstructor>  grid_;

  Mutex                                 synchronize_lock_;
  mask_type                             synchronized_;
  Basis                                 basis_;
  double                                epsilon_;
  double                                epsilon2_;
  
  //! Pointer to virtual interface
  Handle<VMesh>                         vmesh_;

#ifdef HAVE_HASH_MAP
  struct edgehash
  {
    size_t operator()(const pair<index_type, index_type> &a) const
    {
#if defined(__ECC) || defined(_MSC_VER)
      hash_compare<int> hasher;
#else
      hash<int> hasher;
#endif
      return hasher(static_cast<int>(hasher(a.first) + a.second));
    }
#if defined(__ECC) || defined(_MSC_VER)

    static const size_t bucket_size = 4;
    static const size_t min_buckets = 8;

    bool operator()(const pair<index_type, index_type> &a, const pair<index_type, index_type> &b) const
    {
      return a.first < b.first || a.first == b.first && a.second < b.second;
    }
#endif
  };

  struct edgecompare
  {
    bool operator()(const pair<index_type, index_type> &a, const pair<index_type, index_type> &b) const
    {
      return a.first == b.first && a.second == b.second;
    }
  };

#else // HAVE_HASH_MAP

  struct edgecompare
  {
    bool operator()(const pair<index_type, index_type> &a, const pair<index_type, index_type> &b) const
    {
      return a.first < b.first || a.first == b.first && a.second < b.second;
    }
  };
#endif // HAVE_HASH_MAP
};


template <class Basis>
PersistentTypeID
QuadSurfMesh<Basis>::quadsurfmesh_typeid(type_name(-1), "Mesh",
                               QuadSurfMesh<Basis>::maker);

template <class Basis>
const string
QuadSurfMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const string name = type_name(0) + FTNS + type_name(1) + FTNE;
    return name;
  }
  else if (n == 0)
  {
    static const string nm("QuadSurfMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
QuadSurfMesh<Basis>::QuadSurfMesh()
  : points_(0),
    faces_(0),
    edges_(0),
    normals_(0),
    grid_(0),
    synchronize_lock_("QuadSurfMesh Lock"),
    synchronized_(Mesh::NODES_E | Mesh::FACES_E | Mesh::CELLS_E),
    epsilon_(0.0),
    epsilon2_(0.0)
{
  //! Initialize the virtual interface when the mesh is created
  vmesh_ = CreateVQuadSurfMesh(this);
}


template <class Basis>
QuadSurfMesh<Basis>::QuadSurfMesh(const QuadSurfMesh &copy)
  : points_(0),
    faces_(0),
    edges_(0),
    normals_(0),
    grid_(0),
    synchronize_lock_("QuadSurfMesh Lock"),
    synchronized_(Mesh::NODES_E | Mesh::FACES_E | Mesh::CELLS_E),
    epsilon_(copy.epsilon_),
    epsilon2_(copy.epsilon2_)
{
  QuadSurfMesh &lcopy = (QuadSurfMesh &)copy;
  
  //! We need to lock before we can copy these as these
  //! structures are generate dynamically when they are
  //! needed.  
  lcopy.synchronize_lock_.lock();

  points_ = copy.points_;

  edges_ = copy.edges_;
  halfedge_to_edge_ = copy.halfedge_to_edge_;
  synchronized_ |= copy.synchronized_ & Mesh::EDGES_E;

  faces_ = copy.faces_;

  node_neighbors_ = copy.node_neighbors_;
  synchronized_ |= copy.synchronized_ & Mesh::NODE_NEIGHBORS_E;

  normals_ = copy.normals_;
  synchronized_ |= copy.synchronized_ & Mesh::NORMALS_E;

  synchronized_ &= ~Mesh::LOCATE_E;
  if (copy.grid_.get_rep())
  {
    grid_ = scinew SearchGridConstructor(*(copy.grid_.get_rep()));
  }
  synchronized_ &= ~Mesh::LOCATE_E;

  synchronized_ |= copy.synchronized_ & Mesh::EPSILON_E;

  lcopy.synchronize_lock_.unlock();

  //! Create a new virtual interface for this copy
  //! all pointers have changed hence create a new
  //! virtual interface class
  vmesh_ = CreateVQuadSurfMesh(this); 
}


template <class Basis>
QuadSurfMesh<Basis>::~QuadSurfMesh()
{
}


template <class Basis>
BBox
QuadSurfMesh<Basis>::get_bounding_box() const
{
  BBox result;

  for (vector<Point>::size_type i = 0; i < points_.size(); i++)
  {
    result.extend(points_[i]);
  }

  return result;
}


template <class Basis>
void
QuadSurfMesh<Basis>::transform(const Transform &t)
{
  synchronize_lock_.lock();
  vector<Point>::iterator itr = points_.begin();
  vector<Point>::iterator eitr = points_.end();
  while (itr != eitr)
  {
    *itr = t.project(*itr);
    ++itr;
  }
  if (grid_.get_rep()) { grid_->transform(t); }
  synchronize_lock_.unlock();
}


template <class Basis>
void
QuadSurfMesh<Basis>::begin(typename QuadSurfMesh::Node::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E,
            "Must call synchronize NODES_E on QuadSurfMesh first");
  itr = 0;
}


template <class Basis>
void
QuadSurfMesh<Basis>::end(typename QuadSurfMesh::Node::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E,
            "Must call synchronize NODES_E on QuadSurfMesh first");
  itr = static_cast<size_type>(points_.size());
}


template <class Basis>
void
QuadSurfMesh<Basis>::begin(typename QuadSurfMesh::Edge::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on QuadSurfMesh first");
  itr = 0;
}


template <class Basis>
void
QuadSurfMesh<Basis>::end(typename QuadSurfMesh::Edge::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on QuadSurfMesh first");
  itr = static_cast<size_type>(edges_.size());
}


template <class Basis>
void
QuadSurfMesh<Basis>::begin(typename QuadSurfMesh::Face::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on QuadSurfMesh first");
  itr = 0;
}


template <class Basis>
void
QuadSurfMesh<Basis>::end(typename QuadSurfMesh::Face::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on QuadSurfMesh first");
  itr = static_cast<size_type>(faces_.size() / 4);
}


template <class Basis>
void
QuadSurfMesh<Basis>::begin(typename QuadSurfMesh::Cell::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::CELLS_E,
            "Must call synchronize CELLS_E on QuadSurfMesh first");
  itr = 0;
}


template <class Basis>
void
QuadSurfMesh<Basis>::end(typename QuadSurfMesh::Cell::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::CELLS_E,
            "Must call synchronize CELLS_E on QuadSurfMesh first");
  itr = 0;
}




template <class Basis>
bool
QuadSurfMesh<Basis>::locate(typename Node::index_type &loc,
                            const Point &p) const
{
 return (locate_node(loc,p));
}


template <class Basis>
bool
QuadSurfMesh<Basis>::locate(typename Edge::index_type &loc,
                            const Point &p) const
{
  return (locate_edge(loc,p));
}


template <class Basis>
bool
QuadSurfMesh<Basis>::locate(typename Face::index_type &face,
                            const Point &p) const
{
  return (locate_elem(face,p));
}


template <class Basis>
bool
QuadSurfMesh<Basis>::inside4_p(typename Face::index_type idx,
                               const Point &p) const
{
  for (int i = 0; i < 4; i+=2)
  {
    const Point &p0 = points_[faces_[idx*4 + ((i+0)%4)]];
    const Point &p1 = points_[faces_[idx*4 + ((i+1)%4)]];
    const Point &p2 = points_[faces_[idx*4 + ((i+2)%4)]];

    Vector v01(p0-p1);
    Vector v02(p0-p2);
    Vector v0(p0-p);
    Vector v1(p1-p);
    Vector v2(p2-p);
    const double a = Cross(v01, v02).length(); // area of the whole triangle (2x)
    const double a0 = Cross(v1, v2).length();  // area opposite p0
    const double a1 = Cross(v2, v0).length();  // area opposite p1
    const double a2 = Cross(v0, v1).length();  // area opposite p2
    const double s = a0+a1+a2;

    // For the point to be inside a CONVEX quad it must be inside one
    // of the four triangles that can be formed by using three of the
    // quad vertices and the point in question.
    if( fabs(s - a) < epsilon2_ && a > epsilon2_ ) 
    {
      return true;
    }
  }

  return false;
}


template <class Basis>
int
QuadSurfMesh<Basis>::get_weights(const Point &p, typename Face::array_type &l,
                                 double *w)
{
  typename Face::index_type idx;
  if (locate(idx, p))
  {
    l.resize(1);
    l[0] = idx;
    w[0] = 1.0;
    return 1;
  }
  return 0;
}


template <class Basis>
int
QuadSurfMesh<Basis>::get_weights(const Point &p, typename Node::array_type &l,
                                 double *w)
{
  typename Face::index_type idx;
  if (locate(idx, p))
  {
    get_nodes(l,idx);
    vector<double> coords(2);
    if (get_coords(coords, p, idx))
    {
      basis_.get_weights(coords, w);
      return basis_.dofs();
    }
  }
  return 0;
}


template <class Basis>
void
QuadSurfMesh<Basis>::get_random_point(Point &p,
                                      typename Elem::index_type ei,
                                      FieldRNG &rng) const
{
  const Point &a0 = points_[faces_[ei*4+0]];
  const Point &a1 = points_[faces_[ei*4+1]];
  const Point &a2 = points_[faces_[ei*4+2]];

  const Point &b0 = points_[faces_[ei*4+2]];
  const Point &b1 = points_[faces_[ei*4+3]];
  const Point &b2 = points_[faces_[ei*4+0]];

  const double aarea = Cross(a1 - a0, a2 - a0).length();
  const double barea = Cross(b1 - b0, b2 - b0).length();

  if (rng() * (aarea + barea) < aarea)
  {
    // Fold the quad sample into a triangle.
    double u = rng();
    double v = rng();
    if (u + v > 1.0) { u = 1.0 - u; v = 1.0 - v; }
    
    // Compute the position of the random point.
    p = a0+((a1-a0)*u)+((a2-a0)*v);
  }
  else
  {
    // Fold the quad sample into a triangle.
    double u = rng();
    double v = rng();
    if (u + v > 1.0) { u = 1.0 - u; v = 1.0 - v; }
    
    // Compute the position of the random point.
    p = b0+((b1-b0)*u)+((b2-b0)*v);
  }
}



template <class Basis>
bool
QuadSurfMesh<Basis>::synchronize(mask_type tosync)
{
  synchronize_lock_.lock();
 
  if (tosync & (Mesh::EDGES_E|Mesh::DELEMS_E) && !(synchronized_ & Mesh::EDGES_E))
  {
    compute_edges();
  }
  
  if (tosync & Mesh::NORMALS_E && !(synchronized_ & Mesh::NORMALS_E))
  {
    compute_normals();
  }
  
  if (tosync & (Mesh::NODE_NEIGHBORS_E|Mesh::ELEM_NEIGHBORS_E) 
        && !(synchronized_ & Mesh::NODE_NEIGHBORS_E))
  {
    compute_node_neighbors();
  }

  if (tosync & Mesh::LOCATE_E && !(synchronized_ & Mesh::LOCATE_E))
  {
    compute_epsilon();
    compute_grid();
  }

  if (tosync & Mesh::EPSILON_E && !(synchronized_ & Mesh::EPSILON_E))
  {
    compute_epsilon();
  }
  
  synchronize_lock_.unlock();
  return true;
}

template <class Basis>
bool
QuadSurfMesh<Basis>::unsynchronize(mask_type sync)
{
  return (true);
}


template <class Basis>
void
QuadSurfMesh<Basis>::compute_normals()
{
  normals_.resize(points_.size()); // 1 per node

  // build table of faces that touch each node
  vector<vector<typename Face::index_type> > node_in_faces(points_.size());
  //! face normals (not normalized) so that magnitude is also the area.
  vector<Vector> face_normals(faces_.size());
  // Computing normal per face.
  typename Node::array_type nodes(4);
  typename Face::iterator iter, iter_end;
  begin(iter);
  end(iter_end);
  while (iter != iter_end)
  {
    get_nodes(nodes, *iter);

    Point p0, p1, p2, p3;
    get_point(p0, nodes[0]);
    get_point(p1, nodes[1]);
    get_point(p2, nodes[2]);
    get_point(p3, nodes[3]);

    // build table of faces that touch each node
    node_in_faces[nodes[0]].push_back(*iter);
    node_in_faces[nodes[1]].push_back(*iter);
    node_in_faces[nodes[2]].push_back(*iter);
    node_in_faces[nodes[3]].push_back(*iter);

    Vector v0 = p1 - p0;
    Vector v1 = p2 - p1;
    Vector n = Cross(v0, v1);
    face_normals[*iter] = n;

    ++iter;
  }
  //Averaging the normals.
  typename vector<vector<typename Face::index_type> >::iterator nif_iter =
    node_in_faces.begin();
  index_type i = 0;
  while (nif_iter != node_in_faces.end()) 
  {
    const vector<typename Face::index_type> &v = *nif_iter;
    typename vector<typename Face::index_type>::const_iterator fiter =
      v.begin();
    Vector ave(0.L,0.L,0.L);
    while(fiter != v.end()) 
    {
      ave += face_normals[*fiter];
      ++fiter;
    }
    ave.safe_normalize();
    normals_[i] = ave; ++i;
    ++nif_iter;
  }
  synchronized_ |= Mesh::NORMALS_E;
}


template <class Basis>
typename QuadSurfMesh<Basis>::Node::index_type
QuadSurfMesh<Basis>::add_find_point(const Point &p, double err)
{
  typename Node::index_type i;
  if (locate(i, p) && (points_[i] - p).length2() < err)
  {
    return i;
  }
  else
  {
    points_.push_back(p);
    return static_cast<typename Node::index_type>
        (static_cast<size_type>(points_.size()) - 1);
  }
}


template <class Basis>
typename QuadSurfMesh<Basis>::Elem::index_type
QuadSurfMesh<Basis>::add_quad(typename Node::index_type a,
                              typename Node::index_type b,
                              typename Node::index_type c,
                              typename Node::index_type d)
{
  ASSERTMSG(order_face_nodes(a,b,c,d), "add_quad: element that is being added is invalid");
  faces_.push_back(a);
  faces_.push_back(b);
  faces_.push_back(c);
  faces_.push_back(d);
  return static_cast<typename Elem::index_type>((static_cast<index_type>(faces_.size()) - 1) >> 2);
}


#ifdef HAVE_HASH_MAP

struct edgehash
{
  size_t operator()(const pair<index_type, index_type> &a) const
  {
#if defined(__ECC) || defined(_MSC_VER)
    hash_compare<int> hasher;
#else
    hash<int> hasher;
#endif
    return hasher(static_cast<index_type>(hasher(a.first)) + a.second);
  }
#if defined(__ECC) || defined(_MSC_VER)

  static const size_t bucket_size = 4;
  static const size_t min_buckets = 8;

  bool operator()(const pair<index_type, index_type> &a, const pair<index_type, index_type> &b) const
  {
    return a.first < b.first || a.first == b.first && a.second < b.second;
  }
#endif
};

struct edgecompare
{
  bool operator()(const pair<index_type, index_type> &a, const pair<index_type, index_type> &b) const
  {
    return a.first == b.first && a.second == b.second;
  }
};

#else

struct edgecompare
{
  bool operator()(const pair<index_type, index_type> &a, const pair<index_type, index_type> &b) const
  {
    return a.first < b.first || a.first == b.first && a.second < b.second;
  }
};

#endif

#ifdef HAVE_HASH_MAP

#if defined(__ECC) || defined(_MSC_VER)
typedef hash_map<pair<index_type, index_type>, index_type, edgehash> EdgeMapType;
#else
typedef hash_map<pair<index_type, index_type>, index_type, edgehash, edgecompare> EdgeMapType;
#endif

#else

typedef map<pair<index_type, index_type>, index_type, edgecompare> EdgeMapType;

#endif

#ifdef HAVE_HASH_MAP

#if defined(__ECC) || defined(_MSC_VER)
typedef hash_map<pair<index_type, index_type>, list<index_type>, edgehash> EdgeMapType2;
#else
typedef hash_map<pair<index_type, index_type>, list<index_type>, edgehash, edgecompare> EdgeMapType2;
#endif

#else

typedef map<pair<index_type, index_type>, list<index_type>, edgecompare> EdgeMapType2;

#endif

template <class Basis>
void
QuadSurfMesh<Basis>::compute_edges()
{
  EdgeMapType2 edge_map;

  for( index_type i= static_cast<index_type>(faces_.size())-1; i >= 0; i--)
  {
    const index_type a = i;
    const index_type b = a - a % 4 + (a+1) % 4;

    index_type n0 = faces_[a];
    index_type n1 = faces_[b];
    index_type tmp;
    if (n0 > n1) { tmp = n0; n0 = n1; n1 = tmp; }

    if (n0 != n1)
    {
      pair<index_type, index_type> nodes(n0, n1);
      edge_map[nodes].push_front(i);
    }
  }

  typename EdgeMapType2::iterator itr;
  edges_.clear();
  edges_.reserve(edge_map.size());
  halfedge_to_edge_.resize(faces_.size());
  for (itr = edge_map.begin(); itr != edge_map.end(); ++itr)
  {
    edges_.push_back((*itr).second.front());

    list<index_type>::iterator litr = (*itr).second.begin();
    while (litr != (*itr).second.end())
    {
      halfedge_to_edge_[*litr] = edges_.size()-1;
      ++litr;
    }
  }

  synchronized_ |= Mesh::EDGES_E;
}


template <class Basis>
void
QuadSurfMesh<Basis>::compute_node_neighbors()
{
  node_neighbors_.clear();
  node_neighbors_.resize(points_.size());
  index_type i, num_elems = static_cast<index_type>(faces_.size());
  for (i = 0; i < num_elems; i++)
  {
    node_neighbors_[faces_[i]].push_back(i/4);
  }
  synchronized_ |= Mesh::NODE_NEIGHBORS_E;
}


template <class Basis>
void
QuadSurfMesh<Basis>::compute_grid()
{
  BBox bb = get_bounding_box();
  if (bb.valid())
  {
    // Cubed root of number of elems to get a subdivision ballpark.
    typename Elem::size_type csize;  size(csize);
    const index_type s = 
      static_cast<index_type>((static_cast<size_type>(ceil(pow((double)csize , 1.0/3.0)))) / 2 + 1);
    
    index_type sx, sy, sz; sx = sy = sz = s;
    Vector elem_epsilon = bb.diagonal() * (1.0e-4 / s);
    if (elem_epsilon.x() < MIN_ELEMENT_VAL)
    {
      elem_epsilon.x(MIN_ELEMENT_VAL * 100);
      sx = 1;
    }
    if (elem_epsilon.y() < MIN_ELEMENT_VAL)
    {
      elem_epsilon.y(MIN_ELEMENT_VAL * 100);
      sy = 1;
    }
    if (elem_epsilon.z() < MIN_ELEMENT_VAL)
    {
      elem_epsilon.z(MIN_ELEMENT_VAL * 100);
      sz = 1;
    }
    bb.extend(bb.min() - elem_epsilon * 10);
    bb.extend(bb.max() + elem_epsilon * 10);

    grid_ = scinew SearchGridConstructor(sx, sy, sz, bb.min(), bb.max());

    BBox box;
    typename Node::array_type nodes;
    typename Elem::iterator ci, cie;
    begin(ci); end(cie);
    while(ci != cie)
    {
      get_nodes(nodes, *ci);

      box.reset();
      for (index_type i = 0; i < static_cast<index_type>(nodes.size()); i++)
      {
        box.extend(points_[nodes[i]]);
      }
      const Point padmin(box.min() - elem_epsilon);
      const Point padmax(box.max() + elem_epsilon);
      box.extend(padmin);
      box.extend(padmax);

      grid_->insert(*ci, box);

      ++ci;
    }
  }

  synchronized_ |= Mesh::LOCATE_E;
}

template <class Basis>
void
QuadSurfMesh<Basis>::compute_epsilon()
{
  epsilon_ = get_bounding_box().diagonal().length()*1e-8;
  epsilon2_ = epsilon_*epsilon_;
  synchronized_ |= EPSILON_E;
}

template <class Basis>
typename QuadSurfMesh<Basis>::Node::index_type
QuadSurfMesh<Basis>::add_point(const Point &p)
{
  points_.push_back(p);
  return static_cast<typename Node::index_type>(static_cast<index_type>(points_.size() - 1));
}


template <class Basis>
typename QuadSurfMesh<Basis>::Elem::index_type
QuadSurfMesh<Basis>::add_quad(const Point &p0, const Point &p1,
                              const Point &p2, const Point &p3)
{
  return add_quad(add_find_point(p0), add_find_point(p1),
                  add_find_point(p2), add_find_point(p3));
}


#define QUADSURFMESH_VERSION 4
template <class Basis>
void
QuadSurfMesh<Basis>::io(Piostream &stream)
{
  const int version = stream.begin_class(type_name(-1), QUADSURFMESH_VERSION);

  Mesh::io(stream);

  Pio(stream, points_);
  Pio_index(stream, faces_);
  
  // In case the face is degenerate
  // move the degerenaracy to the end
  // this way the visualization works fine 
  if (version != 1)
  {
    if (stream.reading())
    {
      for (size_t i=0; i < faces_.size(); i += 4)
       if(!( order_face_nodes(faces_[i],faces_[i+1],faces_[i+2],faces_[i+3])))
         std::cerr << "Detected an invalid quadrilateral face\n";
    }
  }
  
  if (version == 1)
  {
    // We no longer save out this table, we actually no longer compute it
    // Hence when saving we save an empty table, which may break the old
    // SCIRun version (a version that was not working anyway)
    // When loading an old file we just ignore the table once it is loaded.
    vector<under_type> dummy;
    Pio(stream,dummy);
  }
  
  if (version >= 3) 
  {
    basis_.io(stream);
  }

  stream.end_class();

  if (stream.reading())
  {
    synchronized_ = Mesh::NODES_E | Mesh::FACES_E | Mesh::CELLS_E;
  }

  if (stream.reading())
    vmesh_ = CreateVQuadSurfMesh(this);
}


template <class Basis>
void
QuadSurfMesh<Basis>::size(typename QuadSurfMesh::Node::size_type &s) const
{
  typename Node::iterator itr; end(itr);
  s = *itr;
}


template <class Basis>
void
QuadSurfMesh<Basis>::size(typename QuadSurfMesh::Edge::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on QuadSurfMesh first");
  s = edges_.size();
}


template <class Basis>
void
QuadSurfMesh<Basis>::size(typename QuadSurfMesh::Face::size_type &s) const
{
  typename Face::iterator itr; end(itr);
  s = *itr;
}


template <class Basis>
void
QuadSurfMesh<Basis>::size(typename QuadSurfMesh::Cell::size_type &s) const
{
  typename Cell::iterator itr; end(itr);
  s = *itr;
}


template <class Basis>
const TypeDescription*
get_type_description(QuadSurfMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = SCIRun::get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = scinew TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = scinew TypeDescription("QuadSurfMesh", subs,
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
QuadSurfMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((QuadSurfMesh<Basis> *)0);
}


template <class Basis>
const TypeDescription*
QuadSurfMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((QuadSurfMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Node",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
QuadSurfMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((QuadSurfMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Edge",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
QuadSurfMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((QuadSurfMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Face",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
QuadSurfMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
   const TypeDescription *me =
      SCIRun::get_type_description((QuadSurfMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Cell",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}




} // namespace SCIRun

#endif 
