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
 *  TriMC.h
 *
 *   \author Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   \date September 2002
 *
 */


#ifndef TriMC_h
#define TriMC_h

#include <Core/Algorithms/Visualization/BaseMC.h>
#include <Core/Basis/CrvLinearLgn.h>
#include <Core/Datatypes/CurveMesh.h>
#include <Core/Datatypes/GenericField.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Geom/GeomLine.h>
#include <sci_hash_map.h>

namespace SCIRun {

struct TriMCBase {
  virtual ~TriMCBase() {}
  static const string& get_h_file_path();
};

//! A Marching Cube tesselator for a triangle face

template<class Field>
class TriMC : public BaseMC, public TriMCBase
{
public:
  typedef Field                                  field_type;
  typedef typename Field::mesh_type::Face::index_type  cell_index_type;
  typedef typename Field::mesh_type::Node::index_type  node_index_type;
  typedef typename Field::value_type             value_type;
  typedef typename Field::mesh_type              mesh_type;
  typedef typename Field::mesh_handle_type       mesh_handle_type;

  typedef CurveMesh<CrvLinearLgn<Point> >                 CMesh;
  typedef CrvLinearLgn<double>                            DatBasis;
  typedef GenericField<CMesh, DatBasis, vector<double> >  CField;  


  TriMC( Field *field ) : field_(field), mesh_(field->get_typed_mesh()),
			  lines_(0), curve_(0) {}

  virtual ~TriMC() {}
	
  void extract( cell_index_type, double );
  void reset( int, bool build_field, bool build_geom, bool transparency );

  FieldHandle get_field(double val);

private:
  void extract_n( cell_index_type, double );
  void extract_f( cell_index_type, double );

  CMesh::Node::index_type find_or_add_edgepoint(SCIRun::index_type n0,
						SCIRun::index_type n1,
						double d0,
						const Point &p);

  CMesh::Node::index_type find_or_add_nodepoint(node_index_type &idx);

  void find_or_add_parent(SCIRun::index_type u0, SCIRun::index_type u1,
			  double d0, SCIRun::index_type edge);

  LockingHandle<Field> field_;
  mesh_handle_type mesh_;
  GeomLines *lines_;
  CMesh::handle_type curve_;
};
  

template<class Field>
void TriMC<Field>::reset( int n, bool build_field, bool build_geom, bool transparency )
{
  build_field_ = build_field;
  build_geom_  = build_geom;
  basis_order_ = field_->basis_order();

  edge_map_.clear();
  typename Field::mesh_type::Node::size_type nsize;
  mesh_->size(nsize);
  nnodes_ = nsize;
 
  cell_map_.clear();
  typename Field::mesh_type::Face::size_type csize;
  mesh_->size(csize);
  ncells_ = csize;

  if (basis_order_ == 0)
  {
    mesh_->synchronize(Mesh::EDGES_E);
    mesh_->synchronize(Mesh::ELEM_NEIGHBORS_E);
    if (build_field_)
    {
      node_map_ = vector<SCIRun::index_type>(nsize, -1);
    }
  }

  lines_ = 0;
  if (build_geom_)
  {
    if( transparency )
      lines_ = scinew GeomTranspLines;
    else
      lines_ = scinew GeomLines;
  }
  geomHandle_ = lines_;

  curve_ = 0;
  if (build_field_)
  {
    curve_ = scinew CMesh;
  }
}


template<class Field>
TriMC<Field>::CMesh::Node::index_type
TriMC<Field>::find_or_add_edgepoint(SCIRun::index_type u0, SCIRun::index_type u1,
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
    const CMesh::Node::index_type nodeindex = curve_->add_point(p);
    edge_map_[np] = nodeindex;
    return nodeindex;
  }
  else
  {
    return (*loc).second;
  }
}


template<class Field>
TriMC<Field>::CMesh::Node::index_type
TriMC<Field>::find_or_add_nodepoint(node_index_type &tri_node_idx)
{
  CMesh::Node::index_type curve_node_idx;
  SCIRun::index_type i = node_map_[tri_node_idx];
  if (i != -1) curve_node_idx = (CMesh::Node::index_type) i;
  else 
  {
    Point p;
    mesh_->get_point(p, tri_node_idx);
    curve_node_idx = curve_->add_point(p);
    node_map_[tri_node_idx] = curve_node_idx;
  }
  return curve_node_idx;
}


template<class Field>
void
TriMC<Field>::find_or_add_parent(SCIRun::index_type u0, SCIRun::index_type u1,
				 double d0, SCIRun::index_type edge) 
{
  if (d0 < 0.0) { u1 = -1; }
  if (d0 > 1.0) { u0 = -1; }
  edgepair_t np;
  if (u0 < u1)  { np.first = u0; np.second = u1; np.dfirst = d0; }
  else { np.first = u1; np.second = u0; np.dfirst = 1.0 - d0; }
  const typename edge_hash_type::iterator loc = edge_map_.find(np);
  if (loc == edge_map_.end())
  {
    edge_map_[np] = edge;
  }
  else
  {
    // This should never happen
  }
}


template<class Field>
void TriMC<Field>::extract( cell_index_type cell, double v )
{
  if (basis_order_ == 0)
    extract_f(cell, v);
  else
    extract_n(cell, v);
}


template<class Field>
void TriMC<Field>::extract_f( cell_index_type cell, double iso )
{
  value_type selfvalue, nbrvalue;
  if (!field_->value( selfvalue, cell )) return;
  typename mesh_type::Edge::array_type edges;
  mesh_->get_edges(edges, cell);

  cell_index_type nbr_cell;
  Point p[2];
  typename mesh_type::Node::array_type nodes;
  CMesh::Node::array_type vertices(2);

  for (size_t i=0; i<edges.size(); i++)
  {
    if (mesh_->get_neighbor(nbr_cell, cell, edges[i]) &&
	field_->value(nbrvalue, nbr_cell) &&
	selfvalue <= iso && iso < nbrvalue)
    {
      mesh_->get_nodes(nodes, edges[i]);

      for (size_t j=0; j<2; j++) 
      { 
        mesh_->get_center(p[j], nodes[j]); 
      }

      if (build_geom_)
      {
        lines_->add(p[0], p[1]);
      }

      if (build_field_)
      {
        for (size_t j=0; j<2; j ++)
        {
          vertices[j] = find_or_add_nodepoint(nodes[j]);
        }

	CMesh::Edge::index_type cedge = curve_->add_elem(vertices);

	const double d = (selfvalue - iso) / (selfvalue - nbrvalue);

	find_or_add_parent(cell, nbr_cell, d, cedge);
      }
    }
  }
}


template<class Field>
void TriMC<Field>::extract_n( cell_index_type cell, double v )
{
  typename mesh_type::Node::array_type node;
  Point p[3];
  value_type value[3];

  mesh_->get_nodes( node, cell );

  static int num[8] = { 0, 1, 1, 1, 1, 1, 1, 0 };
  static int clip[8] = { 0, 0, 1, 2, 2, 1, 0, 0 };

  int code = 0;
  for (int i=0; i<3; i++) 
  {
    mesh_->get_point( p[i], node[i] );
    if (!field_->value( value[i], node[i] )) return;
    code |= (value[i] > v ) << i;
  }

  //  if ( show_case != -1 && (code != show_case) ) return;
  if (num[code])
  {
    const int a = clip[code];
    const int b = (a + 1) % 3;
    const int c = (a + 2) % 3;
    
    const double d0 = (v-value[a])/double(value[b]-value[a]);
    const double d1 = (v-value[a])/double(value[c]-value[a]);

    const Point p0(Interpolate(p[a], p[b], d0));
    const Point p1(Interpolate(p[a], p[c], d1));

    if (build_geom_)
    {
      lines_->add( p0, p1 );
    }

    if (build_field_)
    {
      CMesh::Node::array_type cnode(2);
      cnode[0] = find_or_add_edgepoint(node[a], node[b], d0, p0);
      cnode[1] = find_or_add_edgepoint(node[a], node[c], d1, p1);
      if (cnode[0] != cnode[1])
        curve_->add_elem(cnode);
    }
  }
}


template<class Field>
FieldHandle
TriMC<Field>::get_field(double value)
{
  CField *fld = 0;
  if (curve_.get_rep())
  {
    fld = scinew CField(curve_);
    vector<double>::iterator iter = fld->fdata().begin();
    while (iter != fld->fdata().end()) { (*iter)=value; ++iter; }
  }
  return fld;
}
     
} // End namespace SCIRun

#endif // TriMC_h
