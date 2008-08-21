/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2004 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
 *  GetFieldZeroCrossings.cc:
 *
 *  Written by:
 *   Allen Sanderson
 *   School of Computing
 *   University of Utah
 *   April 2005
 *
 *  Copyright (C) 2005 SCI Group
 */

#include <Packages/FusionPSE/Dataflow/Modules/Fields/GetFieldZeroCrossings.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>

namespace FusionPSE {

using namespace std;
using namespace SCIRun;

class GetFieldZeroCrossings : public Module {
public:
  GetFieldZeroCrossings(GuiContext *context);

  virtual ~GetFieldZeroCrossings();

  virtual void execute();

private:
  GuiInt gui_axis_;

  FieldHandle field_output_handle_;
};


DECLARE_MAKER(GetFieldZeroCrossings)


GetFieldZeroCrossings::GetFieldZeroCrossings(GuiContext *context)
  : Module("GetFieldZeroCrossings", context, Filter, "Fields", "FusionPSE"),
    
    gui_axis_(context->subVar("axis"), 2)
{
}


GetFieldZeroCrossings::~GetFieldZeroCrossings()
{
}


void
GetFieldZeroCrossings::execute()
{
  FieldHandle field_input_handle;
  if( !get_input_handle( "Input Field", field_input_handle, true  ) ) return;

  if( field_input_handle->mesh()->topology_geometry() & Mesh::REGULAR ) {

    error( field_input_handle->get_type_description(Field::MESH_TD_E)->get_name() );
    error( "Only availible for topologically irregular data." );
    return;
  }

  if( field_input_handle->basis_order() != 1 ) {
    error( field_input_handle->get_type_description(Field::MESH_TD_E)->get_name() );
    error( "Currently only availible for node data." );
    return;
  }

  // If no data or a changed input field or axis recreate the mesh.
  if( inputs_changed_ ||
      
      !field_output_handle_.get_rep() ||
      
      gui_axis_.changed( true ) ) {
    update_state(Executing);

    const TypeDescription *ftd = field_input_handle->get_type_description();
    const string oftn = 
      field_input_handle->get_type_description(Field::FIELD_NAME_ONLY_E)->get_name() + "<" +
      field_input_handle->get_type_description(Field::MESH_TD_E)->get_name() + ", " +
      field_input_handle->get_type_description(Field::BASIS_TD_E)->get_similar_name("double",
                                                         0, "<", " >, ") +
      field_input_handle->get_type_description(Field::FDATA_TD_E)->get_similar_name("double",
							 0, "<", " >") + " > ";
    CompileInfoHandle ci = GetFieldZeroCrossingsAlgo::get_compile_info(ftd, oftn);

    Handle<GetFieldZeroCrossingsAlgo> algo;
    if (!module_dynamic_compile(ci, algo)) return;
  
    field_output_handle_ = algo->execute(field_input_handle,
					 gui_axis_.get() );
  }

  // Send the data downstream
  send_output_handle( "Output Field", field_output_handle_, true );
}


CompileInfoHandle
GetFieldZeroCrossingsAlgo::get_compile_info(const TypeDescription *ftd,
					    const string &sftd)
{
  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string template_class_name("GetFieldZeroCrossingsAlgoT");
  static const string base_class_name("GetFieldZeroCrossingsAlgo");

  CompileInfo *rval = 
    new CompileInfo(template_class_name + "." +
		       ftd->get_filename() + ".",
                       base_class_name, 
                       template_class_name, 
                       ftd->get_name() + ", " + sftd);

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  ftd->fill_compile_info(rval);
  rval->add_namespace("FusionPSE");
  return rval;
}


} // End namespace SCIRun
