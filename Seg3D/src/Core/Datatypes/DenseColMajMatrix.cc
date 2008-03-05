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

#include <Core/Util/Assert.h>

#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseColMajMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>

#include <Core/Malloc/Allocator.h>
#include <Core/Math/MiscMath.h>
#include <Core/Exceptions/FileNotFound.h>

#include <iostream>
#include <vector>
#include <float.h>

using std::cout;
using std::endl;
using std::vector;

namespace SCIRun {

static Persistent* maker()
{
  return scinew DenseColMajMatrix;
}

PersistentTypeID DenseColMajMatrix::type_id("DenseColMajMatrix", "Matrix", maker);

DenseColMajMatrix*
DenseColMajMatrix::clone()
{
  return scinew DenseColMajMatrix(*this);
}


//! constructors
DenseColMajMatrix::DenseColMajMatrix() :
  dataptr_(0)
{
}


DenseColMajMatrix::DenseColMajMatrix(size_type r, size_type c) :
  Matrix(r, c)
{
  dataptr_ = scinew double[nrows_ * ncols_];
}


DenseColMajMatrix::DenseColMajMatrix(const DenseColMajMatrix& m) :
  Matrix(m.nrows_, m.ncols_)
{
  dataptr_ = scinew double[nrows_ * ncols_];
  memcpy(dataptr_, m.dataptr_, sizeof(double) * nrows_ * ncols_);
}


DenseMatrix *
DenseColMajMatrix::dense()
{
  DenseMatrix *m = scinew DenseMatrix(nrows_, ncols_);
  for (index_type i = 0; i < nrows_; i++)
    for (index_type j = 0; j < ncols_; j++)
      (*m)[i][j] = iget(i, j);
  return m;
}


ColumnMatrix *
DenseColMajMatrix::column()
{
  ColumnMatrix *cm = scinew ColumnMatrix(nrows_);
  for (index_type i=0; i<nrows_; i++)
    (*cm)[i] = iget(i, 0);
  return cm;
}


SparseRowMatrix *
DenseColMajMatrix::sparse()
{
  size_type nnz = 0;
  index_type r, c;
  index_type *rows = scinew index_type[nrows_ + 1];
  for (r=0; r<nrows_; r++)
    for (c=0; c<ncols_; c++)
      if (iget(r, c) != 0.0) nnz++;

  index_type *columns = scinew index_type[nnz];
  double *a = scinew double[nnz];

  index_type count = 0;
  for (r=0; r<nrows_; r++)
  {
    rows[r] = count;
    for (c=0; c<ncols_; c++)
      if (iget(r, c) != 0)
      {
        columns[count] = c;
        a[count] = iget(r, c);
        count++;
      }
  }
  rows[nrows_] = count;

  return scinew SparseRowMatrix(nrows_, ncols_, rows, columns, nnz, a);
}


DenseColMajMatrix *
DenseColMajMatrix::dense_col_maj()
{
  return this;
}


double *
DenseColMajMatrix::get_data_pointer() const
{
  return dataptr_;
}


size_type
DenseColMajMatrix::get_data_size() const
{
  return nrows() * ncols();
}


//! destructor
DenseColMajMatrix::~DenseColMajMatrix()
{
  if (dataptr_) { delete[] dataptr_; }
}


//! assignment operator
DenseColMajMatrix&
DenseColMajMatrix::operator=(const DenseColMajMatrix& m)
{
  if (dataptr_) { delete[] dataptr_; }

  nrows_ = m.nrows_;
  ncols_ = m.ncols_;
  dataptr_ = scinew double[nrows_ * ncols_];
  memcpy(dataptr_, m.dataptr_, sizeof(double) * nrows_ * ncols_);

  return *this;
}


double
DenseColMajMatrix::get(index_type r, index_type c) const
{
  ASSERTRANGE(r, 0, nrows_);
  ASSERTRANGE(c, 0, ncols_);
  return iget(r, c);
}


void
DenseColMajMatrix::put(index_type r, index_type c, double d)
{
  ASSERTRANGE(r, 0, nrows_);
  ASSERTRANGE(c, 0, ncols_);
  iget(r, c) = d;
}


void
DenseColMajMatrix::add(index_type r, index_type c, double d)
{
  ASSERTRANGE(r, 0, nrows_);
  ASSERTRANGE(c, 0, ncols_);
  iget(r, c) += d;
}

double
DenseColMajMatrix::min()
{
  double min = DBL_MAX;
  for (index_type k=0; k<nrows_*ncols_; k++)
    if (dataptr_[k] < min) min = dataptr_[k];
  return (min);
}

double
DenseColMajMatrix::max()
{
  double max = -DBL_MAX;
  for (index_type k=0; k<nrows_*ncols_; k++)
    if (dataptr_[k] > max) max = dataptr_[k];
  return (max);
}



DenseColMajMatrix *
DenseColMajMatrix::transpose() const
{
  DenseColMajMatrix *m = scinew DenseColMajMatrix(ncols_, nrows_);
  for (index_type c=0; c<ncols_; c++)
    for (index_type r=0; r<nrows_; r++)
      m->iget(c, r) = iget(r, c);
  return m;
}


void
DenseColMajMatrix::getRowNonzerosNoCopy(index_type r, size_type &size,
                                        index_type &stride,
                                        index_type *&cols, double *&vals)
{
  size = ncols_;
  stride = nrows_;
  cols = NULL;
  vals = dataptr_ + r;
}


void
DenseColMajMatrix::zero()
{
  memset(dataptr_, 0, sizeof(double) * nrows_ * ncols_);
}


DenseColMajMatrix *
DenseColMajMatrix::identity(size_type size)
{
  DenseColMajMatrix *result = scinew DenseColMajMatrix(size, size);
  result->zero();
  for (index_type i = 0; i < size; i++)
  {
    result->iget(i, i) = 1.0;
  }

  return result;
}


void
DenseColMajMatrix::print() const
{
  std::cout << "DenseColMaj Matrix: " << nrows_ << " by " << ncols_ << std::endl;
  print(std::cout);
}


void
DenseColMajMatrix::print(ostream& ostr) const
{
  for (index_type i=0; i<nrows_; i++)
  {
    for (index_type j=0; j<ncols_; j++)
    {
      ostr << iget(i, j) << "\t";
    }
    ostr << endl;
  }
}


MatrixHandle
DenseColMajMatrix::submatrix(index_type r1, index_type c1, 
                             index_type r2, index_type c2)
{
  ASSERTRANGE(r1, 0, r2+1);
  ASSERTRANGE(r2, r1, nrows_);
  ASSERTRANGE(c1, 0, c2+1);
  ASSERTRANGE(c2, c1, ncols_);
  DenseColMajMatrix *mat = scinew DenseColMajMatrix(r2 - r1 + 1, c2 - c1 + 1);
  for (index_type i = c1; i <= c2; i++)
  {
    // TODO: Test this.
    memcpy(mat->dataptr_ + (i - c1) * (r2 - r1 + 1),
           dataptr_ + c1 * nrows_ + r1,
           (r2 - r1 + 1) * sizeof(double));
  }
  return mat;
}


void
DenseColMajMatrix::mult(const ColumnMatrix& x, ColumnMatrix& b,
                        int& flops, int& memrefs, 
                        index_type beg, index_type end,
                        int spVec) const
{
  // Compute A*x=b
  ASSERTEQ(x.nrows(), ncols_);
  ASSERTEQ(b.nrows(), nrows_);
  if (beg == -1) beg = 0;
  if (end == -1) end = nrows_;
  index_type i, j;
  if (!spVec)
  {
    for (i=beg; i<end; i++)
    {
      double sum = 0.0;
      for (j=0; j<ncols_; j++)
      {
        sum += iget(i, j) * x[j];
      }
      b[i] = sum;
    }
  }
  else
  {
    for (i=beg; i<end; i++) b[i] = 0.0;
    for (j=0; j<ncols_; j++)
    {
      if (x[j])
      {
        for (i=beg; i<end; i++)
        {
          b[i] += iget(i, j) * x[j];
        }
      }
    }
  }
  flops += (end-beg) * ncols_ * 2;
  memrefs += (end-beg) * ncols_ * 2 *sizeof(double)+(end-beg)*sizeof(double);
}


void
DenseColMajMatrix::mult_transpose(const ColumnMatrix& x, ColumnMatrix& b,
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
      double sum = 0.0;
      for (j=0; j<nrows_; j++)
      {
        sum += iget(j, i) * x[j];
      }
      b[i] = sum;
    }
  }
  else
  {
    for (i=beg; i<end; i++) b[i] = 0.0;
    for (j=0; j<nrows_; j++)
    {
      if (x[j])
      {
        for (i=beg; i<end; i++)
        {
          b[i] += iget(j, i) * x[j];
        }
      }
    }
  }
  flops+=(end-beg)*nrows_*2;
  memrefs+=(end-beg)*nrows_*2*sizeof(double)+(end-beg)*sizeof(double);
}


#define DENSEMATRIX_VERSION 3

void
DenseColMajMatrix::io(Piostream& stream)
{
  int version=stream.begin_class("DenseColMajMatrix", DENSEMATRIX_VERSION);

  // Do the base class first.
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
  
  if (stream.reading())
  {
    dataptr_ = scinew double[nrows_ * ncols_];
  }
  stream.begin_cheap_delim();

  int split;
  if (stream.reading())
  {
    if (version > 2)
    {
      Pio(stream, separate_raw_);
      if (separate_raw_)
      {
        Pio(stream, raw_filename_);
        FILE *f = fopen(raw_filename_.c_str(), "r");
        if (f)
        {
          fread(dataptr_, sizeof(double), nrows_ * ncols_, f);
          fclose(f);
        }
        else
        {
          const string errmsg = "Error reading separated file '" +
            raw_filename_ + "'";
          std::cerr << errmsg << "\n";
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
      fwrite(dataptr_, sizeof(double), nrows_ * ncols_, f);
      fclose(f);
    }
  }

  if (!split)
  {
    if (!stream.block_io(dataptr_, sizeof(double), (size_t)(nrows_ * ncols_)))
    {
      for (index_type i = 0; i < (index_type)(nrows_ * ncols_); i++)
      {
        stream.io(dataptr_[i]);
      }
    }
  }
  stream.end_cheap_delim();
  stream.end_class();
}

} // End namespace SCIRun
