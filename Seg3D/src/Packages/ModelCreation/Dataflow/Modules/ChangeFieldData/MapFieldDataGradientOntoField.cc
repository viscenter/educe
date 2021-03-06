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
#include <Core/Malloc/Allocator.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Datatypes/Field.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace ModelCreation {

using namespace SCIRun;

class MapFieldDataGradientOntoField : public Module {
public:
  MapFieldDataGradientOntoField(GuiContext*);
  virtual void execute();

  private:
    GuiString mappingmethod_;
    GuiString integrationmethod_;
    GuiString integrationfilter_;  
    GuiInt    calcnorm_;
};


DECLARE_MAKER(MapFieldDataGradientOntoField)
MapFieldDataGradientOntoField::MapFieldDataGradientOntoField(GuiContext* ctx)
  : Module("MapFieldDataGradientOntoField", ctx, Source, "ChangeFieldData", "ModelCreation"),
    mappingmethod_(ctx->subVar("mappingmethod")),
    integrationmethod_(ctx->subVar("integrationmethod")),
    integrationfilter_(ctx->subVar("integrationfilter")),
    calcnorm_(ctx->subVar("calcnorm"))
{
}


void
MapFieldDataGradientOntoField::execute()
{
  FieldHandle fsrc, fdst, fout;
  
  if (!(get_input_handle("Source",fsrc,true))) return;
  if (!(get_input_handle("Destination",fdst,true))) return;
  
  if (inputs_changed_ || 
      mappingmethod_.changed() ||
      integrationmethod_.changed() ||
      integrationfilter_.changed() ||
      calcnorm_.changed() ||
      !oport_cached("Destination"))
  {
    std::string mappingmethod = mappingmethod_.get();
    std::string integrationmethod = integrationmethod_.get();
    std::string integrationfilter = integrationfilter_.get();
    
    SCIRunAlgo::FieldsAlgo algo(this);
    const bool calcnorm = calcnorm_.get();
    
    if (!(algo.MapFieldDataGradientOntoField(fsrc,fdst,fout,mappingmethod,integrationmethod,integrationfilter,calcnorm))) return;
    
    send_output_handle("Destination",fout,false);
  }
}


} // End namespace ModelCreation


