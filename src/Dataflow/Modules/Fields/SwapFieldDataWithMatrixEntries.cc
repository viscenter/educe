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
 *  SwapFieldDataWithMatrixEntries:
 *  Store/retrieve values from an input matrix to/from the data of a field.
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
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace SCIRun {

class SwapFieldDataWithMatrixEntries : public Module
{
public:
  SwapFieldDataWithMatrixEntries(GuiContext*);
  virtual void execute();

private:
  GuiInt gui_keepscalartype_;
};


DECLARE_MAKER(SwapFieldDataWithMatrixEntries)
SwapFieldDataWithMatrixEntries::SwapFieldDataWithMatrixEntries(GuiContext* ctx)
  : Module("SwapFieldDataWithMatrixEntries", ctx, Source, "ChangeFieldData", "SCIRun"),
  gui_keepscalartype_(ctx->subVar("preserve-scalar-type"), 0)
{
}

void
SwapFieldDataWithMatrixEntries::execute()
{
  FieldHandle field_input_handle;
  MatrixHandle matrix_input_handle;
  
  if(!(get_input_handle("Input Field",field_input_handle,true))) return;

  get_input_handle("Input Matrix",matrix_input_handle,false);
  
  if (inputs_changed_ ||
      gui_keepscalartype_.changed() || 
      (get_oport("Output Field")->nconnections() &&
       !oport_cached("Output Field")) ||
      (get_oport("Output Matrix")->nconnections() &&
       !oport_cached("Output Matrix")))
  {
    SCIRunAlgo::FieldsAlgo algo(this);

    // Get the data.
    if( get_oport("Output Matrix")->nconnections() )
    {
      MatrixHandle matrix_output_handle;
  
      if(!(algo.GetFieldData(field_input_handle,matrix_output_handle))) return;
      send_output_handle("Output Matrix", matrix_output_handle);  
    }

    // Set the data.
    if( get_oport("Output Field")->nconnections() )
    {
      FieldHandle field_output_handle;

      if (matrix_input_handle.get_rep()) 
      {
	if(!(algo.SetFieldData(field_input_handle,
			       field_output_handle,
			       matrix_input_handle,
			       (bool)gui_keepscalartype_.get()))) return;

	field_output_handle->copy_properties(field_input_handle.get_rep());
      }
      else 
      {
	warning("No input matrix passing the field through");

	field_output_handle = field_input_handle;
      }	

      send_output_handle("Output Field", field_output_handle);
    }
  }
}

} // End namespace SCIRun
