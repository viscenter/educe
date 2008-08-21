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


#include <Core/Datatypes/Field.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>


namespace SCIRun {

class ApplyTriSurfPhaseFilter : public Module {
  public:
    ApplyTriSurfPhaseFilter(GuiContext*);
    virtual ~ApplyTriSurfPhaseFilter() {}
    virtual void execute();   
};


DECLARE_MAKER(ApplyTriSurfPhaseFilter)
ApplyTriSurfPhaseFilter::ApplyTriSurfPhaseFilter(GuiContext* ctx)
  : Module("ApplyTriSurfPhaseFilter", ctx, Source, "ChangeFieldData", "SCIRun")
{
}

void ApplyTriSurfPhaseFilter::execute()
{
  FieldHandle input, output;
  FieldHandle phaseline, phasepoint;

  get_input_handle("PhaseField",input,true);
  
  if (inputs_changed_ || !oport_cached("PhaseField") ||
    !oport_cached("PhaseLine") || !oport_cached("PhasePoint"))
  {
    SCIRunAlgo::FieldsAlgo algo(this);
  
    if(!(algo.ApplyTriSurfPhaseFilter(input,output,phaseline,phasepoint))) return;
   
    send_output_handle("PhaseField",output,true);
    send_output_handle("PhaseLine",phaseline,true);
    send_output_handle("PhasePoint",phasepoint,true);
  }
}

} // End namespace SCIRun


