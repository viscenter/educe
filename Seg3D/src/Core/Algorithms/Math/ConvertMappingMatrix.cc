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

#include <Core/Algorithms/Math/ConvertMappingMatrix.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
ConvertMappingMatrixAlgo::ConvertMappingMatrixIntoMappingOrder(ProgressReporter* pr, MatrixHandle input, std::vector<Matrix::index_type>& output)
{
  try
  {
    if (input.get_rep() == 0)
    {
      pr->error("ConvertMappingMatrixIntoMappingOrder: No input matrix");
      return (false);
    }
    
    MatrixHandle sparse = input->sparse();
    if (sparse.get_rep() == 0)
    {
      pr->error("ConvertMappingMatrixIntoMappingOrder: Could not convert mapping matrix into sparse matrix");
      return (false);
    }
    
    SparseRowMatrix* spr = sparse->as_sparse();
    Matrix::index_type* rr = spr->rows;
    Matrix::index_type* cc = spr->columns;
    Matrix::size_type nrows = spr->nrows();
    Matrix::size_type ncols = spr->ncols();
    
    output.clear();
    output.reserve(nrows);
    for (Matrix::index_type r=0; r<nrows; r++)
    {
      if (rr[r] != rr[r+1])
      {
        Matrix::index_type c = cc[rr[r]];
        if (c < 0 || c >= ncols)
        {
          pr->error("ConvertMappingMatrixIntoMappingOrder: Mapping matrix was invalid");
          return (false);     
        }
        output.push_back(static_cast<unsigned int>(c)); 
      }
    }
    
    return (true);
  }
  catch (...)
  {
    // Clean up memory
    output.clear();
    throw;
  }
}


bool
ConvertMappingMatrixAlgo::ConvertMappingMatrixIntoMappingOrder(ProgressReporter* pr, MatrixHandle input, MatrixHandle& output)
{
  output = 0;
  
  if (input.get_rep() == 0)
  {
    pr->error("ConvertMappingMatrixIntoMappingOrder: No input matrix");
    return (false);
  }  

  Matrix::size_type nrows = input->nrows();
  std::vector<Matrix::index_type> order;
  if (!(ConvertMappingMatrixIntoMappingOrder(pr,input,order))) return (false);

  output = scinew DenseMatrix(nrows,1);
  if (output.get_rep() == 0)
  {
    pr->error("ConvertMappingMatrixIntoMappingOrder: could not allocate output matrix");
    return (false);
  }

  double* data = output->get_data_pointer();
  for (Matrix::index_type p=0; p<nrows; p++) data[p] = static_cast<double>(order[p]);

  return (true);
}


bool
ConvertMappingMatrixAlgo::ConvertMappingOrderIntoMappingMatrix(ProgressReporter* pr, MatrixHandle input, MatrixHandle& output, Matrix::size_type ncols)
{

  if (input.get_rep() == 0)
  {
    pr->error("ConvertMappingOrderIntoMappingMatrix: No input matrix");
    return (false);
  }  

  Matrix::size_type size = input->get_data_size();
  double*           data = input->get_data_pointer(); 

  std::vector<Matrix::index_type> order;
  try
  {
    order.resize(size);
    for (Matrix::index_type p=0; p<size; p++) order[p] = static_cast<Matrix::index_type>(data[p]);
  }
  catch (...)
  {
    pr->error("ConvertMappingOrderIntoMappingMatrix: could not allocate output matrix");
    return (false);    
  }
 
  return(ConvertMappingOrderIntoMappingMatrix(pr,order,output,ncols));
}


bool
ConvertMappingMatrixAlgo::ConvertMappingOrderIntoMappingMatrix(ProgressReporter* pr,  std::vector<Matrix::index_type>& input, MatrixHandle& output, Matrix::size_type ncols)
{
  Matrix::index_type* rr = 0;
  Matrix::index_type* cc = 0;
  double* vv= 0;

  Matrix::size_type nrows = input.size();  
  if (ncols == 0) ncols = nrows;
  try
  {
    rr = scinew Matrix::index_type[nrows+1];
    cc = scinew Matrix::index_type[nrows];
    vv = scinew double[nrows];
  
    for (Matrix::index_type r=0; r<nrows+1; r++) rr[r] = r;
    for (Matrix::index_type c=0; c<nrows; c++)
    {
      cc[c] = static_cast<int>(input[c]);
      vv[c] = 1.0;
    }
  }
  catch (...)
  {
    if (rr) delete[] rr;
    if (cc) delete[] cc;
    if (vv) delete[] vv;

    pr->error("ConvertMappingOrderIntoMappingMatrix: could not allocate output matrix");
    return (false);      
  }

  output = scinew SparseRowMatrix(nrows,ncols,rr,cc,nrows,vv);
  if (output.get_rep() == 0)
  {
    pr->error("ConvertMappingOrderIntoMappingMatrix: could not allocate output matrix");
    return (false);  
  }
  return (true);
  
}


bool
ConvertMappingMatrixAlgo::ConvertMappingOrderIntoTransposeMappingMatrix(ProgressReporter* pr, MatrixHandle input, MatrixHandle& output, Matrix::size_type nrows)
{

  if (input.get_rep() == 0)
  {
    pr->error("ConvertMappingOrderIntoMappingMatrix: No input matrix");
    return (false);
  }  

  Matrix::size_type size = input->get_data_size();
  double* data = input->get_data_pointer(); 

  std::vector<Matrix::index_type> order;
  try
  {
    order.resize(size);
    for (int p=0; p<size; p++) order[p] = static_cast<unsigned int>(data[p]);
  }
  catch (...)
  {
    pr->error("ConvertMappingOrderIntoMappingMatrix: could not allocate output matrix");
    return (false);    
  }
 
  return(ConvertMappingOrderIntoTransposeMappingMatrix(pr,order,output,nrows));
}


bool
ConvertMappingMatrixAlgo::ConvertMappingOrderIntoTransposeMappingMatrix(ProgressReporter* pr,  std::vector<Matrix::index_type>& input, MatrixHandle& output, Matrix::size_type nrows)
{
  MatrixHandle tmp;
  if (!(ConvertMappingOrderIntoMappingMatrix(pr, input, tmp, nrows))) return (false);
  output = tmp->transpose();
  return (true);
}


} // end namespace SCIRunAlgo


