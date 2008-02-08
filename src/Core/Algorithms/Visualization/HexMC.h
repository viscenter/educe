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
 *  HexMC.h
 *
 *  \author Yarden Livnat
 *   Department of Computer Science
 *   University of Utah
 *   \date Feb 2001
 *
 *  Copyright (C) 2001 SCI Institute
 */


#ifndef HexMC_h
#define HexMC_h

#include <Core/Geom/GeomTriangles.h>
#include <Core/Algorithms/Visualization/mcube2.h>
#include <Core/Basis/TriLinearLgn.h>
#include <Core/Basis/QuadBilinearLgn.h>
#include <Core/Datatypes/TriSurfMesh.h>
#include <Core/Datatypes/QuadSurfMesh.h>
#include <Core/Containers/FData.h>
#include <Core/Datatypes/GenericField.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <sci_hash_map.h>

namespace SCIRun {

struct HexMCBase {
  virtual ~HexMCBase() {}
  static const string& get_h_file_path();
};
//! A Macrching Cube teselator for a Hexagon cell     

template<class Field>
class HexMC : public HexMCBase
{
public:
  typedef Field                                       field_type;
  typedef typename Field::mesh_type::Cell::index_type cell_index_type;
  typedef typename Field::mesh_type::Node::index_type node_index_type;
  typedef typename Field::value_type                  value_type;
  typedef typename Field::mesh_type                   mesh_type;
  typedef typename Field::mesh_handle_type            mesh_handle_type;
  typedef typename mesh_type::Node::array_type        node_array_type;

  typedef TriSurfMesh<TriLinearLgn<Point> >                 TSMesh;
  typedef TriLinearLgn<double>                              TDatBasis;
  typedef GenericField<TSMesh, TDatBasis, vector<double> >  TSField;  

  typedef QuadSurfMesh<QuadBilinearLgn<Point> >             QSMesh;
  typedef QuadBilinearLgn<double>                           QDatBasis;
  typedef GenericField<QSMesh, QDatBasis, vector<double> >  QSField;  

private:
  LockingHandle<Field> field_;
  mesh_handle_type mesh_;
  GeomFastTriangles *triangles_;
  bool build_field_;
  TSMesh::handle_type trisurf_;
  QSMesh::handle_type quadsurf_;
  SCIRun::size_type nx_, ny_, nz_;

  struct edgepair_t
  {
    SCIRun::index_type first;
    SCIRun::index_type second;
    double dfirst;
  };

  struct edgepairless
  {
    bool operator()(const edgepair_t &a, const edgepair_t &b) const
    {
      return less(a,b);
    }
    static bool less(const edgepair_t &a, const edgepair_t &b)
    {
      return a.first < b.first || (a.first == b.first && a.second < b.second);
    }
  };

#ifdef HAVE_HASH_MAP
  struct edgepairequal
  {
    bool operator()(const edgepair_t &a, const edgepair_t &b) const
    {
      return a.first == b.first && a.second == b.second;
    }
  };

  struct edgepairhash
  {
    unsigned int operator()(const edgepair_t &a) const
    {
#if defined(__ECC) || defined(_MSC_VER)
      hash_compare<unsigned int> h;
#else
      hash<unsigned int> h;
#endif
      return h(a.first ^ a.second);
    }
# if defined(__ECC) || defined(_MSC_VER)

      // These are particularly needed by ICC's hash stuff
      static const size_t bucket_size = 4;
      static const size_t min_buckets = 8;
      
      // This is a less than function.
      bool operator()(const edgepair_t & a, const edgepair_t & b) const {
        return edgepairless::less(a,b);
      }
# endif // endif ifdef __ICC
  };

# if defined(__ECC) || defined(_MSC_VER)
  typedef hash_map<edgepair_t, TSMesh::Node::index_type, edgepairhash> edge_hash_type;
#else
  typedef hash_map<edgepair_t,
		   TSMesh::Node::index_type,
		   edgepairhash,
		   edgepairequal> edge_hash_type;
#endif // !defined(__ECC) && !defined(_MSC_VER)
  
#else
  typedef map<edgepair_t,
	      TSMesh::Node::index_type,
	      edgepairless> edge_hash_type;
#endif

  edge_hash_type   edge_map_;  // Unique edge cuts when surfacing node data
  vector<SCIRun::index_type> node_map_;  // Unique nodes when surfacing cell data.

  TSMesh::Node::index_type find_or_add_edgepoint(SCIRun::index_type n0,
						      SCIRun::index_type n1,
						      double d0,
						      const Point &p);
  QSMesh::Node::index_type find_or_add_nodepoint(node_index_type &);

  void extract_c( const cell_index_type &, double);
  void extract_n( const cell_index_type &, double);

public:
  HexMC( Field *field ) : field_(field), mesh_(field->get_typed_mesh()) {}
  virtual ~HexMC();
	
  void extract( const cell_index_type &, double);
  void reset( int, bool build_field, bool build_geom, bool transparency );
  GeomHandle get_geom() { return triangles_; };
  FieldHandle get_field(double val);
  MatrixHandle get_interpolant();
};
  

template<class Field>    
HexMC<Field>::~HexMC()
{
}
    

template<class Field>
void HexMC<Field>::reset( int n, bool build_field, bool build_geom, bool transparency )
{
  build_field_ = build_field;

  edge_map_.clear();
  nx_ = mesh_->get_ni();
  ny_ = mesh_->get_nj();
  nz_ = mesh_->get_nk();
  if (field_->basis_order() == 0)
  {
    mesh_->synchronize(Mesh::FACES_E);
    mesh_->synchronize(Mesh::ELEM_NEIGHBORS_E);
    if (build_field) { node_map_ = vector<SCIRun::index_type>(nx_ * ny_ * nz_, -1); }
  }

  triangles_ = 0;
  if (build_geom)
  {
    if( transparency )
      triangles_ = scinew GeomTranspTriangles;
    else
      triangles_ = scinew GeomFastTriangles;
  }

  trisurf_ = 0;
  quadsurf_ = 0;
  if (build_field_)
  {
    if (field_->basis_order() == 0)
    {
      quadsurf_ = scinew QSMesh;
    }
    else
    {
      trisurf_ = scinew TSMesh; 
    }
  }
}

template<class Field>
HexMC<Field>::TSMesh::Node::index_type
HexMC<Field>::find_or_add_edgepoint(SCIRun::index_type u0, SCIRun::index_type u1,
				    double d0, const Point &p) 
{
  if (d0 <= 0.0) { u1 = -1; }
  if (d0 >= 1.0) { u0 = -1; }
  edgepair_t np;
  if (u0 < u1)  { np.first = u0; np.second = u1; np.dfirst = d0; }
  else { np.first = u1; np.second = u0; np.dfirst = 1.0 - d0; }
  const typename edge_hash_type::iterator loc = edge_map_.find(np);
  if (loc == edge_map_.end())
  {
    const TSMesh::Node::index_type nodeindex = trisurf_->add_point(p);
    edge_map_[np] = nodeindex;
    return nodeindex;
  }
  else
  {
    return (*loc).second;
  }
}


template<class Field>
HexMC<Field>::QSMesh::Node::index_type
HexMC<Field>::find_or_add_nodepoint(node_index_type &tet_node_idx)
{
  QSMesh::Node::index_type surf_node_idx;
  SCIRun::index_type i = node_map_[tet_node_idx];
  if (i != -1) surf_node_idx = (QSMesh::Node::index_type) i;
  else 
  {
    Point p;
    mesh_->get_point(p, tet_node_idx);
    surf_node_idx = quadsurf_->add_point(p);
    node_map_[tet_node_idx] = surf_node_idx;
  }
  return surf_node_idx;
}


template<class Field>
void
HexMC<Field>::extract( const cell_index_type& cell, double iso )
{
  if (field_->basis_order() == 1)
    extract_n(cell, iso);
  else
    extract_c(cell, iso);
}


template<class Field>
void
HexMC<Field>::extract_c( const cell_index_type& cell, double iso )
{
  value_type selfvalue, nbrvalue;
  if (!field_->value( selfvalue, cell )) return;
  typename mesh_type::Face::array_type faces;
  mesh_->get_faces(faces, cell);

  cell_index_type nbr_cell;
  Point p[4];
  node_array_type face_nodes;
  QSMesh::Node::index_type verts[4];
  
  for (size_t f=0; f<faces.size(); f++)
  {
    if (mesh_->get_neighbor(nbr_cell, cell, faces[f]) &&
	field_->value(nbrvalue, nbr_cell) &&
	(selfvalue > nbrvalue) &&
	((selfvalue - iso) * (nbrvalue - iso) < 0 ))
    {
      mesh_->get_nodes(face_nodes, faces[f]);
      for (int n=0; n<4; n++) { mesh_->get_center(p[n], face_nodes[n]); }

      if (triangles_)
      {
        triangles_->add(p[0], p[1], p[2]);
        triangles_->add(p[2], p[3], p[0]);
      }

      if (build_field_)
      {
        for (int n=0; n<4; n++)
        {
          verts[n]=find_or_add_nodepoint(face_nodes[n]);
        }
        quadsurf_->add_quad(verts[0], verts[1], verts[2], verts[3]);
      }
    }
  }
}


template<class Field>
void
HexMC<Field>::extract_n( const cell_index_type& cell, double iso )
{
  node_array_type node(8);
  Point p[8];
  value_type value[8];
  int code = 0;

  mesh_->get_nodes( node, cell );

  for (int i=7; i>=0; i--) 
  {
    mesh_->get_point( p[i], node[i] );
    if (!field_->value( value[i], node[i] )) return;
    code = code*2+(value[i] < iso );
  }

  if ( code == 0 || code == 255 )
    return;

//  TriangleCase *tcase=&tri_case[code];
  TRIANGLE_CASES *tcase=&triCases[code];
  int *vertex = tcase->edges;
  
  Point q[12];
  TSMesh::Node::index_type surf_node[12];

  // interpolate and project vertices
  SCIRun::index_type v = 0;
  vector<bool> visited(12, false);
  while (vertex[v] != -1) 
  {
    SCIRun::index_type i = vertex[v++];
    if (visited[i]) continue;
    visited[i]=true;
    SCIRun::index_type v1 = edge_tab[i][0];
    SCIRun::index_type v2 = edge_tab[i][1];
    const double d = (value[v1]-iso) / double(value[v1]-value[v2]);
    q[i] = Interpolate(p[v1], p[v2], d);
    if (build_field_)
    {
      surf_node[i] = find_or_add_edgepoint(node[v1], node[v2], d, q[i]);
    }
  }    
  
  v = 0;
  while(vertex[v] != -1) 
  {
    SCIRun::index_type v0 = vertex[v++];
    SCIRun::index_type v1 = vertex[v++];
    SCIRun::index_type v2 = vertex[v++];
    if (triangles_)
    {
      triangles_->add(q[v0], q[v1], q[v2]);
    }
    if (build_field_)
    {
      if (surf_node[v0] != surf_node[v1] &&
          surf_node[v1] != surf_node[v2] &&
          surf_node[v2] != surf_node[v0])
      {
        trisurf_->add_triangle(surf_node[v0], surf_node[v1], surf_node[v2]);
      }
    }
  }
}


template<class Field>
FieldHandle
HexMC<Field>::get_field(double value)
{
  if (field_->basis_order() == 0)
  {
    QSField *fld = 0;
    if (quadsurf_.get_rep())
    {
      fld = scinew QSField(quadsurf_);
      vector<double>::iterator iter = fld->fdata().begin();
      while (iter != fld->fdata().end()) { (*iter)=value; ++iter; }
    }
    return fld;
  }
  else
  {
    TSField *fld = 0;
    if (trisurf_.get_rep())
    {
      fld = scinew TSField(trisurf_);
      vector<double>::iterator iter = fld->fdata().begin();
      while (iter != fld->fdata().end()) { (*iter)=value; ++iter; }
    }
    return fld;
  }
}


template<class Field>
MatrixHandle
HexMC<Field>::get_interpolant()
{
  if (field_->basis_order() == 1)
  {
    const Matrix::size_type nrows = static_cast<Matrix::size_type>(edge_map_.size());
    const Matrix::size_type ncols = nx_ * ny_ * nz_;
    Matrix::index_type *rr = scinew Matrix::index_type[nrows+1];
    Matrix::index_type *cc = scinew Matrix::index_type[nrows*2];
    double *dd = scinew double[nrows*2];

    typename edge_hash_type::iterator eiter = edge_map_.begin();
    while (eiter != edge_map_.end())
    {
      const SCIRun::index_type ei = (*eiter).second;

      cc[ei * 2 + 0] = (*eiter).first.first;
      cc[ei * 2 + 1] = (*eiter).first.second;
      dd[ei * 2 + 0] = 1.0 - (*eiter).first.dfirst;
      dd[ei * 2 + 1] = (*eiter).first.dfirst;
      
      ++eiter;
    }

    Matrix::size_type nnz = 0;
    Matrix::index_type i;
    for (i = 0; i < nrows; i++)
    {
      rr[i] = nnz;
      if (cc[i * 2 + 0] > 0)
      {
        cc[nnz] = cc[i * 2 + 0];
        dd[nnz] = dd[i * 2 + 0];
        nnz++;
      }
      if (cc[i * 2 + 1] > 0)
      {
        cc[nnz] = cc[i * 2 + 1];
        dd[nnz] = dd[i * 2 + 1];
        nnz++;
      }
    }
    rr[i] = nnz;

    return scinew SparseRowMatrix(nrows, ncols, rr, cc, nnz, dd);
  }
  else
  {
    return 0;
  }
}

     
} // End namespace SCIRun

#endif // HexMC_H