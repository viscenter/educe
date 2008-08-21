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


#include <Core/Algorithms/Fields/FieldsAlgo.h>
#include <Core/Containers/StringUtil.h>

#include <Core/Algorithms/Fields/Mapping/MapFieldDataFromElemToNode.h>
#include <Core/Algorithms/Fields/Mapping/MapFieldDataFromNodeToElem.h>
#include <Core/Algorithms/Fields/Mapping/ApplyMappingMatrix.h>
#include <Core/Algorithms/Fields/FilterFieldData/DilateFieldData.h>
#include <Core/Algorithms/Fields/FilterFieldData/ErodeFieldData.h>
#include <Core/Algorithms/Fields/ClearAndChangeFieldBasis.h>
#include <Core/Algorithms/Fields/ClipBySelectionMask.h>
#include <Core/Algorithms/Fields/ConvertMeshType/ConvertMeshToTetVolMesh.h>
#include <Core/Algorithms/Fields/ConvertMeshType/ConvertMeshToTriSurfMesh.h>
#include <Core/Algorithms/Fields/ConvertMeshType/ConvertMeshToPointCloudMesh.h>
#include <Core/Algorithms/Fields/ConvertMeshType/ConvertMeshToUnstructuredMesh.h>
#include <Core/Algorithms/Fields/ConvertMeshType/ConvertMeshToIrregularMesh.h>
#include <Core/Algorithms/Fields/MapCurrentDensityOntoField.h>
#include <Core/Algorithms/Fields/DistanceField/CalculateDistanceField.h>
#include <Core/Algorithms/Fields/DistanceField/CalculateSignedDistanceField.h>
#include <Core/Algorithms/Fields/DomainFields/GetDomainBoundary.h>
#include <Core/Algorithms/Fields/MeshDerivatives/GetBoundingBox.h>
#include <Core/Algorithms/Fields/MeshDerivatives/GetFieldBoundary.h>
#include <Core/Algorithms/Fields/MeshDerivatives/SplitByConnectedRegion.h>
#include <Core/Algorithms/Fields/FindClosestNodeByValue.h>
#include <Core/Algorithms/Fields/FindClosestNode.h>
#include <Core/Algorithms/Fields/FieldData/GetFieldData.h>
#include <Core/Algorithms/Fields/MeshData/GetMeshNodes.h>
#include <Core/Algorithms/Fields/GetFieldMeasure.h>
#include <Core/Algorithms/Fields/DistanceField/CalculateIsInsideField.h>
#include <Core/Algorithms/Fields/DistanceField/CalculateInsideWhichField.h>
#include <Core/Algorithms/Fields/FieldData/ConvertIndicesToFieldData.h>
#include <Core/Algorithms/Fields/CalculateFieldDataCompiled.h>
#include <Core/Algorithms/Fields/ClipFieldByFunction.h>
#include <Core/Algorithms/Fields/ClipFieldWithSeed.h>
#include <Core/Algorithms/Fields/FieldData/ConvertMappingMatrixToFieldData.h>
#include <Core/Algorithms/Fields/Mapping.h>
#include <Core/Algorithms/Fields/MergeFields/JoinFields.h>
#include <Core/Algorithms/Fields/Cleanup/RemoveUnusedNodes.h>
#include <Core/Algorithms/Fields/TransformMesh/ProjectPointsOntoMesh.h>
#include <Core/Algorithms/Fields/TransformMesh/ScaleFieldMeshAndData.h>
#include <Core/Algorithms/Fields/FieldData/SetFieldData.h>
#include <Core/Algorithms/Fields/MeshData/SetMeshNodes.h>
#include <Core/Algorithms/Fields/DomainFields/SplitNodesByDomain.h>
#include <Core/Algorithms/Fields/TransformMeshWithFunction.h>
#include <Core/Algorithms/Fields/CreateMesh/CreateMeshFromNrrd.h>
#include <Core/Algorithms/Fields/CollectPointClouds.h>
#include <Core/Algorithms/Fields/RefineMesh/RefineMesh.h>
#include <Core/Algorithms/Fields/MeshData/GetMeshQualityField.h>
#include <Core/Algorithms/Fields/FilterFieldData/TriSurfPhaseFilter.h>
#include <Core/Algorithms/Fields/TracePoints/TracePoints.h>



#include <sstream>


namespace SCIRunAlgo {

using namespace SCIRun;

FieldsAlgo::FieldsAlgo(ProgressReporter* pr) :
  AlgoLibrary(pr)
{
}

// Set default for filter parameters
FieldsAlgo::FilterSettings::FilterSettings() :
  num_iterations_(3)
{
}


// Dilate Filter
bool 
FieldsAlgo::ApplyDilateFilterToFieldData(FieldHandle input,
					 FieldHandle& output,
					 FilterSettings& settings)
{
  DilateFieldDataAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_int("num_iterations",settings.num_iterations_);
  return (algo.run(input,output));
}


// Erode Filter
bool 
FieldsAlgo::ApplyErodeFilterToFieldData(FieldHandle input,
					FieldHandle& output,
					FilterSettings& settings)
{
  ErodeFieldDataAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_int("num_iterations",settings.num_iterations_);
  return (algo.run(input,output));
}



// SHOULD REWRITE THIS ONE

bool
FieldsAlgo::SimilarMeshes(ProgressReporter *pr,
			  vector< FieldHandle > field_input_handles )
{
  if (field_input_handles.size() == 0)
  {
    // If we encounter a null pointer we return an error message and return to
    // the program to deal with this error. 
    pr->error("No input fields");
    return (false);
  }

  for( unsigned int i=0; i<field_input_handles.size(); i++ ) {
    FieldHandle fHandle = field_input_handles[i];

    if (fHandle.get_rep() == 0)
    {
      // If we encounter a null pointer we return an error message and
      // return to the program to deal with this error.
      pr->error("No input field");
      return (false);
    }
  }


  // Make sure each field has some data to transform.
  for( unsigned int i=0; i<field_input_handles.size(); i++ ) {
    FieldHandle fHandle = field_input_handles[i];

    if (fHandle->basis_order() == -1)
    {
      pr->error("Field " + to_string(i) + " contains no data to transform.");
      return (false);
    }
  }

  FieldHandle fHandle0 = field_input_handles[0];

  bool same_rep = true;

  // Make sure each field has the same basis and mesh.
  for( unsigned int i=1; i<field_input_handles.size(); i++ ) {
    FieldHandle fHandle = field_input_handles[i];
    
    // Are the basis orders the same?
    if (fHandle0->basis_order() != fHandle->basis_order()) {
      pr->error("The Input Fields must share the same data location.");
      return (false);
    }
    
    // Are the meshes the same?
    if (fHandle0->mesh().get_rep() != fHandle->mesh().get_rep() )
      same_rep = false;
  }

  // If the meshes are not the same make sure they are the same type
  // and have the same number of nodes and elements.
  if( !same_rep ) {

    // Same mesh types?
    for( unsigned int i=1; i<field_input_handles.size(); i++ ) {
      FieldHandle fHandle = field_input_handles[i];
      if( fHandle0->get_type_description(Field::MESH_TD_E)->get_name() !=
	  fHandle->get_type_description(Field::MESH_TD_E)->get_name() ) {
	pr->error("The input fields must have the same mesh type.");
	return (false);
      }
    }

    // Do this last, sometimes takes a while.
    // Code to replace the old FieldCountAlgorithm
    SCIRunAlgo::FieldsAlgo algo(pr);

    Field::size_type num_nodes0, num_elems0;
    if (!(algo.GetFieldInfo(fHandle0,num_nodes0,num_elems0))) return (false);

    // Same number of nodes and elements?
    for( unsigned int i=1; i<field_input_handles.size(); i++ ) {
      FieldHandle fHandle = field_input_handles[i];

      Field::size_type num_nodes, num_elems;
      if (!(algo.GetFieldInfo(fHandle,num_nodes,num_elems))) return (false);

      if( num_nodes0 != num_nodes ) {
        pr->error("The input meshes do not have the same number of nodes.");
        return (false);
      }

      if( num_elems0 != num_elems ) {
        pr->error("The input meshes do not have the same number of elements.");
        return (false);
      }
    }

    pr->warning("The input fields do not have the same mesh,");
    pr->warning("but appear to be the same otherwise.");
  }

  return (true);
}


bool
FieldsAlgo::ChangeFieldBasis(FieldHandle input,
			     FieldHandle& output,
			     MatrixHandle &interpolant,
			     std::string newbasis)
{
  error("ChangeFieldBasis: algorithm not implemented");
  return(false);
}


bool
FieldsAlgo::GetBoundingBox(FieldHandle input,
			   FieldHandle& output)
{
  SCIRunAlgo::GetBoundingBoxAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input,output));
}


bool
FieldsAlgo::ClipFieldBySelectionMask(FieldHandle input,
				     FieldHandle& output,
				     MatrixHandle selmask,
				     MatrixHandle &interpolant)
{
  ClipBySelectionMaskAlgo algo;
  return(algo.ClipBySelectionMask(pr_,input,output,selmask,interpolant));
}

bool 
FieldsAlgo::ClipFieldByMesh(FieldHandle input,
			    FieldHandle& output, 
			    FieldHandle objfield,
			    MatrixHandle &interpolant)
{
  MatrixHandle mask;
  FieldHandle  temp;
  if(!(CalculateIsInsideField(input, temp, objfield, "char"))) {
    return (false);
  }
  if(!(GetFieldData(temp,mask))) return (false);
  return(ClipFieldBySelectionMask(input, output, mask, interpolant));
}


bool
FieldsAlgo::GetFieldBoundary(FieldHandle input,
			     FieldHandle& output,
			     MatrixHandle& mapping)
{
  GetFieldBoundaryAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input,output,mapping));
}


bool
FieldsAlgo::GetFieldInfo(FieldHandle input,
			 Field::size_type& numnodes, 
			 Field::size_type& numelems)
{
  numnodes = input->vmesh()->num_nodes();
  numelems = input->vmesh()->num_elems();
  return (true);
}


bool
FieldsAlgo::GetFieldData(FieldHandle input, MatrixHandle& data)
{
  GetFieldDataAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input,data));
}


bool
FieldsAlgo::SetFieldData(FieldHandle input,
			 FieldHandle& output,
			 MatrixHandle data,
			 bool keepscalartype)
{
  SetFieldDataAlgo algo;
  algo.set_progress_reporter(pr_);
  if (keepscalartype && input->vfield()) 
    algo.set_option("scalardatatype",input->vfield()->get_data_type());
  return(algo.run(input,data,output));
}


bool
FieldsAlgo::SetFieldData(FieldHandle input,
			 FieldHandle& output,
			 NrrdDataHandle data,
			 bool keepscalartype)
{
  SetFieldDataAlgo algo;
  algo.set_progress_reporter(pr_);
  if (keepscalartype && input->vfield()) 
    algo.set_option("scalardatatype",input->vfield()->get_data_type());
  return(algo.run(input,data,output));
}


bool
FieldsAlgo::GetFieldNodes(FieldHandle& input
			  , MatrixHandle& data)
{
  SCIRunAlgo::GetMeshNodesAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input,data));
}

bool
FieldsAlgo::SetFieldNodes(FieldHandle& input,
			  FieldHandle& output,
			  MatrixHandle& data)
{
  SCIRunAlgo::SetMeshNodesAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input,data,output));
}

bool
FieldsAlgo::GetFieldMeasure(FieldHandle input,
			    std::string method,
			    double& measure)
{
  GetFieldMeasureAlgo algo;
  return(algo.GetFieldMeasure(pr_,input,method,measure));
}


bool 
FieldsAlgo::MapFieldDataFromNodeToElem(FieldHandle input,
				       FieldHandle& output, 
				       std::string method)
{
  SCIRunAlgo::MapFieldDataFromNodeToElemAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_option("method",method);
  return(algo.run(input,output));
}


bool
FieldsAlgo::MapFieldDataFromElemToNode(FieldHandle input,
				       FieldHandle& output,
				       std::string method)
{
  SCIRunAlgo::MapFieldDataFromElemToNodeAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_option("method",method);
  return(algo.run(input,output));
}

bool
FieldsAlgo::FindClosestNodeByValue(FieldHandle input, 
				   std::vector<Field::index_type>& output, 
				   FieldHandle& points,
				   double valuemin,
				   double valuemax)
{
  FindClosestNodeByValueAlgo algo;
  return(algo.FindClosestNodeByValue(pr_,input,output,
				     points,valuemin,valuemax));
}

bool
FieldsAlgo::FindClosestNode(FieldHandle input, 
			    std::vector<Field::index_type>& output, 
			    FieldHandle& points)
{
  FindClosestNodeAlgo algo;
  return(algo.FindClosestNode(pr_,input,output,points));
}


bool 
FieldsAlgo::GetDomainBoundary(FieldHandle input,FieldHandle& output, 
			      MatrixHandle domainlink, double minrange, 
			      double maxrange, bool userange, 
			      bool addouterboundary, bool innerboundaryonly, 
			      bool noinnerboundary, bool disconnect)
{
  SCIRunAlgo::GetDomainBoundaryAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_int("min_range",static_cast<int>(minrange));
  algo.set_int("max_range",static_cast<int>(maxrange));
  algo.set_int("domain",static_cast<int>(minrange));
  algo.set_bool("use_range",userange);
  algo.set_bool("add_outer_boundary",addouterboundary);
  algo.set_bool("inner_boundary_only",innerboundaryonly);
  algo.set_bool("no_inner_boundary",noinnerboundary);
  algo.set_bool("disconnect_boundaries",disconnect);

  return(algo.run(input,domainlink,output));
}


bool 
FieldsAlgo::ConvertMeshToTetVol(FieldHandle input, FieldHandle& output)
{
  SCIRunAlgo::ConvertMeshToTetVolMeshAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input, output));
}


bool 
FieldsAlgo::ConvertMeshToTriSurf(FieldHandle input, FieldHandle& output)
{
  SCIRunAlgo::ConvertMeshToTriSurfMeshAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input, output));
}

bool 
FieldsAlgo::CalculateIsInsideField(FieldHandle input, FieldHandle& output, 
				   FieldHandle objectfield, 
				   std::string output_type, 
				   double outval, double inval)
{
  CalculateIsInsideFieldAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_scalar("outside_value",outval);
  algo.set_scalar("inside_value",inval);
  algo.set_option("output_type",output_type);
  return(algo.run(input, objectfield, output));
}

bool 
FieldsAlgo::CalculateInsideWhichField(FieldHandle input, 
				      FieldHandle& output, 
				      std::vector<FieldHandle> objectfields, 
				      std::string output_type,
				      double outval)
{
  CalculateInsideWhichFieldAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_scalar("outside_value",outval);
  algo.set_option("output_type",output_type);
  return(algo.run(input, objectfields, output));
}


bool 
FieldsAlgo::ConvertIndicesToFieldData(FieldHandle input, FieldHandle& output, 
				      MatrixHandle data, std::string datatype)
{
  ConvertIndicesToFieldDataAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_option("datatype",datatype);
  return(algo.run(input, data, output));
}

bool 
FieldsAlgo::MapCurrentDensityOntoField(int numproc, FieldHandle pot, 
				       FieldHandle con, FieldHandle dst, 
				       FieldHandle& output, 
				       std::string mappingmethod,
				       std::string integrationmethod, 
				       std::string integrationfilter, 
				       bool multiply_with_normal, 
				       bool calcnorm)
{
  MapCurrentDensityOntoFieldAlgo algo;
  return (algo.MapCurrentDensityOntoField(pr_, numproc, pot, con, dst, output,
					  mappingmethod, integrationmethod,
					  integrationfilter, 
					  multiply_with_normal, calcnorm));
}

bool 
FieldsAlgo::MapCurrentDensityOntoField(FieldHandle pot, FieldHandle con, 
				       FieldHandle dst,  FieldHandle& output, 
				       std::string mappingmethod,
				       std::string integrationmethod, 
				       std::string integrationfilter, 
				       bool multiply_with_normal, 
				       bool calcnorm)
{
  MapCurrentDensityOntoFieldAlgo algo;
  return (algo.MapCurrentDensityOntoField(pr_, 0, pot, con, dst, output, 
					  mappingmethod, integrationmethod,
					  integrationfilter, 
					  multiply_with_normal,
					  calcnorm));
}


bool
FieldsAlgo::ModalMapping(int numproc, FieldHandle src,
			 FieldHandle dst,
			 FieldHandle& output,
			 std::string mappingmethod,
			 std::string integrationmethod,
			 std::string integrationfilter,
			 double def_value)
{
  ModalMappingAlgo algo;
  return (algo.ModalMapping(pr_,numproc,src,dst,output,mappingmethod,integrationmethod,integrationfilter,def_value));
}

bool
FieldsAlgo::ModalMapping(FieldHandle src,
			 FieldHandle dst,
			 FieldHandle& output,
			 std::string mappingmethod,
			 std::string integrationmethod,
			 std::string integrationfilter,
			 double def_value)
{
  ModalMappingAlgo algo;
  return (algo.ModalMapping(pr_,0,src,dst,output,mappingmethod,integrationmethod,integrationfilter,def_value));
}


bool 
FieldsAlgo::MapFieldDataGradientOntoField(int numproc, FieldHandle src, 
					  FieldHandle dst, FieldHandle& output,
					  std::string mappingmethod,
					  std::string integrationmethod, 
					  std::string integrationfilter, 
					  bool calcnorm)
{
  MapFieldDataGradientOntoFieldAlgo algo;
  return (algo.MapFieldDataGradientOntoField(pr_, numproc, src, dst, output, 
					     mappingmethod, integrationmethod,
					     integrationfilter, calcnorm));
}

bool 
FieldsAlgo::MapFieldDataGradientOntoField(FieldHandle src, FieldHandle dst, 
					  FieldHandle& output, 
					  std::string mappingmethod,
					  std::string integrationmethod, 
					  std::string integrationfilter, 
					  bool calcnorm)
{
  MapFieldDataGradientOntoFieldAlgo algo;
  return (algo.MapFieldDataGradientOntoField(pr_, 0, src, dst, output,
					     mappingmethod, integrationmethod,
					     integrationfilter, calcnorm));
}

bool 
FieldsAlgo::MapFieldDataOntoFieldNodes(int numproc, FieldHandle src, 
				       FieldHandle dst, FieldHandle& output, 
				       std::string mappingmethod, 
				       double def_value)
{
  MapFieldDataOntoFieldNodesAlgo algo;
  return (algo.MapFieldDataOntoFieldNodes(pr_, numproc, src, dst, output,
					  mappingmethod, def_value));
}

bool 
FieldsAlgo::MapFieldDataOntoFieldNodes(FieldHandle src, FieldHandle dst, 
				       FieldHandle& output, 
				       std::string mappingmethod, 
				       double def_value)
{
  MapFieldDataOntoFieldNodesAlgo algo;
  return (algo.MapFieldDataOntoFieldNodes(pr_, 0, src, dst, output,
					  mappingmethod, def_value));
}

bool 
FieldsAlgo::ConvertMappingMatrixToFieldData(FieldHandle input, 
					    FieldHandle& output, 
					    MatrixHandle mappingmatrix)
{
  ConvertMappingMatrixToFieldDataAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input, mappingmatrix, output));
}

bool
FieldsAlgo::MakeEditable(FieldHandle input,FieldHandle& output)
{
  output = input;
  if (!input->mesh()->is_editable()) 
  {
    ConvertMeshToUnstructuredMeshAlgo algo;
    algo.set_progress_reporter(pr_); 
    if(!algo.run(input,output))
    {
      error("MakeEditable: Could not unstructure the mesh");
      return(false);
    }
  }
  return (true);
}


bool
FieldsAlgo::MergeFields(std::vector<FieldHandle> inputs,
			FieldHandle& output,
			double tolerance,
			bool mergefields,
			bool mergeelements,
			bool matchvalue)
{
  JoinFieldsAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_bool("merge_nodes",mergefields);
  algo.set_bool("match_node_values",matchvalue);
  algo.set_scalar("tolerance",tolerance);
  algo.set_bool("make_no_data",false);
  
  return(algo.run(inputs,output));
}

bool
FieldsAlgo::MergeMeshes(std::vector<FieldHandle> inputs,
			FieldHandle& output,
			double tolerance,
			bool mergefields,
			bool mergeelements)
{
  JoinFieldsAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_bool("merge_nodes",mergefields);
  algo.set_scalar("tolerance",tolerance);
  algo.set_bool("make_no_data",true);
  
  return(algo.run(inputs,output));
}

bool
FieldsAlgo::MergeNodes(FieldHandle input,
		       FieldHandle& output,
		       double tolerance,
		       bool mergeelements,
		       bool matchvalue)
{
  std::vector<FieldHandle> inputs(1);
  inputs[0] = input;
  
  JoinFieldsAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_scalar("tolerance",tolerance);
  algo.set_bool("merge_nodes",true);
  algo.set_bool("match_node_values",matchvalue);
  return(algo.run(inputs,output));
}


bool
FieldsAlgo::SplitNodesByDomain(FieldHandle input,
			       FieldHandle& output)
{
  SCIRunAlgo::SplitNodesByDomainAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input,output));
}


bool
FieldsAlgo::SplitFieldByConnectedRegion(FieldHandle input,
					std::vector<FieldHandle>& output)
{
  SCIRunAlgo::SplitByConnectedRegionAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input,output));
}


bool
FieldsAlgo::ConvertMeshToPointCloud(FieldHandle input,
				    FieldHandle& output,
				    bool datalocation)
{
  ConvertMeshToPointCloudMeshAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_option("location","node");
  if (datalocation) algo.set_option("location","data");
  return(algo.run(input,output));
}

bool
FieldsAlgo::ConvertMeshToUnstructuredMesh(FieldHandle input,
					  FieldHandle& output)
{
  ConvertMeshToUnstructuredMeshAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input,output));
}


bool
FieldsAlgo::ConvertMeshToIrregularMesh(FieldHandle input,
				       FieldHandle& output)
{
  ConvertMeshToIrregularMeshAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input,output));
}


bool
FieldsAlgo::ClearAndChangeFieldBasis(FieldHandle input,
				     FieldHandle& output,
				     std::string newbasis)
{
  ClearAndChangeFieldBasisAlgo algo;
  return(algo.ClearAndChangeFieldBasis(pr_,input,output,newbasis));
}

bool 
FieldsAlgo::ScaleFieldMeshAndData(FieldHandle input,
				  FieldHandle& output, 
				  double scaledata,
				  double scalemesh, 
				  bool scale_from_center)
{
  ScaleFieldMeshAndDataAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_scalar("data_scale",scaledata);
  algo.set_scalar("mesh_scale",scalemesh);
  algo.set_bool("scale_from_center",scale_from_center);
  return(algo.run(input, output));
}

bool
FieldsAlgo::BundleToFieldArray(BundleHandle input,
			       std::vector<FieldHandle>& output)
{
  output.resize(input->numFields());
  for (int p=0; p < input->numFields(); p++)
    output[p] = input->getField(input->getFieldName(p));
  return (true);
}


bool
FieldsAlgo::FieldArrayToBundle(std::vector<FieldHandle> input,
			       BundleHandle& output)
{
  output = new Bundle();
  if (output.get_rep() == 0)
  {
    error("FieldArrayToBundle: Could not allocate new bundle");
    return (false);
  }
  
  for (size_t p=0; p < input.size(); p++)
  {
    std::ostringstream oss;
    oss << "field" << p; 
    output->setField(oss.str(),input[p]);
  }
  return (true);
}


bool 
FieldsAlgo::DistanceField(FieldHandle input,
			  FieldHandle& output,
			  FieldHandle object)
{
  SCIRunAlgo::CalculateDistanceFieldAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input,object,output));
}

bool 
FieldsAlgo::SignedDistanceField(FieldHandle input,
				FieldHandle& output,
				FieldHandle object)
{
  SCIRunAlgo::CalculateSignedDistanceFieldAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input,object,output));
}


bool 
FieldsAlgo::RemoveUnusedNodes(FieldHandle input, FieldHandle& output)
{
  RemoveUnusedNodesAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input,output));
}

bool
FieldsAlgo::CleanMesh(FieldHandle input,
		      FieldHandle& output,
		      bool removeunusednodes,
		      bool removeunusedelems,
		      bool reorientelems,
		      bool mergenodes,
		      bool mergeelems)
{
  return (false);
}

bool
FieldsAlgo::CalculateFieldDataCompiled(vector< FieldHandle >& field_input_handles,
				       FieldHandle& field_output_handle,
				       string outputDataType,
				       string function,
				       unsigned int& count)
{
  CalculateFieldDataCompiledAlgo algo;
  return(algo.CalculateFieldDataCompiled(pr_,
					 field_input_handles,
					 field_output_handle,
					 outputDataType,
					 function,
					 count));
}


bool
FieldsAlgo::ClipFieldByFunction(vector< FieldHandle >& field_input_handles,
				FieldHandle& field_output_handle,
				MatrixHandle& matrix_output_handle,
				string gMode,
				string function)
{
  ClipFieldByFunctionAlgo algo;
  return(algo.ClipFieldByFunction(pr_,
				  field_input_handles,
				  field_output_handle,
				  matrix_output_handle,
				  gMode,
				  function));
}

bool
FieldsAlgo::
ClipFieldWithSeed( FieldHandle& field_input_handle,
		   FieldHandle& seed_input_handle,
		   FieldHandle& field_output_handle,
		   MatrixHandle& matrix_output_handle,
		   string mode,
		   string function)
{
  ClipFieldWithSeedAlgo algo;
  return(algo.ClipFieldWithSeed(pr_,
				field_input_handle,
				seed_input_handle,
				field_output_handle,
				matrix_output_handle,
				mode,
				function));
}


bool
FieldsAlgo::TransformMeshWithFunction(FieldHandle& field_input_handle,
				      FieldHandle& field_output_handle,
				      string function)
{
  TransformMeshWithFunctionAlgo algo;
  return(algo.TransformMeshWithFunction(pr_,
					field_input_handle,
					field_output_handle,
					function));
}


bool
FieldsAlgo::CreateMeshFromNrrd( NrrdDataHandle nrrd_point_handle,
				NrrdDataHandle nrrd_connection_handle,
				FieldHandle& field_output_handle,
				string QuadOrTet,
				string StructOrUnstruct)
{
  SCIRunAlgo::CreateMeshFromNrrdAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_option("quad_or_tet",QuadOrTet);
  algo.set_option("struct_or_unstruct",StructOrUnstruct);
  
  return(algo.run(nrrd_point_handle,
				 nrrd_connection_handle,
				 field_output_handle));
}


bool
FieldsAlgo::CollectPointClouds(FieldHandle& field_input_handle,
		   FieldHandle& field_output_handle,
		   int num_fields,
		   int num_nodes,
		   unsigned int& count)
{
  CollectPointCloudsAlgo algo;
  return(algo.CollectPointClouds(pr_, 
				 field_input_handle,
				 field_output_handle,
				 num_fields,
				 num_nodes,
				 count));
}

bool
FieldsAlgo::RefineMesh(FieldHandle input, 
		       FieldHandle& output, 
		       string method,
		       string select, 
		       double isoval)
{
  RefineMeshAlgo algo;
  
  algo.set_progress_reporter(pr_);
  algo.set_option("select",select);
  algo.set_scalar("isoval",isoval);
  if (method == "convex") algo.set_bool("hex_convex",true);
  
  return(algo.run(input,output));
}


bool
FieldsAlgo::GetMeshQualityField(FieldHandle input, 
				FieldHandle& output, 
				std::string metric)
{
  GetMeshQualityFieldAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_option("metric",metric);
  return(algo.run(input,output));
}


bool
FieldsAlgo::ProjectPointsOntoMesh(FieldHandle input, 
				  FieldHandle object, 
				  FieldHandle& output, 
				  string method)
{
  SCIRunAlgo::ProjectPointsOntoMeshAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_option("method",method);
  return(algo.run(input,object,output));
}

bool
FieldsAlgo::ApplyTriSurfPhaseFilter(FieldHandle input,
				    FieldHandle& output,
				    FieldHandle& phaseline,
				    FieldHandle& phasepoint)
{
  TriSurfPhaseFilterAlgo algo;
  algo.set_progress_reporter(pr_);
  return(algo.run(input,output,phaseline,phasepoint));
}

bool
FieldsAlgo::TracePoints(FieldHandle pointcloud,
			FieldHandle old_curvefield,
			FieldHandle& curvefield,
			double val, double tol)
{
  SCIRunAlgo::TracePointsAlgo algo;
  algo.set_progress_reporter(pr_);
  algo.set_scalar("tolerance",tol);
  algo.set_scalar("value",val);
  return(algo.run(pointcloud, old_curvefield, curvefield));
}

} // end namespace SCIRunAlgo

