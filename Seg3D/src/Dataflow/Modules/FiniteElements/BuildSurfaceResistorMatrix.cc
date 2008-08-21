/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2004 Scientific Griduting and Imaging Institute,
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

#include <Core/Algorithms/FiniteElements/BuildMatrix/BuildSurfaceResistorMatrix.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>

#include <Dataflow/Network/Module.h>

namespace SCIRun {

class BuildSurfaceResistorMatrix : public Module {
  public:
    BuildSurfaceResistorMatrix(GuiContext*);
    virtual void execute();

  private: 
    SCIRunAlgo::BuildSurfaceResistorMatrixAlgo algo_;
};


DECLARE_MAKER(BuildSurfaceResistorMatrix)
BuildSurfaceResistorMatrix::BuildSurfaceResistorMatrix(GuiContext* ctx)
  : Module("BuildSurfaceResistorMatrix", ctx, Source, "FiniteElements", "SCIRun")
{
  algo_.set_progress_reporter(this);
}


void BuildSurfaceResistorMatrix::execute()
{
  MatrixHandle NodeLink, DElemLink, SRMatrix;
  FieldHandle FEMesh, SurfaceResistor;
  
  get_input_handle("FEMesh",FEMesh,true);
  get_input_handle("SurfaceResistor",SurfaceResistor,true);
  get_input_handle("NodeLink",NodeLink,false);
  get_input_handle("DElemLink",DElemLink,false);

  if (inputs_changed_ || oport_cached("SRMatrix"))
  {
    if(!(algo_.run(FEMesh,SurfaceResistor,NodeLink,DElemLink,SRMatrix))) return;
    send_output_handle("SRMatrix",SRMatrix,false);
  }
}

} // End namespace ModelCreation
