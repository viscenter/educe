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


#ifndef CORE_DATATYPES_TETVOLMESH_H
#define CORE_DATATYPES_TETVOLMESH_H 1

//! Include what kind of support we want to have
//! Need to fix this and couple it sci-defs
#include <Core/Datatypes/MeshSupport.h>

#include <Core/Containers/StackVector.h>
#include <Core/Containers/LockingHandle.h>
#include <Core/Containers/Handle.h>

#include <Core/Geometry/Transform.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/BBox.h>
#include <Core/Geometry/CompGeom.h>

#include <Core/Basis/Locate.h>
#include <Core/Basis/TetLinearLgn.h>
#include <Core/Basis/TetQuadraticLgn.h>
#include <Core/Basis/TetCubicHmt.h>

#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/FieldIterator.h>
#include <Core/Datatypes/FieldRNG.h>
#include <Core/Datatypes/SearchGrid.h>

//! Incude needed for Windows: declares SCISHARE
#include <Core/Datatypes/share.h>

namespace SCIRun {

using std::pair;
using std::string;
using std::vector;

/////////////////////////////////////////////////////
// Declarations for virtual interface

//! Functions for creating the virtual interface
//! Declare the functions that instantiate the virtual interface
template <class Basis> class TetVolMesh;

//! make sure any other mesh other than the preinstantiate ones
//! returns no virtual interface. Altering this behaviour will allow
//! for dynamically compiling the interfae if needed.
template<class MESH>
VMesh* CreateVTetVolMesh(MESH* mesh) { return (0); }

#if (SCIRUN_TETVOL_SUPPORT > 0)

//! These declarations are needed for a combined dynamic compilation as
//! as well as virtual functions solution.
//! Declare that these can be found in a library that is already
//! precompiled. So dynamic compilation will not instantiate them again.
SCISHARE VMesh* CreateVTetVolMesh(TetVolMesh<TetLinearLgn<Point> >* mesh);
#if (SCIRUN_QUADRATIC_SUPPORT > 0)
SCISHARE VMesh* CreateVTetVolMesh(TetVolMesh<TetQuadraticLgn<Point> >* mesh);
#endif
#if (SCIRUN_CUBIC_SUPPORT > 0)
SCISHARE VMesh* CreateVTetVolMesh(TetVolMesh<TetCubicHmt<Point> >* mesh);
#endif

#endif
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////
// Declarations for TetVolMesh class

template <class Basis>
class TetVolMesh : public Mesh
{

//! Make sure the virtual interface has access
template<class MESH> friend class VTetVolMesh;
template<class MESH> friend class VMeshShared;
template<class MESH> friend class VUnstructuredMesh;

public:

  // Types that change depending on 32 or 64bits
  typedef SCIRun::index_type                under_type;
  typedef SCIRun::index_type                index_type;
  typedef SCIRun::size_type                 size_type;
  typedef SCIRun::mask_type                 mask_type;

  typedef LockingHandle<TetVolMesh<Basis> > handle_type;
  typedef Basis                             basis_type;

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
  typedef Cell Elem;
  typedef Face DElem;

  //! Somehow the information of how to interpolate inside an element
  //! ended up in a separate class, as they need to share information
  //! this construction was created to transfer data. 
  //! Hopefully in the future this class will disappear again.
  friend class ElemData;
  class ElemData
  {
  public:
    typedef typename TetVolMesh<Basis>::index_type  index_type;

  
    ElemData(const TetVolMesh<Basis>& msh, const index_type ind) :
      mesh_(msh),
      index_(ind)
    {
      //! Linear and Constant Basis never use edges_
      if (basis_type::polynomial_order() > 1) 
      {
        mesh_.get_edges_from_cell(edges_, index_);
      }
    }
    
    //! the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const 
    {
      return mesh_.cells_[index_ * 4];
    }
    inline
    index_type node1_index() const 
    {
      return mesh_.cells_[index_ * 4 + 1];
    }
    inline
    index_type node2_index() const 
    {
      return mesh_.cells_[index_ * 4 + 2];
    }
    inline
    index_type node3_index() const 
    {
      return mesh_.cells_[index_ * 4 + 3];
    }

    //! the following designed to coordinate with ::get_edges
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
    index_type edge4_index() const 
    {
      return edges_[4];
    }
    inline
    unsigned edge5_index() const 
    {
      return edges_[5];
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
    const TetVolMesh<Basis>          &mesh_;
    //! copy of element index
    const index_type                 index_;
    //! need edges for quadratic meshes    
    typename Edge::array_type        edges_;
    
  };  


  //////////////////////////////////////////////////////////////////
  
  //! Construct a new mesh
  TetVolMesh();
  
  //! Copy a mesh, needed for detaching the mesh from a field   
  TetVolMesh(const TetVolMesh &copy);
  
  //! Clone function for detaching the mesh and automatically generating
  //! a new version if needed.    
  virtual TetVolMesh *clone() { return new TetVolMesh(*this); }

  //! Destructor 
  virtual ~TetVolMesh();

  //! Access point to virtual interface
  virtual VMesh* vmesh() { return (vmesh_.get_rep()); }
  
  //! This one should go at some point, should be reroute throught the
  //! virtual interface
  virtual int basis_order() { return (basis_.polynomial_order()); }

  //! Topological dimension  
  virtual int  dimensionality() const { return 3; }
  
  //! What kind of mesh is this 
  //! structured = no connectivity data
  //! regular    = no node location data
  virtual int  topology_geometry() const 
    { return (Mesh::UNSTRUCTURED | Mesh::IRREGULAR); }

  //! Get the bounding box of the field    
  virtual BBox get_bounding_box() const;
  
  //! Transform a field (transform all nodes using this transformation matrix)  
  virtual void transform(const Transform &t);

  //! Check whether mesh can be altered by adding nodes or elements
  virtual bool is_editable() const { return (true); }

  //! Has this mesh normals.
  virtual bool has_normals() const { return (false); }

  //! Has this mesh face normals
  virtual bool has_face_normals() const { return (true); }


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
  void to_index(typename Node::index_type &index, index_type i) const
    { index = i; }
  void to_index(typename Edge::index_type &index, index_type i) const
    { index = i; }
  void to_index(typename Face::index_type &index, index_type i) const
    { index = i; }
  void to_index(typename Cell::index_type &index, index_type i) const
    { index = i; }


  //! Get the child topology elements of the given topology
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
    { get_nodes_from_cell(array,idx); }

  void get_edges(typename Edge::array_type &array, 
                 typename Node::index_type idx) const
    { ASSERTFAIL("TetVolMesh: get_edges has not been implemented for nodes"); }
  void get_edges(typename Edge::array_type &array, 
                 typename Edge::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_edges(typename Edge::array_type &array,
                 typename Face::index_type idx) const
    { get_edges_from_face(array,idx); }
  void get_edges(typename Edge::array_type &array,
                 typename Cell::index_type idx) const
    { get_edges_from_cell(array,idx); }

  void get_faces(typename Face::array_type &array,
                 typename Node::index_type idx) const
    { ASSERTFAIL("TetVolMesh: get_faces has not been implemented for nodes"); }
  void get_faces(typename Face::array_type &array,
                 typename Edge::index_type idx) const
    { ASSERTFAIL("TetVolMesh: get_faces has not been implemented for edges"); }
  void get_faces(typename Face::array_type &array,
                 typename Face::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_faces(typename Face::array_type &array, 
                 typename Cell::index_type idx) const
    { get_faces_from_cell(array,idx); }

  void get_cells(typename Cell::array_type &array, 
                 typename Node::index_type idx) const
    { get_cells_from_node(array,idx); }
  void get_cells(typename Cell::array_type &array, 
                 typename Edge::index_type idx) const
    { get_cells_from_edge(array,idx); }
  void get_cells(typename Cell::array_type &array,
                 typename Face::index_type idx) const
    { get_cells_from_face(array,idx); }
  void get_cells(typename Cell::array_type &array, 
                 typename Cell::index_type idx) const
    { array.resize(1); array[0]= idx; }

  void get_elems(typename Elem::array_type &array,
                 typename Node::index_type idx) const
    { get_cells_from_node(array,idx); }
  void get_elems(typename Elem::array_type &array,
                 typename Edge::index_type idx) const
    { get_cells_from_edge(array,idx); }
  void get_elems(typename Elem::array_type &array,
                 typename Face::index_type idx) const
    { get_cells_from_face(array,idx); }
  void get_elems(typename Elem::array_type &array, 
                 typename Cell::index_type idx) const
    { array.resize(1); array[0]= idx; }

  void get_delems(typename DElem::array_type &array,
                  typename Node::index_type idx) const
    { ASSERTFAIL("TetVolMesh: get_faces has not been implemented for nodes"); }
  void get_delems(typename DElem::array_type &array,
                  typename Edge::index_type idx) const
    { ASSERTFAIL("TetVolMesh: get_faces has not been implemented for edges"); }
  void get_delems(typename DElem::array_type &array, 
                  typename Face::index_type idx) const
    { array.resize(1); array[0]= idx; }
  void get_delems(typename DElem::array_type &array,
                  typename Cell::index_type idx) const
    { get_faces_from_cell(array,idx); }
  
  //! Generate the list of points that make up a sufficiently accurate
  //! piecewise linear approximation of an edge.
  template<class VECTOR, class INDEX>
  void pwl_approx_edge(vector<VECTOR > &coords,
                       INDEX ci,
                       unsigned which_edge,
                       unsigned div_per_unit) const
  {
    basis_.approx_edge(which_edge, div_per_unit, coords);
  }

  //! Generate the list of points that make up a sufficiently accurate
  //! piecewise linear approximation of an face.
  template<class VECTOR, class INDEX>
  void pwl_approx_face(vector<vector<VECTOR > > &coords,
                       INDEX ci,
                       unsigned which_face,
                       unsigned div_per_unit) const
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
    { get_cell_center(result, idx); }
  
  //! Get the size of an elemnt (length, area, volume)
  double get_size(typename Node::index_type idx) const 
    { return 0.0; }
  double get_size(typename Edge::index_type idx) const
    {
      StackVector<typename Node::index_type,2> ra;
      get_nodes_from_edge(ra, idx);
      const Point &p0 = point(ra[0]);
      const Point &p1 = point(ra[1]);
      
      return (p1.asVector() - p0.asVector()).length();
    }
  double get_size(typename Face::index_type idx) const
    {
      StackVector<typename Node::index_type,3> ra;
      get_nodes_from_face(ra,idx);
      const Point &p0 = point(ra[0]);
      const Point &p1 = point(ra[1]);
      const Point &p2 = point(ra[2]);
      
      return (Cross(p0-p1,p2-p0)).length()*0.5;
    }
  double get_size(typename Cell::index_type idx) const
    {
      StackVector<typename Node::index_type,4> ra;
      get_nodes_from_cell(ra,idx);
      const Point &p0 = point(ra[0]);
      const Point &p1 = point(ra[1]);
      const Point &p2 = point(ra[2]);
      const Point &p3 = point(ra[3]);

      return fabs(Dot(Cross(p1-p0,p2-p0),p3-p0)) / 6.0;
    }  
  
  //! More specific names for get_size
  double get_length(typename Edge::index_type idx) const
    { return get_size(idx); };
  double get_area(typename Face::index_type idx) const
    { return get_size(idx); };
  double get_volume(typename Cell::index_type idx) const
    { return get_size(idx); };

  //! Get neighbors of an element or a node
  
  //! THIS ONE IS FLAWED AS IN 3D SPACE FOR AND ELEMENT TYPE THAT
  //! IS NOT A VOLUME. HENCE IT WORKS HERE, BUT GENERALLY IT IS FLAWED
  //! AS IT ASSUMES ONLY ONE NEIGHBOR, WHEREAS FOR ANYTHING ELSE THAN
  //! A FACE THERE CAN BE MULTIPLE
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


  //! return false if point is out of range.
  bool locate(typename Node::index_type &loc, const Point &p) const;
  bool locate(typename Edge::index_type &loc, const Point &p) const;
  bool locate(typename Face::index_type &loc, const Point &p) const;
  bool locate(typename Cell::index_type &loc, const Point &p) const;

  //! These should become obsolete soon, they do not follow the concept
  //! of the basis functions....
  int get_weights(const Point &p, typename Node::array_type &l, double *w);
  int get_weights(const Point &p, typename Edge::array_type &l, double *w)
    {ASSERTFAIL("TetVolMesh::get_weights for edges isn't supported"); }
  int get_weights(const Point &p, typename Face::array_type &l, double *w)
    {ASSERTFAIL("TetVolMesh::get_weights for faces isn't supported"); }
  int get_weights(const Point &p, typename Cell::array_type &l, double *w);  
  
  
   //! Access the nodes of the mesh 
  template<class INDEX>
  void get_point(Point &result, INDEX index) const
    { result = points_[index]; }    
  template<class INDEX>
  void set_point(const Point &point, INDEX index)
    { points_[index] = point; }
    
  void get_random_point(Point &p, typename Elem::index_type ei, FieldRNG &rng) const;  

  //! Function for getting node normals
  void get_normal(Vector &result, typename Node::index_type index) const
    { ASSERTFAIL("TetVolMesh: This mesh type does not have node normals."); }

  //! Get the normals at the outside of the element
  template<class VECTOR, class INDEX1, class INDEX2>
  void get_normal(Vector &result, VECTOR& coords, INDEX1 eidx, INDEX2 fidx)
    {
      // Improved algorithm, which should be faster as it is fully
      // on the stack.
            
      // Obtain the inverse jacobian
      double Ji[9];
      inverse_jacobian(coords,eidx,Ji);
      
      // Get the normal in local coordinates
      const double un0 = basis_.unit_face_normals[fidx][0];
      const double un1 = basis_.unit_face_normals[fidx][1];
      const double un2 = basis_.unit_face_normals[fidx][2];
      // Do the matrix multiplication: should result in a vector
      // in the global coordinate space
      result.x(Ji[0]*un0+Ji[1]*un1+Ji[2]*un2);
      result.y(Ji[3]*un0+Ji[4]*un1+Ji[5]*un2);
      result.z(Ji[6]*un0+Ji[7]*un1+Ji[8]*un2);
      
      // normalize vector
      result.normalize();
    }
  
  //! Add a new node to the mesh  
  typename Node::index_type add_point(const Point &p);
  typename Node::index_type add_node(const Point &p) 
    { return(add_point(p)); }
 
  //! Add a new element to the mesh
  template<class ARRAY> 
  typename Elem::index_type add_elem(ARRAY a)
  {
    ASSERTMSG(a.size() == 4, "Tried to add non-tet element.");

    const index_type tet = static_cast<index_type>(cells_.size()) / 4;

    cells_.push_back(static_cast<typename Node::index_type>(a[0]));
    cells_.push_back(static_cast<typename Node::index_type>(a[1]));
    cells_.push_back(static_cast<typename Node::index_type>(a[2]));
    cells_.push_back(static_cast<typename Node::index_type>(a[3]));

    return tet;
  }
 
  //! Functions to improve memory management. Often one knows how many
  //! nodes/elements one needs, prereserving memory is often possible. 
  void node_reserve(size_t s) { points_.reserve(s); }
  void elem_reserve(size_t s) { cells_.reserve(s*4); }
  void resize_nodes(size_t s) { points_.resize(s); }
  void resize_elems(size_t s) { cells_.resize(s*4); }


  //! Get the local coordinates for a certain point within an element
  //! This function uses a couple of newton iterations to find the local
  //! coordinate of a point
  template<class VECTOR, class INDEX>
  bool  get_coords(VECTOR &coords, const Point &p, INDEX idx) const
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
    StackVector<Point,3> Jv;
    ElemData ed(*this,idx);
    basis_.derivate(coords,ed,Jv);
    return (DetMatrix3P(Jv));
  }

  //! Get the jacobian of the transformation. In case one wants the non inverted
  //! version of this matrix. This is currentl here for completeness of the 
  //! interface
  template<class VECTOR, class INDEX>
  void jacobian(const VECTOR& coords,INDEX idx, double* J) const
  {
    StackVector<Point,3> Jv;
    ElemData ed(*this,idx);
    basis_.derivate(coords,ed,Jv);
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
  
  //! Get the inverse jacobian of the transformation. This one is needed to 
  //! translate local gradients into global gradients. Hence it is crucial for
  //! calculating gradients of fields, or constructing finite elements.             
  template<class VECTOR, class INDEX>                               
  double inverse_jacobian(const VECTOR& coords, INDEX idx, double* Ji) const
  {
    StackVector<Point,3> Jv;
    ElemData ed(*this,idx);
    basis_.derivate(coords,ed,Jv);
    return (InverseMatrix3P(Jv,Ji));
  }

  template<class INDEX>
  double scaled_jacobian_metric(INDEX idx) const
  {
    StackVector<Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;

    basis_.derivate(basis_.unit_center,ed,Jv);
    double min_jacobian = DetMatrix3P(Jv);
    double min_scale = Jv[0].asVector().length2()*Jv[1].asVector().length2(); 
    size_t num_vertices = basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      basis_.derivate(basis_.unit_vertices[j],ed,Jv);
      temp = ScaledDetMatrix3P(Jv);
      if (temp < min_jacobian) min_jacobian = temp;
      temp = Jv[0].asVector().length2()*Jv[1].asVector().length2();
      if (temp < min_scale) min_scale = temp;
    }
      
    return (min_jacobian/sqrt(min_scale));
  }


  template<class INDEX>
  double jacobian_metric(INDEX idx) const
  {
    StackVector<Point,3> Jv;
    ElemData ed(*this,idx);

    double temp;

    basis_.derivate(basis_.unit_center,ed,Jv);
    double min_jacobian = DetMatrix3P(Jv);
    
    size_t num_vertices = basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
    {
      basis_.derivate(basis_.unit_vertices[j],ed,Jv);
      temp = DetMatrix3P(Jv);
      if(temp < min_jacobian) min_jacobian = temp;
    }
      
    return (min_jacobian);
  }

  //! Find the closest element to a point
  template <class INDEX>
  double find_closest_elem(Point &result, INDEX &face, const Point &p) const
  {
    ASSERTFAIL("TetVolMesh: Find closest element has not yet been implemented.");
  }

  //! Find the closest elements to a point  
  template<class ARRAY>
  double find_closest_elems(Point &result, ARRAY &faces, const Point &p) const
  {  
    ASSERTFAIL("TetVolMesh: Find closest element has not yet been implemented.");  
  }

  double get_epsilon() const 
  { return (epsilon_); }

  //! Export this class using the old Pio system
  virtual void io(Piostream&);
  
  ///////////////////////////////////////////////////
  // STATIC VARIABLES AND FUNCTIONS
  
  //! This ID is created as soon as this class will be instantiated
  static PersistentTypeID tetvolmesh_typeid;
  
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
  static Persistent* maker() { return scinew TetVolMesh(); }
  //! This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return scinew TetVolMesh(); }

  //////////////////////////////////////////////////////////////////
  // Mesh specific functions (these are not implemented in every mesh)

  
  //! not part of the mesh concept but rather specific to tetvol
  //! Return in fi the face that is opposite the node ni in the cell ci.
  //! Return false if bad input, else true indicating the face was found.
  bool get_face_opposite_node(typename Face::index_type &fi,
                              typename Cell::index_type ci,
                              typename Node::index_type ni) const;

  bool get_node_opposite_face(typename Node::index_type &ni,
                              typename Cell::index_type ci,
                              typename Face::index_type fi) const;


  //! Functions needed for cubit interface
  template <class Iter, class Functor>
  void fill_points(Iter begin, Iter end, Functor fill_ftor);
  template <class Iter, class Functor>
  void fill_cells(Iter begin, Iter end, Functor fill_ftor);

  //! Another redundant function
  void unsynchronize();

  // Extra functionality needed by this specific geometry.
  void                  set_nodes(typename Node::array_type &,
                                  typename Cell::index_type);

  // THIS FUNCTION NEEDS TO BE REVISED AS IT DOES NOT SCALE PROPERLY
  // THE TOLERANCE IS NOT RELATIVE, WHICH IS A PROBLEM.........
  typename Node::index_type     add_find_point(const Point &p,
                                               double err = 1.0e-6);

  // Short cut for generating an element....   
  typename Elem::index_type     add_tet(typename Node::index_type a,
                                typename Node::index_type b,
                                typename Node::index_type c,
                                typename Node::index_type d);
  typename Elem::index_type     add_tet(const Point &p0,
                                const Point &p1,
                                const Point &p2,
                                const Point &p3);


  bool         insert_node_in_elem(typename Elem::array_type &tets,
                                   typename Node::index_type &ni,
                                   typename Elem::index_type ci,
                                   const Point &p);

  //! Deleting cells, make sure mesh is detached
  void delete_cells(set<index_type> &to_delete);
  void delete_nodes(set<index_type> &to_delete);

  //! Orient a cell properly
  void orient(typename Cell::index_type ci);

  //! MORE UNSORTED FUNCTIONS
  //! Used to recompute data for individual cells.  Don't use these, they
  // are not synchronous.  Use create_cell_syncinfo instead.
  void create_cell_edges(typename Cell::index_type);
  void delete_cell_edges(typename Cell::index_type, bool table_only = false);
  void create_cell_faces(typename Cell::index_type);
  void delete_cell_faces(typename Cell::index_type, bool table_only = false);
  void create_cell_node_neighbors(typename Cell::index_type);
  void delete_cell_node_neighbors(typename Cell::index_type);
  void insert_cell_into_grid(typename Cell::index_type ci);
  void remove_cell_from_grid(typename Cell::index_type ci);

  void create_cell_syncinfo(typename Cell::index_type ci);
  void delete_cell_syncinfo(typename Cell::index_type ci);
  // Create the partial sync info needed by insert_node_in_elem
  void create_cell_syncinfo_special(typename Cell::index_type ci);
  void delete_cell_syncinfo_special(typename Cell::index_type ci);
  // May reorient the tet to make the signed volume positive.
  typename Elem::index_type add_tet_pos(typename Node::index_type a,
                                        typename Node::index_type b,
                                        typename Node::index_type c,
                                        typename Node::index_type d);
  void mod_tet_pos(typename Elem::index_type tet,
                   typename Node::index_type a,
                   typename Node::index_type b,
                   typename Node::index_type c,
                   typename Node::index_type d);


                                   
  //! must detach, if altering points!
  vector<Point>& get_points() { return points_; }
  

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
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "TetVolMesh: Must call synchronize EDGES_E first");
    
    array.resize(2);
    const PEdge &e = edges_[idx];
    array[0] = static_cast<typename ARRAY::value_type>(e.nodes_[0]);
    array[1] = static_cast<typename ARRAY::value_type>(e.nodes_[1]);
  }

  template<class ARRAY, class INDEX>
  inline void get_nodes_from_face(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "TetVolMesh: Must call synchronize FACES_E first");
    array.resize(3);
    const PFace &f = faces_[idx];
    array[0] = static_cast<typename ARRAY::value_type>(f.nodes_[0]);
    array[1] = static_cast<typename ARRAY::value_type>(f.nodes_[1]);
    array[2] = static_cast<typename ARRAY::value_type>(f.nodes_[2]);
  }

  template<class ARRAY, class INDEX>
  inline void get_nodes_from_cell(ARRAY& array, INDEX idx) const
  {
    array.resize(4);
    for (index_type i = 0; i < 4; i++)
    {
      array[i] = static_cast<typename ARRAY::value_type>(cells_[idx*4+i]);
    }  
  }

  template<class ARRAY, class INDEX>
  inline void get_nodes_from_elem(ARRAY& array, INDEX idx) const
  {
    get_nodes_from_cell(array,idx);
  }

  template<class ARRAY, class INDEX>
  inline void get_edges_from_face(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & (Mesh::FACES_E|Mesh::EDGES_E),
              "TetVolMesh: Must call synchronize FACES_E and EDGES_E first");
    array.resize(3);
    
    const PFace &f = faces_[idx];
    PEdge e0(f.nodes_[0], f.nodes_[1]);
    PEdge e1(f.nodes_[1], f.nodes_[2]);
    PEdge e2(f.nodes_[2], f.nodes_[0]);

    array[0] = static_cast<typename ARRAY::value_type>(
                                          (*(edge_table_.find(e0))).second);
    array[1] = static_cast<typename ARRAY::value_type>(
                                          (*(edge_table_.find(e1))).second);
    array[2] = static_cast<typename ARRAY::value_type>(
                                          (*(edge_table_.find(e2))).second);
  }

  template<class ARRAY, class INDEX>
  inline void get_edges_from_cell(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "TetVolMesh: Must call synchronize EDGES_E first");
    array.resize(6);
    
    const index_type off = idx * 4;
    PEdge e00(cells_[off + 0], cells_[off + 1]);
    PEdge e01(cells_[off + 1], cells_[off + 2]);
    PEdge e02(cells_[off + 2], cells_[off + 0]);
    PEdge e03(cells_[off + 0], cells_[off + 3]);
    PEdge e04(cells_[off + 1], cells_[off + 3]);
    PEdge e05(cells_[off + 2], cells_[off + 3]);


    array[0] = static_cast<typename ARRAY::value_type>(
                                        (*(edge_table_.find(e00))).second);
    array[1] = static_cast<typename ARRAY::value_type>(
                                        (*(edge_table_.find(e01))).second);
    array[2] = static_cast<typename ARRAY::value_type>(
                                        (*(edge_table_.find(e02))).second);
    array[3] = static_cast<typename ARRAY::value_type>(
                                        (*(edge_table_.find(e03))).second);
    array[4] = static_cast<typename ARRAY::value_type>(
                                        (*(edge_table_.find(e04))).second);
    array[5] = static_cast<typename ARRAY::value_type>(
                                        (*(edge_table_.find(e05))).second);
  }

  template<class ARRAY, class INDEX>
  inline void get_edges_from_elem(ARRAY& array, INDEX idx) const
  {
    get_edges_from_cell(array,idx);
  }
  
  
  template<class ARRAY, class INDEX>
  inline void get_faces_from_cell(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "TetVolMesh: Must call synchronize FACES_E first");
    array.resize(4);

    const index_type off = idx * 4;
    PFace f0(cells_[off + 3], cells_[off + 2], cells_[off + 1]);
    PFace f1(cells_[off + 0], cells_[off + 2], cells_[off + 3]);
    PFace f2(cells_[off + 3], cells_[off + 1], cells_[off + 0]);
    PFace f3(cells_[off + 0], cells_[off + 1], cells_[off + 2]);

    array[0] = static_cast<typename ARRAY::value_type>(
                                          (*(face_table_.find(f0))).second);
    array[1] = static_cast<typename ARRAY::value_type>(
                                          (*(face_table_.find(f1))).second);
    array[2] = static_cast<typename ARRAY::value_type>(
                                          (*(face_table_.find(f2))).second);
    array[3] = static_cast<typename ARRAY::value_type>(
                                          (*(face_table_.find(f3))).second);
  }


  template<class ARRAY, class INDEX>
  inline void get_cells_from_node(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
            "TetVolMesh: Must call synchronize NODE_NEIGHBORS_E first.");
    array.resize(node_neighbors_[idx].size());
    for (size_t i = 0; i < node_neighbors_[idx].size(); ++i)
      array[i] = static_cast<typename ARRAY::value_type>(
                                                    node_neighbors_[idx][i]/4);
  }


  template<class ARRAY, class INDEX>
  inline void get_cells_from_edge(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
              "TetVolMesh: Must call synchronize FACES_E first");
    for (size_t i=0; i< edges_[idx].cells_.size(); i++)
      array[i] = static_cast<typename ARRAY::value_type>(edges_[idx].cells_[i]);  
  }

  
  template<class ARRAY, class INDEX>
  inline void get_cells_from_face(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "TetVolMesh: Must call synchronize FACES_E first");
    if (faces_[idx].cells_[1] == MESH_NO_NEIGHBOR)
    {
      array.resize(1);
      array[0] = static_cast<typename ARRAY::value_type>(faces_[idx].cells_[0]);
    }
    else
    {
      array.resize(2);
      // Fix the order for drawing:
      // first return front face and then
      // back face.
      // This was somehow inverted in the table
      array[0] = static_cast<typename ARRAY::value_type>(faces_[idx].cells_[1]);
      array[1] = static_cast<typename ARRAY::value_type>(faces_[idx].cells_[0]);
    }
  }

  template <class INDEX1, class INDEX2>
  inline bool get_elem_neighbor(INDEX1 &neighbor, INDEX1 elem, INDEX2 delem) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "TetVolMesh: Must call synchronize ELEM_NEIGHBORS_E on TetVolMesh first");
    const PFace &f = faces_[delem];

    if (elem == static_cast<INDEX1>(f.cells_[0])) 
    {
      neighbor = static_cast<INDEX1>(f.cells_[1]);
    } 
    else 
    {
      neighbor = static_cast<INDEX1>(f.cells_[0]);
    }
    if (neighbor == MESH_NO_NEIGHBOR) return (false);
    return (true);
  }

  template <class ARRAY, class INDEX1, class INDEX2>
  inline bool get_elem_neighbors(ARRAY &array, INDEX1 elem, INDEX2 delem) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "Must call synchronize ELEM_NEIGHBORS_E on TetVolMesh first");
    const PFace &f = faces_[delem];

    if (elem == static_cast<INDEX1>(f.cells_[0])) 
    {
      array.resize(1);
      array[0] = static_cast<typename ARRAY::value_type>(f.cells_[1]);
    } 
    else 
    {
      array.resize(1);
      array[0] = static_cast<INDEX1>(f.cells_[0]);
    }
    if (array[0] == MESH_NO_NEIGHBOR) 
    {
      array.clear();
      return (false);
    }
    return (true);
  }

  template <class ARRAY, class INDEX>
  inline void get_elem_neighbors(ARRAY &array, INDEX idx) const
  {
    typename Face::array_type faces;
    get_faces_from_cell(faces, idx);
    array.clear();
    array.reserve(faces.size());
    for (size_t i=0; i< faces.size(); i++)
    {
      typename ARRAY::value_type nbor;
      if (get_elem_neighbor(nbor, idx, i))
      {
        array.push_back(nbor);
      }
    }
  }

  //! We should optimize this function more
  template <class ARRAY, class INDEX>
  inline void get_node_neighbors(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on TetVolMesh first.");
    
    set<index_type> inserted;
    for (size_t i = 0; i < node_neighbors_[idx].size(); i++)
    {
      const index_type base = node_neighbors_[idx][i];
      for (index_type c = base; c < base+4; c++)
      {
        inserted.insert(cells_[c]);
      }
    }

    array.clear();
    array.reserve(inserted.size());
    array.insert(array.begin(), inserted.begin(), inserted.end());
  }


  template <class INDEX>
  inline bool locate_node(INDEX &loc, const Point &p) const
  {
    typename Cell::index_type ci;
    if (locate_elem(ci, p)) // first try the fast way.
    {
      typename Node::array_type nodes;
      get_nodes(nodes, ci);

      double mindist = DBL_MAX;
      for (int i=0; i<4; i++)
      {
        const Point &ptmp = point(nodes[i]);
        double dist = (p - ptmp).length2();
        if (i == 0 || dist < mindist)
        {
          mindist = dist;
          loc = static_cast<INDEX>(nodes[i]);
        }
      }
      
      return (true);
    }
    else
    {  // do exhaustive search.
      bool found_p = false;
      double mindist = DBL_MAX;
      typename Node::iterator bi; begin(bi);
      typename Node::iterator ei; end(ei);
      while (bi != ei)
      {
        const Point &c = point(*bi);
        const double dist = (p - c).length2();
        if (!found_p || dist < mindist)
        {
          mindist = dist;
          loc = static_cast<INDEX>(*bi);
          found_p = true;
        }
        ++bi;
      }
      return found_p;
    }
  }


  template <class INDEX>
  inline bool locate_edge(INDEX &edge, const Point &p) const
  {
    bool found_p = false;
    double mindist = DBL_MAX;
    typename Edge::iterator bi; begin(bi);
    typename Edge::iterator ei; end(ei);
    while (bi != ei)
    {
      Point c;
      get_center(c, *bi);
      const double dist = (p - c).length2();
      if (!found_p || dist < mindist)
      {
        mindist = dist;
        edge = static_cast<INDEX>(*bi);
        found_p = true;
      }
      ++bi;
    }
    return found_p;
  }


  template <class INDEX>
  inline bool locate_face(INDEX &face, const Point &p) const
  {
    bool found_p = false;
    double mindist = DBL_MAX;
    typename Face::iterator bi; begin(bi);
    typename Face::iterator ei; end(ei);
    while (bi != ei)
    {
      Point c;
      get_center(c, *bi);
      const double dist = (p - c).length2();
      if (!found_p || dist < mindist)
      {
        mindist = dist;
        face = static_cast<INDEX>(*bi);
        found_p = true;
      }
      ++bi;
    }
    return found_p;
  }


  template <class INDEX>
  inline bool locate_elem(INDEX &cell, const Point &p) const
  {
    if (basis_.polynomial_order() > 1) return elem_locate(cell, *this, p);
    // Check last cell found first.  Copy cache to cell first so that we
    // don't care about thread safeness, such that worst case on
    // context switch is that cache is not found.
    if (cell > INDEX(0) &&
        cell < INDEX(cells_.size()/4) &&
        inside(typename Cell::index_type(cell), p))
    {
        return true;
    }

    ASSERTMSG(synchronized_ & Mesh::LOCATE_E,"TetVolMesh: need to synchronize LOCATE_E first");

    const list<index_type> *candidates;
    if (grid_->lookup(candidates, p))
    {
      list<index_type>::const_iterator iter = candidates->begin();
      while (iter != candidates->end())
      {
        if (inside(typename Cell::index_type(*iter), p))
        {
          cell = static_cast<INDEX>(*iter);
          return true;
        }
        ++iter;
      }
    }
    return false;
  }

  template <class INDEX>
  inline void get_node_center(Point &p, INDEX idx) const
  {
    p = points_[idx]; 
  }


  template <class INDEX>
  inline void get_edge_center(Point &p, INDEX idx) const
  {
    const double s = 1.0/2.0;
    typename Node::array_type arr;
    get_nodes_from_edge(arr, idx);
    p = points_[arr[0]];
    const Point &p1 = points_[arr[1]];
    p.asVector() += p1.asVector();
    p.asVector() *= s;
  }

  template <class INDEX>
  inline void get_face_center(Point &p, INDEX idx) const
  {
    const double s = 1.0/3.0;
    typename Node::array_type arr;
    get_nodes_from_face(arr, idx);
    p = points_[arr[0]];
    const Point &p1 = points_[arr[1]];
    const Point &p2 = points_[arr[2]];

    p.asVector() += p1.asVector();
    p.asVector() += p2.asVector();
    p.asVector() *= s;
  }


  template <class INDEX>
  inline void get_cell_center(Point &p, INDEX idx) const
  {
    const double s = 0.25;
    const Point &p0 = points_[cells_[idx * 4 + 0]];
    const Point &p1 = points_[cells_[idx * 4 + 1]];
    const Point &p2 = points_[cells_[idx * 4 + 2]];
    const Point &p3 = points_[cells_[idx * 4 + 3]];

    p = ((p0.asVector() + p1.asVector() +
          p2.asVector() + p3.asVector()) * s).asPoint();
  }


  //////////////////////////////////////////////////////////////
  // Internal functions that the mesh depends on  
  
protected:
  //! Face information.
  struct PFace {
    // The order of nodes_ corresponds with cells_[0] for CW/CCW purposes.
    typename Node::index_type         nodes_[3];  //! 3 nodes makes a face.
    typename Cell::index_type         cells_[2];  //! 2 cells share this face.
    typename Node::index_type         snodes_[3]; //! sorted nodes,for hashing

    PFace() {
      nodes_[0] = MESH_NO_NEIGHBOR;
      nodes_[1] = MESH_NO_NEIGHBOR;
      nodes_[2] = MESH_NO_NEIGHBOR;
      snodes_[0] = MESH_NO_NEIGHBOR;
      snodes_[1] = MESH_NO_NEIGHBOR;
      snodes_[2] = MESH_NO_NEIGHBOR;
      cells_[0] = MESH_NO_NEIGHBOR;
      cells_[1] = MESH_NO_NEIGHBOR;
    }
    // snodes_ must be sorted. See Hash Function below.
    PFace(typename Node::index_type n1, typename Node::index_type n2,
          typename Node::index_type n3) 
    {
      cells_[0] = MESH_NO_NEIGHBOR;
      cells_[1] = MESH_NO_NEIGHBOR;
      nodes_[0] = n1;
      nodes_[1] = n2;
      nodes_[2] = n3;
      snodes_[0] = n1;
      snodes_[1] = n2;
      snodes_[2] = n3;
      typename Node::index_type tmp;
      // bubble sort the 3 node indices -- smallest one goes to nodes_[0]
      index_type i,j;
      for (i=0; i<2; i++) 
      {
        for (j=i+1; j<3; j++) 
        {
          if (snodes_[i] > snodes_[j]) 
          {
            tmp = snodes_[i]; snodes_[i] = snodes_[j]; snodes_[j] = tmp;
          }
        }
      }
    }

    bool shared() const { return ((cells_[0] != MESH_NO_NEIGHBOR) &&
                                  (cells_[1] != MESH_NO_NEIGHBOR)); }

    //! true if both have the same nodes (order does not matter)
    bool operator==(const PFace &f) const 
    {
      return ((snodes_[0] == f.snodes_[0]) && (snodes_[1] == f.snodes_[1]) &&
              (snodes_[2] == f.snodes_[2]));
    }

    //! Compares each node.  When a non equal node is found the <
    //! operator is applied.
    bool operator<(const PFace &f) const 
    {
      if (snodes_[0] == f.snodes_[0])
        if (snodes_[1] == f.snodes_[1])
            return (snodes_[2] < f.snodes_[2]);
        else
          return (snodes_[1] < f.snodes_[1]);
      else
        return (snodes_[0] < f.snodes_[0]);
    }
  };

  //! Edge information.
  struct PEdge {
    typename Node::index_type         nodes_[2];   //! 2 nodes makes an edge.
    //! list of all the cells this edge is in.
    vector<typename Cell::index_type> cells_;

    PEdge() : cells_(0) 
    {
      nodes_[0] = MESH_NO_NEIGHBOR;
      nodes_[1] = MESH_NO_NEIGHBOR;
    }
    // node_[0] must be smaller than node_[1]. See Hash Function below.
    PEdge(typename Node::index_type n1,
          typename Node::index_type n2) : cells_(0) 
    {
      if (n1 < n2) 
      {
        nodes_[0] = n1;
        nodes_[1] = n2;
      } 
      else 
      {
        nodes_[0] = n2;
        nodes_[1] = n1;
      }
    }

    bool shared() const { return cells_.size() > 1; }

    //! true if both have the same nodes (order does not matter)
    bool operator==(const PEdge &e) const 
    {
      return ((nodes_[0] == e.nodes_[0]) && (nodes_[1] == e.nodes_[1]));
    }

    //! Compares each node.  When a non equal node is found the <
    //! operator is applied.
    bool operator<(const PEdge &e) const 
    {
      if (nodes_[0] == e.nodes_[0])
        return (nodes_[1] < e.nodes_[1]);
      else
        return (nodes_[0] < e.nodes_[0]);
    }
  };

  /*! hash the egde's node_indecies such that edges with the same nodes
   *  hash to the same value. nodes are sorted on edge construction. */
  static const int sz_int = sizeof(int) * 8; // in bits
  struct FaceHash 
  {
    //! These are needed by the hash_map particularly
    // ANSI C++ allows us to initialize these variables in the
    // declaration.  However there may be compilers which will complain
    // about it.
    static const size_t bucket_size = 4;
    static const size_t min_buckets = 8;

    //! These are for our own use (making the hash function).
    static const int sz_third_int = (int)(sz_int * .33);
    static const int up_mask = (~((int)0) << sz_third_int << sz_third_int);
    static const int mid_mask =  up_mask ^ (~((int)0) << sz_third_int);
    static const int low_mask = ~(up_mask | mid_mask);

    //! This is the hash function
    size_t operator()(const PFace &f) const 
    {
      return ((up_mask & (static_cast<int>(f.snodes_[0]) << sz_third_int << sz_third_int)) |
              (mid_mask & (static_cast<int>(f.snodes_[1]) << sz_third_int)) |
              (low_mask & static_cast<int>(f.snodes_[2])));
    }
    //! This should return less than rather than equal to.
    bool operator()(const PFace &f1, const PFace& f2) const 
    {
      return f1 < f2;
    }
  };

  friend struct FaceHash; // needed by the gcc-2.95.3 compiler

  /*! hash the egde's node_indecies such that edges with the same nodes
   *  hash to the same value. nodes are sorted on edge construction. */
  struct EdgeHash {
    //! These are needed by the hash_map particularly
    // ANSI C++ allows us to initialize these variables in the
    // declaration.  However there may be compilers which will complain
    // about it.
    static const size_t bucket_size = 4;
    static const size_t min_buckets = 8;

    //! These are for our own use (making the hash function.
    static const int sz_int = sizeof(int) * 8; // in bits
    static const int sz_half_int = sizeof(int) << 2; // in bits
    static const int up_mask = ((~((int)0)) << sz_half_int);
    static const int low_mask = (~((int)0) ^ up_mask);

    //! This is the hash function
    size_t operator()(const PEdge &e) const {
      return (static_cast<int>(e.nodes_[0]) << sz_half_int) | 
              (low_mask & static_cast<int>(e.nodes_[1]));
    }
    //!  This should return less than rather than equal to.
    bool operator()(const PEdge &e1, const PEdge& e2) const {
      return e1 < e2;
    }
  };

#ifdef HAVE_HASH_MAP
  #if defined(_WIN32)
    //! hash_map is in stdext namespace
  typedef stdext::hash_map<PFace, typename Face::index_type, FaceHash> face_ht;
  typedef stdext::hash_map<PEdge, typename Edge::index_type, EdgeHash> edge_ht;
  #else
    //! hash_map is in stdext namespace
  typedef hash_map<PFace, typename Face::index_type, FaceHash> face_ht;
  typedef hash_map<PEdge, typename Edge::index_type, EdgeHash> edge_ht;  
  #endif
#else
  typedef map<PFace, typename Face::index_type, FaceHash> face_ht;
  typedef map<PEdge, typename Edge::index_type, EdgeHash> edge_ht;
#endif

  const Point &point(typename Node::index_type idx) const
  { return points_[idx]; }


  // These should not be called outside of the synchronize_lock_.
  void compute_node_neighbors();
  void compute_edges();
  void compute_faces();
  void compute_grid();
  void compute_epsilon();
  
  bool inside(typename Cell::index_type idx, const Point &p) const;


  //! all the vertices
  vector<Point>         points_;

  //! each 4 indecies make up a tet
  vector<under_type>    cells_;

  /*! container for face storage. Must be computed each time
    nodes or cells change. */
  vector<PFace>         faces_;
  face_ht               face_table_;
  /*! container for edge storage. Must be computed each time
    nodes or cells change. */
  vector<PEdge>         edges_;
  edge_ht               edge_table_;

  
  //! iterate over the edge_table_ and store the vector of edges_
  void                  build_edge_vec();

  inline
  void remove_edge(typename Node::index_type n1,
		   typename Node::index_type n2,
		   typename Cell::index_type ci,
       bool table_only = false);
  inline
  void hash_edge(typename Node::index_type n1, typename Node::index_type n2,
		 typename Cell::index_type ci);
  

  //! iterate over the face_table_ and store the vector of faces_
  void build_face_vec();

  inline
  void remove_face(typename Node::index_type n1,
		   typename Node::index_type n2,
		   typename Node::index_type n3,
		   typename Cell::index_type ci,
       bool table_only = false);
  inline
  void hash_face(typename Node::index_type n1, typename Node::index_type n2,
                 typename Node::index_type n3, 
                 typename Cell::index_type ci);

  typedef vector<vector<typename Cell::index_type> > NodeNeighborMap;
  NodeNeighborMap       node_neighbors_;

  Vector cell_epsilon_;

  //! This grid is used as an acceleration structure to expedite calls
  //!  to locate.  For each cell in the grid, we store a list of which
  //!  tets overlap that grid cell -- to find the tet which contains a
  //!  point, we simply find which grid cell contains that point, and
  //!  then search just those tets that overlap that grid cell.
  //!  The grid is only built if synchronize(Mesh::LOCATE_E) is called.
  LockingHandle<SearchGridConstructor>  grid_;

  mask_type             synchronized_;
  Mutex                 synchronize_lock_;
  Basis                 basis_;
  double                epsilon_;
  double                epsilon3_;

  Handle<VMesh>         vmesh_;

public:

  //! Always creates 4 tets, does not handle element boundaries properly.
  bool         insert_node_in_cell(typename Cell::array_type &tets,
                                   typename Cell::index_type ci,
                                   typename Node::index_type &ni,
                                   const Point &p);

  void         insert_node_in_face(typename Cell::array_type &tets,
                                   typename Node::index_type ni,
                                   typename Cell::index_type ci,
                                   const PFace &pf);

  void         insert_node_in_edge(typename Cell::array_type &tets,
                                   typename Node::index_type ni,
                                   typename Cell::index_type ci,
                                   const PEdge &e);

}; // end class TetVolMesh


template <class Basis>
template <class Iter, class Functor>
void
TetVolMesh<Basis>::fill_points(Iter begin, Iter end, Functor fill_ftor) 
{
  synchronize_lock_.lock();
  Iter iter = begin;
  points_.resize(end - begin); // resize to the new size
  vector<Point>::iterator piter = points_.begin();
  while (iter != end) 
  {
    *piter = fill_ftor(*iter);
    ++piter; ++iter;
  }
  synchronize_lock_.unlock();
}


template <class Basis>
template <class Iter, class Functor>
void
TetVolMesh<Basis>::fill_cells(Iter begin, Iter end, Functor fill_ftor) 
{
  synchronize_lock_.lock();
  Iter iter = begin;
  cells_.resize((end - begin) * 4); // resize to the new size
  vector<under_type>::iterator citer = cells_.begin();
  while (iter != end) 
  {
    index_type *nodes = fill_ftor(*iter); // returns an array of length 4
    *citer = nodes[0];
    ++citer;
    *citer = nodes[1];
    ++citer;
    *citer = nodes[2];
    ++citer;
    *citer = nodes[3];
    ++citer; ++iter;
  }
  synchronize_lock_.unlock();
}


template <class Basis>
PersistentTypeID
TetVolMesh<Basis>::tetvolmesh_typeid(TetVolMesh<Basis>::type_name(-1), "Mesh",
                           TetVolMesh<Basis>::maker);


template <class Basis>
const string
TetVolMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const string name = type_name(0) + FTNS + type_name(1) + FTNE;
    return name;
  }
  else if (n == 0)
  {
    static const string nm("TetVolMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
TetVolMesh<Basis>::TetVolMesh() :
  points_(0),
  cells_(0),
  faces_(0),
  face_table_(),
  edges_(0),
  edge_table_(),
  node_neighbors_(0),
  grid_(0),
  synchronized_(Mesh::CELLS_E | Mesh::NODES_E),
  synchronize_lock_("TetVolMesh lock"),
  epsilon_(0.0),
  epsilon3_(0.0)
{
  vmesh_ = CreateVTetVolMesh(this);
}

template <class Basis>
TetVolMesh<Basis>::TetVolMesh(const TetVolMesh &copy):
  points_(0),
  cells_(0),
  faces_(0),
  face_table_(),
  edges_(0),
  edge_table_(),
  node_neighbors_(0),
  grid_(0),
  synchronized_(copy.synchronized_),
  synchronize_lock_("TetVolMesh lock"),
  epsilon_(copy.epsilon_),
  epsilon3_(copy.epsilon3_)
{
  TetVolMesh &lcopy = (TetVolMesh &)copy;
  lcopy.synchronize_lock_.lock();

  synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  synchronized_ &= ~Mesh::EDGES_E;
  synchronized_ &= ~Mesh::FACES_E;

  points_ = copy.points_;
  cells_ = copy.cells_;

  synchronized_ &= ~Mesh::LOCATE_E;
  if (copy.grid_.get_rep())
  {
    grid_ = scinew SearchGridConstructor(*(copy.grid_.get_rep()));
    cell_epsilon_ = copy.cell_epsilon_;
  }
  synchronized_ |= copy.synchronized_ & Mesh::LOCATE_E;
  synchronized_ |= copy.synchronized_ & Mesh::EPSILON_E;

  lcopy.synchronize_lock_.unlock();

  //! Create a new virtual interface for this copy
  //! all pointers have changed hence create a new
  //! virtual interface class
  vmesh_ = CreateVTetVolMesh(this);     
}


template <class Basis>
TetVolMesh<Basis>::~TetVolMesh()
{
}


/* To generate a random point inside of a tetrahedron, we generate random
   barrycentric coordinates (independent random variables between 0 and
   1 that sum to 1) for the point. */
template <class Basis>
void
TetVolMesh<Basis>::get_random_point(Point &p, typename Elem::index_type ei,
                                    FieldRNG &rng) const
{
  // get positions of the vertices
  const Point &p0 = point(cells_[ei*4+0]);
  const Point &p1 = point(cells_[ei*4+1]);
  const Point &p2 = point(cells_[ei*4+2]);
  const Point &p3 = point(cells_[ei*4+3]);

  double t = rng();
  double u = rng();
  double v = rng();

  // Fold cube into prism.
  if (t + u > 1.0)
  {
    t = 1.0 - t;
    u = 1.0 - u;
  }

  // Fold prism into tet.
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

  // Convert to Barycentric and compute new point.
  const double a = 1.0 - t - u - v;
  p = (p0.vector()*a + p1.vector()*t + p2.vector()*u + p3.vector()*v).point();
}


template <class Basis>
BBox
TetVolMesh<Basis>::get_bounding_box() const
{
  //! TODO: This could be included in the synchronize scheme
  BBox result;

  typename Node::iterator ni, nie;
  begin(ni);
  end(nie);
  while (ni != nie)
  {
    result.extend(point(*ni));
    ++ni;
  }
  return result;
}


template <class Basis>
void
TetVolMesh<Basis>::transform(const Transform &t)
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
TetVolMesh<Basis>::remove_face(typename Node::index_type n1,
			       typename Node::index_type n2,
			       typename Node::index_type n3,
			       typename Cell::index_type ci,
             bool table_only)
{
  PFace e(n1, n2, n3);
  typename face_ht::iterator iter = face_table_.find(e);

  if (iter == face_table_.end()) 
  {
    ASSERTFAIL("this face did not exist in the table");
  }
  PFace found_face = (*iter).first;
  index_type found_idx = (*iter).second;
  face_table_.erase(iter);
    
  if (!found_face.shared()) 
  {
    // this face belongs to only one cell
    //faces_.erase(faces_.begin() + found_idx);
    found_face.cells_[0] = MESH_NO_NEIGHBOR;
    found_face.cells_[1] = MESH_NO_NEIGHBOR;
  } 
  else 
  {
    if (found_face.cells_[0] == ci) 
    {
      found_face.cells_[0] = found_face.cells_[1];
      // Swap the order so that the face order remains consistent.
      const under_type tmp = found_face.nodes_[0];
      found_face.nodes_[0] = found_face.nodes_[1];
      found_face.nodes_[1] = tmp;
    }
    found_face.cells_[1] = MESH_NO_NEIGHBOR;
  }
  //reinsert
  face_table_[found_face] = found_idx;
  if (!table_only)
  {
    faces_[found_idx] = found_face;
  }
}

template <class Basis>
void
TetVolMesh<Basis>::hash_face(typename Node::index_type n1,
                             typename Node::index_type n2,
                             typename Node::index_type n3,
                             typename Cell::index_type ci)
{
  PFace f(n1, n2, n3);

  typename face_ht::iterator iter = face_table_.find(f);
  if (iter == face_table_.end()) 
  {
    f.cells_[0] = ci;
    face_table_[f] = 0; // insert for the first time
  } 
  else 
  {
    PFace f = (*iter).first;
    if (f.cells_[1] != MESH_NO_NEIGHBOR) 
    {
      cerr << "This Mesh has problems: Cells #"
           << f.cells_[0] << ", #" << f.cells_[1] << ", and #" << ci
           << " are illegally adjacent." << std::endl;
    } 
    else if (f.cells_[0] == ci) 
    {
      cerr << "This Mesh has problems: Cells #"
           << f.cells_[0] << " and #" << ci
           << " are the same." << std::endl;
    } 
    else 
    {
      f.cells_[1] = ci; // add this cell
      face_table_.erase(iter);
      face_table_[f] = 0;
    }
  }
}

template <class Basis>
void
TetVolMesh<Basis>::build_face_vec()

{
  // dump edges into the faces_ container.
  faces_.resize(face_table_.size());
  typename vector<PFace>::iterator f_iter = faces_.begin();
  typename face_ht::iterator ht_iter = face_table_.begin();
  index_type i = 0;
  while (ht_iter != face_table_.end())
  {
    *f_iter = (*ht_iter).first;
    (*ht_iter).second = i;
    ++f_iter; ++ht_iter; i++;
  }
}

template <class Basis>
void
TetVolMesh<Basis>::compute_faces()
{
  face_table_.clear();
  
  typename Cell::iterator ci, cie;
  begin(ci); end(cie);
  typename Node::array_type arr(4);
  while (ci != cie)
  {
    get_nodes(arr, *ci);
    // 4 faces -- each is entered CCW from outside looking in
    hash_face(arr[0], arr[2], arr[1], *ci);
    hash_face(arr[1], arr[2], arr[3], *ci);
    hash_face(arr[0], arr[1], arr[3], *ci);
    hash_face(arr[0], arr[3], arr[2], *ci);
    ++ci;
  }
  build_face_vec();
  synchronized_ |= Mesh::FACES_E;
  
}

template <class Basis>
void
TetVolMesh<Basis>::hash_edge(typename Node::index_type n1,
                             typename Node::index_type n2,
                             typename Cell::index_type ci)
{
  PEdge e(n1, n2);
  typename edge_ht::iterator iter = edge_table_.find(e);
  if (iter == edge_table_.end()) 
  {
    e.cells_.push_back(ci); // add this cell
    edge_table_[e] = 0; // insert for the first time
  } 
  else 
  {
    PEdge e = (*iter).first;
    e.cells_.push_back(ci); // add this cell
    edge_table_.erase(iter);
    edge_table_[e] = 0;
  }
}

template <class Basis>
void
TetVolMesh<Basis>::compute_edges()
{
  typename Cell::iterator ci, cie;
  begin(ci); end(cie);
  typename Node::array_type arr;
  while (ci != cie)
  {
    get_nodes(arr, *ci);
    hash_edge(arr[0], arr[1], *ci);
    hash_edge(arr[1], arr[2], *ci);
    hash_edge(arr[2], arr[0], *ci);
    hash_edge(arr[3], arr[0], *ci);
    hash_edge(arr[3], arr[1], *ci);
    hash_edge(arr[3], arr[2], *ci);
    ++ci;
  }
  build_edge_vec();
  synchronized_ |= Mesh::EDGES_E;
}


template <class Basis>
void
TetVolMesh<Basis>::compute_node_neighbors()
{
  node_neighbors_.clear();
  node_neighbors_.resize(points_.size());
  index_type i, num_cells = cells_.size();
  for (i = 0; i < num_cells; i++)
  {
    node_neighbors_[cells_[i]].push_back(i);
  }
  synchronized_ |= Mesh::NODE_NEIGHBORS_E;
}


template <class Basis>
bool
TetVolMesh<Basis>::synchronize(mask_type sync)
{
  synchronize_lock_.lock();
  if (sync & Mesh::NODE_NEIGHBORS_E && !(synchronized_ & Mesh::NODE_NEIGHBORS_E))
  {
    compute_node_neighbors();
  }
    
  if ((sync & Mesh::EDGES_E) && !(synchronized_ & Mesh::EDGES_E))
  {
    compute_edges();
  }
    
  if ((sync & (Mesh::FACES_E|Mesh::ELEM_NEIGHBORS_E|Mesh::DELEMS_E)) && !(synchronized_ & Mesh::FACES_E))
  {
    compute_faces();
  }  
    
  if (sync & Mesh::LOCATE_E && !(synchronized_ & Mesh::LOCATE_E))
  {
    compute_grid();
    compute_epsilon();
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
TetVolMesh<Basis>::unsynchronize(mask_type sync)
{
  return (true);
}

// THIS ONE SHOULD GO..............
template <class Basis>
void
TetVolMesh<Basis>::unsynchronize()
{
  synchronize_lock_.lock();

  if (synchronized_ & Mesh::NODE_NEIGHBORS_E)
  {
    node_neighbors_.clear();
  }
  if (synchronized_&Mesh::EDGES_E)
  {
    edge_table_.clear();
    edges_.clear();
  }
  if (synchronized_&Mesh::FACES_E)
  {
    face_table_.clear();
    faces_.clear();
  }
  if (synchronized_ & Mesh::LOCATE_E)
  {
    grid_ = 0;
  }
  synchronized_ = Mesh::NODES_E | Mesh::CELLS_E;

  synchronize_lock_.unlock();
}


template <class Basis>
void
TetVolMesh<Basis>::begin(typename TetVolMesh::Node::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E, "NODES_E not synchronized.");
  itr = 0;
}


template <class Basis>
void
TetVolMesh<Basis>::end(typename TetVolMesh::Node::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E, "NODES_E not synchronized.");
  itr = points_.size();
}


template <class Basis>
void
TetVolMesh<Basis>::size(typename TetVolMesh::Node::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E, "NODES_E not synchronized.");
  s = points_.size();
}


template <class Basis>
void
TetVolMesh<Basis>::begin(typename TetVolMesh::Edge::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E, "EDGES_E not synchronized.");
  itr = 0;
}


template <class Basis>
void
TetVolMesh<Basis>::end(typename TetVolMesh::Edge::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E, "EDGES_E not synchronized.");
  itr = static_cast<index_type>(edges_.size());
}


template <class Basis>
void
TetVolMesh<Basis>::size(typename TetVolMesh::Edge::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E, "EDGES_E not synchronized.");
  s = static_cast<index_type>(edges_.size());
}


template <class Basis>
void
TetVolMesh<Basis>::begin(typename TetVolMesh::Face::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E, "FACES_E not synchronized.");
  itr = 0;
}


template <class Basis>
void
TetVolMesh<Basis>::end(typename TetVolMesh::Face::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E, "FACES_E not synchronized.");
  itr = static_cast<index_type>(faces_.size());
}


template <class Basis>
void
TetVolMesh<Basis>::size(typename TetVolMesh::Face::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E, "FACES_E not synchronized.");
  s = static_cast<index_type>(faces_.size());
}


template <class Basis>
void
TetVolMesh<Basis>::begin(typename TetVolMesh::Cell::iterator &itr) const
{
  ASSERTMSG(synchronized_ & CELLS_E, "CELLS_E not synchronized.");
  itr = 0;
}


template <class Basis>
void
TetVolMesh<Basis>::end(typename TetVolMesh::Cell::iterator &itr) const
{
  ASSERTMSG(synchronized_ & CELLS_E, "CELLS_E not synchronized.");
  itr = cells_.size() >> 2;
}


template <class Basis>
void
TetVolMesh<Basis>::size(typename TetVolMesh::Cell::size_type &s) const
{
  ASSERTMSG(synchronized_ & CELLS_E, "CELLS_E not synchronized.");
  s = static_cast<size_type>(cells_.size() >> 2);
}

template <class Basis>
void
TetVolMesh<Basis>::build_edge_vec()
{
  // dump edges into the edges_ container.
  edges_.resize(edge_table_.size());
  typename vector<PEdge>::iterator e_iter = edges_.begin();
  typename edge_ht::iterator ht_iter = edge_table_.begin();
  while (ht_iter != edge_table_.end()) 
  {
    *e_iter = (*ht_iter).first;
    (*ht_iter).second = 
      static_cast<typename Edge::index_type>(e_iter - edges_.begin());
    ++e_iter; ++ht_iter;
  }
}


template <class Basis>
void
TetVolMesh<Basis>::create_cell_edges(typename Cell::index_type c)
{
  typename Node::array_type arr;
  get_nodes(arr, c);
  hash_edge(arr[0], arr[1], c);
  hash_edge(arr[1], arr[2], c);
  hash_edge(arr[2], arr[0], c);
  hash_edge(arr[3], arr[0], c);
  hash_edge(arr[3], arr[1], c);
  hash_edge(arr[3], arr[2], c);

  // This is a bit heavy handed, should optimize this to a more local cleanup.
  build_edge_vec();
}

template <class Basis>
void
TetVolMesh<Basis>::remove_edge(typename Node::index_type n1,
			       typename Node::index_type n2,
			       typename Cell::index_type ci,
             bool table_only)
{
  PEdge e(n1, n2);
  typename edge_ht::iterator iter = edge_table_.find(e);

  if (iter == edge_table_.end()) 
  {
    ASSERTFAIL("this edge did not exist in the table");
  }
  PEdge found_edge = (*iter).first;
  unsigned found_idx = (*iter).second;
  edge_table_.erase(iter);
    
  if (!found_edge.shared())
  {
    // This edge belongs to only one cell.
    if (!table_only)
    {
      edges_.erase(edges_.begin() + found_idx);
    }
  }
  else
  {
    typename vector<typename Cell::index_type>::iterator citer;
    citer = std::find(found_edge.cells_.begin(), found_edge.cells_.end(), ci);
    found_edge.cells_.erase(citer);
    //Reinsert remaining partial edges.
    edge_table_[found_edge] = found_idx;
    if (!table_only)
    {
      edges_[found_idx] = found_edge;
    }
  }
}
 
template <class Basis>
void
TetVolMesh<Basis>::delete_cell_edges(typename Cell::index_type c,
                                     bool table_only)
{
  typename Node::array_type arr;
  get_nodes(arr, c);
  remove_edge(arr[0], arr[1], c, table_only);
  remove_edge(arr[1], arr[2], c, table_only);
  remove_edge(arr[2], arr[0], c, table_only);
  remove_edge(arr[3], arr[0], c, table_only);
  remove_edge(arr[3], arr[1], c, table_only);
  remove_edge(arr[3], arr[2], c, table_only);
}

template <class Basis>
void
TetVolMesh<Basis>::create_cell_faces(typename Cell::index_type c)
{
  typename Node::array_type arr;
  get_nodes(arr, c);
  hash_face(arr[0], arr[2], arr[1], c);
  hash_face(arr[1], arr[2], arr[3], c);
  hash_face(arr[0], arr[1], arr[3], c);
  hash_face(arr[0], arr[3], arr[2], c);

  // This is a bit heavy handed, should optimize this to a more local cleanup.
  build_face_vec();
}


template <class Basis>
void
TetVolMesh<Basis>::delete_cell_faces(typename Cell::index_type c,
                                     bool table_only)
{
  typename Node::array_type arr;
  get_nodes(arr, c);
  remove_face(arr[0], arr[2], arr[1], c, table_only);
  remove_face(arr[1], arr[2], arr[3], c, table_only);
  remove_face(arr[0], arr[1], arr[3], c, table_only);
  remove_face(arr[0], arr[3], arr[2], c, table_only);
}


template <class Basis>
void
TetVolMesh<Basis>::create_cell_node_neighbors(typename Cell::index_type c)
{
  for (index_type i = c*4; i < c*4+4; ++i)
  {
    node_neighbors_[cells_[i]].push_back(i);
  }
}


template <class Basis>
void
TetVolMesh<Basis>::delete_cell_node_neighbors(typename Cell::index_type c)
{
  for (index_type i = c*4; i < c*4+4; ++i)
  {
    const index_type n = cells_[i];
    typename vector<typename Cell::index_type>::iterator node_cells_end =
      node_neighbors_[n].end();
    typename vector<typename Cell::index_type>::iterator cell =
      node_neighbors_[n].begin();
    while (cell != node_cells_end && (*cell) != i) ++cell;

    //! ASSERT that the node_neighbors_ structure contains this cell
    ASSERT(cell != node_cells_end);

    node_neighbors_[n].erase(cell);
  }
}

template <class Basis>
void
TetVolMesh<Basis>::create_cell_syncinfo(typename Cell::index_type ci)
{
  synchronize_lock_.lock();
  if (synchronized_ & Mesh::NODE_NEIGHBORS_E)
    create_cell_node_neighbors(ci);
  if (synchronized_&Mesh::EDGES_E)
    create_cell_edges(ci);
  if (synchronized_&Mesh::FACES_E)
    create_cell_faces(ci);
  if (synchronized_ & Mesh::LOCATE_E)
    insert_cell_into_grid(ci);
  synchronize_lock_.unlock();
}

template <class Basis>
void
TetVolMesh<Basis>::delete_cell_syncinfo(typename Cell::index_type ci)
{
  synchronize_lock_.lock();
  if (synchronized_ & Mesh::NODE_NEIGHBORS_E)
    delete_cell_node_neighbors(ci);
  if (synchronized_&Mesh::EDGES_E)
    delete_cell_edges(ci);
  if (synchronized_&Mesh::FACES_E)
    delete_cell_faces(ci);
  if (synchronized_ & Mesh::LOCATE_E)
    remove_cell_from_grid(ci);
  synchronize_lock_.unlock();
}

template <class Basis>
void
TetVolMesh<Basis>::create_cell_syncinfo_special(typename Cell::index_type ci)
{
  synchronize_lock_.lock();
  if (synchronized_ & Mesh::NODE_NEIGHBORS_E)
    create_cell_node_neighbors(ci);
  if (synchronized_&Mesh::EDGES_E)
  {
    typename Node::array_type arr;
    get_nodes(arr, ci);
    hash_edge(arr[0], arr[1], ci);
    hash_edge(arr[1], arr[2], ci);
    hash_edge(arr[2], arr[0], ci);
    hash_edge(arr[3], arr[0], ci);
    hash_edge(arr[3], arr[1], ci);
    hash_edge(arr[3], arr[2], ci);
  }
  if (synchronized_&Mesh::FACES_E)
  {
    typename Node::array_type arr;
    get_nodes(arr, ci);
    hash_face(arr[0], arr[2], arr[1], ci);
    hash_face(arr[1], arr[2], arr[3], ci);
    hash_face(arr[0], arr[1], arr[3], ci);
    hash_face(arr[0], arr[3], arr[2], ci);
  }
  if (synchronized_ & Mesh::LOCATE_E)
    insert_cell_into_grid(ci);
  synchronize_lock_.unlock();
}

template <class Basis>
void
TetVolMesh<Basis>::delete_cell_syncinfo_special(typename Cell::index_type ci)
{
  synchronize_lock_.lock();
  if (synchronized_ & Mesh::NODE_NEIGHBORS_E)
    delete_cell_node_neighbors(ci);
  if (synchronized_&Mesh::EDGES_E)
    delete_cell_edges(ci, true);
  if (synchronized_&Mesh::FACES_E)
    delete_cell_faces(ci, true);
  if (synchronized_ & Mesh::LOCATE_E)
    remove_cell_from_grid(ci);
  synchronize_lock_.unlock();
}


template <class Basis>
void
TetVolMesh<Basis>::set_nodes(typename Node::array_type &array,
                             typename Cell::index_type idx)
{
  ASSERT(array.size() == 4);

  delete_cell_syncinfo(idx);

  for (index_type n = 0; n < 4; ++n)
    cells_[idx * 4 + n] = array[n];

  create_cell_syncinfo(idx);
}

//! Return in fi the face that is opposite the node ni in the cell ci.
//! Return false if bad input, else true indicating the face was found.
template <class Basis>
bool
TetVolMesh<Basis>::get_face_opposite_node(typename Face::index_type &fi,
                                          typename Cell::index_type ci,
                                          typename Node::index_type ni) const
{
  typename Face::array_type faces;
  get_faces(faces, ci);
  for (index_type i = 0; i < 4; i++) 
  {
    const PFace &f = faces_[faces[i]];
    fi = faces[i];
    if (ni != f.nodes_[0] && ni != f.nodes_[1] && ni != f.nodes_[2]) 
    {
      return true;
    }
  }
  return false;
}

//! Return in ni the node that is opposite the face fi in the cell ci.
//! Return false if bad input, else true indicating the node was found.
template <class Basis>
bool
TetVolMesh<Basis>::get_node_opposite_face(typename Node::index_type &ni,
                                          typename Cell::index_type ci,
                                          typename Face::index_type fi) const
{
  typename Node::array_type nodes;
  get_nodes(nodes, ci);
  const PFace &f = faces_[fi];
  for (int i = 0; i < 4; i++) 
  {
    ni = nodes[i];
    if (ni != f.nodes_[0] && ni != f.nodes_[1] && ni != f.nodes_[2]) 
    {
      return true;
    }
  }
  return false;
}


//! Get neigbor across a specific face.

template <class Basis>
bool
TetVolMesh<Basis>::locate(typename Node::index_type &loc, const Point &p) const
{
  return(locate_node(loc,p));
}


template <class Basis>
bool
TetVolMesh<Basis>::locate(typename Edge::index_type &edge, const Point &p) const
{
  return(locate_edge(edge,p));
}


template <class Basis>
bool
TetVolMesh<Basis>::locate(typename Face::index_type &face, const Point &p) const
{
  return(locate_face(face,p));
}


template <class Basis>
bool
TetVolMesh<Basis>::locate(typename Cell::index_type &cell, const Point &p) const
{
  return(locate_elem(cell,p));
}


template <class Basis>
int
TetVolMesh<Basis>::get_weights(const Point &p, typename Cell::array_type &l,
                               double *w)
{
  typename Cell::index_type idx;
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
TetVolMesh<Basis>::get_weights(const Point &p, typename Node::array_type &l,
                               double *w)
{
  typename Cell::index_type idx;
  if (locate(idx, p))
  {
    get_nodes(l,idx);
    vector<double> coords(3);
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
TetVolMesh<Basis>::insert_cell_into_grid(typename Cell::index_type ci)
{
  // TODO:  This can crash if you insert a new cell outside of the grid.
  // Need to recompute grid at that point.

  BBox box;
  box.extend(points_[cells_[ci*4+0]]);
  box.extend(points_[cells_[ci*4+1]]);
  box.extend(points_[cells_[ci*4+2]]);
  box.extend(points_[cells_[ci*4+3]]);
  const Point padmin(box.min() - cell_epsilon_);
  const Point padmax(box.max() + cell_epsilon_);
  box.extend(padmin);
  box.extend(padmax);
  grid_->insert(ci, box);
}


template <class Basis>
void
TetVolMesh<Basis>::remove_cell_from_grid(typename Cell::index_type ci)
{
  BBox box;
  box.extend(points_[cells_[ci*4+0]]);
  box.extend(points_[cells_[ci*4+1]]);
  box.extend(points_[cells_[ci*4+2]]);
  box.extend(points_[cells_[ci*4+3]]);
  const Point padmin(box.min() - cell_epsilon_);
  const Point padmax(box.max() + cell_epsilon_);
  box.extend(padmin);
  box.extend(padmax);
  grid_->remove(ci, box);
}


template <class Basis>
void
TetVolMesh<Basis>::compute_grid()
{
  BBox bb = get_bounding_box();
  if (bb.valid())
  {
    // Cubed root of number of cells to get a subdivision ballpark.
    typename Cell::size_type csize;  size(csize);
    const size_type s = static_cast<size_type>(ceil(pow((double)csize , (1.0/3.0)))) / 2 + 1;
    cell_epsilon_ = bb.diagonal() * (1.0e-4 / s);
    bb.extend(bb.min() - cell_epsilon_ * 2);
    bb.extend(bb.max() + cell_epsilon_ * 2);

    grid_ = scinew SearchGridConstructor(s, s, s, bb.min(), bb.max());

    typename Node::array_type nodes;
    typename Cell::iterator ci, cie;
    begin(ci); end(cie);
    while(ci != cie)
    {
      insert_cell_into_grid(*ci);
      ++ci;
    }
  }

  synchronized_ |= Mesh::LOCATE_E;
}

template <class Basis>
void
TetVolMesh<Basis>::compute_epsilon()
{
  epsilon_ = get_bounding_box().diagonal().length()*1e-8;
  epsilon3_ = epsilon_*epsilon_*epsilon_;
  synchronized_ |= EPSILON_E;
}



template <class Basis>
bool
TetVolMesh<Basis>::inside(typename Cell::index_type idx, const Point &p) const
{
  // TODO: This has not been tested.
  // TODO: Looks like too much code to check sign of 4 plane/point tests.
  const Point &p0 = points_[cells_[idx*4+0]];
  const Point &p1 = points_[cells_[idx*4+1]];
  const Point &p2 = points_[cells_[idx*4+2]];
  const Point &p3 = points_[cells_[idx*4+3]];
  const double x0 = p0.x();
  const double y0 = p0.y();
  const double z0 = p0.z();
  const double x1 = p1.x();
  const double y1 = p1.y();
  const double z1 = p1.z();
  const double x2 = p2.x();
  const double y2 = p2.y();
  const double z2 = p2.z();
  const double x3 = p3.x();
  const double y3 = p3.y();
  const double z3 = p3.z();

  const double a0 = + x1*(y2*z3-y3*z2) + x2*(y3*z1-y1*z3) + x3*(y1*z2-y2*z1);
  const double a1 = - x2*(y3*z0-y0*z3) - x3*(y0*z2-y2*z0) - x0*(y2*z3-y3*z2);
  const double a2 = + x3*(y0*z1-y1*z0) + x0*(y1*z3-y3*z1) + x1*(y3*z0-y0*z3);
  const double a3 = - x0*(y1*z2-y2*z1) - x1*(y2*z0-y0*z2) - x2*(y0*z1-y1*z0);
  const double iV6 = 1.0 / (a0+a1+a2+a3);

  const double b0 = - (y2*z3-y3*z2) - (y3*z1-y1*z3) - (y1*z2-y2*z1);
  const double c0 = + (x2*z3-x3*z2) + (x3*z1-x1*z3) + (x1*z2-x2*z1);
  const double d0 = - (x2*y3-x3*y2) - (x3*y1-x1*y3) - (x1*y2-x2*y1);
  const double s0 = iV6 * (a0 + b0*p.x() + c0*p.y() + d0*p.z());
  if (s0 < -1e-8)
    return false;

  const double b1 = + (y3*z0-y0*z3) + (y0*z2-y2*z0) + (y2*z3-y3*z2);
  const double c1 = - (x3*z0-x0*z3) - (x0*z2-x2*z0) - (x2*z3-x3*z2);
  const double d1 = + (x3*y0-x0*y3) + (x0*y2-x2*y0) + (x2*y3-x3*y2);
  const double s1 = iV6 * (a1 + b1*p.x() + c1*p.y() + d1*p.z());
  if (s1 < -1e-8)
    return false;

  const double b2 = - (y0*z1-y1*z0) - (y1*z3-y3*z1) - (y3*z0-y0*z3);
  const double c2 = + (x0*z1-x1*z0) + (x1*z3-x3*z1) + (x3*z0-x0*z3);
  const double d2 = - (x0*y1-x1*y0) - (x1*y3-x3*y1) - (x3*y0-x0*y3);
  const double s2 = iV6 * (a2 + b2*p.x() + c2*p.y() + d2*p.z());
  if (s2 < -1e-8)
    return false;

  const double b3 = +(y1*z2-y2*z1) + (y2*z0-y0*z2) + (y0*z1-y1*z0);
  const double c3 = -(x1*z2-x2*z1) - (x2*z0-x0*z2) - (x0*z1-x1*z0);
  const double d3 = +(x1*y2-x2*y1) + (x2*y0-x0*y2) + (x0*y1-x1*y0);
  const double s3 = iV6 * (a3 + b3*p.x() + c3*p.y() + d3*p.z());
  if (s3 < -1e-8)
    return false;

  return true;
}


template <class Basis>
typename TetVolMesh<Basis>::Node::index_type
TetVolMesh<Basis>::add_find_point(const Point &p, double err)
{
  typename Node::index_type i;
  if (locate(i, p) && (points_[i] - p).length2() < epsilon_)
  {
    return i;
  }
  else
  {
    points_.push_back(p);
    if (synchronized_ & Mesh::NODE_NEIGHBORS_E) {
      synchronize_lock_.lock();
      node_neighbors_.push_back(vector<typename Cell::index_type>());
      synchronize_lock_.unlock();
    }
    return points_.size() - 1;
  }
}


template <class Basis>
typename TetVolMesh<Basis>::Elem::index_type
TetVolMesh<Basis>::add_tet(typename Node::index_type a, 
			   typename Node::index_type b,
			   typename Node::index_type c, 
			   typename Node::index_type d)
{
  const index_type tet = static_cast<index_type>(cells_.size()) / 4;
  cells_.push_back(a);
  cells_.push_back(b);
  cells_.push_back(c);
  cells_.push_back(d);
  return tet;
}


template <class Basis>
typename TetVolMesh<Basis>::Elem::index_type
TetVolMesh<Basis>::add_tet_pos(typename Node::index_type a, 
                               typename Node::index_type b,
                               typename Node::index_type c, 
                               typename Node::index_type d)
{
  const index_type tet = static_cast<index_type>(cells_.size()) / 4;
  const Point &p0 = point(a);
  const Point &p1 = point(b);
  const Point &p2 = point(c);
  const Point &p3 = point(d);

  if (Dot(Cross(p1-p0,p2-p0),p3-p0) >= 0.0)
  {
    cells_.push_back(a);
    cells_.push_back(b);
  }
  else
  {
    cells_.push_back(b);
    cells_.push_back(a);
  }
  cells_.push_back(c);
  cells_.push_back(d);
  return tet;
}


template <class Basis>
void
TetVolMesh<Basis>::mod_tet_pos(typename TetVolMesh<Basis>::Elem::index_type ci,
                               typename Node::index_type a, 
                               typename Node::index_type b,
                               typename Node::index_type c, 
                               typename Node::index_type d)
{
  const Point &p0 = point(a);
  const Point &p1 = point(b);
  const Point &p2 = point(c);
  const Point &p3 = point(d);

  if (Dot(Cross(p1-p0,p2-p0),p3-p0) >= 0.0)
  {
    cells_[ci*4+0] = a;
    cells_[ci*4+1] = b;
  }
  else
  {
    cells_[ci*4+0] = b;
    cells_[ci*4+1] = a;
  }
  cells_[ci*4+2] = c;
  cells_[ci*4+3] = d;
}


template <class Basis>
typename TetVolMesh<Basis>::Node::index_type
TetVolMesh<Basis>::add_point(const Point &p)
{
  points_.push_back(p);
  return points_.size() - 1;
}


template <class Basis>
typename TetVolMesh<Basis>::Elem::index_type
TetVolMesh<Basis>::add_tet(const Point &p0, const Point &p1, const Point &p2,
                    const Point &p3)
{
  return add_tet(add_find_point(p0), add_find_point(p1),
                 add_find_point(p2), add_find_point(p3));
}

template <class Basis>
void
TetVolMesh<Basis>::delete_cells(set<index_type> &to_delete)
{
  synchronize_lock_.lock();
  set<index_type>::reverse_iterator iter = to_delete.rbegin();
  while (iter != to_delete.rend()) 
  {
    // erase the correct cell
    typename TetVolMesh<Basis>::Cell::index_type ci = *iter++;
    index_type ind = ci * 4;
    vector<index_type>::iterator cb = cells_.begin() + ind;
    vector<index_type>::iterator ce = cb;
    ce+=4;
    cells_.erase(cb, ce);
  }

  synchronized_ &= ~Mesh::LOCATE_E;
  synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  if (synchronized_ & Mesh::FACES_E)
  {
    synchronized_ &= ~Mesh::FACES_E;
    compute_faces();
  }
  if (synchronized_ & Mesh::EDGES_E)
  {
    synchronized_ &= ~Mesh::EDGES_E;
    compute_edges();
  }
  synchronize_lock_.unlock();
}


template <class Basis>
void
TetVolMesh<Basis>::delete_nodes(set<index_type> &to_delete)
{
  synchronize_lock_.lock();
  set<index_type>::reverse_iterator iter = to_delete.rbegin();
  while (iter != to_delete.rend()) 
  {
    typename TetVolMesh::Node::index_type n = *iter++;
    vector<Point>::iterator pit = points_.begin() + n;
    points_.erase(pit);
  }
  synchronized_ &= ~Mesh::LOCATE_E;
  synchronized_ &= ~Mesh::NODE_NEIGHBORS_E;
  if (synchronized_ & Mesh::FACES_E)
  {
    synchronized_ &= ~Mesh::FACES_E;
    compute_faces();
  }
  if (synchronized_ & Mesh::EDGES_E)
  {
    synchronized_ &= ~Mesh::EDGES_E;
    compute_edges();
  }
  synchronize_lock_.unlock();
}

template <class Basis>
bool
TetVolMesh<Basis>::insert_node_in_cell(typename Cell::array_type &tets,
                                       typename Cell::index_type ci,
                                       typename Node::index_type &pi,
                                       const Point &p)
{
  if (!inside(ci, p)) return false;

  pi = add_point(p);

  delete_cell_syncinfo_special(ci);

  tets.resize(4, ci);
  const index_type index = ci*4;
  tets[1] = add_tet_pos(cells_[index+0], cells_[index+3], cells_[index+1], pi);
  tets[2] = add_tet_pos(cells_[index+1], cells_[index+3], cells_[index+2], pi);
  tets[3] = add_tet_pos(cells_[index+0], cells_[index+2], cells_[index+3], pi);

  mod_tet_pos(ci, cells_[index+0], cells_[index+1], cells_[index+2], pi);

  create_cell_syncinfo_special(ci);
  create_cell_syncinfo_special(tets[1]);
  create_cell_syncinfo_special(tets[2]);
  create_cell_syncinfo_special(tets[3]);

  return true;
}



template <class Basis>
void
TetVolMesh<Basis>::insert_node_in_face(typename Cell::array_type &tets,
                                       typename Node::index_type pi,
                                       typename Cell::index_type ci,
                                       const PFace &f)
{
  // NOTE: This function assumes that the same operation has or will happen 
  // to any neighbor cell across the face.
  
  index_type skip;
  for (skip = 0; skip < 4; skip++)
  {
    typename Node::index_type ni = cells_[ci*4+skip];
    if (ni != f.nodes_[0] && ni != f.nodes_[1] && ni != f.nodes_[2]) break;
  }
  
  delete_cell_syncinfo_special(ci);
  
  const index_type i = ci*4;
  tets.push_back(ci);
  if (skip == 0)
  {
    tets.push_back(add_tet_pos(cells_[i+0], cells_[i+3], cells_[i+1], pi));
    tets.push_back(add_tet_pos(cells_[i+0], cells_[i+2], cells_[i+3], pi));
    mod_tet_pos(ci, cells_[i+0], cells_[i+1], cells_[i+2], pi);
  }
  else if (skip == 1)
  {
    tets.push_back(add_tet_pos(cells_[i+0], cells_[i+3], cells_[i+1], pi));
    tets.push_back(add_tet_pos(cells_[i+1], cells_[i+3], cells_[i+2], pi));
    mod_tet_pos(ci, cells_[i+0], cells_[i+1], cells_[i+2], pi);
  }
  else if (skip == 2)
  {
    tets.push_back(add_tet_pos(cells_[i+1], cells_[i+3], cells_[i+2], pi));
    tets.push_back(add_tet_pos(cells_[i+0], cells_[i+2], cells_[i+3], pi));
    mod_tet_pos(ci, cells_[i+0], cells_[i+1], cells_[i+2], pi);
  }
  else if (skip == 3)
  {
    tets.push_back(add_tet_pos(cells_[i+0], cells_[i+3], cells_[i+1], pi));
    tets.push_back(add_tet_pos(cells_[i+1], cells_[i+3], cells_[i+2], pi));
    mod_tet_pos(ci, cells_[i+0], cells_[i+2], cells_[i+3], pi);
  }

  create_cell_syncinfo_special(ci);
  create_cell_syncinfo_special(tets[tets.size()-2]);
  create_cell_syncinfo_special(tets[tets.size()-1]);
}


template <class Basis>
void
TetVolMesh<Basis>::insert_node_in_edge(typename Cell::array_type &tets,
                                       typename Node::index_type pi,
                                       typename Cell::index_type ci,
                                       const PEdge &e)
{
  index_type skip1, skip2;
  skip1 = -1;
  for (index_type j = 0; j < 4; j++)
  {
    typename Node::index_type ni = cells_[ci*4+j];
    if (ni != e.nodes_[0] && ni != e.nodes_[1])
    {
      if (skip1 == -1) skip1 = j;
      else skip2 = j;
    }
  }

  delete_cell_syncinfo_special(ci);

  bool pushed = false;
  tets.push_back(ci);
  const index_type i = ci*4;
  if (skip1 != 0 && skip2 != 0)
  {
    // add 1 3 2 pi
    tets.push_back(add_tet_pos(cells_[i+1], cells_[i+3], cells_[i+2], pi));
    pushed = true;
  }
  if (skip1 != 1 && skip2 != 1)
  {
    // add 0 2 3 pi
    if (pushed)
    {
      mod_tet_pos(ci, cells_[i+0], cells_[i+2], cells_[i+3], pi);
    }
    else
    {
      tets.push_back(add_tet_pos(cells_[i+0], cells_[i+2], cells_[i+3], pi));
    }
    pushed = true;
  }
  if (skip1 != 2 && skip2 != 2)
  {
    // add 0 3 1 pi
    if (pushed)
    {
      mod_tet_pos(ci, cells_[i+0], cells_[i+3], cells_[i+1], pi);
    }
    else
    {
      tets.push_back(add_tet_pos(cells_[i+0], cells_[i+3], cells_[i+1], pi));
    }
    pushed = true;
  }
  if (skip1 != 3 && skip2 != 3)
  {
    // add 0 1 2 pi
    ASSERTMSG(pushed,
              "insert_node_in_cell_edge::skip1 or skip2 were invalid.");
    mod_tet_pos(ci, cells_[i+0], cells_[i+1], cells_[i+2], pi);
  }

  create_cell_syncinfo_special(ci);
  create_cell_syncinfo_special(tets[tets.size()-1]);
}


template <class Basis>
bool
TetVolMesh<Basis>::insert_node_in_elem(typename Elem::array_type &tets,
                                       typename Node::index_type &pi,
                                       typename Elem::index_type ci,
                                       const Point &p)
{
  const Point &p0 = points_[cells_[ci*4 + 0]];
  const Point &p1 = points_[cells_[ci*4 + 1]];
  const Point &p2 = points_[cells_[ci*4 + 2]];
  const Point &p3 = points_[cells_[ci*4 + 3]];

  // Compute all the new tet areas.
  const double aerr = fabs(Dot(Cross(p1 - p0, p2 - p0), p3 - p0)) * 0.01;
  const double a0 = fabs(Dot(Cross(p1 - p, p2 - p), p3 - p));
  const double a1 = fabs(Dot(Cross(p - p0, p2 - p0), p3 - p0));
  const double a2 = fabs(Dot(Cross(p1 - p0, p - p0), p3 - p0));
  const double a3 = fabs(Dot(Cross(p1 - p0, p2 - p0), p - p0));

  mask_type mask = 0;
  if (a0 >= aerr && a0 >= MIN_ELEMENT_VAL) { mask |= 1; }
  if (a1 >= aerr && a1 >= MIN_ELEMENT_VAL) { mask |= 2; }
  if (a2 >= aerr && a2 >= MIN_ELEMENT_VAL) { mask |= 4; }
  if (a3 >= aerr && a3 >= MIN_ELEMENT_VAL) { mask |= 8; }

  // If we're completely inside then we do a normal 4 tet insert.
  // Test this first because it's most common.
  if (mask == 15) { return insert_node_in_cell(tets, ci, pi, p); }

  // If the tet is degenerate then we just return any corner and are done.
  else if (mask == 0)
  {
    tets.clear();
    tets.push_back(ci);
    pi = cells_[ci*4 + 0];
    return true;
  }

  // If we're on a corner, we're done.  The corner is the point.
  else if (mask == 1)
  {
    tets.clear();
    tets.push_back(ci);
    pi = cells_[ci*4 + 0];
    return true;
  }
  else if (mask == 2)
  {
    tets.clear();
    tets.push_back(ci);
    pi = cells_[ci*4 + 1];
    return true;
  }
  else if (mask == 4)
  {
    tets.clear();
    tets.push_back(ci);
    pi = cells_[ci*4 + 2];
    return true;
  }
  else if (mask == 8)
  {
    tets.clear();
    tets.push_back(ci);
    pi = cells_[ci*4 + 3];
    return true;
  }

  // If we're on an edge, we do an edge insert.
  else if (mask == 3 || mask == 5 || mask == 6 ||
           mask == 9 || mask == 10 || mask == 12)
  {
    PEdge etmp;
    if      (mask == 3) 
    { 
      /* 0-1 edge */ 
      etmp = PEdge(cells_[ci*4 + 0], cells_[ci*4 + 1]);
    } 
    else if (mask == 5) 
    { 
      /* 0-2 edge */
      etmp = PEdge(cells_[ci*4 + 0], cells_[ci*4 + 2]);
    } 
    else if (mask == 6) 
    { 
      /* 1-2 edge */
      etmp = PEdge(cells_[ci*4 + 1], cells_[ci*4 + 2]);
    } 
    else if (mask == 9) 
    { 
      /* 0-3 edge */
      etmp = PEdge(cells_[ci*4 + 0], cells_[ci*4 + 3]);
    } 
    else if (mask == 10) 
    { 
      /* 1-3 edge */
      etmp = PEdge(cells_[ci*4 + 1], cells_[ci*4 + 3]);
    } 
    else if (mask == 12) 
    { 
      /* 2-3 edge */
      etmp = PEdge(cells_[ci*4 + 2], cells_[ci*4 + 3]);
    } 
    PEdge e = (*edge_table_.find(etmp)).first;

    pi = add_point(p);
    tets.clear();
    for (size_t i = 0; i < e.cells_.size(); i++)
    {
      insert_node_in_edge(tets, pi, e.cells_[i], e);
    }
  }

  // If we're on a face, we do a face insert.
  else if (mask == 7 || mask == 11 || mask == 13 || mask == 14)
  { 
    typename Face::index_type fi;
    PFace ftmp;
    if        (mask == 7)  
    {
      /* on 0 1 2 face */
      ftmp = PFace(cells_[ci*4 + 0], cells_[ci*4 + 1], cells_[ci*4 + 2]);
    } 
    else if (mask == 11) 
    {
      /* on 0 1 3 face */
      ftmp = PFace(cells_[ci*4 + 0], cells_[ci*4 + 1], cells_[ci*4 + 3]);
    } 
    else if (mask == 13) 
    {
      /* on 0 2 3 face */
      ftmp = PFace(cells_[ci*4 + 0], cells_[ci*4 + 2], cells_[ci*4 + 3]);
    } 
    else if (mask == 14) 
    {
      /* on 1 2 3 face */
      ftmp = PFace(cells_[ci*4 + 1], cells_[ci*4 + 2], cells_[ci*4 + 3]);
    }
    PFace f = (*face_table_.find(ftmp)).first;
    typename Cell::index_type nbr_tet =
      (ci == f.cells_[0]) ? f.cells_[1] : f.cells_[0];
    
    pi = add_point(p);
    tets.clear();
    insert_node_in_face(tets, pi, ci, f);
    if (nbr_tet != MESH_NO_NEIGHBOR)
    {
      insert_node_in_face(tets, pi, nbr_tet, f);
    }
  }

  return true;
}


template <class Basis>
void
TetVolMesh<Basis>::orient(typename Cell::index_type ci)
{
  const Point &p0 = point(cells_[ci*4+0]);
  const Point &p1 = point(cells_[ci*4+1]);
  const Point &p2 = point(cells_[ci*4+2]);
  const Point &p3 = point(cells_[ci*4+3]);

  // Unsigned volumex6 of the tet.
  const double sgn = Dot(Cross(p1-p0,p2-p0),p3-p0);
  if (sgn < 0.0)
  {
    typename Node::index_type tmp = cells_[ci*4+0];
    cells_[ci*4+0] = cells_[ci*4+1];
    cells_[ci*4+1] = tmp;
  }
}


#define TETVOLMESH_VERSION 4

template <class Basis>
void
TetVolMesh<Basis>::io(Piostream &stream)
{
  const int version = stream.begin_class(type_name(-1), TETVOLMESH_VERSION);
  Mesh::io(stream);

  SCIRun::Pio(stream, points_);
  SCIRun::Pio_index(stream, cells_);
  if (version == 1)
  {
    vector<unsigned int> neighbors;
    SCIRun::Pio(stream, neighbors);
  }

  if (version >= 3) {
    basis_.io(stream);
  }

  stream.end_class();

  if (stream.reading())
    vmesh_ = CreateVTetVolMesh(this);
}


template <class Basis>
const TypeDescription*
get_type_description(TetVolMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = SCIRun::get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = scinew TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = scinew TypeDescription("TetVolMesh", subs,
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
TetVolMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((TetVolMesh<Basis> *)0);
}


template <class Basis>
const TypeDescription*
TetVolMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((TetVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Node",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
TetVolMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((TetVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Edge",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
TetVolMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((TetVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Face",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
TetVolMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((TetVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Cell",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


} // namespace SCIRun

#endif
