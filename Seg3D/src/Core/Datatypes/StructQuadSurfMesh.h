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
  A structured grid is a dataset with regular topology but with irregular
  geometry.  The grid may have any shape but can not be overlapping or
  self-intersecting. The topology of a structured grid is represented
  using a 2D, or 3D vector with the points being stored in an index
  based array. The faces (quadrilaterals) and  cells (Hexahedron) are
  implicitly define based based upon their indexing.

  Structured grids are typically used in finite difference analysis.

  For more information on datatypes see Schroeder, Martin, and Lorensen,
  "The Visualization Toolkit", Prentice Hall, 1998.
*/


#ifndef CORE_DATATYPES_STRUCTQUADSURFMESH_H
#define CORE_DATATYPES_STRUCTQUADSURFMESH_H 1

//! Include what kind of support we want to have
//! Need to fix this and couple it to sci-defs
#include <Core/Datatypes/MeshSupport.h>

#include <Core/Geometry/Point.h>
#include <Core/Geometry/Plane.h>
#include <Core/Geometry/CompGeom.h>

#include <Core/Containers/Array2.h>

#include <Core/Datatypes/ImageMesh.h>
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
class StructQuadSurfMesh;

//! make sure any other mesh other than the preinstantiate ones
//! returns no virtual interface. Altering this behaviour will allow
//! for dynamically compiling the interfae if needed.
template<class MESH>
VMesh* CreateVStructQuadSurfMesh(MESH* mesh) { return (0); }

//! These declarations are needed for a combined dynamic compilation as
//! as well as virtual functions solution.
//! Declare that these can be found in a library that is already
//! precompiled. So dynamic compilation will not instantiate them again.

#if (SCIRUN_STRUCTQUADSURF_SUPPORT > 0)

SCISHARE VMesh* CreateVStructQuadSurfMesh(StructQuadSurfMesh<QuadBilinearLgn<Point> >* mesh);

#endif


template <class Basis>
class StructQuadSurfMesh : public ImageMesh<Basis>
{

//! Make sure the virtual interface has access
template <class MESH> friend class VStructQuadSurfMesh;

public:
  //! Types that change depending on 32 or 64bits
  typedef SCIRun::index_type                 under_type;
  typedef SCIRun::index_type                 index_type;
  typedef SCIRun::size_type                  size_type;
  typedef SCIRun::mask_type                  mask_type;
  
  typedef LockingHandle<StructQuadSurfMesh<Basis> > handle_type;

  StructQuadSurfMesh();
  StructQuadSurfMesh(size_type x, size_type y);
  StructQuadSurfMesh(const StructQuadSurfMesh<Basis> &copy);
  virtual StructQuadSurfMesh *clone()
  { return new StructQuadSurfMesh<Basis>(*this); }
  virtual ~StructQuadSurfMesh() {}

  //! get the mesh statistics
  virtual BBox get_bounding_box() const;
  virtual void transform(const Transform &t);

  virtual bool get_dim(vector<size_type>&) const;
  virtual void set_dim(vector<size_type> dims) {
    ImageMesh<Basis>::ni_ = dims[0];
    ImageMesh<Basis>::nj_ = dims[1];

    points_.resize(dims[1], dims[0]);
    normals_.resize(dims[1], dims[0]);

    //! Create a new virtual interface for this copy
    //! all pointers have changed hence create a new
    //! virtual interface class
    ImageMesh<Basis>::vmesh_ = CreateVStructQuadSurfMesh(this); 
  }

  virtual int topology_geometry() const 
  {
    return (Mesh::STRUCTURED | Mesh::IRREGULAR);
  }

  //! Get the size of an elemnt (length, area, volume)
  double get_size(const typename ImageMesh<Basis>::Node::index_type &) const
  { return 0.0; }
  double get_size(typename ImageMesh<Basis>::Edge::index_type idx) const
  {
    typename ImageMesh<Basis>::Node::array_type arr;
    get_nodes(arr, idx);
    Point p0, p1;
    get_center(p0, arr[0]);
    get_center(p1, arr[1]);
    return (p1.asVector() - p0.asVector()).length();
  }
  double get_size(const typename ImageMesh<Basis>::Face::index_type &idx) const
  {
    //! Sum the sizes of the two triangles.
    const Point &p0 = points_(idx.j_ + 0, idx.i_ + 0);
    const Point &p1 = points_(idx.j_ + 0, idx.i_ + 1);
    const Point &p2 = points_(idx.j_ + 1, idx.i_ + 1);
    const Point &p3 = points_(idx.j_ + 1, idx.i_ + 0);
    const double a0 = Cross(p0 - p1, p2 - p0).length();
    const double a1 = Cross(p2 - p3, p0 - p2).length();
    return (a0 + a1) * 0.5;
  }

  double get_size(typename ImageMesh<Basis>::Cell::index_type) const
  { return 0.0; }
  double get_length(typename ImageMesh<Basis>::Edge::index_type idx) const
  { return get_size(idx); }
  double get_area(const typename ImageMesh<Basis>::Face::index_type &idx) const
  { return get_size(idx); }
  double get_volume(typename ImageMesh<Basis>::Cell::index_type idx) const
  { return get_size(idx); }

  void get_normal(Vector &,
                  const typename ImageMesh<Basis>::Node::index_type &) const;

  void get_normal(Vector &result, vector<double> &coords,
                  typename ImageMesh<Basis>::Elem::index_type eidx,
                  unsigned int)
  {
  
/*
    if (this->basis_.polynomial_order() < 2) 
    {
      typename ImageMesh<Basis>::Node::array_type arr(3);
      get_nodes(arr, eidx);

      const double c0_0 = fabs(coords[0]);
      const double c1_0 = fabs(coords[1]);
      const double c0_1 = fabs(coords[0] - 1.0L);
      const double c1_1 = fabs(coords[1] - 1.0L);

      if (c0_0 < 1e-7 && c1_0 < 1e-7) {
        //! arr[0]
        result = normals_(arr[0].i_, arr[0].j_);
        return;
      } else if (c0_1 < 1e-7 && c1_0 < 1e-7) {
        //! arr[1]
        result = normals_(arr[1].i_, arr[1].j_);
        return;
      } else if (c0_1 < 1e-7 && c1_1 < 1e-7) {
        //! arr[2]
        result = normals_(arr[2].i_, arr[2].j_);
        return;
      } else if (c0_0 < 1e-7 && c1_1 < 1e-7) {
        //! arr[3]
        result = normals_(arr[3].i_, arr[3].j_);
        return;
      }
    }
*/
    ElemData ed(*this, eidx);
    vector<Point> Jv;
    this->basis_.derivate(coords, ed, Jv);
    result = Cross(Jv[0].asVector(), Jv[1].asVector());
    result.normalize();
  }
  //! get the center point (in object space) of an element
  void get_center(Point &,
                  const typename ImageMesh<Basis>::Node::index_type &) const;
  void get_center(Point &,
                  typename ImageMesh<Basis>::Edge::index_type) const;
  void get_center(Point &,
                  const typename ImageMesh<Basis>::Face::index_type &) const;
  void get_center(Point &,
                  typename ImageMesh<Basis>::Cell::index_type) const {}

  bool locate(typename ImageMesh<Basis>::Node::index_type &,
              const Point &) const;
  bool locate(typename ImageMesh<Basis>::Edge::index_type &,
              const Point &) const;
  bool locate(typename ImageMesh<Basis>::Face::index_type &,
              const Point &) const;
  bool locate(typename ImageMesh<Basis>::Cell::index_type &,
              const Point &) const;

  int get_weights(const Point &p,
                  typename ImageMesh<Basis>::Node::array_type &l, double *w);
  int get_weights(const Point & ,
                  typename ImageMesh<Basis>::Edge::array_type & , double * )
  { ASSERTFAIL("StructQuadSurfMesh::get_weights for edges isn't supported"); }
  int get_weights(const Point &p,
                  typename ImageMesh<Basis>::Face::array_type &l, double *w);
  int get_weights(const Point & ,
                  typename ImageMesh<Basis>::Cell::array_type & , double * )
  { ASSERTFAIL("StructQuadSurfMesh::get_weights for cells isn't supported"); }


  bool inside3_p(typename ImageMesh<Basis>::Face::index_type i,
                 const Point &p) const;


  void get_point(Point &point,
                 const typename ImageMesh<Basis>::Node::index_type &index) const
  { get_center(point, index); }
  void set_point(const Point &point,
                 const typename ImageMesh<Basis>::Node::index_type &index);

  void get_random_point(Point &,
                        const typename ImageMesh<Basis>::Elem::index_type &,
                        FieldRNG &rng) const;

  class ElemData
  {
  public:

    ElemData(const StructQuadSurfMesh<Basis>& msh,
             const typename ImageMesh<Basis>::Elem::index_type ind) :
      mesh_(msh),
      index_(ind)
    {}

    //! the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const 
    {
      return (index_.i_ + mesh_.get_ni()*index_.j_);
    }
    inline
    index_type node1_index() const 
    {
      return (index_.i_+ 1 + mesh_.get_ni()*index_.j_);
    }
    inline
    index_type node2_index() const 
    {
      return (index_.i_ + 1 + mesh_.get_ni()*(index_.j_ + 1));

    }
    inline
    index_type node3_index() const 
    {
      return (index_.i_ + mesh_.get_ni()*(index_.j_ + 1));
    }

    //! the following designed to coordinate with ::get_edges
    inline
    index_type edge0_index() const 
    {
      return index_.i_ + index_.j_ * (mesh_.ni_- 1);
    }
    inline
    index_type edge1_index() const 
    {
      return index_.i_ + (index_.j_ + 1) * (mesh_.ni_ - 1);
    }
    inline
    index_type edge2_index() const 
    {
      return index_.i_    *(mesh_.nj_ - 1) + index_.j_ +
        ((mesh_.ni_ - 1) * mesh_.nj_);
     }
    inline
    index_type edge3_index() const 
    {
      return (index_.i_ + 1) * (mesh_.nj_ - 1) + index_.j_ +
        ((mesh_.ni_ - 1) * mesh_.nj_);
    }


    inline
    const Point &node0() const 
    {
      return mesh_.points_(index_.j_, index_.i_);
    }
    inline
    const Point &node1() const 
    {
      return mesh_.points_(index_.j_, index_.i_+1);
    }
    inline
    const Point &node2() const 
    {
      return mesh_.points_(index_.j_+1, index_.i_+1);
    }
    inline
    const Point &node3() const 
    {
      return mesh_.points_(index_.j_+1, index_.i_);
    }
    inline
    const Point &node4() const 
    {
      return mesh_.points_(index_.j_, index_.i_);
    }

  private:
    const StructQuadSurfMesh<Basis>                    &mesh_;
    const typename ImageMesh<Basis>::Elem::index_type  index_;
  };

  friend class ElemData;

  //! Generate the list of points that make up a sufficiently accurate
  //! piecewise linear approximation of an edge.
  void pwl_approx_edge(vector<vector<double> > &coords,
                       typename ImageMesh<Basis>::Elem::index_type ci,
                       unsigned which_edge,
                       unsigned div_per_unit) const
  {
    //! Needs to match unit_edges in Basis/QuadBilinearLgn.cc compare
    //! get_nodes order to the basis order
    int emap[] = {0, 2, 3, 1};
    this->basis_.approx_edge(emap[which_edge], div_per_unit, coords);
  }

  //! Get the local coordinates for a certain point within an element
  //! This function uses a couple of newton iterations to find the
  //! local ! coordinate of a point
  template<class VECTOR>
  bool get_coords(VECTOR &coords, const Point &p,
		  typename ImageMesh<Basis>::Elem::index_type idx) const
  {
    ElemData ed(*this, idx);
    return this->basis_.get_coords(coords, p, ed);
  }

  //! Find the location in the global coordinate system for a local
  //! coordinate ! This function is the opposite of get_coords.
  template<class VECTOR>
  void interpolate(Point &pt, const VECTOR &coords,
		   typename ImageMesh<Basis>::Elem::index_type idx) const
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
		typename ImageMesh<Basis>::Elem::index_type idx,
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
		      typename ImageMesh<Basis>::Elem::index_type idx) const
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
		typename ImageMesh<Basis>::Elem::index_type idx,
		double* J) const
  {
    StackVector<Point,2> Jv;
    ElemData ed(*this,idx);
    this->basis_.derivate(coords,ed,Jv);
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

  //! Get the inverse jacobian of the transformation. This one is
  //! needed to ! translate local gradients into global gradients. Hence
  //! it is crucial for ! calculating gradients of fields, or
  //! constructing finite elements.
  template<class VECTOR>                
  double inverse_jacobian(const VECTOR& coords,
			  typename ImageMesh<Basis>::Elem::index_type idx,
			  double* Ji) const
  {
    StackVector<Point,2> Jv;
    ElemData ed(*this,idx);
    this->basis_.derivate(coords,ed,Jv);
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
  double scaled_jacobian_metric(const INDEX idx) const
  {
    StackVector<Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;

    this->basis_.derivate(this->basis_.unit_center,ed,Jv);
    Jv.resize(3); 
    Vector v = Cross(Jv[0].asVector(),Jv[1].asVector()); v.normalize();
    Jv[2] = v.asPoint();
    double min_jacobian = ScaledDetMatrix3P(Jv);
    
    size_t num_vertices = this->basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      this->basis_.derivate(this->basis_.unit_vertices[j],ed,Jv);
      Jv.resize(3); 
      v = Cross(Jv[0].asVector(),Jv[1].asVector()); v.normalize();
      Jv[2] = v.asPoint();
      temp = ScaledDetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }
      
    return (min_jacobian);
  }


  template<class INDEX>
  double jacobian_metric(const INDEX idx) const
  {
    StackVector<Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;

    this->basis_.derivate(this->basis_.unit_center,ed,Jv);
    Jv.resize(3); 
    Vector v = Cross(Jv[0].asVector(),Jv[1].asVector()); v.normalize();
    Jv[2] = v.asPoint();
    double min_jacobian = DetMatrix3P(Jv);
    
    size_t num_vertices = this->basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      this->basis_.derivate(this->basis_.unit_vertices[j],ed,Jv);
      Jv.resize(3); 
      v = Cross(Jv[0].asVector(),Jv[1].asVector()); v.normalize();
      Jv[2] = v.asPoint();
      temp = DetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }
      
    return (min_jacobian);
  }


  //! Export this class using the old Pio system
  virtual void io(Piostream&);
  //! These IDs are created as soon as this class will be instantiated
  //! The first one is for Pio and the second for the virtual interface
  //! These are currently different as they serve different needs.  static PersistentTypeID type_idts;
  static PersistentTypeID type_idsqs;
  //! Core functionality for getting the name of a templated mesh class  
  static  const string type_name(int n = -1);
  
  virtual bool synchronize(mask_type sync);
  virtual bool unsynchronize(mask_type sync);
  
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
  static Persistent *maker() { return scinew StructQuadSurfMesh<Basis>(); }
  //! This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return scinew StructQuadSurfMesh<Basis>(); }
  //! This function returns a handle for the virtual interface.
  static MeshHandle structquadsurf_maker(size_type x ,size_type y) { return scinew StructQuadSurfMesh<Basis>(x,y); }

  Array2<Point>& get_points() { return (points_); }

  double get_epsilon() const
  { return (epsilon_); }
  
  
protected:

  void compute_epsilon();
  void compute_normals();

  //! compute_edge_neighbors is not defined anywhere... do don't
  //! declare it...  void compute_edge_neighbors(double err = 1.0e-8);

  const Point &point(const typename ImageMesh<Basis>::Node::index_type &idx)
  { return points_(idx.j_, idx.i_); }

  index_type next(index_type i) { return ((i%4)==3) ? (i-3) : (i+1); }
  index_type prev(index_type i) { return ((i%4)==0) ? (i+3) : (i-1); }

  Array2<Point>  points_;
  Array2<Vector> normals_; //! normalized per node
  Mutex          synchronize_lock_;
  mask_type      synchronized_;
  double         epsilon_;

  
};


template <class Basis>
PersistentTypeID
StructQuadSurfMesh<Basis>::type_idsqs(StructQuadSurfMesh<Basis>::type_name(-1),
                                   "Mesh", maker);


template <class Basis>
StructQuadSurfMesh<Basis>::StructQuadSurfMesh()
  : synchronize_lock_("StructQuadSurfMesh Normals Lock"),
    synchronized_(Mesh::ALL_ELEMENTS_E),
    epsilon_(0.0)
{
  //! Create a new virtual interface for this copy ! all pointers have
  //! changed hence create a new ! virtual interface class
  this->vmesh_ = CreateVStructQuadSurfMesh(this);  
}


template <class Basis>
StructQuadSurfMesh<Basis>::StructQuadSurfMesh(size_type x, size_type y)
  : ImageMesh<Basis>(x, y, Point(0.0, 0.0, 0.0), Point(1.0, 1.0, 1.0)),
    points_( y,x),
    normals_( y,x),
    synchronize_lock_("StructQuadSurfMesh Normals Lock"),
    synchronized_(Mesh::ALL_ELEMENTS_E),
    epsilon_(0.0)
{
  //! Create a new virtual interface for this copy ! all pointers have
  //! changed hence create a new ! virtual interface class
  this->vmesh_ = CreateVStructQuadSurfMesh(this);  
}


template <class Basis>
StructQuadSurfMesh<Basis>::StructQuadSurfMesh(const StructQuadSurfMesh &copy)
  : ImageMesh<Basis>(copy),
    synchronize_lock_("StructQuadSurfMesh Normals Lock"),
    synchronized_(copy.synchronized_),
    epsilon_(copy.epsilon_)
{
  StructQuadSurfMesh &lcopy = (StructQuadSurfMesh &)copy;

  points_.copy( copy.points_ );

  lcopy.synchronize_lock_.lock();
  normals_.copy(copy.normals_);
  synchronized_ &= ~Mesh::LOCATE_E;
  synchronized_ |= copy.synchronized_ & Mesh::LOCATE_E;
  lcopy.synchronize_lock_.unlock();

  //! Create a new virtual interface for this copy ! all pointers have
  //! changed hence create a new ! virtual interface class
  this->vmesh_ = CreateVStructQuadSurfMesh(this);  
}


template <class Basis>
bool
StructQuadSurfMesh<Basis>::get_dim(vector<size_type> &array) const
{
  array.resize(2);
  array.clear();

  array.push_back(this->ni_);
  array.push_back(this->nj_);

  return true;
}


template <class Basis>
BBox
StructQuadSurfMesh<Basis>::get_bounding_box() const
{
  BBox result;

  typename ImageMesh<Basis>::Node::iterator ni, nie;
  begin(ni);
  end(nie);
  while (ni != nie)
  {
    Point p;
    get_center(p, *ni);
    result.extend(p);
    ++ni;
  }
  return result;
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::transform(const Transform &t)
{
  typename ImageMesh<Basis>::Node::iterator i, ie;
  begin(i);
  end(ie);

  while (i != ie) 
  {
    points_((*i).j_,(*i).i_) = t.project(points_((*i).j_,(*i).i_));

    ++i;
  }
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::get_normal(Vector &result,
				      const typename ImageMesh<Basis>::Node::index_type &idx ) const
{
  result = normals_(idx.j_, idx.i_);
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::get_center(Point &result,
				      const typename ImageMesh<Basis>::Node::index_type &idx) const
{
  result = points_(idx.j_, idx.i_);
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::get_center(Point &result,
				      typename ImageMesh<Basis>::Edge::index_type idx) const
{
  typename ImageMesh<Basis>::Node::array_type arr;
  get_nodes(arr, idx);
  Point p1;
  get_center(result, arr[0]);
  get_center(p1, arr[1]);

  result.asVector() += p1.asVector();
  result.asVector() *= 0.5;
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::get_center(Point &p,
				      const typename ImageMesh<Basis>::Face::index_type &idx) const
{
  typename ImageMesh<Basis>::Node::array_type nodes;
  get_nodes(nodes, idx);
  ASSERT(nodes.size() == 4);
  typename ImageMesh<Basis>::Node::array_type::iterator nai = nodes.begin();
  get_point(p, *nai);
  ++nai;
  Point pp;
  while (nai != nodes.end())
  {
    get_point(pp, *nai);
    p.asVector() += pp.asVector();
    ++nai;
  }
  p.asVector() *= (1.0 / 4.0);
}


template <class Basis>
bool
StructQuadSurfMesh<Basis>::locate(typename ImageMesh<Basis>::Node::index_type &node,
				  const Point &p) const
{
  node.mesh_ = this;
  typename ImageMesh<Basis>::Face::index_type fi;
  if (locate(fi, p)) { //! first try the fast way.
    typename ImageMesh<Basis>::Node::array_type nodes;
    get_nodes(nodes, fi);

    double dmin = (p-points_(nodes[0].j_, nodes[0].i_)).length2();
    node = nodes[0];
    for (size_t i = 1; i < nodes.size(); i++)
    {
      const double d = (p-points_(nodes[i].j_, nodes[i].i_)).length2();
      if (d < dmin)
      {
        dmin = d;
        node = nodes[i];
      }
    }
    return true;
  }
  else
  {  //! do exhaustive search.
    typename ImageMesh<Basis>::Node::iterator ni, nie;
    begin(ni);
    end(nie);
    if (ni == nie) { return false; }

    double min_dist = (p-points_((*ni).j_, (*ni).i_)).length2();
    node = *ni;
    ++ni;

    while (ni != nie)
    {
      const double dist = (p-points_((*ni).j_, (*ni).i_)).length2();
      if (dist < min_dist)
      {
        node = *ni;
        min_dist = dist;
      }
      ++ni;
    }
    return true;
  }
}


template <class Basis>
bool
StructQuadSurfMesh<Basis>::locate(typename ImageMesh<Basis>::Edge::index_type &loc,
				  const Point &p) const
{
  typename ImageMesh<Basis>::Edge::iterator bi, ei;
  typename ImageMesh<Basis>::Node::array_type nodes;
  begin(bi);
  end(ei);
  loc = 0;

  bool found = false;
  double mindist = 0.0;
  while (bi != ei)
  {
    get_nodes(nodes,*bi);

    Point p0, p1;
    get_center(p0, nodes[0]);
    get_center(p1, nodes[1]);

    const double dist = distance_to_line2(p, p0, p1,epsilon_);
    if (!found || dist < mindist)
    {
      loc = *bi;
      mindist = dist;
      found = true;
    }
    ++bi;
  }
  return found;
}


template <class Basis>
int
StructQuadSurfMesh<Basis>::get_weights(const Point &p,
				       typename ImageMesh<Basis>::Face::array_type &l,
                                       double *w)
{
  typename ImageMesh<Basis>::Face::index_type idx;
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
StructQuadSurfMesh<Basis>::get_weights(const Point &p,
				       typename ImageMesh<Basis>::Node::array_type &l,
                                       double *w)
{
  typename ImageMesh<Basis>::Face::index_type idx;
  if (locate(idx, p))
  {
    get_nodes(l,idx);
    vector<double> coords(2);
    if (get_coords(coords, p, idx)) {
      this->basis_.get_weights(coords, w);
      return this->basis_.dofs();
    }
  }
  return 0;
}


template <class Basis>
bool
StructQuadSurfMesh<Basis>::inside3_p(typename ImageMesh<Basis>::Face::index_type i,
				     const Point &p) const
{
  typename ImageMesh<Basis>::Node::array_type nodes;
  get_nodes(nodes, i);

  size_type n = nodes.size();

  Point * pts = new Point[n];

  for (index_type i = 0; i < n; i++) 
  {
    get_center(pts[i], nodes[i]);
  }

  for (index_type i = 0; i < n; i+=2) 
  {
    Point p0 = pts[(i+0)%n];
    Point p1 = pts[(i+1)%n];
    Point p2 = pts[(i+2)%n];

    Vector v01(p0-p1);
    Vector v02(p0-p2);
    Vector v0(p0-p);
    Vector v1(p1-p);
    Vector v2(p2-p);
    const double a = Cross(v01, v02).length(); //! area of the whole triangle (2x)
    const double a0 = Cross(v1, v2).length();  //! area opposite p0
    const double a1 = Cross(v2, v0).length();  //! area opposite p1
    const double a2 = Cross(v0, v1).length();  //! area opposite p2
    const double s = a0+a1+a2;

    //! If the area of any of the sub triangles is very small then the point
    //! is on the edge of the subtriangle.
    //! TODO : How small is small ???
//!     if( a0 < MIN_ELEMENT_VAL ||
//!      a1 < MIN_ELEMENT_VAL ||
//!      a2 < MIN_ELEMENT_VAL )
//!       return true;

    //! For the point to be inside a CONVEX quad it must be inside one
    //! of the four triangles that can be formed by using three of the
    //! quad vertices and the point in question.
    if( fabs(s - a) < ImageMesh<Basis>::MIN_ELEMENT_VAL && a >
        ImageMesh<Basis>::MIN_ELEMENT_VAL ) {
      delete [] pts;
      return true;
    }
  }
  delete [] pts;
  return false;
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::get_random_point(Point &p,
					    const typename ImageMesh<Basis>::Elem::index_type &ei,
                                            FieldRNG &rng) const
{
  const Point &a0 = points_(ei.j_ + 0, ei.i_ + 0);
  const Point &a1 = points_(ei.j_ + 0, ei.i_ + 1);
  const Point &a2 = points_(ei.j_ + 1, ei.i_ + 1);
  const Point &a3 = points_(ei.j_ + 1, ei.i_ + 0);

  const double aarea = Cross(a1 - a0, a2 - a0).length();
  const double barea = Cross(a3 - a0, a2 - a0).length();

  //! Fold the quad sample into a triangle.
  double u = rng();
  double v = rng();
  if (u + v > 1.0) { u = 1.0 - u; v = 1.0 - v; }
  
  if (rng() * (aarea + barea) < aarea)
  {
    p = a0+((a1-a0)*u)+((a2-a0)*v);
  }
  else
  {
    p = a0+((a3-a0)*u)+((a2-a0)*v);
  }
}


template <class Basis>
bool
StructQuadSurfMesh<Basis>::locate( typename ImageMesh<Basis>::Face::index_type &face,
				   const Point &p) const
{
  if (this->basis_.polynomial_order() > 1) return elem_locate(face, *this, p);
  typename ImageMesh<Basis>::Face::iterator bi, ei;
  begin(bi);
  end(ei);

  while (bi != ei) 
  {
    if( inside3_p( *bi, p ) ) 
    {
      face = *bi;
      return true;
    }

    ++bi;
  }
  return false;
}


template <class Basis>
bool
StructQuadSurfMesh<Basis>::
locate(typename ImageMesh<Basis>::Cell::index_type &loc,
       const Point &) const
{
  loc = 0;
  return false;
}


template <class Basis>
void
StructQuadSurfMesh<Basis>::set_point(const Point &point,
				     const typename ImageMesh<Basis>::Node::index_type &index)
{
  points_(index.j_, index.i_) = point;
}


template <class Basis>
bool
StructQuadSurfMesh<Basis>::synchronize(mask_type sync)
{
  synchronize_lock_.lock();
  if (sync & Mesh::NORMALS_E && !(synchronized_ & Mesh::NORMALS_E))
  {
    compute_normals();
    synchronized_ |= Mesh::NORMALS_E;
  }
  
  if (sync & (Mesh::EPSILON_E|Mesh::LOCATE_E) &&
      !(synchronized_ & (Mesh::EPSILON_E|Mesh::LOCATE_E)))
  {
    compute_epsilon();
    synchronized_ |= (Mesh::EPSILON_E|Mesh::LOCATE_E);
  }
  
  synchronize_lock_.unlock();
  return(true);
}

template <class Basis>
bool
StructQuadSurfMesh<Basis>::unsynchronize(mask_type sync)
{
  return (true);
}

template <class Basis>
void
StructQuadSurfMesh<Basis>::compute_epsilon()
{
  epsilon_ = get_bounding_box().diagonal().length();
}

template <class Basis>
void
StructQuadSurfMesh<Basis>::compute_normals()
{
  normals_.resize(points_.dim1(), points_.dim2()); //! 1 per node

  //! build table of faces that touch each node
  Array2< vector<typename ImageMesh<Basis>::Face::index_type> >
    node_in_faces(points_.dim1(), points_.dim2());

  //! face normals (not normalized) so that magnitude is also the area.
  Array2<Vector> face_normals((points_.dim1()-1),(points_.dim2()-1));

  //! Computing normal per face.
  typename ImageMesh<Basis>::Node::array_type nodes(4);
  typename ImageMesh<Basis>::Face::iterator iter, iter_end;
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

    //! build table of faces that touch each node
    node_in_faces(nodes[0].j_,nodes[0].i_).push_back(*iter);
    node_in_faces(nodes[1].j_,nodes[1].i_).push_back(*iter);
    node_in_faces(nodes[2].j_,nodes[2].i_).push_back(*iter);
    node_in_faces(nodes[3].j_,nodes[3].i_).push_back(*iter);

    Vector v0 = p1 - p0;
    Vector v1 = p2 - p1;
    Vector n = Cross(v0, v1);
    face_normals((*iter).j_, (*iter).i_) = n;

    ++iter;
  }

  //! Averaging the normals.
  typename ImageMesh<Basis>::Node::iterator nif_iter, nif_iter_end;
  begin( nif_iter );
  end( nif_iter_end );

  while (nif_iter != nif_iter_end)
  {
    vector<typename ImageMesh<Basis>::Face::index_type> v = 
      node_in_faces((*nif_iter).j_, (*nif_iter).i_);
    typename vector<typename ImageMesh<Basis>::Face::index_type>::const_iterator
          fiter = v.begin();
    Vector ave(0.L,0.L,0.L);
    while(fiter != v.end())
    {
      ave += face_normals((*fiter).j_,(*fiter).i_);
      ++fiter;
    }
    ave.safe_normalize();
    normals_((*nif_iter).j_, (*nif_iter).i_) = ave;
    ++nif_iter;
  }
}


#define STRUCT_QUAD_SURF_MESH_VERSION 3

template <class Basis>
void
StructQuadSurfMesh<Basis>::io(Piostream& stream)
{
  int version =
    stream.begin_class(type_name(-1), STRUCT_QUAD_SURF_MESH_VERSION);
  ImageMesh<Basis>::io(stream);

  if (version ==2)
  {
    //! The dimensions of this array were swapped
    Array2<Point> tpoints;
    Pio(stream, tpoints);
    
    size_t dim1 = tpoints.dim1();
    size_t dim2 = tpoints.dim2();
    
    points_.resize(dim2,dim1);
    for (size_t i=0; i<dim1; i++)
      for (size_t j=0; j<dim2; j++)
           points_(j,i) = tpoints(i,j);
  }
  else
  {
    Pio(stream, points_);    
  }
  
  stream.end_class();

  if (stream.reading())
    this->vmesh_ = CreateVStructQuadSurfMesh(this);
}


template <class Basis>
const string
StructQuadSurfMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const string name = type_name(0) + FTNS + type_name(1) + FTNE;
    return name;
  }
  else if (n == 0)
  {
    static const string nm("StructQuadSurfMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
const TypeDescription*
get_type_description(StructQuadSurfMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = SCIRun::get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = scinew TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = scinew TypeDescription("StructQuadSurfMesh", subs,
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructQuadSurfMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((StructQuadSurfMesh<Basis> *)0);
}


template <class Basis>
const TypeDescription*
StructQuadSurfMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructQuadSurfMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Node",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructQuadSurfMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructQuadSurfMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Edge",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructQuadSurfMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructQuadSurfMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Face",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructQuadSurfMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructQuadSurfMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Cell",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}

} //! namespace SCIRun

#endif //! SCI_project_StructQuadSurfMesh_h
