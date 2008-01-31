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


#include <Core/Algorithms/Math/SortMatrixRows.h>
#include <Core/Algorithms/Math/SelectMatrixRows.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool 
SortMatrixRowsAlgo::GetSortOrderMatrixRows(ProgressReporter *pr,MatrixHandle input, std::vector<Matrix::index_type>& order, bool asc)
{
  if (input.get_rep() == 0)
  {
    pr->error("GetSortOrderMatrixRows: No input matrix");
    return (false); 
  }

  if (!(input->is_dense()||input->is_column()))
  {
    pr->error("GetSortOrderMatrixRows: This function has only been implemented for DenseMatrix and ColumnMatrix");
    return (false);
  }

  Matrix::size_type nrows = input->nrows();
  order.resize(nrows);
  for (Matrix::index_type p=0; p<nrows; p++) order[p] = p;
  
  if (input->is_dense())
  {
    DenseMatrix* dm = input->as_dense();
    if (asc == true)
      std::sort(order.begin(),order.end(),SortAscDenseMatrix(dm));
    else
      std::sort(order.begin(),order.end(),SortDescDenseMatrix(dm));      
  }

  if (input->is_column())
  {
    ColumnMatrix* cm = input->as_column();
    if (asc == true)
      std::sort(order.begin(),order.end(),SortAscColumnMatrix(cm));
    else
      std::sort(order.begin(),order.end(),SortDescColumnMatrix(cm));      
  }

  return (true);
}



bool 
SortMatrixRowsAlgo::GetSortOrderMatrixRows(ProgressReporter *pr,MatrixHandle input, MatrixHandle& output, bool asc)
{
  if (input.get_rep() == 0)
  {
    pr->error("GetSortOrderMatrixRows: No input matrix");
    return (false); 
  }
  
  Matrix::size_type nrows = input->nrows();
  std::vector<Matrix::index_type> order;
  if (!(GetSortOrderMatrixRows(pr,input,order,asc))) return (false);

  output = scinew DenseMatrix(nrows,1);
  if (output.get_rep() == 0)
  {
    pr->error("GetSortOrderMatrixRows: could not allocate output matrix");
    return (false);
  }

  double* data = output->get_data_pointer();
  for (Matrix::index_type p=0; p<nrows; p++) data[p] = static_cast<double>(order[p]);
  
  return (true);
}


bool 
SortMatrixRowsAlgo::SortMatrixRows(ProgressReporter *pr,MatrixHandle input, MatrixHandle& output, bool asc)
{
  std::vector<Matrix::index_type> order;
  if(!(GetSortOrderMatrixRows(pr,input,order,asc))) return (false);
  SelectMatrixRowsAlgo algo;
  return(algo.SelectMatrixRows(pr,input,output,order));
}

} // end namespace SCIRunAlgo

