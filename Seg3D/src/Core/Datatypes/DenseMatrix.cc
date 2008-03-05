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


#include <stdio.h>

#include <sci_defs/lapack_defs.h>
#include <sci_defs/blas_defs.h>

#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColMajMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Util/Assert.h>
#include <Core/Malloc/Allocator.h>
#include <Core/Math/MiscMath.h>
#include <Core/Util/Assert.h>
#include <Core/Exceptions/FileNotFound.h>
#include <iostream>
#include <vector>
#include <float.h>

#if defined(HAVE_LAPACK)
#include <Core/Math/sci_lapack.h>
#endif

#if defined(HAVE_CBLAS)
#if defined(__APPLE__)
#include <vecLib/cblas.h>
#else
extern "C"{
#include <cblas.h>
}
#endif
#endif

using std::cout;
using std::endl;
using std::vector;

namespace SCIRun {

static Persistent* maker()
{
  return scinew DenseMatrix;
}

PersistentTypeID DenseMatrix::type_id("DenseMatrix", "Matrix", maker);

DenseMatrix*
DenseMatrix::clone()
{
  return scinew DenseMatrix(*this);
}


//! constructors
DenseMatrix::DenseMatrix() :
  data(0),
  dataptr_(0)
{
}


DenseMatrix::DenseMatrix(size_type r, size_type c) :
  Matrix(r, c)
{
  data = scinew double*[nrows_];
  double* tmp = scinew double[nrows_ * ncols_];
  dataptr_ = tmp;
  for (index_type i=0; i<nrows_; i++)
  {
    data[i] = tmp;
    tmp += ncols_;
  }
}


DenseMatrix::DenseMatrix(const DenseMatrix& m) :
  Matrix(m.nrows_, m.ncols_)
{
  data = scinew double*[nrows_];
  double* tmp = scinew double[nrows_ * ncols_];
  dataptr_ = tmp;
  for (index_type i=0; i<static_cast<index_type>(nrows_); i++)
  {
    data[i] = tmp;
    double* p = m.data[i];
    for (int j=0; j<ncols_; j++)
    {
      *tmp++ = *p++;
    }
  }
}


DenseMatrix::DenseMatrix(const Transform& t) :
  Matrix(4, 4)
{
  double dummy[16];
  t.get(dummy);
  data = scinew double*[nrows_];
  double* tmp = scinew double[nrows_ * ncols_];
  dataptr_ = tmp;
  double* p=dummy;
  
  for (index_type i=0; i<nrows_; i++)
  {
    data[i] = tmp;
    for (index_type j=0; j<ncols_; j++)
    {
      *tmp++ = static_cast<double>(*p++);
    }
  }
}


DenseMatrix *
DenseMatrix::dense()
{
  return this;
}


DenseColMajMatrix *
DenseMatrix::dense_col_maj()
{
  DenseColMajMatrix *dm = scinew DenseColMajMatrix(nrows_, ncols_);
  if (dm == 0) return (0);
  
  index_type m1 = static_cast<index_type>(ncols_);
  index_type m2 = static_cast<index_type>(nrows_);
  
  double *v1 = dataptr_;
  double *v2 = dm->get_data_pointer();
  for (index_type i = 0; i < nrows_; i++)
  {
    for (index_type j = 0; j < ncols_; j++)
    {
      v2[i+j*m2] = v1[i*m1+j];
    }
  }
  return dm;
}


ColumnMatrix *
DenseMatrix::column()
{
  ColumnMatrix *cm = scinew ColumnMatrix(nrows_);
  for (index_type i=0; i<static_cast<index_type>(nrows_); i++)
    (*cm)[i] = data[i][0];
  return cm;
}


SparseRowMatrix *
DenseMatrix::sparse()
{
  size_type nnz = 0;
  index_type r, c;
  index_type *rows = scinew index_type[nrows_ + 1];
  for (r=0; r<static_cast<index_type>(nrows_); r++)
    for (c=0; c<static_cast<index_type>(ncols_); c++)
      if (data[r][c] != 0) nnz++;

  index_type *columns = scinew index_type[nnz];
  double *a = scinew double[nnz];

  index_type count = 0;
  for (r=0; r<nrows_; r++)
  {
    rows[r] = count;
    for (c=0; c<ncols_; c++)
      if (data[r][c] != 0)
      {
        columns[count]=c;
        a[count]=data[r][c];
        count++;
      }
  }
  rows[nrows_] = count;

  return scinew SparseRowMatrix(nrows_, ncols_, rows, columns, nnz, a);
}


double *
DenseMatrix::get_data_pointer() const
{
  return dataptr_;
}


Matrix::size_type
DenseMatrix::get_data_size() const
{
  return nrows() * ncols();
}


//! destructor
DenseMatrix::~DenseMatrix()
{
  if (dataptr_) { delete[] dataptr_; }
  if (data) { delete[] data; }
}


//! assignment operator
DenseMatrix&
DenseMatrix::operator=(const DenseMatrix& m)
{
  if (dataptr_) { delete[] dataptr_; }
  if (data) { delete[] data; }
  nrows_ = m.nrows_;
  ncols_ = m.ncols_;
  data = scinew double*[nrows_];
  double* tmp = scinew double[nrows_ * ncols_];
  dataptr_ = tmp;
  for (index_type i=0; i<nrows_; i++)
  {
    data[i]=tmp;
    double* p=m.data[i];
    for (index_type j=0; j<ncols_; j++)
    {
      *tmp++=*p++;
    }
  }
  return *this;
}


double
DenseMatrix::get(index_type r, index_type c) const
{
  ASSERTRANGE(r, 0, nrows_);
  ASSERTRANGE(c, 0, ncols_);
  return data[r][c];
}


void
DenseMatrix::put(index_type r, index_type c, double d)
{
  ASSERTRANGE(r, 0, nrows_);
  ASSERTRANGE(c, 0, ncols_);
  data[r][c]=d;
}


void
DenseMatrix::add(index_type r, index_type c, double d)
{
  ASSERTRANGE(r, 0, nrows_);
  ASSERTRANGE(c, 0, ncols_);
  data[r][c] += d;
}


double
DenseMatrix::min()
{
  double min = DBL_MAX;
  for (index_type k=0; k<nrows_*ncols_; k++)
    if (dataptr_[k] < min) min = dataptr_[k];
  return (min);
}

double
DenseMatrix::max()
{
  double max = -DBL_MAX;
  for (index_type k=0; k<nrows_*ncols_; k++)
    if (dataptr_[k] > max) max = dataptr_[k];
  return (max);
}

DenseMatrix *
DenseMatrix::transpose() const
{
  DenseMatrix *m=scinew DenseMatrix(ncols_, nrows_);
  double *mptr = &((*m)[0][0]);
  for (index_type c=0; c<ncols_; c++)
    for (index_type r=0; r<nrows_; r++)
      *mptr++ = data[r][c];
  return m;
}


void
DenseMatrix::getRowNonzeros(size_type r, Array1<index_type>& idx, Array1<double>& val)
{
  idx.resize(ncols_);
  val.resize(ncols_);
  index_type i=0;
  for (index_type c=0; c<ncols_; c++)
  {
    if (data[r][c]!=0.0)
    {
      idx[i]=c;
      val[i]=data[r][c];
      i++;
    }
  }
}


void
DenseMatrix::getRowNonzerosNoCopy(index_type r, size_type &size, index_type &stride,
                                  index_type *&cols, double *&vals)
{
  size = ncols_;
  stride = 1;
  cols = NULL;
  vals = data[r];
}


void
DenseMatrix::zero()
{
  memset(dataptr_, 0, sizeof(double) * nrows_ * ncols_);
}


int
DenseMatrix::solve(ColumnMatrix& sol, int overwrite)
{
  ColumnMatrix b(sol);
  return solve(b, sol, overwrite);
}


int
DenseMatrix::solve(ColumnMatrix& rhs, ColumnMatrix& lhs, int overwrite)
{
  ASSERT(nrows_ == ncols_);
  ASSERT(rhs.nrows() == ncols_);
  lhs=rhs;

  double **A;
  double **cpy = NULL;
  double *lhsp = lhs.get_data_pointer();
	
  if (!overwrite) { cpy = scinew double*[nrows_]; 
  
  for (index_type j=0; j < nrows_; j++) cpy[j] = data[j]; A = cpy; } 
  else { A = data; }
	
  // Gauss-Jordan with partial pivoting
  index_type i;
  for (i=0; i<nrows_; i++)
  {
    double max=Abs(A[i][i]);
    index_type row=i;
    index_type j;
    for (j=i+1; j<nrows_; j++)
    {
      if(Abs(A[j][i]) > max)
      {
        max=Abs(A[j][i]);
        row=j;
      }
    }
    //  ASSERT(Abs(max) > 1.e-12);
    if (Abs(max) < 1.e-12)
    {
      lhs=rhs;
      if (!overwrite) delete cpy;
      return 0;
    }
    if(row != i)
    {
      // Switch rows (actually their pointers)
      double* tmp=A[i];
      A[i]=A[row];
      A[row]=tmp;
      double dtmp=lhsp[i];
      lhsp[i]=lhsp[row];
      lhsp[row]=dtmp;
    }
    double denom=1./A[i][i];
    double* r1=A[i];
    double s1=lhsp[i];
    for (j=i+1; j<nrows_; j++)
    {
      double factor=A[j][i]*denom;
      double* r2=A[j];
      for (int k=i; k<nrows_; k++)
        r2[k]-=factor*r1[k];
      lhsp[j]-=factor*s1;
    }
  }

  // Back-substitution
  for (i=1; i<nrows_; i++)
  {
    //  cout << "Solve: " << i << " of " << nr << endl;
    //  ASSERT(Abs(A[i][i]) > 1.e-12);
    if (Abs(A[i][i]) < 1.e-12)
    {
      lhs=rhs;
      if (!overwrite) delete cpy;
      return 0;
    }
    double denom=1./A[i][i];
    double* r1=A[i];
    double s1=lhsp[i];
    for (index_type j=0;j<i;j++)
    {
      double factor=A[j][i]*denom;
      double* r2=A[j];
      for (index_type k=i; k<nrows_; k++)
        r2[k] -= factor*r1[k];
      lhsp[j] -= factor*s1;
    }
  }

  // Normalize
  for (i=0; i<nrows_; i++)
  {
    //  cout << "Solve: " << i << " of " << nr << endl;
    //  ASSERT(Abs(A[i][i]) > 1.e-12);
    if (Abs(A[i][i]) < 1.e-12)
    {
      lhs=rhs;
      if (!overwrite) delete cpy;
      return 0;
    }
    double factor=1./A[i][i];
    for (index_type j=0; j<nrows_; j++)
      A[i][j] *= factor;
    lhsp[i] *= factor;
  }
  if (!overwrite) delete cpy;
  return 1;
}


int
DenseMatrix::solve(vector<double>& sol, int overwrite)
{
  vector<double> b(sol);
  return solve(b, sol, overwrite);
}


int
DenseMatrix::solve(const vector<double>& rhs, vector<double>& lhs,
                   int overwrite)
{
  ASSERT(nrows_ == ncols_);
  ASSERT(rhs.size()==(unsigned)ncols_);
  lhs=rhs;

  double **A;
  double **cpy = NULL;
  if (!overwrite) { cpy = scinew double*[nrows_]; 
  for (index_type j=0; j < nrows_; j++) cpy[j] = data[j]; A = cpy; } else { A = data; }

  // Gauss-Jordan with partial pivoting
  index_type i;
  for (i=0; i<nrows_; i++)
  {
    //  cout << "Solve: " << i << " of " << nr << endl;
    double max=Abs(A[i][i]);
    index_type row=i;
    index_type j;
    for (j=i+1; j<nrows_; j++)
    {
      if(Abs(A[j][i]) > max)
      {
        max=Abs(A[j][i]);
        row=j;
      }
    }
    //  ASSERT(Abs(max) > 1.e-12);
    if (Abs(max) < 1.e-12)
    {
      lhs=rhs;
      if (!overwrite) delete cpy;
      return 0;
    }
    if(row != i)
    {
      // Switch rows (actually their pointers)
      double* tmp=A[i];
      A[i]=A[row];
      A[row]=tmp;
      double dtmp=lhs[i];
      lhs[i]=lhs[row];
      lhs[row]=dtmp;
    }
    double denom=1./A[i][i];
    double* r1=A[i];
    double s1=lhs[i];
    for (j=i+1; j<nrows_; j++)
    {
      double factor=A[j][i]*denom;
      double* r2=A[j];
      for (index_type k=i; k<nrows_; k++)
        r2[k]-=factor*r1[k];
      lhs[j]-=factor*s1;
    }
  }

  // Back-substitution
  for (i=1; i<nrows_; i++)
  {
    //  cout << "Lhsve: " << i << " of " << nr << endl;
    //  ASSERT(Abs(A[i][i]) > 1.e-12);
    if (Abs(A[i][i]) < 1.e-12)
    {
      lhs=rhs;
      if (!overwrite) delete cpy;
      return 0;
    }
    double denom=1./A[i][i];
    double* r1=A[i];
    double s1=lhs[i];
    for (index_type j=0;j<i;j++)
    {
      double factor=A[j][i]*denom;
      double* r2=A[j];
      for (index_type k=i;k<nrows_;k++)
        r2[k]-=factor*r1[k];
      lhs[j]-=factor*s1;
    }
  }

  // Normalize
  for (i=0; i<nrows_; i++)
  {
    //  cout << "Solve: " << i << " of " << nr << endl;
    //  ASSERT(Abs(A[i][i]) > 1.e-12);
    if (Abs(A[i][i]) < 1.e-12)
    {
      lhs=rhs;
      if (!overwrite) delete cpy;
      return 0;
    }
    double factor=1./A[i][i];
    for (index_type j=0; j<nrows_; j++)
      A[i][j]*=factor;
    lhs[i]*=factor;
  }
  if (!overwrite) delete cpy;
  return 1;
}


void
DenseMatrix::mult(ColumnMatrix& x, ColumnMatrix& b) const
{
  ASSERTEQ(x.nrows(), ncols_);
  ASSERTEQ(b.nrows(), nrows_);

#if defined(HAVE_CBLAS)
  double ALPHA = 1.0;
  double BETA = 0.0;
  cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, nrows_,
              x.ncols(), ncols_, ALPHA, dataptr_, ncols_,
              x.get_data_pointer(), x.ncols(), BETA,
              b.get_data_pointer(), b.ncols());
#else
  index_type i, j;
  
  double* xdata = x.get_data_pointer();
  double* bdata = b.get_data_pointer();

  size_type m8 = (ncols_)/8;
  size_type m = (ncols_)-m8*8;

  for (i=0; i<nrows_; i++)
  {
    double sum=0;
    double* row = data[i];
    double* xd = xdata;
    
    for (j=0; j<m8; j++)
    {
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
    }

    for (j=0 ; j<m ; j++)
    {
      sum+=(*row)*(*xd);
      row++; xd++;
    }
    
    *bdata=sum;
    bdata++;
  }
#endif
}


void
DenseMatrix::mult(const ColumnMatrix& x, ColumnMatrix& b,
                  int& flops, int& memrefs, 
                  index_type beg, index_type end,
                  int spVec) const
{
  ASSERTEQ(x.nrows(), ncols_);
  ASSERTEQ(b.nrows(), nrows_);

  if (beg == -1) beg = 0;
  if (end == -1) end = nrows_;
  
#if defined(HAVE_CBLAS)
  double ALPHA = 1.0;
  double BETA = 0.0;
  cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, (end-beg),
              1, ncols_, ALPHA, dataptr_+(beg*ncols_), ncols_,
              x.get_data_pointer(), 1, BETA,
              b.get_data_pointer()+beg, 1);
  flops += (end-beg) * ncols_ * 2;
  memrefs += (end-beg) * ncols_ * 2 *sizeof(double)+(end-beg)*sizeof(double);

#else

  double* xdata = x.get_data_pointer();
  double* bdata = b.get_data_pointer();
  
  size_type m8 = (ncols_)/8;
  size_type m = (ncols_)-m8*8;

  index_type i, j;
  if(!spVec)
  {
    for (i=beg; i<end; i++)
    {
      double sum=0;
      double* row=data[i];
      double* xd=xdata;

      for (j=0; j<m8; j++)
      {
        sum+=(*row)*(*xd);
        row++; xd++;
        sum+=(*row)*(*xd);
        row++; xd++;
        sum+=(*row)*(*xd);
        row++; xd++;
        sum+=(*row)*(*xd);
        row++; xd++;
        sum+=(*row)*(*xd);
        row++; xd++;
        sum+=(*row)*(*xd);
        row++; xd++;
        sum+=(*row)*(*xd);
        row++; xd++;
        sum+=(*row)*(*xd);
        row++; xd++;
      }
      
      for (j=0; j<m; j++)
      {
        sum+=(*row)*(*xd);
        row++; xd++;
      }
      (*bdata)=sum;
      bdata++;
    }
  }
  else
  {
    for (i=beg; i<end; i++) b[i]=0;
    for (j=0; j<ncols_; j++)
    {
      if (x[j])
        for (i=beg; i<end; i++)
        {
          b[i]+=data[i][j]*x[j];
        }
    }
  }
  
  flops += (end-beg) * ncols_ * 2;
  memrefs += (end-beg) * ncols_ * 2 *sizeof(double)+(end-beg)*sizeof(double);
#endif
}

void
DenseMatrix::multiply(ColumnMatrix& x, ColumnMatrix& b) const
{
  index_type i, j;
  
  double* xdata = x.get_data_pointer();
  double* bdata = b.get_data_pointer();
  
  size_type m8 = (ncols_)/8;
  size_type m = (ncols_)-m8*8;
  
  for (i=0; i<nrows_; i++)
  {
    double sum=0;
    double* row = data[i];
    double* xd = xdata;
    for (j=0; j<m8; j++)
    {
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
      sum+=(*row)*(*xd);
      row++; xd++;
    }

    for (j=0; j<m; j++)
    {
      sum+=(*row)*(*xd);
      row++; xd++;
    }
    
    *bdata=sum;
    bdata++;
  }
}


void
DenseMatrix::mult_transpose(const ColumnMatrix& x, ColumnMatrix& b,
                            int& flops, int& memrefs, 
                            index_type beg, index_type end,
                            int spVec) const
{
  // Compute At*x=b
  ASSERT(x.nrows() == nrows_);
  ASSERT(b.nrows() == ncols_);
  if (beg == -1) beg = 0;
  if (end == -1) end = ncols_;
  index_type i, j;
  if (!spVec)
  {
    for (i=beg; i<end; i++)
    {
      double sum=0;
      for (j=0; j<nrows_; j++)
      {
        sum+=data[j][i]*x[j];
      }
      b[i]=sum;
    }
  }
  else
  {
    for (i=beg; i<end; i++) b[i]=0;
    for (j=0; j<nrows_; j++)
      if (x[j])
      {
        double *row=data[j];
        for (i=beg; i<end; i++)
          b[i]+=row[i]*x[j];
      }
  }
  flops+=(end-beg)*nrows_*2;
  memrefs+=(end-beg)*nrows_*2*sizeof(double)+(end-beg)*sizeof(double);
}


DenseMatrix *
DenseMatrix::identity(size_type size)
{
  DenseMatrix *result = scinew DenseMatrix(size, size);
  result->zero();
  
  double* d = result->get_data_pointer();
  size_type nrows = result->nrows();
  for (index_type i = 0; i < size; i++)
  {
    (*d) = 1.0;
    d += (nrows+1);
  }

  return result;
}


void
DenseMatrix::print() const
{
  std::cout << "Dense Matrix: " << nrows_ << " by " << ncols_ << std::endl;
  print(std::cout);
}


void
DenseMatrix::print(ostream& ostr) const
{
  for (index_type i=0; i<nrows_; i++)
  {
    for (index_type j=0; j<ncols_; j++)
    {
      ostr << data[i][j] << "\t";
    }
    ostr << endl;
  }
}


void
DenseMatrix::scalar_multiply(double s)
{
  double* d = dataptr_;
  size_type m = nrows_*ncols_;
  for (index_type i=0; i<m; i++)
  {
    (*d) *= s; d++;
  }
}


MatrixHandle
DenseMatrix::submatrix(index_type r1, index_type c1, index_type r2, index_type c2)
{
  ASSERTRANGE(r1, 0, r2+1);
  ASSERTRANGE(r2, r1, nrows_);
  ASSERTRANGE(c1, 0, c2+1);
  ASSERTRANGE(c2, c1, ncols_);
  
  DenseMatrix *mat = scinew DenseMatrix(r2 - r1 + 1, c2 - c1 + 1);
  for (index_type i=r1; i <= r2; i++)
  {
    memcpy(mat->data[i-r1], data[i] + c1, (c2 - c1 + 1) * sizeof(double));
  }
  return mat;
}


#define DENSEMATRIX_VERSION 4

void
DenseMatrix::io(Piostream& stream)
{

  int version=stream.begin_class("DenseMatrix", DENSEMATRIX_VERSION);
  // Do the base class first...
  Matrix::io(stream);

  if (version < 4)
  {
    int nrows = static_cast<int>(nrows_);
    int ncols = static_cast<int>(ncols_);
    stream.io(nrows);
    stream.io(ncols);
    nrows_ = static_cast<size_type>(nrows);
    ncols_ = static_cast<size_type>(ncols);
  }
  else
  {
    long long nrows = static_cast<long long>(nrows_);
    long long ncols = static_cast<long long>(ncols_);
    stream.io(nrows);
    stream.io(ncols);
    nrows_ = static_cast<size_type>(nrows);
    ncols_ = static_cast<size_type>(ncols);
  }
  
  if(stream.reading())
  {
    data=scinew double*[nrows_];
    double* tmp=scinew double[nrows_ * ncols_];
    dataptr_=tmp;
    for (index_type i=0; i<nrows_; i++)
    {
      data[i] = tmp;
      tmp += ncols_;
    }
  }
  stream.begin_cheap_delim();

  int split;
  if (stream.reading())
  {
    if (version > 2)
    {
      split = separate_raw_;
      Pio(stream, split);
      if (separate_raw_)
      {
        Pio(stream, raw_filename_);
        FILE *f=fopen(raw_filename_.c_str(), "r");
        if (f)
        {
          fread(data[0], sizeof(double), nrows_ * ncols_, f);
          fclose(f);
        }
        else
        {
          const string errmsg = "Error reading separated file '" +
            raw_filename_ + "'";
          cerr << errmsg << "\n";
          throw FileNotFound(errmsg, __FILE__, __LINE__);
        }
      }
    }
    else
    {
      separate_raw_ = false;
    }
    split = separate_raw_;
  }
  else
  {     // writing
    string filename = raw_filename_;
    split = separate_raw_;
    if (split)
    {
      if (filename == "")
      {
        if (stream.file_name.c_str())
        {
          size_t pos = stream.file_name.rfind('.');
          if (pos == string::npos) filename = stream.file_name + ".raw";
          else filename = stream.file_name.substr(0,pos) + ".raw";
        } 
        else 
        {
          split=0;
        }
      }
    }
    Pio(stream, split);
    if (split)
    {
      Pio(stream, filename);
      FILE *f = fopen(filename.c_str(), "w");
      fwrite(data[0], sizeof(double), nrows_ * ncols_, f);
      fclose(f);
    }
  }

  if (!split)
  {
    if (!stream.block_io(dataptr_, sizeof(double), (size_t)(nrows_ * ncols_)))
    {
      for (size_t i = 0; i < (size_t)(nrows_ * ncols_); i++)
      {
        stream.io(dataptr_[i]);
      }
    }
  }
  stream.end_cheap_delim();
  stream.end_class();
}


bool
DenseMatrix::invert()
{
  if (nrows_ != ncols_) return false;
#if defined(HAVE_LAPACK)
  lapackinvert(dataptr_, nrows_);
  return true;
#else

  double** newdata=scinew double*[nrows_];
  double* tmp=scinew double[nrows_ * ncols_];
  double* newdataptr_=tmp;

  index_type i;
  for (i=0; i<nrows_; i++)
  {
    newdata[i]=tmp;
    for (index_type j=0; j<nrows_; j++)
    {
      tmp[j]=0;
    }
    tmp[i] = 1;
    tmp += ncols_;
  }

  // Gauss-Jordan with partial pivoting
  for (i=0;i<nrows_;i++)
  {
    double max=Abs(data[i][i]);
    index_type row=i;
    index_type j;
    for (j=i+1; j<nrows_; j++)
    {
      if(Abs(data[j][i]) > max)
      {
        max=Abs(data[j][i]);
        row=j;
      }
    }
    if(row != i)
    {
      // Switch row pointers for original matrix ...
      double* tmp=data[i];
      data[i]=data[row];
      data[row]=tmp;
      // ... but switch actual data values for inverse
      for (j=0; j<ncols_; j++) {
	double ntmp=newdata[i][j];
	newdata[i][j]=newdata[row][j];
	newdata[row][j]=ntmp;
      }
    }
    double denom=1./data[i][i];
    double* r1=data[i];
    double* n1=newdata[i];
    for (j=i+1; j<nrows_; j++)
    {
      double factor=data[j][i]*denom;
      double* r2=data[j];
      double* n2=newdata[j];
      for (index_type k=0;k<nrows_;k++)
      {
        r2[k]-=factor*r1[k];
        n2[k]-=factor*n1[k];
      }
    }
  }

  // Back-substitution
  for (i=1; i<nrows_; i++)
  {
    double denom=1./data[i][i];
    double* r1=data[i];
    double* n1=newdata[i];
    for (index_type j=0;j<i;j++)
    {
      double factor=data[j][i]*denom;
      double* r2=data[j];
      double* n2=newdata[j];
      for (index_type k=0; k<nrows_; k++)
      {
        r2[k]-=factor*r1[k];
        n2[k]-=factor*n1[k];
      }
    }
  }

  // Normalize
  for (i=0;i<nrows_;i++)
  {
    double factor=1./data[i][i];
    for (index_type j=0;j<nrows_; j++)
    {
      data[i][j]*=factor;
      newdata[i][j]*=factor;
    }
  }

  delete[] dataptr_;
  delete[] data;
  dataptr_=newdataptr_;
  data=newdata;
  return true;
#endif
}


void
Mult(DenseMatrix& out, const DenseMatrix& m1, const DenseMatrix& m2)
{
  ASSERTEQ(m1.ncols(), m2.nrows());
  ASSERTEQ(out.nrows(), m1.nrows());
  ASSERTEQ(out.ncols(), m2.ncols());
#if defined(HAVE_CBLAS)
  double ALPHA = 1.0;
  double BETA = 0.0;
  cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, m1.nrows(),
              m2.ncols(), m1.ncols(), ALPHA, m1.dataptr_, m1.ncols(),
              m2.dataptr_, m2.ncols(), BETA,
              out.dataptr_, out.ncols());
#else
  Matrix::size_type nr = out.nrows();
  Matrix::size_type nc = out.ncols();
  Matrix::size_type ndot=m1.ncols();
  for (Matrix::index_type i=0;i<nr;i++)
  {
    const double* row = m1[i];
    for (Matrix::index_type j=0;j<nc;j++)
    {
      double d = 0.0;
      for (Matrix::index_type k=0;k<ndot;k++)
      {
        d += row[k] * m2[k][j];
      }
      out[i][j] = d;
    }
  }
#endif
}


void
Add(DenseMatrix& out, const DenseMatrix& m1, const DenseMatrix& m2)
{
  ASSERTEQ(m1.ncols(), m2.ncols());
  ASSERTEQ(out.ncols(), m2.ncols());
  ASSERTEQ(m1.nrows(), m2.nrows());
  ASSERTEQ(out.nrows(), m2.nrows());

  Matrix::size_type nr=out.nrows();
  Matrix::size_type nc=out.ncols();

  Matrix::size_type m8 = (nr*nc)/8;
  Matrix::size_type m = (nr*nc)-m8*8;
  
  double *p0 = out.get_data_pointer();
  double *p1 = m1.get_data_pointer();
  double *p2 = m2.get_data_pointer();
  
  Matrix::index_type i;
  for (i=0;i<m8;i++)
  {
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;
  }

  for (i=0;i<m;i++)
  {
    (*p0) = (*p1) + (*p2);
    p0++; p1++; p2++;  
  }
}


void
Sub(DenseMatrix& out, const DenseMatrix& m1, const DenseMatrix& m2)
{
  ASSERTEQ(m1.ncols(), m2.ncols());
  ASSERTEQ(out.ncols(), m2.ncols());
  ASSERTEQ(m1.nrows(), m2.nrows());
  ASSERTEQ(out.nrows(), m2.nrows());

  Matrix::size_type nr=out.nrows();
  Matrix::size_type nc=out.ncols();

  Matrix::size_type m8 = (nr*nc)/8;
  Matrix::size_type m = (nr*nc)-m8*8;
  
  double *p0 = out.get_data_pointer();
  double *p1 = m1.get_data_pointer();
  double *p2 = m2.get_data_pointer();
  
  Matrix::index_type i;
  for (i=0;i<m8;i++)
  {
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;
  }

  for (i=0;i<m;i++)
  {
    (*p0) = (*p1) - (*p2);
    p0++; p1++; p2++;  
  }

}


void
Add(DenseMatrix& out, double f1, const DenseMatrix& m1,
    double f2, const DenseMatrix& m2)
{
  ASSERTEQ(m1.ncols(), m2.ncols());
  ASSERTEQ(out.ncols(), m2.ncols());
  ASSERTEQ(m1.nrows(), m2.nrows());
  ASSERTEQ(out.nrows(), m2.nrows());


  Matrix::size_type nr=out.nrows();
  Matrix::size_type nc=out.ncols();

  Matrix::size_type m8 = (nr*nc)/8;
  Matrix::size_type m = (nr*nc)-m8*8;
  
  double *p0 = out.get_data_pointer();
  double *p1 = m1.get_data_pointer();
  double *p2 = m2.get_data_pointer();
  
  Matrix::index_type i;
  for (i=0;i<m8;i++)
  {
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;
  }

  for (i=0;i<m;i++)
  {
    (*p0) = f1*(*p1) + f2*(*p2);
    p0++; p1++; p2++;  
  }
}


void
Add(double f1, DenseMatrix& out, double f2, const DenseMatrix& m1)
{
  ASSERTEQ(out.ncols(), m1.ncols());
  ASSERTEQ(out.nrows(), m1.nrows());
  
  Matrix::size_type nr=out.nrows();
  Matrix::size_type nc=out.ncols();

  Matrix::size_type m8 = (nr*nc)/8;
  Matrix::size_type m = (nr*nc)-m8*8;
  
  double *p0 = out.get_data_pointer();
  double *p1 = m1.get_data_pointer();
  
  Matrix::index_type i;
  for (i=0;i<m8;i++)
  {
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
  }

  for (i=0;i<m;i++)
  {
    (*p0) = f1*(*p0) + f1*(*p1);
    p0++; p1++;
  }
}
  

void
Mult_trans_X(DenseMatrix& out, const DenseMatrix& m1, const DenseMatrix& m2)
{
  ASSERTEQ(m1.nrows(), m2.nrows());
  ASSERTEQ(out.nrows(), m1.ncols());
  ASSERTEQ(out.ncols(), m2.ncols());
  Matrix::size_type nr=out.nrows();
  Matrix::size_type nc=out.ncols();
  Matrix::size_type ndot=m1.nrows();
  for (Matrix::index_type i=0;i<nr;i++)
  {
    for (Matrix::index_type j=0;j<nc;j++)
    {
      double d = 0.0;
      for (Matrix::index_type k=0;k<ndot;k++)
      {
        d += m1[k][i] * m2[k][j];
      }
      out[i][j] = d;
    }
  }
}


void
Mult_X_trans(DenseMatrix& out, const DenseMatrix& m1, const DenseMatrix& m2)
{
  ASSERTEQ(m1.ncols(), m2.ncols());
  ASSERTEQ(out.nrows(), m1.nrows());
  ASSERTEQ(out.ncols(), m2.nrows());

  Matrix::size_type nr=out.nrows();
  Matrix::size_type nc=out.ncols();
  Matrix::size_type ndot=m1.ncols();

  for (Matrix::index_type i=0;i<nr;i++)
  {
    const double* row = m1[i];
    for (Matrix::index_type j=0;j<nc;j++)
    {
      double d = 0.0;
      for (Matrix::index_type k=0; k<ndot; k++)
      {
        d += row[k] * m2[j][k];
      }
      out[i][j]=d;
    }
  }
}


void
DenseMatrix::mult(double s)
{
  double *d = dataptr_;
  size_type m8 = (nrows_*ncols_)/8;
  size_type m = (nrows_*ncols_)-m8*8;

  index_type i;
  for ( i=0; i<m8; i++)
  {
    (*d) *= s; d++;
    (*d) *= s; d++;
    (*d) *= s; d++;
    (*d) *= s; d++;
    (*d) *= s; d++;
    (*d) *= s; d++;
    (*d) *= s; d++;
    (*d) *= s; d++;
  }
    
  for (i=0 ; i<m; i++)
  {
    (*d) *= s; d++;
  }
}


double
DenseMatrix::sumOfCol(size_type n)
{
  ASSERT(n<ncols_);
  ASSERT(n>=0);

  double sum = 0;
  
  for (index_type i=0; i<nrows_; i++)
  {
    sum+=data[i][n];
  }
  
  return sum;
}


double
DenseMatrix::sumOfRow(size_type n)
{
  ASSERT(n < nrows_);
  ASSERT(n >= 0);
  double* rp = data[n];
  double sum = 0;
  index_type i=0;
  while (i<ncols_) sum+=rp[i++];
  return sum;
}


double
DenseMatrix::determinant()
{
  double a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p;
  ASSERTMSG(((nrows_ == 4) && (ncols_ == 4)),
            "Number of Rows and Colums for Determinant must equal 4! (Code not completed)");
  a=data[0][0]; b=data[0][1]; c=data[0][2]; d=data[0][3];
  e=data[1][0]; f=data[1][1]; g=data[1][2]; h=data[1][3];
  i=data[2][0]; j=data[2][1]; k=data[2][2]; l=data[2][3];
  m=data[3][0]; n=data[3][1]; o=data[3][2]; p=data[3][3];

  double q=a*f*k*p - a*f*l*o - a*j*g*p + a*j*h*o + a*n*g*l - a*n*h*k
    - e*b*k*p + e*b*l*o + e*j*c*p - e*j*d*o - e*n*c*l + e*n*d*k
    + i*b*g*p - i*b*h*o - i*f*c*p + i*f*d*o + i*n*c*h - i*n*d*g
    - m*b*g*l + m*b*h*k + m*f*c*l - m*f*d*k - m*j*c*h + m*j*d*g;

  return q;
}


// Added by Saeed Babaeizadeh, Jan. 2006
void 
Concat_rows(DenseMatrix& out, const DenseMatrix& m1, const DenseMatrix& m2)
{
  Matrix::index_type r, c;
  ASSERTEQ(m1.ncols(), m2.ncols());
  for (r = 0; r <= m1.nrows()-1; r++) 
  {
    for (c = 0; c <= m1.ncols()-1; c++) 
    {
      out[r][c] = m1[r][c];
    }
  }
  for (r = m1.nrows(); r <= m1.nrows()+m2.nrows()-1; r++) 
  {
    for (c = 0; c <= m2.ncols()-1; c++) 
    {
      out[r][c] = m2[r-m1.nrows()][c];
    }
  }
  return;
}

// Added by Saeed Babaeizadeh, Jan. 2006
void 
Concat_cols(DenseMatrix& out, const DenseMatrix& m1, const DenseMatrix& m2)
{
  Matrix::index_type r, c;
  ASSERTEQ(m1.nrows(), m2.nrows());
  for (r = 0; r <= m1.nrows()-1; r++) 
  {
    for (c = 0; c <= m1.ncols()-1; c++) 
    {
      out[r][c] = m1[r][c];
    }
  }
  for (r = 0; r <= m2.nrows()-1; r++) 
  {
    for (c = m1.ncols(); c <= m1.ncols()+m2.ncols()-1; c++) 
    {
      out[r][c] = m2[r][c-m1.ncols()];
    }
  }
  return;
}

#if defined(HAVE_LAPACK)

void
DenseMatrix::svd(DenseMatrix& U, SparseRowMatrix& S, DenseMatrix& VT)
{

  ASSERTEQ(U.ncols(), U.nrows());
  ASSERTEQ(VT.ncols(), VT.nrows());
  ASSERTEQ(U.nrows(), nrows_);
  ASSERTEQ(VT.ncols(), ncols_);
  ASSERTEQ(S.nrows(), nrows_);
  ASSERTEQ(S.ncols(), ncols_);

  lapacksvd(data, nrows_, ncols_, S.a, U.data, VT.data);
}

void
DenseMatrix::eigenvalues(ColumnMatrix& R, ColumnMatrix& I)
{

  ASSERTEQ(ncols_, nrows_);
  ASSERTEQ(R.nrows(), I.nrows());
  ASSERTEQ(ncols_, R.nrows());

  double *Er = scinew double[nrows_];
  double *Ei = scinew double[nrows_];

  lapackeigen(data, nrows_, Er, Ei);

  for (Matrix::index_type i = 0; i<nrows_; i++)
  {
    R[i] = Er[i];
    I[i] = Ei[i];
  }
}


void
DenseMatrix::eigenvectors(ColumnMatrix& R, ColumnMatrix& I, DenseMatrix& Vecs)
{

  ASSERTEQ(ncols_, nrows_);
  ASSERTEQ(R.nrows(), I.nrows());
  ASSERTEQ(ncols_, R.nrows());

  double *Er = scinew double[nrows_];
  double *Ei = scinew double[nrows_];

  lapackeigen(data, nrows_, Er, Ei, Vecs.data);

  for (Matrix::index_type i = 0; i<nrows_; i++)
  {
    R[i] = Er[i];
    I[i] = Ei[i];
  }
}


#else

void
DenseMatrix::svd(DenseMatrix& U, SparseRowMatrix& S, DenseMatrix& VT)
{
  ASSERTFAIL("Build was not configured with LAPACK");
}

void
DenseMatrix::eigenvalues(ColumnMatrix& R, ColumnMatrix& I)
{
  ASSERTFAIL("Build was not configured with LAPACK");
}

void
DenseMatrix::eigenvectors(ColumnMatrix& R, ColumnMatrix& I, DenseMatrix& Vecs)
{
  ASSERTFAIL("Build was not configured with LAPACK");
}

#endif

} // End namespace SCIRun
