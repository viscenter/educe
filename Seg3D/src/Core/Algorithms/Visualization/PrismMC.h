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
 *  PrismMC.h
 *
 *   SCI Institute
 *   University of Utah
 *   Feb 2001
 *
 *  Copyright (C) 2001 SCI Institute
 */


#ifndef PrismMC_h
#define PrismMC_h

#include <Core/Algorithms/Visualization/mcube2.h>
#include <Core/Algorithms/Visualization/BaseMC.h>
#include <Core/Geom/GeomTriangles.h>
#include <Core/Basis/TriLinearLgn.h>
#include <Core/Datatypes/TriSurfMesh.h>
#include <Core/Datatypes/GenericField.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <sci_hash_map.h>

namespace SCIRun {

struct PrismMCBase {
  virtual ~PrismMCBase() {}
  static const string& get_h_file_path();
};
//! A Marching Cube tesselator for a prism cell     

template<class Field>
class PrismMC : public BaseMC, public PrismMCBase
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


  PrismMC( Field *field ) : field_(field), mesh_(field->get_typed_mesh()),
			    triangles_(0), trisurf_(0) {}

  virtual ~PrismMC() {}
	
  void reset( int, bool build_field, bool build_geom, bool transparency);
  void extract( cell_index_type, double );

  FieldHandle get_field(double val);

private:
  void extract_n( cell_index_type, double );
  void extract_c( cell_index_type, double );

  TSMesh::Node::index_type find_or_add_edgepoint(SCIRun::index_type u0, 
						 SCIRun::index_type u1,
						 double d0,
						 const Point &p);

  TSMesh::Node::index_type find_or_add_nodepoint(node_index_type &);
  
  void find_or_add_parent(SCIRun::index_type u0, SCIRun::index_type u1,
			  double d0, SCIRun::index_type face);

  LockingHandle<Field> field_;
  mesh_handle_type mesh_;
  GeomFastTriangles *triangles_;
  TSMesh::handle_type trisurf_;
};
  

template<class Field>
void PrismMC<Field>::reset( int n, bool build_field, bool build_geom, bool transparency )
{
  build_field_ = build_field;
  build_geom_ = build_geom;
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
      node_map_ = vector<SCIRun::index_type>(nsize, -1);
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
PrismMC<Field>::TSMesh::Node::index_type
PrismMC<Field>::find_or_add_edgepoint(SCIRun::index_type u0, 
				      SCIRun::index_type u1,
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
PrismMC<Field>::TSMesh::Node::index_type
PrismMC<Field>::find_or_add_nodepoint(node_index_type &tet_node_idx) 
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
PrismMC<Field>::find_or_add_parent(SCIRun::index_type u0, SCIRun::index_type u1,
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
    // This should never happen but does for prisms. This is because
    // the quads are represented as two triangles so the cell is used
    // twice.
//    ASSERTMSG((*loc).second == face, "Two Triangles belong to save parent cell.");
  }
}


template<class Field>
void PrismMC<Field>::extract( cell_index_type cell, double v )
{
  if (basis_order_ == 0)
    extract_c(cell, v);
  else
    extract_n(cell, v);
}


template<class Field>
void PrismMC<Field>::extract_c( cell_index_type cell, double iso )
{
  value_type selfvalue, nbrvalue;
  if (!field_->value( selfvalue, cell )) return;
  typename mesh_type::Face::array_type faces;
  mesh_->get_faces(faces, cell);

  cell_index_type nbr_cell;
  Point p[4];
  typename mesh_type::Node::array_type face_nodes;
  TSMesh::Node::array_type vertices(3);

  for (int i=0; i<faces.size(); i++)
  {
    if (mesh_->get_neighbor(nbr_cell, cell, faces[i]) &&
	field_->value(nbrvalue, nbr_cell) &&
	selfvalue <= iso && iso < nbrvalue)
    {
      mesh_->get_nodes(face_nodes, faces[i]);

      for (int j=0; j<face_nodes.size(); j++)
      {
	mesh_->get_center(p[j], face_nodes[j]);
      }

      if (build_geom_)
      {
        triangles_->add(p[0], p[1], p[2]);
            
        if( face_nodes.size() == 4 )
          triangles_->add(p[0], p[2], p[3]);
      }

      if (build_field_)
      {
	for (int j=0; j<face_nodes.size(); j++)
	{
          vertices[j] = find_or_add_nodepoint(face_nodes[j]);
	}

	TSMesh::Face::index_type tface =
	  trisurf_->add_triangle(vertices[0], vertices[1], vertices[2]);
        
 	const double d = (selfvalue - iso) / (selfvalue - nbrvalue);

	find_or_add_parent(cell, nbr_cell, d, tface);

	if( face_nodes.size() == 4 )
	{
          tface = trisurf_->add_triangle(vertices[0], vertices[2], vertices[3]);

	  const double d = (selfvalue - iso) / (selfvalue - nbrvalue);

	  find_or_add_parent(cell, nbr_cell, d, tface);
	}
      }
    }
  }
}


template<class Field>
void
PrismMC<Field>::extract_n( cell_index_type cell, double iso )
{
  typename mesh_type::Node::array_type node;
  Point p[8];
  value_type value[8];
  int code = 0;

  mesh_->get_nodes( node, cell );

  typename mesh_type::Node::array_type nodes;
  mesh_->get_nodes( nodes, cell );

  // Fake having 8 nodes and use the HexMC algorithm.
  node.resize(8);
  
  node[0] = nodes[0];
  node[1] = nodes[1];
  node[2] = nodes[2];
  node[3] = nodes[0];
  node[4] = nodes[3];
  node[5] = nodes[4];
  node[6] = nodes[5];
  node[7] = nodes[3];

  for (size_t i=node.size()-1; i>=0; i--) 
  {
    mesh_->get_point( p[i], node[i] );
    if (!field_->value( value[i], node[i] )) return;
    code = code*2+(value[i] < iso );
  }

  if ( code == 0 || code == 255 )
    return;

  TRIANGLE_CASES *tcase=&triCases[code];
  SCIRun::index_type *vertex = tcase->edges;
  
  Point q[12];
  TSMesh::Node::index_type surf_node[12];

  // interpolate and project vertices
  int v = 0;
  vector<bool> visited(12, false);
  while (vertex[v] != -1) 
  {
    SCIRun::index_type i = vertex[v++];
    if (visited[i]) continue;
    visited[i]=true;
    const SCIRun::index_type v1 = edge_tab[i][0];
    const SCIRun::index_type v2 = edge_tab[i][1];
    const double d = (value[v1]-iso)/double(value[v1]-value[v2]);
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

    // Degenerate triangle can be built since ponit were duplicated
    // above in order to make a hexvol for MC. 
    if( v0 != v1 && v0 != v2 && v1 != v2 ) 
    {
      if (build_geom_)
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
	  cell_map_.push_back( cell );
        }
      }
    }
  }
}


template<class Field>
FieldHandle
PrismMC<Field>::get_field(double value)
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

#endif // PrismMC_h
