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

#ifndef CORE_DATATYPES_POINTCLOUDMESH_H
#define CORE_DATATYPES_POINTCLOUDMESH_H 1

//! Include what kind of support we want to have
//! Need to fix this and couple it sci-defs
#include <Core/Datatypes/MeshSupport.h>

#include <Core/Containers/StackVector.h>
#include <Core/Containers/LockingHandle.h>
#include <Core/Containers/Handle.h>

#include <Core/Geometry/Transform.h>
#include <Core/Geometry/BBox.h>
#include <Core/Geometry/Point.h>

#include <Core/Basis/Locate.h>
#include <Core/Basis/Constant.h>

#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/FieldIterator.h>
#include <Core/Datatypes/FieldRNG.h>

//! Incude needed for Windows: declares SCISHARE
#include <Core/Datatypes/share.h>

namespace SCIRun {

using std::string;
using std::vector;

/////////////////////////////////////////////////////
// Declarations for virtual interface

//! Functions for creating the virtual interface
//! Declare the functions that instantiate the virtual interface
template <class Basis> class PointCloudMesh;

//! make sure any other mesh other than the preinstantiate ones
//! returns no virtual interface. Altering this behaviour will allow
//! for dynamically compiling the interfae if needed.

template<class MESH>
VMesh* CreateVPointCloudMesh(MESH* mesh) { return (0); }

//! Declare that these can be found in a library that is already
//! precompiled. So dynamic compilation will not instantiate them again.
#if (SCIRUN_POINTCLOUD_SUPPORT > 0)
SCISHARE VMesh* CreateVPointCloudMesh(PointCloudMesh<ConstantBasis<Point> >* mesh);

#endif
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////
// Declarations for PointCloudMesh class

template <class Basis>
class PointCloudMesh : public Mesh
{

//! Make sure the virtual interface has access
template<class MESH> friend class VPointCloudMesh;
template<class MESH> friend class VMeshShared;

public:
  // Types that change depending on 32 or 64bits
  typedef SCIRun::index_type            under_type;
  typedef SCIRun::index_type            index_type;
  typedef SCIRun::size_type             size_type;
  typedef SCIRun::mask_type             mask_type; 

  typedef LockingHandle<PointCloudMesh<Basis> > handle_type;
  typedef Basis                                 basis_type;

  //! Index and Iterator types required for Mesh Concept.
  struct Node {
    typedef NodeIndex<under_type>       index_type;
    typedef NodeIterator<under_type>    iterator;
    typedef NodeIndex<under_type>       size_type;
    typedef StackVector<index_type, 1>  array_type;
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

  typedef Node Elem;
  typedef Node DElem;

  //! Somehow the information of how to interpolate inside an element
  //! ended up in a separate class, as they need to share information
  //! this construction was created to transfer data. 
  //! Hopefully in the future this class will disappear again.
  friend class ElemData;
  class ElemData
  {
  public:
    typedef typename PointCloudMesh<Basis>::index_type  index_type;

    ElemData(const PointCloudMesh<Basis>& msh, const index_type ind) :
      mesh_(msh),
      index_(ind)
    {}

    // the following designed to coordinate with ::get_nodes
    inline index_type node0_index() const 
    {
      return index_;
    }

    inline
    const Point &node0() const 
    {
      return mesh_.points_[index_];
    }

  private:
    const PointCloudMesh<Basis>          &mesh_;
    const index_type                     index_;
  };


  //////////////////////////////////////////////////////////////////

  //! Construct a new mesh
  PointCloudMesh(); 
  
  //! Copy a mesh, needed for detaching the mesh from a field 
  PointCloudMesh(const PointCloudMesh &copy);
  
  //! Clone function for detaching the mesh and automatically generating
  //! a new version if needed.
  virtual PointCloudMesh *clone() { return new PointCloudMesh(*this); }
  
  //! Destructor
  virtual ~PointCloudMesh();

  //! Access point to virtual interface
  virtual VMesh* vmesh() { return (vmesh_.get_rep()); }

  //! This one should go at some point, should be reroute throught the
  //! virtual interface
  virtual int basis_order() 
  {
    return (basis_.polynomial_order()); 
  }

  //! Topological dimension
  virtual int dimensionality() const { return 0; }

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
  virtual bool is_editable() const { return true; }

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


  // This is actually get_nodes(typename Node::array_type &, Elem::index_type)
  // for compilation purposes.  IE It is redundant unless we are
  // templated by Elem type and we don't know that Elem is Node.
  // This is needed in ClipField, for example.
  void get_nodes(typename Node::array_type &a, 
                 typename Node::index_type i) const
    { a.resize(1); a[0] = i; }
  void get_nodes(typename Node::array_type &, 
                 typename Edge::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_nodes has not been implemented for edges"); }
  void get_nodes(typename Node::array_type &, 
                 typename Face::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_nodes has not been implemented for faces"); }
  void get_nodes(typename Node::array_type &, 
                 typename Cell::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_nodes has not been implemented for cells"); }


  void get_edges(typename Edge::array_type &array, 
                 typename Node::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_edges has not been implemented for nodes"); }
  void get_edges(typename Edge::array_type &array,
                 typename Edge::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_edgees has not been implemented for edges"); }
  void get_edges(typename Edge::array_type &array, 
                 typename Face::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_edgees has not been implemented for faces"); }
  void get_edges(typename Edge::array_type &array, 
                 typename Cell::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_edges has not been implemented for cells"); }

  void get_faces(typename Face::array_type &array, 
                 typename Node::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_faces has not been implemented"); }
  void get_faces(typename Face::array_type &array,
                 typename Edge::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_faces has not been implemented"); }
  void get_faces(typename Face::array_type &array, 
                 typename Face::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_faces has not been implemented"); }
  void get_faces(typename Face::array_type &array, 
                 typename Cell::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_faces has not been implemented"); }

  void get_cells(typename Cell::array_type &array, 
                 typename Node::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_cells has not been implemented"); }
  void get_cells(typename Cell::array_type &array, 
                 typename Edge::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_cells has not been implemented"); }
  void get_cells(typename Cell::array_type &array, 
                 typename Face::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_cells has not been implemented"); }
  void get_cells(typename Cell::array_type &array, 
                 typename Cell::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_cells has not been implemented"); }
    
    
  //! get the parent element(s) of the given index
  void get_elems(typename Elem::array_type &result,
                 typename Node::index_type idx) const
    { result.clear(); result.push_back(idx); }
  void get_elems(typename Elem::array_type &result,
                 typename Edge::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_elems has not been implemented for edges"); }
  void get_elems(typename Elem::array_type &result,
                 typename Face::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_elems has not been implemented for faces"); }
  void get_elems(typename Elem::array_type &result,
                 typename Cell::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_elems has not been implemented for cells"); }

  void get_delems(typename DElem::array_type &array, 
                  typename Node::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_delems has not been implemented for nodes"); }
  void get_delems(typename DElem::array_type &array, 
                  typename Edge::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_delems has not been implemented for edges"); }
  void get_delems(typename DElem::array_type &array, 
                  typename Face::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_delems has not been implemented for faces"); }
  void get_delems(typename DElem::array_type &array, 
                  typename Cell::index_type idx) const
    { ASSERTFAIL("PointCloudMesh: get_delems has not been implemented for cells"); }

  //! Generate the list of points that make up a sufficiently accurate
  //! piecewise linear approximation of an edge.
  template<class VECTOR>
  void pwl_approx_edge(VECTOR &,
                       typename Elem::index_type,
                       unsigned int,
                       unsigned int) const
  {
    ASSERTFAIL("PointCloudMesh: cannot approximiate edges");  
  }

  template<class VECTOR>
  void pwl_approx_face(VECTOR &,
                       typename Elem::index_type,
                       unsigned int,
                       unsigned int) const
  {
    ASSERTFAIL("PointCloudMesh: cannot approximiate faces");  
  }


  //! get the center point (in object space) of an element
  void get_center(Point &p, typename Node::index_type i) const 
    { p = points_[i]; }
  void get_center(Point &, typename Edge::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_center has not been implemented for edges"); }
  void get_center(Point &, typename Face::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_center has not been implemented for faces"); }
  void get_center(Point &, typename Cell::index_type) const
    { ASSERTFAIL("PointCloudMesh: get_center has not been implemented for cells"); }

  //! Get the size of an elemnt (length, area, volume)
  double get_size(typename Node::index_type) const { return 0.0; }
  double get_size(typename Edge::index_type) const { return 0.0; }
  double get_size(typename Face::index_type) const { return 0.0; }
  double get_size(typename Cell::index_type) const { return 0.0; }
  double get_length(typename Edge::index_type idx) const { return 0.0; }
  double get_area(typename Face::index_type idx) const { return 0.0; }
  double get_volume(typename Cell::index_type idx) const { return 0.0; }


  //! Get neighbors of an element or a node
  //! THIS ONE IS FLAWED AS IN 3D SPACE  A NODE CAN BE CONNECTED TO
  //! MANY ELEMENTS
  bool get_neighbor(typename Elem::index_type &neighbor,
                    typename Elem::index_type elem,
                    typename DElem::index_type delem) const
    { return (false); }
  void get_neighbors(vector<typename Node::index_type> &array,
                     typename Node::index_type node) const
    { array.resize(0); }
  bool get_neighbors(vector<typename Elem::index_type> &array,
                     typename Elem::index_type elem,
                     typename DElem::index_type delem) const 
    { array.resize(0); return (false); }

  //! Locate a point in a mesh, find which is the closest node
  bool locate(typename Node::index_type &n, const Point &p) const
    { return (locate_node(n,p)); }
  bool locate(typename Edge::index_type &, const Point &) const
    { return (false); }
  bool locate(typename Face::index_type &, const Point &) const
    { return (false); }
  bool locate(typename Cell::index_type &, const Point &) const
    { return (false); }

  //! These should become obsolete soon, they do not follow the concept
  //! of the basis functions....
  int get_weights(const Point &p, typename Node::array_type &l, double *w);
  int get_weights(const Point &p, typename Edge::array_type &l, double *w)
    { ASSERTFAIL("PointCloudField: get_weights for edges isn't supported"); }
  int get_weights(const Point &p, typename Face::array_type &l, double *w)
    { ASSERTFAIL("PointCloudField: get_weights for faces isn't supported"); }
  int get_weights(const Point &p, typename Cell::array_type &l, double *w)
    { ASSERTFAIL("PointCloudField: get_weights for cells isn't supported"); }


  void get_point(Point &p, typename Node::index_type i) const
    { get_center(p,i); }
  void set_point(const Point &p, typename Node::index_type i)
    { points_[i] = p; }
  void get_random_point(Point &p, const typename Elem::index_type i,
                        FieldRNG &rng) const
    { get_center(p, i); }

  void get_normal(Vector &, typename Node::index_type) const
    { ASSERTFAIL("PointCloudMesh: this mesh type does not have node normals."); }
  template<class VECTOR>
  void get_normal(Vector &, VECTOR &, typename Elem::index_type, unsigned int) const
    { ASSERTFAIL("PointCloudMesh: this mesh type does not have element normals."); }

  //! use these to build up a new PointCloudField mesh
  typename Node::index_type add_node(const Point &p) { return add_point(p); }
  typename Node::index_type add_point(const Point &p);
  
  template <class ARRAY>
  typename Elem::index_type add_elem(ARRAY a)
  {
    return (static_cast<typename Elem::index_type>(a[0]));
  }

  void node_reserve(size_t s) { points_.reserve(s); }
  void elem_reserve(size_t s) { points_.reserve(s); }
  void resize_nodes(size_t s) { points_.resize(s); }
  void resize_elems(size_t s) { points_.resize(s); }


  //! THESE FUNCTIONS ARE DEFINED INSIDE THE CLASS AS THESE ARE TEMPLATED
  //! FUNCTIONS INSIDE A TEMPLATED CLASS. THIS WAY OF DEFINING THE FUNCTIONS
  //! IS SUPPORTED BY MOST COMPILERS

  //! Get the local coordinates for a certain point within an element
  //! This function uses a couple of newton iterations to find the local
  //! coordinate of a point
  template<class VECTOR>
  bool get_coords(VECTOR &coords, const Point &p, typename Elem::index_type idx) const
  {
    coords.resize(1);
    coords[0] = 0.0;
    return true;
  }

  //! Find the location in the global coordinate system for a local coordinate
  //! This function is the opposite of get_coords.
  template<class VECTOR>
  void interpolate(Point &pt, const VECTOR &coords, typename Node::index_type idx) const
  {
    get_center(pt, idx);
  }

  //! Interpolate the derivate of the function, This infact will return the
  //! jacobian of the local to global coordinate transformation. This function
  //! is mainly intended for the non linear elements
  template<class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords, typename Elem::index_type idx, VECTOR2 &J) const
  {
    J.resize(1);
    J[0].x(0.0);
    J[0].y(0.0);
    J[0].z(0.0);
  }


  //! Get the determinant of the jacobian, which is the local volume of an element
  //! and is intended to help with the integration of functions over an element.
  template<class VECTOR>
  double det_jacobian(const VECTOR& coords, typename Elem::index_type idx) const
  {
    return (1.0);
  }

  //! Get the jacobian of the transformation. In case one wants the non inverted
  //! version of this matrix. This is currentl here for completeness of the 
  //! interface
  template<class VECTOR>
  void jacobian(const VECTOR& coords, typename Elem::index_type idx, double* J) const
  {
    J[0] = 1.0;
    J[1] = 0.0;
    J[2] = 0.0;
    J[3] = 0.0;
    J[4] = 1.0;
    J[5] = 0.0;
    J[6] = 0.0;
    J[7] = 0.0;
    J[8] = 1.0;
  }

  //! Get the inverse jacobian of the transformation. This one is needed to 
  //! translate local gradients into global gradients. Hence it is crucial for
  //! calculating gradients of fields, or constructing finite elements.        
  template<class VECTOR>                
  double inverse_jacobian(const VECTOR& coords, typename Elem::index_type idx, double* Ji) const
  {
    Ji[0] = 1.0;
    Ji[1] = 0.0;
    Ji[2] = 0.0;
    Ji[3] = 0.0;
    Ji[4] = 1.0;
    Ji[5] = 0.0;
    Ji[6] = 0.0;
    Ji[7] = 0.0;
    Ji[8] = 1.0;
 
    return (1.0);
  }


  template<class INDEX>
  double scaled_jacobian_metric(INDEX idx) const
  {
    return (0.0);
  }

  template<class INDEX>
  double jacobian_metric(INDEX idx) const
  {
    return (0.0);
  }

  template <class INDEX>
  bool locate_node(INDEX &idx, const Point &p) const
  {
    typename Node::iterator ni, nie;
    begin(ni);
    end(nie);


    if (ni == nie)
    {
      return (false);
    }

    double closest = (p-points_[*ni]).length2();
    idx = static_cast<INDEX>(*ni);
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
    typename Elem::iterator ei, eie;
    begin(ei);
    end(eie);


    if (ei == eie)
    {
      return (false);
    }

    double closest = (p-points_[*ei]).length2();
    idx = static_cast<INDEX>(*ei);

    ++ei;
    for (; ei != eie; ++ei)
    {
      if ( (p-points_[*ei]).length2() < closest )
      {
        closest = (p-points_[*ei]).length2();
        idx = static_cast<INDEX>(*ei);
      }
    }

    return true;
  }


  template <class INDEX> 
  double find_closest_elem(Point& result,INDEX &i, const Point &point) const
  {
    if(locate_node(i,point))
    {
      get_center(result,typename Node::index_type(i));
      return ((result-point).length());
    }
    result = point;
    return (0.0);
  }


  template <class ARRAY> 
  double find_closest_elems(Point& result,ARRAY &i, const Point &point) const
  {
    typename Elem::index idx;
    if(locate_node(idx,point))
    {
      get_center(result,idx);
      i.resize(1); i[0] = static_cast<typename ARRAY::value_type>(idx);
      return ((result-point).length());
    }
    result = point;
    i.resize(0);
    return (0.0);
  }


  double get_epsilon() const 
    { return (epsilon_); }

  //! Export this class using the old Pio system
  virtual void io(Piostream&);
  
  ///////////////////////////////////////////////////
  // STATIC VARIABLES AND FUNCTIONS  
    
  //! These IDs are created as soon as this class will be instantiated
  static PersistentTypeID pointcloud_typeid;
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
  { return node_type_description(); }

  //! This function returns a maker for Pio.
  static Persistent *maker() { return scinew PointCloudMesh(); }
  //! This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return scinew PointCloudMesh(); }

protected:
  template <class ARRAY, class INDEX>
  void get_nodes_from_elem(ARRAY nodes,INDEX idx)
  {
    nodes.resize(1); nodes[0] = idx;
  }

  template <class ARRAY, class INDEX>
  void get_edges_from_elem(ARRAY edges,INDEX idx)
  {
    edges.resize(0);
  }

protected:
  void compute_epsilon();

  //! the nodes
  vector<Point> points_;

  //! basis fns
  Basis         basis_;

  //! Record which parts of the mesh are synchronized
  mask_type     synchronized_;
  //! Lock to synchronize between threads
  Mutex         synchronize_lock_;
  
  double        epsilon_;

  //! Virtual interface
  Handle<VMesh> vmesh_;

};  // end class PointCloudMesh


template<class Basis>
PointCloudMesh<Basis>::PointCloudMesh() :
  synchronized_(ALL_ELEMENTS_E),
  synchronize_lock_("PointCloudMesh Lock"),  
  epsilon_(0.0)
{
  //! Initialize the virtual interface when the mesh is created
  vmesh_ = CreateVPointCloudMesh(this);
}
  
template<class Basis>
PointCloudMesh<Basis>::PointCloudMesh(const PointCloudMesh &copy) : 
  points_(copy.points_),
  basis_(copy.basis_),  
  synchronized_(copy.synchronized_),
  synchronize_lock_("PointCloudMesh Lock"),
  epsilon_(copy.epsilon_)
{
  //! Create a new virtual interface for this copy
  //! all pointers have changed hence create a new
  //! virtual interface class
  vmesh_ = CreateVPointCloudMesh(this);
}
    
template<class Basis>
PointCloudMesh<Basis>::~PointCloudMesh() 
{
}




template <class Basis>
PersistentTypeID
PointCloudMesh<Basis>::pointcloud_typeid(type_name(-1), "Mesh",
                                 PointCloudMesh<Basis>::maker);


template <class Basis>
BBox
PointCloudMesh<Basis>::get_bounding_box() const
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
PointCloudMesh<Basis>::transform(const Transform &t)
{
  vector<Point>::iterator itr = points_.begin();
  vector<Point>::iterator eitr = points_.end();
  while (itr != eitr)
  {
    *itr = t.project(*itr);
    ++itr;
  }
}


template <class Basis>
int
PointCloudMesh<Basis>::get_weights(const Point &p,
                                   typename Node::array_type &l,
                                   double *w)
{
  typename Node::index_type idx;
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
typename PointCloudMesh<Basis>::Node::index_type
PointCloudMesh<Basis>::add_point(const Point &p)
{
  points_.push_back(p);
  return points_.size() - 1;
}


#define PointCloudFieldMESH_VERSION 2

template <class Basis>
void
PointCloudMesh<Basis>::io(Piostream& stream)
{
  int version = stream.begin_class(type_name(-1), PointCloudFieldMESH_VERSION);

  Mesh::io(stream);

  // IO data members, in order
  Pio(stream,points_);

  if (version >= 2) {
    basis_.io(stream);
  }

  stream.end_class();

  if (stream.reading())
    vmesh_ = CreateVPointCloudMesh(this);
}


template <class Basis>
const string
PointCloudMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const string name = type_name(0) + FTNS + type_name(1) + FTNE;
    return name;
  }
  else if (n == 0)
  {
    static const string nm("PointCloudMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
void
PointCloudMesh<Basis>::begin(typename PointCloudMesh::Node::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
PointCloudMesh<Basis>::end(typename PointCloudMesh::Node::iterator &itr) const
{
  itr = static_cast<index_type>(points_.size());
}


template <class Basis>
void
PointCloudMesh<Basis>::size(typename PointCloudMesh::Node::size_type &s) const
{
  s = static_cast<index_type>(points_.size());
}


template <class Basis>
void
PointCloudMesh<Basis>::begin(typename PointCloudMesh::Edge::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
PointCloudMesh<Basis>::end(typename PointCloudMesh::Edge::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
PointCloudMesh<Basis>::size(typename PointCloudMesh::Edge::size_type &s) const
{
  s = 0;
}


template <class Basis>
void
PointCloudMesh<Basis>::begin(typename PointCloudMesh::Face::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
PointCloudMesh<Basis>::end(typename PointCloudMesh::Face::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
PointCloudMesh<Basis>::size(typename PointCloudMesh::Face::size_type &s) const
{
  s = 0;
}


template <class Basis>
void
PointCloudMesh<Basis>::begin(typename PointCloudMesh::Cell::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
PointCloudMesh<Basis>::end(typename PointCloudMesh::Cell::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
PointCloudMesh<Basis>::size(typename PointCloudMesh::Cell::size_type &s) const
{
  s = 0;
}


template<class Basis>
bool
PointCloudMesh<Basis>::synchronize(mask_type sync)
{
  synchronize_lock_.lock();
  
  if (sync & Mesh::EPSILON_E && !(synchronized_ & Mesh::EPSILON_E))
  {
    compute_epsilon();
    synchronized_ |= (Mesh::EPSILON_E);
  }
  
  synchronize_lock_.unlock();

  // No object to synchronize, hence always will succeed
  return(true);
}

template<class Basis>
bool
PointCloudMesh<Basis>::unsynchronize(mask_type sync)
{
  synchronize_lock_.lock();
    if (sync & Mesh::EPSILON_E) synchronized_ &= ~(Mesh::EPSILON_E);
    // No object to synchronize, hence always will succeed
  synchronize_lock_.unlock();
  return(true);
}

template<class Basis>
void
PointCloudMesh<Basis>::compute_epsilon()
{
  epsilon_ = get_bounding_box().diagonal().length()*1e-8;
}


template <class Basis>
const TypeDescription*
get_type_description(PointCloudMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = SCIRun::get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = scinew TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = scinew TypeDescription("PointCloudMesh", subs,
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
PointCloudMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((PointCloudMesh<Basis> *)0);
}


template <class Basis>
const TypeDescription*
PointCloudMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PointCloudMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Node",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
PointCloudMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PointCloudMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Edge",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
PointCloudMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PointCloudMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Face",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
PointCloudMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PointCloudMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Cell",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}

} // namespace SCIRun

#endif 
