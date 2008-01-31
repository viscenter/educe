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

#include <Core/Algorithms/Math/MathAlgo.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Malloc/Allocator.h>

namespace SCIRun {

using namespace SCIRun;

class RemoveZeroRowsAndColumns : public Module {
public:
  RemoveZeroRowsAndColumns(GuiContext*);

  virtual void execute();
};


DECLARE_MAKER(RemoveZeroRowsAndColumns)

RemoveZeroRowsAndColumns::RemoveZeroRowsAndColumns(GuiContext* ctx) :
  Module("RemoveZeroRowsAndColumns", ctx, Source, "Math", "SCIRun")
{
}

void
RemoveZeroRowsAndColumns::execute()
{
  MatrixHandle input, output, leftmapping, rightmapping;
  
  get_input_handle("Matrix",input,true);
  
  if (inputs_changed_ || !oport_cached("ReducedMatrix") ||
    !oport_cached("LeftMapping")||!oport_cached("RightMapping"))
  {
    SCIRunAlgo::MathAlgo malgo(this);
  
    std::vector<Matrix::index_type> rows;
    std::vector<Matrix::index_type> columns;
    malgo.FindNonZeroMatrixRows(input,rows);
    malgo.FindNonZeroMatrixColumns(input,columns);
    
    malgo.SelectSubMatrix(input,output,rows,columns);
    
    malgo.ConvertMappingOrderIntoTransposeMappingMatrix(rows,leftmapping,input->nrows());
    malgo.ConvertMappingOrderIntoMappingMatrix(columns,rightmapping,input->ncols());
    
    send_output_handle("ReducedMatrix",output,true);
    send_output_handle("LeftMapping",leftmapping,true);
    send_output_handle("RightMapping",rightmapping,true);
  }
}

} // End namespace SCIRun


