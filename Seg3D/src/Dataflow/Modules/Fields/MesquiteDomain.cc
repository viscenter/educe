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

#include <Dataflow/Modules/Fields/MesquiteDomain.h>

#include <Vector3D.hpp>
#include <MsqError.hpp>
#include <iostream>

namespace SCIRun {

using namespace std;

//! Modifies "coordinate" so that it lies on the
//! domain to which "entity_handle" is constrained.
//! The handle determines the domain.  The coordinate
//! is the proposed new position on that domain.
void
MesquiteDomain::snap_to( Mesquite::Mesh::EntityHandle entity_handle,
                         Mesquite::Vector3D &coordinate) const
{
  Point n_p, new_result;
  n_p.x( coordinate[0] );
  n_p.y( coordinate[1] );
  n_p.z( coordinate[2] );
  VMesh::Elem::index_type face_id;
  double dist;
  domain_mesh_->find_closest_elem(dist, new_result, face_id, n_p );    
  coordinate.set( new_result.x(), new_result.y(), new_result.z() );
}


//! Returns the normal of the domain to which
//! "entity_handle" is constrained.  For non-planar surfaces,
//! the normal is calculated at the point on the domain that
//! is closest to the passed in value of "coordinate".  If the
//! domain does not have a normal, or the normal cannot
//! be determined, "coordinate" is set to (0,0,0).  Otherwise,
//! "coordinate" is set to the domain's normal at the
//! appropriate point.
//! In summary, the handle determines the domain.  The coordinate
//! determines the point of interest on that domain.
void
MesquiteDomain::normal_at( Mesquite::Mesh::EntityHandle entity_handle,
                           Mesquite::Vector3D &coordinate) const
{
  Point n_p, new_result;
  n_p.x( coordinate[0] );
  n_p.y( coordinate[1] );
  n_p.z( coordinate[2] );
  VMesh::Elem::index_type face_id;
  double dist;
  domain_mesh_->find_closest_elem(dist, new_result, face_id, n_p );  
  Vector result;
  VMesh::coords_type bogus;
  bogus.push_back( new_result.x() );
  bogus.push_back( new_result.y() );
  bogus.push_back( new_result.z() );
  domain_mesh_->get_normal( result, bogus, face_id, 0);
  coordinate.set( result.x(), result.y(), result.z() );
}


void
MesquiteDomain::normal_at( const Mesquite::Mesh::EntityHandle* entity_handles,
                           Mesquite::Vector3D coordinates[],
                           unsigned count,
                           Mesquite::MsqError &/*err*/) const
{
  unsigned int i;
  for( i = 0; i < count; ++i )
  {
    normal_at( entity_handles[i], coordinates[i] );
  }
}

 
void
MesquiteDomain::closest_point( Mesquite::Mesh::EntityHandle handle,
                               const Mesquite::Vector3D& position,
                               Mesquite::Vector3D& closest,
                               Mesquite::Vector3D& normal,
                               Mesquite::MsqError& /*err*/ ) const
{
  Point p, close_pt;
  p.x( position[0] );
  p.y( position[1] );
  p.z( position[2] );
  VMesh::Elem::index_type face_id;
  double dist;
  domain_mesh_->find_closest_elem(dist, close_pt, face_id, p );    
  closest.set( close_pt.x(), close_pt.y(), close_pt.z() );
 
  // Need to set the normal.
  Vector result;
  VMesh::coords_type bogus;
  bogus.push_back( close_pt.x() );
  bogus.push_back( close_pt.y() );
  bogus.push_back( close_pt.z() );
  domain_mesh_->get_normal( result, bogus, face_id, 0);
  normal.set( result.x(), result.y(), result.z() );
}


void
MesquiteDomain::domain_DoF( const Mesquite::Mesh::EntityHandle* handle_array,
                            unsigned short* dof_array,
                            size_t num_handles,
                            Mesquite::MsqError& err ) const
{
  size_t i;
  for( i = 0; i < num_handles; ++i )
  {
    dof_array[i] = (short)2;
  }  
}


} //namespace SCIRun
