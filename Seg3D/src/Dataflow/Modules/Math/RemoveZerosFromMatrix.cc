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
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Algorithms/Math/MathAlgo.h>

namespace SCIRun {

using namespace SCIRun;

class RemoveZerosFromMatrix : public Module {
public:
  RemoveZerosFromMatrix(GuiContext*);
  
  virtual void execute();
  
private:
  GuiString  row_or_column_;

};


DECLARE_MAKER(RemoveZerosFromMatrix)

RemoveZerosFromMatrix::RemoveZerosFromMatrix(GuiContext* ctx) :
  Module("RemoveZerosFromMatrix", ctx, Source, "Math", "SCIRun"),
  row_or_column_(ctx->subVar("row_or_col"))
{
}

void
RemoveZerosFromMatrix::execute()
{
  MatrixHandle input, output;
  
  get_input_handle("Matrix",input,true);
  
  if (inputs_changed_ || !oport_cached("Matrix") || row_or_column_.changed())
  {
    SCIRunAlgo::MathAlgo malgo(this);
    
    if (row_or_column_.get() == "row")
    {
      std::vector<Matrix::index_type> nz;
      malgo.FindNonZeroMatrixRows(input,nz);
      malgo.SelectMatrixRows(input,output,nz);
    }
    else
    {
      std::vector<Matrix::index_type> nz;
      malgo.FindNonZeroMatrixColumns(input,nz);
      malgo.SelectMatrixColumns(input,output,nz);
    }

    send_output_handle("Matrix",output,true);
  }
}


} // End namespace SCIRun


