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
 *  CollectPointClouds.cc:  Collect a (time) series of point clouds
 *  turing them into a set of curves.
 *
 *  Written by:
 *   Allen R. Sanderson
 *   SCI Institute
 *   University of Utah
 *   July 2007
 *
 *  Copyright (C) 2007 SCI Institute
 */

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace SCIRun {

class CollectPointClouds : public Module
{
public:
  CollectPointClouds(GuiContext* ctx);
  virtual ~CollectPointClouds();

  virtual void execute();
  virtual void tcl_command(GuiArgs& args, void* userdata);

protected:
  GuiInt gui_num_fields_;
  GuiInt gui_count_;

  Field::size_type numnodes_;
  Field::size_type numelems_;

  FieldHandle field_output_handle_;
};


DECLARE_MAKER(CollectPointClouds)


CollectPointClouds::CollectPointClouds(GuiContext* ctx)
  : Module("CollectPointClouds", ctx, Filter,"MiscField", "SCIRun"),
    gui_num_fields_(get_ctx()->subVar("num_fields"), 1),
    gui_count_(get_ctx()->subVar("count", 0), 0),
    field_output_handle_(0)
{
}


CollectPointClouds::~CollectPointClouds()
{
}


void
CollectPointClouds::execute()
{
  FieldHandle field_input_handle;

  // Get the input field.
  if (!get_input_handle("Point Cloud", field_input_handle, true ) )
    return;

  if( gui_count_.get() == 0 ) 
  {
    SCIRunAlgo::FieldsAlgo algo(this);
    if(!(algo.GetFieldInfo(field_input_handle, numnodes_, numelems_))) return;
  }

  // Check to see if the input field or GUI has changed.
  if( inputs_changed_ ||
      gui_num_fields_.changed( true ) ||
      !oport_cached("Curve") )
  {
    unsigned int count = gui_count_.get();

    SCIRunAlgo::FieldsAlgo algo(this);

    if (!(algo.CollectPointClouds(field_input_handle,
				  field_output_handle_,
				  gui_num_fields_.get(),
				  numnodes_,
				  count))) return;

    gui_count_.set(count);
    gui_count_.reset();

    send_output_handle("Curve", field_output_handle_, true);
  }
}


void CollectPointClouds::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2){
    args.error("CollectPointClouds needs a minor command");
    return;
  }

  if (args[1] == "clear") {
    gui_count_.set(0);
    gui_count_.reset();

    field_output_handle_ = 0;
  } else {
    Module::tcl_command(args, userdata);
  }
}

} // End namespace SCIRun
