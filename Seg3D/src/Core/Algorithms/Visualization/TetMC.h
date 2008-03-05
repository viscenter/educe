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
 *  TetMC.h
 *
 *  \author Yarden Livnat
 *   Department of Computer Science
 *   University of Utah
 *   \date Feb 2001
 *
 *  Copyright (C) 2001 SCI Institute
 */


#ifndef TetMC_h
#define TetMC_h

#include <Core/Algorithms/Visualization/BaseMC.h>
#include <Core/Basis/TriLinearLgn.h>
#include <Core/Datatypes/TriSurfMesh.h>
#include <Core/Datatypes/GenericField.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Geom/GeomTriangles.h>
#include <sci_hash_map.h>

namespace SCIRun {

struct TetMCBase {
  virtual ~TetMCBase() {}
  static const string& get_h_file_path();
};
//! A Marching Cube tesselator for a tetrahedral cell     

template<class Field>
class TetMC : public BaseMC, public TetMCBase
{
public:
  typedef Field                                  field_type;
  typedef typename Field::mesh_type::Cell::index_type  cell_index_type;
  typedef typename Field::mesh_type::Node::index_type  node_index_type;
  typedef typename Field::value_type             value_type;
  typedef typename Field::mesh_type              mesh_type;
  typedef typename Field::mesh_handle_type       mesh_handle_type;

  typedef TriSurfMesh<TriLinearLgn<Point> >                 TSMesh;
  typedef TriLinearLgn<double>                              TDatBasis;
  typedef GenericField<TSMesh, TDatBasis, vector<double> >  TSField;  


public:
  TetMC( Field *field ) : field_(field), mesh_(field->get_typed_mesh()),
			  triangles_(0), trisurf_(0) {}

  virtual ~TetMC() {}
	
  void extract( cell_index_type, double );
  void reset( int, bool build_field, bool build_geom, bool transparency );

  GeomHandle get_geom() { return triangles_; }
  FieldHandle get_field(double val);

private:
  void extract_n( cell_index_type, double );
  void extract_c( cell_index_type, double );

  TSMesh::Node::index_type find_or_add_edgepoint(SCIRun::index_type n0, SCIRun::index_type n1,
						      double d0,
						      const Point &p);
  TSMesh::Node::index_type find_or_add_nodepoint(node_index_type &n0);

  void find_or_add_parent(SCIRun::index_type u0, SCIRun::index_type u1,
			  double d0, SCIRun::index_type face);

  LockingHandle<Field> field_;
  mesh_handle_type mesh_;
  GeomFastTriangles *triangles_;
  TSMesh::handle_type trisurf_;
};
  

template<class Field>
void TetMC<Field>::reset( int n, bool build_field, bool build_geom, bool transparency )
{
  build_field_ = build_field;
  build_geom_  = build_geom;
  basis_order_ = field_->basis_order();

  edge_map_.clear();
  typename Field::mesh_type::Node::size_type nsize;
  mesh_->size(nsize);
  nnodes_ = nsize;

  cell_map_.clear();
  typename Field::mesh_type::Cell::size_type csize;
  mesh_->size(csize);
  ncells_ = csize;

  if (basis_order_ == 0)
  {
    mesh_->synchronize(Mesh::FACES_E);
    mesh_->synchronize(Mesh::ELEM_NEIGHBORS_E);
    if (build_field_)
    {
      node_map_ = vector<SCIRun::index_type>(nnodes_, -1);
    }
  }

  triangles_ = 0;
  if (build_geom_)
  {
    if( transparency )
      triangles_ = scinew GeomTranspTriangles;
    else
      triangles_ = scinew GeomFastTriangles;
  }
  geomHandle_ = triangles_;
  
  trisurf_ = 0;
  if (build_field_)
  {
    trisurf_ = scinew TSMesh;
  }
}


template<class Field>
TetMC<Field>::TSMesh::Node::index_type
TetMC<Field>::find_or_add_edgepoint(SCIRun::index_type u0, SCIRun::index_type u1,
                                    double d0, const Point &p) 
{
  if (d0 < 0.0) { u1 = -1; }
  if (d0 > 1.0) { u0 = -1; }
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
TetMC<Field>::TSMesh::Node::index_type
TetMC<Field>::find_or_add_nodepoint(node_index_type &tet_node_idx) 
{
  TSMesh::Node::index_type surf_node_idx;
  SCIRun::index_type i = node_map_[tet_node_idx];
  if (i != -1) surf_node_idx = (TSMesh::Node::index_type) i;
  else 
  {
    Point p;
    mesh_->get_point(p, tet_node_idx);
    surf_node_idx = trisurf_->add_point(p);
    node_map_[tet_node_idx] = surf_node_idx;
  }
  return surf_node_idx;
}


template<class Field>
void
TetMC<Field>::find_or_add_parent(SCIRun::index_type u0, SCIRun::index_type u1,
				 double d0, SCIRun::index_type face) 
{
  if (d0 < 0.0) { u1 = -1; }
  if (d0 > 1.0) { u0 = -1; }
  edgepair_t np;
  if (u0 < u1)  { np.first = u0; np.second = u1; np.dfirst = d0; }
  else { np.first = u1; np.second = u0; np.dfirst = 1.0 - d0; }
  const typename edge_hash_type::iterator loc = edge_map_.find(np);
  if (loc == edge_map_.end())
  {
    edge_map_[np] = face;
  }
  else
  {
    ASSERT(loc == edge_map_.end())
  }
}


template<class Field>
void TetMC<Field>::extract( cell_index_type cell, double v )
{
  if (basis_order_ == 0)
    extract_c(cell, v);
  else
    extract_n(cell, v);
}


template<class Field>
void TetMC<Field>::extract_c( cell_index_type cell, double iso )
{
  value_type selfvalue, nbrvalue;
  if (!field_->value( selfvalue, cell )) return;
  typename mesh_type::Face::array_type faces;
  mesh_->get_faces(faces, cell);

  cell_index_type nbr_cell;
  Point p[3];
  typename mesh_type::Node::array_type nodes;
  TSMesh::Node::array_type vertices(3);

  for (size_t i=0; i<faces.size(); i++)
  {
    if (mesh_->get_neighbor(nbr_cell, cell, faces[i]) &&
	field_->value(nbrvalue, nbr_cell) &&
	selfvalue <= iso && iso < nbrvalue)
    {
      mesh_->get_nodes(nodes, faces[i]);
      
      for (int j=0; j<3; j++)
      {
	mesh_->get_center(p[j], nodes[j]);
      }

      if (build_geom_)
      {
        triangles_->add(p[0], p[1], p[2]);
      }

      if (build_field_)
      {
	for (int j=0; j<3; j++)
        {
          vertices[j] = find_or_add_nodepoint(nodes[j]);
        }
        
	TSMesh::Face::index_type tface = trisurf_->add_elem(vertices);
	  
	const double d = (selfvalue - iso) / (selfvalue - nbrvalue);

	find_or_add_parent(cell, nbr_cell, d, tface);
      }
    }
  }
}


template<class Field>
void TetMC<Field>::extract_n( cell_index_type cell, double v )
{
  static int num[16] = { 0, 1, 1, 2, 1, 2, 2, 1, 1, 2, 2, 1, 2, 1, 1, 0 };
  static int order[16][4] = {
    {0, 0, 0, 0},   /* none - ignore */
    {3, 2, 0, 1},   /* 3 */
    {2, 1, 0, 3},   /* 2 */
    {2, 1, 0, 3},   /* 2, 3 */
    {1, 3, 0, 2},   /* 1 */
    {1, 0, 2, 3},   /* 1, 3 */
    {1, 3, 0, 2},   /* 1, 2 */
    {0, 2, 3, 1},   /* 1, 2, 3 */
    {0, 2, 1, 3},   /* 0 */
    {2, 3, 0, 1},   /* 0, 3 - reverse of 1, 2 */
    {3, 0, 2, 1},   /* 0, 2 - reverse of 1, 3 */
    {1, 0, 3, 2},   /* 0, 2, 3 - reverse of 1 */
    {3, 1, 0, 2},   /* 0, 1 - reverse of 2, 3 */
    {2, 3, 0, 1},   /* 0, 1, 3 - reverse of 2 */
    {3, 2, 1, 0},   /* 0, 1, 2 - reverse of 3 */
    {0, 0, 0, 0}    /* all - ignore */
  };
    
    
  typename mesh_type::Node::array_type node;
  Point p[4];
  value_type value[4];

  mesh_->get_nodes( node, cell );
  for (int i=0; i<4; i++)
    mesh_->get_point( p[i], node[i] );

// fix the node[i] ordering so tet is orientationally consistent
//  if (Dot(Cross(p[0]-p[1],p[0]-p[2]),p[0]-p[3])>0) {
//    typename mesh_type::Node::index_type nd=node[0];
//    node[0]=node[1];
//    node[1]=nd;
//  }

  int code = 0;
  for (int i=0; i<4; i++) 
  {
    mesh_->get_point( p[i], node[i] );
    if (!field_->value( value[i], node[i] )) return;
    code = code*2+(value[i] > v );
  }

  //  if ( show_case != -1 && (code != show_case) ) return;
  switch ( num[code] ) 
  {
  case 1: 
    {
      // make a single triangle
      int o = order[code][0];
      int i = order[code][1];
      int j = order[code][2];
      int k = order[code][3];

      const double v1 = (v-value[o])/double(value[i]-value[o]);
      const double v2 = (v-value[o])/double(value[j]-value[o]);
      const double v3 = (v-value[o])/double(value[k]-value[o]);
      const Point p1(Interpolate( p[o],p[i], v1));
      const Point p2(Interpolate( p[o],p[j], v2));
      const Point p3(Interpolate( p[o],p[k], v3));

      if (build_geom_)
      {
        triangles_->add( p1, p2, p3 );
      }
      if (build_field_)
      {
        TSMesh::Node::index_type i1, i2, i3;
        i1 = find_or_add_edgepoint(node[o], node[i], v1, p1);
        i2 = find_or_add_edgepoint(node[o], node[j], v2, p2);
        i3 = find_or_add_edgepoint(node[o], node[k], v3, p3);
        if (i1 != i2 && i2 != i3 && i3 != i1) {
          trisurf_->add_triangle(i1, i2, i3);
	  cell_map_.push_back( cell );
	}
      }
    }
    break;
  case 2: 
    {
      // make order triangles
      const int o = order[code][0];
      const int i = order[code][1];
      const int j = order[code][2];
      const int k = order[code][3];
      const double v1 = (v-value[o])/double(value[i]-value[o]);
      const double v2 = (v-value[o])/double(value[j]-value[o]);
      const double v3 = (v-value[k])/double(value[j]-value[k]);
      const double v4 = (v-value[k])/double(value[i]-value[k]);
      const Point p1(Interpolate( p[o],p[i], v1));
      const Point p2(Interpolate( p[o],p[j], v2));
      const Point p3(Interpolate( p[k],p[j], v3));
      const Point p4(Interpolate( p[k],p[i], v4));

      if (build_geom_)
      {
        triangles_->add( p1, p2, p3 );
        triangles_->add( p1, p3, p4 );
      }
      if (build_field_)
      {
        TSMesh::Node::index_type i1, i2, i3, i4;
        i1 = find_or_add_edgepoint(node[o], node[i], v1, p1);
        i2 = find_or_add_edgepoint(node[o], node[j], v2, p2);
        i3 = find_or_add_edgepoint(node[k], node[j], v3, p3);
        i4 = find_or_add_edgepoint(node[k], node[i], v4, p4);
        if (i1 != i2 && i2 != i3 && i3 != i1) {
          trisurf_->add_triangle(i1, i2, i3);
	  cell_map_.push_back( cell );
	}
        if (i1 != i3 && i3 != i4 && i4 != i1) {
          trisurf_->add_triangle(i1, i3, i4);
	  cell_map_.push_back( cell );
	}
      }
    }
    break;
  default:
    // do nothing. 
    // MarchingCubes calls extract on each and every cell. i.e., this is
    // not an error
    break;
  }
}


template<class Field>
FieldHandle
TetMC<Field>::get_field(double value)
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

} // End namespace SCIRun

#endif // TetMC_h
