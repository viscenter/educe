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

#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/Field.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Datatypes/FieldInterface.h>
#include <Dataflow/Modules/Fields/SmoothMesh.h>
#include <Core/Containers/StringUtil.h>
#include <iostream>

namespace SCIRun {

class SmoothMesh : public Module
{
private:
  GuiString smooth_boundary_;
  GuiString smooth_scheme_;
  GuiString smooth_isomethod_;
  GuiDouble smooth_isovalue_;

public:
  SmoothMesh(GuiContext* ctx);
  virtual ~SmoothMesh();

  virtual void execute();
};


DECLARE_MAKER(SmoothMesh)


SmoothMesh::SmoothMesh(GuiContext* ctx)
        : Module("SmoothMesh", ctx, Filter, "ChangeMesh", "SCIRun"),
          smooth_boundary_(get_ctx()->subVar("smoothboundary"), "Off" ),
          smooth_scheme_(get_ctx()->subVar("smoothscheme"), "SmartLaplacian" ),
          smooth_isomethod_(get_ctx()->subVar("isomethod"),""),
          smooth_isovalue_(get_ctx()->subVar("isovalue"),0.0)
{
}


SmoothMesh::~SmoothMesh()
{
}


void
SmoothMesh::execute()
{
  // Get input field.
  FieldHandle ifieldhandle;
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
  
    string ext = "";
    const TypeDescription *mtd = ifieldhandle->mesh()->get_type_description();
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
      ext = "Quad";
    }
    else if (mtd->get_name().find("LatVolMesh") != string::npos)
    {
      warning("SmoothMesh: This module does not support LatVolMeshes (these meshes do not need smoothing)");
      send_output_handle("Smoothed", ifieldhandle);
      return;
    }
    else if (mtd->get_name().find("ImageMesh") != string::npos)
    {
      error("SmoothMesh: This module does not support ImageMeshes (these meshes do not need smoothing)");
      send_output_handle("Smoothed", ifieldhandle);
      return;
    }
    else
    {
      error("SmoothMesh: Unsupported mesh type. This module only works on TetVolMesh, HexVolMesh, TriSurfMesh, and QuadSurfMesh");
      return;
    }

    const TypeDescription *ftd = ifieldhandle->get_type_description();
    CompileInfoHandle ci = SmoothMeshAlgo::get_compile_info(ftd, ext);
    Handle<SmoothMeshAlgo> algo;
    if (!DynamicCompilation::compile(ci, algo, false, this))
    {
      error("Unable to compile SmoothMesh algorithm.");
      return;
    }

    const bool sb = (smooth_boundary_.get() == "On");
    const double isovalue = smooth_isovalue_.get();
    const std::string isomethod = smooth_isomethod_.get();
    const std::string scheme = smooth_scheme_.get(); 
    FieldHandle ofield =
      algo->execute(this, ifieldhandle, sb, scheme, isomethod, isovalue);
    
    send_output_handle("Smoothed", ofield);
  }
}


CompileInfoHandle
SmoothMeshAlgo::get_compile_info(const TypeDescription *fsrc,
                                   string ext)
{
  // Use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  const string template_class_name("SmoothMeshAlgo" + ext);
  static const string base_class_name("SmoothMeshAlgo");

  CompileInfo *rval = 
    scinew CompileInfo(template_class_name + "." +
		       fsrc->get_filename() + ".",
                       base_class_name, 
                       template_class_name, 
                       fsrc->get_name());

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  fsrc->fill_compile_info(rval);

  return rval;
}


} // End namespace SCIRun

