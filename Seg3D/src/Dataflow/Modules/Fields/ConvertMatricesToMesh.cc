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
 *  ConvertMatricesToMesh: Store/retrieve values from an input matrix to/from 
 *            the data of a field
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   February 2001
 *
 *  Copyright (C) 2001 SCI Institute
 */

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Modules/Fields/ConvertMatricesToMesh.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Core/Containers/Handle.h>
#include <iostream>
#include <stdio.h>

namespace SCIRun {

class ConvertMatricesToMesh : public Module
{
private:
  GuiString gui_fieldname_;
  GuiString gui_meshname_;
  GuiString gui_fieldbasetype_;
  GuiString gui_datatype_;

public:
  ConvertMatricesToMesh(GuiContext* ctx);
  virtual ~ConvertMatricesToMesh();

  virtual void execute();
};


DECLARE_MAKER(ConvertMatricesToMesh)
ConvertMatricesToMesh::ConvertMatricesToMesh(GuiContext* ctx)
  : Module("ConvertMatricesToMesh", ctx, Filter, "NewField", "SCIRun"),
    gui_fieldname_(get_ctx()->subVar("fieldname"), "Created Field"),
    gui_meshname_(get_ctx()->subVar("meshname"), "Created Mesh"),
    gui_fieldbasetype_(get_ctx()->subVar("fieldbasetype"), "TetVolField"),
    gui_datatype_(get_ctx()->subVar("datatype"), "double")
{
}



ConvertMatricesToMesh::~ConvertMatricesToMesh()
{
}



void
ConvertMatricesToMesh::execute()
{
  MatrixHandle elementshandle;
  if (!get_input_handle("Mesh Elements", elementshandle)) return;

  MatrixHandle positionshandle;
  if (!get_input_handle("Mesh Positions", positionshandle)) return;

  MatrixHandle normalshandle;
  if (!get_input_handle("Mesh Normals", normalshandle, false))
  {
    remark("No input normals connected, not used.");
  }

  CompileInfoHandle ci =
      ConvertMatricesToMeshAlgo::get_compile_info(gui_fieldbasetype_.get(),
				       gui_datatype_.get());
  Handle<ConvertMatricesToMeshAlgo> algo;
  if (!DynamicCompilation::compile(ci, algo, false, this))
  {
    error("Could not compile algorithm.");
    return;
  }

  FieldHandle result_field =
    algo->execute(this, elementshandle, positionshandle,
		  normalshandle, 1);

  if (!result_field.get_rep())
  {
    return;
  }

  send_output_handle("Output Field", result_field);
}



CompileInfoHandle
ConvertMatricesToMeshAlgo::get_compile_info(const string &basename,
				 const string &datatype)
{
  // Use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string template_class_name("ConvertMatricesToMeshAlgoT");
  static const string base_class_name("ConvertMatricesToMeshAlgo");


  string ftype;
  string basis_inc;
  string mesh_inc;
  if (basename == "Curve") {
    ftype = "GenericField<CurveMesh<CrvLinearLgn<Point> >, CrvLinearLgn<" + 
      datatype + ">, vector<" + datatype + "> > ";
    basis_inc = "Core/Basis/CrvLinearLgn.h";
    mesh_inc = "Core/Datatypes/CurveMesh.h";
  } else if (basename == "HexVol") {
    ftype = 
      "GenericField<HexVolMesh<HexTrilinearLgn<Point> >, HexTrilinearLgn<" + 
      datatype + ">, vector<" + datatype + "> > ";
    basis_inc = "Core/Basis/HexTrilinearLgn.h";
    mesh_inc = "Core/Datatypes/HexVolMesh.h";
  } else if (basename == "PointCloud") {
    ftype = 
      "GenericField<PointCloudMesh<ConstantBasis<Point> >, ConstantBasis<" +
      datatype + ">, vector<" + datatype + "> > ";
    basis_inc = "Core/Basis/Constant.h";
    mesh_inc = "Core/Datatypes/PointCloudMesh.h";

  } else if (basename == "PrismVol") {
    ftype = 
      "GenericField<PrismVolMesh<PrismLinearLgn<Point> >, PrismLinearLgn<" + 
      datatype + ">, vector<" + datatype + "> > ";
    basis_inc = "Core/Basis/PrismLinearLgn.h";
    mesh_inc = "Core/Datatypes/PrismVolMesh.h";
  } else if (basename == "QuadSurf") {
    ftype = 
      "GenericField<QuadSurfMesh<QuadBilinearLgn<Point> >, QuadBilinearLgn<" +
      datatype + ">, vector<" + datatype + "> > ";
    basis_inc = "Core/Basis/QuadBilinearLgn.h";
    mesh_inc = "Core/Datatypes/QuadSurfMesh.h";
  } else if (basename == "TetVol") {
    ftype = 
      "GenericField<TetVolMesh<TetLinearLgn<Point> >, TetLinearLgn<" +
      datatype + ">, vector<" + datatype + "> > ";
    basis_inc = "Core/Basis/TetLinearLgn.h";
    mesh_inc = "Core/Datatypes/TetVolMesh.h";
  } else if (basename == "TriSurf") {
    ftype = 
      "GenericField<TriSurfMesh<TriLinearLgn<Point> >, TriLinearLgn<" +
      datatype + ">, vector<" + datatype + "> > ";
    basis_inc = "Core/Basis/TriLinearLgn.h";
    mesh_inc = "Core/Datatypes/TriSurfMesh.h";    
  }


  CompileInfo *rval = 
    scinew CompileInfo(template_class_name + "." +
		       to_filename(ftype) + ".",
                       base_class_name, 
                       template_class_name, 
                       ftype);

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  rval->add_basis_include(basis_inc);
  rval->add_mesh_include(mesh_inc);
  rval->add_field_include("Core/Datatypes/GenericField.h");
  rval->add_namespace("SCIRun");

  return rval;
}



} // End namespace SCIRun

