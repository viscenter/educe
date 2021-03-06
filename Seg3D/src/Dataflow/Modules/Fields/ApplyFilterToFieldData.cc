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


#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace SCIRun {

using namespace SCIRun;
using namespace SCIRunAlgo;

class ApplyFilterToFieldData : public Module {
public:
  ApplyFilterToFieldData(GuiContext*);

  virtual void execute();

private:
  GuiString method_;
  GuiString edmethod_;
  GuiInt edniter_;
};


DECLARE_MAKER(ApplyFilterToFieldData)

ApplyFilterToFieldData::ApplyFilterToFieldData(GuiContext* ctx) :
  Module("ApplyFilterToFieldData", ctx, Source, "ChangeFieldData", "SCIRun"),
    method_(ctx->subVar("method")),
    edmethod_(ctx->subVar("ed-method")),
    edniter_(ctx->subVar("ed-iterations"))
{
}


void
ApplyFilterToFieldData::execute()
{
  FieldHandle input, output;
  
  get_input_handle("Field",input,true);
  
  if (inputs_changed_ || !oport_cached("Field") || method_.changed() ||
      edmethod_.changed() || edniter_.changed())
  {
    FieldsAlgo algo(this);
  
    if (method_.get() == "erodedilate")
    {
      FieldsAlgo::FilterSettings settings;
      settings.num_iterations_ = edniter_.get();
      if (edmethod_.get() == "erode")
      {
        if(!(algo.ApplyErodeFilterToFieldData(input,output,settings))) return;
      }
      else  if (edmethod_.get() == "dilate")
      {
        if(!(algo.ApplyDilateFilterToFieldData(input,output,settings))) return;
      }
      else
      {
        if(!(algo.ApplyErodeFilterToFieldData(input,output,settings))) return;
        if(!(algo.ApplyDilateFilterToFieldData(input,output,settings))) return;      
      }
    }
  
    send_output_handle("Field",output,true);
  }
}


} // End namespace SCIRun


