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

#include <Core/Algorithms/Math/AppendMatrix/AppendMatrix.h>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
AppendMatrixAlgo::
run(MatrixHandle input,MatrixHandle& output,
    MatrixHandle append)
{
  std::vector<Matrix::index_type> dummy;
  return(run(input,output,append,dummy));
}

bool
AppendMatrixAlgo::
run(MatrixHandle input,MatrixHandle& output,
    MatrixHandle append,std::vector<Matrix::index_type>& indices)
{
  algo_start("AppendMatrix",false);
  string method = get_option("method");
  
  if (method == "append_rows")
  {
    if (input.get_rep() == 0)
    {
      if (append.get_rep() == 0)
      {
        output = 0;
        warning("Base matrix and matrix to append are empty");
        algo_end(); return (true);
      }
    
      output = append;
      Matrix::size_type m = append->nrows();
      indices.resize(m);
      for (Matrix::index_type r=0; r<m; r++) indices[r] = r;
      algo_end(); return (true);
    }

    if (append.get_rep() == 0)
    {
      output = input;
      indices.clear();
      warning("Matrix to append is empty");
      algo_end(); return (true);
    } 
    
    Matrix::size_type m = input->nrows();
    Matrix::size_type n = input->ncols();
    
    Matrix::size_type am = append->nrows();
    Matrix::size_type an = append->ncols();
          
    if (an != n)
    {
      error("The number of columns in input matrix is not equal to number of columns in row matrix");
      algo_end(); return (false);
    }
    
    Matrix::index_type newm = m+am;
    Matrix::index_type newn = n;
    
    if (input->is_sparse())
    {
      MatrixHandle a = dynamic_cast<Matrix *>(append->sparse());
      if (a.get_rep() == 0)
      {
        error("Could not convert matrix to sparse matrix");
        algo_end(); return (false);      
      }
    
      Matrix::index_type *rr = input->get_row();
      Matrix::index_type *cc = input->get_col();
      double *vv = input->get_val();
    
      Matrix::index_type *arr = a->get_row();
      Matrix::index_type *acc = a->get_col();
      double *avv = a->get_val();
   
      if (rr==0 || cc==0 || vv==0 || arr==0 || acc == 0 || avv == 0)
      {
        error("Sparse matrix is invalid");
        algo_end(); return (false);      
      }   
    
      Matrix::size_type newnnz = rr[m]+arr[am];
      Matrix::index_type *nrr = new Matrix::index_type[newm+1];
      Matrix::index_type *ncc = new Matrix::index_type[newnnz];
      double *nvv = new double[newnnz];
      
      if (nrr==0 || ncc==0 || nvv==0)
      {
        if (nrr) delete[] nrr;
        if (ncc) delete[] ncc;
        if (nvv) delete[] nvv;
        error("Could not allocate output matrix");
        algo_end(); return (false);      
      }

      for (Matrix::index_type r=0;r<m;r++) nrr[r] = rr[r];
      for (Matrix::index_type r=0;r<am;r++) nrr[m+r] = arr[r]+rr[m];
      nrr[newm] = newnnz;
      
      Matrix::size_type nnz = rr[m];
      for (Matrix::index_type r=0;r<nnz;r++) { ncc[r] = cc[r]; nvv[r] = vv[r]; }
      Matrix::size_type annz = arr[am];
      for (Matrix::index_type r=0;r<annz;r++) { ncc[r+nnz] = acc[r]; nvv[r+nnz] = avv[r]; } 
      
      indices.resize(newm-m);
      for (Matrix::index_type r=m;r<newm;r++) indices[r-m]=r;
      
      output = dynamic_cast<Matrix*>(new SparseRowMatrix(newm,newn,nrr,ncc,newnnz,nvv));
      if (output.get_rep() == 0)
      {
        error("Could not allocate output matrix");
        algo_end(); return (false);          
      }
      
      algo_end(); return (true);
    }
    else
    {
      MatrixHandle i = dynamic_cast<Matrix *>(input->dense());
      MatrixHandle a = dynamic_cast<Matrix *>(append->dense());
      if (a.get_rep() == 0 || i.get_rep() == 0)
      {
        error("Could not convert matrix to dense matrix");
        algo_end(); return (false);      
      }    
    
      output = dynamic_cast<Matrix *>(new DenseMatrix(newm,newn));
      
      double *iptr = i->get_data_pointer();
      double *aptr = a->get_data_pointer();
      double *optr = output->get_data_pointer();
      
      if (aptr == 0 || iptr == 0 ||optr == 0)
      {
        error("Could not convert matrix to dense matrix");
        algo_end(); return (false);      
      }    
      
      Matrix::index_type mn = m*n;
      for (Matrix::index_type r=0; r<mn;r++) optr[r] = iptr[r];
      optr += mn;
      mn = am*an;
      for (Matrix::index_type r=0; r<mn;r++) optr[r] = aptr[r]; 
    
      indices.resize(newm-m);
      for (Matrix::index_type r=m;r<newm;r++) indices[r-m]=r;
      
      algo_end(); return (true);  
    }
  
  
  }
  else
  {
    if (input.get_rep() == 0)
    {
      if (append.get_rep() == 0)
      {
        output = 0;
        warning("MatrixAppendColumns: Base matrix and matrix to append are empty");
        return (true);
      }
    
      output = append;
      Matrix::size_type n = append->ncols();
      indices.resize(n);
      for (Matrix::index_type r=0; r<n; r++) indices[r] = r;
      algo_end(); return (true);
    }


    if (append.get_rep() == 0)
    {
      output = input;
      indices.clear();
      algo_end(); return (true);
    }  
    
    Matrix::size_type m = input->nrows();
    Matrix::size_type n = input->ncols();
    
    Matrix::size_type am = append->nrows();
    Matrix::size_type an = append->ncols();
    
    if (am != m)
    {
      error("MatrixAppendColumns: The number of rows in input matrix is not equal to number of rows in column matrix");
      algo_end(); return (false);
    }
    
    Matrix::size_type newm = m;
    Matrix::size_type newn = n+an;
    
    if (input->is_sparse())
    {
      MatrixHandle a = dynamic_cast<Matrix *>(append->sparse());
      if (a.get_rep() == 0)
      {
        error("MatrixAppendColumns: Could not convert matrix to sparse matrix");
        algo_end(); return (false);      
      }
    
      Matrix::index_type *rr = input->get_row();
      Matrix::index_type *cc = input->get_col();
      double *vv = input->get_val();
    
      Matrix::index_type *arr = a->get_row();
      Matrix::index_type *acc = a->get_col();
      double *avv = a->get_val();
   
      if (rr==0 || cc==0 || vv==0 || arr==0 || acc == 0 || avv == 0)
      {
        error("Sparse matrix is invalid");
        algo_end(); return (false);      
      }   
    
      Matrix::size_type newnnz = rr[m]+arr[am];
      Matrix::index_type *nrr = new Matrix::index_type[newm];
      Matrix::index_type *ncc = new Matrix::index_type[newnnz];
      double *nvv = new double[newnnz];
      
      if (nrr==0 || ncc==0 || nvv==0)
      {
        if (nrr) delete[] nrr;
        if (ncc) delete[] ncc;
        if (nvv) delete[] nvv;
        error("Could not allocate output matrix");
        algo_end(); return (false);      
      }

      Matrix::index_type k = 0;
      for (Matrix::index_type r=0;r<m;r++) 
      {
        nrr[r] = k;
        for (Matrix::index_type q=rr[r];q<rr[r+1]; q++)
        {
          ncc[k] = cc[q];
          nvv[k] = vv[q];
          k++;
        }
        for (Matrix::index_type q=arr[r];q<arr[r+1]; q++)
        {
          ncc[k] = acc[q];
          nvv[k] = avv[q];
          k++;
        }    
      }
      nrr[m] = k;
      
      indices.resize(newn-n);
      for (Matrix::index_type r=n;r<newn;r++) indices[r-n] = r;

      output = dynamic_cast<Matrix*>(new SparseRowMatrix(newm,newn,nrr,ncc,newnnz,nvv));
      if (output.get_rep() == 0)
      {
        error("Could not allocate output matrix");
        algo_end(); return (false);          
      }
          
      algo_end(); return (true);
    }
    else
    {
      MatrixHandle i = dynamic_cast<Matrix *>(input->dense());
      MatrixHandle a = dynamic_cast<Matrix *>(append->dense());
      if (a.get_rep() == 0 || i.get_rep() == 0)
      {
        error("Could not convert matrix to dense matrix");
        algo_end(); return (false);      
      }    
    
      output = dynamic_cast<Matrix *>(new DenseMatrix(newm,newn));
      
      double *iptr = i->get_data_pointer();
      double *aptr = a->get_data_pointer();
      double *optr = output->get_data_pointer();
      
      if (aptr == 0 || iptr == 0 ||optr == 0)
      {
        error("Could not convert matrix to dense matrix");
        algo_end(); return (false);      
      }    
      
      for (Matrix::index_type r=0; r<m; r++)
      {
        for (Matrix::index_type q=0;q<n;q++) 
        { 
          optr[q] = iptr[q];
        }
        optr += n;
        iptr += n;
        for (Matrix::index_type q=0;q<an;q++) 
        { 
          optr[q] = aptr[q];
        }
        optr += an;
        aptr += an;
      }
      
      indices.resize(newn-n);
      for (Matrix::index_type r=n;r<newn;r++) indices[r-n] = r;
      
      algo_end(); return (true);
    }
  }
}


} // end namespace SCIRunAlgo
