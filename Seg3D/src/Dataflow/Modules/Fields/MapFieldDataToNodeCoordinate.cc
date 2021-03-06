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
 *  MapFieldDataToNodeCoordinate.cc:  Replace a mesh coordinate with the fdata values
 *
 *  Written by:
 *   David Weinstein
 *   Department of Computer Science
 *   University of Utah
 *   March 2001
 *
 *  Copyright (C) 2001 SCI Group
 */

#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Dataflow/Modules/Fields/MapFieldDataToNodeCoordinate.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Geometry/Transform.h>
#include <Core/Malloc/Allocator.h>
#include <Core/Math/MiscMath.h>
#include <Core/Containers/Handle.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

namespace SCIRun {


class MapFieldDataToNodeCoordinate : public Module
{
  GuiInt gui_coord_;
public:
  MapFieldDataToNodeCoordinate(GuiContext* ctx);
  virtual ~MapFieldDataToNodeCoordinate();

  virtual void execute();
protected:
  int last_generation_;
  int last_gui_coord_;
};


DECLARE_MAKER(MapFieldDataToNodeCoordinate)

MapFieldDataToNodeCoordinate::MapFieldDataToNodeCoordinate(GuiContext* ctx)
  : Module("MapFieldDataToNodeCoordinate", ctx, Filter, "ChangeMesh", "SCIRun"),
    gui_coord_(get_ctx()->subVar("coord"), 2),
    last_generation_(0)
{
}

MapFieldDataToNodeCoordinate::~MapFieldDataToNodeCoordinate()
{
}

void
MapFieldDataToNodeCoordinate::execute()
{
  // Get input field.
  FieldHandle ifield;
  if (!get_input_handle("Input Field", ifield)) return;

  if (ifield->query_scalar_interface(this).get_rep() == 0)
  {
    error("This module only works on scalar fields.");
    return;
  }
  if (ifield->basis_order() != 1) {
    error("This module only works for fields with data at nodes.");
    return;
  }

  string sname = ifield->get_type_description(Field::MESH_TD_E)->get_name();

  if (sname.find("TetVolMesh")     == string::npos &&
      sname.find("HexVolMesh")     == string::npos &&
      sname.find("TriSurfMesh")    == string::npos &&
      sname.find("QuadSurfMesh")   == string::npos &&
      sname.find("CurveMesh")      == string::npos &&
      sname.find("PointCloudMesh") == string::npos) {
    error("Can't change coordinates of this field (mesh) type.");
    return;
  }

  int coord = gui_coord_.get();
  if (coord == 3) {
    if (sname.find("TriSurfMesh")  == string::npos &&
	sname.find("QuadSurfMesh") == string::npos) {
      error("Can't get a normal from this type of mesh.");
      return;
    }
  }
      
  if (last_generation_ != ifield->generation ||
      last_gui_coord_ != coord ||
      !oport_cached("Output Field"))
  {
    last_generation_ = ifield->generation;
    last_gui_coord_ = coord;

    const TypeDescription *ftd = ifield->get_type_description();

    CompileInfoHandle ci = MapFieldDataToNodeCoordinateAlgo::get_compile_info(ftd);

    Handle<MapFieldDataToNodeCoordinateAlgo> algo;
    if (!module_dynamic_compile(ci, algo)) return;
    
    FieldHandle ofield(algo->execute(ifield, coord));
    
    send_output_handle("Output Field", ofield);
  }
}


CompileInfoHandle
MapFieldDataToNodeCoordinateAlgo::get_compile_info(const TypeDescription *field_td)
{
  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string template_class_name("MapFieldDataToNodeCoordinateAlgoT");
  static const string base_class_name("MapFieldDataToNodeCoordinateAlgo");

  CompileInfo *rval = 
    scinew CompileInfo(template_class_name + "." +
		       field_td->get_filename() + ".",
                       base_class_name, 
                       template_class_name, 
                       field_td->get_name());

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  field_td->fill_compile_info(rval);
  return rval;
}

} // End namespace SCIRun
