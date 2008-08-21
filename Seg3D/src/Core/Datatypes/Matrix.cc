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


/*
 *  Matrix.cc: Matrix definitions
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   July 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

#include <Core/Util/Assert.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColMajMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Math/MiscMath.h>


namespace SCIRun {

PersistentTypeID Matrix::type_id("Matrix", "PropertyManager", 0);

#define MATRIX_VERSION 3

Matrix::~Matrix()
{
}

void
Matrix::io(Piostream& stream)
{
  int version = stream.begin_class("Matrix", MATRIX_VERSION);
  if (version < 2) 
  {
    int tmpsym;
    stream.io(tmpsym);
  }
  if (version > 2) 
  {
    PropertyManager::io(stream);
  }
  stream.end_class();
}


void
Matrix::scalar_multiply(double s)
{
  double *ptr = get_data_pointer();
  const size_type sz = get_data_size();
  for (index_type i = 0; i < sz; i++)
  {
    ptr[i] *= s;
  }
}


Transform Matrix::toTransform() 
{
  Transform t;
  if (nrows() != 4 || ncols() != 4) 
  {
    cerr << "Error - can't make a transform from this matrix.\n";
    return t;
  }
  double dummy[16];
  int cnt=0;
  for (index_type i=0; i<4; i++) 
    for (index_type j=0; j<4; j++, cnt++)
      dummy[cnt] = get(i,j);
  t.set(dummy);
  return t;
}
  

void
Mult(ColumnMatrix& result, const Matrix& mat, const ColumnMatrix& v)
{
  mat.mult(v, result);
}

DenseMatrix *
Matrix::direct_inverse()
{
  if (nrows() != ncols()) return 0;
  DenseMatrix *A=dense();
  if (is_dense()) A=new DenseMatrix(*A);
  A->invert();
  return A;
}

DenseMatrix *
Matrix::iterative_inverse()
{
  if (nrows() != ncols()) return 0;
  size_type n=nrows();
  SparseRowMatrix* B(SparseRowMatrix::identity(n));
  DenseMatrix *D = B->dense();
  DenseMatrix *X = new DenseMatrix(n,n);
  bicg_solve(*D, *X);
  delete D;
  delete B;
  return X;
}

int 
Matrix::cg_solve(const ColumnMatrix& rhs, ColumnMatrix& lhs) const
{
  double err;
  int niter;
  return cg_solve(rhs, lhs, err, niter);
}

int 
Matrix::cg_solve(const DenseMatrix& rhs, DenseMatrix& lhs) const
{
  double err;
  int niter;
  return cg_solve(rhs, lhs, err, niter);
}

int 
Matrix::cg_solve(const DenseMatrix& rhs, DenseMatrix& lhs,
		 double &err, int &niter,
		 double max_error, int toomany, int useLhsAsGuess) const
{
  if (rhs.ncols() != lhs.ncols()) return 0;
  for (index_type i=0; i<rhs.ncols(); i++) 
  {
    ColumnMatrix rh(rhs.nrows()), lh(lhs.nrows());
    index_type j;
    for (j=0; j<rh.nrows(); j++)
      rh[j]=rhs[i][j];
    if (!cg_solve(rh, lh, err, niter, max_error, 
		  toomany, useLhsAsGuess)) return 0;
    for (j=0; j<rh.nrows(); j++)
      lhs[i][j]=lh[j];
  }
  return 1;
}

int
Matrix::cg_solve(const ColumnMatrix& rhs, ColumnMatrix& lhs,
                 double &err, int &niter,
                 double max_error, int toomany, int useLhsAsGuess) const
{
  size_type size=nrows();  
  niter=0;
  if (!useLhsAsGuess) lhs.zero();

  if(toomany == 0) toomany=100*size;
        
  ColumnMatrix diag(size), R(size), Z(size), P(size);

  index_type i;
  for(i=0;i<size;i++) {
    if (Abs(get(i,i)>0.000001)) diag[i]=1./get(i,i);
    else diag[i]=1;
  }
 
  mult(lhs, R);    
  Sub(R, rhs, R);
  mult(R, Z);

  double bnorm=rhs.vector_norm();
  err=R.vector_norm()/bnorm;

  if(err == 0) 
  {
    lhs=rhs;
    return 1;
  } else if (err>1000000) return 0;

  double bkden=0;
  while(niter < toomany)
  {
    if(err < max_error)
      return 1;

    niter++;
    
    // Simple Preconditioning...
    Mult(Z, R, diag);
    
    // Calculate coefficient bk and direction vectors p and pp
    double bknum=Dot(Z, R);

    if(niter==1)
    {
      Copy(P, Z);
    } 
    else 
    {
      double bk=bknum/bkden;
      ScMult_Add(P, bk, P, Z);
    }

    // Calculate coefficient ak, new iterate x and new residuals r and rr
    mult(P, Z);
    bkden=bknum;
    double akden=Dot(Z, P);
    
    double ak=bknum/akden;
    ScMult_Add(lhs, ak, P, lhs);
    ScMult_Add(R, -ak, Z, R);
    
    err=R.vector_norm()/bnorm;
    if (err>1000000) return 0;
  }
  return 0;
}

int 
Matrix::bicg_solve(const ColumnMatrix& rhs, ColumnMatrix& lhs) const
{
  double err;
  int niter;
  return bicg_solve(rhs, lhs, err, niter);
}

int 
Matrix::bicg_solve(const DenseMatrix& rhs, DenseMatrix& lhs) const
{
  double err;
  int niter;
  return bicg_solve(rhs, lhs, err, niter);
}

int 
Matrix::bicg_solve(const DenseMatrix& rhs, DenseMatrix& lhs,
                   double &err, int &niter,
                   double max_error, int toomany, int useLhsAsGuess) const
{
  if (rhs.ncols() != lhs.ncols()) return 0;
  for (index_type i=0; i<rhs.ncols(); i++) 
  {
    ColumnMatrix rh(rhs.nrows()), lh(lhs.nrows());
    index_type j;
    for (j=0; j<rh.nrows(); j++)
      rh[j]=rhs[i][j];
    if (!bicg_solve(rh, lh, err, niter, max_error, useLhsAsGuess)) return 0;
    for (j=0; j<rh.nrows(); j++)
      lhs[i][j]=lh[j];
  }
  return 1;
}

int
Matrix::bicg_solve(const ColumnMatrix& rhs, ColumnMatrix& lhs,
		   double &err, int &niter, 
		   double max_error, int toomany, int useLhsAsGuess) const
{
  size_type size=nrows();  
  niter=0;
  if (!useLhsAsGuess) lhs.zero();
  
  if(toomany == 0) toomany=100*size;

  ColumnMatrix diag(size), R(size), R1(size), Z(size), Z1(size), 
    P(size), P1(size);

  index_type i;
  for(i=0;i<size;i++) 
  {
    if (Abs(get(i,i)>0.000001)) diag[i]=1./get(i,i);
    else diag[i]=1;
  }
  

  mult(lhs, R );
  Sub(R, rhs, R );

  double bnorm=rhs.vector_norm();
  err=R.vector_norm()/bnorm;
    
  if(err == 0)
  {
    lhs=rhs;
    return 1;
  } 
  else 
  {
    if (err>1000000) return 0;
  }

  // BiCG
  Copy(R1, R);

  double bkden=0;
  while(niter < toomany)
  {
    if(err < max_error)
      return 1;

    niter++;

    // Simple Preconditioning...
    Mult(Z, R, diag);
    // BiCG
    Mult(Z1, R1, diag);
    
    // Calculate coefficient bk and direction vectors p and pp
    // BiCG - change R->R1
    double bknum=Dot(Z, R1);
    
    // BiCG
    if ( bknum == 0 ) 
    {
      return 1;
    }
    
    if(niter==1)
    {
      Copy(P, Z);
      // BiCG
      Copy(P1, Z1);
    } 
    else 
    {
      double bk=bknum/bkden;
      ScMult_Add(P, bk, P, Z);
      // BiCG
      ScMult_Add(P1, bk, P1, Z1);
    }

    // Calculate coefficient ak, new iterate x and new residuals r and rr
    mult(P, Z);
    bkden=bknum;

    // BiCG
    mult_transpose(P1, Z1);

    // BiCG = change P -> P1
    double akden=Dot(Z, P1);

    double ak=bknum/akden;
    ScMult_Add(lhs, ak, P, lhs);
    ScMult_Add(R, -ak, Z, R);
    // BiCG
    ScMult_Add(R1, -ak, Z1, R1);
    
    err=R.vector_norm()/bnorm;

    if (err>1000000) return 0;
  }
  return 0;
}


DenseMatrix *
Matrix::as_dense()
{
  return dynamic_cast<DenseMatrix *>(this);
}

SparseRowMatrix *
Matrix::as_sparse()
{
  return dynamic_cast<SparseRowMatrix *>(this);
}

ColumnMatrix *
Matrix::as_column()
{
  return dynamic_cast<ColumnMatrix *>(this);
}

DenseColMajMatrix *
Matrix::as_dense_col_maj()
{
  return dynamic_cast<DenseColMajMatrix *>(this);
}


} // End namespace SCIRun
