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
 *  GenerateLinearSegments.cc:  Unfinished modules
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

#include <Dataflow/Modules/Fields/GenerateLinearSegments.h>

namespace SCIRun {

class GenerateLinearSegments : public Module
{
public:
  GenerateLinearSegments(GuiContext* ctx);
  virtual ~GenerateLinearSegments();

  virtual void execute();

private:
  GuiDouble                     gui_step_size_;
  GuiInt                        gui_max_steps_;
  GuiInt                        gui_direction_;
  GuiInt                        gui_value_;
};


DECLARE_MAKER(GenerateLinearSegments)

GenerateLinearSegments::GenerateLinearSegments(GuiContext* ctx)
  : Module("GenerateLinearSegments", ctx, Filter, "MiscField", "SCIRun"),
  gui_step_size_(get_ctx()->subVar("stepsize"), 0.1),
  gui_max_steps_(get_ctx()->subVar("maxsteps"), 10),
  gui_direction_(get_ctx()->subVar("direction"), 1),
  gui_value_(get_ctx()->subVar("value"), 1)
{
}

GenerateLinearSegments::~GenerateLinearSegments()
{
}

void
GenerateLinearSegments::execute()
{
  FieldHandle field_input_handle;

  if( !get_input_handle( "Input Field", field_input_handle, true ) )
    return;

  if (!field_input_handle->query_vector_interface(this).get_rep()) {
    error( "This module only works on vector data.");
    return;
  }

  // Check to see if the input field(s) has changed.
  if( inputs_changed_ ||
      gui_step_size_.changed( true ) ||
      gui_max_steps_.changed( true ) ||
      gui_direction_.changed( true ) ||
      gui_value_.changed( true ) ||
      !oport_cached("Output  Linear Segments") )
  {
    update_state(Executing);

    Field *iField = field_input_handle.get_rep();

    const TypeDescription *iftd = iField->get_type_description();
    
    const TypeDescription *ifdtd = 
      (*iField->get_type_description(Field::FDATA_TD_E)->get_sub_type())[0];
    const TypeDescription *iltd = iField->order_type_description();

    string dsttype;
    if (gui_value_.get() == 0)
      dsttype = ifdtd->get_name();
    else
    dsttype = "double";

    const string dftn =
      "GenericField<CurveMesh<CrvLinearLgn<Point> >, CrvLinearLgn<" +
      dsttype + ">, vector<" + dsttype + "> > ";

    CompileInfoHandle aci =
      GenerateLinearSegmentsAlgo::get_compile_info(iftd, iltd,
						    dftn, gui_value_.get());

    Handle<GenerateLinearSegmentsAlgo> accalgo;
    if (!module_dynamic_compile(aci, accalgo)) return;
      
    FieldHandle field_output_handle =
      accalgo->execute(this, field_input_handle,
		       gui_step_size_.get(),
		       gui_max_steps_.get(),
		       gui_direction_.get(),
		       gui_value_.get());

    send_output_handle( "Output Linear Segments", field_output_handle );
  }
}


CompileInfoHandle
GenerateLinearSegmentsAlgo::get_compile_info(const TypeDescription *fsrc,
					      const TypeDescription *sloc,
					      const string &fdst,
					      int value)
{
  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string template_class_name("GenerateLinearSegmentsAlgoT");
  static const string base_class_name("GenerateLinearSegmentsAlgo");

  CompileInfo *rval = 
    scinew CompileInfo(template_class_name + (value?"M":"F") + "." +
		       fsrc->get_filename() + ".",
                       base_class_name, 
                       template_class_name + (value?"M":"F"),
		       fsrc->get_name() + ", " +
                       fdst);

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  rval->add_basis_include("Core/Basis/CrvLinearLgn.h");
  rval->add_mesh_include("Core/Datatypes/CurveMesh.h");
  fsrc->fill_compile_info(rval);
  return rval;
}

} // End namespace SCIRun










