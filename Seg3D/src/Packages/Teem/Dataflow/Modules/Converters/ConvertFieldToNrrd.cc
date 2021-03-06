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

#include <Core/Algorithms/Converter/ConverterAlgo.h>
#include <Core/Datatypes/Field.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Datatypes/NrrdData.h>
#include <Dataflow/Network/Ports/NrrdPort.h>

#include <Dataflow/Network/Module.h>
#include <Core/Malloc/Allocator.h>

namespace SCITeem {

using namespace SCIRun;

class ConvertFieldToNrrd : public Module {
public:
  ConvertFieldToNrrd(GuiContext*);

  virtual void execute();

};


DECLARE_MAKER(ConvertFieldToNrrd)

ConvertFieldToNrrd::ConvertFieldToNrrd(GuiContext* ctx) : 
  Module("ConvertFieldToNrrd", ctx, Source, "Converters", "Teem")
{
}

void 
ConvertFieldToNrrd::execute()
{
  // Define local handles of data objects:
  NrrdDataHandle nrrd;
  FieldHandle ofield;

  // Get the new input data: 
  if (!(get_input_handle("Field",ofield,true))) return;

  // Only reexecute if the input changed. SCIRun uses simple scheduling
  // that executes every module downstream even if no data has changed:  
  if (inputs_changed_ || !oport_cached("Nrrd"))
  {
    SCIRunAlgo::ConverterAlgo algo(this);
    if (!(algo.FieldToNrrd(ofield,nrrd))) return;

    // send new output if there is any:    
    send_output_handle("Nrrd",nrrd,false);
  }
}

} // End namespace SCITeem
