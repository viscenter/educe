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


#ifndef CORE_DATATYPES_PRISMVOLMESH_H
#define CORE_DATATYPES_PRISMVOLMESH_H 1

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
#include <Core/Basis/PrismLinearLgn.h>
#include <Core/Basis/PrismQuadraticLgn.h>
#include <Core/Basis/PrismCubicHmt.h>

#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/FieldIterator.h>
#include <Core/Datatypes/FieldRNG.h>
#include <Core/Datatypes/SearchGrid.h>

#include <Core/Math/MinMax.h>

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
template <class Basis> class PrismVolMesh;

//! make sure any other mesh other than the preinstantiate ones
//! returns no virtual interface. Altering this behaviour will allow
//! for dynamically compiling the interfae if needed.
template<class MESH>
VMesh* CreateVPrismVolMesh(MESH* mesh) { return (0); }

#if (SCIRUN_PRISMVOL_SUPPORT > 0)

//! These declarations are needed for a combined dynamic compilation as
//! as well as virtual functions solution.
//! Declare that these can be found in a library that is already
//! precompiled. So dynamic compilation will not instantiate them again.
SCISHARE VMesh* CreateVPrismVolMesh(PrismVolMesh<PrismLinearLgn<Point> >* mesh);
#if (SCIRUN_QUADRATIC_SUPPORT > 0)
SCISHARE VMesh* CreateVPrismVolMesh(PrismVolMesh<PrismQuadraticLgn<Point> >* mesh);
#endif
#if (SCIRUN_CUBIC_SUPPORT > 0)
SCISHARE VMesh* CreateVPrismVolMesh(PrismVolMesh<PrismCubicHmt<Point> >* mesh);
#endif

#endif
/////////////////////////////////////////////////////




#define TRI_NNODES 3
#define QUAD_NNODES 4

#define PRISM_NNODES 6
#define PRISM_NEDGES 9
#define PRISM_NFACES 5
#define PRISM_NTRIS  2
#define PRISM_NQUADS 3

#define isTRI(i)  ((i) < PRISM_NTRIS)
#define isQUAD(i) ( PRISM_NTRIS <= (i) && (i) < PRISM_NTRIS + PRISM_NQUADS)



static const unsigned int
PrismFaceTable[PRISM_NFACES][4] = { { 0, 1, 2, 6 },
                                    { 5, 4, 3, 6 },
                                    { 4, 5, 2, 1 },
                                    { 3, 4, 1, 0 },
                                    { 0, 2, 5, 3 } };

static const unsigned int
PrismNodeNeighborTable[PRISM_NNODES][3] = { { 1,2,3 },
                                            { 0,2,4 },
                                            { 0,1,5 },
                                            { 0,4,5 },
                                            { 1,3,5 },
                                            { 2,3,4 } };
template <class Basis>
class PrismVolMesh : public Mesh
{

//! Make sure the virtual interface has access
template<class MESH> friend class VPrismVolMesh;
template<class MESH> friend class VMeshShared;
template<class MESH> friend class VUnstructuredMesh;

public:
  // Types that change depending on 32 or 64bits
  typedef SCIRun::index_type                under_type;
  typedef SCIRun::index_type                index_type;
  typedef SCIRun::size_type                 size_type;
  typedef SCIRun::mask_type                 mask_type;

  typedef LockingHandle<PrismVolMesh<Basis> > handle_type;
  typedef Basis                               basis_type;

  //! Index and Iterator types required for Mesh Concept.
  struct Node {
    typedef NodeIndex<under_type>       index_type;
    typedef NodeIterator<under_type>    iterator;
    typedef NodeIndex<under_type>       size_type;
    typedef vector<index_type>          array_type;
  };

  struct Cell {
    typedef CellIndex<under_type>       index_type;
    typedef CellIterator<under_type>    iterator;
    typedef CellIndex<under_type>       size_type;
    typedef vector<index_type>          array_type;
  };

  // Used for hashing operations below
  static const int sizeof_uint = sizeof(unsigned int) * 8; // in bits

  //! An edge is indexed via the cells structure.
  //! There are 9 unique edges in each cell of 6 nodes.
  //! Therefore, the edge index / 9 == cell index
  //! And, the edge index % 9 == which edge in that cell
  //! Edges indices are stored in a hash_set and a hash_multiset.
  //! The hash_set stores shared edges only once.
  //! The hash_multiset stores all shared edges together.
   struct Edge 
   {
    typedef EdgeIndex<under_type>       index_type;

    //! edgei return the two nodes make the edge
    static pair<typename Node::index_type,
                typename Node::index_type> edgei(index_type idx)
    {
      const index_type base = (idx / PRISM_NEDGES) * PRISM_NNODES;
      switch (idx % PRISM_NEDGES)
      {
      default:
      case 0: return pair<typename Node::index_type,
                          typename Node::index_type>(base+0,base+1);
      case 1: return pair<typename Node::index_type,
                          typename Node::index_type>(base+0,base+2);
      case 2: return pair<typename Node::index_type,
                          typename Node::index_type>(base+0,base+3);
      case 3: return pair<typename Node::index_type,
                          typename Node::index_type>(base+1,base+2);
      case 4: return pair<typename Node::index_type,
                          typename Node::index_type>(base+1,base+4);
      case 5: return pair<typename Node::index_type,
                          typename Node::index_type>(base+2,base+5);
      case 6: return pair<typename Node::index_type,
                          typename Node::index_type>(base+3,base+4);
      case 7: return pair<typename Node::index_type,
                          typename Node::index_type>(base+3,base+5);
      case 8: return pair<typename Node::index_type,
                          typename Node::index_type>(base+4,base+5);
      }
    }

    //! A fucntor that returns a boolean indicating weather two
    //! edges indices share the same nodes, and thus the same edge in space
    //! Used as a template parameter to STL containers typedef'd below
    struct eqEdge : public binary_function<index_type, index_type, bool>
    {
    private:
      const vector<under_type> &cells_;
    public:
      eqEdge(const vector<under_type> &cells) :
        cells_(cells) {};

      // Since the indicies of the nodes can be in any order, we need
      // to order them before we do comparisons.  This can be done
      // simply for two items using Min and Max.  This works, because
      // index_type is a single integral value.
      bool operator()(index_type ei1, index_type ei2) const
      {
        const pair<index_type, index_type> e1 = edgei(ei1), e2 = edgei(ei2);
        return (Max(cells_[e1.first], cells_[e1.second]) ==
                Max(cells_[e2.first], cells_[e2.second]) &&
                Min(cells_[e1.first], cells_[e1.second]) ==
                Min(cells_[e2.first], cells_[e2.second]));
      };
    };

    //! A fucntor that returns a boolean indicating weather two
    //! edges indices are less than each other.
    //! Used as a template parameter to STL containers typedef'd below
    struct lessEdge : public binary_function<index_type, index_type, bool>
    {
    private:
      const vector<under_type> &cells_;
    public:
      lessEdge(const vector<under_type> &cells) :
        cells_(cells) {};
      static bool lessthen(const vector<under_type> &cells, index_type ei1, index_type ei2) {
        const pair<index_type, index_type> e1 = edgei(ei1), e2 = edgei(ei2);
        index_type e1min = Min(cells[e1.first], cells[e1.second]);
        index_type e2min = Min(cells[e2.first], cells[e2.second]);
        if (e1min == e2min) {
          index_type e1max = Max(cells[e1.first], cells[e1.second]);
          index_type e2max = Max(cells[e2.first], cells[e2.second]);
          return  e1max < e2max;
        } else {
          return e1min < e2min;
        }
      }

      bool operator()(index_type ei1, index_type ei2) const
      {
        return lessthen(cells_, ei1, ei2);
      };
    };

#if defined(HAVE_HASH_SET)
    //! A functor that hashes an edge index according to the node
    //! indices of that edge
    //! Used as a template parameter to STL hash_[set,map] containers
    struct CellEdgeHasher : public unary_function<size_t, index_type>
    {
    private:
      const vector<under_type> &cells_;
    public:
      CellEdgeHasher(const vector<under_type> &cells) :
        cells_(cells) {};
      static const int size = sizeof_uint / 2; // in bits
      static const int mask = (~(unsigned int)0) >> (sizeof_uint - size);
      size_t operator()(index_type cell) const
      {
        pair<index_type,index_type> e = edgei(cell);
        const int n0 = static_cast<int>(cells_[e.first]) & mask;
        const int n1 = static_cast<int>(cells_[e.second]) & mask;
        return Min(n0, n1) << size | Max(n0, n1);
      }
#if defined(__ECC) || defined(_MSC_VER)

      // These are particularly needed by ICC's hash stuff
      static const size_t bucket_size = 4;
      static const size_t min_buckets = 8;

      // This is a less than function.
      bool operator()(index_type ei1, index_type ei2) const {
        return lessEdge::lessthen(cells_, ei1, ei2);
      }
#endif // endif ifdef __ICC
    };

#if defined(__ECC) || defined(_MSC_VER)
    // The comparator function needs to be a member of CellEdgeHasher
    typedef hash_multiset<index_type, CellEdgeHasher> HalfEdgeSet;
    typedef hash_set<index_type, CellEdgeHasher> EdgeSet;
#else
    typedef eqEdge EdgeComparitor;
    typedef hash_multiset<index_type, CellEdgeHasher, EdgeComparitor> HalfEdgeSet;
#   if defined(__sgi)
    typedef typename hash_multiset<index_type, CellEdgeHasher, EdgeComparitor>::size_type HESsize_type;
    typedef typename hash_multiset<index_type, CellEdgeHasher, EdgeComparitor>::allocator_type HESallocator_type;
#   endif
    typedef hash_set<index_type, CellEdgeHasher, EdgeComparitor> EdgeSet;
#endif // end if defined(__ECC) || defined(_MSC_VER)
#else // ifdef HAVE_HASH_SET
    typedef lessEdge EdgeComparitor;
    typedef multiset<index_type, EdgeComparitor> HalfEdgeSet;
    typedef set<index_type, EdgeComparitor> EdgeSet;
#endif

    //! This iterator will traverse each shared edge once in no
    //! particular order.
    typedef typename EdgeSet::iterator           iterator;
    typedef typename EdgeSet::const_iterator     const_iterator;
    typedef EdgeIndex<under_type>                size_type;
    typedef vector<index_type>                   array_type;
  };


  struct Face
  {
    typedef FaceIndex<under_type> index_type;

    struct eqFace : public binary_function<index_type, index_type, bool>
    {
    private:
      const vector<under_type> &cells_;
    public:
      eqFace(const vector<under_type> &cells) :
        cells_(cells) {};
      bool operator()(index_type fi1, index_type fi2) const
      {
        const under_type f1_offset = fi1 % PRISM_NFACES;
        const under_type f1_base = fi1 / PRISM_NFACES * PRISM_NNODES;

        const under_type f2_offset = fi2 % PRISM_NFACES;
        const under_type f2_base = fi2 / PRISM_NFACES * PRISM_NNODES;

        const under_type f1_n0 =
          cells_[f1_base + PrismFaceTable[f1_offset][0] ];
        const under_type f1_n1 =
          cells_[f1_base + PrismFaceTable[f1_offset][1] ];
        const under_type f1_n2 =
          cells_[f1_base + PrismFaceTable[f1_offset][2] ];

        const under_type f2_n0 =
          cells_[f2_base + PrismFaceTable[f2_offset][0] ];
        const under_type f2_n1 =
          cells_[f2_base + PrismFaceTable[f2_offset][1] ];
        const under_type f2_n2 =
          cells_[f2_base + PrismFaceTable[f2_offset][2] ];

        if( isTRI(f1_offset) && isTRI(f2_offset) ) {
          return (Max(f1_n0, f1_n1, f1_n2) == Max(f2_n0, f2_n1, f2_n2) &&
                  Mid(f1_n0, f1_n1, f1_n2) == Mid(f2_n0, f2_n1, f2_n2) &&
                  Min(f1_n0, f1_n1, f1_n2) == Min(f2_n0, f2_n1, f2_n2));
        } else if( isQUAD(f1_offset) && isQUAD(f2_offset) ) {
          const under_type f1_n3 =
            cells_[f1_base + PrismFaceTable[f1_offset][3] ];
          const under_type f2_n3 =
            cells_[f2_base + PrismFaceTable[f2_offset][3] ];

          return
            (Max( Max(f1_n0, f1_n1), Max(f1_n2, f1_n3)) ==
             Max( Max(f2_n0, f2_n1), Max(f2_n2, f1_n3)) &&

             Max( Mid(f1_n0, f1_n1, f1_n2), Mid(f1_n1, f1_n2, f1_n3)) ==
             Max( Mid(f2_n0, f2_n1, f2_n2), Mid(f2_n1, f2_n2, f2_n3)) &&

             Min( Mid(f1_n0, f1_n1, f1_n2), Mid(f1_n1, f1_n2, f1_n3)) ==
             Min( Mid(f2_n0, f2_n1, f2_n2), Mid(f2_n1, f2_n2, f2_n3)) &&

             Min( Min(f1_n0, f1_n1), Min(f1_n2, f1_n3)) ==
             Min( Min(f2_n0, f2_n1), Min(f2_n2, f1_n3)) );
        } else
          return false;
      }
    };

    struct lessFace : public binary_function<index_type, index_type, bool>
    {
    private:
      const vector<under_type> &cells_;
    public:
      lessFace(const vector<under_type> &cells) :
        cells_(cells) {};

      static bool lessthen(const vector<under_type> &cells, index_type fi1, index_type fi2)
      {
        const under_type f1_offset = fi1 % PRISM_NFACES;
        const under_type f1_base = fi1 / PRISM_NFACES * PRISM_NNODES;

        const under_type f2_offset = fi2 % PRISM_NFACES;
        const under_type f2_base = fi2 / PRISM_NFACES * PRISM_NNODES;

        const under_type f1_n0 =
          cells[f1_base + PrismFaceTable[f1_offset][0] ];
        const under_type f1_n1 =
          cells[f1_base + PrismFaceTable[f1_offset][1] ];
        const under_type f1_n2 =
          cells[f1_base + PrismFaceTable[f1_offset][2] ];

        const under_type f2_n0 =
          cells[f2_base + PrismFaceTable[f2_offset][0] ];
        const under_type f2_n1 =
          cells[f2_base + PrismFaceTable[f2_offset][1] ];
        const under_type f2_n2 =
          cells[f2_base + PrismFaceTable[f2_offset][2] ];

        if( isTRI(f1_offset) && isTRI(f2_offset) ) {
          index_type f1max = Max(f1_n0, f1_n1, f1_n2);
          index_type f2max = Max(f2_n0, f2_n1, f2_n2);
          if (f1max == f2max) {
            index_type f1mid = Mid(f1_n0, f1_n1, f1_n2);
            index_type f2mid = Mid(f2_n0, f2_n1, f2_n2);
            if (f1mid == f2mid)
              return Min(f1_n0, f1_n1, f1_n2) <
                     Min(f2_n0, f2_n1, f2_n2);
            else
              return f1mid < f2mid;
          } else
            return f1max < f2max;
        } else if( isQUAD(f1_offset) && isQUAD(f2_offset) ) {
          const under_type f1_n3 =
            cells[f1_base + PrismFaceTable[f1_offset][3] ];
          const under_type f2_n3 =
            cells[f2_base + PrismFaceTable[f2_offset][3] ];

          index_type f1max1 = Max( Max(f1_n0, f1_n1),
                                   Max(f1_n2, f1_n3));
          index_type f2max1 = Max( Max(f2_n0, f2_n1),
                                   Max(f2_n2, f1_n3));
          if (f1max1 == f2max1) {
            index_type f1max2 = Max( Mid(f1_n0, f1_n1, f1_n2),
                                     Mid(f1_n1, f1_n2, f1_n3));
            index_type f2max2 = Max( Mid(f2_n0, f2_n1, f2_n2),
                                     Mid(f2_n1, f2_n2, f2_n3));
            if (f1max2 == f2max2) {
              index_type f1min1 = Min( Mid(f1_n0, f1_n1, f1_n2),
                                       Mid(f1_n1, f1_n2, f1_n3));
              index_type f2min1 = Min( Mid(f2_n0, f2_n1, f2_n2),
                                       Mid(f2_n1, f2_n2, f2_n3));
              if (f1min1 == f2min1)
                return Min( Min(f1_n0, f1_n1), Min(f1_n2, f1_n3)) <
                       Min( Min(f2_n0, f2_n1), Min(f2_n2, f1_n3));
              else
                return f1min1 < f2min1;
            } else {
              return f1max2 < f2max2;
            }
          } else {
            return f1max1 < f2max1;
          }
        } else if( isQUAD(f1_offset) && isTRI(f2_offset) ) {
          // We need to have a way of differentiating when f1 and f2
          // are not both quads or tris.
          return true;
        } else
          return false;
      }
      bool operator()(index_type fi1, index_type fi2) const
      {
        return lessthen(cells_, fi1, fi2);
      }
    };


#if defined(HAVE_HASH_SET)
    struct CellFaceHasher: public unary_function<size_t, index_type>
    {
    private:
      const vector<under_type> &cells_;
    public:
      CellFaceHasher(const vector<under_type> &cells) :
        cells_(cells) {};
      static const int size = sizeof_uint / 4; // in bits
      static const int mask = (~(unsigned int)0) >> (sizeof_uint - size);
      size_t operator()(index_type idx) const
      {
        const unsigned int offset = idx%PRISM_NFACES;
        const unsigned int base   = idx / PRISM_NFACES * PRISM_NNODES; // base cell index

        const under_type n0 = cells_[base + PrismFaceTable[offset][0]] & mask;
        const under_type n1 = cells_[base + PrismFaceTable[offset][1]] & mask;
        const under_type n2 = cells_[base + PrismFaceTable[offset][2]] & mask;

        if( isTRI(offset) )
          return Min(n0,n1,n2)<<size*2 | Mid(n0,n1,n2)<<size | Max(n0,n1,n2);
        else if( isQUAD(offset) ) {
          const under_type n3 = cells_[base + PrismFaceTable[offset][3]] & mask;

          return
            Min(Min(n0,n1),Min(n2,n3))<<size*3 |

            Min( Mid(n0,n1,n2),Mid(n1,n2,n3))<<size*2 |

            Max( Mid(n0,n1,n2),Mid(n1,n2,n3))<<size |

            Max(Max(n0,n1),Max(n2,n3));
        } else
          return 0;
      }

#if defined(__ECC) || defined(_MSC_VER)

      // These are particularly needed by ICC's hash stuff
      static const size_t bucket_size = 4;
      static const size_t min_buckets = 8;

      // This is a less than function.
      bool operator()(index_type fi1, index_type fi2) const {
        return lessFace::lessthen(cells_, fi1, fi2);
      }
#endif // endif ifdef __ICC

    };
#if defined(__ECC) || defined(_MSC_VER)
    // The comparator function needs to be a member of CellFaceHasher
    typedef hash_multiset<index_type, CellFaceHasher> HalfFaceSet;
    typedef hash_set<index_type, CellFaceHasher> FaceSet;
#else
    typedef eqFace FaceComparitor;
    typedef hash_multiset<index_type, CellFaceHasher,FaceComparitor> HalfFaceSet;
    typedef hash_set<index_type, CellFaceHasher, FaceComparitor> FaceSet;
#   if defined(__sgi)
    typedef typename hash_multiset<index_type, CellFaceHasher, FaceComparitor>::size_type HFSsize_type;
    typedef typename hash_multiset<index_type, CellFaceHasher, FaceComparitor>::allocator_type HFSallocator_type;
#   endif
#endif // end if defined(__ECC) || defined(_MSC_VER)
#else // ifdef HAVE_HASH_SET
    typedef lessFace FaceComparitor;
    typedef multiset<index_type, FaceComparitor> HalfFaceSet;
    typedef set<index_type, FaceComparitor> FaceSet;
#endif



    typedef typename FaceSet::iterator          iterator;
    typedef typename FaceSet::const_iterator    const_iterator;
    typedef FaceIndex<under_type>               size_type;
    typedef vector<index_type>                  array_type;
  };

  enum { ELEMENTS_E = CELLS_E };

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
    typedef typename PrismVolMesh<Basis>::index_type  index_type;

    ElemData(const PrismVolMesh<Basis>& msh,
             const index_type ind) :
      mesh_(msh),
      index_(ind)
    {}

    // the following designed to coordinate with ::get_nodes
    inline
    index_type node0_index() const 
    {
      return mesh_.cells_[index_ * 6];
    }
    inline
    index_type node1_index() const 
    {
      return mesh_.cells_[index_ * 6 + 1];
    }
    inline
    index_type node2_index() const 
    {
      return mesh_.cells_[index_ * 6 + 2];
    }
    inline
    index_type node3_index() const 
    {
      return mesh_.cells_[index_ * 6 + 3];
    }
    inline
    index_type node4_index() const 
    {
      return mesh_.cells_[index_ * 6 + 4];
    }
    inline
    index_type node5_index() const 
    {
      return mesh_.cells_[index_ * 6 + 5];
    }

    // the following designed to coordinate with ::get_edges
    inline
    index_type edge0_index() const 
    {
      return index_ * 6;
    }
    inline
    index_type edge1_index() const 
    {
      return index_ * 6 + 1;
    }
    inline
    index_type edge2_index() const 
    {
      return index_ * 6 + 2;
    }
    inline
    index_type edge3_index() const 
    {
      return index_ * 6 + 3;
    }
    inline
    index_type edge4_index() const 
    {
      return index_ * 6 + 4;
    }
    inline
    index_type edge5_index() const 
    {
      return index_ * 6 + 5;
    }
    inline
    index_type edge6_index() const 
    {
      return index_ * 6 + 6;
    }
    inline
    index_type edge7_index() const 
    {
      return index_ * 6 + 7;
    }
    inline
    index_type edge8_index() const 
    {
      return index_ * 6 + 8;
    }
    inline
    index_type edge9_index() const 
    {
      return index_ * 6 + 9;
    }
    inline
    index_type edge10_index() const 
    {
      return index_ * 6 + 10;
    }
    inline
    index_type edge11_index() const 
    {
      return index_ * 6 + 11;
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
    inline
    const Point &node4() const 
    {
      return mesh_.points_[node4_index()];
    }
    inline
    const Point &node5() const 
    {
      return mesh_.points_[node5_index()];
    }

  private:
    //! reference to the mesh
    const PrismVolMesh<Basis>          &mesh_;
    //! copy of element index
    const index_type                   index_;
   };

  //////////////////////////////////////////////////////////////////
  
  //! Construct a new mesh
  PrismVolMesh();

  
  //! Copy a mesh, needed for detaching the mesh from a field   
  PrismVolMesh(const PrismVolMesh &copy);
  
  //! Clone function for detaching the mesh and automatically generating
  //! a new version if needed.    
  virtual PrismVolMesh *clone() { return new PrismVolMesh(*this); }

  //! Destructor 
  virtual ~PrismVolMesh();

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
  virtual int topology_geometry() const { return (UNSTRUCTURED | IRREGULAR); }

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
  virtual bool synchronize(mask_type mask);
  virtual bool unsynchronize(mask_type mask);
   
  //! Get the basis class.    
  Basis& get_basis() { return basis_; }

  double get_epsilon() const { return (0.0); }

  //! begin/end iterators 
  void begin(typename Node::iterator &) const;
  void begin(typename Edge::iterator &);
  void begin(typename Face::iterator &);
  void begin(typename Edge::const_iterator &) const;
  void begin(typename Face::const_iterator &) const;
  void begin(typename Cell::iterator &) const;

  void end(typename Node::iterator &) const;
  void end(typename Edge::iterator &);
  void end(typename Face::iterator &);
  void end(typename Edge::const_iterator &) const;
  void end(typename Face::const_iterator &) const;
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
    { ASSERTFAIL("PrismVolMesh: get_faces has not been implemented for nodes"); }
  void get_faces(typename Face::array_type &array,
                 typename Edge::index_type idx) const
    { ASSERTFAIL("PrismVolMesh: get_faces has not been implemented for edges"); }
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
    { ASSERTFAIL("PrismVolMesh: get_faces has not been implemented for nodes"); }
  void get_delems(typename DElem::array_type &array,
                  typename Edge::index_type idx) const
    { ASSERTFAIL("PrismVolMesh: get_faces has not been implemented for edges"); }
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
    { get_node_center(result, idx); }
  void get_center(Point &result, typename Edge::index_type idx) const
    { get_edge_center(result, idx); }
  void get_center(Point &result, typename Face::index_type idx) const
    { get_face_center(result, idx); }
  void get_center(Point &result, typename Cell::index_type idx) const
    { get_cell_center(result, idx); }

  //! Get the size of an elemnt (length, area, volume)
  double get_size(typename Node::index_type) const 
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
      StackVector<typename Node::index_type,4> ra;
      get_nodes_from_face(ra,idx);
      const Point &p0 = point(ra[0]);
      const Point &p1 = point(ra[1]);
      const Point &p2 = point(ra[2]);

      if( isTRI( idx % PRISM_NFACES ) )
        return (Cross(p0-p1,p2-p0)).length()*0.5;
      else if( isQUAD( idx % PRISM_NFACES ) ){
        const Point &p3 = point(ra[3]);
        return ((Cross(p0-p1,p2-p0)).length()+
                (Cross(p0-p3,p2-p0)).length())*0.5;

      }
      ASSERTFAIL("Index not TRI or QUAD.");
    }
  double get_size(typename Cell::index_type idx) const
  {
    StackVector<typename Node::index_type,6> ra;
    get_nodes_from_cell(ra,idx);
    const Point &p0 = point(ra[0]);
    const Point &p1 = point(ra[1]);
    const Point &p2 = point(ra[2]);
    const Point &p3 = point(ra[3]);
    const Point &p4 = point(ra[4]);
    const Point &p5 = point(ra[5]);

    return ((Dot(Cross(p1-p0,p2-p0),p4-p0)) +
            (Dot(Cross(p2-p0,p3-p0),p4-p0)) +
            (Dot(Cross(p3-p2,p4-p2),p5-p2)) ) * 0.1666666666666666;
  }
  
  //! More specific names for get_size
  double get_length(typename Edge::index_type idx) const 
    { return get_size(idx); };
  double get_area  (typename Face::index_type idx) const 
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
  int get_weights(const Point & , typename Edge::array_type & , double * )
    { ASSERTFAIL("PrismVolMesh::get_weights for edges isn't supported"); }
  int get_weights(const Point & , typename Face::array_type & , double * )
    { ASSERTFAIL("PrismVolMesh::get_weights for faces isn't supported"); }
  int get_weights(const Point &p, typename Cell::array_type &l, double *w);

   //! Access the nodes of the mesh 
  template<class INDEX>
  void get_point(Point &result, INDEX index) const
    { result = points_[index]; }
  template<class INDEX>
  void set_point(const Point &point, INDEX index)
    { points_[index] = point; }

  void get_random_point(Point &p, const typename Elem::index_type &ei,
                        FieldRNG &rng) const;
  
  //! Function for getting node normals  
  void get_normal(Vector &, typename Node::index_type) const
    { ASSERTFAIL("This mesh type does not have node normals."); }

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
    ASSERTMSG(a.size() == PRISM_NNODES, "Tried to add non-prism element.");

    const index_type idx = static_cast<index_type>(cells_.size()) / PRISM_NNODES;

    cells_.push_back(static_cast<typename Node::index_type>(a[0]));
    cells_.push_back(static_cast<typename Node::index_type>(a[1]));
    cells_.push_back(static_cast<typename Node::index_type>(a[2]));
    cells_.push_back(static_cast<typename Node::index_type>(a[3]));
    cells_.push_back(static_cast<typename Node::index_type>(a[4]));
    cells_.push_back(static_cast<typename Node::index_type>(a[5]));

//    create_cell_node_neighbors(idx);
//    create_cell_edges(idx);
//    create_cell_faces(idx);
//    synchronized_ &= ~LOCATE_E;

    return idx;  
  }

  //! Functions to improve memory management. Often one knows how many
  //! nodes/elements one needs, prereserving memory is often possible. 
  void node_reserve(size_type s) { points_.reserve(static_cast<size_t>(s)); }
  void elem_reserve(size_type s) { cells_.reserve(static_cast<size_t>(s*6)); }
  void resize_nodes(size_type s) { points_.resize(static_cast<size_t>(s)); }
  void resize_elems(size_type s) { cells_.resize(static_cast<size_t>(s*6)); }
  
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
    
    size_t num_vertices = basis_.number_of_vertices();
    for (size_t j=0;j < num_vertices;j++)
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
    ASSERTFAIL("PrismVolMesh: Find closest element has not yet been implemented.");
  }

  //! Find the closest elements to a point  
  template<class ARRAY>
  double find_closest_elems(Point &result, ARRAY &faces, const Point &p) const
  {  
    ASSERTFAIL("PrismVolMesh: Find closest element has not yet been implemented.");  
  }

  double get_double() const
    { return (epsilon_); }

  //! Export this class using the old Pio system
  virtual void io(Piostream&);
  
  ///////////////////////////////////////////////////
  // STATIC VARIABLES AND FUNCTIONS
    
  //! This ID is created as soon as this class will be instantiated
  static PersistentTypeID prismvol_typeid;

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
  static Persistent* maker() { return scinew PrismVolMesh<Basis>; }
  //! This function returns a handle for the virtual interface.
  static MeshHandle mesh_maker() { return scinew PrismVolMesh<Basis>; }

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
  inline void get_nodes_from_edge(ARRAY &array, INDEX idx) const
  {
    array.resize(2);
    pair<typename Edge::index_type, typename Edge::index_type> edge = Edge::edgei(typename Edge::index_type(idx));
    array[0] = static_cast<typename ARRAY::value_type>(cells_[edge.first]);
    array[1] = static_cast<typename ARRAY::value_type>(cells_[edge.second]);
  }


  // Always returns nodes in counter-clockwise order
  template <class ARRAY, class INDEX>
  inline void get_nodes_from_face(ARRAY &array, INDEX idx) const
  {
    // Get the base cell index and the face offset
    const index_type offset = idx%5;
    const index_type base = idx / 5 * 6;

    if( isTRI( offset ) )
      array.resize(3);
    else if( isQUAD( offset ) )
      array.resize(4);

    array[0] = static_cast<typename ARRAY::value_type>(cells_[base+PrismFaceTable[offset][0]]);
    array[1] = static_cast<typename ARRAY::value_type>(cells_[base+PrismFaceTable[offset][1]]);
    array[2] = static_cast<typename ARRAY::value_type>(cells_[base+PrismFaceTable[offset][2]]);

    if( isQUAD( offset ) )
      array[3] = static_cast<typename ARRAY::value_type>(cells_[base+PrismFaceTable[offset][3]]);
  }


  template <class ARRAY, class INDEX>
  inline void get_nodes_from_cell(ARRAY &array, INDEX idx) const
  {
    array.resize(6);
    const index_type base = idx*6;
    for (index_type i=0; i<6; i++ )
      array[i] = static_cast<typename ARRAY::value_type>(cells_[base+i]);
  }

  template <class ARRAY, class INDEX>
  inline void get_nodes_from_elem(ARRAY &array, INDEX idx) const
  {
    get_nodes_from_cell(array,idx);
  }

  template <class ARRAY, class INDEX>
  inline void get_edges_from_face(ARRAY &array, INDEX idx) const
  {
    ASSERTFAIL("Not implemented yet");
  }


  template <class ARRAY, class INDEX>
  inline void get_edges_from_cell(ARRAY &array, INDEX idx) const
  {
    array.resize(9);
    const index_type base = idx * 9;
    for (index_type i=0; i<9; i++)
      array[i] = base + i;
  }


  template <class ARRAY, class INDEX>
  inline void get_edges_from_elem(ARRAY &array, INDEX idx) const
  {
    get_edges_from_cell(array,idx);
  }

  template <class ARRAY, class INDEX>
  inline void get_faces_from_cell(ARRAY &array, INDEX idx) const
  {
    array.resize(6);
    for (index_type i = 0; i < 6; i++)
    {
      array[i] = idx * 6 + i;
    }
  }


  template <class ARRAY, class INDEX>
  inline void get_cells_from_edge(ARRAY &array, INDEX idx) const
  {
    pair<typename Edge::HalfEdgeSet::const_iterator,
      typename Edge::HalfEdgeSet::const_iterator> range =
      all_edges_.equal_range(typename Edge::index_type(idx));

    //! ASSERT that this cell's edges have been computed
    ASSERT(range.first != range.second);

    array.clear();
    while (range.first != range.second) 
    {
      array.push_back(static_cast<typename ARRAY::value_type>((*range.first)/ 9));
      ++range.first;
    }
  }


  template <class ARRAY, class INDEX>
  inline void get_cells_from_face(ARRAY &array, INDEX idx) const
  {
    pair<typename Face::HalfFaceSet::const_iterator,
      typename Face::HalfFaceSet::const_iterator> range =
      all_faces_.equal_range(typename Face::index_type(idx));

    //! ASSERT that this cell's faces have been computed
    ASSERT(range.first != range.second);

    array.clear();
    while (range.first != range.second) 
    {
      array.push_back(static_cast<typename ARRAY::value_type>((*range.first)/6));
      ++range.first;
    }
  }


  template <class ARRAY, class INDEX>
  inline void get_cells_from_node(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on PrismVolMesh first.");
    array.clear();
    for (size_t i=0; i<node_neighbors_[idx].size(); ++i)
      array.push_back(static_cast<typename ARRAY::value_type>(node_neighbors_[idx][i]/6));
  }


  template <class INDEX1, class INDEX2>
  inline bool get_elem_neighbor(INDEX1 &neighbor, INDEX1 from, INDEX2 idx) const
  {
    typename Face::index_type neigh;
    bool ret_val = get_neighbor(neigh, typename Face::index_type(idx));
    neighbor = neigh / 6;
    return ret_val;
  }


  template <class ARRAY, class INDEX1, class INDEX2>
  inline bool get_elem_neighbors(ARRAY &neighbor, INDEX1 from, INDEX2 idx) const
  {
    typename Face::index_type neigh;
    bool ret_val = get_neighbor(neigh, typename Face::index_type(idx));
    if (ret_val)
    {
      neighbor.resize(1);
      neighbor[0] = neigh/6;
    }
    else
    {
      neighbor.resize(0);
    }
    return ret_val;
  }

  template <class ARRAY, class INDEX>
  inline void get_elem_neighbors(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & FACES_E,
              "Must call synchronize ELEM_NEIGHBORS_E on PrismVolMesh first.");
    array.clear();
    typename Face::index_type face;
    const index_type base = idx*6;
    for (index_type i=0; i<6; i++) 
    {
      face.index_ = base + i;
      pair<const typename Face::HalfFaceSet::const_iterator,
           const typename Face::HalfFaceSet::const_iterator> range =
        all_faces_.equal_range(face);
      for (typename Face::HalfFaceSet::const_iterator iter = range.first;
           iter != range.second; ++iter)
        if (*iter != face)
          array.push_back(typename ARRAY::value_type(*iter/6));
    }
  }



  template <class INDEX>
  bool locate_node(INDEX &loc, const Point &p) const
  {
    typename Cell::index_type ci;
    if (locate_elem(ci, p)) {// first try the fast way.
      typename Node::array_type nodes;
      get_nodes(nodes, ci);

      Point ptmp;
      double mindist = DBL_MAX;
      for (int i=0; i<PRISM_NNODES; i++) 
      {
        get_center(ptmp, nodes[i]);
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
        Point c;
        get_center(c, *bi);
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
  bool locate_edge(INDEX &edge, const Point &p) const
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
  bool locate_face(INDEX &face, const Point &p) const
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
  bool locate_elem(INDEX &cell, const Point &p) const
  {
    if (basis_.polynomial_order() > 1) return elem_locate(cell, *this, p);
    // Check last cell found first.  Copy cache to cell first so that we
    // don't care about thread safeness, such that worst case on
    // context switch is that cache is not found.
    if (cell > INDEX(0) && cell < INDEX(cells_.size()/PRISM_NNODES) 
        && inside(typename Cell::index_type(cell), p))
    {
      return (true);
    }

    ASSERT(synchronized_ & LOCATE_E);

    index_type *iter, *end;
    if (grid_->lookup(&iter, &end, p))
    {
      while (iter != end)
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



  template <class ARRAY, class INDEX>
  inline void get_node_neighbors(ARRAY &array, INDEX idx) const
  {
    ASSERTMSG(synchronized_ & NODE_NEIGHBORS_E,
              "Must call synchronize NODE_NEIGHBORS_E on PrismVolMesh first.");
    array.clear();
    set<index_type> inserted;
    for (size_t i=0; i<node_neighbors_[idx].size(); i++) 
    {
      const index_type base =
        node_neighbors_[idx][i]/6*6;
      for (index_type c=base; c<base+6; c++) 
      {
        if (cells_[c] != idx && inserted.find(cells_[c]) == inserted.end()) 
        {
          inserted.insert(cells_[c]);
          array.push_back(typename ARRAY::value_type(cells_[c]));
        }
      }
    }
  }


  template <class INDEX>
  inline void get_node_center(Point &p, INDEX idx) const
  {
    p = points_[idx];
  }


  template <class INDEX>
  inline void get_edge_center(Point &p, INDEX idx) const
  {
    StackVector<typename Node::index_type,2> arr;
    get_nodes_from_edge(arr, idx);
    p = points_[arr[0]];
    const Point &p1 = points_[arr[1]];
    p.asVector() += p1.asVector();
    p.asVector() *= 0.5;
  }


  template <class INDEX>
  inline void get_face_center(Point &p, INDEX idx) const
  {
    StackVector<typename Node::index_type,4> arr;
    get_nodes_from_face(arr, idx);
    p = points_[arr[0]];
    const Point &p1 = points_[arr[1]];
    const Point &p2 = points_[arr[2]];

    p.asVector() += p1.asVector();
    p.asVector() += p2.asVector();
    
    if (arr.size() == 3)
    {
      p.asVector() *= (1.0/3.0);
    }
    else
    {
      const Point &p3 = points_[arr[3]];
      p.asVector() += p3.asVector();
      p.asVector() *= 0.25;
    }
  }


  template <class INDEX>
  inline void get_cell_center(Point &p, INDEX idx) const
  {
    StackVector<typename Node::index_type,6> arr;
    get_nodes_from_cell(arr, idx);
    p = points_[arr[0]];
    const Point &p1 = points_[arr[1]];
    const Point &p2 = points_[arr[2]];
    const Point &p3 = points_[arr[3]];
    const Point &p4 = points_[arr[4]];
    const Point &p5 = points_[arr[5]];

    p.asVector() += p1.asVector();
    p.asVector() += p2.asVector();
    p.asVector() += p3.asVector();
    p.asVector() += p4.asVector();
    p.asVector() += p5.asVector();

    p.asVector() *= (1.0/6.0);
  }


public:

  //////////////////////////////////////////////////////////////////
  // Mesh specific functions (these are not implemented in every mesh)

  void get_center(Point &p, typename Node::array_type& arr) const;
  bool get_neighbor(typename Face::index_type &neighbor,
                                  typename Face::index_type idx) const;


  template <class Iter, class Functor>
  void fill_points(Iter begin, Iter end, Functor fill_ftor);
  template <class Iter, class Functor>
  void fill_cells(Iter begin, Iter end, Functor fill_ftor);


  // flip() and rewind_mesh() aren't defined anywhere... so don't declare it.
  //void flip(typename Cell::index_type, bool recalculate = false);
  //void rewind_mesh();

  double polygon_area(const typename Node::array_type &ni, const Vector N) const;
  void orient(typename Cell::index_type idx);
  bool inside(typename Cell::index_type idx, const Point &p) const;
  
  void                  set_nodes(typename Node::array_type &, typename Cell::index_type);

  typename Node::index_type     add_find_point(const Point &p, double err = 1.0e-3);

  typename Elem::index_type     add_prism(typename Node::index_type a,
                                  typename Node::index_type b,
                                  typename Node::index_type c,
                                  typename Node::index_type d,
                                  typename Node::index_type e,
                                  typename Node::index_type f);
  typename Elem::index_type     add_prism(const Point &p0,
                                  const Point &p1,
                                  const Point &p2,
                                  const Point &p3,
                                  const Point &p4,
                                  const Point &p5);

  //! Subdivision methods
  void                  delete_cells(set<index_type> &to_delete);

  bool                  is_edge(typename Node::index_type n0,
                                typename Node::index_type n1,
                                typename Edge::array_type *edges = 0);

  bool                  is_face(typename Node::index_type n0,
                                typename Node::index_type n1,
                                typename Node::index_type n2,
                                typename Face::array_type *faces = 0);

  bool                  is_face(typename Node::index_type n0,
                                typename Node::index_type n1,
                                typename Node::index_type n2,
                                typename Node::index_type n3,
                                typename Face::array_type *faces = 0);

protected:
  const Point &point(typename Node::index_type idx) const { return points_[idx]; }

  void                  compute_node_neighbors();
  void                  compute_edges();
  void                  compute_faces();
  void                  compute_grid();
  void                  compute_epsilon();

  //! Used to recompute data for individual cells
  void                  create_cell_edges(typename Cell::index_type);
  void                  delete_cell_edges(typename Cell::index_type);
  void                  create_cell_faces(typename Cell::index_type);
  void                  delete_cell_faces(typename Cell::index_type);
  void                  create_cell_node_neighbors(typename Cell::index_type);
  void                  delete_cell_node_neighbors(typename Cell::index_type);

  typename Elem::index_type     mod_prism(typename Cell::index_type cell,
                                  typename Node::index_type a,
                                  typename Node::index_type b,
                                  typename Node::index_type c,
                                  typename Node::index_type d,
                                  typename Node::index_type e,
                                  typename Node::index_type f);


  //////////////////////////////////////////////////////////////
  // Internal functions that the mesh depends on  
  
protected:
  //! all the vertices
  vector<Point>         points_;
  Mutex                 points_lock_;

  //! each 6 indecies make up a prism
  vector<index_type>    cells_;
  Mutex                 cells_lock_;

  typedef LockingHandle<typename Edge::HalfEdgeSet> HalfEdgeSetHandle;
  typedef LockingHandle<typename Edge::EdgeSet> EdgeSetHandle;

#if defined(HAVE_HASH_SET)
  typename Edge::CellEdgeHasher  edge_hasher_;
#if !defined(__ECC) && !defined(_MSC_VER)
  typename Edge::EdgeComparitor  edge_comp_;
#endif
#else // ifdef HAVE_HASH_SET
  typename Edge::EdgeComparitor  edge_comp_;

#endif
  typename Edge::HalfEdgeSet    all_edges_;
#if defined(__digital__) || defined(_AIX) || defined(__ECC)
  mutable
#endif
  typename Edge::EdgeSet edges_;
  Mutex                  edge_lock_;

  typedef LockingHandle<typename Face::HalfFaceSet> HalfFaceSetHandle;
  typedef LockingHandle<typename Face::FaceSet> FaceSetHandle;

#if defined(HAVE_HASH_SET)
  typename Face::CellFaceHasher face_hasher_;
#if !defined(__ECC) && !defined(_MSC_VER)
  typename Face::FaceComparitor  face_comp_;

#endif
#else // ifdef HAVE_HASH_SET
  typename Face::FaceComparitor face_comp_;
#endif

  typename Face::HalfFaceSet    all_faces_;

#if defined(__digital__) || defined(_AIX) || defined(__ECC)
  mutable
#endif
  typename Face::FaceSet faces_;
  Mutex                  face_lock_;

  typedef vector<vector<typename Cell::index_type> > NodeNeighborMap;
  //  typedef LockingHandle<NodeMap> NodeMapHandle;
  NodeNeighborMap       node_neighbors_;
  Mutex                 node_neighbor_lock_;

  //! This grid is used as an acceleration structure to expedite calls
  //!  to locate.  For each cell in the grid, we store a list of which
  //!  prisms overlap that grid cell -- to find the prism which contains a
  //!  point, we simply find which grid cell contains that point, and
  //!  then search just those prisms that overlap that grid cell.
  //!  The grid is only built if synchronize(Mesh::LOCATE_E) is called.
  LockingHandle<SearchGrid>  grid_;
  Mutex                      grid_lock_; // Bad traffic!

  mask_type                  synchronized_;
  Basis                      basis_;

  double                     epsilon_;
  
#if defined(__sgi)
  Edge::HESallocator_type edge_allocator_;
  Face::HFSallocator_type face_allocator_;
#endif

  Handle<VMesh>         vmesh_;


}; // end class PrismVolMesh


template <class Basis>
template <class Iter, class Functor>
void
PrismVolMesh<Basis>::fill_points(Iter begin, Iter end, Functor fill_ftor)
{
  points_lock_.lock();
  Iter iter = begin;
  points_.resize(end - begin); // resize to the new size
  vector<Point>::iterator piter = points_.begin();
  while (iter != end) 
  {
    *piter = fill_ftor(*iter);
    ++piter; ++iter;
  }
  points_lock_.unlock();
  this->dirty_ = true;
}


template <class Basis>
template <class Iter, class Functor>
void
PrismVolMesh<Basis>::fill_cells(Iter begin, Iter end, Functor fill_ftor)
{
  cells_lock_.lock();
  Iter iter = begin;
  cells_.resize((end - begin) * PRISM_NNODES); // resize to the new size
  vector<under_type>::iterator citer = cells_.begin();
  while (iter != end) 
  {
    int *nodes = fill_ftor(*iter); // returns an array of length NNODES

    for( int i=0; i<PRISM_NNODES; i++ ) 
    {
      *citer = nodes[i];
      ++citer;
    }
    ++iter;
  }
  cells_lock_.unlock();
  this->dirty_ = true;
}


template <class Basis>
PersistentTypeID
PrismVolMesh<Basis>::prismvol_typeid(PrismVolMesh<Basis>::type_name(-1), "Mesh",
                             PrismVolMesh<Basis>::maker);

template <class Basis>
const string
PrismVolMesh<Basis>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const string name = type_name(0) + FTNS + type_name(1) + FTNE;
    return name;
  }
  else if (n == 0)
  {
    static const string nm("PrismVolMesh");
    return nm;
  }
  else
  {
    return find_type_name((Basis *)0);
  }
}


template <class Basis>
PrismVolMesh<Basis>::PrismVolMesh() :
  points_(0),
  points_lock_("PrismVolMesh points_ fill lock"),
  cells_(0),
  cells_lock_("PrismVolMesh cells_ fill lock"),

  //! Unique Edges
#if defined(HAVE_HASH_SET)
  edge_hasher_(cells_),
#if defined(__ECC) || defined(_MSC_VER)
  all_edges_(edge_hasher_),
  edges_(edge_hasher_),
#else
  edge_comp_(cells_),
#    if defined(__sgi)
  // The SGI compiler can't figure this out on its own, so we have to help it.
  all_edges_((Edge::HESsize_type)0, edge_hasher_, edge_comp_, edge_allocator_),
  edges_( (Edge::HESsize_type)0, edge_hasher_, edge_comp_, edge_allocator_ ),
#    else
  all_edges_( 0, edge_hasher_, edge_comp_ ),
  edges_( 0, edge_hasher_, edge_comp_ ),
#    endif
#endif // if defined(__ECC) || defined(_MSC_VER)
#else // ifdef HAVE_HASH_SET
  all_edges_(edge_comp_),
  edges_(edge_comp_),
  edge_comp_(cells_),
#endif // ifdef HAVE_HASH_SET

  edge_lock_("PrismVolMesh edges_ fill lock"),

  //! Unique Faces
#if defined(HAVE_HASH_SET)
  face_hasher_(cells_),
#if defined(__ECC) || defined(_MSC_VER)
  all_faces_(face_hasher_),
  faces_(face_hasher_),
#else
  face_comp_(cells_),
#    if defined(__sgi)
  // The SGI compiler can't figure this out on its own, so we have to help it.
  all_faces_((Face::HFSsize_type)0, face_hasher_, face_comp_, face_allocator_),
  faces_( (Face::HFSsize_type)0, face_hasher_, face_comp_, face_allocator_ ),
#    else
  all_faces_( 0, face_hasher_, face_comp_ ),
  faces_( 0, face_hasher_, face_comp_ ),
#    endif
#endif // if defined(__ECC) || defined(_MSC_VER)
#else // ifdef HAVE_HASH_SET
  all_faces_(face_comp_),
  faces_(face_comp_),
  face_comp_(cells_),
#endif // ifdef HAVE_HASH_SET

  face_lock_("PrismVolMesh faces_ fill lock"),

  node_neighbors_(0),
  node_neighbor_lock_("PrismVolMesh node_neighbors_ fill lock"),
  grid_(0),
  grid_lock_("PrismVolMesh grid_ fill lock"),
  synchronized_(CELLS_E | NODES_E),
  epsilon_(0.0)
{
  vmesh_ = CreateVPrismVolMesh(this);
}


template <class Basis>
PrismVolMesh<Basis>::PrismVolMesh(const PrismVolMesh &copy):
  points_(0),
  points_lock_("PrismVolMesh points_ fill lock"),
  cells_(0),
  cells_lock_("PrismVolMesh cells_ fill lock"),
#if defined(HAVE_HASH_SET)
  edge_hasher_(cells_),
#if defined(__ECC) || defined(_MSC_VER)
  all_edges_(edge_hasher_),
  edges_(edge_hasher_),
#else
  edge_comp_(cells_),
#    if defined(__sgi)
  // The SGI compiler can't figure this out on its own, so we have to help it.
  all_edges_((Edge::HESsize_type)0, edge_hasher_, edge_comp_, edge_allocator_),
  edges_( (Edge::HESsize_type)0, edge_hasher_, edge_comp_, edge_allocator_ ),
#    else
  all_edges_( 0, edge_hasher_, edge_comp_ ),
  edges_( 0, edge_hasher_, edge_comp_ ),
#    endif
#endif // if defined(__ECC) || defined(_MSC_VER)
#else // ifdef HAVE_HASH_SET
  all_edges_(edge_comp_),
  edges_(edge_comp_),
  edge_comp_(cells_),
#endif // ifdef HAVE_HASH_SET

  edge_lock_("PrismVolMesh edges_ fill lock"),

#if defined(HAVE_HASH_SET)
  face_hasher_(cells_),
#if defined(__ECC) || defined(_MSC_VER)
  all_faces_(face_hasher_),
  faces_(face_hasher_),
#else
  face_comp_(cells_),
#    if defined(__sgi)
  // The SGI compiler can't figure this out on its own, so we have to help it.
  all_faces_((Face::HFSsize_type)0, face_hasher_, face_comp_, face_allocator_),
  faces_( (Face::HFSsize_type)0, face_hasher_, face_comp_, face_allocator_ ),
#    else
  all_faces_( 0, face_hasher_, face_comp_ ),
  faces_( 0, face_hasher_, face_comp_ ),
#    endif
#endif // if defined(__ECC) || defined(_MSC_VER)
#else // ifdef HAVE_HASH_SET
  all_faces_(face_comp_),
  faces_(face_comp_),
  face_comp_(cells_),
#endif // ifdef HAVE_HASH_SET

  face_lock_("PrismVolMesh edges_ fill lock"),

  node_neighbors_(0),
  node_neighbor_lock_("PrismVolMesh node_neighbors_ fill lock"),
  grid_(0),
  grid_lock_("PrismVolMesh grid_ fill lock"),
  synchronized_(copy.synchronized_),
  epsilon_(copy.epsilon_)
{
  synchronized_ &= ~EDGES_E;
  synchronized_ &= ~FACES_E;

  PrismVolMesh &lcopy = (PrismVolMesh &)copy;

  lcopy.points_lock_.lock();
  points_ = copy.points_;
  lcopy.points_lock_.unlock();

  lcopy.cells_lock_.lock();
  cells_ = copy.cells_;
  lcopy.cells_lock_.unlock();

  lcopy.grid_lock_.lock();

  synchronized_ &= ~LOCATE_E;
  if (copy.grid_.get_rep())
  {
    grid_ = scinew SearchGrid(*(copy.grid_.get_rep()));
  }
  synchronized_ |= copy.synchronized_ & LOCATE_E;
  synchronized_ |= copy.synchronized_ & EPSILON_E;

  lcopy.grid_lock_.unlock();

  //! Create a new virtual interface for this copy
  //! all pointers have changed hence create a new
  //! virtual interface class
  vmesh_ = CreateVPrismVolMesh(this);  
}


template <class Basis>
PrismVolMesh<Basis>::~PrismVolMesh()
{
}


/* To generate a random point inside of a prism, we generate random
   barrycentric coordinates (independent random variables between 0 and
   1 that sum to 1) for the point. */
template <class Basis>
void
PrismVolMesh<Basis>::get_random_point(Point &p,
                                      const typename Elem::index_type &ei,
                                      FieldRNG &rng) const
{
  // TODO: This code looks incorrect.  Should sample cube and fold it,
  // or dice into tets.

  // Get positions of the vertices.
  typename Node::array_type ra;
  get_nodes(ra,ei);

  Vector v = Vector(0,0,0);

  double sum = 0;

  for(unsigned int i=0; i<PRISM_NNODES; i++ ) 
  {
    const Point &p0 = point(ra[i]);
    const double w = rng();

    v += p0.asVector() * w;
    sum += w;
  }

  p = (v / sum).asPoint();
}


template <class Basis>
BBox
PrismVolMesh<Basis>::get_bounding_box() const
{
  BBox result;

  typename Node::iterator ni, nie;
  begin(ni);
  end(nie);
  while (ni != nie) 
  {
    const Point &p = point(*ni);
    result.extend(p);
    ++ni;
  }
  return result;
}


template <class Basis>
void
PrismVolMesh<Basis>::transform(const Transform &t)
{
  vector<Point>::iterator itr = points_.begin();
  vector<Point>::iterator eitr = points_.end();
  while (itr != eitr) 
  {
    *itr = t.project(*itr);
    ++itr;
  }

  grid_lock_.lock();
  if (grid_.get_rep()) { grid_->transform(t); }
  grid_lock_.unlock();
}


template <class Basis>
void
PrismVolMesh<Basis>::compute_faces()
{
  face_lock_.lock();
  if (synchronized_ & FACES_E) 
  {
    face_lock_.unlock();
    return;
  }
  faces_.clear();
  all_faces_.clear();
  size_t num_faces = (cells_.size()) /  PRISM_NNODES * PRISM_NFACES;
  for (size_t i = 0; i < num_faces; i++) 
  {
    faces_.insert(i);
    all_faces_.insert(i);
  }

  synchronized_ |= FACES_E;
  face_lock_.unlock();
}


template <class Basis>
void
PrismVolMesh<Basis>::compute_edges()
{
  edge_lock_.lock();
  if (synchronized_ & EDGES_E) 
  {
    edge_lock_.unlock();
    return;
  }
  edges_.clear();
  all_edges_.clear();
  index_type num_edges = (cells_.size()) / PRISM_NNODES * PRISM_NEDGES;
  for (index_type i = 0; i < num_edges; i++) 
  {
    edges_.insert(i);
    all_edges_.insert(i);
  }
  synchronized_ |= EDGES_E;
  edge_lock_.unlock();
}


template <class Basis>
void
PrismVolMesh<Basis>::compute_node_neighbors()
{
  node_neighbor_lock_.lock();
  if (synchronized_ & NODE_NEIGHBORS_E) 
  {
    node_neighbor_lock_.unlock();
    return;
  }
  node_neighbors_.clear();
  node_neighbors_.resize(points_.size());
  index_type num_cells = static_cast<size_t>(cells_.size());
  for (index_type i = 0; i < num_cells; i++)
    node_neighbors_[cells_[i]].push_back(i);

  synchronized_ |= NODE_NEIGHBORS_E;
  node_neighbor_lock_.unlock();
}


template <class Basis>
bool
PrismVolMesh<Basis>::synchronize(mask_type tosync)
{
  if (tosync & NODE_NEIGHBORS_E && !(synchronized_ & NODE_NEIGHBORS_E))
  {
    compute_node_neighbors();
  }
  
  if (tosync & EDGES_E && !(synchronized_ & EDGES_E))
  {
    compute_edges();
  }
  
  if (tosync & (FACES_E|ELEM_NEIGHBORS_E|DELEMS_E) && !(synchronized_ & FACES_E))
  {
    compute_faces();
  }
  
  if (tosync & LOCATE_E && !(synchronized_ & LOCATE_E))
  {
    compute_grid();
    compute_epsilon();
  }
  if (tosync & Mesh::EPSILON_E && !(synchronized_ & Mesh::EPSILON_E))
  {
    compute_epsilon();
  }    
    
  return true;
}


template <class Basis>
bool
PrismVolMesh<Basis>::unsynchronize(mask_type tosync)
{
  if (tosync & Mesh::EPSILON_E) synchronized_ &= ~(Mesh::EPSILON_E);
    // No object to synchronize, hence always will succeed

  return (true);
}

template <class Basis>
void
PrismVolMesh<Basis>::begin(typename PrismVolMesh::Node::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
PrismVolMesh<Basis>::end(typename PrismVolMesh::Node::iterator &itr) const
{
  itr = points_.size();
}


template <class Basis>
void
PrismVolMesh<Basis>::size(typename PrismVolMesh::Node::size_type &s) const
{
  s = points_.size();
}


template <class Basis>
void
PrismVolMesh<Basis>::begin(typename PrismVolMesh::Edge::iterator &itr)
{
  ASSERTMSG(synchronized_ & EDGES_E, "Must call synchronize on mesh first");
  itr = edges_.begin();
}


template <class Basis>
void
PrismVolMesh<Basis>::end(typename PrismVolMesh::Edge::iterator &itr)
{
  ASSERTMSG(synchronized_ & EDGES_E, "Must call synchronize on mesh first");
  itr = edges_.end();
}

template <class Basis>
void
PrismVolMesh<Basis>::begin(typename PrismVolMesh::Edge::const_iterator &itr) const
{
  ASSERTMSG(synchronized_ & EDGES_E, "Must call synchronize on mesh first");
  itr = edges_.begin();
}


template <class Basis>
void
PrismVolMesh<Basis>::end(typename PrismVolMesh::Edge::const_iterator &itr) const
{
  ASSERTMSG(synchronized_ & EDGES_E, "Must call synchronize on mesh first");
  itr = edges_.end();
}

template <class Basis>
void
PrismVolMesh<Basis>::size(typename PrismVolMesh::Edge::size_type &s) const
{
  ASSERTMSG(synchronized_ & EDGES_E, "Must call synchronize on mesh first");
  s = edges_.size();
}


template <class Basis>
void
PrismVolMesh<Basis>::begin(typename PrismVolMesh::Face::iterator &itr)
{
  ASSERTMSG(synchronized_ & FACES_E, "Must call synchronize on mesh first");
  itr = faces_.begin();
}


template <class Basis>
void
PrismVolMesh<Basis>::end(typename PrismVolMesh::Face::iterator &itr)
{
  ASSERTMSG(synchronized_ & FACES_E, "Must call synchronize on mesh first");
  itr = faces_.end();
}

template <class Basis>
void
PrismVolMesh<Basis>::begin(typename PrismVolMesh::Face::const_iterator &itr) const
{
  ASSERTMSG(synchronized_ & FACES_E, "Must call synchronize on mesh first");
  itr = faces_.begin();
}


template <class Basis>
void
PrismVolMesh<Basis>::end(typename PrismVolMesh::Face::const_iterator &itr) const
{
  ASSERTMSG(synchronized_ & FACES_E, "Must call synchronize on mesh first");
  itr = faces_.end();
}

template <class Basis>
void
PrismVolMesh<Basis>::size(typename PrismVolMesh::Face::size_type &s) const
{
  ASSERTMSG(synchronized_ & FACES_E, "Must call synchronize on mesh first");
  s = faces_.size();
}


template <class Basis>
void
PrismVolMesh<Basis>::begin(typename PrismVolMesh::Cell::iterator &itr) const
{
  itr = 0;
}


template <class Basis>
void
PrismVolMesh<Basis>::end(typename PrismVolMesh::Cell::iterator &itr) const
{
  itr = cells_.size() / 6;
}


template <class Basis>
void
PrismVolMesh<Basis>::size(typename PrismVolMesh::Cell::size_type &s) const
{
  s = cells_.size() / 6;
}


template <class Basis>
void
PrismVolMesh<Basis>::create_cell_edges(typename Cell::index_type idx)
{
  if (!(synchronized_&EDGES_E)) return;
  edge_lock_.lock();
  const index_type base = idx * PRISM_NEDGES;
  for (index_type i=base; i<base+PRISM_NEDGES; ++i)
  {
    edges_.insert(i);
    all_edges_.insert(i);
  }
  edge_lock_.unlock();
}


template <class Basis>
void
PrismVolMesh<Basis>::delete_cell_edges(typename Cell::index_type idx)
{
  if (!(synchronized_&EDGES_E)) return;
  edge_lock_.lock();
  const index_type base = idx * PRISM_NEDGES;
  for ( index_type i=base; i<base+ PRISM_NEDGES; ++i)
  {
    //! If the Shared Edge Set is represented by the particular
    //! cell/edge index that is being recomputed, then
    //! remove it (and insert a non-recomputed edge if any left)
    bool shared_edge_exists = true;
    typename Edge::iterator shared_edge = edges_.find(i);
    // ASSERT guarantees edges were computed correctly for this cell
    ASSERT(shared_edge != edges_.end());
    if ((*shared_edge).index_ == i)
    {
      edges_.erase(shared_edge);
      shared_edge_exists = false;
    }

    typename Edge::HalfEdgeSet::iterator half_edge_to_delete = all_edges_.end();
    pair<typename Edge::HalfEdgeSet::iterator, typename Edge::HalfEdgeSet::iterator> range =
      all_edges_.equal_range(i);
    for (typename Edge::HalfEdgeSet::iterator e = range.first; e != range.second; ++e)
    {
      if ((*e).index_ == i)
      {
        half_edge_to_delete = e;
      }
      else if (!shared_edge_exists)
      {
        edges_.insert((*e).index_);
        shared_edge_exists = true;
      }
      //! At this point, the edges_ set has the new index for this
      //! shared edge and we know what half-edge is getting deleted below
      if (half_edge_to_delete != all_edges_.end() && shared_edge_exists) break;
    }
    //! ASSERT guarantees edges were computed correctly for this cell
    ASSERT(half_edge_to_delete != all_edges_.end());
    all_edges_.erase(half_edge_to_delete);
  }
  edge_lock_.unlock();
}


template <class Basis>
void
PrismVolMesh<Basis>::create_cell_faces(typename Cell::index_type idx)
{
  if (!(synchronized_&FACES_E)) return;
  
  face_lock_.lock();
  const index_type base = idx * PRISM_NFACES;
  for (index_type i=base; i<base+PRISM_NFACES; i++) 
  {
    faces_.insert(i);
    all_faces_.insert(i);
  }
  face_lock_.unlock();
}


template <class Basis>
void
PrismVolMesh<Basis>::delete_cell_faces(typename Cell::index_type idx)
{
  if (!(synchronized_&FACES_E)) return;
  
  face_lock_.lock();
  const index_type base = idx * PRISM_NFACES;
  for ( index_type i=base; i<base+PRISM_NFACES; ++i) 
  {
    // If the Shared Face Set is represented by the particular
    // cell/face index that is being recomputed, then
    // remove it (and insert a non-recomputed shared face if any exist)
    bool shared_face_exists = true;
    typename Face::FaceSet::iterator shared_face = faces_.find(i);
    ASSERT(shared_face != faces_.end());
    if ((*shared_face).index_ == i)
    {
      faces_.erase(shared_face);
      shared_face_exists = false;
    }

    typename Face::HalfFaceSet::iterator half_face_to_delete = all_faces_.end();
    pair<typename Face::HalfFaceSet::iterator, typename Face::HalfFaceSet::iterator> range =
      all_faces_.equal_range(i);
    for (typename Face::HalfFaceSet::iterator e = range.first; e != range.second; ++e) {
      if ((*e).index_ == i)
      {
        half_face_to_delete = e;
      }
      else if (!shared_face_exists) {
        faces_.insert((*e).index_);
        shared_face_exists = true;
      }
      if (half_face_to_delete != all_faces_.end() && shared_face_exists)
        break;
    }

    //! If this ASSERT is reached, it means that the faces
    //! were not computed correctlyfor this cell
    ASSERT(half_face_to_delete != all_faces_.end());
    all_faces_.erase(half_face_to_delete);
  }
  face_lock_.unlock();
}


template <class Basis>
void
PrismVolMesh<Basis>::create_cell_node_neighbors(typename Cell::index_type idx)
{
  if (!(synchronized_ & NODE_NEIGHBORS_E)) return;
  node_neighbor_lock_.lock();

  const index_type base = idx * PRISM_NNODES;

  for (index_type i=base; i<base+PRISM_NNODES; i++)
    node_neighbors_[cells_[i]].push_back(i);

  node_neighbor_lock_.unlock();
}


template <class Basis>
void
PrismVolMesh<Basis>::delete_cell_node_neighbors(typename Cell::index_type idx)
{
  if (!(synchronized_ & NODE_NEIGHBORS_E)) return;
  node_neighbor_lock_.lock();

  const index_type base = idx * PRISM_NNODES;
  for ( index_type i=base; i<base+PRISM_NNODES; ++i)
  {
    const index_type n = cells_[i];

    typename vector<typename Cell::index_type>::iterator node_cells_end =
      node_neighbors_[n].end();

    typename vector<typename Cell::index_type>::iterator cell =
      node_neighbors_[n].begin();

    while (cell != node_cells_end && (*cell) != i)
      ++cell;

    //! ASSERT that the node_neighbors_ structure contains this cell
    ASSERT(cell != node_cells_end);

    node_neighbors_[n].erase(cell);
  }
  node_neighbor_lock_.unlock();
}


//! Given two nodes (n0, n1), return all edge indexes that span those two nodes
template <class Basis>
bool
PrismVolMesh<Basis>::is_edge(typename Node::index_type n0,
                             typename Node::index_type n1,
                             typename Edge::array_type *array)
{
  ASSERTMSG(synchronized_ & EDGES_E,
            "Must call synchronize EDGES_E on PrismVolMesh first.");
  edge_lock_.lock();
  cells_lock_.lock();

  //! Create a phantom cell with edge 0 being the one we're searching for
  const index_type fake_edge = cells_.size() /  PRISM_NNODES *  PRISM_NEDGES;
  vector<under_type>::iterator c0 = cells_.insert(cells_.end(),n0);
  vector<under_type>::iterator c1 = cells_.insert(cells_.end(),n1);

  //! Search the all_edges_ multiset for edges matching our fake_edge
  pair<typename Edge::HalfEdgeSet::iterator, typename Edge::HalfEdgeSet::iterator> range =
    all_edges_.equal_range(fake_edge);

  if (array) 
  {
    array->clear();
    copy(range.first, range.second, array->end());
  }

  //! Delete the pahntom cell
  cells_.erase(c0);
  cells_.erase(c1);

  edge_lock_.unlock();
  cells_lock_.unlock();

  return range.first != range.second;
}


//! Given three nodes (n0, n1, n2), return all face indexes that
//! span those three nodes
template <class Basis>
bool
PrismVolMesh<Basis>::is_face(typename Node::index_type n0,
                             typename Node::index_type n1,
                             typename Node::index_type n2,
                             typename Face::array_type *array)
{
  ASSERTMSG(synchronized_ & FACES_E,
            "Must call synchronize FACES_E on PrismVolMesh first.");
  face_lock_.lock();
  cells_lock_.lock();

  //! Create a phantom cell with face 3 being the one we're searching for
  const unsigned int fake_face = cells_.size() + 3;
  vector<index_type>::iterator c0 = cells_.insert(cells_.end(),n0);
  vector<index_type>::iterator c1 = cells_.insert(cells_.end(),n1);
  vector<index_type>::iterator c2 = cells_.insert(cells_.end(),n2);

  //! Search the all_face_ multiset for edges matching our fake_edge
  pair<typename Face::HalfFaceSet::const_iterator, typename Face::HalfFaceSet::const_iterator>
     range = all_faces_.equal_range(fake_face);

  if (array) 
  {
    array->clear();
    copy(range.first, range.second, array->end());
  }

  //! Delete the phantom cell
  cells_.erase(c0);
  cells_.erase(c1);
  cells_.erase(c2);

  face_lock_.unlock();
  cells_lock_.unlock();
  return range.first != range.second;
}


//! Given four nodes (n0, n1, n2, n3), return all face indexes that
//! span those four nodes
template <class Basis>
bool
PrismVolMesh<Basis>::is_face(typename Node::index_type n0,
                             typename Node::index_type n1,
                             typename Node::index_type n2,
                             typename Node::index_type n3,
                             typename Face::array_type *array)
{
  ASSERTMSG(synchronized_ & FACES_E,
            "Must call synchronize FACES_E on PrismVolMesh first.");
  face_lock_.lock();
  cells_lock_.lock();

  //! Create a phantom cell with face 4 being the one we're searching for
  const unsigned int fake_face = cells_.size() + 4;
  vector<index_type>::iterator c0 = cells_.insert(cells_.end(),n0);
  vector<index_type>::iterator c1 = cells_.insert(cells_.end(),n1);
  vector<index_type>::iterator c2 = cells_.insert(cells_.end(),n2);
  vector<index_type>::iterator c3 = cells_.insert(cells_.end(),n3);

  //! Search the all_face_ multiset for edges matching our fake_edge
  pair<typename Face::HalfFaceSet::const_iterator, typename Face::HalfFaceSet::const_iterator>
     range = all_faces_.equal_range(fake_face);

  if (array) 
  {
    array->clear();
    copy(range.first, range.second, array->end());
  }

  //! Delete the pahntom cell
  cells_.erase(c0);
  cells_.erase(c1);
  cells_.erase(c2);
  cells_.erase(c3);

  face_lock_.unlock();
  cells_lock_.unlock();
  return range.first != range.second;
}


template <class Basis>
void
PrismVolMesh<Basis>::set_nodes(typename Node::array_type &array,
                               typename Cell::index_type idx)
{
  ASSERT(array.size() == PRISM_NNODES);

  delete_cell_edges(idx);
  delete_cell_faces(idx);
  delete_cell_node_neighbors(idx);

  const index_type base = idx * PRISM_NNODES;

  for (index_type i=0; i<PRISM_NNODES; i++)
    cells_[base + i] = array[i];

  synchronized_ &= ~LOCATE_E;
  create_cell_edges(idx);
  create_cell_faces(idx);
  create_cell_node_neighbors(idx);

}



//! given a face index, return the face index that spans the same nodes
template <class Basis>
bool
PrismVolMesh<Basis>::get_neighbor(typename Face::index_type &neighbor,
                                  typename Face::index_type idx) const
{
  ASSERTMSG(synchronized_ & FACES_E,
            "Must call synchronize ELEM_NEIGHBORS_E on PrismVolMesh first.");
  pair<typename Face::HalfFaceSet::const_iterator,
       typename Face::HalfFaceSet::const_iterator> range =
    all_faces_.equal_range(idx);

  // ASSERT that this face was computed
  ASSERT(range.first != range.second);

  // Cell has no neighbor
  typename Face::HalfFaceSet::const_iterator second = range.first;
  if (++second == range.second) 
  {
    neighbor = MESH_NO_NEIGHBOR;
    return false;
  }

  if ((*range.first).index_ == idx)
    neighbor = (*second).index_;
  else if ((*second).index_ == idx)
    neighbor = (*range.first).index_;
  else {ASSERTFAIL("Non-Manifold Face in all_faces_ structure.");}

  return true;
}


template <class Basis>
bool
PrismVolMesh<Basis>::locate(typename Node::index_type &loc, const Point &p) const
{
  return(locate_node(loc,p));
}


template <class Basis>
bool
PrismVolMesh<Basis>::locate(typename Edge::index_type &edge, const Point &p) const
{
  return(locate_edge(edge,p));
}


template <class Basis>
bool
PrismVolMesh<Basis>::locate(typename Face::index_type &face, const Point &p) const
{
  return(locate_face(face,p));
}


template <class Basis>
bool
PrismVolMesh<Basis>::locate(typename Cell::index_type &cell, const Point &p) const
{
  return (locate_elem(cell,p));
}


template <class Basis>
int
PrismVolMesh<Basis>::get_weights(const Point &p, typename Cell::array_type &l,
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
PrismVolMesh<Basis>::get_weights(const Point &p, typename Node::array_type &l,
                                 double *w)
{
  typename Cell::index_type idx;
  if (locate(idx, p))
  {
    get_nodes(l,idx);
    vector<double> coords(3);
    if (get_coords(coords, p, idx)) {
      basis_.get_weights(coords, w);
      return basis_.dofs();
    }
  }
  return 0;
}


//===============================================================
// area3D_Polygon(): computes the area of a 3D planar polygon
//    Input:  int n = the number of vertices in the polygon
//            Point* V = an array of n+2 vertices in a plane
//                       with V[n]=V[0] and V[n+1]=V[1]
//            Point N = unit normal vector of the polygon's plane
//    Return: the (float) area of the polygon

// Copyright 2000, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

template <class Basis>
double
PrismVolMesh<Basis>::polygon_area(const typename Node::array_type &ni,
                                  const Vector N) const
{
  double area = 0;
  double an, ax, ay, az;  // abs value of normal and its coords
  int   coord;           // coord to ignore: 1=x, 2=y, 3=z
  index_type i, j, k;         // loop indices
  const index_type n = ni.size();

  // select largest abs coordinate to ignore for projection
  ax = (N.x()>0 ? N.x() : -N.x());     // abs x-coord
  ay = (N.y()>0 ? N.y() : -N.y());     // abs y-coord
  az = (N.z()>0 ? N.z() : -N.z());     // abs z-coord

  coord = 3;                     // ignore z-coord
  if (ax > ay) 
  {
    if (ax > az) coord = 1;      // ignore x-coord
  }
  else if (ay > az) coord = 2;   // ignore y-coord

  // compute area of the 2D projection
  for (i=1, j=2, k=0; i<=n; i++, j++, k++)
    switch (coord) 
    {
    case 1:
      area += (points_[ni[i%n]].y() *
               (points_[ni[j%n]].z() - points_[ni[k%n]].z()));
      continue;
    case 2:
      area += (points_[ni[i%n]].x() *
               (points_[ni[j%n]].z() - points_[ni[k%n]].z()));
      continue;
    case 3:
      area += (points_[ni[i%n]].x() *
               (points_[ni[j%n]].y() - points_[ni[k%n]].y()));
      continue;
    }

  // scale to get area before projection
  an = sqrt( ax*ax + ay*ay + az*az);  // length of normal vector
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
void
PrismVolMesh<Basis>::compute_epsilon()
{
  epsilon_ = get_bounding_box().diagonal().length()*1e-8;
  synchronized_ |= EPSILON_E;
}


template <class Basis>
void
PrismVolMesh<Basis>::compute_grid()
{
  grid_lock_.lock();
  if (synchronized_ & LOCATE_E) 
  {
    grid_lock_.unlock();
    return;
  }

  BBox bb = get_bounding_box();
  if (bb.valid())
  {
    // Cubed root of number of cells to get a subdivision ballpark.
    typename Cell::size_type csize;  size(csize);
    const int s = (int)(ceil(pow((double)csize , (1.0/3.0)))) / 2 + 1;
    const Vector cell_epsilon = bb.diagonal() * (1.0e-4 / s);
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
      box.extend(points_[nodes[0]]);
      box.extend(points_[nodes[1]]);
      box.extend(points_[nodes[2]]);
      box.extend(points_[nodes[3]]);
      box.extend(points_[nodes[4]]);
      box.extend(points_[nodes[5]]);
      const Point padmin(box.min() - cell_epsilon);
      const Point padmax(box.max() + cell_epsilon);
      box.extend(padmin);
      box.extend(padmax);

      sgc.insert(*ci, box);

      ++ci;
    }

    grid_ = scinew SearchGrid(sgc);
  }

  synchronized_ |= LOCATE_E;
  grid_lock_.unlock();
}


template <class Basis>
void
PrismVolMesh<Basis>::orient(typename Cell::index_type idx)
{
  Point center;
  get_center(center, idx);

  typename Face::array_type faces;
  get_faces(faces, idx);

  for (index_type i=0; i<PRISM_NFACES; i++)
  {
    typename Node::array_type ra;
    get_nodes(ra, faces[i]);
    const Point &p0 = point(ra[0]);
    const Point &p1 = point(ra[1]);
    const Point &p2 = point(ra[2]);

    const Vector v0(p0 - p1), v1(p2 - p1);
    const Vector normal = Cross(v0, v1);
    const Vector off1(center - p1);

    double dotprod = Dot(off1, normal);

    if( fabs( dotprod ) <  MIN_ELEMENT_VAL )
    {
      cerr << "Warning cell " << idx << " face " << i;
      cerr << " is malformed " << endl;
    }
  }
}


template <class Basis>
bool
PrismVolMesh<Basis>::inside(typename Cell::index_type idx, const Point &p) const
{
  vector<double> c(3);
  return get_coords(c, p, idx);
}

template <class Basis>
typename PrismVolMesh<Basis>::Node::index_type
PrismVolMesh<Basis>::add_find_point(const Point &p, double err)
{
  typename Node::index_type i;
  if (locate(i, p) && (points_[i] - p).length2() < err)
    return i;
  else
  {
    points_.push_back(p);
    if (synchronized_ & NODE_NEIGHBORS_E)
    {
      node_neighbor_lock_.lock();
      node_neighbors_.push_back(vector<typename Cell::index_type>());
      node_neighbor_lock_.unlock();
    }
    return points_.size() - 1;
  }
}


template <class Basis>
typename PrismVolMesh<Basis>::Elem::index_type
PrismVolMesh<Basis>::add_prism(typename Node::index_type a,
                               typename Node::index_type b,
                               typename Node::index_type c,
                               typename Node::index_type d,
                               typename Node::index_type e,
                               typename Node::index_type f)
{
  const index_type idx = static_cast<size_t>(cells_.size()) / PRISM_NNODES;
  cells_.push_back(a);
  cells_.push_back(b);
  cells_.push_back(c);
  cells_.push_back(d);
  cells_.push_back(e);
  cells_.push_back(f);

  create_cell_node_neighbors(idx);
  create_cell_edges(idx);
  create_cell_faces(idx);
  synchronized_ &= ~LOCATE_E;

  return idx;
}

template <class Basis>
typename PrismVolMesh<Basis>::Node::index_type
PrismVolMesh<Basis>::add_point(const Point &p)
{
  points_.push_back(p);
  if (synchronized_ & NODE_NEIGHBORS_E)
  {
    node_neighbor_lock_.lock();
    node_neighbors_.push_back(vector<typename Cell::index_type>());
    node_neighbor_lock_.unlock();
  }
  return points_.size() - 1;
}


template <class Basis>
void
PrismVolMesh<Basis>::get_center(Point &p, typename Node::array_type& arr) const
{
  Vector v(0,0,0);

  for( index_type i=0; i<static_cast<index_type>(arr.size()); i++ ) 
  {
    const Point &p0 = point(arr[i]);
    v += p0.asVector();
  }

  p = (v / (double) arr.size()).asPoint();
}


template <class Basis>
typename PrismVolMesh<Basis>::Elem::index_type
PrismVolMesh<Basis>::add_prism(const Point &p0, const Point &p1,
                               const Point &p2, const Point &p3,
                               const Point &p4, const Point &p5)
{
  return add_prism(add_find_point(p0), add_find_point(p1),
                   add_find_point(p2), add_find_point(p3),
                   add_find_point(p4), add_find_point(p5));
}


template <class Basis>
void
PrismVolMesh<Basis>::delete_cells(set<index_type> &to_delete)
{
  vector<index_type> old_cells = cells_;
  index_type i = 0;

  cells_.clear();
  cells_.reserve(old_cells.size() - to_delete.size()*PRISM_NNODES);

  for (set<index_type>::iterator deleted=to_delete.begin();
       deleted!=to_delete.end(); deleted++)
  {
    for (;i < *deleted; i++) 
    {
      const index_type base = i * PRISM_NNODES;
      for (index_type c=base; c<base+PRISM_NNODES; c++)
        cells_.push_back(old_cells[c]);
    }

    ++i;
  }

  for (; i < (index_type)(old_cells.size()/PRISM_NNODES); i++)
  {
    const index_type base = i * PRISM_NNODES;
    for (index_type c=base; c<base+PRISM_NNODES; c++)
      cells_.push_back(old_cells[c]);
  }
}


template <class Basis>
typename PrismVolMesh<Basis>::Elem::index_type
PrismVolMesh<Basis>::mod_prism(typename Cell::index_type idx,
                               typename Node::index_type a,
                               typename Node::index_type b,
                               typename Node::index_type c,
                               typename Node::index_type d,
                               typename Node::index_type e,
                               typename Node::index_type f)
{
  delete_cell_node_neighbors(idx);
  delete_cell_edges(idx);
  delete_cell_faces(idx);
  const index_type base = idx * PRISM_NNODES;
  cells_[base+0] = a;
  cells_[base+1] = b;
  cells_[base+2] = c;
  cells_[base+3] = d;
  cells_[base+4] = e;
  cells_[base+5] = f;
  create_cell_node_neighbors(idx);
  create_cell_edges(idx);
  create_cell_faces(idx);
  synchronized_ &= ~LOCATE_E;
  return idx;
}


#define PRISM_VOL_MESH_VERSION 3

template <class Basis>
void
PrismVolMesh<Basis>::io(Piostream &stream)
{
  const int version = stream.begin_class(type_name(-1),
                                         PRISM_VOL_MESH_VERSION);
  Mesh::io(stream);

  SCIRun::Pio(stream, points_);
  SCIRun::Pio_index(stream, cells_);
  if (version == 1) 
  {
    vector<int> neighbors;
    SCIRun::Pio(stream, neighbors);
  }
  if (version >= 2) 
  {
    basis_.io(stream);
  }
  stream.end_class();

  if (stream.reading())
    vmesh_ = CreateVPrismVolMesh(this);
}


template <class Basis>
const TypeDescription*
get_type_description(PrismVolMesh<Basis> *)
{
  static TypeDescription *td = 0;
  if (!td) 
  {
    const TypeDescription *sub = SCIRun::get_type_description((Basis*)0);
    TypeDescription::td_vec *subs = scinew TypeDescription::td_vec(1);
    (*subs)[0] = sub;
    td = scinew TypeDescription("PrismVolMesh", subs,
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }

  return td;
}


template <class Basis>
const TypeDescription*
PrismVolMesh<Basis>::get_type_description() const
{
  return SCIRun::get_type_description((PrismVolMesh<Basis> *)0);
}


template <class Basis>
const TypeDescription*
PrismVolMesh<Basis>::node_type_description()
{
  static TypeDescription *td = 0;
  if (!td) 
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PrismVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Node",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
PrismVolMesh<Basis>::edge_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PrismVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Edge",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
PrismVolMesh<Basis>::face_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PrismVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Face",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


template <class Basis>
const TypeDescription*
PrismVolMesh<Basis>::cell_type_description()
{
  static TypeDescription *td = 0;
  if (!td)
  {
    const TypeDescription *me =
      SCIRun::get_type_description((PrismVolMesh<Basis> *)0);
    td = scinew TypeDescription(me->get_name() + "::Cell",
                                string(__FILE__),
                                "SCIRun",
                                TypeDescription::MESH_E);
  }
  return td;
}


} // namespace SCIRun


#endif // SCI_project_PrismVolMesh_h
