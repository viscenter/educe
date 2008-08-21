/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2004 Scientific Coutputputing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without linputitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whoutput the
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

#include <Core/Algorithms/Math/ReorderMatrix/CutHillMcKee.h>

#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool 
CutHillMcKeeAlgo::run(MatrixHandle  input, 
                      MatrixHandle& output,
                      MatrixHandle& mapping)
{
  algo_start("CutHillMcKee",false);
  
  bool calcmapping = get_bool("build_mapping");

  Matrix::index_type *rr, *cc;
  double *d;
  Matrix::size_type n,m,nnz;

  if (input.get_rep() == 0)
  {
    error("No input matrix was found");
    algo_end(); return (false);
  }
  
  if (input->ncols() != input->nrows())
  {
    error("Matrix is not square");
    algo_end(); return (false);  
  }
  
  if (input->is_sparse() == false) 
  {
    error("Matrix is not sparse");
    algo_end(); return (false);
  }
  
  SparseRowMatrix* sinput = input->as_sparse();
  
  output = sinput->clone();
  if (output.get_rep() == 0)
  {
    error("Could not copy sparse matrix");
    algo_end(); return (false);  
  }
  
  m  = sinput->nrows();
  n  = sinput->ncols();
  nnz = sinput->nnz;
  rr = sinput->rows;
  cc = sinput->columns;
  d  = sinput->a;
  
  // reserve mapping space
  
  Matrix::index_type *mrr = 0, *mcc = 0;
  double *md = 0;

  Matrix::index_type *drr, *dcc;
  double *dd;

  if (calcmapping)
  {
    mrr = new Matrix::index_type[m+1];
    mcc = new Matrix::index_type[m];
    md  = new double[m];
    
    if ((mrr == 0)||(mcc == 0)||(md == 0))
    {
      if (mrr ) delete[] mrr;
      if (mcc ) delete[] mcc;
      if (md ) delete[] md;      

      error("Could not reserve space for mapping matrix");    
      algo_end(); return (false);
    }    
  }

  SparseRowMatrix* soutput = output->as_sparse();  
  drr = soutput->rows;
  dcc = soutput->columns;
  dd  = soutput->a;

  Matrix::index_type *Q, *R, *S, *X;
  Matrix::index_type *degree;
  Matrix::index_type ns,nr,nq,nx,nss;
  
  // Count number of connections
  degree = drr;
  for (Matrix::index_type p = 0;p<m;p++) degree[p] = (degree[p+1] - degree[p]);
  
  // clear the mask of already processed nodes

  X = new Matrix::index_type[m];
  Q = new Matrix::index_type[m];
  S = new Matrix::index_type[m];
  R = new Matrix::index_type[m];

  if ((X==0)||(Q==0)||(S==0)||(R==0))
  {
    if (X) delete[] X;
    if (Q) delete[] Q;
    if (S) delete[] S;
    if (R) delete[] R;
    error("Could not allocate enough memory");
    algo_end(); return (false);
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
    mapping = new SparseRowMatrix(m,m,mrr,mcc,m,md);

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

  algo_end(); return (true);
}

} // end namespace SCIRunAlgo
