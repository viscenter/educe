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

#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/VMesh.h>

#include <Core/Geometry/Transform.h>
#include <Core/Geometry/BBox.h>

namespace SCIRun {

LockingHandle<Mesh>
VMesh::mesh()
{
  return (MeshHandle(0));
}

void 
VMesh::size(Node::size_type& size) const
{
  size = 0;
}

void 
VMesh::size(ENode::size_type& size) const
{
  size = 0;
}

void 
VMesh::size(Edge::size_type& size) const
{
  size = 0;
}

void 
VMesh::size(Face::size_type& size) const
{
  size = 0;
}

void 
VMesh::size(Cell::size_type& size) const
{
  size = 0;
}

void 
VMesh::size(Elem::size_type& size) const
{
  size = 0;
}

void 
VMesh::size(DElem::size_type& size) const
{
  size = 0;
}
  
void 
VMesh::get_nodes(Node::array_type& nodes, Node::index_type i) const
{
  nodes.resize(1);
  nodes[0] = i;
}
  
void 
VMesh::get_nodes(Node::array_type& nodes, Edge::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,Edge::index_type) has not been implemented");
}

void 
VMesh::get_nodes(Node::array_type& nodes, Face::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,Face::index_type) has not been implemented");
}

void 
VMesh::get_nodes(Node::array_type& nodes, Cell::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,Cell::index_type) has not been implemented");
}

void 
VMesh::get_nodes(Node::array_type& nodes, Elem::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,Elem::index_type) has not been implemented");
}  

void 
VMesh::get_nodes(Node::array_type& nodes, DElem::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,DElem::index_type) has not been implemented");
}



void 
VMesh::get_enodes(ENode::array_type& nodes, Node::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_nodesl(ENode::array_type,Node::index_type) has not been implemented");
}
  
void 
VMesh::get_enodes(ENode::array_type& nodes, Edge::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_nodes(VNLode::array_type,Edge::index_type) has not been implemented");
}

void 
VMesh::get_enodes(ENode::array_type& nodes, Face::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,Face::index_type) has not been implemented");
}

void 
VMesh::get_enodes(ENode::array_type& nodes, Cell::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,Cell::index_type) has not been implemented");
}

void 
VMesh::get_enodes(ENode::array_type& nodes, Elem::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,Elem::index_type) has not been implemented");
}  

void 
VMesh::get_enodes(ENode::array_type& nodes, DElem::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_nodes(Node::array_type,DElem::index_type) has not been implemented");
}



void 
VMesh::get_edges(Edge::array_type& edges, Node::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_edges(Edge::array_type,Node::index_type) has not been implemented");
}

void 
VMesh::get_edges(Edge::array_type& edges, Edge::index_type i) const
{
  edges.resize(1);
  edges[0] = i;
}

void 
VMesh::get_edges(Edge::array_type& edges, Face::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_edges(Edge::array_type,Face:index_type) has not been implemented");
}

void 
VMesh::get_edges(Edge::array_type& edges, Cell::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_edges(Edge::array_type,Cell::index_type) has not been implemented");
}

void 
VMesh::get_edges(Edge::array_type& edges, Elem::index_type i) const
{  
  ASSERTFAIL("VMesh interface: get_edges(Edge::array_type,Elem::index_type) has not been implemented");
}

void 
VMesh::get_edges(Edge::array_type& edges, DElem::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_edges(Edge::array_type,DElem::index_type) has not been implemented");
}


void 
VMesh::get_faces(Face::array_type& faces, Node::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_faces(Face::array_type,Node::index_type) has not been implemented");
}

void 
VMesh::get_faces(Face::array_type& faces, Edge::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_faces(Face::array_type,Edge::index_type) has not been implemented");
}

void 
VMesh::get_faces(Face::array_type& faces, Face::index_type i) const
{
  faces.resize(1);
  faces[0] = i;
}

void 
VMesh::get_faces(Face::array_type& faces, Cell::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_faces(Face::array_type,Cell::index_type) has not been implemented");
}

void 
VMesh::get_faces(Face::array_type& faces, Elem::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_faces(Face::array_type,Elem::index_type) has not been implemented");
}

void 
VMesh::get_faces(Face::array_type& faces, DElem::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_faces(Face::array_type,DElem::index_type) has not been implemented");
}



void 
VMesh::get_cells(Cell::array_type& cells, Node::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_cells(Cell::array_type,Node::index_type) has not been implemented");
}

void 
VMesh::get_cells(Cell::array_type& cells, Edge::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_cells(Cell::array_type,Edge::index_type) has not been implemented");
}

void 
VMesh::get_cells(Cell::array_type& cells, Face::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_cells(Cell::array_type,Face::index_type) has not been implemented");
}

void 
VMesh::get_cells(Cell::array_type& cells, Cell::index_type i) const
{
  cells.resize(1);
  cells[0] = i;
}

void 
VMesh::get_cells(Cell::array_type& cells, Elem::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_cells(Cell::array_type,Elem::index_type) has not been implemented");
}

void 
VMesh::get_cells(Cell::array_type& cells, DElem::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_cells(Cell::array_type,DElem::index_type) has not been implemented");
}

  
  
void 
VMesh::get_elems(Elem::array_type& elems, Node::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_elems(Elem::array_type,Node::index_type) has not been implemented");
}

void 
VMesh::get_elems(Elem::array_type& elems, Edge::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_elems(Elem::array_type,Edge::index_type) has not been implemented");
}

void 
VMesh::get_elems(Elem::array_type& elems, Face::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_elems(Elem::array_type,Face::index_type) has not been implemented");
}

void 
VMesh::get_elems(Elem::array_type& elems, Cell::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_elems(Elem::array_type,Cell::index_type) has not been implemented");
}

void 
VMesh::get_elems(Elem::array_type& elems, Elem::index_type i) const
{
  elems.resize(1);
  elems[0] = i;
}

void 
VMesh::get_elems(Elem::array_type& elems, DElem::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_elems(Elem::array_type,DElem::index_type) has not been implemented");
}



void 
VMesh::get_delems(DElem::array_type& delems, Node::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_delems(DElem::array_type,Node::index_type) has not been implemented");
}

void 
VMesh::get_delems(DElem::array_type& delems, Edge::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_delems(DElem::array_type,Edge::index_type) has not been implemented");
}

void 
VMesh::get_delems(DElem::array_type& delems, Face::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_delems(DElem::array_type,Face::index_type) has not been implemented");
}

void 
VMesh::get_delems(DElem::array_type& delems, Cell::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_delems(DElem::array_type,Cell::index_type) has not been implemented");
}

void 
VMesh::get_delems(DElem::array_type& delems, Elem::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_delems(DElem::array_type,Elem::index_type) has not been implemented");
}

void 
VMesh::get_delems(DElem::array_type& delems, DElem::index_type i) const
{
  delems.resize(1);
  delems[0] = i;
}

void 
VMesh::get_center(Point &point, Node::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_center(Point,Node::index_type) has not been implemented");
}

void 
VMesh::get_center(Point &point, ENode::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_center(Point,ENode::index_type) has not been implemented");
}

void 
VMesh::get_center(Point &point, Edge::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_center(Point,Edge::index_type) has not been implemented");
}

void 
VMesh::get_center(Point &point, Face::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_center(Point,Face::index_type) has not been implemented");
}

void 
VMesh::get_center(Point &point, Cell::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_center(Point,Cell::index_type) has not been implemented");
}

void 
VMesh::get_center(Point &point, Elem::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_center(Point,Elem::index_type) has not been implemented");
}

void 
VMesh::get_center(Point &point, DElem::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_center(Point,DElem::index_type) has not been implemented");
}

void 
VMesh::get_centers(points_type &points, Node::array_type& array) const
{
  ASSERTFAIL("VMesh interface: get_centers(points_type,Node::array_type) has not been implemented");
}

void 
VMesh::get_centers(points_type &points, Elem::array_type& array) const
{
  ASSERTFAIL("VMesh interface: get_centers(points_type,Elem::array_type) has not been implemented");
}

double 
VMesh::get_size(VMesh::Edge::index_type i) const
{
  return (0.0);
}

double 
VMesh::get_size(VMesh::Face::index_type i) const
{
  return (0.0);
} 

double 
VMesh::get_size(VMesh::Cell::index_type i) const
{
  return (0.0);
}

double 
VMesh::get_size(VMesh::Elem::index_type i) const
{
  return (0.0);
}

double 
VMesh::get_size(VMesh::DElem::index_type i) const
{
  return (0.0);
}
  
bool 
VMesh::locate(Node::index_type &i, const Point &point) const
{
  ASSERTFAIL("VMesh interface: locate(Node::index_type,Point) has not been implemented");
}

bool 
VMesh::locate(Elem::index_type &i, const Point &point) const
{
  ASSERTFAIL("VMesh interface: locate(Elem::index_type,Point) has not been implemented");
}

void 
VMesh::mlocate(vector<Node::index_type> &i, const vector<Point> &point) const
{
  ASSERTFAIL("VMesh interface: mlocate(vector<Node::index_type>,Point) has not been implemented");
}

void 
VMesh::mlocate(vector<Elem::index_type> &i, const vector<Point> &point) const
{
  ASSERTFAIL("VMesh interface: mlocate(vector<Elem::index_type>,Point) has not been implemented");
}


double 
VMesh::find_closest_elem(Point& result,
                         VMesh::Elem::index_type &i, 
                         const Point &point) const
{
  ASSERTFAIL("VMesh interface: find_closest_elem(Point,Elem::index_type,Point) has not been implemented");
}

double 
VMesh::find_closest_elems(Point& result,
                          VMesh::Elem::array_type &i, 
                          const Point &point) const
{
  ASSERTFAIL("VMesh interface: find_closest_elems(Point,Elem::array_type,Point) has not been implemented");
}

  
bool 
VMesh::get_coords(coords_type &coords, const Point &point, Elem::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_coords(coords_type,Point,Elem::index_type) has not been implemented");
}


void 
VMesh::interpolate(Point &p, const coords_type &coords, Elem::index_type i) const
{
  ASSERTFAIL("VMesh interface: interpolate(Point,coords_type,Elem::index_type) has not been implemented");
}

void 
VMesh::minterpolate(vector<Point> &p, const vector<coords_type> &coords, Elem::index_type i) const
{
  ASSERTFAIL("VMesh interface: minterpolate(vector<Point>,vector<coords_type>,Elem::index_type) has not been implemented");
}


void 
VMesh::derivate(dpoints_type &p, const coords_type &coords, Elem::index_type i) const
{
  ASSERTFAIL("VMesh interface: derivate(dpoints_type,coords_type,Elem::index_type) has not been implemented");
}


void 
VMesh::get_normal(Vector &result, coords_type &coords, Elem::index_type eidx, unsigned int f) const
{
  ASSERTFAIL("VMesh interface: get_normal() has not been implemented");
}  


void 
VMesh::get_random_point(Point &point, Elem::index_type i,FieldRNG &rng) const
{
  ASSERTFAIL("VMesh interface: get_random_point(Point,Elem::index_type) has not been implemented");
}

void 
VMesh::set_point(const Point &point, Node::index_type i)
{
  ASSERTFAIL("VMesh interface: set_point(Point,Node::index_type) has not been implemented");
}
  
void 
VMesh::set_point(const Point &point, ENode::index_type i)
{
  ASSERTFAIL("VMesh interface: set_point(Point,ENode::index_type) has not been implemented");
}  

void 
VMesh::node_reserve(size_t size)
{
  ASSERTFAIL("VMesh interface: node_reserve(size_t size) has not been implemented");
}

void 
VMesh::elem_reserve(size_t size)
{
  ASSERTFAIL("VMesh interface: elem_reserve(size_t size) has not been implemented");
}

void 
VMesh::resize_nodes(size_t size)
{
  ASSERTFAIL("VMesh interface: resize_nodes(size_t size) has not been implemented");
}

void 
VMesh::resize_elems(size_t size)
{
  ASSERTFAIL("VMesh interface: resize_elems(size_t size) has not been implemented");
}


void 
VMesh::add_node(const Point &point, Node::index_type &i)
{
  ASSERTFAIL("VMesh interface: this mesh cannot be edited (add_node)");  
}

void 
VMesh::add_enode(const Point &point, ENode::index_type &i)
{
  ASSERTFAIL("VMesh interface: this mesh cannot be edited (add_enode)");  
}

void  
VMesh::add_elem(const Node::array_type &nodes, Elem::index_type &i)
{
  ASSERTFAIL("VMesh interface: this mesh cannot be edited (add_elem)");  
}

bool
VMesh::get_neighbor(Elem::index_type &neighbor, Elem::index_type elem, DElem::index_type delem) const
{
  ASSERTFAIL("VMesh interface: get_neighbor(Elem::index_type,Elem::index_type,DElem::index_type) has not been implemented");  
}

void
VMesh::get_neighbors(Elem::array_type &elems, Elem::index_type elem) const
{
  ASSERTFAIL("VMesh interface: get_neighbors(Elem::index_type,Elem::index_type) has not been implemented");  
}

bool
VMesh::get_neighbors(Elem::array_type &elems, Elem::index_type elem, DElem::index_type delem) const
{
  ASSERTFAIL("VMesh interface: get_neighbors(Elem::array_type,Elem::index_type,DElem::index_type) has not been implemented");  
}

void
VMesh::get_neighbors(Node::array_type &nodes, Node::index_type node) const
{
  ASSERTFAIL("VMesh interface: get_neighbors(Node::index_type,Node::index_type) has not been implemented");  
}

void 
VMesh::pwl_approx_edge(vector<coords_type > &coords, Elem::index_type ci, unsigned int which_edge, unsigned int div_per_unit) const
{
  ASSERTFAIL("VMesh interface: pwl_appprox_edge has not been implemented");  
}

void 
VMesh::pwl_approx_face(vector<vector<coords_type > > &coords, Elem::index_type ci, unsigned int which_face, unsigned int div_per_unit) const
{
  ASSERTFAIL("VMesh interface: pwl_appprox_face has not been implemented");  
}

void 
VMesh::get_normal(Vector& norm,Node::index_type i) const
{
  ASSERTFAIL("VMesh interface: get_normal has not been implemented");  
}

double
VMesh::det_jacobian(const coords_type& coords, Elem::index_type idx) const
{
  ASSERTFAIL("VMesh interface: det_jacobian has not been implemented");
}

void
VMesh::jacobian(const coords_type& coords, Elem::index_type idx, double *J) const
{
  ASSERTFAIL("VMesh interface: jacobian has not been implemented");
}

double
VMesh::inverse_jacobian(const coords_type& coords, Elem::index_type idx, double* Ji) const
{
  ASSERTFAIL("VMesh interface: inverse_jacobian has not been implemented");
}

double
VMesh::scaled_jacobian_metric(Elem::index_type idx) const
{
  ASSERTFAIL("VMesh interface: scaled_jacobian_metric has not yet been implemented");
}

double
VMesh::jacobian_metric(Elem::index_type idx) const
{
  ASSERTFAIL("VMesh interface: jacobian_metric has not yet been implemented");
}

void
VMesh::get_dimensions(dimension_type& dim)
{
  dim.resize(1);
  Node::size_type sz;
  size(sz);
  dim[0] = sz;
}

void
VMesh::get_elem_dimensions(dimension_type& dim)
{
  dim.resize(1);
  Elem::size_type sz;
  size(sz);
  dim[0] = sz;
}


BBox
VMesh::get_bounding_box() const
{
  ASSERTFAIL("VMesh interface: get_bounding_box has not yet been implemented");  
}

bool
VMesh::synchronize(unsigned int sync)
{
  ASSERTFAIL("VMesh interface: synchronize has not yet been implemented");  
}

bool
VMesh::unsynchronize(unsigned int sync)
{
  ASSERTFAIL("VMesh interface: synchronize has not yet been implemented");  
}


void 
VMesh::transform(const Transform &t)
{
  ASSERTFAIL("VMesh interface: transform has not yet been implemented");  
}

Transform 
VMesh::get_transform() const
{
  ASSERTFAIL("VMesh interface: get_transform has not yet been implemented");  
}
 
void 
VMesh::get_canonical_transform(Transform &t)
{
  ASSERTFAIL("VMesh interface: get_canonical_transform has not yet been implemented");  
}

double
VMesh::get_epsilon() const
{
  ASSERTFAIL("VMesh interface: get_epsilon has not yet been implemented");  
}


void 
VMesh::get_interpolate_weights(const coords_type& coords, 
                               Elem::index_type elem, 
                               ElemInterpolate& ei,
                               int basis_order) const
{
  ASSERTFAIL("VMesh interface: get_interpolate_weights has not yet been implemented");  
}
                                       
void 
VMesh::get_interpolate_weights(const Point& p, 
                               ElemInterpolate& ei,
                               int basis_order) const
{
  ASSERTFAIL("VMesh interface: get_interpolate_weights has not yet been implemented");  
}
        

void 
VMesh::get_minterpolate_weights(const vector<coords_type>& coords, 
                                Elem::index_type elem, 
                                MultiElemInterpolate& ei,
                                int basis_order) const
{
  ASSERTFAIL("VMesh interface: get_minterpolate_weights has not yet been implemented");  
}
                                       
void 
VMesh::get_minterpolate_weights(const vector<Point>& p, 
                                MultiElemInterpolate& ei,
                                int basis_order) const
{
  ASSERTFAIL("VMesh interface: get_minterpolate_weights has not yet been implemented");  
}                                       
                                                                                                     

void 
VMesh::get_gradient_weights(const coords_type& coords, 
                            Elem::index_type elem, 
                            ElemGradient& eg,
                            int basis_order) const
{
  ASSERTFAIL("VMesh interface: get_gradient_weights has not yet been implemented");  
}

void 
VMesh::get_gradient_weights(const Point& p, 
                            ElemGradient& eg,
                            int basis_order) const
{
  ASSERTFAIL("VMesh interface: get_gradient_weights has not yet been implemented");  
}
                                       

void 
VMesh::get_mgradient_weights(const vector<coords_type>& coords, 
                             Elem::index_type elem, 
                             MultiElemGradient& eg,
                             int basis_order) const
{
  ASSERTFAIL("VMesh interface: get_mgradient_weights has not yet been implemented");  
}

void 
VMesh::get_mgradient_weights(const vector<Point>& p, 
                             MultiElemGradient& eg,
                             int basis_order) const
{
  ASSERTFAIL("VMesh interface: get_mgradient_weights has not yet been implemented");  
}

void 
VMesh::get_weights(const coords_type& coords, 
                   vector<double>& weights, 
                   int basis_order) const                                 
{
  ASSERTFAIL("VMesh interface: get_weights has not yet been implemented");  
}  

void 
VMesh::get_derivate_weights(const coords_type& coords, 
                            vector<double>& weights,
                            int basis_order) const   
{
  ASSERTFAIL("VMesh interface: get_derivate_weights has not yet been implemented");  
}  


void 
VMesh::get_linear_weights(coords_type& coords, vector<double>& weights)
{
  ASSERTFAIL("VMesh interface: get_linear_weights has not yet been implemented");  
}


void 
VMesh::get_quadratic_weights(coords_type& coords, vector<double>& weights)
{
  ASSERTFAIL("VMesh interface: get_quadratic_weights has not yet been implemented");  
}

void 
VMesh::get_cubic_weights(coords_type& coords, vector<double>& weights)
{
  ASSERTFAIL("VMesh interface: get_cubic_weights has not yet been implemented");  
}

void 
VMesh::get_linear_derivate_weights(coords_type& coords, vector<double>& weights)
{
  ASSERTFAIL("VMesh interface: get_linear_derivate_weights has not yet been implemented");  
}

void 
VMesh::get_quadratic_derivate_weights(coords_type& coords, vector<double>& weights)
{
  ASSERTFAIL("VMesh interface: get_quadratic_derivate_weights has not yet been implemented");  
}

void 
VMesh::get_cubic_derivate_weights(coords_type& coords, vector<double>& weights)
{
  ASSERTFAIL("VMesh interface: get_cubic_derivate_weights has not yet been implemented");  
}

void 
VMesh::get_gaussian_scheme(vector<VMesh::coords_type>& coords, 
                           vector<double>& weights, int order) const
{
  ASSERTFAIL("VMesh interface: get_gaussian_scheme has not yet been implemented");  
}
                                   
void 
VMesh::get_regular_scheme(vector<VMesh::coords_type>& coords, 
                                  vector<double>& weights, int order) const
{
  ASSERTFAIL("VMesh interface: get_regular_scheme has not yet been implemented");  
}

} // end namespace

