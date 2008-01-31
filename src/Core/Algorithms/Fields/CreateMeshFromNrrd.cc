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
 *  CreateMeshFromNrrd.cc:
 *
 *  Create a Field Mesh from Nrrd data. Incoming Nrrds may consist of
 *  mesh points and optionally mesh connections.
 *
 *  Written by:
 *   Allen R. Sanderson
 *   SCI Institute
 *   University of Utah
 *   Febuary 2007
 *
 *  Copyright (C) 2007 SCI Institute
 */

#include <Core/Malloc/Allocator.h>

#include <Core/Algorithms/Fields/CreateMeshFromNrrd.h>
#include <Core/Algorithms/Fields/CreateFieldFromMesh.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>
#include <iostream>


namespace SCIRun {

bool CreateMeshFromNrrdAlgo::
CreateMeshFromNrrd( ProgressReporter *pr,
			NrrdDataHandle pHandle,
			NrrdDataHandle cHandle,
			FieldHandle& fHandle,
			string quad_or_tet,
			string struct_or_unstruct ) {

  MeshHandle mHandle = 0;

  // initialize to 1 so it will at least go through i,j,k loops once.
  int idim = 1, jdim = 1, kdim = 1;

  string property;

  unsigned int topology_ = UNKNOWN;
  unsigned int geometry_ = UNKNOWN;

  // Make sure the points coming in make sense.
  if( pHandle->nrrd_->axis[0].size != 3 ) {
    pr->error("Points Nrrd must contain 3D points.");
    pr->error("If the points are 2D use UnuPad to turn in to 3D points.");
    return false;
  }

  //////////////// DETERMINE MESH BASED ON INPUTS AND GUI INFO ////////////////
  const TypeDescription *btd = 0;   //  Basis type def

  // Have points and connections.
  if ( cHandle != 0 ) {

    if (cHandle->nrrd_->dim != 1 && cHandle->nrrd_->dim != 2) {
      pr->error("Connections Nrrd must be two dimensional (number of points in each connection by the number of elements)");
      return false;
    }

    unsigned int which = 0; // which index contains p

    unsigned int nconnections = cHandle->nrrd_->axis[which].size;
      
    if (nconnections != 2 && nconnections != 3 &&
	nconnections != 4 && nconnections != 6 && nconnections != 8) {

      if( cHandle->nrrd_->dim == 1 ) {
	pr->error("Connections nrrd must have one axis with size 2, 3, 4, 6, or 8.");
	return false;

      } else {
	pr->warning("Connections nrrd might not be p x n where p is the number of points in each connection and n is the number of elements.");
	pr->warning("Assuming n x p.");

	which = 1;
	nconnections = cHandle->nrrd_->axis[which].size;
      
	if (nconnections != 2 && nconnections != 3 &&
	    nconnections != 4 && nconnections != 6 && nconnections != 8) {

	  pr->error("Connections nrrd must have one axis with size 2, 3, 4, 6, or 8.");
	  return false;
	}
      }
    }


    // If there is an elem type property make sure it matches what is 
    // found automatically.
    string elem_type = "Auto";

    string property;
    unsigned int nconnections_prop = 0;

    if ( cHandle->get_property( "Elem Type",property )) {
      if( property.find( "Curve" ) != string::npos ) {
	elem_type = "Curve";
	nconnections_prop = 2;
      } else if( property.find( "Tri" ) != string::npos ) {
	elem_type = "Tri";
	nconnections_prop = 3;
      } else if( property.find( "Tet" ) != string::npos ) {
	elem_type = "Tet";
	nconnections_prop = 4;
      } else if( property.find( "Quad" ) != string::npos ) {
	elem_type = "Quad";
	nconnections_prop = 4;
      } else if( property.find( "Prism" ) != string::npos ) {
	elem_type = "Prism";
	nconnections = 6;
      } else if( property.find( "Hex" ) != string::npos ) {
	elem_type = "Hex";
	nconnections_prop = 8;
      }
    }

    if( nconnections_prop == 0 ) {
      if (pHandle->get_property( "Elem Type",property )) {
	pr->warning("Elem Type defined in Points nrrd instead of Connectivity nrrd.");
	if( property.find( "Curve" ) != string::npos ) {
	  elem_type = "Curve";
	  nconnections = 2;
	} else if( property.find( "Tri" ) != string::npos ) {
	  elem_type = "Tri";
	  nconnections_prop = 3;
	} else if( property.find( "Tet" ) != string::npos ) {
	  elem_type = "Tet";
	  nconnections_prop = 4;
	} else if( property.find( "Quad" ) != string::npos ) {
	  elem_type = "Quad";
	  nconnections = 4;
	} else if( property.find( "Prism" ) != string::npos ) {
	  elem_type = "Prism";
	  nconnections_prop = 6;
	} else if( property.find( "Hex" ) != string::npos ) {
	  elem_type = "Hex";
	  nconnections_prop = 8;
	}
      }
    }

    if( nconnections_prop && nconnections_prop != nconnections ) {
      pr->warning("The elem type properties and the number of connections found do not match.");
      pr->warning("Using the number of connections found in the nrrd data.");
    }

    topology_ = UNSTRUCTURED;
    geometry_ = IRREGULAR;

    switch (nconnections) {
    case 2:
      // 2 -> curve
      mHandle = scinew CMesh();
      btd = get_type_description((CrvLinearLgn<double>*)0);
      break;
    case 3:
      // 3 -> tri
      mHandle = scinew TSMesh();
      btd = get_type_description((TriLinearLgn<double>*)0);
      break;
    case 4:
      // 4 -> quad/tet (ask which if this case)
      if (quad_or_tet == "Quad") {
	mHandle = scinew QSMesh();
	btd = get_type_description((QuadBilinearLgn<double>*)0);
      } else  if (quad_or_tet == "Tet") {
	mHandle = scinew TVMesh();
	btd = get_type_description((TetLinearLgn<double>*)0);
      } else if (elem_type == "Tet") {
	mHandle = scinew TVMesh();
	btd = get_type_description((TetLinearLgn<double>*)0);
      } else if (elem_type == "Quad") {
	mHandle = scinew QSMesh();
	btd = get_type_description((QuadBilinearLgn<double>*)0);
      } else {
	pr->error("Auto detection of Elem Type using properties failed.");
	pr->error("Connections Nrrd indicates 4 points per connection. Please indicate whether a Tet or Quad in UI.");
	return false;
      }
      break;
    case 6:
      // 6 -> prism
      mHandle = scinew PVMesh();
      btd = get_type_description((PrismLinearLgn<double>*)0);
      break;
    case 8:
      // 8 -> hex
      mHandle = scinew HVMesh();
      btd = get_type_description((HexTrilinearLgn<double>*)0);
      break;
    }
      
    // create the mesh using points and connections
    const TypeDescription *mtd = mHandle->get_type_description();

    pr->remark( "Creating an unstructured " + mtd->get_name() );
      
    CompileInfoHandle ci =
      CreateMeshFromNrrdAlgo::get_compile_info("Unstructured",
					       mtd,
					       pHandle->nrrd_->type,
					       cHandle->nrrd_->type);
      
    Handle<UnstructuredCreateMeshFromNrrdAlgo> algo;
      
    if(!(SCIRun::DynamicCompilation::compile(ci,algo,pr))) {
      pr->compile_error(ci->filename_);
      SCIRun::DynamicLoader::scirun_loader().cleanup_failed_compile(ci);  
      return(false);
    }

    algo->execute(mHandle, pHandle, cHandle, nconnections, which);

  } else {

    // Have only points.
    geometry_ = IRREGULAR;

    switch (pHandle->nrrd_->dim) {
    case 1:
      topology_ = UNSTRUCTURED;
      // data 1D -> single point in a PointCloud
      mHandle = scinew PCMesh();
      btd = get_type_description((ConstantBasis<double>*)0);
      break;

    case 2:
      // data 1D ask if point cloud or structcurvemesh
      if (struct_or_unstruct == "PointCloud") {
	topology_ = UNSTRUCTURED;
	// data 2D -> PointCloud
	mHandle = scinew PCMesh();
	btd = get_type_description((ConstantBasis<double>*)0);

      } else if (struct_or_unstruct == "StructCurve") {
	topology_ = STRUCTURED;
	// data 2D -> StructCurve
	mHandle = scinew SCMesh( pHandle->nrrd_->axis[1].size );
	btd = get_type_description((CrvLinearLgn<double>*)0);
	idim = pHandle->nrrd_->axis[1].size;

      } else {
	// Try to figure out based on properties of the points
	if (pHandle->get_property( "Topology" , property)) {
	  if( property.find( "Unstructured" ) != string::npos ) {
	    topology_ = UNSTRUCTURED;
	    // data 2D -> PointCloud
	    mHandle = scinew PCMesh();
	    btd = get_type_description((ConstantBasis<double>*)0);
	  } else if( property.find( "Structured" ) != string::npos ) {
	    topology_ = STRUCTURED;
	    // data 2D -> StructCurve
	    mHandle = scinew SCMesh( pHandle->nrrd_->axis[1].size );
	    btd = get_type_description((CrvLinearLgn<double>*)0);
	    idim = pHandle->nrrd_->axis[1].size;
	  }
	}
	
	if( topology_ == UNKNOWN ) {
	  if (pHandle->get_property( "Elem Type", property)) { 
	    if ( property.find( "Curve") != string::npos ) {
	      topology_ = UNSTRUCTURED;
	    // data 2D -> PointCloud
	      mHandle = scinew PCMesh();
	      btd = get_type_description((ConstantBasis<double>*)0);
	    } else if ( property.find( "Curve") != string::npos ) {
	      topology_ = STRUCTURED;
	      // data 2D -> StructCurve
	      mHandle = scinew SCMesh( pHandle->nrrd_->axis[1].size );
	      btd = get_type_description((CrvLinearLgn<double>*)0);
	      idim = pHandle->nrrd_->axis[1].size;
	    }
	  }
	}

	if( topology_ == UNKNOWN ) {
	  pr->warning("Unable to determine if creating Point Cloud or Struct Curve. Defaulting to Point Cloud");
	  mHandle = scinew PCMesh();
	  btd = get_type_description((ConstantBasis<double>*)0);
	  topology_ = UNSTRUCTURED;
	}
      }
      break;

    case 3:
      topology_ = STRUCTURED;
      // data 2D -> StructQuad
      mHandle = scinew SQSMesh(pHandle->nrrd_->axis[1].size,
			       pHandle->nrrd_->axis[2].size);
      btd = get_type_description((QuadBilinearLgn<double>*)0);
      idim = pHandle->nrrd_->axis[1].size;
      jdim = pHandle->nrrd_->axis[2].size;
      //kdim = 1
      break;

    case 4:
      topology_ = STRUCTURED;
      // data 3D -> StructHexVol
      mHandle = scinew SHVMesh(pHandle->nrrd_->axis[1].size,
			       pHandle->nrrd_->axis[2].size,
			       pHandle->nrrd_->axis[3].size);
      btd = get_type_description((HexTrilinearLgn<double>*)0);
      idim = pHandle->nrrd_->axis[1].size;
      jdim = pHandle->nrrd_->axis[2].size;
      kdim = pHandle->nrrd_->axis[3].size;
      break;

    default:
      pr->error("Incorrect dimensions for Data Nrrd");
      return false;
    }

    // Create the mesh
    if (topology_ == UNSTRUCTURED) {

      // This happens only for point clouds with no connectivity
      const TypeDescription *mtd = mHandle->get_type_description();

      pr->remark( "Creating an unstructured " + mtd->get_name() );

      CompileInfoHandle ci =
	CreateMeshFromNrrdAlgo::get_compile_info("Unstructured",
						     mtd,
						     pHandle->nrrd_->type,
						     pHandle->nrrd_->type);

      Handle<UnstructuredCreateMeshFromNrrdAlgo> algo;
	  
      if(!(SCIRun::DynamicCompilation::compile(ci,algo,pr))) {
	pr->compile_error(ci->filename_);
	SCIRun::DynamicLoader::scirun_loader().cleanup_failed_compile(ci);  
	return(false);
      }

      algo->execute(mHandle, pHandle, cHandle, 0, 0);

    } else {
      const TypeDescription *mtd = mHandle->get_type_description();
	  
      pr->remark( "Creating a structured " + mtd->get_name() );
	  
      CompileInfoHandle ci =
	CreateMeshFromNrrdAlgo::get_compile_info("Structured",
						     mtd,
						     pHandle->nrrd_->type,
						     pHandle->nrrd_->type);
      
      Handle<StructuredCreateMeshFromNrrdAlgo> algo;
	  
      if(!(SCIRun::DynamicCompilation::compile(ci,algo,pr))) {
	pr->compile_error(ci->filename_);
	SCIRun::DynamicLoader::scirun_loader().cleanup_failed_compile(ci);  
	return(false);
      }
	  
      algo->execute(mHandle, pHandle, idim, jdim, kdim );
    }
  }

  if( mHandle != 0 ) {
    CompileInfoHandle ci =
      CreateFieldFromMeshAlgo::get_compile_info( mHandle, btd, 0, 1 );
      
    Handle<CreateFieldFromMeshAlgo> algo;
    
    if(!(SCIRun::DynamicCompilation::compile(ci,algo,pr))) {
      pr->compile_error(ci->filename_);
      SCIRun::DynamicLoader::scirun_loader().cleanup_failed_compile(ci);  
      return(false);
    }
  
    return( algo->execute(pr, mHandle, fHandle) );
  } else
    return (false);
}


CompileInfoHandle
CreateMeshFromNrrdAlgo::get_compile_info( const string topoStr,
					  const TypeDescription *mtd,
					  const unsigned int ptype,
					  const unsigned int ctype)
{
  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  const string base_class_name(topoStr + "CreateMeshFromNrrdAlgo");
  const string template_class_name(topoStr + "CreateMeshFromNrrdAlgoT");

  string pTypeStr,  cTypeStr;
  string pTypeName, cTypeName;

  get_nrrd_compile_type( ptype, pTypeStr, pTypeName );
  get_nrrd_compile_type( ctype, cTypeStr, cTypeName );

  CompileInfo *rval = 
    scinew CompileInfo(template_class_name + "." +
		       mtd->get_filename() + "." +
		       pTypeName + "." + cTypeName + ".",
                       base_class_name, 
                       template_class_name,
                       mtd->get_name() + ", " + pTypeStr + ", " + cTypeStr );

  // Add in the include path to compile this obj
  rval->add_include(include_path);

  rval->add_data_include("Core/Geometry/Vector.h");
  rval->add_data_include("Core/Geometry/Tensor.h");

  rval->add_basis_include("Core/Basis/Constant.h");
  rval->add_basis_include("Core/Basis/CrvLinearLgn.h");
  rval->add_basis_include("Core/Basis/TriLinearLgn.h");
  rval->add_basis_include("Core/Basis/QuadBilinearLgn.h");
  rval->add_basis_include("Core/Basis/TetLinearLgn.h");
  rval->add_basis_include("Core/Basis/PrismLinearLgn.h");
  rval->add_basis_include("Core/Basis/HexTrilinearLgn.h");

  rval->add_mesh_include("Core/Datatypes/PointCloudMesh.h");
  rval->add_mesh_include("Core/Datatypes/CurveMesh.h");
  rval->add_mesh_include("Core/Datatypes/TriSurfMesh.h");
  rval->add_mesh_include("Core/Datatypes/QuadSurfMesh.h");
  rval->add_mesh_include("Core/Datatypes/TetVolMesh.h");
  rval->add_mesh_include("Core/Datatypes/PrismVolMesh.h");
  rval->add_mesh_include("Core/Datatypes/HexVolMesh.h");
  rval->add_mesh_include("Core/Datatypes/LatVolMesh.h");
  rval->add_mesh_include("Core/Datatypes/ImageMesh.h");
  rval->add_mesh_include("Core/Datatypes/ScanlineMesh.h");
  rval->add_mesh_include("Core/Datatypes/StructHexVolMesh.h");
  rval->add_mesh_include("Core/Datatypes/StructQuadSurfMesh.h");
  rval->add_mesh_include("Core/Datatypes/StructCurveMesh.h");
  rval->add_container_include("Core/Containers/FData.h");
  rval->add_field_include("Core/Datatypes/GenericField.h");

  mtd->fill_compile_info(rval);
  return rval;
}

} // End namespace SCIRun
