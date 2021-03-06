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
 *  CalculateVectorMagnitudes.cc:  Unfinished modules
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   March 2001
 *
 *  Copyright (C) 2001 SCI Group
 */

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Containers/Handle.h>

#include <Dataflow/Modules/Fields/CalculateVectorMagnitudes.h>

namespace SCIRun {

class CalculateVectorMagnitudes : public Module
{
public:
  CalculateVectorMagnitudes(GuiContext* ctx);
  virtual ~CalculateVectorMagnitudes();

  virtual void execute();
};


DECLARE_MAKER(CalculateVectorMagnitudes)

CalculateVectorMagnitudes::CalculateVectorMagnitudes(GuiContext* ctx)
  : Module("CalculateVectorMagnitudes", ctx, Filter, "ChangeFieldData", "SCIRun")
{
}

CalculateVectorMagnitudes::~CalculateVectorMagnitudes()
{
}

void
CalculateVectorMagnitudes::execute()
{
  FieldHandle field_input_handle;

  if( !get_input_handle( "Input Field", field_input_handle, true ) )
    return;

  if (!field_input_handle->query_vector_interface(this).get_rep()) {
    error( "This module only works on vector data.");
    return;
  }

  // If no data or a changed recalcute.
  if( inputs_changed_ ||

       !oport_cached("Output CalculateVectorMagnitudes") )
  {
    update_state(Executing);

    const TypeDescription *vftd = field_input_handle->get_type_description();
    const string oftn = 
      field_input_handle->get_type_description(Field::FIELD_NAME_ONLY_E)->get_name() + "<" +
      field_input_handle->get_type_description(Field::MESH_TD_E)->get_name() + ", " +
      field_input_handle->get_type_description(Field::BASIS_TD_E)->get_similar_name("double",
                                                         0, "<", " >, ") +
      field_input_handle->get_type_description(Field::FDATA_TD_E)->get_similar_name("double",
							 0, "<", " >") + " > ";
    CompileInfoHandle ci = CalculateVectorMagnitudesAlgo::get_compile_info(vftd, oftn);

    Handle<CalculateVectorMagnitudesAlgo> algo;
    if (!module_dynamic_compile(ci, algo)) return;

    FieldHandle field_output_handle = algo->execute(field_input_handle);

    send_output_handle( "Output CalculateVectorMagnitudes", field_output_handle );
  }
}


CompileInfoHandle
CalculateVectorMagnitudesAlgo::get_compile_info(const TypeDescription *vftd,
				      const string &sftd)
{
  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string template_class_name("CalculateVectorMagnitudesAlgoT");
  static const string base_class_name("CalculateVectorMagnitudesAlgo");

  CompileInfo *rval = 
    scinew CompileInfo(template_class_name + "." +
		       vftd->get_filename() + ".",
                       base_class_name, 
                       template_class_name, 
                       vftd->get_name() + "," + sftd);

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  vftd->fill_compile_info(rval);
  return rval;
}

} // End namespace SCIRun










