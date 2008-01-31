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
  A structured curve is a dataset with regular topology but with
  irregular geometry.  The line defined may have any shape but can not
  be overlapping or self-intersecting.

  The topology of structured curve is represented using a 1D vector with
  the points being stored in an index based array. The ordering of the curve is
  implicity defined based based upon its indexing.

  For more information on datatypes see Schroeder, Martin, and Lorensen,
  "The Visualization Toolkit", Prentice Hall, 1998.
*/


#ifndef CORE_DATATYPES_STRUCTCURVEMESH_H
#define CORE_DATATYPES_STRUCTCURVEMESH_H 1

//! Include what kind of support we want to have
//! Need to fix this and couple it to sci-defs
#include <Core/Datatypes/MeshSupport.h>

#include <Core/Geometry/Point.h>
#include <Core/Geometry/Plane.h>
#include <Core/Geometry/CompGeom.h>

#include <Core/Containers/Array2.h>

#include <Core/Datatypes/ScanlineMesh.h>
#include <Core/Datatypes/SearchGrid.h>

//! Incude needed for Windows: declares SCISHARE
#include <Core/Datatypes/share.h>

namespace SCIRun {

using std::string;
using std::vector;


//! Declarations for virtual interface


//! Functions for creating the virtual interface
//! Declare the functions that instantiate the virtual interface
template <class Basis>
class StructCurveMesh;

//! make sure any other mesh other than the preinstantiate ones
//! returns no virtual interface. Altering this behaviour will allow
//! for dynamically compiling the interfae if needed.
template<class MESH>
VMesh* CreateVStructCurveMesh(MESH* mesh) { return (0); }

//! These declarations are needed for a combined dynamic compilation as
//! as well as virtual functions solution.
//! Declare that these can be found in a library that is already
//! precompiled. So dynamic compilation will not instantiate them again.

#if (SCIRUN_STRUCTCURVE_SUPPORT > 0)

SCISHARE VMesh* CreateVStructCurveMesh(StructCurveMesh<CrvLinearLgn<Point> >* mesh);

#endif


template <class Basis>
class StructCurveMesh : public ScanlineMesh<Basis>
{

//! Make sure the virtual interface has access
template <class MESH> friend class VStructCurveMesh;

public:
  //! Types that change depending on 32 or 64bits
  typedef SCIRun::index_type            under_type;
  typedef SCIRun::index_type            index_type;
  typedef SCIRun::size_type             size_type;
  typedef SCIRun::mask_type             mask_type; 

  StructCurveMesh();
  StructCurveMesh(size_type n);  
  StructCurveMesh(const StructCurveMesh &copy);
  virtual StructCurveMesh *clone() { return new StructCurveMesh(*this); }
  virtual ~StructCurveMesh() {}

  //! get the mesh statistics
  double get_cord_length() const;
  virtual BBox get_bounding_box() const;
  virtual void transform(const Transform &t);

  virtual bool get_dim(vector<size_type>&) const;
  virtual void set_dim(vector<size_type> dims) {
    ScanlineMesh<Basis>::ni_ = dims[0];

    points_.resize(dims[0]);

    //! Create a new virtual interface for this copy
    //! all pointers have changed hence create a new
    //! virtual interface class
    ScanlineMesh<Basis>::vmesh_ = CreateVStructCurveMesh(this); 
  }

  virtual int topology_geometry() const
  {
    return (Mesh::STRUCTURED | Mesh::IRREGULAR);
  }

  //! get the child elements of the given index
  void get_nodes(typename ScanlineMesh<Basis>::Node::array_type &,
                 typename ScanlineMesh<Basis>::Edge::index_type) const;
  void get_nodes(typename ScanlineMesh<Basis>::Node::array_type &,
                 typename ScanlineMesh<Basis>::Face::index_type) const {}
  void get_nodes(typename ScanlineMesh<Basis>::Node::array_type &,
                 typename ScanlineMesh<Basis>::Cell::index_type) const {}
  void get_edges(typename ScanlineMesh<Basis>::Edge::array_type &,
                 typename ScanlineMesh<Basis>::Face::index_type) const {}
  void get_edges(typename ScanlineMesh<Basis>::Edge::array_type &,
                 typename ScanlineMesh<Basis>::Cell::index_type) const {}
  void get_edges(typename ScanlineMesh<Basis>::Edge::array_type &a,
                 typename ScanlineMesh<Basis>::Edge::index_type idx) const
  { a.push_back(idx);}

  //! Get the size of an elemnt (length, area, volume)
  double get_size(typename ScanlineMesh<Basis>::Node::index_type) const
  { return 0.0; }
  double get_size(typename ScanlineMesh<Basis>::Edge::index_type idx) const
  {
    typename ScanlineMesh<Basis>::Node::array_type arr;
    get_nodes(arr, idx);
    Point p0, p1;
    get_center(p0, arr[0]);
    get_center(p1, arr[1]);
    return (p1.asVector() - p0.asVector()).length();
  }

  double get_size(typename ScanlineMesh<Basis>::Face::index_type) const
  { return 0.0; }
  double get_size(typename ScanlineMesh<Basis>::Cell::index_type) const
  { return 0.0; }
  double get_length(typename ScanlineMesh<Basis>::Edge::index_type idx) const
  { return get_size(idx); }
  double get_area(typename ScanlineMesh<Basis>::Face::index_type idx) const
  { return get_size(idx); }
  double get_volume(typename ScanlineMesh<Basis>::Cell::index_type idx) const
  { return get_size(idx); }

  int get_valence(typename ScanlineMesh<Basis>::Node::index_type idx) const
  { return (idx == 0 || idx == static_cast<index_type>((points_.size()-1)) ? 1 : 2); }

  int get_valence(typename ScanlineMesh<Basis>::Edge::index_type) const
  { return 0; }
  int get_valence(typename ScanlineMesh<Basis>::Face::index_type) const
  { return 0; }
  int get_valence(typename ScanlineMesh<Basis>::Cell::index_type) const
  { return 0; }

  //! get the center point (in object space) of an element
  void get_center(Point &,
                  const typename ScanlineMesh<Basis>::Node::index_type&) const;
  void get_center(Point &,
                  const typename ScanlineMesh<Basis>::Edge::index_type&) const;
  void get_center(Point &,
                  const typename ScanlineMesh<Basis>::Face::index_type&) const
  {}
  void get_center(Point &,
                  const typename ScanlineMesh<Basis>::Cell::index_type&) const
  {}

  bool locate(typename ScanlineMesh<Basis>::Node::index_type &,
              const Point &) const;
  bool locate(typename ScanlineMesh<Basis>::Edge::index_type &,
              const Point &) const;
  bool locate(typename ScanlineMesh<Basis>::Face::index_type &,
              const Point &) const
  { return false; }
  bool locate(typename ScanlineMesh<Basis>::Cell::index_type &,
              const Point &) const
  { return false; }

  int get_weights(const Point &,
                  typename ScanlineMesh<Basis>::Node::array_type &, double *w);
  int get_weights(const Point &,
                  typename ScanlineMesh<Basis>::Edge::array_type &, double *w);
  int get_weights(const Point &,
                  typename ScanlineMesh<Basis>::Face::array_type &, double *)
  {ASSERTFAIL("StructCurveMesh::get_weights for faces isn't supported"); }
  int get_weights(const Point &,
                  typename ScanlineMesh<Basis>::Cell::array_type &, double *)
  {ASSERTFAIL("StructCurveMesh::get_weights for cells isn't supported"); }

  void get_point(Point &p, typename ScanlineMesh<Basis>::Node::index_type i) const
  { get_center(p,i); }
  void set_point(const Point &p, typename ScanlineMesh<Basis>::Node::index_type i)
  { points_[i] = p; }

  void get_random_point(Point &p,
                        const typename ScanlineMesh<Basis>::Elem::index_type idx,
                        FieldRNG &rng) const;

  void get_normal(Vector &,
                  typename ScanlineMesh<Basis>::Node::index_type) const
  { ASSERTFAIL("This mesh type does not have node normals."); }

  void get_normal(Vector &, vector<double> &,
                  typename ScanlineMesh<Basis>::Elem::index_type,
                  unsigned int)
  { ASSERTFAIL("This mesh type does not have element normals."); }

  class ElemData
  {
  public:
    typedef typename StructCurveMesh<Basis>::index_type  index_type;

    ElemData(const StructCurveMesh<Basis>& msh,
             const typename ScanlineMesh<Basis>::Elem::index_type ind) :
      mesh_(msh),
      index_(ind)
    {}

    //! the following designed to coordinate with ::get_nodes
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


    //! the following designed to coordinate with ::get_edges
    inline
    index_type edge0_index() const 
    {
      return index_;
    }

    inline
    const Point &node0() const 
    {
      return mesh_.points_[index_];
    }
    inline
    const Point &node1() const 
    {
      return mesh_.points_[index_+1];
    }

  private:
    const StructCurveMesh<Basis>                             &mesh_;
    const typename ScanlineMesh<Basis>::Elem::index_type     index_;
  };

  friend class ElemData;

 //! Generate the list of points that make up a sufficiently accurate
  //! piecewise linear approximation of an edge.
  void pwl_approx_edge(vector<vector<double> > &coords,
                       typename ScanlineMesh<Basis>::Elem::index_type ci,
                       unsigned int,
                       unsigned int div_per_unit) const
  {
    //! Needs to match unit_edges in Basis/QuadBilinearLgn.cc
    //! compare get_nodes order to the basis order
    this->basis_.approx_edge(0, div_per_unit, coords);
  }

  //! Generate the list of points that make up a sufficiently accurate
  //! piecewise linear approximation of an face.
  void pwl_approx_face(vector<vector<vector<double> > > &coords,
                       typename ScanlineMesh<Basis>::Elem::index_type ci,
                       typename ScanlineMesh<Basis>::Face::index_type fi,
                       unsigned int div_per_unit) const
  {
    ASSERTFAIL("ScanlineMesh has no faces");
  }


  //! Get the local coordinates for a certain point within an element
  //! This function uses a couple of newton iterations to find the
  //! local ! coordinate of a point
  template<class VECTOR>
  bool get_coords(VECTOR &coords, const Point &p,
		  typename ScanlineMesh<Basis>::Elem::index_type idx) const
  {
    ElemData ed(*this, idx);
    return this->basis_.get_coords(coords, p, ed);
  }

  //! Find the location in the global coordinate system for a local
  //! coordinate ! This function is the opposite of get_coords.
  template<class VECTOR>
  void interpolate(Point &pt, const VECTOR &coords,
		   typename ScanlineMesh<Basis>::Elem::index_type idx) const
  {
    ElemData ed(*this, idx);
    pt = this->basis_.interpolate(coords, ed);
  }

  //! Interpolate the derivate of the function, This infact will
  //! return the ! jacobian of the local to global coordinate
  //! transformation. This function ! is mainly intended for the non
  //! linear elements
  template<class VECTOR1, class VECTOR2>
  void derivate(const VECTOR1 &coords,
		typename ScanlineMesh<Basis>::Elem::index_type idx,
		VECTOR2 &J) const
  {
    ElemData ed(*this, idx);
    this->basis_.derivate(coords, ed, J);
  }

  //! Get the determinant of the jacobian, which is the local volume
  //! of an element ! and is intended to help with the integration of
  //! functions over an element.
  template<class VECTOR>
  double det_jacobian(const VECTOR& coords,
		      typename ScanlineMesh<Basis>::Elem::index_type idx) const
  {
    double J[9];
    jacobian(coords,idx,J);
    return (DetMatrix3x3(J));
  }

  //! Get the jacobian of the transformation. In case one wants the
  //! non inverted ! version of this matrix. This is currentl here for
  //! completeness of the ! interface
  template<class VECTOR>
  void jacobian(const VECTOR& coords,
		typename ScanlineMesh<Basis>::Elem::index_type idx,
		double* J) const
  {
    StackVector<Point,1> Jv;
    ElemData ed(*this,idx);
    this->basis_.derivate(coords,ed,Jv);
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

  //! Get the inverse jacobian of the transformation. This one is
  //! needed to ! translate local gradients into global gradients. Hence
  //! it is crucial for ! calculating gradients of fields, or
  //! constructing finite elements.
  template<class VECTOR>                
  double inverse_jacobian(const VECTOR& coords,
			  typename ScanlineMesh<Basis>::Elem::index_type idx,
			  double* Ji) const
  {
    StackVector<Point,1> Jv;
    ElemData ed(*this,idx);
    this->basis_.derivate(coords,ed,Jv);
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

    this->basis_.derivate(this->basis_.unit_center,ed,Jv);
    Jv.resize(3); 
    Vector v,w;
    Jv[0].asVector().find_orthogonal(v,w);
    Jv[1] = v.asPoint();
    Jv[2] = w.asPoint();
    double min_jacobian = ScaledDetMatrix3P(Jv);
    
    size_t num_vertices = this->basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      this->basis_.derivate(this->basis_.unit_vertices[j],ed,Jv);
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

    this->basis_.derivate(this->basis_.unit_center,ed,Jv);
    Jv.resize(3); 
    Vector v,w;
    Jv[0].asVector().find_orthogonal(v,w);
    Jv[1] = v.asPoint();
    Jv[2] = w.asPoint();
    double min_jacobian = DetMatrix3P(Jv);
    
    size_t num_vertices = this->basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      this->basis_.derivate(this->basis_.unit_vertices[j],ed,Jv);
      Jv.resize(3); 
      Jv[0].asVector().find_orthogonal(v,w);
      Jv[1] = v.asPoint();
      Jv[2] = w.asPoint();
      temp = DetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }
      
    return (min_jacobian);
  }


  //! Export this class using the old Pio system
  virtual void io(Piostream&);
  //! These IDs are created as soon as this class will be instantiated
  //! The first one is for Pio and the second for the virtual
  //! interface ! These are currently different as they serve different
  //! needs.  static PersistentTypeID type_idts;
  static PersistentTypeID scanline_typeid;
  //! Core functionality for getting the name of a templated mesh class  
  static  const string type_name(int n = -1);
  
  //! Type description, used for finding names of the mesh class for
  //! dynamic compilation purposes. Soem of this should be obsolete  
  virtual const TypeDescription *get_type_description() const;

  //! This function returns a maker for Pio.
  static Persistent *maker() { return scinew StructCurveMesh<Basis>(); }
  //! This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return scinew StructCurveMesh<Basis>();}
  //! This function returns a handle for the virtual interface.
  static MeshHandle structcurve_maker(size_type x) { return scinew StructCurveMesh<Basis>(x);}

  vector<Point>& get_points() { return (points_); }

  virtual bool synchronize(mask_type sync);
  virtual bool unsynchronize(mask_type sync);

  double get_epsilon() const;

private:

  void compute_epsilon();

  vector<Point> points_;
  Mutex         synchronize_lock_;
  mask_type     synchronized_;  
  double        epsilon_;
};


template <class Basis>
PersistentTypeID
StructCurveMesh<Basis>::scanline_typeid(StructCurveMesh<Basis>::type_name(-1),
                                "Mesh", maker);


template <class Basis>
StructCurveMesh<Basis>::StructCurveMesh():
  points_(0),
  synchronize_lock_("Synchronize lock"),
  synchronized_(Mesh::ALL_ELEMENTS_E),
  epsilon_(0.0)
{
    //! Create a new virtual interface for this copy
    //! all pointers have changed hence create a new
    //! virtual interface class
    this->vmesh_ = CreateVStructCurveMesh(this);     
}


template <class Basis>
StructCurveMesh<Basis>::StructCurveMesh(size_type n)
  : ScanlineMesh<Basis>(n, Point(0.0, 0.0, 0.0), Point(1.0, 1.0, 1.0)),
    points_(n),
    synchronize_lock_("StructCurveMesh lock"),
    synchronized_(Mesh::EDGES_E | Mesh::NODES_E),
    epsilon_(0.0)
{
  //! Create a new virtual interface for this copy
  //! all pointers have changed hence create a new
  //! virtual interface class
  this->vmesh_ = CreateVStructCurveMesh(this);  
}


template <class Basis>
StructCurveMesh<Basis>::StructCurveMesh(const StructCurveMesh &copy)
  : ScanlineMesh<Basis>(copy),
    points_(copy.points_),
    synchronize_lock_("StructCurveMesh lock"),
    synchronized_(copy.synchronized_),
    epsilon_(copy.epsilon_)
{
  //! Create a new virtual interface for this copy
  //! all pointers have changed hence create a new
  //! virtual interface class
  this->vmesh_ = CreateVStructCurveMesh(this);   
}


template <class Basis>
bool
StructCurveMesh<Basis>::get_dim(vector<size_type> &array) const
{
  array.resize(1);
  array.clear();

  array.push_back(this->ni_);

  return true;
}


template <class Basis>
double
StructCurveMesh<Basis>::get_epsilon() const
{
  return(epsilon_);
}

template <class Basis>
void
StructCurveMesh<Basis>::compute_epsilon()
{
  epsilon_ = get_bounding_box().diagonal().length()*1e-8;
}

template <class Basis>
BBox
StructCurveMesh<Basis>::get_bounding_box() const
{
  BBox result;

  typename ScanlineMesh<Basis>::Node::iterator i, ie;
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
StructCurveMesh<Basis>::transform(const Transform &t)
{
  typename ScanlineMesh<Basis>::Node::iterator i, ie;
  begin(i);
  end(ie);

  while (i != ie) 
  {
    points_[*i] = t.project(points_[*i]);

    ++i;
  }
}


template <class Basis>
double
StructCurveMesh<Basis>::get_cord_length() const
{
  double result = 0.0;

  typename ScanlineMesh<Basis>::Node::iterator i, i1, ie;
  begin(i);
  begin(i1);
  end(ie);

  while (i1 != ie)
  {
    ++i1;
    result += (points_[*i] - points_[*i1]).length();
    ++i;
  }

  return result;
}


template <class Basis>
void
StructCurveMesh<Basis>::get_nodes(typename ScanlineMesh<Basis>::Node::array_type &array, typename ScanlineMesh<Basis>::Edge::index_type idx) const
{
  array.resize(2);
  array[0] = typename ScanlineMesh<Basis>::Node::index_type(idx);
  array[1] = typename ScanlineMesh<Basis>::Node::index_type(idx + 1);
}


template <class Basis>
void
StructCurveMesh<Basis>::get_center(Point &result, const typename ScanlineMesh<Basis>::Node::index_type &idx) const
{
  result = points_[idx];
}


template <class Basis>
void
StructCurveMesh<Basis>::get_center(Point &result, const typename ScanlineMesh<Basis>::Edge::index_type &idx) const
{
  const Point &p0 = points_[typename ScanlineMesh<Basis>::Node::index_type(idx)];
  const Point &p1 = points_[typename ScanlineMesh<Basis>::Node::index_type(idx+1)];

  result = Point(p0+p1)/2.0;
}


template <class Basis>
bool
StructCurveMesh<Basis>::locate(typename ScanlineMesh<Basis>::Node::index_type &idx, const Point &p) const
{
  typename ScanlineMesh<Basis>::Node::iterator ni, nie;
  begin(ni);
  end(nie);

  idx = *ni;

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
      idx = *ni;
    }
  }

  return true;
}


template <class Basis>
bool
StructCurveMesh<Basis>::locate(typename ScanlineMesh<Basis>::Edge::index_type &idx, const Point &p) const
{
  if (this->basis_.polynomial_order() > 1) return elem_locate(idx, *this, p);
  typename ScanlineMesh<Basis>::Edge::iterator ei;
  typename ScanlineMesh<Basis>::Edge::iterator eie;
  double cosa, closest=DBL_MAX;
  typename ScanlineMesh<Basis>::Node::array_type nra;
  double dist1, dist2, dist3, dist4;
  Point n1,n2,q;

  begin(ei);
  end(eie);

  if (ei==eie)
    return false;

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
      idx = *ei;
    } 
    else if ( (cosa < 0) && (dist1 < closest) ) 
    {
      closest = dist1;
      idx = *ei;
    } 
    else if ( (cosa > dist3) && (dist2 < closest) ) 
    {
      closest = dist2;
      idx = *ei;
    }
  }

  return true;
}


template <class Basis>
int
StructCurveMesh<Basis>::get_weights(const Point &p,
				    typename ScanlineMesh<Basis>::Node::array_type &l,
				    double *w)
{
  typename ScanlineMesh<Basis>::Edge::index_type idx;
  if (locate(idx, p))
  {
    get_nodes(l,idx);
    vector<double> coords(1);
    if (get_coords(coords, p, idx))
    {
      this->basis_.get_weights(coords, w);
      return this->basis_.dofs();
    }
  }
  return 0;
}


template <class Basis>
int
StructCurveMesh<Basis>::get_weights(const Point &p,
				    typename ScanlineMesh<Basis>::Edge::array_type &l,
                                    double *w)
{
  typename ScanlineMesh<Basis>::Edge::index_type idx;
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
StructCurveMesh<Basis>::get_random_point(Point &p,
					 const typename ScanlineMesh<Basis>::Elem::index_type i,
                                         FieldRNG &rng) const
{
  const Point &p0 =points_[typename ScanlineMesh<Basis>::Node::index_type(i)];
  const Point &p1=points_[typename ScanlineMesh<Basis>::Node::index_type(i+1)];

  p = p0 + (p1 - p0) * rng();
}


template <class Basis>
bool
StructCurveMesh<Basis>::synchronize(mask_type sync)
{
  synchronize_lock_.lock();
  if (sync & Mesh::EPSILON_E && !(synchronized_ & Mesh::EPSILON_E))
  {
    compute_epsilon();
    synchronized_ |= Mesh::EPSILON_E;
  }
  synchronize_lock_.unlock();
  return (true);
}

template <class Basis>
bool
StructCurveMesh<Basis>::unsynchronize(mask_type sync)
{
  return true;
}


#define STRUCT_CURVE_MESH_VERSION 1

template <class Basis>
void
StructCurveMesh<Basis>::io(Piostream& stream)
{
  stream.begin_class(type_name(-1), STRUCT_CURVE_MESH_VERSION);
  ScanlineMesh<Basis>::io(stream);

  //! IO data members, in order
  Pio(stream, points_);

  stream.end_class();

  if (stream.reading())
    this->vmesh_ = CreateVStructCurveMesh(this);
}


template <class Basis>
const string
StructCurveMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const string name = type_name(0) + FTNS + type_name(1) + FTNE;
    return name;
  }
  else if (n == 0)
  {
    static const string nm("StructCurveMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
const TypeDescription*
get_type_description(StructCurveMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = SCIRun::get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = scinew TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = scinew TypeDescription("StructCurveMesh", subs,
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructCurveMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((StructCurveMesh<Basis> *)0);
}


} //! namespace SCIRun

#endif //! SCI_project_StructCurveMesh_h
