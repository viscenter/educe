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

#include <Core/Algorithms/Math/LinearSystem/AddKnownsToLinearSystem.h>
#include <Core/Algorithms/Fields/FieldData/GetFieldData.h>

#include <Core/Datatypes/Field.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

#include <Core/Algorithms/Fields/FieldsAlgo.h>
#include <Dataflow/Network/Module.h>

namespace ModelCreation {

using namespace SCIRun;

class CreateFEDirichletBC : public Module {
  public:
    CreateFEDirichletBC(GuiContext*);
    virtual void execute();

  private:
    SCIRunAlgo::AddKnownsToLinearSystemAlgo add_knowns_algo_;
    SCIRunAlgo::GetFieldDataAlgo set_fielddata_algo_;
};


DECLARE_MAKER(CreateFEDirichletBC)
CreateFEDirichletBC::CreateFEDirichletBC(GuiContext* ctx)
  : Module("CreateFEDirichletBC", ctx, Source, "FiniteElements", "ModelCreation")
{
  add_knowns_algo_.set_progress_reporter(this);
  set_fielddata_algo_.set_progress_reporter(this);
}

void CreateFEDirichletBC::execute()
{
  FieldHandle BCfield;
  MatrixHandle FEin, FEout, RHSin, RHSout, BC;
  
  if (!(get_input_handle("BCField",BCfield,true))) return;
  if (!(get_input_handle("FEMatrix",FEin,true))) return;
  get_input_handle("RHS",RHSin,false);
  
  if (inputs_changed_ || !oport_cached("FEMatrix") || !oport_cached("RHS"))
  {    
    if (!(set_fielddata_algo_.run(BCfield,BC))) return;
    if (!(add_knowns_algo_.run(FEin,RHSin,BC,FEout,RHSout))) return;
      
    send_output_handle("FEMatrix",FEout,false);
    send_output_handle("RHS",RHSout,false);
  }
}

} // End namespace ModelCreation


