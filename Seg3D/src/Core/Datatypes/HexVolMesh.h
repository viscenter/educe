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


#ifndef CORE_DATATYPES_HEXVOLMESH
#define CORE_DATATYPES_HEXVOLMESH 1

//! Include what kind of support we want to have
//! Need to fix this and couple it sci-defs
#include <Core/Datatypes/MeshSupport.h>

#include <Core/Containers/StackVector.h>
#include <Core/Containers/LockingHandle.h>
#include <Core/Containers/Handle.h>

#include <Core/Geometry/BBox.h>
#include <Core/Geometry/CompGeom.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Plane.h>
#include <Core/Geometry/Transform.h>
#include <Core/Geometry/Vector.h>

#include <Core/Basis/Locate.h>
#include <Core/Basis/HexTrilinearLgn.h>
#include <Core/Basis/HexTriquadraticLgn.h>
#include <Core/Basis/HexTricubicHmt.h>

#include <Core/Datatypes/FieldIterator.h>
#include <Core/Datatypes/FieldRNG.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/SearchGrid.h>
#include <Core/Datatypes/VMesh.h>

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
template <class Basis>
class HexVolMesh;

//! make sure any other mesh other than the preinstantiate ones
//! returns no virtual interface. Altering this behaviour will allow
//! for dynamically compiling the interfae if needed.
template<class MESH>
VMesh* CreateVHexVolMesh(MESH* mesh) { return (0); }

//! These declarations are needed for a combined dynamic compilation as
//! as well as virtual functions solution.
//! Declare that these can be found in a library that is already
//! precompiled. So dynamic compilation will not instantiate them again.

#if (SCIRUN_HEXVOL_SUPPORT > 0)

SCISHARE VMesh* CreateVHexVolMesh(HexVolMesh<HexTrilinearLgn<Point> >* mesh);
#if (SCIRUN_QUADRATIC_SUPPORT > 0)
SCISHARE VMesh* CreateVHexVolMesh(HexVolMesh<HexTriquadraticLgn<Point> >* mesh);
#endif
#if (SCIRUN_CUBIC_SUPPORT > 0)
SCISHARE VMesh* CreateVHexVolMesh(HexVolMesh<HexTricubicHmt<Point> >* mesh);
#endif

#endif
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////
// Declarations for HexVolMesh class

template <class Basis>
class HexVolMesh : public Mesh
{
  //! Make sure the virtual interface has access
  template <class MESH> friend class VHexVolMesh;
  template <class MESH> friend class VMeshShared;
  template <class MESH> friend class VUnstructuredMesh;

public:
  typedef LockingHandle<HexVolMesh<Basis> > handle_type;
  typedef Basis                         basis_type;

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
    typedef typename HexVolMesh<Basis>::index_type  index_type;
  
    ElemData(const HexVolMesh<Basis>& msh, const index_type ind) :
      mesh_(msh),
      index_(ind)
    {
      //Linear and Constant Basis never use edges_
      if (basis_type::polynomial_order() > 1) {
        mesh_.get_edges_from_cell(edges_, index_);
      }
    }

    // the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const {
      return mesh_.cells_[index_ * 8];
    }
    inline
    index_type node1_index() const {
      return mesh_.cells_[index_ * 8 + 1];
    }
    inline
    index_type node2_index() const {
      return mesh_.cells_[index_ * 8 + 2];
    }
    inline
    index_type node3_index() const {
      return mesh_.cells_[index_ * 8 + 3];
    }
    inline
    index_type node4_index() const {
      return mesh_.cells_[index_ * 8 + 4];
    }
    inline
    index_type node5_index() const {
      return mesh_.cells_[index_ * 8 + 5];
    }
    inline
    index_type node6_index() const {
      return mesh_.cells_[index_ * 8 + 6];
    }
    inline
    index_type node7_index() const {
      return mesh_.cells_[index_ * 8 + 7];
    }

    inline
    index_type edge0_index() const {
      return edges_[0];
    }
    inline
    index_type edge1_index() const {
      return edges_[1];
    }
    inline
    index_type edge2_index() const {
      return edges_[2];
    }
    inline
    index_type edge3_index() const {
      return edges_[3];
    }
    inline
    index_type edge4_index() const {
      return edges_[4];
    }
    inline
    index_type edge5_index() const {
      return edges_[5];
    }
    inline
    index_type edge6_index() const {
      return edges_[6];
    }
    inline
    index_type edge7_index() const {
      return edges_[7];
    }
    inline
    index_type edge8_index() const {
      return edges_[8];
    }
    inline
    index_type edge9_index() const {
      return edges_[9];
    }
    inline
    index_type edge10_index() const {
      return edges_[10];
    }
    inline
    index_type edge11_index() const {
      return edges_[11];
    }

    inline
    const Point &node0() const {
      return mesh_.points_[node0_index()];
    }
    inline
    const Point &node1() const {
      return mesh_.points_[node1_index()];
    }
    inline
    const Point &node2() const {
      return mesh_.points_[node2_index()];
    }
    inline
    const Point &node3() const {
      return mesh_.points_[node3_index()];
    }
    inline
    const Point &node4() const {
      return mesh_.points_[node4_index()];
    }
    inline
    const Point &node5() const {
      return mesh_.points_[node5_index()];
    }
    inline
    const Point &node6() const {
      return mesh_.points_[node6_index()];
    }
    inline
    const Point &node7() const {
      return mesh_.points_[node7_index()];
    }

  private:
    //! reference of the mesh
    const HexVolMesh<Basis>          &mesh_;
    //! copy of index
    const index_type                 index_;
    //! need edges for quadratic meshes
    typename Edge::array_type        edges_;
   };


  //////////////////////////////////////////////////////////////////
  
  //! Construct a new mesh
  HexVolMesh();
  
  //! Copy a mesh, needed for detaching the mesh from a field 
  HexVolMesh(const HexVolMesh &copy);
  
  //! Clone function for detaching the mesh and automatically generating
  //! a new version if needed.  
  virtual HexVolMesh *clone() { return new HexVolMesh(*this); }
  
  //! Destructor  
  virtual ~HexVolMesh();

  //! Access point to virtual interface
  virtual VMesh* vmesh() { return (vmesh_.get_rep()); }

  //! This one should go at some point, should be reroute throught the
  //! virtual interface
  virtual int basis_order() { return (basis_.polynomial_order()); }

  //! Topological dimension
  virtual int dimensionality() const { return 3; }

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
  virtual bool has_normals() const { return (false); }

  //! Has this mesh face normals
  virtual bool has_face_normals() const { return (true); }

  double get_epsilon() const { return (epsilon_); }

  //! Compute tables for doing topology, these need to be synchronized
  //! before doing a lot of operations.
  virtual bool synchronize(mask_type sync);
  virtual bool unsynchronize(mask_type sync);

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
    { ASSERTFAIL("HexVolMesh: get_edges has not been implemented for nodes"); }
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
    { ASSERTFAIL("HexVolMesh: get_faces has not been implemented for nodes"); }
  void get_faces(typename Face::array_type &array,
                 typename Edge::index_type idx) const
    { ASSERTFAIL("HexVolMesh: get_faces has not been implemented for edges"); }
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
    { ASSERTFAIL("HexVolMesh: get_faces has not been implemented for nodes"); }
  void get_delems(typename DElem::array_type &array,
                  typename Edge::index_type idx) const
    { ASSERTFAIL("HexVolMesh: get_faces has not been implemented for edges"); }
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
    { get_node_center(result,idx); }
  void get_center(Point &result, typename Edge::index_type idx) const
    { get_edge_center(result,idx); }
  void get_center(Point &result, typename Face::index_type idx) const
    { get_face_center(result,idx); }
  void get_center(Point &result, typename Cell::index_type idx) const  
    { get_cell_center(result,idx); }
  
  //! Get the size of an elemnt (length, area, volume)
  double get_size(typename Node::index_type idx) const 
    { return 0.0; }
  double get_size(typename Edge::index_type idx) const;
  double get_size(typename Face::index_type idx) const;
  double get_size(typename Cell::index_type idx) const;

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
  
  //! These are more general neighbor functions
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
  bool locate(typename Cell::index_type &loc, const Point &p) const;

  //! These should become obsolete soon, they do not follow the concept
  //! of the basis functions....
  int get_weights(const Point &p, typename Node::array_type &l, double *w) const;
  int get_weights(const Point &p, typename Edge::array_type &l, double *w ) const
    { ASSERTFAIL("HexVolMesh: get_weights for edges isn't supported"); }
  int get_weights(const Point &p, typename Face::array_type &l, double *w ) const
    { ASSERTFAIL("HexVolMesh: get_weights for faces isn't supported"); }
  int get_weights(const Point &p, typename Cell::array_type &l, double *w) const;

  //! Access the nodes of the mesh
  void get_point(Point &result, typename Node::index_type index) const
    { result = points_[index]; }
  void set_point(const Point &point, typename Node::index_type index)
    { points_[index] = point; }
  void get_random_point(Point &p, typename Elem::index_type i, FieldRNG &r) const;

  //! Normals for visualizations
  void get_normal(Vector &result, typename Node::index_type index) const
    { ASSERTFAIL("HexVolMesh: this mesh type does not have node normals."); }
  
  
  //! Get the normals at the outside of the element
  template<class VECTOR, class INDEX1, class INDEX2>
  void get_normal(Vector &result, VECTOR &coords, 
            INDEX1 eidx, INDEX2 fidx) const
    {
      // Improved algorithm, which should be faster as it is fully
      // on the stack.
      unsigned int fmap[] = {0, 5, 1, 3, 4, 2};
      unsigned int face = fmap[fidx];
      
      // Obtain the inverse jacobian
      double Ji[9];
      inverse_jacobian(coords,eidx,Ji);
      
      // Get the normal in local coordinates
      const double un0 = basis_.unit_face_normals[face][0];
      const double un1 = basis_.unit_face_normals[face][1];
      const double un2 = basis_.unit_face_normals[face][2];
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
    ASSERTMSG(a.size() == 8, "Tried to add non-hex element.");
    cells_.push_back(static_cast<typename Node::index_type>(a[0]));
    cells_.push_back(static_cast<typename Node::index_type>(a[1]));
    cells_.push_back(static_cast<typename Node::index_type>(a[2]));
    cells_.push_back(static_cast<typename Node::index_type>(a[3]));
    cells_.push_back(static_cast<typename Node::index_type>(a[4]));
    cells_.push_back(static_cast<typename Node::index_type>(a[5]));
    cells_.push_back(static_cast<typename Node::index_type>(a[6]));
    cells_.push_back(static_cast<typename Node::index_type>(a[7]));
    return static_cast<typename Elem::index_type>((cells_.size() >> 3)-1);
  }


  //! Functions to improve memory management. Often one knows how many
  //! nodes/elements one needs, prereserving memory is often possible.  
  void node_reserve(size_type s) { points_.reserve(static_cast<typename vector<Point>::size_type>(s)); }
  void elem_reserve(size_type s) { cells_.reserve(static_cast<typename vector<index_type>::size_type>(s*8)); }
  void resize_nodes(size_type s) { points_.resize(static_cast<typename vector<Point>::size_type>(s)); }
  void resize_elems(size_type s) { cells_.resize(static_cast<typename vector<index_type>::size_type>(s*8)); }
  

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
    StackVector<Point,3> Jv;
    ElemData ed(*this,idx);
    basis_.derivate(coords,ed,Jv);
    return (DetMatrix3P(Jv));
  }

  //! Get the jacobian of the transformation. In case one wants the non inverted
  //! version of this matrix. This is currentl here for completeness of the 
  //! interface
  template<class VECTOR, class INDEX>
  void jacobian(const VECTOR& coords, INDEX idx, double* J) const
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
    double min_jacobian = ScaledDetMatrix3P(Jv);
    
    size_type num_vertices = static_cast<size_type>(basis_.number_of_vertices());
    for (size_type j=0;j < num_vertices;j++)
    {
      basis_.derivate(basis_.unit_vertices[j],ed,Jv);
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
    double min_jacobian = DetMatrix3P(Jv);
    
    size_type num_vertices = static_cast<size_type>(basis_.number_of_vertices());
    for (size_type j=0;j < num_vertices;j++)
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
    ASSERTFAIL("HexVolMesh: Find closest element has not yet been implemented.");
  }

  //! Find the closest elements to a point  
  template<class ARRAY>
  double find_closest_elems(Point &result, ARRAY &faces, const Point &p) const
  {  
    ASSERTFAIL("HexVolMesh: Find closest element has not yet been implemented.");  
  }

  //! Export this class using the old Pio system
  virtual void io(Piostream&);
  
  ///////////////////////////////////////////////////
  // STATIC VARIABLES AND FUNCTIONS
    
  //! These IDs are created as soon as this class will be instantiated
  static PersistentTypeID hexvolmesh_typeid;
  
  //! Core functionality for getting the name of a templated mesh class
  static  const string type_name(int n = -1);

  //! Type description, used for finding names of the mesh class for
  //! dynamic compilation purposes. Soem of this should be obsolete  
  //! Persistent IO
  virtual const TypeDescription *get_type_description() const;
  static const TypeDescription* node_type_description();
  static const TypeDescription* edge_type_description();
  static const TypeDescription* face_type_description();
  static const TypeDescription* cell_type_description();
  static const TypeDescription* elem_type_description()
  { return cell_type_description(); }

  //! This function returns a maker for Pio.
  static Persistent* maker() { return scinew HexVolMesh<Basis>; }
  //! This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return scinew HexVolMesh<Basis>; }


  //////////////////////////////////////////////////////////////////
  // Mesh specific functions (these are not implemented in every mesh)

  //! Trying figure out which face is made out of 4 nodes (?)
  bool get_face(typename Face::index_type &array,
                typename Node::index_type n1, 
                typename Node::index_type n2,
                typename Node::index_type n3, 
                typename Node::index_type n4) const;
                
                
  //! Functions for Cubit
  //! WE SHOULD MAKE THESE GENERAL AND IN EVERY MESHTYPE
  template <class Iter, class Functor>
  void fill_points(Iter begin, Iter end, Functor fill_ftor);
  template <class Iter, class Functor>
  void fill_cells(Iter begin, Iter end, Functor fill_ftor);
  template <class Iter, class Functor>
  void fill_neighbors(Iter begin, Iter end, Functor fill_ftor);
  template <class Iter, class Functor>
  void fill_data(Iter begin, Iter end, Functor fill_ftor);
                
  // THIS FUNCTION NEEDS TO BE REVISED AS IT DOES NOT SCALE PROPERLY
  // THE TOLERANCE IS NOT RELATIVE, WHICH IS A PROBLEM.........
  typename Node::index_type add_find_point(const Point &p,
                                           double err = 1.0e-6);
  
  // Short cut for generating an element....                                                        
  void add_hex(typename Node::index_type a, typename Node::index_type b,
               typename Node::index_type c,
               typename Node::index_type d, typename Node::index_type e,
               typename Node::index_type f,
               typename Node::index_type g, typename Node::index_type h);
  void add_hex(const Point &p0, const Point &p1, const Point &p2,
               const Point &p3, const Point &p4, const Point &p5,
               const Point &p6, const Point &p7);
                
  //! only call this if you have modified the geometry, this will delete
  //! extra computed synch data and reset the flags to an unsynchronized state.
  void unsynchronize();

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
      "HexVolMesh: Must call synchronize EDGES_E first");
    array.resize(2);
    PEdge e = edges_[idx];
    array[0] = static_cast<typename ARRAY::value_type>(e.nodes_[0]);
    array[1] = static_cast<typename ARRAY::value_type>(e.nodes_[1]);
  }

  template<class ARRAY, class INDEX>
  inline void get_nodes_from_face(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
      "HexVolMesh: Must call synchronize FACES_E first");
    const PFace &f = faces_[idx];
    array.resize(4);
    array[0] = static_cast<typename ARRAY::value_type>(f.nodes_[0]);
    array[1] = static_cast<typename ARRAY::value_type>(f.nodes_[1]);
    array[2] = static_cast<typename ARRAY::value_type>(f.nodes_[2]);
    array[3] = static_cast<typename ARRAY::value_type>(f.nodes_[3]);
  }

  template<class ARRAY, class INDEX>
  inline void get_nodes_from_cell(ARRAY& array, INDEX idx) const
  {
    array.resize(8);
    const int off = idx * 8;
    array[0] = static_cast<typename ARRAY::value_type>(cells_[off]);
    array[1] = static_cast<typename ARRAY::value_type>(cells_[off + 1]);
    array[2] = static_cast<typename ARRAY::value_type>(cells_[off + 2]);
    array[3] = static_cast<typename ARRAY::value_type>(cells_[off + 3]);
    array[4] = static_cast<typename ARRAY::value_type>(cells_[off + 4]);
    array[5] = static_cast<typename ARRAY::value_type>(cells_[off + 5]);
    array[6] = static_cast<typename ARRAY::value_type>(cells_[off + 6]);
    array[7] = static_cast<typename ARRAY::value_type>(cells_[off + 7]);
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
      "HexVolMesh: Must call synchronize FACES_E and EDGES_E first");

    array.clear();
    array.reserve(4);
    const PFace &f = faces_[idx];
    
    if (f.nodes_[0] != f.nodes_[1])
    {
      PEdge e(f.nodes_[0], f.nodes_[1]);  
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(edge_table_.find(e))).second));
    }
    if (f.nodes_[1] != f.nodes_[2])
    {
      PEdge e(f.nodes_[1], f.nodes_[2]);  
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(edge_table_.find(e))).second));
    }
    if (f.nodes_[2] != f.nodes_[3])
    {
      PEdge e(f.nodes_[2], f.nodes_[3]);  
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(edge_table_.find(e))).second));
    }
    if (f.nodes_[3] != f.nodes_[0])
    {
      PEdge e(f.nodes_[3], f.nodes_[0]);  
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(edge_table_.find(e))).second));
    }
  }

  template<class ARRAY, class INDEX>
  inline void get_edges_from_cell(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
      "HexVolMesh: Must call synchronize EDGES_E first");

    array.clear();
    array.reserve(12);
    const index_type off = idx * 8;
    typename Node::index_type n1,n2;
    
    n1 = cells_[off   ]; n2 = cells_[off + 1];
    if (n1 != n2) { PEdge e(n1,n2); array.push_back(static_cast<typename ARRAY::value_type>((*(edge_table_.find(e))).second)); }
    n1 = cells_[off + 1]; n2 = cells_[off + 2];
    if (n1 != n2) { PEdge e(n1,n2); array.push_back(static_cast<typename ARRAY::value_type>((*(edge_table_.find(e))).second)); }
    n1 = cells_[off + 2]; n2 = cells_[off + 3];
    if (n1 != n2) { PEdge e(n1,n2); array.push_back(static_cast<typename ARRAY::value_type>((*(edge_table_.find(e))).second)); }
    n1 = cells_[off + 3]; n2 = cells_[off   ];
    if (n1 != n2) { PEdge e(n1,n2); array.push_back(static_cast<typename ARRAY::value_type>((*(edge_table_.find(e))).second)); }

    n1 = cells_[off + 4]; n2 = cells_[off + 5];
    if (n1 != n2) { PEdge e(n1,n2); array.push_back(static_cast<typename ARRAY::value_type>((*(edge_table_.find(e))).second)); }
    n1 = cells_[off + 5]; n2 = cells_[off + 6];
    if (n1 != n2) { PEdge e(n1,n2); array.push_back(static_cast<typename ARRAY::value_type>((*(edge_table_.find(e))).second)); }
    n1 = cells_[off + 6]; n2 = cells_[off + 7];
    if (n1 != n2) { PEdge e(n1,n2); array.push_back(static_cast<typename ARRAY::value_type>((*(edge_table_.find(e))).second)); }
    n1 = cells_[off + 7]; n2 = cells_[off + 4];
    if (n1 != n2) { PEdge e(n1,n2); array.push_back(static_cast<typename ARRAY::value_type>((*(edge_table_.find(e))).second)); }

    n1 = cells_[off    ]; n2 = cells_[off + 4];
    if (n1 != n2) { PEdge e(n1,n2); array.push_back(static_cast<typename ARRAY::value_type>((*(edge_table_.find(e))).second)); }
    n1 = cells_[off + 5]; n2 = cells_[off + 1];
    if (n1 != n2) { PEdge e(n1,n2); array.push_back(static_cast<typename ARRAY::value_type>((*(edge_table_.find(e))).second)); }
    n1 = cells_[off + 2]; n2 = cells_[off + 6];
    if (n1 != n2) { PEdge e(n1,n2); array.push_back(static_cast<typename ARRAY::value_type>((*(edge_table_.find(e))).second)); }
    n1 = cells_[off + 7]; n2 = cells_[off + 3];
    if (n1 != n2) { PEdge e(n1,n2); array.push_back(static_cast<typename ARRAY::value_type>((*(edge_table_.find(e))).second)); }
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
                "HexVolMesh: Must call synchronize FACES_E first");

    array.clear();
    array.reserve(8);
    
    const index_type off = idx * 8;
    typename Node::index_type n1,n2,n3,n4;
    
    // Put faces in node ordering from smallest node and then following CW or CCW
    // ordering. Test for degenerate elements. Degenerate faces are only added if they
    // are valid (only two neighboring nodes are equal)
    n1 = cells_[off    ]; n2 = cells_[off + 1]; 
    n3 = cells_[off + 2]; n4 = cells_[off + 3];
    if (order_face_nodes(n1,n2,n3,n4))
    {
      PFace f(n1,n2,n3,n4);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(face_table_.find(f))).second));
    }
    n1 = cells_[off + 7]; n2 = cells_[off + 6]; 
    n3 = cells_[off + 5]; n4 = cells_[off + 4];
    if (order_face_nodes(n1,n2,n3,n4))
    {
      PFace f(n1,n2,n3,n4);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(face_table_.find(f))).second));
    }
    n1 = cells_[off    ]; n2 = cells_[off + 4]; 
    n3 = cells_[off + 5]; n4 = cells_[off + 1];
    if (order_face_nodes(n1,n2,n3,n4))
    {
      PFace f(n1,n2,n3,n4);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(face_table_.find(f))).second));
    }
    n1 = cells_[off + 2]; n2 = cells_[off + 6]; 
    n3 = cells_[off + 7]; n4 = cells_[off + 3];
    if (order_face_nodes(n1,n2,n3,n4))
    {
      PFace f(n1,n2,n3,n4);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(face_table_.find(f))).second));
    }
    n1 = cells_[off + 3]; n2 = cells_[off + 7]; 
    n3 = cells_[off + 4]; n4 = cells_[off    ];
    if (order_face_nodes(n1,n2,n3,n4))
    {
      PFace f(n1,n2,n3,n4);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(face_table_.find(f))).second));
    }
    n1 = cells_[off + 1]; n2 = cells_[off + 5]; 
    n3 = cells_[off + 6]; n4 = cells_[off + 2];
    if (order_face_nodes(n1,n2,n3,n4))
    { 
      PFace f(n1,n2,n3,n4);
      array.push_back(static_cast<typename ARRAY::value_type>(
                                            (*(face_table_.find(f))).second));
    }
  }

  template<class ARRAY, class INDEX>
  inline void get_cells_from_node(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & (Mesh::NODE_NEIGHBORS_E|Mesh::EDGES_E),
      "HexVolMesh: Must call synchronize NODE_NEIGHBORS_E and EDGES_E first");

    vector<typename Node::index_type> neighbors;
    set<typename ARRAY::value_type> unique_cells;
    // Get all the nodes that share an edge with this node
    get_node_neighbors(neighbors, idx);
    // Iterate through all those edges
    for (size_t n = 0; n < neighbors.size(); n++)
    {
      // Get the edge information for the current edge
      typename edge_ht::const_iterator iter =
                  edge_table_.find(PEdge(
                    static_cast<typename Node::index_type>(idx),neighbors[n]));
      ASSERTMSG(iter != edge_table_.end(),
                "Edge not found in HexVolMesh::edge_table_");
      // Insert all cells that share this edge into
      // the unique set of cell indices
      for (size_t c = 0; c < (iter->first).cells_.size(); c++)
        unique_cells.insert(static_cast<typename ARRAY::value_type>(
                                                    (iter->first).cells_[c]));
    }

    // Copy the unique set of cells to our Cells array return argument
    array.resize(unique_cells.size());
    copy(unique_cells.begin(), unique_cells.end(), array.begin());  
  }

  
  template<class ARRAY, class INDEX>
  inline void get_cells_from_edge(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::EDGES_E,
                    "HexVolMesh: Must call synchronize EDGES_E first");
    for (size_t i=0; i<edges_[idx].cells_.size();i++)
      array[i] = static_cast<typename ARRAY::value_type>(edges_[idx].cells_[i]);
  }


  template<class ARRAY, class INDEX>
  inline void get_cells_from_face(ARRAY& array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
                    "HexVolMesh: Must call synchronize FACES_E first");
    if (faces_[idx].cells_[1] == MESH_NO_NEIGHBOR)
    {
      array.resize(1);
      array[0] = static_cast<typename ARRAY::value_type>(faces_[idx].cells_[0]);
    }
    else
    {
      array.resize(2);
      array[0] = static_cast<typename ARRAY::value_type>(faces_[idx].cells_[0]);
      array[1] = static_cast<typename ARRAY::value_type>(faces_[idx].cells_[1]);
    }
  }


 template <class INDEX1, class INDEX2>
 inline bool get_elem_neighbor(INDEX1 &neighbor, INDEX1 elem, INDEX2 delem) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "Must call synchronize FACES_E on HexVolMesh first");

    const PFace &f = faces_[delem];

    if (static_cast<typename Cell::index_type>(elem) == f.cells_[0]) 
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
  
  
  template <class ARRAY,class INDEX1, class INDEX2>
  inline bool get_elem_neighbors(ARRAY &array, INDEX1 elem, INDEX2 delem) const
  {
    ASSERTMSG(synchronized_ & Mesh::FACES_E,
              "Must call synchronize FACES_E on HexVolMesh first");

    const PFace &f = faces_[delem];
    if (elem == static_cast<INDEX1>(f.cells_[0])) 
    {
      array.resize(1);
      array[0] = static_cast<typename ARRAY::value_type>(f.cells_[1]);
    } 
    else 
    {
      array.resize(1);
      array[0] = static_cast<typename ARRAY::value_type>(f.cells_[0]);
    }
    if (array[0] == MESH_NO_NEIGHBOR)
    {
      array.clear();
      return (false);
    }
    return (true);
  }


  template <class ARRAY,class INDEX>
  inline void get_elem_neighbors(ARRAY &array, INDEX elem) const
  {
    typename Face::array_type faces;
    get_faces_from_cell(faces, elem);
    
    array.clear();
    typename Face::array_type::iterator iter = faces.begin();
    while(iter != faces.end()) 
    {
      typename ARRAY::value_type nbor;
      if (get_elem_neighbor(nbor, elem, *iter)) 
      {
        array.push_back(nbor);
      }
      ++iter;
    }
  }


  template <class ARRAY,class INDEX>
  inline void get_node_neighbors(ARRAY &array, INDEX node) const
  {
    ASSERTMSG(synchronized_ & Mesh::NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on HexVolMesh first");
    size_t sz = node_neighbors_[node].size();
    array.resize(sz);
    for (size_t i=0; i< sz; i++)
    {
      array[i] = static_cast<typename ARRAY::value_type>(node_neighbors_[node][i]);
    }
  }


  template <class INDEX>
  inline bool locate_node(INDEX &loc, const Point &p) const
  {
    index_type grid_index;
    double dmin = DBL_MAX;
    
    // Check whether point is within search grid
    if (grid_->lookup_point(grid_index,p))
    {
      typename Node::array_type nodes;
      index_type* start;
      index_type* end;
      
      // check all nodes connected to elements in search grid.
      grid_->lookup(&start,&end,grid_index);
      while(start != end)
      {
        get_nodes_from_elem(nodes,*start);
        for (size_t i = 1; i < nodes.size(); i++)
        {
          const double d = (p - points_[nodes[i]]).length2();
          if (d < dmin)
          {
            dmin = d;
            loc = static_cast<INDEX>(nodes[i]);
          }
        }        
        start++;
      }
      
      // Every point within bounding box range should be checked
      const Point pmin = p-Vector(dmin,dmin,dmin);
      const Point pmax = p+Vector(dmin,dmin,dmin);
      const BBox  bbox(pmin,pmax);
      
      // get all grid elements that could contain nodes closer than the one
      // found so far.
      vector<index_type> grid_indices;      
      grid_->lookup_bbox(grid_indices,bbox);
      for (size_t q=0; q< grid_indices.size();q++)
      {
        if (grid_indices[q] != grid_index)
        {        
          grid_->lookup(&start,&end,grid_index);
          while(start != end)
          {
            get_nodes_from_elem(nodes,*start);
            for (size_t i = 1; i < nodes.size(); i++)
            {
              const double d = (p - points_[nodes[i]]).length2();
              if (d < dmin)
              {
                dmin = d;
                loc = static_cast<INDEX>(nodes[i]);
              }
            }        
            start++;
          }
        }
      }
      return (true);
    }
    else
    {  // do exhaustive search.
      typename Node::iterator ni, nie;
      begin(ni);
      end(nie);
      if (ni == nie) { return (false); }

      double min_dist = (p - points_[*ni]).length2();
      loc = static_cast<INDEX>(*ni);
      ++ni;

      while (ni != nie)
      {
        const double dist = (p - points_[*ni]).length2();
        if (dist < min_dist)
        {
          loc = static_cast<INDEX>(*ni);
          min_dist = dist;
        }
        ++ni;
      }
      return (true);
    }
  }


  template <class INDEX>
  bool locate_edge(INDEX &edge, const Point &p) const
  {
    // TO DO: INCORRECT IMPLEMENTATION, NEED TO FIX THIS FUNCTION
    typename Cell::index_type cell;
    if (locate(cell, p))
    {
      typename Edge::array_type edges;
      get_edges(edges, cell);

      if (edges.size() == 0) { return false; }

      edge = static_cast<INDEX>(edges[0]);
      Point loc;
      get_center(loc, edges[0]);
      double mindist = (p -loc).length2();
      for (size_t i = 0; i < edges.size(); i++)
      {
        get_center(loc, edges[i]);
        const double dist = (p -loc).length2();
        if (dist < mindist)
        {
          edge = static_cast<INDEX>(edges[i]);
          mindist = dist;
        }
      }
      return (true);
    }
    return (false);
  }


  template <class INDEX>
  bool locate_face(INDEX &face, const Point &p) const
  {
    // TO DO: INCORRECT IMPLEMENTATION, NEED TO FIX THIS FUNCTION
    typename Cell::index_type cell;
    if (locate(cell, p))
    {
      typename Face::array_type faces;
      get_faces(faces, cell);

      if (faces.size() == 0) { return (false); }

      face = static_cast<INDEX>(faces[0]);
      Point loc;
      get_center(loc, faces[0]);
      double mindist = (p - loc).length2();
      for (size_t i = 0; i < faces.size(); i++)
      {
        get_center(loc, faces[i]);
        const double dist = (p - loc).length2();
        if (dist < mindist)
        {
          face = static_cast<INDEX>(faces[i]);
          mindist = dist;
        }
      }
      return (true);
    }
    return (false);
  }


  template <class INDEX>
  bool locate_elem(INDEX &cell, const Point &p) const
  {
    if (basis_.polynomial_order() > 1) return elem_locate(cell, *this, p);

    if (cell >= 0 && cell < static_cast<INDEX>(cells_.size()/8) 
        && inside8_p(typename Cell::index_type(cell), p))
    {
      return (true);
    }

    ASSERTMSG(synchronized_ & Mesh::LOCATE_E,
              "Must call synchronize LOCATE_E on HexVolMesh first.");

    if(grid_.get_rep() == 0) return (false);

    index_type *iter, *end;

    if (grid_->lookup(&iter, &end, p))
    {
      while (iter != end)
      {
        if (inside8_p(typename Cell::index_type(*iter), p))
        {
          cell = static_cast<INDEX>(*iter);
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
    Point p1;
    get_node_center(result, arr[0]);
    get_node_center(p1, arr[1]);

    result.asVector() += p1.asVector();
    result.asVector() *= 0.5;
  }


  template <class INDEX>
  inline void get_face_center(Point &p, INDEX idx) const
  {
    //! NEED TO CLEAN UP THIS CODE
    //! NEED TO FILTER OUT DEGENERATE FACES
    typename Node::array_type nodes;
    get_nodes_from_face(nodes, idx);
    ASSERT(nodes.size() == 4);
    typename Node::array_type::iterator nai = nodes.begin();
    get_point(p, *nai);
    ++nai;
    Point pp;
    while (nai != nodes.end())
    {
      get_point(pp, *nai);
      p.asVector() += pp.asVector();
      ++nai;
    }
    p.asVector() *= 0.25;
  }

  template <class INDEX>
  inline void get_cell_center(Point &p, INDEX idx) const
  {
    //! NEED TO CLEAN UP THIS CODE
    //! NEED TO FILTER OUT DEGENERATE ELEMENTS
    typename Node::array_type nodes;
    get_nodes_from_cell(nodes, idx);
    ASSERT(nodes.size() == 8);
    typename Node::array_type::iterator nai = nodes.begin();
    get_point(p, *nai);
    ++nai;
    Point pp;
    while (nai != nodes.end())
    {
      get_point(pp, *nai);
      p.asVector() += pp.asVector();
      ++nai;
    }
    p.asVector() *= 0.125;
  }

  
  //////////////////////////////////////////////////////////////
  // Internal functions that the mesh depends on

protected:

  void compute_edges();
  void compute_faces();
  void compute_node_neighbors();
  void compute_grid();
  void compute_epsilon();
  
  const Point &point(typename Node::index_type i) { return points_[i]; }


  bool inside8_p(typename Cell::index_type i, const Point &p) const;

  //! all the nodes.
  vector<Point>        points_;
  //! each 8 indecies make up a Hex
  vector<under_type>   cells_;

  //! Face information.
  struct PFace {
    typename Node::index_type         nodes_[4];   //! 4 nodes makes a face.
    typename Cell::index_type         cells_[2];   //! 2 cells may have this face is in common.
 
    PFace() {
      nodes_[0] = MESH_NO_NEIGHBOR;
      nodes_[1] = MESH_NO_NEIGHBOR;
      nodes_[2] = MESH_NO_NEIGHBOR;
      nodes_[3] = MESH_NO_NEIGHBOR;
      cells_[0] = MESH_NO_NEIGHBOR;
      cells_[1] = MESH_NO_NEIGHBOR;
    }
    
    // snodes_ must be sorted. See Hash Function below.
    PFace(typename Node::index_type n1, typename Node::index_type n2,
          typename Node::index_type n3, typename Node::index_type n4) {
      cells_[0] = MESH_NO_NEIGHBOR;
      cells_[1] = MESH_NO_NEIGHBOR;
      nodes_[0] = n1;
      nodes_[1] = n2;
      nodes_[2] = n3;
      nodes_[3] = n4;
    }

    bool shared() const { return ((cells_[0] != MESH_NO_NEIGHBOR) &&
                                  (cells_[1] != MESH_NO_NEIGHBOR)); }

    //! true if both have the same nodes (order does not matter)
    bool operator==(const PFace &f) const {
      if (nodes_[2] == nodes_[3])
      {
        return ((nodes_[0] == f.nodes_[0]) &&
                (((nodes_[1]==f.nodes_[1])&&(nodes_[2] == f.nodes_[2]))||
                  ((nodes_[1]==f.nodes_[2])&&(nodes_[2] == f.nodes_[1]))));      
      }
      else
      {
        return (((nodes_[0] == f.nodes_[0])&&(nodes_[2] == f.nodes_[2])) &&
                (((nodes_[1]==f.nodes_[1])&&(nodes_[3] == f.nodes_[3]))||
                  ((nodes_[1]==f.nodes_[3])&&(nodes_[3] == f.nodes_[1]))));
      }
    }

    //! Compares each node.  When a non equal node is found the <
    //! operator is applied.
    bool operator<(const PFace &f) const {
      if (nodes_[2] == nodes_[3])
      {
        if ((nodes_[1] < nodes_[2]) && (f.nodes_[1] < f.nodes_[2]))
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[1] == f.nodes_[1])
              return (nodes_[2] < f.nodes_[2]);
            else
              return (nodes_[1] < f.nodes_[1]);
          else
            return (nodes_[0] < f.nodes_[0]);
        }
        else if ((nodes_[1] < nodes_[2]) && (f.nodes_[1] >= f.nodes_[2]))
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[1] == f.nodes_[2])
              return (nodes_[2] < f.nodes_[1]);
            else
              return (nodes_[1] < f.nodes_[2]);
          else
            return (nodes_[0] < f.nodes_[0]);      
        }
        else if ((nodes_[1] >= nodes_[2]) && (f.nodes_[1] < f.nodes_[2]))
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[2] == f.nodes_[1])
              return (nodes_[1] < f.nodes_[2]);
            else
              return (nodes_[2] < f.nodes_[1]);
          else
            return (nodes_[0] < f.nodes_[0]);        
        }
        else
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[2] == f.nodes_[2])
              return (nodes_[1] < f.nodes_[1]);
            else
              return (nodes_[2] < f.nodes_[2]);
          else
            return (nodes_[0] < f.nodes_[0]);           
        }
      }
      else
      {
        if ((nodes_[1] < nodes_[3]) && (f.nodes_[1] < f.nodes_[3]))
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[1] == f.nodes_[1])
              if (nodes_[2] == f.nodes_[2])
                return (nodes_[3] < f.nodes_[3]);
              else
                return (nodes_[2] < f.nodes_[2]);
            else
              return (nodes_[1] < f.nodes_[1]);
          else
            return (nodes_[0] < f.nodes_[0]);
        }
        else if ((nodes_[1] < nodes_[3]) && (f.nodes_[1] >= f.nodes_[3]))
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[1] == f.nodes_[3])
              if (nodes_[2] == f.nodes_[2])
                return (nodes_[3] < f.nodes_[1]);
              else
                return (nodes_[2] < f.nodes_[2]);
            else
              return (nodes_[1] < f.nodes_[3]);
          else
            return (nodes_[0] < f.nodes_[0]);      
        }
        else if ((nodes_[1] >= nodes_[3]) && (f.nodes_[1] < f.nodes_[3]))
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[3] == f.nodes_[1])
              if (nodes_[2] == f.nodes_[2])
                return (nodes_[1] < f.nodes_[3]);
              else
                return (nodes_[2] < f.nodes_[2]);
            else
              return (nodes_[3] < f.nodes_[1]);
          else
            return (nodes_[0] < f.nodes_[0]);      
        }
        else
        {
          if (nodes_[0] == f.nodes_[0])
            if (nodes_[3] == f.nodes_[3])
              if (nodes_[2] == f.nodes_[2])
                return (nodes_[1] < f.nodes_[1]);
              else
                return (nodes_[2] < f.nodes_[2]);
            else
              return (nodes_[3] < f.nodes_[3]);
          else
            return (nodes_[0] < f.nodes_[0]);      
        }
      }
    }
    
  };


  //! Edge information.
  struct PEdge {
    typename Node::index_type         nodes_[2];   //! 2 nodes makes an edge.
    //! list of all the cells this edge is in.
    vector<typename Cell::index_type> cells_;

    PEdge() : cells_(0) {
      nodes_[0] = MESH_NO_NEIGHBOR;
      nodes_[1] = MESH_NO_NEIGHBOR;
    }
    // node_[0] must be smaller than node_[1]. See Hash Function below.
    PEdge(typename Node::index_type n1,
          typename Node::index_type n2) : cells_(0) {
      if (n1 < n2) {
        nodes_[0] = n1;
        nodes_[1] = n2;
      } else {
        nodes_[0] = n2;
        nodes_[1] = n1;
      }
    }

    bool shared() const { return cells_.size() > 1; }

    //! true if both have the same nodes (order does not matter)
    bool operator==(const PEdge &e) const {
      return ((nodes_[0] == e.nodes_[0]) && (nodes_[1] == e.nodes_[1]));
    }

    //! Compares each node.  When a non equal node is found the <
    //! operator is applied.
    bool operator<(const PEdge &e) const {
      if (nodes_[0] == e.nodes_[0])
        return (nodes_[1] < e.nodes_[1]);
      else
        return (nodes_[0] < e.nodes_[0]);
    }
  };

  /*! hash the egde's node_indecies such that edges with the same nodes
   *  hash to the same value. nodes are sorted on edge construction. */
  static const int sz_int = sizeof(int) * 8; // in bits
  struct FaceHash {
    //! These are needed by the hash_map particularly
    // ANSI C++ allows us to initialize these variables in the
    // declaration.  However there may be compilers which will complain
    // about it.
    static const size_t bucket_size = 4;
    static const size_t min_buckets = 8;

    //! These are for our own use (making the hash function.
    static const int sz_quarter_int = (int)(sz_int * .25);
    static const int top4_mask = ((~((int)0)) << sz_quarter_int << sz_quarter_int << sz_quarter_int);
    static const int up4_mask = top4_mask ^ (~((int)0) << sz_quarter_int << sz_quarter_int);
    static const int mid4_mask =  top4_mask ^ (~((int)0) << sz_quarter_int);
    static const int low4_mask = ~(top4_mask | mid4_mask);

    //! This is the hash function
    size_t operator()(const PFace &f) const {
      if (f.nodes_[1] < f.nodes_[3] )
      {
        return ((f.nodes_[0] << sz_quarter_int << sz_quarter_int <<sz_quarter_int) |
              (up4_mask & (f.nodes_[1] << sz_quarter_int << sz_quarter_int)) |
              (mid4_mask & (f.nodes_[2] << sz_quarter_int)) |
              (low4_mask & f.nodes_[3]));
      }
      else
      {
        return ((f.nodes_[0] << sz_quarter_int << sz_quarter_int <<sz_quarter_int) |
              (up4_mask & (f.nodes_[3] << sz_quarter_int << sz_quarter_int)) |
              (mid4_mask & (f.nodes_[2] << sz_quarter_int)) |
              (low4_mask & f.nodes_[1]));
      }
    }
    //! This should return less than rather than equal to.
    bool operator()(const PFace &f1, const PFace& f2) const {
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
      return (e.nodes_[0] << sz_half_int) | (low_mask & e.nodes_[1]);
    }
    //!  This should return less than rather than equal to.
    bool operator()(const PEdge &e1, const PEdge& e2) const {
      return e1 < e2;
    }
  };

//! Define the hash_map type, as this is not yet an approved type under Windows
//! it is located in stdext

#ifdef HAVE_HASH_MAP
  #if defined(_WIN32)
    //! hash_map is in stdext namespace
    typedef stdext::hash_map<PFace, typename Face::index_type, FaceHash> face_ht;
    typedef stdext::hash_map<PEdge, typename Edge::index_type, EdgeHash> edge_ht;
  #else
    //! hash_map is in std namespace
    typedef hash_map<PFace, typename Face::index_type, FaceHash> face_ht;
    typedef hash_map<PEdge, typename Edge::index_type, EdgeHash> edge_ht;
  #endif
#else
  typedef map<PFace, typename Face::index_type, FaceHash> face_ht;
  typedef map<PEdge, typename Edge::index_type, EdgeHash> edge_ht;
#endif


  /*! container for face storage. Must be computed each time
    nodes or cells change. */
  vector<PFace>            faces_;
  face_ht                  face_table_;
  /*! container for edge storage. Must be computed each time
    nodes or cells change. */
  vector<PEdge>            edges_;
  edge_ht                  edge_table_;



  inline
  void hash_edge(typename Node::index_type n1, typename Node::index_type n2,
                 typename Cell::index_type ci, edge_ht &table) const;

  inline
  void hash_face(typename Node::index_type n1, typename Node::index_type n2,
                 typename Node::index_type n3, typename Node::index_type n4,
                 typename Cell::index_type ci, unsigned int facenumber,
                 face_ht &table) const;

  inline bool order_face_nodes(typename Node::index_type& n1, typename Node::index_type& n2,
                 typename Node::index_type& n3, typename Node::index_type& n4) const;


  //! useful functors
  struct FillNodeNeighbors {
    FillNodeNeighbors(vector<vector<typename Node::index_type> > &n,
                      const HexVolMesh &m) :
      nbor_vec_(n),
      mesh_(m)
    {}

    void operator()(typename Edge::index_type e) {
      nodes_.clear();
      mesh_.get_nodes(nodes_, e);
      nbor_vec_[nodes_[0]].push_back(nodes_[1]);
      nbor_vec_[nodes_[1]].push_back(nodes_[0]);
    }

    vector<vector<typename Node::index_type> > &nbor_vec_;
    const HexVolMesh            &mesh_;
    typename Node::array_type   nodes_;
  };

  vector<vector<typename Node::index_type> > node_neighbors_;

  LockingHandle<SearchGrid>     grid_;

  Mutex                         synchronize_lock_;
  mask_type                     synchronized_;
  Basis                         basis_;
  double                        epsilon_;
  double                        epsilon3_;
  
  //! Pointer to virtual interface  
  Handle<VMesh>                 vmesh_;
};


template <class Basis>
HexVolMesh<Basis>::HexVolMesh() :
  points_(0),
  cells_(0),
  faces_(0),
  face_table_(),
  edges_(0),
  edge_table_(),
  grid_(0),
  synchronize_lock_("HexVolMesh Lock"),
  synchronized_(Mesh::NODES_E | Mesh::CELLS_E),
  epsilon_(0.0),
  epsilon3_(0.0)
{
  //! Initialize the virtual interface when the mesh is created
  vmesh_ = CreateVHexVolMesh(this);
}

template <class Basis>
HexVolMesh<Basis>::HexVolMesh(const HexVolMesh &copy):
  points_(copy.points_),
  cells_(copy.cells_),
  faces_(0),
  face_table_(),
  edges_(0),
  edge_table_(),
  grid_(0),
  synchronize_lock_("HexVolMesh Lock"),
  synchronized_(Mesh::NODES_E | Mesh::CELLS_E),
  epsilon_(0.0),
  epsilon3_(0.0)
{
  //! Ugly construction circumventing const
  HexVolMesh &lcopy = (HexVolMesh &)copy;

  //! We need to lock before we can copy these as these
  //! structures are generate dynamically when they are
  //! needed.
  lcopy.synchronize_lock_.lock();
  
  face_table_ = copy.face_table_;
  faces_ = copy.faces_;
  synchronized_ |= copy.synchronized_ & Mesh::FACES_E;

  edge_table_ = copy.edge_table_;
  edges_ = copy.edges_;
  synchronized_ |= copy.synchronized_ & Mesh::EDGES_E;

  synchronized_ &= ~Mesh::LOCATE_E;
  if (copy.grid_.get_rep())
  {
    grid_ = scinew SearchGrid(*(copy.grid_.get_rep()));
  }
  synchronized_ |= copy.synchronized_ & Mesh::LOCATE_E;

  lcopy.synchronize_lock_.unlock();
  
  //! Create a new virtual interface for this copy
  //! all pointers have changed hence create a new
  //! virtual interface class
  vmesh_ = CreateVHexVolMesh(this);   
  epsilon_ = copy.epsilon_;
  epsilon3_ = epsilon_*epsilon_*epsilon_;
}

template <class Basis>
HexVolMesh<Basis>::~HexVolMesh()
{
}



template <class Basis>
template <class Iter, class Functor>
void
HexVolMesh<Basis>::fill_points(Iter begin, Iter end, Functor fill_ftor)
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
HexVolMesh<Basis>::fill_cells(Iter begin, Iter end, Functor fill_ftor)
{
  synchronize_lock_.lock();
  Iter iter = begin;
  cells_.resize((end - begin) * 8); // resize to the new size
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
    *citer = nodes[4];
    ++citer; ++iter;
    *citer = nodes[5];
    ++citer; ++iter;
    *citer = nodes[6];
    ++citer; ++iter;
    *citer = nodes[7];
    ++citer; ++iter;
  }
  synchronize_lock_.unlock();
}

template <class Basis>
PersistentTypeID
HexVolMesh<Basis>::hexvolmesh_typeid(HexVolMesh<Basis>::type_name(-1), "Mesh",  maker);


template <class Basis>
const string
HexVolMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const string name = type_name(0) + FTNS + type_name(1) + FTNE;
    return name;
  }
  else if (n == 0)
  {
    static const string nm("HexVolMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}




/* To generate a random point inside of a hexrahedron, we generate random
   barrycentric coordinates (independent random variables between 0 and
   1 that sum to 1) for the point. */
template <class Basis>
void
HexVolMesh<Basis>::get_random_point(Point &p,
                                    typename Cell::index_type ei,
                                    FieldRNG &rng) const
{
  const Point &p0 = points_[cells_[ei*8+0]];
  const Point &p1 = points_[cells_[ei*8+1]];
  const Point &p2 = points_[cells_[ei*8+2]];
  const Point &p3 = points_[cells_[ei*8+3]];
  const Point &p4 = points_[cells_[ei*8+4]];
  const Point &p5 = points_[cells_[ei*8+5]];
  const Point &p6 = points_[cells_[ei*8+6]];
  const Point &p7 = points_[cells_[ei*8+7]];

  const double a0 = tetrahedra_volume(p0, p1, p2, p5);
  const double a1 = tetrahedra_volume(p0, p2, p3, p7);
  const double a2 = tetrahedra_volume(p0, p5, p2, p7);
  const double a3 = tetrahedra_volume(p0, p5, p7, p4);
  const double a4 = tetrahedra_volume(p5, p2, p7, p6);

  const double w = rng() * (a0 + a1 + a2 + a3 + a4);
  
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
  
  if (w > (a0 + a1 + a2 + a3))
  {
    p = (p5.vector()*a + p2.vector()*t + p7.vector()*u + p6.vector()*v).point();
  }
  else if (w > (a0 + a1 + a2))
  {
    p = (p0.vector()*a + p5.vector()*t + p7.vector()*u + p4.vector()*v).point();
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
BBox
HexVolMesh<Basis>::get_bounding_box() const
{
  BBox result;

  typename Node::iterator ni, nie;
  begin(ni);
  end(nie);
  while (ni != nie)
  {
    Point p;
    get_point(p, *ni);
    result.extend(p);
    ++ni;
  }
  return result;
}


template <class Basis>
void
HexVolMesh<Basis>::transform(const Transform &t)
{
  vector<Point>::iterator itr = points_.begin();
  vector<Point>::iterator eitr = points_.end();
  while (itr != eitr)
  {
    *itr = t.project(*itr);
    ++itr;
  }

  synchronize_lock_.lock();
  if (grid_.get_rep()) { grid_->transform(t); }
  synchronize_lock_.unlock();
}

template <class Basis>
bool
HexVolMesh<Basis>::order_face_nodes(typename Node::index_type& n1,
                                    typename Node::index_type& n2,
                                    typename Node::index_type& n3,
                                    typename Node::index_type& n4) const
{
  // Check for degenerate or misformed face
  // Opposite faces cannot be equal
  if ((n1 == n3)||(n2==n4)) return (false);

  // Face must have three unique identifiers otherwise it was condition
  // n1==n3 || n2==n4 would be met.
 

  if ((n1 < n2)&&(n1 < n3)&&(n1 < n4))
  {
  }
  else if ((n2 < n3)&&(n2 < n4))
  {
    typename Node::index_type t;
    // shift one position to left
    t = n1; n1 = n2; n2 = n3; n3 = n4; n4 = t;  
  }
  else if (n3 < n4)
  {
    typename Node::index_type t;
    // shift two positions to left
    t = n1; n1 = n3; n3 = t; t = n2; n2 = n4; n4 = t;
  }
  else
  {
    typename Node::index_type t;
    // shift one positions to right
    t = n4; n4 = n3; n3 = n2; n2 = n1; n1 = t;    
  }
       
  if (n1==n2)
  {
    if (n3==n4) return (false); // this is a line not a face
     n2 = n3; n3 = n4; 
  }
  else if (n2 == n3)
  {
    if (n1==n4) return (false); // this is a line not a face
    n3 = n4;
  }
  else if (n4 == n1)
  {
    n4 = n3;
  }
  return (true);
}


template <class Basis>
void
HexVolMesh<Basis>::hash_face(typename Node::index_type n1,
                             typename Node::index_type n2,
                             typename Node::index_type n3,
                             typename Node::index_type n4,
                             typename Cell::index_type ci,
                             unsigned int face_number, 
                             face_ht &table) const
{
  // Reorder nodes while maintaining CCW or CW orientation
  // Check for degenerate faces, if faces has degeneracy it
  // will be ignored (e.g. nodes on opposite corners are equal,
  // or more then two nodes are equal)
   
  if (!(order_face_nodes(n1,n2,n3,n4))) return;
  PFace f(n1, n2, n3, n4);

  typename face_ht::iterator iter = table.find(f);
  if (iter == table.end()) {
    f.cells_[0] = ci;
    table[f] = 0; // insert for the first time
  } else {
    PFace f = (*iter).first;
    if (f.cells_[1] != MESH_NO_NEIGHBOR) {
      cerr << "This Mesh has problems: Cells #"
           << f.cells_[0] << ", #" << f.cells_[1] << ", and #" << ci
           << " are illegally adjacent." << std::endl;
    } else if (f.cells_[0] == ci) {
      cerr << "This Mesh has problems: Cells #"
           << f.cells_[0] << " and #" << ci
           << " are the same." << std::endl;
    } else {
      f.cells_[1] = ci; // add this cell
      table.erase(iter);
      table[f] = 0;
    }
  }
}


template <class Basis>
void
HexVolMesh<Basis>::compute_faces()
{
  face_table_.clear();

  typename Cell::iterator ci, cie;
  begin(ci); end(cie);
  typename Node::array_type arr(8);
  while (ci != cie)
  {
    get_nodes(arr, *ci);
    // 6 faces -- each is entered CCW from outside looking in

    hash_face(arr[0], arr[1], arr[2 ], arr[3], *ci, 0, face_table_);
    hash_face(arr[7], arr[6], arr[5], arr[4], *ci, 1, face_table_);
    hash_face(arr[0], arr[4], arr[5], arr[1], *ci, 2, face_table_);
    hash_face(arr[2], arr[6], arr[7], arr[3], *ci, 3, face_table_);
    hash_face(arr[3], arr[7], arr[4], arr[0], *ci, 4, face_table_);
    hash_face(arr[1], arr[5], arr[6], arr[2], *ci, 5, face_table_);

    ++ci;
  }
  // dump edges into the edges_ container.
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

  synchronized_ |= Mesh::FACES_E;
}


template <class Basis>
void
HexVolMesh<Basis>::hash_edge(typename Node::index_type n1,
                             typename Node::index_type n2,
                             typename Cell::index_type ci,
                             edge_ht &table) const
{
  if (n1 == n2) return;
  PEdge e(n1, n2);
  typename edge_ht::iterator iter = table.find(e);
  if (iter == table.end()) {
    e.cells_.push_back(ci); // add this cell
    table[e] = 0; // insert for the first time
  } else {
    PEdge e = (*iter).first;
    e.cells_.push_back(ci); // add this cell
    table.erase(iter);
    table[e] = 0;
  }
}


template <class Basis>
void
HexVolMesh<Basis>::compute_edges()
{
  typename Cell::iterator ci, cie;
  begin(ci); end(cie);
  typename Node::array_type arr;
  while (ci != cie)
  {
    get_nodes(arr, *ci);
    hash_edge(arr[0], arr[1], *ci, edge_table_);
    hash_edge(arr[1], arr[2], *ci, edge_table_);
    hash_edge(arr[2], arr[3], *ci, edge_table_);
    hash_edge(arr[3], arr[0], *ci, edge_table_);

    hash_edge(arr[4], arr[5], *ci, edge_table_);
    hash_edge(arr[5], arr[6], *ci, edge_table_);
    hash_edge(arr[6], arr[7], *ci, edge_table_);
    hash_edge(arr[7], arr[4], *ci, edge_table_);

    hash_edge(arr[0], arr[4], *ci, edge_table_);
    hash_edge(arr[5], arr[1], *ci, edge_table_);

    hash_edge(arr[2], arr[6], *ci, edge_table_);
    hash_edge(arr[7], arr[3], *ci, edge_table_);
    ++ci;
  }
  // dump edges into the edges_ container.
  edges_.resize(edge_table_.size());
  typename vector<PEdge>::iterator e_iter = edges_.begin();
  typename edge_ht::iterator ht_iter = edge_table_.begin();
  while (ht_iter != edge_table_.end()) {
    *e_iter = (*ht_iter).first;
    (*ht_iter).second = static_cast<typename Edge::index_type>(
                                                      e_iter - edges_.begin());
    ++e_iter; ++ht_iter;
  }

  synchronized_ |= Mesh::EDGES_E;
}


template <class Basis>
bool
HexVolMesh<Basis>::synchronize(mask_type mask)
{
  synchronize_lock_.lock();

  if (mask & Mesh::EDGES_E && !(synchronized_ & Mesh::EDGES_E))
  {
    compute_edges();
  }
  
  if (mask & (Mesh::FACES_E|Mesh::DELEMS_E|Mesh::ELEM_NEIGHBORS_E) && !(synchronized_ & Mesh::FACES_E))
  {
    compute_faces();
  }
  
  if (mask & Mesh::LOCATE_E && !(synchronized_ & Mesh::LOCATE_E))
  {
    if (!(synchronized_ & Mesh::FACES_E)) compute_faces();
    compute_epsilon();
    compute_grid();
  }
  
  if (mask & Mesh::NODE_NEIGHBORS_E && !(synchronized_ & Mesh::NODE_NEIGHBORS_E))
  {
    compute_node_neighbors();
  }
  
  if (mask & Mesh::EPSILON_E && !(synchronized_ & Mesh::EPSILON_E))
  {
    compute_epsilon();
  }

  synchronize_lock_.unlock();
  return true;
}

template<class Basis>
bool
HexVolMesh<Basis>::unsynchronize(mask_type mask)
{
  synchronize_lock_.lock();
  
  if (mask & Mesh::EPSILON_E) synchronized_ &= ~(Mesh::EPSILON_E);
    // No object to synchronize, hence always will succeed
  
  synchronize_lock_.unlock();
  return(true);
}


// THIS ONE SHOULD GO..............
template <class Basis>
void
HexVolMesh<Basis>::unsynchronize()
{
  synchronize_lock_.lock();

  if (synchronized_ & Mesh::NODE_NEIGHBORS_E)
  {
    node_neighbors_.clear();
  }
  if (synchronized_&Mesh::EDGES_E)
  {
    edge_table_.clear();
  }
  if (synchronized_&Mesh::FACES_E)
  {
    face_table_.clear();
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
HexVolMesh<Basis>::begin(typename HexVolMesh::Node::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E,
            "Must call synchronize NODES_E on HexVolMesh first");
  itr = 0;
}


template <class Basis>
void
HexVolMesh<Basis>::end(typename HexVolMesh::Node::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E,
            "Must call synchronize NODES_E on HexVolMesh first");
  itr = static_cast<typename Node::iterator>(points_.size());
}


template <class Basis>
void
HexVolMesh<Basis>::size(typename HexVolMesh::Node::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::NODES_E,
            "Must call synchronize NODES_E on HexVolMesh first");
  s = static_cast<typename Node::size_type>(points_.size());
}


template <class Basis>
void
HexVolMesh<Basis>::begin(typename HexVolMesh::Edge::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on HexVolMesh first");
  itr = 0;
}


template <class Basis>
void
HexVolMesh<Basis>::end(typename HexVolMesh::Edge::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on HexVolMesh first");
  itr = static_cast<typename Edge::iterator>(edges_.size());
}


template <class Basis>
void
HexVolMesh<Basis>::size(typename HexVolMesh::Edge::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::EDGES_E,
            "Must call synchronize EDGES_E on HexVolMesh first");
  s = static_cast<typename Edge::size_type>(edges_.size());
}


template <class Basis>
void
HexVolMesh<Basis>::begin(typename HexVolMesh::Face::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on HexVolMesh first");
  itr = 0;
}


template <class Basis>
void
HexVolMesh<Basis>::end(typename HexVolMesh::Face::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on HexVolMesh first");
  itr = static_cast<typename Face::iterator>(faces_.size());
}


template <class Basis>
void
HexVolMesh<Basis>::size(typename HexVolMesh::Face::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on HexVolMesh first");
  s = static_cast<typename Face::size_type>(faces_.size());
}


template <class Basis>
void
HexVolMesh<Basis>::begin(typename HexVolMesh::Cell::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::CELLS_E,
            "Must call synchronize CELLS_E on HexVolMesh first");
  itr = 0;
}


template <class Basis>
void
HexVolMesh<Basis>::end(typename HexVolMesh::Cell::iterator &itr) const
{
  ASSERTMSG(synchronized_ & Mesh::CELLS_E,
            "Must call synchronize CELLS_E on HexVolMesh first");
  itr = static_cast<typename Cell::iterator>(cells_.size() >> 3);
}


template <class Basis>
void
HexVolMesh<Basis>::size(typename HexVolMesh::Cell::size_type &s) const
{
  ASSERTMSG(synchronized_ & Mesh::CELLS_E,
            "Must call synchronize CELLS_E on HexVolMesh first");
  s = static_cast<typename Cell::size_type>(cells_.size() >> 3);
}


template <class Basis>
bool
HexVolMesh<Basis>::get_face(typename Face::index_type &face,
                            typename Node::index_type n1, 
                            typename Node::index_type n2,
                            typename Node::index_type n3, 
                            typename Node::index_type n4) const
{
  ASSERTMSG(synchronized_ & Mesh::FACES_E,
            "Must call synchronize FACES_E on HexVolMesh first");
  if(!(order_face_nodes(n1,n2,n3,n4))) return (false);
  PFace f(n1, n2, n3, n4);
  typename face_ht::const_iterator fiter = face_table_.find(f);
  if (fiter == face_table_.end()) {
    return false;
  }
  face = (*fiter).second;
  return true;
}


template <class Basis>
void
HexVolMesh<Basis>::compute_node_neighbors()
{
  if (!(synchronized_ & Mesh::EDGES_E)) { compute_edges(); }

  node_neighbors_.clear();
  node_neighbors_.resize(points_.size());
  typename Edge::iterator ei, eie;
  begin(ei); end(eie);
  for_each(ei, eie, FillNodeNeighbors(node_neighbors_, *this));

  synchronized_ |= Mesh::NODE_NEIGHBORS_E;
}




template <class Basis>
double
HexVolMesh<Basis>::get_size(typename Edge::index_type idx) const
{
  typename Node::array_type arr;
  get_nodes(arr, idx);
  Point p0, p1;
  get_center(p0, arr[0]);
  get_center(p1, arr[1]);
  return (p1.asVector() - p0.asVector()).length();
}

template <class Basis>
double
HexVolMesh<Basis>::get_size(typename Face::index_type idx) const
{
  typename Node::array_type ra;
  get_nodes(ra,idx);
  Point p0,p1,p2,p3;
  get_point(p0,ra[0]);
  get_point(p1,ra[1]);
  get_point(p2,ra[2]);
  get_point(p3,ra[3]);
  return ( (Cross(p0-p1,p2-p0)).length()*0.5 + (Cross(p2-p3,p0-p2)).length()*0.5) ;
}

template <class Basis>
double
HexVolMesh<Basis>::get_size(typename Cell::index_type idx) const
{
  const Point &p0 = points_[cells_[idx*8+0]];
  const Point &p1 = points_[cells_[idx*8+1]];
  const Point &p2 = points_[cells_[idx*8+2]];
  const Point &p3 = points_[cells_[idx*8+3]];
  const Point &p4 = points_[cells_[idx*8+4]];
  const Point &p5 = points_[cells_[idx*8+5]];
  const Point &p6 = points_[cells_[idx*8+6]];
  const Point &p7 = points_[cells_[idx*8+7]];

  const double a0 = tetrahedra_volume(p0, p1, p2, p5);
  const double a1 = tetrahedra_volume(p0, p2, p3, p7);
  const double a2 = tetrahedra_volume(p0, p5, p2, p7);
  const double a3 = tetrahedra_volume(p0, p5, p7, p4);
  const double a4 = tetrahedra_volume(p5, p2, p7, p6);
  
  return a0 + a1 + a2 + a3 + a4;
}


template <class Basis>
bool
HexVolMesh<Basis>::locate(typename Node::index_type &loc, const Point &p) const
{
  return (locate_elem(loc,p));
}


template <class Basis>
bool
HexVolMesh<Basis>::locate(typename Edge::index_type &edge, const Point &p) const
{
  return(locate_edge(edge,p));
}


template <class Basis>
bool
HexVolMesh<Basis>::locate(typename Face::index_type &face, const Point &p) const
{
  return(locate_face(face,p));
}



template <class Basis>
bool
HexVolMesh<Basis>::locate(typename Cell::index_type &cell,
			  const Point &p) const
{
  return(locate_elem(cell,p));
}



template <class Basis>
int
HexVolMesh<Basis>::get_weights(const Point &p, typename Node::array_type &l,
                               double *w) const
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
int
HexVolMesh<Basis>::get_weights(const Point &p, typename Cell::array_type &l,
                              double *w) const
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
void
HexVolMesh<Basis>::compute_epsilon()
{
  epsilon_ = get_bounding_box().diagonal().length()*1e-8;
  epsilon3_ = epsilon_*epsilon_*epsilon_;
  synchronized_ |= EPSILON_E;
}


template <class Basis>
void
HexVolMesh<Basis>::compute_grid()
{
  if (grid_.get_rep() != 0) {return; } // only create once.

  BBox bb = get_bounding_box();
  if (!bb.valid()) {return; }

  // Cubed root of number of cells to get a subdivision ballpark.
  const double one_third = 1.L/3.L;
  typename Cell::size_type csize;  size(csize);
  const size_type s = (static_cast<size_type>(ceil(pow((double)csize , one_third)))) / 2 + 1;
  const Vector cell_epsilon = bb.diagonal() * (0.01 / s);
  bb.extend(bb.min() - cell_epsilon*2);
  bb.extend(bb.max() + cell_epsilon*2);

  SearchGridConstructor sgc(s, s, s, bb.min(), bb.max());

  BBox box;
  typename Node::array_type nodes;
  typename Cell::iterator ci, cie;
  begin(ci); end(cie);
  while(ci != cie)
  {
    get_nodes(nodes, *ci);

    box.reset();
    for (size_t i = 0; i < nodes.size(); i++)
    {
      box.extend(points_[nodes[i]]);
    }
    const Point padmin(box.min() - cell_epsilon);
    const Point padmax(box.max() + cell_epsilon);
    box.extend(padmin);
    box.extend(padmax);

    sgc.insert(*ci, box);

    ++ci;
  }

  grid_ = scinew SearchGrid(sgc);

  synchronized_ |= Mesh::LOCATE_E;
}


template <class Basis>
bool
HexVolMesh<Basis>::inside8_p(typename Cell::index_type idx,
                             const Point &p) const
{
  typename Face::array_type faces;
  get_faces(faces, idx);

  Point center;
  get_center(center, idx);

  for (size_t i = 0; i < faces.size(); i++) 
  {
    typename Node::array_type nodes;
    get_nodes(nodes, faces[i]);
    Point p0, p1, p2;
    get_center(p0, nodes[0]);
    get_center(p1, nodes[1]);
    get_center(p2, nodes[2]);

    const Vector v0(p1 - p0), v1(p2 - p0);
    const Vector normal = Cross(v0, v1);
    const Vector off0(p - p0);
    const Vector off1(center - p0);

    double dotprod = Dot(off0, normal);

    // Account for round off - the point may be on the plane!!
    if( fabs(dotprod) < epsilon3_ )
      continue;

    // If orientated correctly the second dot product is not needed.
    // Only need to check to see if the sign is negative.
    if (dotprod * Dot(off1, normal) < 0.0)
      return false;
  }

  return true;
}


template <class Basis>
typename HexVolMesh<Basis>::Node::index_type
HexVolMesh<Basis>::add_find_point(const Point &p, double err)
{
  typename Node::index_type i;
  if (locate(i, p) && ((points_[i] - p).length2() < err))
  {
    return i;
  }
  else
  {
    points_.push_back(p);
    return static_cast<typename Node::index_type>(points_.size() - 1);
  }
}


template <class Basis>
void
HexVolMesh<Basis>::add_hex(typename Node::index_type a,
                           typename Node::index_type b,
                           typename Node::index_type c,
                           typename Node::index_type d,
                           typename Node::index_type e,
                           typename Node::index_type f,
                           typename Node::index_type g,
                           typename Node::index_type h)
{
  cells_.push_back(a);
  cells_.push_back(b);
  cells_.push_back(c);
  cells_.push_back(d);
  cells_.push_back(e);
  cells_.push_back(f);
  cells_.push_back(g);
  cells_.push_back(h);
}


template <class Basis>
typename HexVolMesh<Basis>::Node::index_type
HexVolMesh<Basis>::add_point(const Point &p)
{
  points_.push_back(p);
  return static_cast<typename Node::index_type>(points_.size() - 1);
}



template <class Basis>
void
HexVolMesh<Basis>::add_hex(const Point &p0, const Point &p1,
                           const Point &p2, const Point &p3,
                           const Point &p4, const Point &p5,
                           const Point &p6, const Point &p7)
{
  add_hex(add_find_point(p0), add_find_point(p1),
          add_find_point(p2), add_find_point(p3),
          add_find_point(p4), add_find_point(p5),
          add_find_point(p6), add_find_point(p7));
}


#define HEXVOLMESH_VERSION 5

template <class Basis>
void
HexVolMesh<Basis>::io(Piostream &stream)
{
  const int version = stream.begin_class(type_name(-1), HEXVOLMESH_VERSION);
  Mesh::io(stream);

  SCIRun::Pio(stream, points_);
  SCIRun::Pio_index(stream, cells_);
  if (version == 1)
  {
    // Read this and then purge it
    vector<under_type>  face_neighbors;
    SCIRun::Pio_index(stream, face_neighbors);
  }

  if (version >= 3) {
    basis_.io(stream);
  }

  stream.end_class();

  if (stream.reading())
  {
    synchronized_ = NODES_E | CELLS_E;
  }

  if (stream.reading())
    vmesh_ = CreateVHexVolMesh(this);
}


template <class Basis>
const TypeDescription* get_type_description(HexVolMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *sub = SCIRun::get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = scinew TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = scinew TypeDescription("HexVolMesh", subs,
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
HexVolMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((HexVolMesh<Basis> *)0);
}


template <class Basis>
const TypeDescription*
HexVolMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((HexVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Node",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
HexVolMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((HexVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Edge",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
HexVolMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((HexVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Face",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
HexVolMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((HexVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Cell",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}

} // namespace SCIRun

#endif 
