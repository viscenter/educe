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
 *  TransformMeshWithFunction: Transform a mesh using a function
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   January 2005
 *
 *  Copyright (C) 2005 SCI Group
 */

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace SCIRun {

class TransformMeshWithFunction : public Module
{
  public:
    TransformMeshWithFunction(GuiContext* ctx);
    virtual ~TransformMeshWithFunction() {}

    virtual void execute();
    virtual void presave();

  protected:
    GuiString gui_function_;
};


DECLARE_MAKER(TransformMeshWithFunction)


TransformMeshWithFunction::TransformMeshWithFunction(GuiContext* ctx)
  : Module("TransformMeshWithFunction", ctx, Filter,"ChangeMesh", "SCIRun"),
    gui_function_(get_ctx()->subVar("function"),
		  "result = (Point) (Vector(x,y,z) * Vector(1.0, 2.0, 3.0));")
{
}


void
TransformMeshWithFunction::execute()
{
  FieldHandle field_input_handle;

  // Get the input field.
  if (!get_input_handle("Input Field", field_input_handle, true ) )
    return;

  // Update gui_function_ before the get.
  get_gui()->execute(get_id() + " update_text");

  // The transform can only work on irregular meshes.
  if( !(field_input_handle->mesh()->topology_geometry() & Mesh::IRREGULAR) ) {

    error( field_input_handle->get_type_description(Field::MESH_TD_E)->get_name() );
    error( "Only availible for irregular meshes." );
    error( "Need to convert the mesh to an irregular mesh first." );
    return;
  }

  // Check to see if the input field(s) has changed.
  if( inputs_changed_ ||
      gui_function_.changed( true ) ||
      !oport_cached("Output Field") )
  {
    FieldHandle  field_output_handle;

    string function = gui_function_.get();

    SCIRunAlgo::FieldsAlgo algo(this);

    if (!(algo.TransformMeshWithFunction(field_input_handle,
					 field_output_handle,
					 function))) return;

    send_output_handle("Output Field", field_output_handle);
  }
}


void
TransformMeshWithFunction::presave()
{
  // update gui_function_ before saving.
  get_gui()->execute(get_id() + " update_text");
}


} // End namespace SCIRun
