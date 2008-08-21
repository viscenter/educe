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

#include <Core/Algorithms/Math/SelectMatrix/SelectMatrixRows.h>

#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>

// STL Fucntions we need
#include <algorithm>
#include <vector>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
SelectMatrixRowsAlgo::
run(MatrixHandle input, MatrixHandle& output, MatrixHandle rows)
{
  algo_start("SelectMatrixRows",false);

  Matrix::size_type num_sel_rows = rows->get_data_size();
  double* sel_rows_ptr = rows->get_data_pointer();
  
  std::vector<Matrix::index_type> sel_rows;
  try
  {
    sel_rows.resize(num_sel_rows);
    for (int p=0; p< num_sel_rows; p++) 
      sel_rows[p] = static_cast<Matrix::index_type>(sel_rows_ptr[p]);
  }
  catch (...)
  {
    error("Could not allocate enough memory");
    algo_end(); return (false); 
  }
  
  bool ret = run(input,output,sel_rows);
  algo_end(); return(ret);
}

bool
SelectMatrixRowsAlgo::
run(MatrixHandle input, MatrixHandle& output, 
    std::vector<Matrix::index_type>& rows)
{
  algo_start("SelectMatrixRows",false);

  if (input.get_rep() == 0)
  {
    error("No input matrix");
    algo_end(); return (false);
  }

  if (rows.size() == 0)
  {
    error("No row indices given");
    algo_end(); return (false);  
  }
  
  Matrix::size_type m = input->nrows();
  Matrix::size_type n = input->ncols();
    
  for (size_t r=0; r<rows.size(); r++)
  {
    if (rows[r] >= static_cast<Matrix::index_type>(m))
    {
      error("Selected row exceeds matrix dimensions");
      algo_end(); return (false);
    }
  }
  
  if (input->is_sparse())
  {
    Matrix::index_type *rr = input->get_row();
    Matrix::index_type *cc = input->get_col();
    double *vv = input->get_val();
    
    if (rr==0 || cc==0 || vv == 0)
    {
      error("Sparse matrix is invalid");
      algo_end(); return (false);      
    }
    
    Matrix::index_type k =0;
    for (size_t r=0; r<rows.size(); r++)
    {
      k += rr[rows[r]+1]-rr[rows[r]];
    }
        
    if (k==0) k=1; // we need to allocate memory no matter what, it is required by the SparseRowMatrix

    Matrix::index_type *nrr = 0;
    Matrix::index_type *ncc = 0;
    double *nvv = 0;
    
    try
    {
      nrr = new Matrix::index_type[rows.size()+1];
      ncc = new Matrix::index_type[k];
      nvv = new double[k];
    }
    catch (...)
    {
      if (nrr) delete[] nrr;
      if (ncc) delete[] ncc;
      if (nvv) delete[] nvv;
      error("Could not allocate output matrix");
      algo_end(); return (false);      
    }    
    
    k = 0;
    for (size_t r=0; r<rows.size(); r++)
    {
      nrr[r] = k;
      for (Matrix::index_type q=rr[rows[r]]; q<rr[rows[r]+1]; q++, k++)
      {
        ncc[k] = cc[q];
        nvv[k] = vv[q];
      }
    }
    nrr[rows.size()] = k;

    output = dynamic_cast<Matrix*>(new SparseRowMatrix(rows.size(),n,nrr,ncc,k,nvv));
    if (output.get_rep() == 0)
    {
      error("Could not allocate output matrix");
      algo_end(); return (false);          
    }
    
    algo_end(); return (true);
  }
  else
  {
    MatrixHandle mat;
    try
    {
      mat = input->dense();
      
      if (mat.get_rep() == 0)
      {
        error("Could not convert matrix into dense matrix");
        algo_end(); return (false);    
      }
      
      output = new DenseMatrix(rows.size(),n);
      
      if (output.get_rep() == 0)
      {
        error("Could not allocate space for output matrix");
        algo_end(); return (false);    
      }
    }
    catch (...)
    {
      error("Could not allocate output matrix");
      algo_end(); return (false);
    }
    
    double* src = mat->get_data_pointer();
    double* dst = output->get_data_pointer(); 
    
    if (dst==0 || src == 0)
    {
      error("Could not allocate output matrix");
      algo_end(); return (false);
    }
    
    Matrix::index_type nr = static_cast<Matrix::index_type>(rows.size());
    
    for (Matrix::index_type p=0;p<nr;p++)
    {
      for (Matrix::index_type q=0;q<n;q++)
      {
        dst[p*n + q] = src[rows[p]*n +q];
      }
    }
				
    algo_end(); return (true);
  }
}

} // end namespace SCIRunAlgo
