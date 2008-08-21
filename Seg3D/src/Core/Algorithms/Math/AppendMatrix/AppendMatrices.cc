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

#include <Core/Algorithms/Math/AppendMatrix/AppendMatrices.h>

#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
AppendMatricesAlgo::
run(std::vector<MatrixHandle>& inputs,MatrixHandle& output)
{
  algo_start("AppendMatrices",false);
  string method = get_option("method");
  
  // check whether we have any matrices
  if (inputs.size() == 0)
  {
    error("No input matrices");
    algo_end(); return (false);
  }
  
  // If there is one inout, nothing needs to be appended
  if (inputs.size() == 1)
  {
    output = inputs[0];
    algo_end(); return (true);
  }
  
  // separate algorithms for row and column appending
  if (method == "append_rows")
  {
    // Get size of matrix 1
    Matrix::size_type m =inputs[0]->nrows();
    Matrix::size_type n =inputs[0]->ncols();
    
    // check whether all matrices have the same number of columns
    for (size_t j=1; j<inputs.size();j++)
    {
      if (inputs[j]->ncols() != n)
      {
        error("A matrix with a different number of columns cannot be appended row wise");
        algo_end(); return (false);
      }
      m += inputs[j]->nrows();
    }

    // n and m will have new dimensions
    if (inputs[0]->is_sparse())
    {
      // sparse version
      SparseRowMatrix *srm = inputs[0]->sparse();
      Matrix::size_type nnz = srm->nnz;
      
      // Figure out the total number of non zeros
      for (size_t j=1; j<inputs.size(); j++)
      {
        //Only merge sparse matrices with sparse matrices
        if (!(inputs[j]->is_sparse()))
        {
          error("The algorithm does not support appending matrices of different types");
          algo_end(); return (false);
        }
        srm = inputs[j]->sparse();
        nnz += srm->nnz;
      }

      // Setup new matrix, reserve its memeroy
      Matrix::index_type* rr = 0;
      Matrix::index_type* cc = 0;
      double*             vv = 0;
      
      try
      {
        rr = new Matrix::index_type[m+1];
        cc = new Matrix::index_type[nnz];
        vv = new double[nnz];
      }
      catch(...)
      {
        if (rr) delete[] rr;
        if (cc) delete[] cc;
        if (vv) delete[] vv;
        error("Could not allocate output matrix");
        algo_end(); return (false);
      }
      
      Matrix::index_type ms = 0;
      Matrix::index_type me = 0;
      Matrix::index_type nns = 0;
      Matrix::index_type nne = 0;
      
      for (size_t j=0; j<inputs.size(); j++)
      {
        srm = inputs[j]->sparse();
        
        Matrix::index_type* rows = srm->rows;
        Matrix::index_type* columns = srm->columns;
        double* a = srm->a;

        me = ms + srm->nrows();
                
        for (Matrix::index_type r= ms; r<me; r++)
        {
          rr[r] = rows[r-ms]+nns;
        }
        ms = me;
        
        nne = nns+srm->nnz;
        for (Matrix::index_type c=nns; c<nne; c++)
        {
          cc[c] = columns[c-nns];
          vv[c] = a[c-nns];
        }
        nns = nne;
      }
      
      rr[m] = nnz;
      
      output = new SparseRowMatrix(m,n,rr,cc,nnz,vv);
      if (output.get_rep() == 0)
      {
        error("Could not allocate output matrix");
        algo_end(); return (false);          
      }
    }
    else if (inputs[0]->is_dense())
    {
      for (size_t j=1; j<inputs.size(); j++)
      {
        if (!(inputs[j]->is_dense()))
        {
          error("The algorithm does not support appending matrices of different types");
          algo_end(); return (false);
        }
      }

      // Create new matrix
      output = new DenseMatrix(m,n);
      if (output.get_rep() == 0)
      {
        error("Could not allocate output matrix");
        algo_end(); return (false);          
      }
     
      // copy all data sequentially into new matrix             
      Matrix::size_type offset = 0;
      double* ndata = output->get_data_pointer();
      
      for (size_t j=0; j<inputs.size(); j++)
      {      
        double* data = inputs[j]->get_data_pointer();
        Matrix::size_type size = inputs[j]->get_data_size();
        for (Matrix::index_type j=0; j<size; j++)
        {
          ndata[offset+j] = data[j];
        }
        offset += size;
      }
    }
    else if (inputs[0]->is_column())
    {
      for (size_t j=1; j<inputs.size(); j++)
      {
        if (!(inputs[j]->is_column()))
        {
          error("The algorithm does not support appending matrices of different types");
          algo_end(); return (false);
        }
      }

      // Create new matrix
      output = new ColumnMatrix(m);
      if (output.get_rep() == 0)
      {
        error("Could not allocate output matrix");
        algo_end(); return (false);          
      }
     
      // copy all data sequentially into new matrix             
      Matrix::size_type offset = 0;
      double* ndata = output->get_data_pointer();
      
      for (size_t j=0; j<inputs.size(); j++)
      {      
        double* data = inputs[j]->get_data_pointer();
        Matrix::size_type size = inputs[j]->get_data_size();
        for (Matrix::index_type j=0; j<size; j++)
        {
          ndata[offset+j] = data[j];
        }
        offset += size;
      }
    }
    else
    {
      error("This algorithm is not support for this type");
      algo_end(); return (false);                
    }
    
    algo_end(); return (true);
  }
  else
  {
    // Get size of matrix 1
    Matrix::size_type m =inputs[0]->nrows();
    Matrix::size_type n =inputs[0]->ncols();
    
    // check whether all matrices have the same number of columns
    for (size_t j=1; j<inputs.size();j++)
    {
      if (inputs[j]->nrows() != m)
      {
        error("A Matrix with a different number of rows cannot be appended column wise");
        algo_end(); return (false);
      }
      n += inputs[0]->ncols();
    }
  

    // n and m will have new dimensions
    if (inputs[0]->is_sparse())
    {
      // sparse version
      SparseRowMatrix *srm = inputs[0]->sparse();
      Matrix::size_type nnz = srm->nnz;

      Matrix::index_type* rr = new Matrix::index_type[m+1];
      
      if (rr == 0)
      {
        if (rr) delete[] rr;
        error("Could not allocate output matrix");
        algo_end(); return (false);
      }
            
      for (Matrix::size_type j=0; j<m+1; j++) rr[j] = 0;
      
      vector<SparseRowMatrix*> srms(inputs.size())
      ;
      // Figure out the total number of non zeros
      for (size_t j=1; j<inputs.size(); j++)
      {
        //Only merge sparse matrices with sparse matrices
        if (!(inputs[j]->is_sparse()))
        {
          error("The algorithm does not support appending matrices of different types");
          algo_end(); return (false);
        }
        srms[j] = inputs[j]->sparse();
        nnz += srms[j]->nnz;
        
        Matrix::index_type* rows = srms[j]->rows;
        for (Matrix::size_type r=1; r<m+1; r++)
        {
          rr[r] += rows[r]-rows[r-1];
        }
      }
    
      Matrix::size_type s = 0;
      for (Matrix::size_type r=1;r<m+1;r++)
      {
        s += rr[r]; rr[r] = s;
      }
       
      // Setup new matrix, reserve its memory
      Matrix::index_type* cc = new Matrix::index_type[nnz];
      double*             vv = new double[nnz];
    
      if (cc == 0 || vv == 0)
      {
        if (cc) delete[] cc;
        if (vv) delete[] vv;
        error("Could not allocate output matrix");
        algo_end(); return (false);
      }

      Matrix::index_type k = 0;
      for (Matrix::size_type r=0;r<m;r++)
      {
        for (size_t j=0; j < srms.size(); j++)
        {
          Matrix::index_type* rows = srms[j]->rows;
          Matrix::index_type* columns = srms[j]->columns;
          double* values = srms[j]->a;
          Matrix::index_type start = rows[r];
          Matrix::index_type end = rows[r+1];
          for(Matrix::index_type i= start; i< end; i++, k++)
          {
            cc[k] = columns[i];
            vv[k] = values[i];
          }
        }
      }
      
      output = new SparseRowMatrix(m,n,rr,cc,nnz,vv);
      if (output.get_rep() == 0)
      {
        error("Could not allocate output matrix");
        algo_end(); return (false);          
      }
    }
    else if (inputs[0]->is_dense())
    {
      for (size_t j=1; j<inputs.size(); j++)
      {
        if (!(inputs[j]->is_dense()))
        {
          error("The algorithm does not support appending matrices of different types");
          algo_end(); return (false);
        }
      }

      // Create new matrix
      output = new DenseMatrix(m,n);
      if (output.get_rep() == 0)
      {
        error("Could not allocate output matrix");
        algo_end(); return (false);          
      }
     
      // copy all data sequentially into new matrix             
      Matrix::size_type offset = 0;
      double* ndata = output->get_data_pointer();
      
      for (size_t j=0; j<inputs.size(); j++)
      {      
        double* data = inputs[j]->get_data_pointer();
        Matrix::size_type mm = inputs[j]->ncols();
        Matrix::index_type k = 0;
        for (Matrix::index_type p=0; p<n; p++)
          for(Matrix::index_type q=0; q<mm; q++, k++)
          {
            ndata[p*m+q+offset] = data[k];
          }
        offset += mm;  
      }
    }
    else if (inputs[0]->is_column())
    {
      for (size_t j=1; j<inputs.size(); j++)
      {
        if (!(inputs[j]->is_column()))
        {
          error("The algorithm does not support appending matrices of different types");
          algo_end(); return (false);
        }
      }
      
      // Create new matrix
      output = new DenseMatrix(m,n);
      if (output.get_rep() == 0)
      {
        error("Could not allocate output matrix");
        algo_end(); return (false);          
      }
     
      // copy all data sequentially into new matrix             
      Matrix::size_type offset = 0;
      double* ndata = output->get_data_pointer();
      
      for (size_t j=0; j<inputs.size(); j++)
      {      
        double* data = inputs[j]->get_data_pointer();
        Matrix::index_type k = 0;
        for (Matrix::index_type p=0; p<n; p++, k++)
        {
          ndata[p*m+offset] = data[k];
        }
        offset++;  
      }
    }
    else
    {
      error("This algorithm is not support for this type");
      algo_end(); return (false);                
    }
  }
  return(false);
}


} // end namespace SCIRunAlgo
