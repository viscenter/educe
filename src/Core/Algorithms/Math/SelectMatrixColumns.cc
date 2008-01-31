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

#include <Core/Algorithms/Math/SelectMatrixColumns.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
SelectMatrixColumnsAlgo::SelectMatrixColumns(ProgressReporter* pr, MatrixHandle input, MatrixHandle& output, MatrixHandle columns)
{
  Matrix::size_type num_sel_columns = columns->get_data_size();
  double* sel_columns_ptr = columns->get_data_pointer();

  std::vector<Matrix::index_type> sel_columns;
  try
  {
    sel_columns.resize(num_sel_columns);
    for (Matrix::index_type p=0; p< num_sel_columns; p++) sel_columns[p] = static_cast<Matrix::index_type>(sel_columns_ptr[p]);
  }
  catch (...)
  {
    pr->error("SelectMatrixColumns: Could not allocate enough memory");
    return (false); 
  }  
  return(SelectMatrixColumns(pr,input,output,sel_columns));
}

bool
SelectMatrixColumnsAlgo::SelectMatrixColumns(ProgressReporter* pr, MatrixHandle input, MatrixHandle& output, std::vector<Matrix::index_type> columns)
{
  if (input.get_rep() == 0)
  {
    pr->error("SelectMatrixColumns: No input matrix");
    return (false);
  }

  if (columns.size() == 0)
  {
    pr->error("SelectMatrixColumns: No row indices given");
    return (false);  
  }
  
  Matrix::size_type m = input->nrows();
  Matrix::size_type n = input->ncols();
    
  for (size_t r=0; r<columns.size(); r++)
  {
    if (columns[r] >= static_cast<Matrix::index_type>(n))
    {
      pr->error("SelectMatrixColumns: selected column exceeds matrix dimensions");
      return (false);
    }
  }
  
  if (input->is_sparse())
  {
    Matrix::index_type *rr = input->get_row();
    Matrix::index_type *cc = input->get_col();
    double *vv = input->get_val();
    
    if (rr==0 || cc==0 || vv==0)
    {
      pr->error("SelectMatrixColumns: Sparse matrix is invalid");
      return (false);      
    }
   
    std::vector<size_t> s(n, n);
    for (size_t r=0; r< columns.size(); r++) s[columns[r]] = r;
      
    Matrix::index_type k =0;
    for (Matrix::index_type r=0; r<m; r++)
    {
      for (Matrix::index_type q=rr[r]; q<rr[r+1]; q++)
      {
        if (s[cc[q]] < (size_t)n) k++;
      }
    }
    
    Matrix::index_type *nrr = 0;
    Matrix::index_type *ncc = 0;
    double *nvv = 0;
    try
    {
      nrr = scinew Matrix::index_type[m+1];
      ncc = scinew Matrix::index_type[k];
      nvv = scinew double[k];
    }
    catch (...)
    {
      if (nrr) delete[] nrr;
      if (ncc) delete[] ncc;
      if (nvv) delete[] nvv;
      pr->error("SelectMatrixColumns: Could not allocate output matrix");
      return (false);      
    }    

    k =0;
    for (int r=0; r<m; r++)
    {
      nrr[r] =k;
      for (Matrix::index_type q=rr[r]; q<rr[r+1]; q++)
      {
        if (s[cc[q]] < (size_t)n)
        {
          ncc[k] = s[cc[q]];
          nvv[k] = vv[q];
          k++;
        }
      }
    }    
    nrr[m] = k;

    output = dynamic_cast<Matrix*>(scinew SparseRowMatrix(m,columns.size(),nrr,ncc,k,nvv));
    if (output.get_rep() == 0)
    {
      pr->error("MatrixSelectRows: Could not allocate output matrix");
      return (false);          
    }
		        
    return (true);
  }
  else
  {
    MatrixHandle mat = input->dense();
    
    try
    {
      output = scinew DenseMatrix(m,columns.size());
    }
    catch (...)
    {
      pr->error("SelectMatrixColumns: Could not allocate output matrix");
      return (false);
    }
      
    if (mat.get_rep() == 0)
    {
      pr->error("SelectMatrixColumns: Could not convert matrix into dense matrix");
      return (false);    
    }
    
    double* src = mat->get_data_pointer();
    double* dst = output->get_data_pointer(); 
    
    if (dst==0 || src == 0)
    {
      pr->error("SelectMatrixColumns: Could not allocate output matrix");
      return (false);
    }
    
    Matrix::size_type nc = static_cast<Matrix::size_type>(columns.size());
    
    for (Matrix::index_type p=0;p<m;p++)
    {
      for (Matrix::index_type q=0;q<nc;q++)
      {
        dst[p*nc + q] = src[p*n +columns[q]];
      }
    }
			
    return (true);
  }
}


} // end namespace SCIRunAlgo

