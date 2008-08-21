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
#include <Core/Datatypes/MatrixOperations.h>

#include <sstream>

namespace SCIRunAlgo {

using namespace SCIRun;

MathAlgo::MathAlgo(ProgressReporter* pr) :
  AlgoLibrary(pr)
{
}

bool
MathAlgo::ResizeMatrix(MatrixHandle input, MatrixHandle& output, 
                       Matrix::size_type m, Matrix::size_type n)
{ 
  if (input.get_rep() == 0)
  {
    error("ResizeMatrix: No input matrix was given");
    return (false);
  }
  
  if (input->is_sparse())
  {
    double* val = input->get_val();
    Matrix::index_type* row = input->get_row();
    Matrix::index_type* col = input->get_col();
    Matrix::size_type   sm = input->nrows();
 
    Matrix::size_type newnnz=0;
    for (Matrix::index_type p=1; p<(m+1); p++)
    {
      if (p <= sm)
      {
        for (Matrix::index_type q = row[p-1]; q < row[p]; q++)
        {
          if (col[q] < n) newnnz++;
        }
      }
    }
 
    double* newval = new double[newnnz];  
    Matrix::index_type* newcol = new Matrix::index_type[newnnz];
    Matrix::index_type* newrow = new Matrix::index_type[m+1];
    
    if ((newval == 0)||(newcol == 0)||(newrow == 0))
    {
      error("ResizeMatrix: Could not allocate output matrix");
      if (newval) delete newval;
      if (newcol) delete newcol;
      if (newrow) delete newrow;
      return (false);
    }
    
    Matrix::index_type k = 0;
    newrow[0] = 0;
    
    for (Matrix::index_type p=1; p<(m+1); p++)
    {
      if (p <= sm)
      {
        for (Matrix::index_type q = row[p-1]; q < row[p]; q++)
        {
          if (col[q] < n) 
          {
            newval[k] = val[q];
            newcol[k] = col[q];
            k++;
          }        
        }
      }
      newrow[p] = k;
    }
    
    output = new SparseRowMatrix(m,n,newrow,newcol,newnnz,newval);
    if (output.get_rep() == 0)
    {
      error("ResizeMatrix: Could not allocate output matrix");
      if (newval) delete newval;
      if (newcol) delete newcol;
      if (newrow) delete newrow;
      return false;    
    }
    return true;
  }
  else
  {
    MatrixHandle mat = input->dense();
    output = new DenseMatrix(m,n);
    if (output.get_rep() == 0)
    {
      error("ResizeMatrix: Could not allocate output matrix");
      return false;
    }
  
    Matrix::size_type sm = input->nrows();
    Matrix::size_type sn = input->ncols();
    
    double *src = input->get_data_pointer();
    double *dst = output->get_data_pointer();
    
    Matrix::index_type p,q;
    for (p=0;(p<m)&&(p<sm);p++)
    {
      for (q=0;(q<n)&&(q<sn);q++)
      {
        dst[q+p*n] = src[q+p*sn];
      }
      for (;q<n;q++) dst[q+p*n] = 0.0;
    }
    for (;p<m;p++)
      for (q=0;q<n;q++) dst[q+p*n]= 0.0;
   
    return true;
  }
  
  return false;
}

bool 
MathAlgo::IdentityMatrix(Matrix::size_type n,MatrixHandle& output)
{
  Matrix::index_type *rows = new Matrix::index_type[n+1];
  Matrix::index_type *cols = new Matrix::index_type[n];
  double* vals = new double[n];
  
  if ((rows==0)||(cols==0)||(vals==0))
  {
    if (rows) delete[] rows;
    if (cols) delete[] cols;
    if (vals) delete[] vals;
    error("IdentityMatrix: Could not allocate output matrix");
    return (false);  
  }

  for (Matrix::index_type r=0; r<n+1; r++)
  {
    rows[r] = r;
  }
  
  for (Matrix::index_type c=0; c<n; c++) 
  {
    cols[c] = c;
    vals[c] = 1.0;
  }
  
  output = new SparseRowMatrix(n,n,rows,cols,n,vals);
  if (output.get_rep()) return (true);

  error("IdentityMatrix: Could not allocate output matrix");  
  return (false);  
}



bool
MathAlgo::CreateSparseMatrix(SparseElementVector& input,
                             MatrixHandle& output, 
                             Matrix::size_type m, Matrix::size_type n)
{
  std::sort(input.begin(),input.end());
  
  Matrix::size_type nnz = 1;
  size_t q = 0;
  for (size_t p=1; p < input.size(); p++)
  {
    if (input[p] == input[q])
    {
      input[q].val += input[p].val; 
	  input[p].val = 0.0;
    }
    else
    {
      nnz++;
      q = p;
    }
  }
  
  // reserve memory
  
  Matrix::index_type *rows = new Matrix::index_type[m+1];
  Matrix::index_type *cols = new Matrix::index_type[nnz];
  double *vals = new double[nnz];
  
  if ((rows == 0)||(cols == 0)||(vals == 0))
  {
    if (rows) delete[] rows;
    if (cols) delete[] cols;
    if (vals) delete[] vals;
    error("CreateSparseMatrix: Could not allocate memory for matrix");
    return false;
  }
  
  rows[0] = 0;
  q = 0;
  
  Matrix::index_type k = 0;
  for (Matrix::index_type p=0; p < m; p++)
  {
    while ((q < input.size()) && (input[q].row == p)) 
	{ 
	  if (input[q].val)
	  {
	    cols[k] = input[q].col; 
	    vals[k] = input[q].val;
		k++; 
	  }
	  q++; 
	}
    rows[p+1] = k;
  }   
  
  output = new SparseRowMatrix(m,n,rows,cols,nnz,vals);
  if (output.get_rep()) return true;
  
  return false;
}


bool
MathAlgo::ApplyRowOperation(MatrixHandle input, MatrixHandle& output,
                            std::string method)
{
  if (input.get_rep() == 0)
  {
    error("ApplyRowOperation: no input matrix found");
    return false;
  }
  
  Matrix::size_type nrows = input->nrows();
  Matrix::size_type ncols = input->ncols();
  
  output = new DenseMatrix(nrows, 1);
  if (output.get_rep() == 0)
  {
    error("ApplyRowOperation: could not create output matrix");
    return false;  
  }

  double *dest = output->get_data_pointer();
  for (Matrix::index_type q=0; q<nrows; q++) dest[q] = 0.0;

  if (input->is_sparse())
  {
    Matrix::index_type *rows = input->sparse()->rows;
    double *vals = input->sparse()->a;
    
    if (method == "Sum")
    {
      for (Matrix::index_type q=0; q<nrows; q++) { for (Matrix::index_type r = rows[q]; r < rows[q+1]; r++) dest[q] += vals[r]; }        
    }
    else if ((method == "Average")||(method == "Mean"))
    {
      for (Matrix::index_type q=0; q<nrows; q++) 
      {
        for (Matrix::index_type r = rows[q]; r < rows[q+1]; r++) dest[q] += vals[r];  
        dest[q] /= static_cast<double>(ncols);  
      }            
    }
    else if (method == "Norm")
    {
      for (Matrix::index_type q=0; q<nrows; q++) 
      {
        for (Matrix::index_type r = rows[q]; r < rows[q+1]; r++) dest[q] += (vals[r])*(vals[r]);  
        if (rows[q] != rows[q+1]) dest[q] = sqrt(dest[q]);  
      }                
    }
    else if (method == "Variance")
    {
      double mean;
      for (Matrix::index_type q=0; q<nrows; q++) 
      {
        mean = 0.0;
        for (Matrix::index_type r = rows[q]; r < rows[q+1]; r++) mean += vals[r];
        if (ncols) mean /= static_cast<double>(ncols);
        for (Matrix::index_type r = rows[q]; r < rows[q+1]; r++) dest[q] += (vals[r]-mean)*(vals[r]-mean);
        dest[q] += (ncols-(rows[q+1]-rows[q]))*mean*mean;  
        if (ncols > 1) dest[q] = dest[q]/static_cast<double>(ncols-1); else dest[q] = 0.0;
      }               
    }
    else if (method == "StdDev")
    {
      double mean;
      for (Matrix::index_type q=0; q<nrows; q++) 
      {
        mean = 0.0;
        for (Matrix::index_type r = rows[q]; r < rows[q+1]; r++) mean += vals[r];
        if (ncols) mean /= static_cast<double>(ncols);
        for (Matrix::index_type r = rows[q]; r < rows[q+1]; r++) dest[q] += (vals[r]-mean)*(vals[r]-mean);  
        dest[q] += (ncols-(rows[q+1]-rows[q]))*mean*mean;  
        if (ncols > 1) dest[q] = sqrt(dest[q]/static_cast<double>(ncols-1)); else dest[q] = 0.0;
      }               
    }
    else if (method == "Maximum")
    {
      for (Matrix::index_type q=0; q<nrows; q++) 
      {
        if (rows[q+1]-rows[q] == ncols) dest[q] = -DBL_MAX; else dest[q] = 0.0;
        for (Matrix::index_type r = rows[q]; r < rows[q+1]; r++) if (vals[r] > dest[q]) dest[q] = vals[r];
      }                   
    }
    else if (method == "Minimum")
    {
      for (Matrix::index_type q=0; q<nrows; q++) 
      {
        if (rows[q+1]-rows[q] == ncols) dest[q] = DBL_MAX; else dest[q] = 0.0;
        for (Matrix::index_type r = rows[q]; r < rows[q+1]; r++) if (vals[r] < dest[q]) dest[q] = vals[r];
      }                   
    }
    else if (method == "Median")
    {
      for (Matrix::index_type q=0; q<nrows; q++) 
      {
        std::vector<double> dpos;
        std::vector<double> dneg;
        for (Matrix::index_type r = rows[q]; r < rows[q+1]; r++) if (vals[r] < 0.0 ) dneg.push_back(vals[r]); else dpos.push_back(vals[r]);
        if (dpos.size() >= static_cast<size_t>(ncols/2))
          std::sort(dpos.begin(),dpos.end());
        if (dneg.size() >= static_cast<size_t>(ncols/2))
          std::sort(dneg.begin(),dneg.end());
        if (2*(ncols/2) == ncols && ncols)
        {
          double val1 = 0.0;
          double val2 = 0.0;
          Matrix::size_type q1 = (ncols/2)-1;
          Matrix::size_type q2 = ncols/2; 
          if ( q1 < static_cast<Matrix::size_type>(dneg.size())) val1 = dneg[q1];
          if ( q2 < static_cast<Matrix::size_type>(dneg.size())) val2 = dneg[q2];
          if ( (ncols-1)-q1 < static_cast<Matrix::size_type>(dpos.size())) 
              val1 = dpos[dpos.size()-ncols+q1];
          if ( (ncols-1)-q2 < static_cast<Matrix::size_type>(dpos.size())) 
              val2 = dpos[dpos.size()-ncols+q2];          
          dest[q] = 0.5*(val1+val2);
        }
        else
        {
          double val1 = 0.0;
           Matrix::size_type q1 = (ncols/2);
          if ( q1 < static_cast<Matrix::size_type>(dneg.size())) val1 = dneg[q1];
          if ( (ncols-1)-q1 < static_cast<Matrix::size_type>(dpos.size()))
            val1 = dpos[dpos.size()-ncols+q1];
          dest[q] = val1;
        }       
      }                   
    }
    else
    {
      error ("ApplyRowOperation: This method has not yet been implemented");
      return false;
    }
  }
  else
  {
    DenseMatrix* mat = input->dense();
    
     Matrix::size_type m = mat->nrows();
     Matrix::size_type n = mat->ncols();
    double* data = mat->get_data_pointer();
  
    if (method == "Sum")
    {
      Matrix::index_type k = 0;
      for (Matrix::index_type p=0; p<m; p++) dest[p] = 0.0;
      for (Matrix::index_type p=0; p<m; p++) 
        for (Matrix::index_type q=0; q<n; q++) dest[p] += data[k++]; 
    }
    else if (method == "Mean")
    {
      Matrix::index_type k = 0;
      for (Matrix::index_type p=0; p<m; p++) dest[p] = 0.0;
      for (Matrix::index_type p=0; p<m; p++) 
        for (Matrix::index_type q=0; q<n; q++) dest[p] += data[k++]; 
      for (Matrix::index_type p=0; p<m; p++) dest[p] /= static_cast<double>(n);  
    }
    else if (method == "Variance")
    {
      std::vector<double> mean(m);
      Matrix::index_type k = 0;
      for (Matrix::index_type p=0; p<m; p++) dest[p] = 0.0;
      for (Matrix::index_type p=0; p<m; p++) 
        for (Matrix::index_type q=0; q<n; q++) mean[p] += data[k++];
      for (Matrix::index_type p=0; p<m; p++) mean[p] /= static_cast<double>(n);  
      k = 0;
      for (Matrix::index_type p=0; p<m; p++) 
        for (Matrix::index_type q=0; q<n; q++, k++) 
          dest[p] += (data[k]-mean[p])*(data[k]-mean[p]); 
      for (Matrix::index_type p=0; p<m; p++)  
        if (n > 1) dest[p] /= static_cast<double>(n-1);  else dest[p] = 0.0;
    }
    else if (method == "StdDev")
    {
      std::vector<double> mean(m);
      Matrix::index_type k = 0;
      for (Matrix::index_type p=0; p<m; p++) dest[p] = 0.0;
      for (Matrix::index_type p=0; p<m; p++) 
        for (Matrix::index_type q=0; q<n; q++) mean[p] += data[k++];
      for (Matrix::index_type p=0; p<m; p++) mean[p] /= static_cast<double>(n);  
      k = 0;
      for (Matrix::index_type p=0; p<m; p++) 
        for (Matrix::index_type q=0; q<n; q++, k++) dest[p] += (data[k]-mean[p])*(data[k]-mean[p]); 
      for (Matrix::index_type p=0; p<m; p++)  
        if (n > 1) dest[p] = sqrt(dest[p]/static_cast<double>(n-1));  
        else dest[p] = 0.0;
    }
    else if (method == "Norm")
    {
      Matrix::index_type k = 0;
      for (Matrix::index_type p=0; p<m; p++) dest[p] = 0.0;
      for (Matrix::index_type p=0; p<m; p++) 
        for (Matrix::index_type q=0; q<n; q++, k++) dest[p] += data[k]*data[k]; 
      for (Matrix::index_type p=0; p<m; p++) dest[p] = sqrt(dest[p]); 
    }
    else if (method == "Maximum")
    {
      Matrix::index_type k = 0;
      for (Matrix::index_type p=0; p<m; p++) dest[p] = -DBL_MAX;
      for (Matrix::index_type p=0; p<m; p++) 
        for (Matrix::index_type q=0; q<n; q++, k++) 
          if (data[k] > dest[p]) dest[p] = data[k]; 
    }
    else if (method == "Minimum")
    {
      Matrix::index_type k = 0;
      for (Matrix::index_type p=0; p<m; p++) dest[p] = DBL_MAX;
      for (Matrix::index_type p=0; p<m; p++) 
        for (Matrix::index_type q=0; q<n; q++, k++) 
          if (data[k] < dest[p]) dest[p] = data[k]; 
    }
    else if (method == "Median")
    {
      Matrix::index_type k = 0;
      std::vector<double> v(n);

      for (Matrix::index_type p=0; p<m; p++)
      {
        for (Matrix::index_type q=0; q<n; q++, k++) v[q] = data[k];
        std::sort(v.begin(),v.end());
        if ((n/2)*2 == n)
        {
          dest[p] = 0.5*(v[n/2]+v[(n/2) -1]);
        }
        else
        {
          dest[p] = v[n/2];
        }
      }
    }
    else
    {
      error("ApplyRowOperation: This method has not yet been implemented");
      return false;    
    }
  }
  
  return true;
}


bool
MathAlgo::ApplyColumnOperation(MatrixHandle input, MatrixHandle& output,
                               std::string method)
{
  if (input.get_rep() == 0)
  {
    error("ApplyRowOperation: no input matrix found");
    return false;
  }
  MatrixHandle t = input->transpose();
  if(!(ApplyRowOperation(t,t,method))) return false;
  output = t->transpose();
  return true;
} 

} // end SCIRun namespace
