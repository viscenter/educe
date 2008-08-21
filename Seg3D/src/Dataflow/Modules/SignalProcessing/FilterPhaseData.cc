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

#include <Core/Algorithms/SignalProcessing/PhaseData/FilterPhaseData.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

namespace SCIRun {

using namespace SCIRun;

class FilterPhaseData: public Module {
public:
  FilterPhaseData(GuiContext*);
  virtual void execute();

private:
  GuiInt filter_length_;
  SCIRunAlgo::FilterPhaseDataAlgo algo_;
};


DECLARE_MAKER(FilterPhaseData)

FilterPhaseData::FilterPhaseData(GuiContext* ctx) :
  Module("FilterPhaseData", ctx, Source, "SignalProcessing", "SCIRun"),
  filter_length_(get_ctx()->subVar("filter-length"))
{
  algo_.set_progress_reporter(this);
}

void
FilterPhaseData::execute()
{
  MatrixHandle input, output;
  get_input_handle("Data",input,true);
  
  if (inputs_changed_ || filter_length_.changed() || !oport_cached("FilteredData"))
  {
//    algo_.set_int("filter_length",filter_length_.get());
    algo_.run(input,output);
    send_output_handle("FilteredData",output,true);
  }
}

} // End namespace SCIRun
