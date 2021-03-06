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

class ConvertMeshToPointCloud : public Module
{
public:
  ConvertMeshToPointCloud(GuiContext* ctx);
  virtual void execute();
  
private:
    GuiInt datalocation_;
};

DECLARE_MAKER(ConvertMeshToPointCloud)
ConvertMeshToPointCloud::ConvertMeshToPointCloud(GuiContext* ctx)
  : Module("ConvertMeshToPointCloud", ctx, Filter, "ChangeMesh", "SCIRun"),
    datalocation_(get_ctx()->subVar("datalocation"))
{
}

void
ConvertMeshToPointCloud::execute()
{
  FieldHandle ifield, ofield;
  if (!(get_input_handle("Field",ifield,true))) return;

  if (inputs_changed_ || datalocation_.changed() || !oport_cached("Field"))
  {
    bool dloc = static_cast<bool>(datalocation_.get());
    SCIRunAlgo::FieldsAlgo algo(this);
    if (!(algo.ConvertMeshToPointCloud(ifield,ofield,dloc))) return;

    send_output_handle("Field", ofield);
  }
}

} // End namespace SCIRun

