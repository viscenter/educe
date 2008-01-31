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
 *  CreateFieldFromMesh.cc:
 *
 *  Create a Field from a mesh.
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

#include <Core/Algorithms/Fields/CreateFieldFromMesh.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>
#include <iostream>


namespace SCIRun {


bool CreateFieldFromMeshAlgo::
CreateFieldFromMesh( ProgressReporter *pr,
		     MeshHandle mHandle,
		     const TypeDescription *btd,
		     const TypeDescription *dtd,
		     FieldHandle& fHandle ) {

  ASSERTMSG(btd != 0, "Basis Type Description not valid");

  CompileInfoHandle ci =
    CreateFieldFromMeshAlgo::get_compile_info(mHandle, btd, dtd, 0);
  
  Handle<CreateFieldFromMeshAlgo> algo;
  
  if(!(SCIRun::DynamicCompilation::compile(ci,algo,pr))) {
    pr->compile_error(ci->filename_);
    SCIRun::DynamicLoader::scirun_loader().cleanup_failed_compile(ci);  
    return(false);
  }

  return (algo->execute( pr, mHandle, fHandle ));
}


CompileInfoHandle
CreateFieldFromMeshAlgo::get_compile_info(MeshHandle mHandle,
					  const TypeDescription *btd,
					  const TypeDescription *dtd,
					  int rank)
{
  // Now create field based on the mesh created above and send it
  const TypeDescription *mtd = mHandle->get_type_description();

  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string base_class_name("CreateFieldFromMeshAlgo");
  const string template_class_name("CreateFieldFromMeshAlgoT");
  
  string extension;
  switch (rank) 
 {
  case 6:
    extension = "Tensor";
    break;
  case 7:
    extension = "Tensor";
    break;
  case 9:
    extension = "Tensor";
    break;
  case 3:
    extension = "Vector";
    break;
  default:
    extension = "Scalar";
    break;
  }

  string dtype_str = rank <= 1 ? "double" : extension;
  string fname = "";
  if (dtd != 0 ) {
    fname = "GenericField<" +mtd->get_name() + ", " + 
      btd->get_similar_name(dtype_str, 0) + ", " + 
      dtd->get_similar_name(dtype_str, 0) + " >";
  } else {
    // build dtd string, assuming that strucutred data use FData
    // and everything else contains vectors of the data
    string data_str = "";

    if (mHandle->topology_geometry() & Mesh::STRUCTURED &&
	mHandle->dimensionality() == 3) 
      data_str = "FData3d<" + dtype_str + "," + mtd->get_name() + " >";
    else if (mHandle->topology_geometry() & Mesh::STRUCTURED &&
	     mHandle->dimensionality() == 2) 
      data_str = "FData2d<" + dtype_str + "," + mtd->get_name() + " >";
    else
      data_str = "vector<" + dtype_str + ">";
    
    fname = "GenericField<" + mtd->get_name() + ", " + 
      btd->get_similar_name(dtype_str, 0) + ", " + 
      data_str + "  >"; 
  }
  
  CompileInfo *rval = 
    scinew CompileInfo(base_class_name + extension + "." +
		       to_filename(fname) + ".",
                       base_class_name,
                       template_class_name,
		       fname + ", " + 
		       mtd->get_name() );
  
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

 
} // End namespace Teem


