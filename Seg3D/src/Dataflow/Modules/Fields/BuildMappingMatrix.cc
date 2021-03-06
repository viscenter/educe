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
 *  BuildMappingMatrix.cc:  Build an interpolant field -- a field that says
 *         how to project the data from one field onto the data of a second
 *         field.
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   Jan 2005
 *
 *  Copyright (C) 2005 SCI Institute
 */

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Malloc/Allocator.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Dataflow/Modules/Fields/BuildMappingMatrix.h>
#include <Core/Containers/Handle.h>
#include <Core/Util/DynamicCompilation.h>
#include <iostream>
#include <stdio.h>

namespace SCIRun {

using std::vector;
using std::pair;


class BuildMappingMatrix : public Module
{
  GuiString  interpolation_basis_;
  GuiInt     map_source_to_single_dest_;
  GuiInt     exhaustive_search_;
  GuiDouble  exhaustive_search_max_dist_;
  GuiInt     np_;

public:
  BuildMappingMatrix(GuiContext* ctx);
  virtual ~BuildMappingMatrix();
  virtual void execute();
};

DECLARE_MAKER(BuildMappingMatrix)
BuildMappingMatrix::BuildMappingMatrix(GuiContext* ctx) : 
  Module("BuildMappingMatrix", ctx, Filter, "MiscField", "SCIRun"),
  interpolation_basis_(get_ctx()->subVar("interpolation_basis"), "linear"),
  map_source_to_single_dest_(get_ctx()->subVar("map_source_to_single_dest"), 0),
  exhaustive_search_(get_ctx()->subVar("exhaustive_search"), 0),
  exhaustive_search_max_dist_(get_ctx()->subVar("exhaustive_search_max_dist"), -1.0),
  np_(get_ctx()->subVar("np"), 1)
{
}

BuildMappingMatrix::~BuildMappingMatrix()
{
}

void
BuildMappingMatrix::execute()
{
  FieldHandle fdst_h;
  if (!get_input_handle("Destination", fdst_h)) return;

  if (fdst_h->basis_order() == -1)
  {
    warning("No data location in destination to interpolate to.");
    return;
  }

  FieldHandle fsrc_h;
  if (!get_input_handle("Source", fsrc_h)) return;

  if (fsrc_h->basis_order() == -1)
  {
    warning("No data location in Source field to interpolate from.");
    return;
  }
  
  CompileInfoHandle ci =
    BuildMappingMatrixAlgo::get_compile_info(fsrc_h->mesh()->get_type_description(),
					    fsrc_h->order_type_description(),
					    fdst_h->mesh()->get_type_description(),
					    fdst_h->order_type_description(),
					    fdst_h->get_type_description());
  Handle<BuildMappingMatrixAlgo> algo;
  if (!DynamicCompilation::compile(ci, algo, this)) return;

  fsrc_h->mesh()->synchronize(Mesh::LOCATE_E);
  const int interp_basis = (interpolation_basis_.get() == "linear")?1:0;
  MatrixHandle mh(algo->execute(this,
                                fsrc_h->mesh(), fdst_h->mesh(), interp_basis,
                                map_source_to_single_dest_.get(),
                                exhaustive_search_.get(),
                                exhaustive_search_max_dist_.get(), np_.get()));

  send_output_handle("Mapping", mh);
}

CompileInfoHandle
BuildMappingMatrixAlgo::get_compile_info(const TypeDescription *msrc,
					const TypeDescription *lsrc,
					const TypeDescription *mdst,
					const TypeDescription *ldst,
					const TypeDescription *fdst)
{
  // Use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string template_class_name("BuildMappingMatrixAlgoT");
  static const string base_class_name("BuildMappingMatrixAlgo");

  CompileInfo *rval = 
    scinew CompileInfo(template_class_name + "." +
		       lsrc->get_filename() + "." +
		       ldst->get_filename() + ".",
                       base_class_name, 
                       template_class_name, 
                       msrc->get_name() + ", " +
                       lsrc->get_name() + ", " +
                       mdst->get_name() + ", " +
                       ldst->get_name());

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  msrc->fill_compile_info(rval);
  fdst->fill_compile_info(rval);
  return rval;
}

} // End namespace SCIRun
