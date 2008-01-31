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
 *  CalculateFieldDataCompiled: Field data operations
 *
 *  Written by:
 *   Allen R. Sanderson
 *   Michael Callahan
 *   SCI Institute
 *   University of Utah
 *   February 2006
 *
 *  Copyright (C) 2006 SCI Group
 */


#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace SCIRun {

class CalculateFieldDataCompiled : public Module
{
public:
  CalculateFieldDataCompiled(GuiContext* ctx);
  virtual ~CalculateFieldDataCompiled();

  virtual void execute();
  virtual void tcl_command(GuiArgs& args, void* userdata);

  virtual void presave();
  virtual void post_read();

protected:
  GuiString gui_output_data_type_;
  GuiString gui_function_;
  GuiInt    gui_cache_;
  GuiInt    gui_count_;

  FieldHandle field_output_handle_;
};


DECLARE_MAKER(CalculateFieldDataCompiled)


CalculateFieldDataCompiled::CalculateFieldDataCompiled(GuiContext* ctx)
  : Module("CalculateFieldDataCompiled", ctx, Filter,"ChangeFieldData", "SCIRun"),
    gui_output_data_type_(get_ctx()->subVar("outputdatatype"), "port 0 input"),
    gui_function_(get_ctx()->subVar("function"), "result = v0 + v1 + v2;"),
    gui_cache_(get_ctx()->subVar("cache"), 0),
    gui_count_(get_ctx()->subVar("count", 0), 0),
    field_output_handle_(0)
{
}


CalculateFieldDataCompiled::~CalculateFieldDataCompiled()
{
}


void
CalculateFieldDataCompiled::execute()
{
  std::vector<FieldHandle> field_input_handles;
  
  // Get the input field(s).
  if (!get_dynamic_input_handles("Input Field", field_input_handles, true ) )
    return;

  StringHandle sHandle;

  if (get_input_handle( "Function", sHandle, false )) {
    gui_function_.set( sHandle->get() );
    gui_function_.reset();

    get_gui()->execute(get_id() + " set_text");
  }

  // Update gui_function_ before the get.
  get_gui()->execute(get_id() + " update_text");

  // Check to see if the input field(s) has changed.
  if( inputs_changed_ ||
      gui_output_data_type_.changed( true ) ||
      gui_function_.changed( true ) ||
      gui_cache_.changed( true ) ||
      !oport_cached("Output Field") )
  {

    // If not caching set the field and count to zero.
    if( !(gui_cache_.get()) ) 
    {
      field_output_handle_ = 0;
     
      gui_count_.set(0);
      gui_count_.reset();
    }

    string outputDataType = gui_output_data_type_.get();
    string function = gui_function_.get();
    unsigned int count = gui_count_.get();

    SCIRunAlgo::FieldsAlgo algo(this);
    if (!(algo.CalculateFieldDataCompiled(field_input_handles,
					  field_output_handle_,
					  outputDataType,
					  function,
					  count))) return;

    send_output_handle("Output Field", field_output_handle_, true);

    // If not caching set the field and count to zero.
    if( !(gui_cache_.get()) )
    {
      field_output_handle_ = 0;
      gui_count_.set(0);
    }
    else
    {
      gui_count_.set(count);
    }

    gui_count_.reset();
  }
}


void CalculateFieldDataCompiled::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2){
    args.error("CalculateFieldDataCompiled needs a minor command");
    return;
  }

  if (args[1] == "clear") 
  {
    gui_count_.set(0);
    gui_count_.reset();

    field_output_handle_ = 0;
  } else {
    Module::tcl_command(args, userdata);
  }
}


void
CalculateFieldDataCompiled::presave()
{
  // update gui_function_ before saving.
  get_gui()->execute(get_id() + " update_text");
}


void
CalculateFieldDataCompiled::post_read()
{
  if (gui_output_data_type_.get() == "input")
  {
    gui_output_data_type_.set("port 0 input");
    gui_output_data_type_.reset();
  }
}

} // End namespace SCIRun
