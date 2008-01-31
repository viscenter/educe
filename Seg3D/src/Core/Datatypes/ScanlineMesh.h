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
 *  ScanlineMesh.h: Templated Mesh defined on a 3D Regular Grid
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   January 2001
 *
 *  Copyright (C) 2001 SCI Group
 *
 */


#ifndef CORE_DATATYPES_SCANLINEMESH_H
#define CORE_DATATYPES_SCANLINEMESH_H 1

//! Include what kind of support we want to have
//! Need to fix this and couple it sci-defs
#include <Core/Datatypes/MeshSupport.h>

#include <Core/Containers/StackVector.h>
#include <Core/Containers/LockingHandle.h>
#include <Core/Containers/Handle.h>

#include <Core/Basis/Locate.h>
#include <Core/Basis/CrvLinearLgn.h>

#include <Core/Geometry/Transform.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/BBox.h>

#include <Core/Datatypes/FieldRNG.h>
#include <Core/Datatypes/FieldIterator.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/VMesh.h>

//! Incude needed for Windows: declares SCISHARE
#include <Core/Datatypes/share.h>

namespace SCIRun {

using std::string;
using std::vector;

/////////////////////////////////////////////////////
// Declarations for virtual interface


//! Functions for creating the virtual interface
//! Declare the functions that instantiate the virtual interface
template <class Basis>
class ScanlineMesh;

//! make sure any other mesh other than the preinstantiate ones
//! returns no virtual interface. Altering this behaviour will allow
//! for dynamically compiling the interfae if needed.
template<class MESH>
VMesh* CreateVScanlineMesh(MESH* mesh) { return (0); }

//! These declarations are needed for a combined dynamic compilation as
//! as well as virtual functions solution.
//! Declare that these can be found in a library that is already
//! precompiled. So dynamic compilation will not instantiate them again.

#if (SCIRUN_SCANLINE_SUPPORT > 0)

SCISHARE VMesh* CreateVScanlineMesh(ScanlineMesh<CrvLinearLgn<Point> >* mesh);

#endif
/////////////////////////////////////////////////////


template <class Basis>
class ScanlineMesh : public Mesh
{

//! Make sure the virtual interface has access
template <class MESH> friend class VScanlineMesh;

public:

  // Types that change depending on 32 or 64bits
  typedef SCIRun::index_type            under_type;
  typedef SCIRun::index_type            index_type;
  typedef SCIRun::size_type             size_type;
  typedef SCIRun::mask_type             mask_type; 
   
  typedef LockingHandle<ScanlineMesh<Basis> > handle_type;
  typedef Basis           basis_type;

  //! Index and Iterator types required for Mesh Concept.
  struct Node {
    typedef NodeIndex<under_type>       index_type;
    typedef NodeIterator<under_type>    iterator;
    typedef NodeIndex<under_type>       size_type;
    typedef StackVector<index_type, 8>  array_type;
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

  typedef Edge Elem;
  typedef Node DElem;

  friend class ElemData;

  class ElemData
  {
  public:
    typedef typename ScanlineMesh<Basis>::index_type  index_type;

    ElemData(const ScanlineMesh<Basis>& msh,
             const typename Elem::index_type ind) :
      mesh_(msh),
      index_(ind)
    {}

    // the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const 
    {
      return (index_);
    }
    inline
    index_type node1_index() const 
    {
      return (index_ + 1);
    }


    // the following designed to coordinate with ::get_edges
    inline
    index_type edge0_index() const 
    {
      return index_;
    }

    inline
    const Point node0() const 
    {
      Point p(index_, 0.0, 0.0);
      return mesh_.transform_.project(p);
    }
    inline
    const Point node1() const 
    {
      Point p(index_ + 1, 0.0, 0.0);
      return mesh_.transform_.project(p);
    }

  private:
    const ScanlineMesh<Basis>          &mesh_;
    const index_type                   index_;
  };

  ScanlineMesh() : min_i_(0), ni_(0) 
  {
    vmesh_ = CreateVScanlineMesh(this);   
    compute_jacobian();   
  }
  
  ScanlineMesh(index_type nx, const Point &min, const Point &max);
  ScanlineMesh(ScanlineMesh* mh, index_type offset, index_type nx)
    : min_i_(offset), ni_(nx), transform_(mh->transform_) 
  { 
    //! Create a new virtual interface for this copy
    //! all pointers have changed hence create a new
    //! virtual interface class
    vmesh_ = CreateVScanlineMesh(this);   
  
    compute_jacobian(); 
  }
  ScanlineMesh(const ScanlineMesh &copy)
    : min_i_(copy.get_min_i()), ni_(copy.get_ni()),
      transform_(copy.transform_) 
  { 
    //! Create a new virtual interface for this copy
    //! all pointers have changed hence create a new
    //! virtual interface class
    vmesh_ = CreateVScanlineMesh(this);   

    compute_jacobian(); 
  }
  virtual ScanlineMesh *clone() { return new ScanlineMesh(*this); }
  virtual ~ScanlineMesh() 
  {  
  }

  //! Access point to virtual interface
  virtual VMesh* vmesh() { return (vmesh_.get_rep()); }
  
  virtual int basis_order() { return basis_.polynomial_order(); }

  //! get the mesh statistics
  index_type get_min_i() const { return min_i_; }
  bool get_min(vector<index_type>&) const;
  size_type get_ni() const { return ni_; }
  virtual bool get_dim(vector<size_type>&) const;
  Vector diagonal() const;
  virtual BBox get_bounding_box() const;
  virtual void transform(const Transform &t);
  virtual void get_canonical_transform(Transform &t);

  //! set the mesh statistics
  void set_min_i(index_type i) {min_i_ = i; }
  void set_min(vector<index_type> mins);
  void set_ni(size_type i)
  {
    ni_ = i;

    //! Create a new virtual interface for this copy
    //! all pointers have changed hence create a new
    //! virtual interface class
    vmesh_ = CreateVScanlineMesh(this); 
  }
  virtual void set_dim(vector<size_type> dims);

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

  void to_index(typename Node::index_type &index, index_type i) const
  { index = i; }
  void to_index(typename Edge::index_type &index, index_type i) const
  { index = i; }
  void to_index(typename Face::index_type &index, index_type i) const
  { index = i; }
  void to_index(typename Cell::index_type &index, index_type i) const
  { index = i; }

  //! get the child elements of the given index
  void get_nodes(typename Node::array_type &, typename Edge::index_type) const;
  void get_nodes(typename Node::array_type &, typename Face::index_type) const
  {}
  void get_nodes(typename Node::array_type &, typename Cell::index_type) const
  {}
  void get_edges(typename Edge::array_type &, typename Edge::index_type) const
  {}
  void get_edges(typename Edge::array_type &, typename Face::index_type) const
  {}
  void get_edges(typename Edge::array_type &, typename Cell::index_type) const
  {}

  //Stub, used by ShowField.
  void get_faces(typename Face::array_type &, typename Elem::index_type) const
  {}

  //! get the parent element(s) of the given index
  void get_elems(typename Elem::array_type &result,
                 typename Node::index_type idx) const;
  void get_elems(typename Elem::array_type &result,
                 typename Edge::index_type idx) const {}
  void get_elems(typename Elem::array_type &result,
                 typename Face::index_type idx) const {}

  //! Wrapper to get the derivative elements from this element.
  void get_delems(typename DElem::array_type &result,
                  typename Elem::index_type idx) const
  {
    get_nodes(result, idx);
  }

  //! Get the size of an elemnt (length, area, volume)
  double get_size(typename Node::index_type) const { return 0.0; }
  double get_size(typename Edge::index_type idx) const
  {
    typename Node::array_type ra;
    get_nodes(ra,idx);
    Point p0,p1;
    get_point(p0,ra[0]);
    get_point(p1,ra[1]);
    return (p0-p1).length();
  }
  double get_size(typename Face::index_type) const { return 0.0; };
  double get_size(typename Cell::index_type) const { return 0.0; };
  double get_length(typename Edge::index_type idx) const
  { return get_size(idx); }
  double get_area(typename Face::index_type idx) const
  { return get_size(idx); }
  double get_volume(typename Cell::index_type idx) const
  { return get_size(idx); }

  int get_valence(typename Node::index_type idx) const
  { return (idx == 0 || idx == ni_ - 1) ? 1 : 2; }
  int get_valence(typename Edge::index_type) const { return 0; }
  int get_valence(typename Face::index_type) const { return 0; }
  int get_valence(typename Cell::index_type) const { return 0; }

  //! get the center point (in object space) of an element
  void get_center(Point &, typename Node::index_type) const;
  void get_center(Point &, typename Edge::index_type) const;
  void get_center(Point &, typename Face::index_type) const {}
  void get_center(Point &, typename Cell::index_type) const {}

  bool locate(typename Node::index_type &, const Point &) const;
  bool locate(typename Edge::index_type &, const Point &) const;
  bool locate(typename Face::index_type &, const Point &) const { return false; }
  bool locate(typename Cell::index_type &, const Point &) const { return false; }

  int get_weights(const Point &p, typename Node::array_type &l, double *w);
  int get_weights(const Point &p, typename Edge::array_type &l, double *w);
  int get_weights(const Point & , typename Face::array_type & , double * )
  { ASSERTFAIL("ScanlineMesh::get_weights for faces isn't supported"); }
  int get_weights(const Point & , typename Cell::array_type & , double * )
  { ASSERTFAIL("ScanlineMesh::get_weights for cells isn't supported"); }

  void get_random_point(Point &p, typename Elem::index_type i, FieldRNG &rng) const;

  void get_point(Point &p, typename Node::index_type i) const { get_center(p, i); }
  void get_normal(Vector &, typename Node::index_type) const
  { ASSERTFAIL("This mesh type does not have node normals."); }
  void get_normal(Vector &, vector<double> &, typename Elem::index_type,
                  unsigned int)
  { ASSERTFAIL("This mesh type does not have element normals."); }



  // Unsafe due to non-constness of unproject.
  Transform &get_transform() { return transform_; }
  Transform &set_transform(const Transform &trans)
  { transform_ = trans; compute_jacobian(); return transform_; }

  virtual int dimensionality() const { return 1; }
  virtual int topology_geometry() const { return (Mesh::STRUCTURED | Mesh::REGULAR); }
  Basis& get_basis() { return basis_; }

 //! Generate the list of points that make up a sufficiently accurate
  //! piecewise linear approximation of an edge.
  void pwl_approx_edge(vector<vector<double> > &coords,
                       typename Elem::index_type ci,
                       unsigned int,
                       unsigned int div_per_unit) const
  {
    // Needs to match unit_edges in Basis/QuadBilinearLgn.cc
    // compare get_nodes order to the basis order
    basis_.approx_edge(0, div_per_unit, coords);
  }

  //! Generate the list of points that make up a sufficiently accurate
  //! piecewise linear approximation of an face.
  void pwl_approx_face(vector<vector<vector<double> > > &coords,
                       typename Elem::index_type ci,
                       typename Face::index_type fi,
                       unsigned int div_per_unit) const
  {
    ASSERTFAIL("ScanlineMesh has no faces");
  }

  //! Get the local coordinates for a certain point within an element
  //! This function uses a couple of newton iterations to find the local
  //! coordinate of a point
  template<class VECTOR>
  bool get_coords(VECTOR &coords, const Point &p, typename Elem::index_type idx) const
  {
    // If polynomial order is larger, use the complicated HO basis implementation
    // Since this is a latvol and most probably linear, this function is to expensive
    if (basis_.polynomial_order() > 1) 
    {
      ElemData ed(*this, idx);
      return (basis_.get_coords(coords, p, ed));
    }
    
    // Cheap implementation that assumes it is a regular grid
    // This implementation should be faster then the interpolate of the linear
    // basis which needs to work as well for the unstructured hexvol :(
    const Point r = transform_.unproject(p);
    
    coords.resize(1);
    coords[0] = static_cast<typename VECTOR::value_type>(r.x()-static_cast<double>(idx));
    
    if (static_cast<double>(coords[0]) < 0.0) if (static_cast<double>(coords[0]) > -(MIN_ELEMENT_VAL)) 
        coords[0] = static_cast<typename VECTOR::value_type>(0.0); else return (false);
    if (static_cast<double>(coords[0]) > 1.0) if (static_cast<double>(coords[0]) < 1.0+(MIN_ELEMENT_VAL)) 
        coords[0] = static_cast<typename VECTOR::value_type>(1.0); else return (false);    

    return (true);    
  }

  //! Find the location in the global coordinate system for a local coordinate
  //! This function is the opposite of get_coords.
  template<class VECTOR>
  void interpolate(Point &pt, const VECTOR &coords, typename Elem::index_type idx) const
  {
    // only makes sense for higher order
    if (basis_.polynomial_order() > 1) 
    {
      ElemData ed(*this, idx);
      pt = basis_.interpolate(coords, ed);
      return;
    }
    
    Point p(static_cast<double>(idx)+static_cast<double>(coords[0]),0.0,0.0);
    pt = transform_.project(p);
  }

  //! Interpolate the derivate of the function, This infact will return the
  //! jacobian of the local to global coordinate transformation. This function
  //! is mainly intended for the non linear elements
  template<class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords, typename Elem::index_type idx, VECTOR2 &J) const
  {
    // only makes sense for higher order
    if (basis_.polynomial_order() > 1) 
    {
      ElemData ed(*this, idx);
      basis_.derivate(coords, ed, J);
      return;
    }
 
    // Cheaper implementation
    J.resize(1);
    J[0] = transform_.project(Point(1.0,0.0,0.0)); 
  }

  //! Get the determinant of the jacobian, which is the local volume of an element
  //! and is intended to help with the integration of functions over an element.
  template<class VECTOR>
  double det_jacobian(const VECTOR& coords,
                typename Elem::index_type idx) const
  {
    if (basis_.polynomial_order() > 1) 
    {  
      double J[9];
      jacobian(coords,idx,J);
      return (DetMatrix3x3(J));
    }
    
    return (det_jacobian_);
  }

  //! Get the jacobian of the transformation. In case one wants the non inverted
  //! version of this matrix. This is currentl here for completeness of the 
  //! interface
  template<class VECTOR>
  void jacobian(const VECTOR& coords, typename Elem::index_type idx, double* J) const
  {
    if (basis_.polynomial_order() > 1) 
    {  
      StackVector<Point,3> Jv;
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

      return;
    }    
  
    J[0] = jacobian_[0];
    J[1] = jacobian_[1];
    J[2] = jacobian_[2];
    J[3] = jacobian_[3];
    J[4] = jacobian_[4];
    J[5] = jacobian_[5];
    J[6] = jacobian_[6];
    J[7] = jacobian_[7];
    J[8] = jacobian_[8];
  }

  //! Get the inverse jacobian of the transformation. This one is needed to 
  //! translate local gradients into global gradients. Hence it is crucial for
  //! calculating gradients of fields, or constructing finite elements.             
  template<class VECTOR>
  double inverse_jacobian(const VECTOR& coords, typename Elem::index_type idx, double* Ji) const
  {
    if (basis_.polynomial_order() > 1) 
    {  
      StackVector<Point,2> Jv;
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
  
    Ji[0] = inverse_jacobian_[0];
    Ji[1] = inverse_jacobian_[1];
    Ji[2] = inverse_jacobian_[2];
    Ji[3] = inverse_jacobian_[3];
    Ji[4] = inverse_jacobian_[4];
    Ji[5] = inverse_jacobian_[5];
    Ji[6] = inverse_jacobian_[6];
    Ji[7] = inverse_jacobian_[7];
    Ji[8] = inverse_jacobian_[8];

    return (det_inverse_jacobian_);
  }

  double scaled_jacobian_metric(typename Elem::index_type idx) const
    { return (scaled_jacobian_); }

  double jacobian_metric(typename Elem::index_type idx) const
    { return (det_jacobian_); }


  double get_epsilon() const;

  //! Export this class using the old Pio system
  virtual void io(Piostream&);
  //! These IDs are created as soon as this class will be instantiated
  //! The first one is for Pio and the second for the virtual interface
  //! These are currently different as they serve different needs.
  static PersistentTypeID scanline_typeid;
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
  static Persistent *maker() { return scinew ScanlineMesh(); }
  //! This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return scinew ScanlineMesh(); }
  //! This function returns a handle for the virtual interface.
  static MeshHandle scanline_maker(size_type x, const Point& min, const Point& max) { return scinew ScanlineMesh(x,min,max); }

  //! This function will find the closest element and the location on that
  //! element that is the closest
  double find_closest_elem(Point &result, typename Elem::index_type &elem,
                           const Point &p) const;

  //! This function will return multiple elements if the closest point is
  //! located on a node or edge. All bordering elements are returned in that 
  //! case. 
  double find_closest_elems(Point &result, vector<typename Elem::index_type> &elem,
                            const Point &p) const;

protected:

  void compute_jacobian();

  //! the min typename Node::index_type ( incase this is a subLattice )
  index_type           min_i_;

  //! the typename Node::index_type space extents of a ScanlineMesh
  //! (min=min_typename Node::index_type, max=min+extents-1)
  size_type            ni_;

  //! the object space extents of a ScanlineMesh
  Transform            transform_;

  //! the basis fn
  Basis                basis_;

  Handle<VMesh>        vmesh_;

  // The jacobian is the same for every element
  // hence store them as soon as we know the transfrom_
  // This should speed up FE computations on these regular grids.
  double jacobian_[9];
  double inverse_jacobian_[9];
  double det_jacobian_;
  double scaled_jacobian_;
  double det_inverse_jacobian_;
  
};


template <class Basis>
PersistentTypeID
ScanlineMesh<Basis>::scanline_typeid(type_name(-1), "Mesh",
                             ScanlineMesh<Basis>::maker);


template <class Basis>
ScanlineMesh<Basis>::ScanlineMesh(size_type ni,
                                  const Point &min, const Point &max)
  : min_i_(0), ni_(ni)
{
  transform_.pre_scale(Vector(1.0 / (ni_ - 1.0), 1.0, 1.0));
  transform_.pre_scale(max - min);
  transform_.pre_translate(Vector(min));
  transform_.compute_imat();
  compute_jacobian();
  
  //! Create a new virtual interface for this copy
  //! all pointers have changed hence create a new
  //! virtual interface class
  vmesh_ = CreateVScanlineMesh(this);   
}


template <class Basis>
BBox
ScanlineMesh<Basis>::get_bounding_box() const
{
  Point p0(0.0, 0.0, 0.0);
  Point p1(ni_ - 1, 0.0, 0.0);

  BBox result;
  result.extend(transform_.project(p0));
  result.extend(transform_.project(p1));
  return result;
}


template <class Basis>
Vector
ScanlineMesh<Basis>::diagonal() const
{
  return get_bounding_box().diagonal();
}


template <class Basis>
void
ScanlineMesh<Basis>::transform(const Transform &t)
{
  transform_.pre_trans(t);
  compute_jacobian();
}


template <class Basis>
void
ScanlineMesh<Basis>::get_canonical_transform(Transform &t)
{
  t = transform_;
  t.post_scale(Vector(ni_ - 1.0, 1.0, 1.0));
}


template <class Basis>
bool
ScanlineMesh<Basis>::get_min(vector<index_type> &array ) const
{
  array.resize(1);
  array.clear();

  array.push_back(min_i_);

  return true;
}


template <class Basis>
bool
ScanlineMesh<Basis>::get_dim(vector<size_type> &array) const
{
  array.resize(1);
  array.clear();

  array.push_back(ni_);

  return true;
}


template <class Basis>
void
ScanlineMesh<Basis>::set_min(vector<index_type> min)
{
  min_i_ = min[0];
}


template <class Basis>
void
ScanlineMesh<Basis>::set_dim(vector<size_type> dim)
{
  ni_ = dim[0];

  //! Create a new virtual interface for this copy
  //! all pointers have changed hence create a new
  //! virtual interface class
  vmesh_ = CreateVScanlineMesh(this); 
}


template <class Basis>
void
ScanlineMesh<Basis>::get_nodes(typename Node::array_type &array,
                               typename Edge::index_type idx) const
{
  array.resize(2);
  array[0] = typename Node::index_type(idx);
  array[1] = typename Node::index_type(idx + 1);
}


template <class Basis>
void
ScanlineMesh<Basis>::get_elems(typename Edge::array_type &result,
                               typename Node::index_type index) const
{
  result.reserve(2);
  result.clear();
  if (index > 0)
  {
    result.push_back(typename Edge::index_type(index-1));
  }
  if (index < ni_-1)
  {
    result.push_back(typename Edge::index_type(index));
  }
}


template <class Basis>
void
ScanlineMesh<Basis>::get_center(Point &result,
                                typename Node::index_type idx) const
{
  Point p(idx, 0.0, 0.0);
  result = transform_.project(p);
}


template <class Basis>
void
ScanlineMesh<Basis>::get_center(Point &result,
                                typename Edge::index_type idx) const
{
  Point p(idx + 0.5, 0.0, 0.0);
  result = transform_.project(p);
}


// TODO: verify
template <class Basis>
bool
ScanlineMesh<Basis>::locate(typename Edge::index_type &elem, const Point &p) const
{
  if (basis_.polynomial_order() > 1) return elem_locate(elem, *this, p);
  const Point r = transform_.unproject(p);
  elem = (unsigned int)(r.x());

  if (elem >= (ni_ - 1))
  {
    return false;
  }
  else
  {
    return true;
  }
}


// TODO: verify
template <class Basis>
bool
ScanlineMesh<Basis>::locate(typename Node::index_type &node, const Point &p) const
{
  const Point r = transform_.unproject(p);
  node = static_cast<index_type>(r.x() + 0.5);

  if (node >= ni_)
  {
    return false;
  }
  else
  {
    return true;
  }
}


template <class Basis>
int
ScanlineMesh<Basis>::get_weights(const Point &p, typename Node::array_type &l,
                                 double *w)
{
  typename Edge::index_type idx;
  if (locate(idx, p))
  {
    get_nodes(l,idx);
    vector<double> coords(1);
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
ScanlineMesh<Basis>::get_weights(const Point &p, typename Edge::array_type &l,
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
ScanlineMesh<Basis>::get_random_point(Point &p,
                                      typename Elem::index_type ei,
                                      FieldRNG &rng) const
{
  Point p0, p1;
  get_center(p0, typename Node::index_type(ei));
  get_center(p1, typename Node::index_type(under_type(ei)+1));

  p = p0 + (p1 - p0) * rng();
}


#define SCANLINEMESH_VERSION 4

template <class Basis>
void
ScanlineMesh<Basis>::io(Piostream& stream)
{
  int version = stream.begin_class(type_name(-1), SCANLINEMESH_VERSION);

  Mesh::io(stream);

  // IO data members, in order
  Pio(stream, ni_);
  
  if (version < 4)
  {
    unsigned int ni = static_cast<unsigned int>(ni_);
    Pio(stream, ni);
    ni_ = static_cast<size_type>(ni);
  }
  else
  {
    Pio_size(stream, ni_);
  }  
  
  if (version < 2 && stream.reading() )
  {
    Pio_old(stream, transform_);
  }
  else
  {
    Pio(stream, transform_);
  }
  if (version >= 3)
  {
    basis_.io(stream);
  }
  stream.end_class();

  if (stream.reading())
    vmesh_ = CreateVScanlineMesh(this);
}


template <class Basis>
const string
ScanlineMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const string name = type_name(0) + FTNS + type_name(1) + FTNE;
    return name;
  }
  else if (n == 0)
  {
    static const string nm("ScanLineMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
void
ScanlineMesh<Basis>::begin(typename ScanlineMesh::Node::iterator &itr) const
{
  itr = typename Node::iterator(min_i_);
}


template <class Basis>
void
ScanlineMesh<Basis>::end(typename ScanlineMesh::Node::iterator &itr) const
{
  itr = typename Node::iterator(min_i_ + ni_);
}


template <class Basis>
void
ScanlineMesh<Basis>::size(typename ScanlineMesh::Node::size_type &s) const
{
  s = typename Node::size_type(ni_);
}


template <class Basis>
void
ScanlineMesh<Basis>::begin(typename ScanlineMesh::Edge::iterator &itr) const
{
  itr = typename Edge::iterator(min_i_);
}


template <class Basis>
void
ScanlineMesh<Basis>::end(typename ScanlineMesh::Edge::iterator &itr) const
{
  itr = typename Edge::iterator(min_i_+ni_-1);
}


template <class Basis>
void
ScanlineMesh<Basis>::size(typename ScanlineMesh::Edge::size_type &s) const
{
  s = typename Edge::size_type(ni_ - 1);
}


template <class Basis>
void
ScanlineMesh<Basis>::begin(typename ScanlineMesh::Face::iterator &itr) const
{
  itr = typename Face::iterator(0);
}


template <class Basis>
void
ScanlineMesh<Basis>::end(typename ScanlineMesh::Face::iterator &itr) const
{
  itr = typename Face::iterator(0);
}


template <class Basis>
void
ScanlineMesh<Basis>::size(typename ScanlineMesh::Face::size_type &s) const
{
  s = typename Face::size_type(0);
}


template <class Basis>
void
ScanlineMesh<Basis>::begin(typename ScanlineMesh::Cell::iterator &itr) const
{
  itr = typename Cell::iterator(0);
}


template <class Basis>
void
ScanlineMesh<Basis>::end(typename ScanlineMesh::Cell::iterator &itr) const
{
  itr = typename Cell::iterator(0);
}


template <class Basis>
void
ScanlineMesh<Basis>::size(typename ScanlineMesh::Cell::size_type &s) const
{
  s = typename Cell::size_type(0);
}


template <class Basis>
double
ScanlineMesh<Basis>::get_epsilon() const
{
  Point p(ni_-1,0.0,0.0);
  Point q = transform_.project(p);
  return (q.asVector().length()*1e-8);
}

template <class Basis>
const TypeDescription*
get_type_description(ScanlineMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = SCIRun::get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = scinew TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = scinew TypeDescription("ScanlineMesh", subs,
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
ScanlineMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((ScanlineMesh *)0);
}


template <class Basis>
const TypeDescription*
ScanlineMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((ScanlineMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Node",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
ScanlineMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((ScanlineMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Edge",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
ScanlineMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((ScanlineMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Face",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
ScanlineMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((ScanlineMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Cell",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
void 
ScanlineMesh<Basis>::compute_jacobian()
{
  if (basis_.polynomial_order() < 2)
  { 
    Vector J1 = transform_.project(Vector(1.0,0.0,0.0)); 
    Vector J2, J3;
    J1.find_orthogonal(J2,J3);
    J2.normalize();
    J3.normalize();
    
    jacobian_[0] = J1.x();
    jacobian_[1] = J1.y();
    jacobian_[2] = J1.z();
    jacobian_[3] = J2.x();
    jacobian_[4] = J2.y();
    jacobian_[5] = J2.z();
    jacobian_[6] = J3.x();
    jacobian_[7] = J3.y();
    jacobian_[8] = J3.z();
    
    det_jacobian_ = DetMatrix3x3(jacobian_);
    scaled_jacobian_ = ScaledDetMatrix3x3(jacobian_);
    det_inverse_jacobian_ = InverseMatrix3x3(jacobian_,inverse_jacobian_);
  }
}


template <class Basis>
double 
ScanlineMesh<Basis>::find_closest_elem(Point &result, typename Elem::index_type &elem,
                           const Point &p) const
{
  result = transform_.unproject(p);
  
  if (result.x() < 0.0) result.x(0.0);
  if (result.x() > static_cast<double>(ni_-1)) result.x(static_cast<double>(ni_-1));
  elem = static_cast<index_type>(floor(result.x()));
  if (elem > ni_-1) elem = ni_-1;
  
  return((p-result).length());
}

template <class Basis>
double 
ScanlineMesh<Basis>::find_closest_elems(Point &result,
                            vector<typename Elem::index_type> &elem,
                            const Point &p) const
{
  result = transform_.unproject(p);
  
  if (result.x() < 0.0) result.x(0.0);
  if (result.x() > static_cast<double>(ni_-1)) result.x(static_cast<double>(ni_-1));
  
  index_type i = static_cast<index_type>(floor(result.x()));
  elem.push_back(i);
  
  if ((fabs(static_cast<double>(i)-result.x()) < MIN_ELEMENT_VAL) && ((i-1)>0))
  {
    elem.push_back(i-1);  
  }
  
  if ((fabs(static_cast<double>(i+1)-result.x()) < MIN_ELEMENT_VAL) && (i<(ni_-1)))
  {
    elem.push_back(i+1);  
  }

  return((p-result).length());
}

} // namespace SCIRun

#endif // SCI_project_ScanlineMesh_h
