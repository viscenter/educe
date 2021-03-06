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

#ifndef CORE_DATATYPES_CURVEMESH_H
#define CORE_DATATYPES_CURVEMESH_H 1

//! Include what kind of support we want to have
//! Need to fix this and couple it sci-defs
#include <Core/Datatypes/MeshSupport.h>

#include <Core/Containers/StackVector.h>
#include <Core/Containers/LockingHandle.h>
#include <Core/Containers/Handle.h>

#include <Core/Geometry/BBox.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Transform.h>
#include <Core/Geometry/Vector.h>

#include <Core/Basis/Locate.h>
#include <Core/Basis/CrvLinearLgn.h>
#include <Core/Basis/CrvQuadraticLgn.h>
#include <Core/Basis/CrvCubicHmt.h>

#include <Core/Datatypes/FieldIterator.h>
#include <Core/Datatypes/FieldRNG.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/SearchGrid.h>
#include <Core/Datatypes/VMesh.h>

//! Incude needed for Windows: declares SCISHARE
#include <Core/Datatypes/share.h>

namespace SCIRun {

using std::string;
using std::pair;


/////////////////////////////////////////////////////
// Declarations for virtual interface

template <class Basis> class CurveMesh;

//! make sure any other mesh other than the preinstantiate ones
//! returns no virtual interface. Altering this behaviour will allow
//! for dynamically compiling the interfae if needed.
template<class MESH>
VMesh* CreateVCurveMesh(MESH* mesh) { return (0); }

#if (SCIRUN_CURVE_SUPPORT > 0)
//! Declare that these can be found in a library that is already
//! precompiled. So dynamic compilation will not instantiate them again.
SCISHARE VMesh* CreateVCurveMesh(CurveMesh<CrvLinearLgn<Point> >* mesh);
#if (SCIRUN_QUADRATIC_SUPPORT > 0)
SCISHARE VMesh* CreateVCurveMesh(CurveMesh<CrvQuadraticLgn<Point> >* mesh);
#endif
#if (SCIRUN_CUBIC_SUPPORT > 0)
SCISHARE VMesh* CreateVCurveMesh(CurveMesh<CrvCubicHmt<Point> >* mesh);
#endif

#endif
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
// Declarations for CurveMesh class

template <class Basis>
class CurveMesh : public Mesh
{

//! Make sure the virtual interface has access
template<class MESH> friend class VCurveMesh;
template<class MESH> friend class VMeshShared;
template<class MESH> friend class VUnstructuredMesh;

public:

  // Types that change depending on 32 or 64bits
  typedef SCIRun::index_type            under_type;
  typedef SCIRun::index_type            index_type;
  typedef SCIRun::size_type             size_type;
  typedef SCIRun::mask_type             mask_type; 
  
  typedef LockingHandle<CurveMesh<Basis> > handle_type;
  typedef Basis                            basis_type;

  //! Index and Iterator types required for Mesh Concept.
  struct Node {
    typedef NodeIndex<under_type>       index_type;
    typedef NodeIterator<under_type>    iterator;
    typedef NodeIndex<under_type>       size_type;
    typedef StackVector<index_type, 2>  array_type;
  };

  struct Edge {
    typedef EdgeIndex<under_type>       index_type;
    typedef EdgeIterator<under_type>    iterator;
    typedef EdgeIndex<under_type>       size_type;
    typedef vector<index_type>          array_type;
  };

  //! For dynamic compilation to be compatible with
  //! other mesh types
  struct Face {
    typedef FaceIndex<under_type>       index_type;
    typedef FaceIterator<under_type>    iterator;
    typedef FaceIndex<under_type>       size_type;
    typedef vector<index_type>          array_type;
  };

  //! For dynamic compilation to be compatible with
  //! other mesh types
  struct Cell {
    typedef CellIndex<under_type>       index_type;
    typedef CellIterator<under_type>    iterator;
    typedef CellIndex<under_type>       size_type;
    typedef vector<index_type>          array_type;
  };

  //! Elem refers to the most complex topological object
  //! DElem refers to object just below Elem in the topological hierarchy

  typedef Edge Elem;
  typedef Node DElem;

  //! Definition specific to this class (should at some point jsut
  //! remove this and make it similar to the other classes)
  typedef pair<typename Node::index_type,
               typename Node::index_type> index_pair_type;


  //! Somehow the information of how to interpolate inside an element
  //! ended up in a separate class, as they need to share information
  //! this construction was created to transfer data. 
  //! Hopefully in the future this class will disappear again.
  friend class ElemData;
  
  class ElemData
  {
  public:
    typedef typename CurveMesh<Basis>::index_type  index_type;

    ElemData(const CurveMesh<Basis>& msh, const index_type idx) :
      mesh_(msh),
      index_(idx)
    {}

    inline
    index_type node0_index() const {
      return mesh_.edges_[2*index_];
    }
    inline
    index_type node1_index() const {
      return mesh_.edges_[2*index_+1];
    }

    inline
    const Point &node0() const {
      return mesh_.points_[mesh_.edges_[2*index_]];
    }
    inline
    const Point &node1() const {
      return mesh_.points_[mesh_.edges_[2*index_+1]];
    }

    inline
    index_type edge0_index() const 
    {
      return index_;
    }

  private:
    //! reference of the mesh
    const CurveMesh<Basis>   &mesh_;
    //! copy of index
    const index_type         index_;
  };

  //////////////////////////////////////////////////////////////////

  //! Construct a new mesh
  CurveMesh();
  
  //! Copy a mesh, needed for detaching the mesh from a field 
  CurveMesh(const CurveMesh &copy);
  
  //! Clone function for detaching the mesh and automatically generating
  //! a new version if needed.
  virtual CurveMesh *clone() { return new CurveMesh(*this); }

  //! Destructor
  virtual ~CurveMesh(); 

  //! Obtain the virtual interface pointer
  virtual VMesh* vmesh() { return (vmesh_.get_rep()); }

  //! This one should go at some point, should be reroute throught the
  //! virtual interface
  virtual int basis_order() { return (basis_.polynomial_order()); }

  //! Topological dimension
  virtual int dimensionality() const { return 1; }
  
  //! What kind of mesh is this 
  //! structured = no connectivity data
  //! regular    = no node location data
  virtual int  topology_geometry() const 
    { return (Mesh::UNSTRUCTURED | Mesh::IRREGULAR); }

  //! Get the bounding box of the field
  virtual BBox get_bounding_box() const;
  
  //! Transform a field
  virtual void transform(const Transform &t);
  
  //! Check whether mesh can be altered by adding nodes or elements
  virtual bool is_editable() const { return (true); }

  //! Has this mesh normals.
  virtual bool has_normals() const { return (false); }  

  //! Compute tables for doing topology, these need to be synchronized
  //! before doing a lot of operations.
  virtual bool synchronize(mask_type sync);
  virtual bool unsynchronize(mask_type sync);

  //! Get the basis class
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
  
  //! Get the iteration sizes
  void size(typename Node::size_type &) const;
  void size(typename Edge::size_type &) const;
  void size(typename Face::size_type &) const;
  void size(typename Cell::size_type &) const;

  //! These are here to convert indices to unsigned int
  //! counters. Some how the decision was made to use multi
  //! dimensional indices in some fields, these functions
  //! should deal with different pointer types.
  //! Use the virtual interface to avoid all this non sense.
  inline void to_index(typename Node::index_type &index, index_type i) const 
    { index = i; }
  inline void to_index(typename Edge::index_type &index, index_type i) const 
    { index = i; }
  inline void to_index(typename Face::index_type &index, index_type i) const 
    { index = i; }
  inline void to_index(typename Cell::index_type &index, index_type i) const 
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
    { ASSERTFAIL("CurveMesh: get_nodes has not been implemented for faces"); }
  void get_nodes(typename Node::array_type &array, 
                 typename Cell::index_type idx) const
    { ASSERTFAIL("CurveMesh: get_nodes has not been implemented for cells"); }

  void get_edges(typename Edge::array_type &array, 
                 typename Node::index_type idx) const
    { get_edges_from_node(array,idx); }
  void get_edges(typename Edge::array_type &array,
                 typename Edge::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_edges(typename Edge::array_type &array, 
                 typename Face::index_type idx) const
    { ASSERTFAIL("CurveMesh: get_edges has not been implemented for faces"); }
  void get_edges(typename Edge::array_type &array, 
                 typename Cell::index_type idx) const
    { ASSERTFAIL("CurveMesh: get_edges has not been implemented for cells"); }

  void get_faces(typename Face::array_type &array, 
                 typename Node::index_type idx) const
    { ASSERTFAIL("CurveMesh: get_faces has not been implemented"); }
  void get_faces(typename Face::array_type &array,
                 typename Edge::index_type idx) const
    { ASSERTFAIL("CurveMesh: get_faces has not been implemented"); }
  void get_faces(typename Face::array_type &array, 
                 typename Face::index_type idx) const
    { ASSERTFAIL("CurveMesh: get_faces has not been implemented"); }
  void get_faces(typename Face::array_type &array, 
                 typename Cell::index_type idx) const
    { ASSERTFAIL("CurveMesh: get_faces has not been implemented"); }

  void get_cells(typename Cell::array_type &array, 
                 typename Node::index_type idx) const
    { ASSERTFAIL("CurveMesh: get_cells has not been implemented"); }
  void get_cells(typename Cell::array_type &array, 
                 typename Edge::index_type idx) const
    { ASSERTFAIL("CurveMesh: get_cells has not been implemented"); }
  void get_cells(typename Cell::array_type &array, 
                 typename Face::index_type idx) const
    { ASSERTFAIL("CurveMesh: get_cells has not been implemented"); }
  void get_cells(typename Cell::array_type &array, 
                 typename Cell::index_type idx) const
    { ASSERTFAIL("CurveMesh: get_cells has not been implemented"); }

  void get_elems(typename Elem::array_type &array, 
                 typename Node::index_type idx) const
    { get_edges_from_node(array,idx); }
  void get_elems(typename Elem::array_type &array, 
                 typename Edge::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_elems(typename Elem::array_type &array, 
                 typename Face::index_type idx) const
    { ASSERTFAIL("CurveMesh: get_elems has not been implemented for cells"); }
  void get_elems(typename Face::array_type &array,
                 typename Cell::index_type idx) const
    { ASSERTFAIL("CurveMesh: get_elems has not been implemented for cells"); }

  void get_delems(typename DElem::array_type &array, 
                  typename Node::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_delems(typename DElem::array_type &array, 
                  typename Edge::index_type idx) const
    { get_nodes_from_edge(array,idx); }
  void get_delems(typename DElem::array_type &array, 
                  typename Face::index_type idx) const
    { ASSERTFAIL("CurveMesh: get_delems has not been implemented for faces"); }
  void get_delems(typename DElem::array_type &array, 
                  typename Cell::index_type idx) const
    { ASSERTFAIL("CurveMesh: get_delems has not been implemented for cells"); }
    
                
  
  //! Generate the list of points that make up a sufficiently accurate
  //! piecewise linear approximation of an edge.
  template<class VECTOR>
  void pwl_approx_edge(VECTOR &coords,
                       typename Elem::index_type,
                       unsigned int which_edge,
                       unsigned int div_per_unit) const
  {
    // only one edge in the unit edge.
    basis_.approx_edge(which_edge, div_per_unit, coords);
  }

  //! Generate the list of points that make up a sufficiently accurate
  //! piecewise linear approximation of an face.
  template<class VECTOR>
  void pwl_approx_face(VECTOR &coords,
                       typename Elem::index_type ci,
                       typename Face::index_type fi,
                       unsigned int div_per_unit) const
  {
    ASSERTFAIL("CurveMesh: cannot approximate faces");
  }

  //! get the center point (in object space) of an element
  void get_center(Point &result, typename Node::index_type idx) const
    { result = points_[idx]; }
  void get_center(Point &, typename Edge::index_type) const;
  void get_center(Point &, typename Face::index_type) const
    { ASSERTFAIL("CurveMesh: get_center has not been implemented for faces"); }
  void get_center(Point &, typename Cell::index_type) const
    { ASSERTFAIL("CurveMesh: get_center has not been implemented for cells"); }

  //! Get the size of an element (length, area, volume)
  double get_size(typename Node::index_type /*idx*/) const 
    { return 0.0; }
  double get_size(typename Edge::index_type idx) const;
  double get_size(typename Face::index_type /*idx*/) const
    { return 0.0; }
  double get_size(typename Cell::index_type /*idx*/) const
    { return 0.0; }
 
  //! More specific names for get_size       
  double get_length(typename Edge::index_type idx) const
    { return get_size(idx); }
  double get_area(typename Face::index_type idx) const
    { return 0.0; }
  double get_volume(typename Cell::index_type idx) const
    { return 0.0; }

  //! Get neighbors of an element or a node
  //! THIS ONE IS FLAWED AS IN 3D SPACE  A NODE CAN BE CONNECTED TO
  //! MANY ELEMENTS
  bool get_neighbor(typename Elem::index_type &neighbor,
                    typename Elem::index_type edge,
                    typename DElem::index_type node) const  
  {
    return(get_elem_neighbor(neighbor,edge,node));
  }

  //! These are more general neighbor functions      
  void get_neighbors(vector<typename Node::index_type> &array,
                     typename Node::index_type idx) const
  {
    get_node_neighbors(array,idx);
  }
                     
  bool get_neighbors(vector<typename Elem::index_type> &array,
                     typename Elem::index_type edge,
                     typename DElem::index_type idx) const
  {
    return(get_elem_neighbors(array,edge,idx));
  }

  void get_neighbors(vector<typename Elem::index_type> &array,
                     typename Elem::index_type idx) const
  {
    get_elem_neighbors(array,idx);
  }

  //! Locate a point in a mesh, find which is the closest node
  bool locate(typename Node::index_type &i, const Point &p) const
    { return(locate_node(i,p)); }
  bool locate(typename Edge::index_type &i, const Point &p) const
    { return(locate_elem(i,p)); }
  bool locate(typename Face::index_type &, const Point &) const
    { return (false); }
  bool locate(typename Cell::index_type &, const Point &) const
    { return (false); }


  //! These should become obsolete soon, they do not follow the concept
  //! of the basis functions....
  int get_weights(const Point &p, typename Node::array_type &l, double *w);
  int get_weights(const Point &p, typename Edge::array_type &l, double *w);
  int get_weights(const Point &p, typename Face::array_type &l, double *w)
    { ASSERTFAIL("CurveMesh::get_weights for faces isn't supported"); }
  int get_weights(const Point &p, typename Cell::array_type &l, double *w)
    { ASSERTFAIL("CurveMesh::get_weights for cells isn't supported"); }

  //! Access the nodes of the mesh
  void get_point(Point &result, typename Node::index_type idx) const
    { get_center(result,idx); }
  void set_point(const Point &point, typename Node::index_type index)
    { points_[index] = point; }
  void get_random_point(Point &p, typename Elem::index_type i, FieldRNG &r) const;

  //! Normals for visualizations
  void get_normal(Vector &result, typename Node::index_type index) const
    { ASSERTFAIL("CurveMesh: This mesh type does not have node normals."); }
  
  template<class VECTOR, class INDEX1, class INDEX2>
  void get_normal(Vector &, VECTOR&, INDEX1, INDEX2) const
    { ASSERTFAIL("CurveMesh: This mesh type does not have element normals."); }

  //! use these to build up a new contour mesh
  typename Node::index_type add_node(const Point &p)
  {
    points_.push_back(p);
    return static_cast<under_type>(points_.size() - 1);
  }
  
  typename Node::index_type add_point(const Point &point)
    { return add_node(point); }
    
  typename Edge::index_type add_edge(typename Node::index_type i1,
                                     typename Node::index_type i2)
  {
    edges_.push_back(i1);
    edges_.push_back(i2);
    return static_cast<index_type>((edges_.size()>>1)-1);
  }
  
  template <class ARRAY>
  typename Elem::index_type add_elem(ARRAY a)
  {
    ASSERTMSG(a.size() == 2, "Tried to add non-line element.");
    edges_.push_back(static_cast<typename Node::index_type>(a[0]));
    edges_.push_back(static_cast<typename Node::index_type>(a[1]));
    return static_cast<index_type>((edges_.size()>>1)-1);
  }
  
  //! Functions to improve memory management. Often one knows how many
  //! nodes/elements one needs, prereserving memory is often possible.
  void node_reserve(size_type s) { points_.reserve(static_cast<size_t>(s)); }
  void elem_reserve(size_type s) { edges_.reserve(static_cast<size_t>(2*s)); }
  void resize_nodes(size_type s) { points_.resize(static_cast<size_t>(s)); }
  void resize_elems(size_type s) { edges_.resize(static_cast<size_t>(2*s)); }

  //! Get the local coordinates for a certain point within an element
  //! This function uses a couple of newton iterations to find the local
  //! coordinate of a point
  template<class VECTOR, class INDEX>
  bool get_coords(VECTOR &coords,const Point &p,INDEX idx) const
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
    StackVector<Point,1> Jv;
    ElemData ed(*this,idx);
    basis_.derivate(coords,ed,Jv);
    Vector Jv1, Jv2;
    Jv[0].asVector().find_orthogonal(Jv1,Jv2);
    J[0] = Jv[0].x();
    J[1] = Jv[0].y();
    J[2] = Jv[0].z();
    J[3] = Jv1.x();
    J[4] = Jv1.y();
    J[5] = Jv1.z();
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
    StackVector<Point,1> Jv;
    ElemData ed(*this,idx);
    basis_.derivate(coords,ed,Jv);
    double J[9];
    Vector Jv1, Jv2;
    Jv[0].asVector().find_orthogonal(Jv1,Jv2);
    J[0] = Jv[0].x();
    J[1] = Jv[0].y();
    J[2] = Jv[0].z();
    J[3] = Jv1.x();
    J[4] = Jv1.y();
    J[5] = Jv1.z();
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
    Vector v,w;
    Jv[0].asVector().find_orthogonal(v,w);
    Jv[1] = v.asPoint();
    Jv[2] = w.asPoint();
    double min_jacobian = ScaledDetMatrix3P(Jv);
    
    size_type num_vertices = static_cast<size_type>(basis_.number_of_vertices());
    for (size_type j=0;j < num_vertices;j++)
    {
      basis_.derivate(basis_.unit_vertices[j],ed,Jv);
      Jv.resize(3); 
      Jv[0].asVector().find_orthogonal(v,w);
      Jv[1] = v.asPoint();
      Jv[2] = w.asPoint();
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
    Vector v,w;
    Jv[0].asVector().find_orthogonal(v,w);
    Jv[1] = v.asPoint();
    Jv[2] = w.asPoint();
    double min_jacobian = DetMatrix3P(Jv);
    
    size_type num_vertices = static_cast<size_type>(basis_.number_of_vertices());
    for (size_type j=0;j < num_vertices;j++)
    {
      basis_.derivate(basis_.unit_vertices[j],ed,Jv);
      Jv.resize(3); 
      Jv[0].asVector().find_orthogonal(v,w);
      Jv[1] = v.asPoint();
      Jv[2] = w.asPoint();
      temp = DetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }
      
    return (min_jacobian);
  }


  template <class INDEX>
  bool locate_node(INDEX &idx, const Point &p) const
  {
    typename Node::iterator ni, nie;
    begin(ni);
    end(nie);

    idx = static_cast<INDEX>(*ni);

    if (ni == nie)
    {
      return false;
    }

    double closest = (p-points_[*ni]).length2();

    ++ni;
    for (; ni != nie; ++ni)
    {
      if ( (p-points_[*ni]).length2() < closest )
      {
        closest = (p-points_[*ni]).length2();
        idx = static_cast<INDEX>(*ni);
      }
    }

    return true;
  }


  template <class INDEX>
  bool locate_elem(INDEX &idx, const Point &p) const
  {
    if (basis_.polynomial_order() > 1) return elem_locate(idx, *this, p);
    typename Edge::iterator ei;
    typename Edge::iterator eie;
    double cosa, closest=DBL_MAX;
    typename Node::array_type nra;
    double dist1, dist2, dist3, dist4;
    Point n1,n2,q;

    begin(ei);
    end(eie);

    if (ei==eie)
    {
      return (false);
    }

    for (; ei != eie; ++ei)
    {
      get_nodes(nra,*ei);

      n1 = points_[nra[0]];
      n2 = points_[nra[1]];

      dist1 = (p-n1).length();
      dist2 = (p-n2).length();
      dist3 = (n1-n2).length();

      cosa = Dot(n1-p,n1-n2)/((n1-p).length()*dist3);

      q = n1 + (n1-n2) * (n1-n2)/dist3;

      dist4 = (p-q).length();

      if ( (cosa > 0) && (cosa < dist3) && (dist4 < closest) ) 
      {
        closest = dist4;
        idx = static_cast<INDEX>(*ei);
      } 
      else if ( (cosa < 0) && (dist1 < closest) ) 
      {
        closest = dist1;
        idx = static_cast<INDEX>(*ei);
      } 
      else if ( (cosa > dist3) && (dist2 < closest) ) 
      {
        closest = dist2;
        idx = static_cast<INDEX>(*ei);
      }
    }

    return (true);
  }

  //! Find the closest element to a point
  template <class INDEX>
  double find_closest_elem(Point &result, INDEX &face, const Point &p) const
  {
    ASSERTFAIL("CurveMesh: Find closest element has not yet been implemented.");
  }
  
  //! Find the closest elements to a point
  template<class ARRAY>
  double find_closest_elems(Point &result, ARRAY &faces, const Point &p) const
  {  
    ASSERTFAIL("CurveMesh: Find closest element has not yet been implemented.");  
  }
  
  double get_epsilon() const
  { return (epsilon_); }
  
  //! Export this class using the old Pio system
  virtual void io(Piostream&);

  ///////////////////////////////////////////////////
  // STATIC VARIABLES AND FUNCTIONS
  
  //! These IDs are created as soon as this class will be instantiated
  //! The first one is for Pio and the second for the virtual interface
  //! These are currently different as they serve different needs.
  static PersistentTypeID curvemesh_typeid;
  
  //! Core functionality for getting the name of a templated mesh class
  static const string type_name(int n = -1);

  //! Type description, used for finding names of the mesh class for
  //! dynamic compilation purposes. Soem of this should be obsolete
  virtual const TypeDescription *get_type_description() const;
  static const TypeDescription* node_type_description();
  static const TypeDescription* edge_type_description();
  static const TypeDescription* face_type_description();
  static const TypeDescription* cell_type_description();
  static const TypeDescription* elem_type_description()
    { return edge_type_description(); }

  //! This function returns a maker for Pio.
  static Persistent *maker() { return new CurveMesh<Basis>(); }
  //! This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return scinew CurveMesh<Basis>(); }


  //! Functions local to CurveMesh, the latter are not thread safe
  //! THESE ARE NOT WELL INTEGRATED YET
  typename Node::index_type delete_node(typename Node::index_type i1)
  {
    CHECKARRAYBOUNDS(static_cast<index_type>(i1),
		     static_cast<index_type>(0),
		     static_cast<index_type>(points_.size()));

    vector<Point>::iterator niter;
    niter = points_.begin() + i1;
    points_.erase(niter);
    return static_cast<typename Node::index_type>(points_.size() - 1);
  }

  typename Node::index_type delete_nodes(typename Node::index_type i1,
					 typename Node::index_type i2)
  {
    CHECKARRAYBOUNDS(static_cast<index_type>(i1),
		     static_cast<index_type>(0),
		     static_cast<index_type>(points_.size()));

    CHECKARRAYBOUNDS(static_cast<index_type>(i2),
		     static_cast<index_type>(0),
		     static_cast<index_type>(points_.size()+1));

    vector<Point>::iterator niter1;
    niter1 = points_.begin() + i1;

    vector<Point>::iterator niter2;
    niter2 = points_.begin() + i2;

    points_.erase(niter1, niter2);
    return static_cast<typename Node::index_type>(points_.size() - 1);
  }

  typename Edge::index_type delete_edge(typename Edge::index_type i1)
  {
    CHECKARRAYBOUNDS(static_cast<index_type>(i1),
		     static_cast<index_type>(0),
		     static_cast<index_type>(edges_.size()>>1));

    typename vector<index_type>::iterator niter1;
    niter1 = edges_.begin() + 2*i1;

    typename vector<index_type>::iterator niter2;
    niter2 = edges_.begin() + 2*i1+2;

    edges_.erase(niter1, niter2);
    return static_cast<typename Edge::index_type>((edges_.size()>>1) - 1);
  }

  typename Edge::index_type delete_edges(typename Edge::index_type i1,
					 typename Edge::index_type i2)
  {
    CHECKARRAYBOUNDS(static_cast<index_type>(i1), 
		     static_cast<index_type>(0), 
		     static_cast<index_type>(edges_.size()>>1));

    CHECKARRAYBOUNDS(static_cast<index_type>(i2), 
		     static_cast<index_type>(0), 
		     static_cast<index_type>((edges_.size()>>1)+1));

    typename vector<index_type>::iterator niter1;
    niter1 = edges_.begin() + 2*i1;

    typename vector<index_type>::iterator niter2;
    niter2 = edges_.begin() + 2*i2;

    edges_.erase(niter1, niter2);

    return static_cast<typename Edge::index_type>((edges_.size()>>1) - 1);
  }

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
  inline void get_nodes_from_edge(ARRAY& array, INDEX idx) const
  {
    array.resize(2); 
    array[0] = static_cast<typename ARRAY::value_type>(edges_[2*idx]); 
    array[1] = static_cast<typename ARRAY::value_type>(edges_[2*idx+1]);
  }
  
  template<class ARRAY, class INDEX>
  inline void get_edges_from_node(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
      "CurveMesh: Must call synchronize NODE_NEIGHBORS_E on CurveMesh first");
    
    array.resize(node_neighbors_[idx].size());
    for (typename NodeNeighborMap::size_type i = 0; i < node_neighbors_[idx].size(); ++i)
      array[i] = 
        static_cast<typename ARRAY::value_type>(node_neighbors_[idx][i]);
  }

  template<class ARRAY, class INDEX>
  inline void get_nodes_from_elem(ARRAY& array, INDEX idx) const
  {
    get_nodes_from_edge(array,idx);
  }

  template<class ARRAY, class INDEX>
  inline void get_edges_from_elem(ARRAY& array, INDEX idx) const
  {
    array.resize(1); array[0] = typename ARRAY::value_type(idx);
  }

  template <class INDEX1, class INDEX2>
  bool 
  get_elem_neighbor(INDEX1 &neighbor, INDEX1 edge,INDEX2 node) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
                  "Must call synchronize NODE_NEIGHBORS_E on CurveMesh first");

    if (node_neighbors_[node].size() > 1)
    {
      if (node_neighbors_[node][0] == edge) 
        neighbor = static_cast<INDEX1>(node_neighbors_[node][1]);
      else neighbor = static_cast<INDEX1>(node_neighbors_[node][0]);
      return (true);    
    }
    return (false);      
  }


  template <class ARRAY, class INDEX>
  void
  get_node_neighbors(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
                  "Must call synchronize NODE_NEIGHBORS_E on CurveMesh first");
    array.resize(node_neighbors_[idx].size());
    for (typename NodeNeighborMap::size_type p=0;p<node_neighbors_[idx].size();p++)
    {
      if (edges_[2*(node_neighbors_[idx][p])] == idx)
        array[p] = static_cast<typename ARRAY::value_type>(edges_[2*(node_neighbors_[idx][p])+1]);
      else
        array[p] = static_cast<typename ARRAY::value_type>(edges_[2*(node_neighbors_[idx][p])]);
    }
  }

  
  template <class ARRAY, class INDEX1, class INDEX2>
  bool
  get_elem_neighbors(ARRAY &array, INDEX1 edge, INDEX2 idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
                  "Must call synchronize NODE_NEIGHBORS_E on CurveMesh first");
    
    typename NodeNeighborMap::size_type sz = node_neighbors_[idx].size();
    if (sz < 1) return (false);

    array.clear();
    array.reserve(sz-1);
    for (typename NodeNeighborMap::size_type i=0; i<sz;i++)
    {
      if (node_neighbors_[idx][i] != static_cast<typename ARRAY::value_type>(idx)) 
          array.push_back(typename ARRAY::value_type(node_neighbors_[idx][i]));
    }
    return (true);
  }
  
  template <class ARRAY, class INDEX>
  void
  get_elem_neighbors(ARRAY &array, INDEX idx) const

  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
                  "Must call synchronize NODE_NEIGHBORS_E on CurveMesh first");
    typename Node::index_type n1 = edges_[2*idx];
    typename Node::index_type n2 = edges_[2*idx+1];
    
    typename NodeNeighborMap::size_type s1 = node_neighbors_[n1].size();
    typename NodeNeighborMap::size_type s2 = node_neighbors_[n2].size();
    
    array.clear();
    array.reserve(s1+s2-2);
    for (typename NodeNeighborMap::size_type i=0; i<s1;i++)
    {
      if (node_neighbors_[n1][i] != idx) 
        array.push_back(typename ARRAY::value_type(node_neighbors_[n1][i]));
    }
    for (typename NodeNeighborMap::size_type i=0; i<s2;i++)
    {
      if (node_neighbors_[n2][i] != idx) 
        array.push_back(typename ARRAY::value_type(node_neighbors_[n2][i]));
    }
  }


  void compute_node_neighbors();
  void compute_epsilon();

  //////////////////////////////////////////////////////////////
  // Actual data stored in the mesh
  
  //! Vector with the node locations
  vector<Point>           points_;
  //! Vector with connectivity data
  vector<index_type>      edges_;
  //! The basis function, contains additional information on elements
  Basis                   basis_;

  //! Record which parts of the mesh are synchronized
  mask_type               synchronized_;
  //! Lock to synchronize between threads
  Mutex                   synchronize_lock_;

  //! Vector indicating which edges are conected to which
  //! node. This is the reverse of the connectivity data
  //! stored in the edges_ array.
  typedef vector<vector<typename Edge::index_type> > NodeNeighborMap;
  NodeNeighborMap         node_neighbors_;
  double                  epsilon_;

  //! Pointer to virtual interface
  //! This one is created as soon as the mesh is generated
  //! Put this one in a handle as we have a virtual destructor
  Handle<VMesh>           vmesh_;
};


template<class Basis>
CurveMesh<Basis>::CurveMesh() :
  synchronized_(ALL_ELEMENTS_E),
  synchronize_lock_("CurveMesh Lock"),
  epsilon_(0.0)
{
  //! Initialize the virtual interface when the mesh is created
  vmesh_ = CreateVCurveMesh(this);
}
  
template<class Basis>
CurveMesh<Basis>::CurveMesh(const CurveMesh &copy) :
  points_(copy.points_),
  edges_(copy.edges_),
  basis_(copy.basis_),
  synchronized_(copy.synchronized_),
  synchronize_lock_("CurveMesh Lock"),
  epsilon_(0.0)
{
  //! Ugly construction circumventing const
  CurveMesh &lcopy = (CurveMesh &)copy;

  //! We need to lock before we can copy these as these
  //! structures are generate dynamically when they are
  //! needed.
  lcopy.synchronize_lock_.lock();
  node_neighbors_ = copy.node_neighbors_;
  synchronized_ |= copy.synchronized_ & NODE_NEIGHBORS_E;
  lcopy.synchronize_lock_.unlock();
  epsilon_ = copy.epsilon_;
  
  //! Create a new virtual interface for this copy
  //! all pointers have changed hence create a new
  //! virtual interface class
  vmesh_ = CreateVCurveMesh(this);
}
  
template<class Basis>
CurveMesh<Basis>::~CurveMesh() 
{
}



template <class Basis>
const TypeDescription* get_type_description(CurveMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = scinew TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = scinew TypeDescription("CurveMesh", subs,
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
CurveMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((CurveMesh<Basis> *)0);
}


template <class Basis>
const TypeDescription*
CurveMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((CurveMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Node",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
CurveMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((CurveMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Edge",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
CurveMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((CurveMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Face",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
CurveMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((CurveMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Cell",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


//! Add an entry into the database for Pio
template <class Basis>
PersistentTypeID
CurveMesh<Basis>::curvemesh_typeid(type_name(-1), "Mesh", CurveMesh<Basis>::maker);

//////////////////////////////////////////////////////////////////////
// Functions in the mesh

template <class Basis>
BBox
CurveMesh<Basis>::get_bounding_box() const
{
  BBox result;

  typename Node::iterator i, ie;
  begin(i);
  end(ie);

  while (i != ie)
  {
    result.extend(points_[*i]);
    ++i;
  }

  return result;
}


template <class Basis>
void
CurveMesh<Basis>::transform(const Transform &t)
{
  vector<Point>::iterator itr = points_.begin();
  vector<Point>::iterator eitr = points_.end();
  while (itr != eitr)
  {
    *itr = t.project(*itr);
    ++itr;
  }
  
  //! If we have nodes on the edges they should be transformed in
  //! the same way
  size_type num_enodes = static_cast<size_type>(basis_.size_node_values());
  for (size_type i=0; i<num_enodes; i++)
  {
    Point p;
    basis_.get_node_value(p,i);
    p =t.project(p);
    basis_.set_node_value(p,i);
  }
  
  //! Projecting derivates is more difficult
}



template <class Basis>
double
CurveMesh<Basis>::get_size(typename Edge::index_type idx) const
{
  ElemData ed(*this, idx);
  vector<Point> pledge;
  vector<vector<double> > coords;
  // Perhaps there is a better choice for the number of divisions.
  pwl_approx_edge(coords, idx, 0, 5);

  double total = 0.0L;
  vector<vector<double> >::iterator iter = coords.begin();
  vector<vector<double> >::iterator last = coords.begin() + 1;
  while (last != coords.end()) 
  {
    vector<double> &c0 = *iter++;
    vector<double> &c1 = *last++;
    Point p0 = basis_.interpolate(c0, ed);
    Point p1 = basis_.interpolate(c1, ed);
    total += (p1.asVector() - p0.asVector()).length();
  }
  return total;
}


template <class Basis>
void
CurveMesh<Basis>::get_center(Point &result,
                             typename Edge::index_type idx) const
{
  ElemData cmcd(*this, idx);
  vector<double> coord(1,0.5L);
  result =  basis_.interpolate(coord, cmcd);
}





template <class Basis>
int
CurveMesh<Basis>::get_weights(const Point &p, typename Node::array_type &l,
                              double *w)
{
  typename Edge::index_type idx;
  if (locate(idx, p))
  {
    get_nodes(l,idx);
    StackVector<double,1> coords(1);
    if (get_coords(coords, p, idx))
    {
      basis_.get_weights(coords, w);
      return basis_.dofs();
    }
  }
  return 0;
}


template <class Basis>
int
CurveMesh<Basis>::get_weights(const Point &p, typename Edge::array_type &l,
                              double *w)
{
  typename Edge::index_type idx;
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
void
CurveMesh<Basis>::get_random_point(Point &p,
                                   typename Elem::index_type ei,
                                   FieldRNG &rng) const
{
  const Point &p0 = points_[edges_[2*ei]];
  const Point &p1 = points_[edges_[2*ei+1]];

  p = p0 + (p1 - p0) * rng();
}


template <class Basis>
bool
CurveMesh<Basis>::synchronize(mask_type sync)
{
  synchronize_lock_.lock();

  //! Test whether we need to synchronize for any of the neighor function
  //! calls. If so compute the node_neighbor_ array, which we can use
  //! for the element neighbors as well.
  
  if (sync & (Mesh::NODE_NEIGHBORS_E|Mesh::ELEM_NEIGHBORS_E) 
      && !(synchronized_ & Mesh::NODE_NEIGHBORS_E))
  {
    compute_node_neighbors();
  }
  
  if (sync & Mesh::EPSILON_E && !(synchronized_ & Mesh::EPSILON_E))
  {
    compute_epsilon();
  }
  
  synchronize_lock_.unlock();
  return (true);
}


template <class Basis>
bool
CurveMesh<Basis>::unsynchronize(mask_type sync)
{
  return (true);
}

//! Compute the inverse route of connectivity: from node to edge
template <class Basis>
void
CurveMesh<Basis>::compute_node_neighbors()
{
  node_neighbors_.clear();
  node_neighbors_.resize(points_.size());
  index_type i;
  size_type  num_elems = (edges_.size()>>1);
  for (i = 0; i < num_elems; i++)
  {
    node_neighbors_[edges_[2*i]].push_back(i);
    node_neighbors_[edges_[2*i+1]].push_back(i);
  }
  synchronized_ |= Mesh::NODE_NEIGHBORS_E;  
}

template <class Basis>
void
CurveMesh<Basis>::compute_epsilon()
{
  epsilon_ = get_bounding_box().diagonal().length()*1e-8;
  synchronized_ |= Mesh::EPSILON_E;  
}

#define CURVE_MESH_VERSION 3

template <class Basis>
void
CurveMesh<Basis>::io(Piostream& stream)
{
  int version = stream.begin_class(type_name(-1), CURVE_MESH_VERSION);

  Mesh::io(stream);

  // IO data members, in order
  Pio(stream, points_);
  if (version < 3)
  {
    vector<pair<unsigned int,unsigned int> > tmp;
    Pio(stream,tmp);
    edges_.resize(tmp.size()*2);
    for (vector<pair<unsigned int,unsigned int> >::size_type j=0;j<tmp.size();j++)
    {
      edges_[2*j] = tmp[j].first;
      edges_[2*j+1] = tmp[j].second;
    }
  }
  else
  {
    Pio_index(stream, edges_);
  }
  if (version >= 2) 
  {
    basis_.io(stream);
  }
  stream.end_class();

  if (stream.reading())
    vmesh_ = CreateVCurveMesh(this);
}


template <class Basis>
const string
CurveMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const string name = type_name(0) + FTNS + type_name(1) + FTNE;
    return name;
  }
  else if (n == 0)
  {
    static const string nm("CurveMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
void
CurveMesh<Basis>::begin(typename CurveMesh<Basis>::Node::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
CurveMesh<Basis>::end(typename CurveMesh<Basis>::Node::iterator &itr) const
{
  itr = static_cast<index_type>(points_.size());
}


template <class Basis>
void
CurveMesh<Basis>::begin(typename CurveMesh<Basis>::Edge::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
CurveMesh<Basis>::end(typename CurveMesh<Basis>::Edge::iterator &itr) const
{
  itr = static_cast<index_type>(edges_.size()>>1);
}


template <class Basis>
void
CurveMesh<Basis>::begin(typename CurveMesh<Basis>::Face::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
CurveMesh<Basis>::end(typename CurveMesh<Basis>::Face::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
CurveMesh<Basis>::begin(typename CurveMesh<Basis>::Cell::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
CurveMesh<Basis>::end(typename CurveMesh<Basis>::Cell::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
CurveMesh<Basis>::size(typename CurveMesh<Basis>::Node::size_type &s) const
{
  s = static_cast<size_type>(points_.size());
}


template <class Basis>
void
CurveMesh<Basis>::size(typename CurveMesh<Basis>::Edge::size_type &s) const
{
  s = static_cast<size_type>(edges_.size()>>1);
}


template <class Basis>
void
CurveMesh<Basis>::size(typename CurveMesh<Basis>::Face::size_type &s) const
{
  s = 0;
}


template <class Basis>
void
CurveMesh<Basis>::size(typename CurveMesh<Basis>::Cell::size_type &s) const
{
  s = 0;
}



} // namespace SCIRun

#endif
