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
 *  EdgeMC.h
 *
 *   SCI Institute
 *   University of Utah
 *   Feb 2001
 *
 *  Copyright (C) 2001 SCI Institute
 */


#ifndef EdgeMC_h
#define EdgeMC_h

#include <Core/Algorithms/Visualization/BaseMC.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/CrvLinearLgn.h>
#include <Core/Datatypes/CurveMesh.h>
#include <Core/Datatypes/PointCloudMesh.h>
#include <Core/Datatypes/GenericField.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Geom/GeomPoint.h>
#include <sci_hash_map.h>


namespace SCIRun {

struct EdgeMCBase {
  virtual ~EdgeMCBase() {}
  static const string& get_h_file_path();
};

//! A Marching Square tesselator for a curve line

template<class Field>
class EdgeMC :  public BaseMC, public EdgeMCBase
{
public:
  typedef Field                                  field_type;
  typedef typename Field::mesh_type::Edge::index_type  edge_index_type;
  typedef typename Field::mesh_type::Node::index_type  node_index_type;
  typedef typename Field::value_type             value_type;
  typedef typename Field::mesh_type              mesh_type;
  typedef typename Field::mesh_handle_type       mesh_handle_type;

  typedef PointCloudMesh<ConstantBasis<Point> >  PCMesh;
  typedef ConstantBasis<double>                  PCBasis;
  typedef GenericField<PCMesh, PCBasis, vector<double> > PCField;   


  EdgeMC( Field *field ) : field_(field), mesh_(field->get_typed_mesh()),
			   points_(0), pointcloud_(0) {}

  virtual ~EdgeMC() {}
	
  void reset( int, bool build_field, bool build_geom, bool transparency );
  void extract( edge_index_type, double );

  FieldHandle get_field(double val);

private:
  void extract_n( edge_index_type, double );
  void extract_e( edge_index_type, double );

  PCMesh::Node::index_type find_or_add_edgepoint(SCIRun::index_type u0,
						 SCIRun::index_type u1,
						 double d0,
						 const Point &p) ;

  PCMesh::Node::index_type find_or_add_nodepoint(node_index_type &curve_node_idx);

  void find_or_add_parent(SCIRun::index_type u0, SCIRun::index_type u1,
			  double d0, SCIRun::index_type edge);

  LockingHandle<Field> field_;
  mesh_handle_type mesh_;
  GeomPoints *points_;
  PCMesh::handle_type pointcloud_;
};
  

template<class Field>
void EdgeMC<Field>::reset( int n, bool build_field,
			   bool build_geom,
			   bool transparency )
{
  build_field_ = build_field;
  build_geom_ = build_geom;
  basis_order_ = field_->basis_order();

  edge_map_.clear();
  typename Field::mesh_type::Node::size_type nsize;
  mesh_->size(nsize);
  nnodes_ = nsize;

  cell_map_.clear();
  typename Field::mesh_type::Edge::size_type csize;
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

  points_ = 0;
  if (build_geom_)
  {
    if( transparency )
      points_ = scinew GeomTranspPoints;
    else
      points_ = scinew GeomPoints;
  }
  geomHandle_ = points_;

  pointcloud_ = 0;
  if (build_field_)
  {
    pointcloud_ = scinew PCMesh;
  }
}


template<class Field>
EdgeMC<Field>::PCMesh::Node::index_type
EdgeMC<Field>::find_or_add_edgepoint(SCIRun::index_type u0,
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
    const PCMesh::Node::index_type nodeindex = pointcloud_->add_point(p);
    edge_map_[np] = nodeindex;
    return nodeindex;
  }
  else
  {
    return (*loc).second;
  }
}


template<class Field>
EdgeMC<Field>::PCMesh::Node::index_type
EdgeMC<Field>::find_or_add_nodepoint(node_index_type &curve_node_idx)
{
  PCMesh::Node::index_type point_node_idx;
  SCIRun::index_type i = node_map_[curve_node_idx];
  if (i != -1) point_node_idx = (PCMesh::Node::index_type) i;
  else {
    Point p;
    mesh_->get_point(p, curve_node_idx);
    point_node_idx = pointcloud_->add_point(p);
    node_map_[curve_node_idx] = point_node_idx;
  }
  return curve_node_idx;
}


template<class Field>
void
EdgeMC<Field>::find_or_add_parent(SCIRun::index_type u0, SCIRun::index_type u1,
				  double d0, SCIRun::index_type point) 
{
  if (d0 < 0.0) { u1 = -1; }
  if (d0 > 1.0) { u0 = -1; }
  edgepair_t np;
  if (u0 < u1)  { np.first = u0; np.second = u1; np.dfirst = d0; }
  else { np.first = u1; np.second = u0; np.dfirst = 1.0 - d0; }
  const typename edge_hash_type::iterator loc = edge_map_.find(np);
  if (loc == edge_map_.end())
  {
    edge_map_[np] = point;
  }
  else
  {
    // This should never happen
  }
}


template<class Field>
void EdgeMC<Field>::extract( edge_index_type edge, double v )
{
  if (basis_order_ == 0)
    extract_e(edge, v);
  else
    extract_n(edge, v);
}


template<class Field>
void EdgeMC<Field>::extract_e( edge_index_type edge, double iso )
{
  value_type selfvalue, nbrvalue;
  if (!field_->value( selfvalue, edge )) return;
  typename mesh_type::Node::array_type nodes;
  mesh_->get_nodes(nodes, edge);

  edge_index_type nbr_edge;
  Point p0;
  PCMesh::Node::array_type vertices(1);

  for (size_t i=0; i<nodes.size(); i++)
  {
    if (mesh_->get_neighbor(nbr_edge, edge, nodes[i]) &&
	field_->value(nbrvalue, nbr_edge) &&
	selfvalue <= iso && iso < nbrvalue)
    {
      mesh_->get_center(p0, nodes[i]);
      
      if (build_geom_)
      {
	points_->add(p0);
      }

      if (build_field_)
      {
        vertices[0] = find_or_add_nodepoint(nodes[i]);

	PCMesh::Edge::index_type pcpoint = pointcloud_->add_elem(vertices);

	const double d = (selfvalue - iso) / (selfvalue - nbrvalue);

	find_or_add_parent(edge, nbr_edge, d, pcpoint);
      }
    }
  }
}


template<class Field>
void EdgeMC<Field>::extract_n( edge_index_type edge, double v )
{
  typename mesh_type::Node::array_type node;
  Point p[2];
  value_type value[2];

  mesh_->get_nodes( node, edge );

  static int num[4] = { 0, 1, 1, 0 };

  int code = 0;
  for (int i=0; i<2; i++) {
    mesh_->get_point( p[i], node[i] );
    if (!field_->value( value[i], node[i] )) return;
    code |= (value[i] > v ) << i;
  }

  //  if ( show_case != -1 && (code != show_case) ) return;
  if (num[code])
  {
    const double d0 = (v-value[0])/double(value[1]-value[0]);
    const Point p0(Interpolate(p[0], p[1], d0));

    if (build_geom_)
      points_->add( p0 );

    if (build_field_)
    {
      PCMesh::Node::array_type cnode(1);
      cnode[0] = find_or_add_edgepoint(node[0], node[1], d0, p0);

      pointcloud_->add_elem(cnode);
      cell_map_.push_back( edge );
    }
  }
}


template<class Field>
FieldHandle
EdgeMC<Field>::get_field(double value)
{
  PCField *fld = 0;
  if (pointcloud_.get_rep())
  {
    fld = scinew PCField(pointcloud_);
    vector<double>::iterator iter = fld->fdata().begin();
    while (iter != fld->fdata().end()) { (*iter)=value; ++iter; }
  }
  return fld;
}

} // End namespace SCIRun

#endif // EdgeMC_h
