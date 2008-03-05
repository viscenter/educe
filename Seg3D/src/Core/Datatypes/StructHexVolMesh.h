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
  geometry.

  The grid may have any shape but can not be overlapping or self-intersecting.

  The topology of a structured grid is represented using 2D, or 3D vector with
  the points being stored in an index based array. The faces (quadrilaterals)
  and cells (Hexahedron) are implicitly define based based upon their indexing.

  Structured grids are typically used in finite difference analysis.

  For more information on datatypes see Schroeder, Martin, and Lorensen,
  "The Visualization Toolkit", Prentice Hall, 1998.
*/


#ifndef CORE_DATATYPES_STRUCTHEXVOLMESH_H
#define CORE_DATATYPES_STRUCTHEXVOLMESH_H 1

//! Include what kind of support we want to have
//! Need to fix this and couple it to sci-defs
#include <Core/Datatypes/MeshSupport.h>

#include <Core/Geometry/Point.h>
#include <slivr/Plane.h>
#include <Core/Geometry/CompGeom.h>

#include <Core/Containers/Array3.h>

#include <Core/Datatypes/LatVolMesh.h>
#include <Core/Datatypes/SearchGrid.h>

//! Incude needed for Windows: declares SCISHARE
#include <Core/Datatypes/share.h>

namespace SCIRun {
using SLIVR::Plane;
using std::string;
using std::vector;


//! Declarations for virtual interface


//! Functions for creating the virtual interface
//! Declare the functions that instantiate the virtual interface
template <class Basis>
class StructHexVolMesh;

//! make sure any other mesh other than the preinstantiate ones
//! returns no virtual interface. Altering this behaviour will allow
//! for dynamically compiling the interfae if needed.
template<class MESH>
VMesh* CreateVStructHexVolMesh(MESH* mesh) { return (0); }

//! These declarations are needed for a combined dynamic compilation as
//! as well as virtual functions solution.
//! Declare that these can be found in a library that is already
//! precompiled. So dynamic compilation will not instantiate them again.

#if (SCIRUN_STRUCTHEXVOL_SUPPORT > 0)

SCISHARE VMesh* CreateVStructHexVolMesh(StructHexVolMesh<HexTrilinearLgn<Point> >* mesh);

#endif


template <class Basis>
class StructHexVolMesh : public LatVolMesh<Basis>
{

template <class MESH>
friend class VStructHexVolMesh;

public:
  //! Types that change depending on 32 or 64bits
  typedef SCIRun::index_type                 under_type;
  typedef SCIRun::index_type                 index_type;
  typedef SCIRun::size_type                  size_type;
  typedef SCIRun::mask_type                  mask_type;

  typedef LockingHandle<StructHexVolMesh<Basis> > handle_type;

  StructHexVolMesh();
  StructHexVolMesh(size_type i, size_type j, size_type k);
  StructHexVolMesh(const StructHexVolMesh<Basis> &copy);
  virtual StructHexVolMesh *clone() { return new StructHexVolMesh<Basis>(*this); }
  virtual ~StructHexVolMesh() {}

  class ElemData
  {
  public:
    typedef typename StructHexVolMesh<Basis>::index_type  index_type;

    ElemData(const StructHexVolMesh<Basis>& msh,
             const typename LatVolMesh<Basis>::Cell::index_type ind) :
      mesh_(msh),
      index_(ind)
    {}

    //! the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const {
      return (index_.i_ + mesh_.get_ni()*index_.j_ +
              mesh_.get_ni()*mesh_.get_nj()*index_.k_);
    }
    inline
    index_type node1_index() const {
      return (index_.i_+ 1 + mesh_.get_ni()*index_.j_ +
              mesh_.get_ni()*mesh_.get_nj()*index_.k_);
    }
    inline
    index_type node2_index() const {
      return (index_.i_ + 1 + mesh_.get_ni()*(index_.j_ + 1) +
              mesh_.get_ni()*mesh_.get_nj()*index_.k_);

    }
    inline
    index_type node3_index() const {
      return (index_.i_ + mesh_.get_ni()*(index_.j_ + 1) +
              mesh_.get_ni()*mesh_.get_nj()*index_.k_);
    }
    inline
    index_type node4_index() const {
      return (index_.i_ + mesh_.get_ni()*index_.j_ +
              mesh_.get_ni()*mesh_.get_nj()*(index_.k_ + 1));
    }
    inline
    index_type node5_index() const {
      return (index_.i_ + 1 + mesh_.get_ni()*index_.j_ +
              mesh_.get_ni()*mesh_.get_nj()*(index_.k_ + 1));
    }

    inline
    index_type node6_index() const {
      return (index_.i_ + 1 + mesh_.get_ni()*(index_.j_ + 1) +
              mesh_.get_ni()*mesh_.get_nj()*(index_.k_ + 1));
    }
    inline
    index_type node7_index() const {
      return (index_.i_ + mesh_.get_ni()*(index_.j_ + 1) +
              mesh_.get_ni()*mesh_.get_nj()*(index_.k_ + 1));
    }

    inline
    const Point &node0() const {
      return mesh_.points_(index_.k_, index_.j_, index_.i_);
    }
    inline
    const Point &node1() const {
      return mesh_.points_(index_.k_, index_.j_, index_.i_+1);
    }
    inline
    const Point &node2() const {
      return mesh_.points_(index_.k_, index_.j_+1, index_.i_+1);
    }
    inline
    const Point &node3() const {
      return mesh_.points_(index_.k_, index_.j_+1, index_.i_);
    }
    inline
    const Point &node4() const {
      return mesh_.points_(index_.k_+1, index_.j_, index_.i_);
    }
    inline
    const Point &node5() const {
      return mesh_.points_(index_.k_+1, index_.j_, index_.i_+1);
    }
    inline
    const Point &node6() const {
      return mesh_.points_(index_.k_+1, index_.j_+1, index_.i_+1);
    }
    inline
    const Point &node7() const {
      return mesh_.points_(index_.k_+1, index_.j_+1, index_.i_);
    }

  private:
    const StructHexVolMesh<Basis>                       &mesh_;
    const typename LatVolMesh<Basis>::Cell::index_type  index_;
    
  };

  friend class ElemData;

  //! get the mesh statistics
  virtual BBox get_bounding_box() const;
  virtual void transform(const Transform &t);

  virtual bool get_dim(vector<size_type>&) const;
  virtual void set_dim(vector<size_type> dims) {
    LatVolMesh<Basis>::ni_ = dims[0];
    LatVolMesh<Basis>::nj_ = dims[1];
    LatVolMesh<Basis>::nk_ = dims[2];

    points_.resize(dims[2], dims[1], dims[0]);

    //! Create a new virtual interface for this copy
    //! all pointers have changed hence create a new
    //! virtual interface class
    LatVolMesh<Basis>::vmesh_ = CreateVStructHexVolMesh(this); 
  }

  virtual int topology_geometry() const
  {
    return (Mesh::STRUCTURED | Mesh::IRREGULAR);
  }

  //! get the center point (in object space) of an element
  void get_center(Point &,
                  const typename LatVolMesh<Basis>::Node::index_type &) const;
  void get_center(Point &, typename LatVolMesh<Basis>::Edge::index_type) const;
  void get_center(Point &, typename LatVolMesh<Basis>::Face::index_type) const;
  void get_center(Point &,
                  const typename LatVolMesh<Basis>::Cell::index_type &) const;

  double get_size(const typename LatVolMesh<Basis>::Node::index_type &idx) const;
  double get_size(typename LatVolMesh<Basis>::Edge::index_type idx) const;
  double get_size(typename LatVolMesh<Basis>::Face::index_type idx) const;
  double get_size(
               const typename LatVolMesh<Basis>::Cell::index_type &idx) const;
  double get_length(typename LatVolMesh<Basis>::Edge::index_type idx) const
  { return get_size(idx); };
  double get_area(typename LatVolMesh<Basis>::Face::index_type idx) const
  { return get_size(idx); };
  double get_volume(const
                    typename LatVolMesh<Basis>::Cell::index_type &i) const
  { return get_size(i); };

  bool locate(typename LatVolMesh<Basis>::Node::index_type &,
              const Point &) const;
  bool locate(typename LatVolMesh<Basis>::Edge::index_type &,
              const Point &) const
  { return false; }
  bool locate(typename LatVolMesh<Basis>::Face::index_type &,
              const Point &) const
  { return false; }
  bool locate(typename LatVolMesh<Basis>::Cell::index_type &, 
              const Point &) const;


  int get_weights(const Point &,
                  typename LatVolMesh<Basis>::Node::array_type &, double *);
  int get_weights(const Point &,
                  typename LatVolMesh<Basis>::Edge::array_type &, double *)
  { ASSERTFAIL("StructHexVolMesh::get_weights for edges isn't supported"); }
  int get_weights(const Point &,
                  typename LatVolMesh<Basis>::Face::array_type &, double *)
  { ASSERTFAIL("StructHexVolMesh::get_weights for faces isn't supported"); }
  int get_weights(const Point &,
                  typename LatVolMesh<Basis>::Cell::array_type &, double *);

  void get_point(Point &point,
              const typename LatVolMesh<Basis>::Node::index_type &index) const
  { get_center(point, index); }
  void set_point(const Point &point,
                 const typename LatVolMesh<Basis>::Node::index_type &index);


  void get_random_point(Point &p,
                        const typename LatVolMesh<Basis>::Elem::index_type & i,
                        FieldRNG &rng) const;

  //! Get the local coordinates for a certain point within an element
  //! This function uses a couple of newton iterations to find the local
  //! coordinate of a point
  template<class VECTOR>
  bool get_coords(VECTOR &coords, const Point &p,
		  typename LatVolMesh<Basis>::Elem::index_type idx) const
  {
    ElemData ed(*this, idx);
    return this->basis_.get_coords(coords, p, ed);
  }

  //! Find the location in the global coordinate system for a local
  //! coordinate ! This function is the opposite of get_coords.
  template<class VECTOR>
  void interpolate(Point &pt, const VECTOR &coords,
		   typename LatVolMesh<Basis>::Elem::index_type idx) const
  {
    ElemData ed(*this, idx);
    pt = this->basis_.interpolate(coords, ed);
  }

  //! Interpolate the derivate of the function, This infact will
  //! return the jacobian of the local to global coordinate
  //! transformation. This function is mainly intended for the non
  //! linear elements
  template<class VECTOR1, class VECTOR2>  
  void derivate(const VECTOR1 &coords,
		typename LatVolMesh<Basis>::Elem::index_type idx,
		VECTOR2 &J) const
  {
    ElemData ed(*this, idx);
    this->basis_.derivate(coords, ed, J);
  }
  
  //! Get the determinant of the jacobian, which is the local volume
  //! of an element and is intended to help with the integration of
  //! functions over an element.
  template<class VECTOR>  
  double det_jacobian(const VECTOR& coords,
		      typename LatVolMesh<Basis>::Elem::index_type idx) const
  {
    double J[9];
    jacobian(coords,idx,J);
    return (DetMatrix3x3(J));
  }

  //! Get the jacobian of the transformation. In case one wants the
  //! non inverted version of this matrix. This is currentl here for
  //! completeness of the interface
  template<class VECTOR>
  void jacobian(const VECTOR& coords,
		typename LatVolMesh<Basis>::Elem::index_type idx,
		double* J) const
  {
    StackVector<Point,3> Jv;
    ElemData ed(*this,idx);
    this->basis_.derivate(coords,ed,Jv);
    J[0] = Jv[0].x();
    J[1] = Jv[0].y();
    J[2] = Jv[0].z();
    J[3] = Jv[1].x();
    J[4] = Jv[1].y();
    J[5] = Jv[1].z();
    J[6] = Jv[2].x();
    J[7] = Jv[2].y();
    J[8] = Jv[2].z();
  }

  //! Get the inverse jacobian of the transformation. This one is
  //! needed to translate local gradients into global gradients. Hence
  //! it is crucial for calculating gradients of fields, or
  //! constructing finite elements.
  template<class VECTOR>                 
  double inverse_jacobian(const VECTOR& coords,
			  typename LatVolMesh<Basis>::Elem::index_type idx,
			  double* Ji) const
  {
    StackVector<Point,3> Jv;
    ElemData ed(*this,idx);
    this->basis_.derivate(coords,ed,Jv);
    double J[9];
    J[0] = Jv[0].x();
    J[1] = Jv[0].y();
    J[2] = Jv[0].z();
    J[3] = Jv[1].x();
    J[4] = Jv[1].y();
    J[5] = Jv[1].z();
    J[6] = Jv[2].x();
    J[7] = Jv[2].y();
    J[8] = Jv[2].z();
    
    return (InverseMatrix3x3(J,Ji));
  }  

  double scaled_jacobian_metric(typename LatVolMesh<Basis>::Elem::index_type idx) const
  {
    StackVector<Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;
    this->basis_.derivate(this->basis_.unit_center,ed,Jv);
    double min_jacobian = ScaledDetMatrix3P(Jv);
    
    size_t num_vertices = this->basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      this->basis_.derivate(this->basis_.unit_vertices[j],ed,Jv);
      temp = ScaledDetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }
      
    return (min_jacobian);
  }

  double jacobian_metric(typename LatVolMesh<Basis>::Elem::index_type idx) const
  {
    StackVector<Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;
    this->basis_.derivate(this->basis_.unit_center,ed,Jv);
    double min_jacobian = DetMatrix3P(Jv);
    
    size_t num_vertices = this->basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      this->basis_.derivate(this->basis_.unit_vertices[j],ed,Jv);
      temp = DetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }
      
    return (min_jacobian);
  }


  double get_epsilon() const
  { return (epsilon_); }

  virtual bool synchronize(mask_type);
  virtual bool unsynchronize(mask_type);
  
  //! Export this class using the old Pio system
  virtual void io(Piostream&);
  static PersistentTypeID structhexvol_typeid;
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
  { return cell_type_description(); }

  //! This function returns a maker for Pio.
  static Persistent *maker() { return scinew StructHexVolMesh<Basis>(); }
  //! This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker()  { return scinew StructHexVolMesh<Basis>(); }
  //! This function returns a handle for the virtual interface.
  static MeshHandle structhexvol_maker(size_type x,
				       size_type y,
				       size_type z)
  {
    return scinew StructHexVolMesh<Basis>(x,y,z);
  }

  Array3<Point>& get_points() { return (points_); }

private:
  void compute_grid();
  void compute_epsilon();

  bool inside8_p(typename LatVolMesh<Basis>::Cell::index_type idx,
                 const Point &p) const;
  double polygon_area(const typename LatVolMesh<Basis>::Node::array_type &,
                      const Vector) const;
  double pyramid_volume(const typename LatVolMesh<Basis>::Node::array_type &,
                        const Point &)const;
  const Point &point(
                const typename LatVolMesh<Basis>::Node::index_type &idx) const
  { return points_(idx.k_, idx.j_, idx.i_); }


  Array3<Point> points_;

  LockingHandle<SearchGrid>           grid_;
  Mutex                               synchronize_lock_;
  mask_type                           synchronized_;
  double                              epsilon_;
  double                              epsilon3_; //! for volumertic comparison
};


template <class Basis>
PersistentTypeID
StructHexVolMesh<Basis>::structhexvol_typeid(StructHexVolMesh<Basis>::type_name(-1),
                                 "Mesh", maker);

template <class Basis>
StructHexVolMesh<Basis>::StructHexVolMesh():
  grid_(0),
  synchronize_lock_("Synchronize lock"),
  synchronized_(Mesh::ALL_ELEMENTS_E),
  epsilon_(0.0),
  epsilon3_(0.0)
{
    //! Create a new virtual interface for this copy
    //! all pointers have changed hence create a new
    //! virtual interface class
    this->vmesh_ = CreateVStructHexVolMesh(this);     
}


template <class Basis>
StructHexVolMesh<Basis>::StructHexVolMesh(size_type i,
                                          size_type j,
                                          size_type k) :
  LatVolMesh<Basis>(i, j, k, Point(0.0, 0.0, 0.0), Point(1.0, 1.0, 1.0)),
  points_(k, j, i),
  grid_(0),
  synchronize_lock_("Synchronize lock"),
  synchronized_(Mesh::ALL_ELEMENTS_E),
  epsilon_(0.0),
  epsilon3_(0.0)
{
    //! Create a new virtual interface for this copy
    //! all pointers have changed hence create a new
    //! virtual interface class
    this->vmesh_ = CreateVStructHexVolMesh(this);   
}


template <class Basis>
StructHexVolMesh<Basis>::StructHexVolMesh(const StructHexVolMesh<Basis> &copy):
  LatVolMesh<Basis>(copy),
  grid_(0),
  synchronize_lock_("Synchronize lock"),
  synchronized_(Mesh::ALL_ELEMENTS_E),
  epsilon_(copy.epsilon_),
  epsilon3_(copy.epsilon3_)
{
  points_.copy( copy.points_ );

  StructHexVolMesh &lcopy = (StructHexVolMesh &)copy;
  lcopy.synchronize_lock_.lock();

  synchronized_ &= ~Mesh::LOCATE_E;
  if (copy.grid_.get_rep())
  {
    grid_ = scinew SearchGrid(*(copy.grid_.get_rep()));
  }
  
  synchronized_ |= copy.synchronized_ & Mesh::LOCATE_E;
  synchronized_ |= copy.synchronized_ & Mesh::EPSILON_E;


  lcopy.synchronize_lock_.unlock();
  
  //! Create a new virtual interface for this copy
  //! all pointers have changed hence create a new
  //! virtual interface class
  this->vmesh_ = CreateVStructHexVolMesh(this); 
}


template <class Basis>
bool
StructHexVolMesh<Basis>::get_dim(vector<size_type> &array) const
{
  array.resize(3);
  array.clear();

  array.push_back(this->ni_);
  array.push_back(this->nj_);
  array.push_back(this->nk_);

  return true;
}


template <class Basis>
BBox
StructHexVolMesh<Basis>::get_bounding_box() const
{
  BBox result;

  typename LatVolMesh<Basis>::Node::iterator ni, nie;
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
StructHexVolMesh<Basis>::transform(const Transform &t)
{
  typename LatVolMesh<Basis>::Node::iterator i, ie;
  begin(i);
  end(ie);

  while (i != ie)
  {
    points_((*i).k_,(*i).j_,(*i).i_) =
      t.project(points_((*i).k_,(*i).j_,(*i).i_));

    ++i;
  }

  synchronize_lock_.lock();
  if (grid_.get_rep()) { grid_->transform(t); }
  synchronize_lock_.unlock();

}


template <class Basis>
void
StructHexVolMesh<Basis>::get_center(Point &result,
				    const typename LatVolMesh<Basis>::Node::index_type &idx) const
{
  result = points_(idx.k_, idx.j_, idx.i_);
}


template <class Basis>
void
StructHexVolMesh<Basis>::get_center(Point &result,
				    typename LatVolMesh<Basis>::Edge::index_type idx) const
{
  typename LatVolMesh<Basis>::Node::array_type arr;
  get_nodes(arr, idx);
  Point p1;
  get_center(result, arr[0]);
  get_center(p1, arr[1]);

  result.asVector() += p1.asVector();
  result.asVector() *= 0.5;
}


template <class Basis>
void
StructHexVolMesh<Basis>::get_center(Point &result,
				    typename LatVolMesh<Basis>::Face::index_type idx) const
{
  typename LatVolMesh<Basis>::Node::array_type nodes;
  get_nodes(nodes, idx);
  ASSERT(nodes.size() == 4);
  typename LatVolMesh<Basis>::Node::array_type::iterator nai = nodes.begin();
  get_point(result, *nai);
  ++nai;
  Point pp;
  while (nai != nodes.end())
  {
    get_point(pp, *nai);
    result.asVector() += pp.asVector();
    ++nai;
  }
  result.asVector() *= (1.0 / 4.0);
}


template <class Basis>
void
StructHexVolMesh<Basis>::get_center(Point &result,
				    const typename LatVolMesh<Basis>::Cell::index_type &idx) const
{
  typename LatVolMesh<Basis>::Node::array_type nodes;
  get_nodes(nodes, idx);
  ASSERT(nodes.size() == 8);
  typename LatVolMesh<Basis>::Node::array_type::iterator nai = nodes.begin();
  get_point(result, *nai);
  ++nai;
  Point pp;
  while (nai != nodes.end())
  {
    get_point(pp, *nai);
    result.asVector() += pp.asVector();
    ++nai;
  }
  result.asVector() *= (1.0 / 8.0);
}


template <class Basis>
bool
StructHexVolMesh<Basis>::inside8_p( typename LatVolMesh<Basis>::Cell::index_type idx,
				    const Point &p) const
{
  static const int table[6][3][3] =
  {{{0, 0, 0},
    {0, 1, 0},
    {0, 0, 1}},

   {{0, 0, 0},
    {1, 0, 0},
    {0, 1, 0}},

   {{0, 0, 0},
    {0, 0, 1},
    {1, 0, 0}},

   {{1, 1, 1},
    {1, 1, 0},
    {1, 0, 1}},

   {{1, 1, 1},
    {1, 0, 1},
    {0, 1, 1}},

   {{1, 1, 1},
    {0, 1, 1},
    {1, 1, 0}}};

  Point center;
  get_center(center, idx);

  for (int i = 0; i < 6; i++)
  {
    typename LatVolMesh<Basis>::Node::index_type n0(this,
						    idx.i_ + table[i][0][0],
						    idx.j_ + table[i][0][1],
						    idx.k_ + table[i][0][2]);
    typename LatVolMesh<Basis>::Node::index_type n1(this,
						    idx.i_ + table[i][1][0],
						    idx.j_ + table[i][1][1],
						    idx.k_ + table[i][1][2]);
    typename LatVolMesh<Basis>::Node::index_type n2(this,
						    idx.i_ + table[i][2][0],
						    idx.j_ + table[i][2][1],
						    idx.k_ + table[i][2][2]);

    Point p0, p1, p2;
    get_center(p0, n0);
    get_center(p1, n1);
    get_center(p2, n2);

    const Vector v0(p1 - p0), v1(p2 - p0);
    const Vector normal = Cross(v0, v1);
    const Vector off0(p - p0);
    const Vector off1(center - p0);

    double dotprod = Dot(off0, normal);

    //! Account for round off - the point may be on the plane!!
    if( fabs( dotprod ) < epsilon3_ )
      continue;

    //! If orientated correctly the second dot product is not needed.
    //! Only need to check to see if the sign is negative.
    if (dotprod * Dot(off1, normal) < 0.0)
      return false;
  }

  return true;
}


template <class Basis>
bool
StructHexVolMesh<Basis>::locate(typename LatVolMesh<Basis>::Cell::index_type &cell,
				const Point &p) const
{
  if (this->basis_.polynomial_order() > 1) return elem_locate(cell, *this, p);

  //! Check last cell found first. Copy cache to cell first so that we
  //! don't care about thread safeness, such that worst case on context
  //! switch is that cache is not found.

  if (cell.mesh_)
  {
    if (cell >= typename LatVolMesh<Basis>::Cell::index_type(this, 0, 0, 0) &&
        cell < typename LatVolMesh<Basis>::Cell::index_type(this,
							    this->ni_ - 1,
                                                            this->nj_ - 1,
                                                            this->nk_ - 1) &&
        inside8_p(cell, p))
    {
        return true;
    }
  }

  ASSERTMSG(synchronized_ & Mesh::LOCATE_E,
            "Must call synchronize LOCATE_E on HexVolMesh first.");

  index_type *iter, *end;
  if (grid_->lookup(&iter, &end, p))
  {
    while (iter != end)
    {
      typename LatVolMesh<Basis>::Cell::index_type idx;

      to_index(idx, *iter);

      if( inside8_p(idx, p) )
      {
        cell = idx;
        return true;
      }
      ++iter;
    }
  }

  return false;
}


template <class Basis>
bool
StructHexVolMesh<Basis>::locate(typename LatVolMesh<Basis>::Node::index_type &node,
				const Point &p) const
{
  node.mesh_ = this;
  typename LatVolMesh<Basis>::Cell::index_type ci;
  if (locate(ci, p)) { //! first try the fast way.
    typename LatVolMesh<Basis>::Node::array_type nodes;
    get_nodes(nodes, ci);

    double dmin =
      (p - points_(nodes[0].k_, nodes[0].j_, nodes[0].i_)).length2();
    node = nodes[0];
    for (size_t i = 1; i < nodes.size(); i++)
    {
      const double d =
        (p - points_(nodes[i].k_, nodes[i].j_, nodes[i].i_)).length2();
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
    typename LatVolMesh<Basis>::Node::iterator ni, nie;
    begin(ni);
    end(nie);
    if (ni == nie) { return false; }

    double min_dist = (p - points_((*ni).k_, (*ni).j_, (*ni).i_)).length2();
    node = *ni;
    ++ni;

    while (ni != nie)
    {
      const double dist =
        (p - points_((*ni).k_, (*ni).j_, (*ni).i_)).length2();
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
int
StructHexVolMesh<Basis>::get_weights(const Point &p,
				     typename LatVolMesh<Basis>::Node::array_type &l,
                                     double *w)
{
  typename LatVolMesh<Basis>::Cell::index_type idx;
  if (locate(idx, p))
  {
    get_nodes(l,idx);
    vector<double> coords(3);
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
StructHexVolMesh<Basis>::get_weights(const Point &p,
				     typename LatVolMesh<Basis>::Cell::array_type &l,
                                     double *w)
{
  typename LatVolMesh<Basis>::Cell::index_type idx;
  if (locate(idx, p))
  {
    l.resize(1);
    l[0] = idx;
    w[0] = 1.0;
    return 1;
  }
  return 0;
}


//! ===================================================================
//! area3D_Polygon(): computes the area of a 3D planar polygon
//!    Input:  int n = the number of vertices in the polygon
//!            Point* V = an array of n+2 vertices in a plane
//!                       with V[n]=V[0] and V[n+1]=V[1]
//!            Point N = unit normal vector of the polygon's plane
//!    Return: the (float) area of the polygon

//! Copyright 2000, softSurfer (www.softsurfer.com)
//! This code may be freely used and modified for any purpose
//! providing that this copyright notice is included with it.
//! SoftSurfer makes no warranty for this code, and cannot be held
//! liable for any real or imagined damage resulting from its use.
//! Users of this code must verify correctness for their application.

template <class Basis>
double
StructHexVolMesh<Basis>::polygon_area(const typename LatVolMesh<Basis>::Node::array_type &ni,
				      const Vector N) const
{
  double area = 0;
  double an, ax, ay, az;  //! abs value of normal and its coords
  int   coord;           //! coord to ignore: 1=x, 2=y, 3=z
  index_type   i, j, k;         //! loop indices
  const size_type n = ni.size();

  //! select largest abs coordinate to ignore for projection
  ax = (N.x()>0 ? N.x() : -N.x());     //! abs x-coord
  ay = (N.y()>0 ? N.y() : -N.y());     //! abs y-coord
  az = (N.z()>0 ? N.z() : -N.z());     //! abs z-coord

  coord = 3;                     //! ignore z-coord
  if (ax > ay) 
  {
    if (ax > az) coord = 1;    //! ignore x-coord
  }
  else if (ay > az) coord = 2;   //! ignore y-coord

  //! compute area of the 2D projection
  for (i=1, j=2, k=0; i<=n; i++, j++, k++)
    switch (coord) 
    {
    case 1:
      area += (point(ni[i%n]).y() *
               (point(ni[j%n]).z() - point(ni[k%n]).z()));
      continue;
    case 2:
      area += (point(ni[i%n]).x() *
               (point(ni[j%n]).z() - point(ni[k%n]).z()));
      continue;
    case 3:
      area += (point(ni[i%n]).x() *
               (point(ni[j%n]).y() - point(ni[k%n]).y()));
      continue;
    }

  //! scale to get area before projection
  an = sqrt( ax*ax + ay*ay + az*az);  //! length of normal vector
  switch (coord) 
  {
  case 1:
    area *= (an / (2*ax));
    break;
  case 2:
    area *= (an / (2*ay));
    break;
  case 3:
    area *= (an / (2*az));
  }
  return area;
}


template <class Basis>
double
StructHexVolMesh<Basis>::pyramid_volume(const typename LatVolMesh<Basis>::Node::array_type &face,
					const Point &p) const
{
  Vector e1(point(face[1])-point(face[0]));
  Vector e2(point(face[1])-point(face[2]));
  if (Cross(e1,e2).length2()>0.0)
  {
    Plane plane(point(face[0]), point(face[1]), point(face[2]));
    //! double dist = plane.eval_point(p);
    return fabs(plane.eval_point(p)*polygon_area(face,plane.normal())*0.25);
  }
  Vector e3(point(face[3])-point(face[2]));
  if (Cross(e2,e3).length2()>0.0) {
    Plane plane(point(face[1]), point(face[2]), point(face[3]));
    //! double dist = plane.eval_point(p);
    return fabs(plane.eval_point(p)*polygon_area(face,plane.normal())*0.25);
  }
  return 0.0;
}


inline double tri_area(const Point &a, const Point &b, const Point &c)
{
  return (0.5*Cross((a-b),(b-c)).length());
}


template <class Basis>
void
StructHexVolMesh<Basis>::compute_grid()
{

  BBox bb = get_bounding_box();
  if (bb.valid())
  {
    //! Cubed root of number of cells to get a subdivision ballpark.
    typename LatVolMesh<Basis>::Cell::size_type csize;  size(csize);
    const int s = (int)(ceil(pow((double)csize , (1.0/3.0)))) / 2 + 1;
    const Vector cell_epsilon = bb.diagonal() * (1.0e-4 / s);
    bb.extend(bb.min() - cell_epsilon*2);
    bb.extend(bb.max() + cell_epsilon*2);

    SearchGridConstructor sgc(s, s, s, bb.min(), bb.max());

    BBox box;
    typename LatVolMesh<Basis>::Node::array_type nodes;
    typename LatVolMesh<Basis>::Cell::iterator ci, cie;
    begin(ci); end(cie);
    while(ci != cie)
    {
      get_nodes(nodes, *ci);

      box.reset();
      for (size_t i = 0; i < nodes.size(); i++)
      {
        box.extend(points_(nodes[i].k_, nodes[i].j_, nodes[i].i_));
      }
      const Point padmin(box.min() - cell_epsilon);
      const Point padmax(box.max() + cell_epsilon);
      box.extend(padmin);
      box.extend(padmax);

      sgc.insert(*ci, box);

      ++ci;
    }

    grid_ = scinew SearchGrid(sgc);
  }
}


template <class Basis>
double
StructHexVolMesh<Basis>::get_size(const typename LatVolMesh<Basis>::Node::index_type &idx) const
{
  return 0.0;
}


template <class Basis>
double
StructHexVolMesh<Basis>::get_size(typename LatVolMesh<Basis>::Edge::index_type idx) const
{
  typename LatVolMesh<Basis>::Node::array_type arr;
  get_nodes(arr, idx);
  Point p0, p1;
  get_center(p0, arr[0]);
  get_center(p1, arr[1]);

  return (p1.asVector() - p0.asVector()).length();
}


template <class Basis>
double
StructHexVolMesh<Basis>::get_size(typename LatVolMesh<Basis>::Face::index_type idx) const
{
  typename LatVolMesh<Basis>::Node::array_type nodes;
  get_nodes(nodes, idx);
  Point p0, p1, p2, p3;
  get_point(p0, nodes[0]);
  get_point(p1, nodes[1]);
  get_point(p2, nodes[2]);
  get_point(p3, nodes[3]);

  return ((Cross(p0-p1,p2-p0)).length()+(Cross(p0-p3,p2-p0)).length())*0.5;
}


template <class Basis>
double
StructHexVolMesh<Basis>::get_size(const typename LatVolMesh<Basis>::Cell::index_type &idx) const
{
  typename LatVolMesh<Basis>::Node::array_type nodes;
  get_nodes(nodes, idx);
  Point p0, p1, p2, p3, p4, p5, p6, p7;
  get_point(p0, nodes[0]);
  get_point(p1, nodes[1]);
  get_point(p2, nodes[2]);
  get_point(p3, nodes[3]);
  get_point(p4, nodes[4]);
  get_point(p5, nodes[5]);
  get_point(p6, nodes[6]);
  get_point(p7, nodes[7]);

  const double a0 = tetrahedra_volume(p0, p1, p2, p5);
  const double a1 = tetrahedra_volume(p0, p2, p3, p7);
  const double a2 = tetrahedra_volume(p0, p5, p2, p7);
  const double a3 = tetrahedra_volume(p0, p5, p7, p4);
  const double a4 = tetrahedra_volume(p5, p2, p7, p6);
  
  return (a0 + a1 + a2 + a3 + a4); 
}


template <class Basis>
void
StructHexVolMesh<Basis>::set_point(const Point &p,
				   const typename LatVolMesh<Basis>::Node::index_type &idx)
{
  points_(idx.k_, idx.j_, idx.i_) = p;
}


template<class Basis>
void
StructHexVolMesh<Basis>::get_random_point(Point &p,
					  const typename LatVolMesh<Basis>::Elem::index_type &ei,
                                          FieldRNG &rng) const
{
  const Point &p0 = points_(ei.k_+0, ei.j_+0, ei.i_+0);
  const Point &p1 = points_(ei.k_+0, ei.j_+0, ei.i_+1);
  const Point &p2 = points_(ei.k_+0, ei.j_+1, ei.i_+1);
  const Point &p3 = points_(ei.k_+0, ei.j_+1, ei.i_+0);
  const Point &p4 = points_(ei.k_+1, ei.j_+0, ei.i_+0);
  const Point &p5 = points_(ei.k_+1, ei.j_+0, ei.i_+1);
  const Point &p6 = points_(ei.k_+1, ei.j_+1, ei.i_+1);
  const Point &p7 = points_(ei.k_+1, ei.j_+1, ei.i_+0);

  const double a0 = tetrahedra_volume(p0, p1, p2, p5);
  const double a1 = tetrahedra_volume(p0, p2, p3, p7);
  const double a2 = tetrahedra_volume(p0, p5, p2, p7);
  const double a3 = tetrahedra_volume(p0, p5, p7, p4);
  const double a4 = tetrahedra_volume(p5, p2, p7, p6);

  const double w = rng() * (a0 + a1 + a2 + a3 + a4);
  
  double t = rng();
  double u = rng();
  double v = rng();

  //! Fold cube into prism.
  if (t + u > 1.0)
  {
    t = 1.0 - t;
    u = 1.0 - u;
  }

  //! Fold prism into tet.
  if (u + v > 1.0)
  {
    const double tmp = v;
    v = 1.0 - t - u;
    u = 1.0 - tmp;
  }
  else if (t + u + v > 1.0)
  {
    const double tmp = v;
    v = t + u + v - 1.0;
    t = 1.0 - u - tmp;
  }
 
  //! Convert to Barycentric and compute new point.
  const double a = 1.0 - t - u - v; 
      
  if (w > (a0 + a1 + a2 + a3))
  {
    p = (p5.vector()*a + p2.vector()*t + p7.vector()*u + p6.vector()*v).point();
  }
  else if (w > (a0 + a1 + a2))
  {
    p = (p0.vector()*a + p5.vector()*t + p7.vector()*u + p5.vector()*v).point();
  }
  else if (w > (a0 + a1))
  {
    p = (p0.vector()*a + p5.vector()*t + p2.vector()*u + p7.vector()*v).point();
  }
  else if (w > a0)
  {
    p = (p0.vector()*a + p2.vector()*t + p3.vector()*u + p7.vector()*v).point();
  }
  else
  {
    p = (p0.vector()*a + p1.vector()*t + p2.vector()*u + p5.vector()*v).point();
  }
}


template <class Basis>
bool
StructHexVolMesh<Basis>::synchronize(unsigned int tosync)
{
  synchronize_lock_.lock();
 
  if (tosync & Mesh::LOCATE_E && !(synchronized_ & Mesh::LOCATE_E))
  {
    compute_grid();
    synchronized_ |= Mesh::LOCATE_E;
    if (!(synchronized_ & Mesh::EPSILON_E))
    {
      compute_epsilon();
      synchronized_ |= Mesh::EPSILON_E;
    }
  }

  if (tosync & Mesh::EPSILON_E && !(synchronized_ & Mesh::EPSILON_E))
  {  
    compute_epsilon();
    synchronized_ |= Mesh::EPSILON_E;  
  }

  synchronize_lock_.unlock();
  
  return (true);
}

template <class Basis>
bool
StructHexVolMesh<Basis>::unsynchronize(unsigned int tosync)
{
  return true;
}

template <class Basis>
void
StructHexVolMesh<Basis>::compute_epsilon()
{
  epsilon_ = get_bounding_box().diagonal().length()*1e-8;
  epsilon3_ = epsilon_*epsilon_*epsilon_;
  synchronized_ |= Mesh::EPSILON_E;
}

#define STRUCT_HEX_VOL_MESH_VERSION 2

template <class Basis>
void
StructHexVolMesh<Basis>::io(Piostream& stream)
{
  int version =  stream.begin_class(type_name(-1), STRUCT_HEX_VOL_MESH_VERSION);
  LatVolMesh<Basis>::io(stream);
  //! IO data members, in order

  if (version == 1)
  {
    //! The dimensions of this array were swapped
    Array3<Point> tpoints;
    Pio(stream, tpoints);
    
    size_type dim1 = tpoints.dim1();
    size_type dim2 = tpoints.dim2();
    size_type dim3 = tpoints.dim3();
    
    points_.resize(dim3,dim2,dim1);
    for (size_type i=0; i<dim1; i++)
      for (size_type j=0; j<dim2; j++)
        for (size_type k=0; k<dim3; k++)
           points_(k,j,i) = tpoints(i,j,k);
  }
  else
  {
    Pio(stream, points_);    
  }
  stream.end_class();

  if (stream.reading())
    this->vmesh_ = CreateVStructHexVolMesh(this);
}


template <class Basis>
const string
StructHexVolMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const string name = type_name(0) + FTNS + type_name(1) + FTNE;
    return name;
  }
  else if (n == 0)
  {
    static const string nm("StructHexVolMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
const TypeDescription*
get_type_description(StructHexVolMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = SCIRun::get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = scinew TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = scinew TypeDescription("StructHexVolMesh", subs,
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructHexVolMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((StructHexVolMesh<Basis> *)0);
}


template <class Basis>
const TypeDescription*
StructHexVolMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructHexVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Node",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructHexVolMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructHexVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Edge",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructHexVolMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructHexVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Face",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
StructHexVolMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((StructHexVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Cell",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}

} //! namespace SCIRun

#endif //! SCI_project_StructHexVolMesh_h
