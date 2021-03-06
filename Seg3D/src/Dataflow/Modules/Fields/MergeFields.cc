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
 *  MergeFields.cc:  Insert a field into a TetVolMesh
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   January 2006
 *
 *  Copyright (C) 2006 SCI Group
 */

#include <Core/Util/DynamicCompilation.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Datatypes/FieldInterface.h>
#include <Dataflow/Modules/Fields/MergeFields.h>
#include <Core/Containers/StringUtil.h>
#include <iostream>

namespace SCIRun {


class MergeFields : public Module
{
  int tet_generation_;
  int insert_generation_;

  FieldHandle combined_;
  FieldHandle extended_;
  MatrixHandle mapping_;

public:
  MergeFields(GuiContext* ctx);
  virtual ~MergeFields();

  virtual void execute();

};


DECLARE_MAKER(MergeFields)

MergeFields::MergeFields(GuiContext* ctx)
  : Module("MergeFields", ctx, Filter, "NewField", "SCIRun"),
    tet_generation_(-1),
    insert_generation_(-1)
{
}


MergeFields::~MergeFields()
{
}


void
MergeFields::execute()
{
  FieldHandle tet_field;
  if (!get_input_handle("Container Mesh", tet_field)) return;

  bool tri = false;
  const TypeDescription *ftd0 = tet_field->get_type_description();
  if (ftd0->get_name().find("TriSurfMesh") != string::npos)
  {
    tri = true;
  }
  else if (ftd0->get_name().find("TetVolMesh") == string::npos)
  {
    error("Container Mesh must contain a TetVolMesh or TriSurfMesh.");
  }

  // Get input field 1.
  FieldHandle insert_field;
  if (!get_input_handle("Insert Field", insert_field)) return;

  bool update = false;

  // Check to see if the source field has changed.
  if( tet_generation_ != tet_field->generation ) {
    tet_generation_ = tet_field->generation;
    update = true;
  }

  // Check to see if the source field has changed.
  if( insert_generation_ != insert_field->generation ) {
    insert_generation_ = insert_field->generation;
    update = true;
  }

  if( !combined_.get_rep() || update)
  {
    const TypeDescription *ftd1 = insert_field->get_type_description();

    CompileInfoHandle ci = MergeFieldsAlgo::get_compile_info(ftd0, ftd1, tri);
    Handle<MergeFieldsAlgo> algo;
    if (!DynamicCompilation::compile(ci, algo, this)) {
      error("Dynamic compilation failed.");
      return;
    }

    combined_ = tet_field;
    tet_field = 0;
    combined_.detach();
    combined_->mesh_detach();
    const int dim = insert_field->mesh()->dimensionality();

    vector<unsigned int> added_nodes;
    vector<unsigned int> added_elems;
    if (dim == 0)
    {
      algo->execute_0(combined_, insert_field, added_nodes, added_elems);
    }
    if (dim >= 1)
    {
      algo->execute_1(combined_, insert_field, added_nodes, added_elems);
    }
    if (dim >= 2 && !tri)
    {
      algo->execute_2(combined_, insert_field, added_nodes, added_elems);
    }

    CompileInfoHandle ci2 = MergeFieldsExtract::get_compile_info(ftd0, dim);
    Handle<MergeFieldsExtract> algo2;
    if (!DynamicCompilation::compile(ci2, algo2, this)) {
      error("Dynamic compilation failed.");
      return;
    }
    algo2->extract(extended_, mapping_, combined_, added_nodes, added_elems);
  }

  send_output_handle("Combined Field", combined_, true);
  send_output_handle("Extended Insert Field", extended_, true);
  send_output_handle("Combined To Extended Mapping", mapping_, true);
}


CompileInfoHandle
MergeFieldsAlgo::get_compile_info(const TypeDescription *ftet,
                                  const TypeDescription *finsert,
                                  bool tri)
{
  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string base_class_name("MergeFieldsAlgo");

  CompileInfo *rval = 
    scinew CompileInfo(base_class_name + (tri?"Tri":"Tet") + "." +
		       ftet->get_filename() + "." +
                       finsert->get_filename() + ".",
                       base_class_name, 
                       base_class_name + (tri?"Tri":"Tet"), 
                       ftet->get_name() + ", " +
                       finsert->get_name());

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  ftet->fill_compile_info(rval);
  finsert->fill_compile_info(rval);

  return rval;
}


CompileInfoHandle
MergeFieldsExtract::get_compile_info(const TypeDescription *ftet,
                                     int dim)
{
  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string template_class_name("MergeFieldsExtractT");
  static const string base_class_name("MergeFieldsExtract");

  string outname;
  if (dim == 0)
  {
    outname = "GenericField<PointCloudMesh<ConstantBasis<Point> >, ConstantBasis<double>, vector<double> > ";
  }
  else if (dim == 1)
  {
    outname = "GenericField<CurveMesh<CrvLinearLgn<Point> >, CrvLinearLgn<double>, vector<double> > ";
  }
  else if (dim == 2)
  {
    outname = "GenericField<TriSurfMesh<TriLinearLgn<Point> >, TriLinearLgn<double>, vector<double> > ";
  }
  else if (dim == 3)
  {
    outname = "GenericField<TetVolMesh<TetLinearLgn<Point> >, TetLinearLgn<double>, vector<double> > ";
  }


  CompileInfo *rval = 
    scinew CompileInfo(template_class_name + "." +
		       ftet->get_filename() + "." + to_string(dim) + ".",
                       base_class_name, 
                       template_class_name, 
                       ftet->get_name() + ", " +
                       outname);

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  ftet->fill_compile_info(rval);

  if (dim == 0)
  {
    rval->add_basis_include("Core/Basis/Constant.h");
    rval->add_mesh_include("Core/Datatypes/PointCloudMesh.h");
  }
  else if (dim == 1)
  {
    rval->add_basis_include("Core/Basis/CrvLinearLgn.h");
    rval->add_mesh_include("Core/Datatypes/CurveMesh.h");
  }
  else if (dim == 2)
  {
    rval->add_basis_include("Core/Basis/TriLinearLgn.h");
    rval->add_mesh_include("Core/Datatypes/TriSurfMesh.h");
  }
  else if (dim == 3)
  {
    rval->add_basis_include("Core/Basis/TetLinearLgn.h");
    rval->add_mesh_include("Core/Datatypes/TetVolMesh.h");
  }
  
  return rval;
}


} // End namespace SCIRun

