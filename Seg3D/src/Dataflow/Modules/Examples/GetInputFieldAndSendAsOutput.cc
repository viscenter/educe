//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  
//    File   : GetInputFieldAndSendAsOutput.cc
//    Author : Martin Cole
//    Date   : Mon Sep 11 11:22:14 2006


#include <Dataflow/Network/Module.h>
#include <Core/Malloc/Allocator.h>
#include <Core/Datatypes/Field.h>
#include <Dataflow/Network/Ports/SimplePort.h>
#include <iostream>

namespace SCIRun {
using namespace std;
using namespace SCIRun;

class GetInputFieldAndSendAsOutput : public Module 
{
public:
  GetInputFieldAndSendAsOutput(GuiContext*);
  virtual ~GetInputFieldAndSendAsOutput();

  virtual void execute();
  virtual void tcl_command(GuiArgs&, void*);
};


DECLARE_MAKER(GetInputFieldAndSendAsOutput)
GetInputFieldAndSendAsOutput::GetInputFieldAndSendAsOutput(GuiContext* ctx) : 
  Module("GetInputFieldAndSendAsOutput", ctx, Source, "Examples", "SCIRun")
{
}

GetInputFieldAndSendAsOutput::~GetInputFieldAndSendAsOutput()
{
}

void
GetInputFieldAndSendAsOutput::execute()
{
  FieldHandle field_handle;
  if (! get_input_handle("InField", field_handle, true)) {
    error("GetInputFieldAndSendAsOutput must have a SCIRun::Field as input to continue.");
    return;
  }
  cerr << "GetInputFieldAndSendAsOutput module got data :" << field_handle.get_rep() << endl;


  send_output_handle("OutField", field_handle);
}

void
GetInputFieldAndSendAsOutput::tcl_command(GuiArgs& args, void* userdata)
{
  Module::tcl_command(args, userdata);
}

} // End namespace SCIRun


