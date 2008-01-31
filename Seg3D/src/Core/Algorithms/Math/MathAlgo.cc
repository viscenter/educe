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
#include <Core/Algorithms/Math/FindMatrixRows.h>
#include <Core/Algorithms/Math/FindMatrixColumns.h>
#include <Core/Algorithms/Math/SelectMatrixColumns.h>
#include <Core/Algorithms/Math/SelectMatrixRows.h>
#include <Core/Algorithms/Math/SortMatrixRows.h>
#include <Core/Algorithms/Math/SortMatrixColumns.h>
#include <Core/Algorithms/Math/ConvertMappingMatrix.h>


#include <Core/Algorithms/Math/BuildFEMatrix.h>
#include <Core/Algorithms/Math/CreateFEDirichletBC.h>

#include <Core/Datatypes/MatrixOperations.h>

#include <sgi_stl_warnings_off.h>
#include <sstream>
#include <sgi_stl_warnings_on.h>

namespace SCIRunAlgo {

using namespace SCIRun;

MathAlgo::MathAlgo(ProgressReporter* pr) :
  AlgoLibrary(pr)
{
}


bool
MathAlgo::BuildFEMatrix(FieldHandle field, MatrixHandle& matrix, int num_proc,
                        MatrixHandle conductivity_table,
                        MatrixHandle GeomToComp, MatrixHandle CompToGeom)
{
  BuildFEMatrixAlgo algo;
  
  if(!(algo.BuildFEMatrix(pr_, field, matrix, conductivity_table, num_proc)))
    return false;
  
  if ((GeomToComp.get_rep()==0)&&(CompToGeom.get_rep()==0))
  {
    if (field->is_property("GeomToComp")&&field->is_property("CompToGeom"))
    {
      field->get_property("GeomToComp",GeomToComp);
      field->get_property("CompToGeom",CompToGeom);
      matrix = CompToGeom*matrix*GeomToComp;
    }  
    return true;
  }
  
  if (CompToGeom.get_rep() == 0) CompToGeom = GeomToComp->transpose();
  if (GeomToComp.get_rep() == 0) GeomToComp = CompToGeom->transpose();
  matrix = GeomToComp*matrix*CompToGeom;
  return true;
}


bool 
MathAlgo::CreateFEDirichletBC(MatrixHandle FEin, MatrixHandle RHSin, MatrixHandle BC, 
                          MatrixHandle& FEout, MatrixHandle& RHSout)
{
  CreateFEDirichletBCAlgo algo;
  return (algo.CreateFEDirichletBC(pr_, FEin, RHSin, BC, FEout, RHSout));
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
 
    double* newval = scinew double[newnnz];  
    Matrix::index_type* newcol = scinew Matrix::index_type[newnnz];
    Matrix::index_type* newrow = scinew Matrix::index_type[m+1];
    
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
    
    output = scinew SparseRowMatrix(m,n,newrow,newcol,newnnz,newval);
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
    output = scinew DenseMatrix(m,n);
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
  Matrix::index_type *rows = scinew Matrix::index_type[n+1];
  Matrix::index_type *cols = scinew Matrix::index_type[n];
  double* vals = scinew double[n];
  
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
  
  output = scinew SparseRowMatrix(n,n,rows,cols,n,vals);
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
  
  Matrix::index_type *rows = scinew Matrix::index_type[m+1];
  Matrix::index_type *cols = scinew Matrix::index_type[nnz];
  double *vals = scinew double[nnz];
  
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
  
  output = scinew SparseRowMatrix(m,n,rows,cols,nnz,vals);
  if (output.get_rep()) return true;
  
  return false;
}


bool
MathAlgo::ReverseCuthillmcKee(MatrixHandle im, MatrixHandle& om,
                              MatrixHandle& mapping, bool calcmapping)
{
  Matrix::index_type *rr, *cc;
  double *d;
  Matrix::size_type n,m,nnz;
  
  if (im.get_rep() == 0)
  {
    error("ReverseCuthillmcKee: No input matrix was found");
    return false;
  }
  
  if (im->ncols() != im->nrows())
  {
    error("ReverseCuthillmcKee: Matrix is not square");
    return false;  
  }
  
  if (im->is_sparse() == false) 
  {
    error("ReverseCuthillmcKee: Matrix is not sparse");
    return false;
  }
  SparseRowMatrix* sim = im->as_sparse();
  
  om = sim->clone();
  if (om.get_rep() == 0)
  {
    error("ReverseCuthillmcKee: Could not copy sparse matrix");
    return false;  
  }
  
  m  = sim->nrows();
  n  = sim->ncols();
  nnz = sim->nnz;
  rr = sim->rows;
  cc = sim->columns;
  d  = sim->a;
  
  // reserve mapping space  
  Matrix::index_type *mrr = 0, *mcc = 0;
  double *md = 0;

  Matrix::index_type *drr, *dcc;
  double *dd;

  if (calcmapping)
  {
    mrr = scinew Matrix::index_type[m+1];
    mcc = scinew Matrix::index_type[m];
    md  = scinew double[m];
    if ((mrr == 0)||(mcc == 0)||(md == 0))
    {
      if (mrr ) delete[] mrr;
      if (mcc ) delete[] mcc;
      if (md ) delete[] md;      

      error("ReverseCuthillmcKee: Could not reserve space for mapping matrix");    
      return false;
    }
    

  }

  SparseRowMatrix* som = om->as_sparse();  
  drr = som->rows;
  dcc = som->columns;
  dd  = som->a;

  Matrix::index_type *Q, *R, *S, *X;
  Matrix::index_type *degree;
  Matrix::index_type ns,nr,nq,nx,nss;
  
  // Count number of connections
  degree = drr;
  for (Matrix::index_type p = 0;p<m;p++) degree[p] = (degree[p+1] - degree[p]);
  
  // clear the mask of already processed nodes

  X = scinew Matrix::index_type[m];
  Q = scinew Matrix::index_type[m];
  S = scinew Matrix::index_type[m];
  R = scinew Matrix::index_type[m];
  
  if ((X==0)||(Q==0)||(S==0)||(R==0))
  {
    if (X) delete[] X;
    if (Q) delete[] Q;
    if (S) delete[] S;
    if (R) delete[] R;
    error("ReverseCuthillmcKee: Could not allocate enough memory");
    return false;
  }
  
  for (Matrix::index_type p=0;p<m;p++) Q[p] = 0;
  
  Matrix::index_type root = 0;
  for (Matrix::index_type p=0;p<m;p++) if((degree[p] < degree[root])&&(Q[p] == 0)) root = p;

  nss = 0;
  nr = 0;
  ns = 0;
  nq = 0;
  nx = 0;
  
  R[nr++] = root;
  Q[root] = 1; nq++;
  X[nx++] = root;
  
  nr = 0;
  for (Matrix::index_type p = rr[root];p<rr[root+1];p++) if (Q[cc[p]] == 0) { R[nr++] = cc[p]; Q[cc[p]] = 1; nq++; }
 
  while(1)
  {
    if(nr > 0) 
    { 
      for (Matrix::index_type i =0;i<nr;i++)
      {
        Matrix::index_type t;
        Matrix::index_type k = i;
        // sort entries by order
        for (Matrix::index_type j=i+1;j<nr;j++) { if (degree[R[k]] > degree[R[j]]) { k = j;}
        if (k > i)  {t = R[i]; R[i] = R[k]; R[k] = t;}  }
      }
      for (Matrix::index_type j=0;j<nr;j++) { X[nx++] = R[j]; S[ns++] = R[j]; }
      nr  = 0;
    }
    if (nx == m) break;
    
    if (nss < ns)
    {
      root = S[nss++];
      nr = 0;
      for (Matrix::index_type p = rr[root];p<rr[root+1];p++) if (Q[cc[p]] == 0) { R[nr++] = cc[p]; Q[cc[p]] = 1; nq++; }
    }
    else
    {
      for (Matrix::index_type p=0;p<m;p++) if(Q[p] == 0) { root = p; break;}
      for (Matrix::index_type p=0;p<m;p++) if((degree[p] < degree[root])&&(Q[p] == 0)) root = p;
      Q[root] = 1; nq++;
      X[nx++] = root;
      nr = 0;
      for (Matrix::index_type p = rr[root];p<rr[root+1];p++) if (Q[cc[p]] == 0) { R[nr++] = cc[p]; Q[cc[p]] = 1; nq++; }
    }    
  }

  Matrix::index_type t;
  // do reverse ordering CM -> RCM
  for (Matrix::index_type p=0;p<m;p++) { t = X[p]; X[p] = X[m-1-p]; X[m-1-p] = t; }
  
  // finish mapping matrix and inverse mapping matrix
  if (calcmapping)
  {
    for (Matrix::index_type p=0;p<(m+1);p++) { mrr[p] = p; }
    for (Matrix::index_type p=0;p<m;p++) { md[p] = 1.0;  }
    for (Matrix::index_type p=0;p<m;p++) { mcc[X[p]] = p; }
    mapping = scinew SparseRowMatrix(m,m,mrr,mcc,m,md);
  }
    
  for (Matrix::index_type p=0;p<m;p++) { Q[X[p]] = p; }
  
  // finish the reordering new matrix
  Matrix::index_type i,j;
  drr[0] = 0;
  i = 0;
  for (Matrix::index_type p=0;p<m;p++)
  {
    j = Q[p];
    drr[p+1] = drr[p]+(rr[j+1]-rr[j]);
    for (Matrix::index_type r=rr[j];r<rr[j+1];r++)
    {
      dcc[i] = X[cc[r]];
      dd[i] = d[r];
      i++;
    }
  }

  delete[] X;
  delete[] Q;
  delete[] S;
  delete[] R;

  return true;
} 
 

bool
MathAlgo::CuthillmcKee(MatrixHandle im, MatrixHandle& om,
                       MatrixHandle& mapping, bool calcmapping)
{
  Matrix::index_type *rr, *cc;
  double *d;
  Matrix::size_type n,m,nnz;

  if (im.get_rep() == 0)
  {
    error("ReverseCuthillmcKee: No input matrix was found");
    return false;
  }
  
  if (im->ncols() != im->nrows())
  {
    error("ReverseCuthillmcKee: Matrix is not square");
    return false;  
  }
  
  if (im->is_sparse() == false) 
  {
    error("CuthillmcKee: Matrix is not sparse");
    return false;
  }
  
  SparseRowMatrix* sim = im->as_sparse();
  
  om = sim->clone();
  if (om.get_rep() == 0)
  {
    error("CuthillmcKee: Could not copy sparse matrix");
    return false;  
  }
  
  m  = sim->nrows();
  n  = sim->ncols();
  nnz = sim->nnz;
  rr = sim->rows;
  cc = sim->columns;
  d  = sim->a;
  
  // reserve mapping space
  
  Matrix::index_type *mrr = 0, *mcc = 0;
  double *md = 0;

  Matrix::index_type *drr, *dcc;
  double *dd;

  if (calcmapping)
  {
    mrr = scinew Matrix::index_type[m+1];
    mcc = scinew Matrix::index_type[m];
    md  = scinew double[m];
    
    if ((mrr == 0)||(mcc == 0)||(md == 0))
    {
      if (mrr ) delete[] mrr;
      if (mcc ) delete[] mcc;
      if (md ) delete[] md;      

      error("CuthillmcKee: Could not reserve space for mapping matrix");    
      return false;
    }    
    
  }

  SparseRowMatrix* som = om->as_sparse();  
  drr = som->rows;
  dcc = som->columns;
  dd  = som->a;

  Matrix::index_type *Q, *R, *S, *X;
  Matrix::index_type *degree;
  Matrix::index_type ns,nr,nq,nx,nss;
  
  // Count number of connections
  degree = drr;
  for (Matrix::index_type p = 0;p<m;p++) degree[p] = (degree[p+1] - degree[p]);
  
  // clear the mask of already processed nodes

  X = scinew Matrix::index_type[m];
  Q = scinew Matrix::index_type[m];
  S = scinew Matrix::index_type[m];
  R = scinew Matrix::index_type[m];

  if ((X==0)||(Q==0)||(S==0)||(R==0))
  {
    if (X) delete[] X;
    if (Q) delete[] Q;
    if (S) delete[] S;
    if (R) delete[] R;
    error("CuthillmcKee: Could not allocate enough memory");
    return false;
  }
    
  for (Matrix::index_type p=0;p<m;p++) Q[p] = 0;
  
  Matrix::index_type root = 0;
  for (Matrix::index_type p=0;p<m;p++) if((degree[p] < degree[root])&&(Q[p] == 0)) root = p;

  nss = 0;
  nr = 0;
  ns = 0;
  nq = 0;
  nx = 0;
  
  R[nr++] = root;
  Q[root] = 1; nq++;
  X[nx++] = root;
  
  nr = 0;
  for (Matrix::index_type p = rr[root];p<rr[root+1];p++) if (Q[cc[p]] == 0) { R[nr++] = cc[p]; Q[cc[p]] = 1; nq++; }
 
  while(1)
  {
    if(nr > 0) 
    { 
      for (Matrix::index_type i =0;i<nr;i++)
      {
        Matrix::index_type t;
        Matrix::index_type k = i;
        // sort entries by order
        for (Matrix::index_type j=i+1;j<nr;j++) { if (degree[R[k]] > degree[R[j]]) { k = j;}
        if (k > i)  {t = R[i]; R[i] = R[k]; R[k] = t;}  }
      }
      for (Matrix::index_type j=0;j<nr;j++) { X[nx++] = R[j]; S[ns++] = R[j]; }
      nr  = 0;
    }
    if (nx == m) break;
    
    if (nss < ns)
    {
      root = S[nss++];
      nr = 0;
      for (Matrix::index_type p = rr[root];p<rr[root+1];p++) if (Q[cc[p]] == 0) { R[nr++] = cc[p]; Q[cc[p]] = 1; nq++; }
    }
    else
    {
      for (Matrix::index_type p=0;p<m;p++) if(Q[p] == 0) { root = p; break;}
      for (Matrix::index_type p=0;p<m;p++) if((degree[p] < degree[root])&&(Q[p] == 0)) root = p;
      Q[root] = 1; nq++;
      X[nx++] = root;
      nr = 0;
      for (Matrix::index_type p = rr[root];p<rr[root+1];p++) if (Q[cc[p]] == 0) { R[nr++] = cc[p]; Q[cc[p]] = 1; nq++; }
    }    
  }

  // Finish mapping matrix and inverse mapping matrix.
  if (calcmapping)
  {
    for (Matrix::index_type p=0;p<(m+1);p++) { mrr[p] = p; }
    for (Matrix::index_type p=0;p<m;p++) { md[p] = 1.0;  }
    for (Matrix::index_type p=0;p<m;p++) { mcc[X[p]] = p; }
    mapping = scinew SparseRowMatrix(m,m,mrr,mcc,m,md);

  }
    
   for (Matrix::index_type p=0;p<m;p++) { Q[X[p]] = p; }
  
  // finish the reordering new matrix
  Matrix::index_type i,j;
  drr[0] = 0;
  i = 0;
  for (Matrix::index_type p=0;p<m;p++)
  {
    j = Q[p];
    drr[p+1] = drr[p]+(rr[j+1]-rr[j]);
    for (Matrix::index_type r=rr[j];r<rr[j+1];r++)
    {
      dcc[i] = X[cc[r]];
      dd[i] = d[r];
      i++;
    }
  }

  delete[] X;
  delete[] Q;
  delete[] S;
  delete[] R;


  return true;
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
  
  output = scinew DenseMatrix(nrows, 1);
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


bool 
MathAlgo::MatrixSelectRows(MatrixHandle input, MatrixHandle& output, std::vector<Matrix::index_type> rows)
{
  SelectMatrixRowsAlgo algo;
  return(algo.SelectMatrixRows(pr_,input,output,rows));
}

bool 
MathAlgo::MatrixSelectColumns(MatrixHandle input, MatrixHandle& output, std::vector<Matrix::index_type> columns)
{
  SelectMatrixColumnsAlgo algo;
  return(algo.SelectMatrixColumns(pr_,input,output,columns));
}


bool 
MathAlgo::FindNonZeroMatrixRows(MatrixHandle input, std::vector<Matrix::index_type>& rows)
{
  FindMatrixRowsAlgo algo;
  return(algo.FindNonZeroMatrixRows(pr_,input, rows)); 
}

bool 
MathAlgo::FindZeroMatrixRows(MatrixHandle input, std::vector<Matrix::index_type>& rows)
{
  FindMatrixRowsAlgo algo;
  return(algo.FindZeroMatrixRows(pr_,input, rows));
}

bool 
MathAlgo::FindNonZeroMatrixColumns(MatrixHandle input, std::vector<Matrix::index_type>& columns)
{
  FindMatrixColumnsAlgo algo;
  return(algo.FindNonZeroMatrixColumns(pr_,input, columns));
}

bool 
MathAlgo::FindZeroMatrixColumns(MatrixHandle input, std::vector<Matrix::index_type>& columns)
{
  FindMatrixColumnsAlgo algo;
  return(algo.FindZeroMatrixColumns(pr_,input, columns));
}


bool 
MathAlgo::MatrixAppendRows(MatrixHandle input,MatrixHandle& output,MatrixHandle rows,std::vector<Matrix::index_type>& newrows)
{
  if (input.get_rep() == 0)
  {
    if (rows.get_rep() == 0)
    {
      output = 0;
      warning("MatrixAppendRows: Base matrix and matrix to append are empty");
      return (true);
    }
  
    output = rows;
    Matrix::size_type m = rows->nrows();
    newrows.clear();
    for (Matrix::index_type r=0; r<m; r++) newrows.push_back(r);
    return (true);
  }

  if (rows.get_rep() == 0)
  {
    output = input;
    newrows.clear();
    warning("MatrixAppendRows: Matrix to append is empty");
    return (true);
  } 
  
    
  Matrix::size_type m = input->nrows();
  Matrix::size_type n = input->ncols();
  
  Matrix::size_type am = rows->nrows();
  Matrix::size_type an = rows->ncols();
  
        
  if (an != n)
  {
    error("MatrixAppendRows: The number of columns in input matrix is not equal to number of columns in row matrix");
    return (false);
  }
  
  Matrix::index_type newm = m+am;
  Matrix::index_type newn = n;
  
  if (input->is_sparse())
  {
    MatrixHandle a = dynamic_cast<Matrix *>(rows->sparse());
    if (a.get_rep() == 0)
    {
      error("MatrixAppendRows: Could not convert matrix to sparse matrix");
      return (false);      
    }
  
    Matrix::index_type *rr = input->get_row();
    Matrix::index_type *cc = input->get_col();
    double *vv = input->get_val();

	  input->sparse()->validate();
	  a->sparse()->validate();
	
    Matrix::index_type *arr = a->get_row();
    Matrix::index_type *acc = a->get_col();
    double *avv = a->get_val();
 
    if (rr==0 || cc==0 || vv==0 || arr==0 || acc == 0 || avv == 0)
    {
      error("MatrixAppendRows: Sparse matrix is invalid");
      return (false);      
    }   
  
    Matrix::size_type newnnz = rr[m]+arr[am];
    Matrix::index_type *nrr = scinew Matrix::index_type[newm+1];
    Matrix::index_type *ncc = scinew Matrix::index_type[newnnz];
    double *nvv = scinew double[newnnz];
    
    if (nrr==0 || ncc==0 || nvv==0)
    {
      if (nrr) delete[] nrr;
      if (ncc) delete[] ncc;
      if (nvv) delete[] nvv;
      error("MatrixAppendRows: Could not allocate output matrix");
      return (false);      
    }

    for (Matrix::index_type r=0;r<m;r++) nrr[r] = rr[r];
    for (Matrix::index_type r=0;r<am;r++) nrr[m+r] = arr[r]+rr[m];
    nrr[newm] = newnnz;
    
    Matrix::size_type nnz = rr[m];
    for (Matrix::index_type r=0;r<nnz;r++) { ncc[r] = cc[r]; nvv[r] = vv[r]; }
    Matrix::size_type annz = arr[am];
    for (Matrix::index_type r=0;r<annz;r++) { ncc[r+nnz] = acc[r]; nvv[r+nnz] = avv[r]; } 
    
    newrows.clear();
    for (Matrix::index_type r=m;r<newm;r++) newrows.push_back(r);
    
		output = dynamic_cast<Matrix*>(scinew SparseRowMatrix(newm,newn,nrr,ncc,newnnz,nvv));
    if (output.get_rep() == 0)
    {
      error("MatrixSelectRows: Could not allocate output matrix");
      return (false);          
    }
    
    return (true);
  }
  else
  {
    MatrixHandle i = dynamic_cast<Matrix *>(input->dense());
    MatrixHandle a = dynamic_cast<Matrix *>(rows->dense());
    if (a.get_rep() == 0 || i.get_rep() == 0)
    {
      error("MatrixAppendRows: Could not convert matrix to dense matrix");
      return (false);      
    }    
  
    output = dynamic_cast<Matrix *>(scinew DenseMatrix(newm,newn));
    
    double *iptr = i->get_data_pointer();
    double *aptr = a->get_data_pointer();
    double *optr = output->get_data_pointer();
    
    if (aptr == 0 || iptr == 0 ||optr == 0)
    {
      error("MatrixAppendRows: Could not convert matrix to dense matrix");
      return (false);      
    }    
    
    Matrix::index_type mn = m*n;
    for (Matrix::index_type r=0; r<mn;r++) optr[r] = iptr[r];
    optr += mn;
    mn = am*an;
    for (Matrix::index_type r=0; r<mn;r++) optr[r] = aptr[r]; 
  
    newrows.clear();
    for (Matrix::index_type r=m;r<newm;r++) newrows.push_back(r);
    
    return (true);
  }
}


bool 
MathAlgo::MatrixAppendColumns(MatrixHandle input,MatrixHandle& output,MatrixHandle columns,std::vector<Matrix::index_type>& newcolumns)
{
  if (input.get_rep() == 0)
  {
    if (columns.get_rep() == 0)
    {
      output = 0;
      warning("MatrixAppendColumns: Base matrix and matrix to append are empty");
      return (true);
    }
  
    output = columns;
    Matrix::size_type n = columns->ncols();
    newcolumns.clear();
    for (Matrix::index_type r=0; r<n; r++) newcolumns.push_back(r);
    return (true);
  }


  if (columns.get_rep() == 0)
  {
    output = input;
    newcolumns.clear();
    return (true);
  }  
  
  Matrix::size_type m = input->nrows();
  Matrix::size_type n = input->ncols();
  
  Matrix::size_type am = columns->nrows();
  Matrix::size_type an = columns->ncols();
  
  if (am != m)
  {
    error("MatrixAppendColumns: The number of rows in input matrix is not equal to number of rows in column matrix");
    return (false);
  }
  
  Matrix::size_type newm = m;
  Matrix::size_type newn = n+an;
  
  if (input->is_sparse())
  {
    MatrixHandle a = dynamic_cast<Matrix *>(columns->sparse());
    if (a.get_rep() == 0)
    {
      error("MatrixAppendColumns: Could not convert matrix to sparse matrix");
      return (false);      
    }
  
    Matrix::index_type *rr = input->get_row();
    Matrix::index_type *cc = input->get_col();
    double *vv = input->get_val();
  
    Matrix::index_type *arr = a->get_row();
    Matrix::index_type *acc = a->get_col();
    double *avv = a->get_val();
 
    if (rr==0 || cc==0 || vv==0 || arr==0 || acc == 0 || avv == 0)
    {
      error("MatrixAppendColumns: Sparse matrix is invalid");
      return (false);      
    }   
  
    Matrix::size_type newnnz = rr[m]+arr[am];
    Matrix::index_type *nrr = scinew Matrix::index_type[newm];
    Matrix::index_type *ncc = scinew Matrix::index_type[newnnz];
    double *nvv = scinew double[newnnz];
    
    if (nrr==0 || ncc==0 || nvv==0)
    {
      if (nrr) delete[] nrr;
      if (ncc) delete[] ncc;
      if (nvv) delete[] nvv;
      error("MatrixAppendColumns: Could not allocate output matrix");
      return (false);      
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
    
    newcolumns.clear();
    for (Matrix::index_type r=n;r<newn;r++) newcolumns.push_back(r);

    output = dynamic_cast<Matrix*>(scinew SparseRowMatrix(newm,newn,nrr,ncc,newnnz,nvv));
    if (output.get_rep() == 0)
    {
      error("MatrixSelectColumns: Could not allocate output matrix");
      return (false);          
    }
		    
    return (true);
  }
  else
  {
    MatrixHandle i = dynamic_cast<Matrix *>(input->dense());
    MatrixHandle a = dynamic_cast<Matrix *>(columns->dense());
    if (a.get_rep() == 0 || i.get_rep() == 0)
    {
      error("MatrixAppendColumns: Could not convert matrix to dense matrix");
      return (false);      
    }    
  
    output = dynamic_cast<Matrix *>(scinew DenseMatrix(newm,newn));
    
    double *iptr = i->get_data_pointer();
    double *aptr = a->get_data_pointer();
    double *optr = output->get_data_pointer();
    
    if (aptr == 0 || iptr == 0 ||optr == 0)
    {
      error("MatrixAppendColumns: Could not convert matrix to dense matrix");
      return (false);      
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
    
    newcolumns.clear();
    for (Matrix::index_type r=n;r<newn;r++) newcolumns.push_back(r);
    
    return (true);
  }
}


bool 
MathAlgo::SelectSubMatrix(MatrixHandle input, MatrixHandle& output, std::vector<Matrix::index_type> rows, std::vector<Matrix::index_type> columns)
{
  if (input.get_rep() == 0)
  {
    error("SelectSubMatrix: No input matrix");
    return (false);
  }

  if (rows.size() == 0)
  {
    error("SelectSubMatrix: No row indices given");
    return (false);  
  }

  if (columns.size() == 0)
  {
    error("SelectSubMatrix: No column indices given");
    return (false);  
  }

  Matrix::size_type m = input->nrows();
  Matrix::size_type n = input->ncols();
    
  for (size_t r=0; r<rows.size(); r++)
  {
    if (rows[r] >= static_cast<Matrix::index_type>(m))
    {
      error("SelectSubMatrix: selected row exceeds matrix dimensions");
      return (false);
    }
  }

  for (size_t r=0; r<columns.size(); r++)
  {
    if (columns[r] >= static_cast<Matrix::index_type>(n))
    {
      error("SelectSubMatrix: selected column exceeds matrix dimensions");
      return (false);
    }
  }


  if (input->is_sparse())
  {
    Matrix::index_type *rr = input->get_row();
    Matrix::index_type *cc = input->get_col();
    double *vv = input->get_val();
    
    if (rr==0 || cc==0 || vv == 0)
    {
      error("SelectSubMatrix: Sparse matrix is invalid");
      return (false);      
    }

    std::vector<Matrix::index_type> s(n,n);
    for (Matrix::index_type r=0;
              r< static_cast<Matrix::index_type>(columns.size()); r++) 
      s[columns[r]] = r;
  
    Matrix::index_type k =0;
    for (Matrix::index_type r=0; r<static_cast<Matrix::index_type>(rows.size()); r++)
    {
      for (Matrix::index_type q=rr[rows[r]]; q<rr[rows[r]+1]; q++)
      {
        if (s[cc[q]] < n) k++;
      }
    }
    
    Matrix::index_type *nrr = scinew Matrix::index_type[rows.size()+1];
    Matrix::index_type *ncc = scinew Matrix::index_type[k];
    double *nvv = scinew double[k];
    
    if (nrr==0 || ncc==0 || nvv==0)
    {
      if (nrr) delete[] nrr;
      if (ncc) delete[] ncc;
      if (nvv) delete[] nvv;
      error("MatrixSelectRows: Could not allocate output matrix");
      return (false);      
    }


    k =0;
    for (Matrix::index_type r=0; r<static_cast<Matrix::index_type>(rows.size()); r++)
    {
      nrr[r] = k;
      for (Matrix::index_type q=rr[rows[r]]; q<rr[rows[r]+1]; q++)
      {
        if (s[cc[q]] < n) 
        {
          ncc[k] = s[cc[q]];
          nvv[k] = vv[q];
          k++;
        }
      }
    }
    nrr[rows.size()] = k;

    output = dynamic_cast<Matrix*>(scinew SparseRowMatrix(rows.size(),columns.size(),nrr,ncc,k,nvv));
    if (output.get_rep() == 0)
    {
      error("MatrixSelectRows: Could not allocate output matrix");
      return (false);          
    }
    
    return (true);
  }
  else
  {
    MatrixHandle mat = input->dense();
    
    if (mat.get_rep() == 0)
    {
      error("MatrixSelectRows: Could not convert matrix into dense matrix");
      return (false);    
    }
    
    
    output = dynamic_cast<Matrix *>(scinew DenseMatrix(rows.size(),columns.size()));
    if (output.get_rep() == 0)
    {
      error("MatrixSelectRows: Could not allocate output matrix");
      return (false);
    }
    
    double* src = mat->get_data_pointer();
    double* dst = output->get_data_pointer(); 
    
    if (dst==0 || src == 0)
    {
      error("MatrixSelectRows: Could not allocate output matrix");
      return (false);
    }
    
    Matrix::size_type nr = static_cast<Matrix::size_type>(rows.size());
    Matrix::size_type nc = static_cast<Matrix::size_type>(columns.size());
    
    for (Matrix::index_type p=0;p<nr;p++)
    {
      for (Matrix::index_type q=0;q<nc;q++)
      {
        dst[p*nc + q] = src[rows[p]*n +columns[q]];
      }
    }
    return (true);
  }
}

    
bool 
MathAlgo::SelectMatrixRows(MatrixHandle input, MatrixHandle& output, std::vector<Matrix::index_type> rows)
{
  SelectMatrixRowsAlgo algo;
  return(algo.SelectMatrixRows(pr_,input,output,rows));
}

bool 
MathAlgo::SelectMatrixRows(MatrixHandle input, MatrixHandle& output, MatrixHandle rows)
{
  SelectMatrixRowsAlgo algo;
  return(algo.SelectMatrixRows(pr_,input,output,rows));
}

bool 
MathAlgo::SelectMatrixColumns(MatrixHandle input, MatrixHandle& output, std::vector<Matrix::index_type> columns)
{
  SelectMatrixColumnsAlgo algo;
  return(algo.SelectMatrixColumns(pr_,input,output,columns));
}
    
bool 
MathAlgo::SelectMatrixColumns(MatrixHandle input, MatrixHandle& output, MatrixHandle columns)
{
  SelectMatrixColumnsAlgo algo;
  return(algo.SelectMatrixColumns(pr_,input,output,columns));
}
    
bool 
MathAlgo::GetSortOrderMatrixRows(MatrixHandle input, std::vector<Matrix::index_type>& order, bool asc)
{
  SortMatrixRowsAlgo algo;
  return(algo.GetSortOrderMatrixRows(pr_,input,order,asc));
}

bool 
MathAlgo::GetSortOrderMatrixRows(MatrixHandle input, MatrixHandle& order, bool asc)
{
  SortMatrixRowsAlgo algo;
  return(algo.GetSortOrderMatrixRows(pr_,input,order,asc));
}
    
bool 
MathAlgo::SortMatrixRows(MatrixHandle input, MatrixHandle& output, bool asc)
{
  SortMatrixRowsAlgo algo;
  return(algo.SortMatrixRows(pr_,input,output,asc));
}

bool 
MathAlgo::GetSortOrderMatrixColumns(MatrixHandle input, std::vector<Matrix::index_type>& order, bool asc)
{
  SortMatrixColumnsAlgo algo;
  return(algo.GetSortOrderMatrixColumns(pr_,input,order,asc));
}

bool 
MathAlgo::GetSortOrderMatrixColumns(MatrixHandle input, MatrixHandle& order, bool asc)
{
  SortMatrixColumnsAlgo algo;
  return(algo.GetSortOrderMatrixColumns(pr_,input,order,asc));
}
    
bool 
MathAlgo::SortMatrixColumns(MatrixHandle input, MatrixHandle& output, bool asc)
{
  SortMatrixColumnsAlgo algo;
  return(algo.SortMatrixColumns(pr_,input,output,asc));
}

bool 
MathAlgo::ConvertMappingMatrixIntoMappingOrder(MatrixHandle input, std::vector<Matrix::index_type>& output)
{
  return(ConvertMappingMatrixAlgo::ConvertMappingMatrixIntoMappingOrder(pr_,input,output));
}

bool 
MathAlgo::ConvertMappingMatrixIntoMappingOrder(MatrixHandle input, MatrixHandle& output)
{
  return(ConvertMappingMatrixAlgo::ConvertMappingMatrixIntoMappingOrder(pr_,input,output));
}

bool 
MathAlgo::ConvertMappingOrderIntoMappingMatrix(MatrixHandle input, MatrixHandle& output, Matrix::size_type ncols)
{
  return(ConvertMappingMatrixAlgo::ConvertMappingOrderIntoMappingMatrix(pr_,input,output, ncols));
}

bool 
MathAlgo::ConvertMappingOrderIntoMappingMatrix(std::vector<Matrix::index_type>& input, MatrixHandle& output, Matrix::size_type ncols)
{
  return(ConvertMappingMatrixAlgo::ConvertMappingOrderIntoMappingMatrix(pr_,input,output, ncols));
}

bool 
MathAlgo::ConvertMappingOrderIntoTransposeMappingMatrix(MatrixHandle input, MatrixHandle& output, Matrix::size_type nrows)
{
  return(ConvertMappingMatrixAlgo::ConvertMappingOrderIntoTransposeMappingMatrix(pr_,input,output, nrows));
}

bool 
MathAlgo::ConvertMappingOrderIntoTransposeMappingMatrix(std::vector<Matrix::index_type>& input, MatrixHandle& output, Matrix::size_type nrows)
{
  return(ConvertMappingMatrixAlgo::ConvertMappingOrderIntoTransposeMappingMatrix(pr_,input,output, nrows));
}


} // end SCIRun namespace
