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

#include <Core/Algorithms/Math/FindMatrixColumns.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool 
FindMatrixColumnsAlgo::FindNonZeroMatrixColumns(ProgressReporter* pr, MatrixHandle input, std::vector<Matrix::index_type>& columns)
{
  if (input.get_rep() == 0)
  {
    pr->error("FindNonZeroMatrixColumns: No input matrix");
    return (false);
  }

  columns.clear();

  Matrix::size_type m = input->nrows();
  Matrix::size_type n = input->ncols();
      
  if (input->is_sparse())
  {
    Matrix::index_type *rr = input->get_row();
    Matrix::index_type *cc = input->get_col();
    double *vv = input->get_val();
    
    if (rr==0 || cc==0 || vv==0)
    {
      pr->error("FindNonZeroMatrixColumns: Sparse matrix is invalid");
      return (false);      
    }

    std::vector<bool> s(n,false);
    
    for (Matrix::index_type r=0; r<rr[m];r++)
    {
      if (vv[r] != 0.0)
      {
        s[cc[r]] = true;
      }
    }

    for (Matrix::index_type q=0; q<n; q++)
    {
      if (s[q]) columns.push_back(q);
    }
    
    return (true);
  }  
  else
  {
    MatrixHandle mat = input->dense();
    if (mat.get_rep() == 0)
    {
      pr->error("FindNonZeroMatrixColumns: Could not convert matrix into dense matrix");
      return (false);    
    }
  
    double *data = mat->get_data_pointer();
    for (Matrix::index_type p=0; p<n; p++)
    {
      Matrix::index_type q = 0;
      for (q=0;q<m; q++)
      {
        if (data[p*m+q] != 0.0) break;
      }
      if (q != m) columns.push_back(p);
    }
  
    return (true);
  }
}

bool 
FindMatrixColumnsAlgo::FindZeroMatrixColumns(ProgressReporter* pr, MatrixHandle input, std::vector<Matrix::index_type>& columns)
{
  if (input.get_rep() == 0)
  {
    pr->error("FindZeroMatrixColumns: No input matrix");
    return (false);
  }

  columns.clear();

  Matrix::size_type m = input->nrows();
  Matrix::size_type n = input->ncols();
      
  if (input->is_sparse())
  {
    Matrix::index_type *rr = input->get_row();
    Matrix::index_type *cc = input->get_col();
    double *vv = input->get_val();
    
    if (rr==0 || cc==0 || vv==0)
    {
      pr->error("FindZeroMatrixColumns: Sparse matrix is invalid");
      return (false);      
    }

    std::vector<bool> s(n,false);
    
    for (Matrix::index_type r=0; r<rr[m];r++)
    {
      if (vv[r] != 0.0)
      {
        s[cc[r]] = true;
      }
    }

    for (Matrix::index_type q=0; q<n; q++) if (!(s[q])) columns.push_back(q);
    
    return (true);
  }  
  else
  {
    MatrixHandle mat = input->dense();
    if (mat.get_rep() == 0)
    {
      pr->error("FindZeroMatrixColumns: Could not convert matrix into dense matrix");
      return (false);    
    }
  

    double *data = mat->get_data_pointer();
    for (Matrix::index_type p=0; p<n; p++)
    {
      Matrix::index_type q = 0;
      for (q=0;q<m; q++)
      {
        if (data[p*m+q] != 0.0) break;
      }
      if (q != m) columns.push_back(static_cast<Matrix::index_type>(p));
    }
  
    return (true);
  }
}

} // end namespace SCIRunAlgo
