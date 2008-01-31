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
 *  SetFieldData.cc:  Set the field data using either a matrix or a NRRD.
 *
 *  Written by:
 *   SCI Institute
 *   University of Utah
 *   July 2007
 *
 *  Copyright (C) 2007 SCI Institute
 */

#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace SCIRun {

class SetFieldData : public Module {

public:
  SetFieldData(GuiContext*);
  virtual void execute();

protected:
  GuiInt gui_keepscalartype_;
};


DECLARE_MAKER(SetFieldData)
SetFieldData::SetFieldData(GuiContext* ctx)
  : Module("SetFieldData", ctx, Source, "ChangeFieldData", "SCIRun"),
  gui_keepscalartype_(ctx->subVar("keepscalartype"))
{
}

void
SetFieldData::execute()
{
  FieldHandle field_input_handle;
  if(!(get_input_handle("Field",field_input_handle,true))) return;

  MatrixHandle matrix_input_handle;
  get_input_handle("Matrix Data",matrix_input_handle,false);

  NrrdDataHandle nrrddata_input_handle;  
  get_input_handle("Nrrd Data",nrrddata_input_handle,false);
  
  if (inputs_changed_ ||
      gui_keepscalartype_.changed() ||
      !oport_cached("Field"))
  {
    SCIRunAlgo::FieldsAlgo algo(this);

    FieldHandle field_output_handle;

    if( matrix_input_handle.get_rep() )
    {
      if(!(algo.SetFieldData(field_input_handle,
			     field_output_handle,
			     matrix_input_handle,
			     (bool) gui_keepscalartype_.get()))) return;
    }
    else if( nrrddata_input_handle.get_rep() )
    {
      if(!(algo.SetFieldData(field_input_handle,
			     field_output_handle,
			     nrrddata_input_handle,
			     (bool) gui_keepscalartype_.get()))) return;
    }
    else
    {
      error( "No input Matrix or Nrrd." );
      return;
    }

    send_output_handle("Field", field_output_handle);
  }
}

} // End namespace SCIRun
