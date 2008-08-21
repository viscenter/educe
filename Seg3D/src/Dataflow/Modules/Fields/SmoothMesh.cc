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
 *  SmoothMesh.cc:  Smooth a given mesh.
 *
 *  Written by:
 *   Jason Shepherd
 *   Department of Computer Science
 *   University of Utah
 *   January 2006
 *
 *  Copyright (C) 2006 SCI Group
 */

#include <Core/Algorithms/Fields/MeshDerivatives/GetFieldBoundary.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Containers/StringUtil.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Modules/Fields/MesquiteMesh.h>
#include <Dataflow/Modules/Fields/MesquiteDomain.h>

#include <sci_hash_map.h>
#include <algorithm>
#include <set>
#include <string>

#include <Mesquite.hpp>
#include <MeshImpl.hpp>
#include <MeshWriter.hpp>
#include <MsqError.hpp>
#include <InstructionQueue.hpp>
#include <LaplacianSmoother.hpp>
#include <SmartLaplacianSmoother.hpp>
#include <TerminationCriterion.hpp>
#include <TopologyInfo.hpp>
#include <UntangleBetaQualityMetric.hpp>
#include <TerminationCriterion.hpp>
#include <IdealWeightInverseMeanRatio.hpp>
#include <LPtoPTemplate.hpp>
#include <FeasibleNewton.hpp>
#include <ConjugateGradient.hpp>

namespace SCIRun {

class SmoothMesh : public Module
{
  public:
    SmoothMesh(GuiContext* ctx);
    virtual ~SmoothMesh() {}

    virtual void execute();

    bool compute_boundary( FieldHandle fieldh,
                           FieldHandle& ofieldh,
                           const std::string scheme,
                           std::string isomethod,
                           double isovalue );

    bool compute_domain_surface( FieldHandle fieldh,
                                 FieldHandle &ofieldh);

    bool laplacian_smoother( FieldHandle fieldh, FieldHandle &ofieldh, 
                             std::string isomethod, double isovalue );
    bool smart_laplacian_smoother( FieldHandle fieldh, FieldHandle &ofieldh,
                                   std::string isomethod, double isovalue );
    bool shape_improvement_wrapper( FieldHandle fieldh, FieldHandle &ofieldh,
                                    std::string isomethod, double isovalue );

    bool smooth_mesh(FieldHandle fieldh,
                        FieldHandle& output_fieldh,
                        bool boundary,
                        std::string scheme,
                        std::string isomethod,
                        double isovalue );
  private:
    GuiString smooth_boundary_;
    GuiString smooth_scheme_;
    GuiString smooth_isomethod_;
    GuiDouble smooth_isovalue_;
    
    SCIRunAlgo::GetFieldBoundaryAlgo field_boundary_algo_;
};


DECLARE_MAKER(SmoothMesh)

SmoothMesh::SmoothMesh(GuiContext* ctx)
        : Module("SmoothMesh", ctx, Filter, "ChangeMesh", "SCIRun"),
          smooth_boundary_(get_ctx()->subVar("smoothboundary"), "Off" ),
          smooth_scheme_(get_ctx()->subVar("smoothscheme"), "SmartLaplacian" ),
          smooth_isomethod_(get_ctx()->subVar("isomethod"),""),
          smooth_isovalue_(get_ctx()->subVar("isovalue"),0.0)
{
   field_boundary_algo_.set_progress_reporter(this);
}

void
SmoothMesh::execute()
{
  // Get input field.
  FieldHandle ifieldhandle;
  FieldHandle ofieldhandle;
  MatrixHandle IsoValue;
  
  get_input_handle("Input", ifieldhandle,true);
  get_input_handle("IsoValue", IsoValue, false);

  if (inputs_changed_ || smooth_boundary_.changed() ||
      smooth_scheme_.changed() || smooth_isomethod_.changed() ||
      smooth_isomethod_.changed() || smooth_isovalue_.changed() ||
      !oport_cached("Smoothed"))
  {
    if (IsoValue.get_rep())
    {
      if (IsoValue->nrows()*IsoValue->ncols() == 1)
      {
        double* data = IsoValue->get_data_pointer();
        smooth_isovalue_.set(data[0]);
        get_ctx()->reset();
      }
    }
  
    const bool sb = (smooth_boundary_.get() == "On");
    const double isovalue = smooth_isovalue_.get();
    const std::string isomethod = smooth_isomethod_.get();
    const std::string scheme = smooth_scheme_.get(); 
      
    if(!(smooth_mesh(ifieldhandle, ofieldhandle, sb, scheme, isomethod, isovalue))) return;
   
    send_output_handle("Smoothed", ofieldhandle);
  }
}


bool
SmoothMesh::smooth_mesh(FieldHandle fieldh,
                        FieldHandle& output_fieldh,
                        bool boundary,
                        std::string scheme,
                        std::string isomethod,
                        double isovalue )
{
  if (fieldh.get_rep() == 0)
  {
    error("No input field.");
    return (false);
  }
  
  if( fieldh->vmesh()->num_elems() == 0 )
  {
    error( "No elements to smooth." );
    return (false);
  }
  
  if ( boundary )
  {
    if(!(compute_boundary( fieldh, fieldh, scheme, isomethod, isovalue )))
    return (false);
  }
  
  if( scheme == "None" )
  {
    output_fieldh = fieldh;
    return (true);
  }
  else if( scheme == "Laplacian" )
  {
    return(laplacian_smoother( fieldh, output_fieldh, isomethod, isovalue));
  }
  else if( scheme == "SmartLaplacian" )
  { 
    return(smart_laplacian_smoother( fieldh, output_fieldh, isomethod, isovalue ));
  }
  else if( scheme == "ShapeImprovement" )
  {
   return(shape_improvement_wrapper( fieldh, output_fieldh, isomethod, isovalue ));
  }
  else
  {
    error( "Unknown Smoothing Scheme." );
    return (false);
  }
}


bool
SmoothMesh::laplacian_smoother( FieldHandle fieldh,
                                FieldHandle& output_fieldh,
                                std::string isomethod,
                                double isovalue )
{
  // Need to make a copy of the field, so that the previous one is not damaged.
  output_fieldh = fieldh->clone();
  // Copy properties to new output field
  output_fieldh->copy_properties(fieldh.get_rep());
  // Get the pointer to the new field
  output_fieldh->mesh_detach();
  
  Mesquite::MsqError err;
  Mesquite::IdealWeightInverseMeanRatio inverse_mean_ratio( err );
  Mesquite::LaplacianSmoother* lapl1 = new Mesquite::LaplacianSmoother( err );

//  Mesquite::QualityAssessor mQA( inverse_mean_ratio, Mesquite::QualityAssessor::MAXIMUM, err );
//  Mesquite::QualityAssessor *mQA = new MeshQualityAssessor( inverse_mean_ratio, 4, err );
  
  // Set stopping criterion.
  Mesquite::TerminationCriterion *m_term = new Mesquite::TerminationCriterion();
  m_term->add_criterion_type_with_int( Mesquite::TerminationCriterion::NUMBER_OF_ITERATES, 10, err );
  lapl1->set_outer_termination_criterion( m_term );
  
  Mesquite::InstructionQueue iqueue;
//  iqueue.add_quality_assessor( mQA, err ); 
//  MSQ_CHKERR(err);
  iqueue.set_master_quality_improver( lapl1, err );
//  MSQ_CHKERR(err);
//  iqueue.add_quality_assessor( mQA, err ); 
//  MSQ_CHKERR(err);  

  if ( err )
  {
    error( "SmoothMesh: Unexpected error from Mesquite code." );
    error( std::string(err.error_message()));
    return (false);
  }
  
  MesquiteMesh entity_mesh(this, output_fieldh, isomethod, isovalue );  

  FieldHandle domain_field_h;
  if (!compute_domain_surface(output_fieldh, domain_field_h ))
  {
    return (false);
  }

  if( domain_field_h.get_rep() )
  {
    MesquiteDomain domain( domain_field_h );
    iqueue.run_instructions( &entity_mesh, &domain, err ); 
  }
  else
  {
    iqueue.run_instructions( &entity_mesh, err ); 
  }

  MSQ_CHKERR(err);
  if (err)
  {
    error("SmoothMesh: Error occurred during Mesquite Laplacian smoothing." );
    error(std::string(err.error_message()));
    return (false);
  }
  
  return (true);
}



bool
SmoothMesh::smart_laplacian_smoother( FieldHandle fieldh,
                                      FieldHandle& output_fieldh,
                                      std::string isomethod,
                                      double isovalue )
{
  // Need to make a copy of the field, so that the previous one is not damaged.
  output_fieldh = fieldh->clone();
  // Copy properties to new output field
  output_fieldh->copy_properties(fieldh.get_rep());
  // Get the pointer to the new field
  output_fieldh->mesh_detach();

  double cull_eps = 1e-4;
  Mesquite::MsqError err;
  Mesquite::SmartLaplacianSmoother sl_smoother(NULL,err);
  // Set stopping criterion.
  Mesquite::TerminationCriterion term_inner, term_outer;
  term_inner.set_culling_type( Mesquite::TerminationCriterion::VERTEX_MOVEMENT_ABSOLUTE, cull_eps, err );
  term_outer.add_criterion_type_with_int( Mesquite::TerminationCriterion::NUMBER_OF_ITERATES, 100, err );
  term_outer.add_criterion_type_with_double( Mesquite::TerminationCriterion::CPU_TIME, 600, err );
  // Sets a culling method on the first QualityImprover.
  sl_smoother.set_inner_termination_criterion(&term_inner);
  sl_smoother.set_outer_termination_criterion(&term_outer);
  Mesquite::InstructionQueue iqueue;
    
  iqueue.disable_automatic_quality_assessment();
  iqueue.disable_automatic_midnode_adjustment();
    
  iqueue.set_master_quality_improver( &sl_smoother, err );  
    
  if ( err )
  {
    error( "Unexpected error from Mesquite code." );
    error( std::string(err.error_message()));
    return (false);
  }

  MesquiteMesh entity_mesh( this, output_fieldh, isomethod, isovalue );  

  FieldHandle domain_field_h;
  if (!compute_domain_surface( output_fieldh, domain_field_h ))
  {
    return (false);
  }

  if (domain_field_h.get_rep())
  {
    MesquiteDomain domain( domain_field_h );
    iqueue.run_instructions( &entity_mesh, &domain, err ); 
  }
  else
  {
    iqueue.run_instructions( &entity_mesh, err ); 
  }

  MSQ_CHKERR(err);
  if (err)
  {
    error( "SmoothMesh: Error occurred during Mesquite smart laplacian smoothing." );
    error( std::string(err.error_message()));
    return (false);
  }

  return (true);
}


bool
SmoothMesh::shape_improvement_wrapper(FieldHandle fieldh,
                                      FieldHandle& output_fieldh,
                                      std::string isomethod,
                                      double isovalue )
{
  // Need to make a copy of the field, so that the previous one is not damaged.
  output_fieldh = fieldh->clone();
  // Copy properties to new output field
  output_fieldh->copy_properties(fieldh.get_rep());
  // Make sure the old and new field do not share a mesh
  output_fieldh->mesh_detach();

  // Arbitrary defined variables.
  double untangle_beta_val=1e-8;
  double unt_successive_eps=1e-10;
  double abs_successive_eps=1.e-3;
  double grad_norm=1.e-6;
  double max_cpu_time=300; // 5 minutes
  
  Mesquite::MsqError err;
  Mesquite::InstructionQueue queue_untangle;
  Mesquite::InstructionQueue queue_shape;

  queue_untangle.disable_automatic_quality_assessment();
  queue_untangle.disable_automatic_midnode_adjustment();
  queue_shape.disable_automatic_quality_assessment();
  queue_shape.disable_automatic_midnode_adjustment();
  
  // Set up untangle metric and function.
  Mesquite::UntangleBetaQualityMetric untangle_metric(untangle_beta_val);
  Mesquite::LPtoPTemplate untangle_function(&untangle_metric, 2, err);
  untangle_function.set_gradient_type(Mesquite::ObjectiveFunction::ANALYTICAL_GRADIENT);

  // Use cg for the untangle function.
  Mesquite::ConjugateGradient untangle_opt(&untangle_function, err);

  untangle_opt.set_patch_type(Mesquite::PatchData::GLOBAL_PATCH, err,1 ,1);
  Mesquite::TerminationCriterion untangle_inner, untangle_outer;
  untangle_inner.add_criterion_type_with_double(Mesquite::TerminationCriterion::QUALITY_IMPROVEMENT_ABSOLUTE,0.0,err);
  untangle_inner.add_criterion_type_with_double(Mesquite::TerminationCriterion::CPU_TIME,max_cpu_time,err);
  untangle_inner.add_criterion_type_with_double(Mesquite::TerminationCriterion::SUCCESSIVE_IMPROVEMENTS_ABSOLUTE,unt_successive_eps,err);
  untangle_outer.add_criterion_type_with_int(Mesquite::TerminationCriterion::NUMBER_OF_ITERATES,1,err);
  untangle_opt.set_inner_termination_criterion(&untangle_inner);
  untangle_opt.set_outer_termination_criterion(&untangle_outer);

  // Set up shape metric and function.
  Mesquite::IdealWeightInverseMeanRatio mean_ratio(err);
  
  mean_ratio.set_averaging_method(Mesquite::QualityMetric::SUM, err);
  Mesquite::LPtoPTemplate obj_func(&mean_ratio, 1, err);
  obj_func.set_gradient_type( Mesquite::ObjectiveFunction::ANALYTICAL_GRADIENT);
  obj_func.set_dividing_by_n(true);

  // Use feasible newton.
  Mesquite::FeasibleNewton shape_opt( &obj_func );
  shape_opt.set_patch_type(Mesquite::PatchData::GLOBAL_PATCH, err);
  Mesquite::TerminationCriterion shape_inner, shape_outer;
  shape_inner.add_criterion_type_with_double(Mesquite::TerminationCriterion::GRADIENT_L2_NORM_ABSOLUTE,grad_norm,err);
  shape_inner.add_criterion_type_with_double(Mesquite::TerminationCriterion::SUCCESSIVE_IMPROVEMENTS_ABSOLUTE,abs_successive_eps,err);
  shape_outer.add_criterion_type_with_int(Mesquite::TerminationCriterion::NUMBER_OF_ITERATES,1,err);
  shape_opt.set_inner_termination_criterion(&shape_inner);
  shape_opt.set_outer_termination_criterion(&shape_outer);

  // Add both to the queue.
  queue_untangle.set_master_quality_improver(&untangle_opt, err);
  queue_shape.set_master_quality_improver(&shape_opt,err);
  if(err)
  {
    error( "Unexpected error from Mesquite code." );
    error( std::string(err.error_message()));
    return (false);
  }
  
  untangle_inner.add_criterion_type_with_double(Mesquite::TerminationCriterion::CPU_TIME,max_cpu_time,err);

  MesquiteMesh entity_mesh( this, output_fieldh, isomethod, isovalue );

  FieldHandle domain_field_h;
  if (!compute_domain_surface(output_fieldh, domain_field_h ))
  {
    return false;
  }
  
  if (domain_field_h.get_rep())
  {
    MesquiteDomain domain( domain_field_h );
    queue_untangle.run_instructions(&entity_mesh, &domain, err);

    MSQ_CHKERR(err);

    if( err )
    {
      error( "Error occurred during Mesquite untangling." );
      error( std::string(err.error_message()));
      return (false);
    }
    else if (untangle_inner.get_current_function_value()>1.e-12)
    {
      error( "Objective function was not zero.  Untangle unsuccessful." );
      return (false);
    }

    shape_inner.add_criterion_type_with_double(Mesquite::TerminationCriterion::CPU_TIME,max_cpu_time,err);
    queue_shape.run_instructions(&entity_mesh, &domain, err);  
    MSQ_CHKERR(err);
    if( err )
    {
      error( "Error occurred during smoothing operation." );
      error( std::string(err.error_message()));
      return (false);
    }
  }
  else
  {
    queue_untangle.run_instructions(&entity_mesh, err);

    MSQ_CHKERR(err);

    if( err )
    {
      error( "Error occurred during Mesquite untangling." );
      error( std::string(err.error_message()));
      return (false);
    }
    else if (untangle_inner.get_current_function_value()>1.e-12)
    {
      error( "Objective function was not zero.  Untangle unsuccessful." );
      return (false);
    }

    shape_inner.add_criterion_type_with_double(Mesquite::TerminationCriterion::CPU_TIME,max_cpu_time,err);
    queue_shape.run_instructions(&entity_mesh,  err);  
    MSQ_CHKERR(err);
    if( err )
    {
      error( "Error occurred during smoothing operation." );
      error( std::string(err.error_message()));
      return (false);
    }
  }

  return (true);
}


bool
SmoothMesh::compute_domain_surface( FieldHandle fieldh,
                                    FieldHandle &ofieldh)
{
  FieldInformation fi(fieldh);
  if (fi.is_trisurfmesh() || fi.is_quadsurfmesh())
  {
    ofieldh = fieldh;
    return (true);
  }
  ofieldh = 0;
  return (true);
}


bool
SmoothMesh::compute_boundary( FieldHandle fieldh,
                              FieldHandle& output_fieldh,
                              const std::string scheme,
                              std::string isomethod,
                              double isovalue )
{
  FieldHandle  boundary_fieldh;
  FieldHandle  smooth_boundary_fieldh; 
  MatrixHandle boundary_mappingh;
  FieldInformation fi(fieldh);
  
  if (fi.is_hexvolmesh() || fi.is_tetvolmesh())
  {
    output_fieldh = fieldh->clone();
    output_fieldh->copy_properties(fieldh.get_rep() );
    output_fieldh->mesh_detach();  
    VMesh* omesh = output_fieldh->vmesh();
    
    if(!(field_boundary_algo_.run(fieldh,boundary_fieldh,boundary_mappingh))) return (false);
    if(!(smooth_mesh(boundary_fieldh,smooth_boundary_fieldh,false,scheme,isomethod,isovalue))) return (false);
    
    VMesh* smooth_boundary = smooth_boundary_fieldh->vmesh();
    VMesh::Node::iterator bound_iter; 
    VMesh::Node::iterator bound_itere; 
    smooth_boundary->vmesh()->begin( bound_iter );
    smooth_boundary->end( bound_itere );
    while( bound_iter != bound_itere )
    {
      VMesh::Node::index_type bi = *bound_iter;
      ++bound_iter;
        
      Matrix::index_type size, stride, *cols;
      double *vals;
      boundary_mappingh->getRowNonzerosNoCopy( bi, size, stride, cols, vals );
        
      Point p;
      smooth_boundary->get_point( p, bi );
      omesh->set_point( p, VMesh::Node::index_type(*cols) );
    }
  }
  else if (fi.is_trisurfmesh() || fi.is_quadsurfmesh())
  {
    output_fieldh = fieldh;
  }
  else
  {
    warning( "Currently unable to smooth the boundary of this mesh." );
    remark( "Proceeding with smoothing of the interior elements." );
  }
  
  return (true);
}



} // End namespace SCIRun

