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
 *  ClipVolumeByIsovalue.cc:  Clip out parts of a field.
 *
 *  Written by:
 *   Allen R. Sanderson
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   March 2001
 *
 *  Copyright (C) 2001 SCI Group
 */


#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Modules/Fields/ClipVolumeByIsovalue.h>
#include <iostream>

namespace SCIRun {

int ClipVolumeByIsovalueAlgo::tet_permute_table[15][4] = {
  { 0, 0, 0, 0 }, // 0x0
  { 3, 0, 2, 1 }, // 0x1
  { 2, 3, 0, 1 }, // 0x2
  { 0, 1, 2, 3 }, // 0x3
  { 1, 2, 0, 3 }, // 0x4
  { 0, 2, 3, 1 }, // 0x5
  { 0, 3, 1, 2 }, // 0x6
  { 0, 1, 2, 3 }, // 0x7
  { 0, 1, 2, 3 }, // 0x8
  { 2, 1, 3, 0 }, // 0x9
  { 3, 1, 0, 2 }, // 0xa
  { 1, 2, 0, 3 }, // 0xb
  { 3, 2, 1, 0 }, // 0xc
  { 2, 3, 0, 1 }, // 0xd
  { 3, 0, 2, 1 }, // 0xe
};


int ClipVolumeByIsovalueAlgo::tri_permute_table[7][3] = {
  { 0, 0, 0 }, // 0x0
  { 2, 0, 1 }, // 0x1
  { 1, 2, 0 }, // 0x2
  { 0, 1, 2 }, // 0x3
  { 0, 1, 2 }, // 0x4
  { 1, 2, 0 }, // 0x5
  { 2, 0, 1 }, // 0x6
};

class ClipVolumeByIsovalue : public Module
{
private:
  GuiDouble  gui_iso_value_min_;
  GuiDouble  gui_iso_value_max_;
  GuiDouble  gui_iso_value_;
  GuiInt     gui_lte_;
  //gui_update_type_ must be declared after gui_iso_value_max_ which is
  //traced in the tcl code. If gui_update_type_ is set to Auto having it
  //last will prevent the net from executing when it is instantiated.
  GuiString  gui_update_type_;

public:
  ClipVolumeByIsovalue(GuiContext* ctx);
  virtual ~ClipVolumeByIsovalue();

  virtual void execute();
};


DECLARE_MAKER(ClipVolumeByIsovalue)


ClipVolumeByIsovalue::ClipVolumeByIsovalue(GuiContext* context)
  : Module("ClipVolumeByIsovalue", context, Filter, "NewField", "SCIRun"),
    gui_iso_value_min_(context->subVar("isoval-min"),  0.0),
    gui_iso_value_max_(context->subVar("isoval-max"), 99.0),
    gui_iso_value_(context->subVar("isoval"), 0.0),
    gui_lte_(context->subVar("lte"), 1),
    gui_update_type_(context->subVar("update_type"), "On Release")
{
}


ClipVolumeByIsovalue::~ClipVolumeByIsovalue()
{
}


void
ClipVolumeByIsovalue::execute()
{
  FieldHandle field_input_handle;
  if( !get_input_handle( "Input", field_input_handle, true ) ) return;

  // Check to see if the input field has changed.
  if( inputs_changed_ ) {

    ScalarFieldInterfaceHandle sfi =
      field_input_handle->query_scalar_interface(this);

    if (!sfi.get_rep()) {
      error("Input field does not contain scalar data.");
      return;
    }

    if (!field_input_handle->mesh()->is_editable()) {
      error("Not an editable mesh type.");
      error("Try passing Field through an Unstructure module first.");
      return;
    }

    pair<double, double> minmax;
    sfi->compute_min_max(minmax.first, minmax.second);

    // Check to see if the gui min max are different than the field.
    if( gui_iso_value_min_.get() != minmax.first ||
	gui_iso_value_max_.get() != minmax.second ) {

      gui_iso_value_min_.set( minmax.first );
      gui_iso_value_max_.set( minmax.second );

      ostringstream str;
      str << get_id() << " set_min_max ";
      get_gui()->execute(str.str().c_str());
    }

    if (field_input_handle->basis_order() != 1)
    {
      error("Isoclipping can only be done for fields with data at nodes.  Note: you can insert a ChangeFieldDataAt module (and an ApplyInterpMatrix module) upstream to push element data to the nodes.");
      return;
    }
  }

  MatrixHandle matrix_input_handle;
  if (get_input_handle("Optional Isovalue", matrix_input_handle, false) )
  {
    if( matrix_input_handle->nrows() == 1 &&
	matrix_input_handle->ncols() == 1 )
    {
      if( matrix_input_handle->get(0, 0) != gui_iso_value_.get() )
      {
	gui_iso_value_.set(matrix_input_handle->get(0, 0));
	gui_iso_value_.reset();

	inputs_changed_ = true;
      }
    }
    else
    {
      error("Input matrix contains more than one value.");
      return;
    }
  }

  if( inputs_changed_ ||
      gui_iso_value_.changed( true ) ||
      !oport_cached("Clipped") ||
      !oport_cached("Mapping") )
  {
    update_state(Executing);
  
    string ext = "";
    const TypeDescription *mtd =
      field_input_handle->mesh()->get_type_description();

    if (mtd->get_name().find("TetVolMesh") != string::npos)
    {
      ext = "Tet";
    }
    else if (mtd->get_name().find("TriSurfMesh") != string::npos)
    {
      ext = "Tri";
    }
    else if (mtd->get_name().find("HexVolMesh") != string::npos)
    {
      ext = "Hex";
    }
    else if (mtd->get_name().find("QuadSurfMesh") != string::npos)
    {
      error("QuadSurfFields are not directly supported in this module.  Please first convert it into a TriSurfField by inserting a SCIRun::FieldsGeometry::QuadToTri module upstream.");
      return;
    }
    else if (mtd->get_name().find("LatVolMesh") != string::npos)
    {
      error("LatVolFields are not directly supported in this module.  Please first convert it into a TetVolField by inserting an upstream SCIRun::FieldsGeometry::Unstructure module, followed by a SCIRun::FieldsGeometry::HexToTet module.");
      return;
    }
    else if (mtd->get_name().find("ImageMesh") != string::npos)
    {
      error("ImageFields are not supported in this module.  Please first convert it into a TriSurfField by inserting an upstream SCIRun::FieldsGeometry::Unstructure module, followed by a SCIRun::FieldsGeometry::QuadToTri module.");
      return;
    }
    else
    {
      error("Unsupported mesh type.  This module only works on Tets and Tris.");
      return;
    }

    const TypeDescription *ftd = field_input_handle->get_type_description();
    CompileInfoHandle ci =
      ClipVolumeByIsovalueAlgo::get_compile_info(ftd, ext);

    Handle<ClipVolumeByIsovalueAlgo> algo;
    if (!DynamicCompilation::compile(ci, algo, false, this))
    {
      error("Unable to compile ClipVolumeByIsovalue algorithm.");
      return;
    }
    
    MatrixHandle matrix_output_handle(0);

    FieldHandle field_output_handle =
      algo->execute(this, field_input_handle,
		    gui_iso_value_.get(), gui_lte_.get(),
		    matrix_output_handle);
  
    send_output_handle("Clipped",  field_output_handle);
    send_output_handle("Mapping", matrix_output_handle);
  }
}

CompileInfoHandle
ClipVolumeByIsovalueAlgo::get_compile_info(const TypeDescription *fsrc,
			      string ext)
{
  // Use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  const string template_class_name("ClipVolumeByIsovalueAlgo" + ext);
  static const string base_class_name("ClipVolumeByIsovalueAlgo");

  CompileInfo *rval = 
    scinew CompileInfo(template_class_name + "." +
		       fsrc->get_filename() + ".",
                       base_class_name, 
                       template_class_name, 
                       fsrc->get_name());

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  rval->add_mesh_include("Core/Datatypes/TriSurfMesh.h");
  rval->add_basis_include("Core/Basis/TriLinearLgn.h");
  fsrc->fill_compile_info(rval);

  return rval;
}


} // End namespace SCIRun

