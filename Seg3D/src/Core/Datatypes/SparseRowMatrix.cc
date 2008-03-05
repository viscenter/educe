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
 *  SparseRowMatrix.cc: ?
 *
 *  Written by:
 *   Author: ?
 *   Department of Computer Science
 *   University of Utah
 *   Date: ?
 *
 *  Copyright (C) 199? SCI Group
 */

#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColMajMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Math/MiscMath.h>
#include <Core/Math/MinMax.h>
#include <Core/Util/Assert.h>
#include <Core/Malloc/Allocator.h>

#include <iostream>
#include <algorithm>
using std::cerr;
using std::endl;

#include <float.h>
#include <stdio.h>
#include <memory.h>

namespace SCIRun {

Persistent*
SparseRowMatrix::maker()
{
  return scinew SparseRowMatrix;
}


PersistentTypeID SparseRowMatrix::type_id("SparseRowMatrix", "Matrix",
					  SparseRowMatrix::maker);


SparseRowMatrix*
SparseRowMatrix::clone()
{
  return scinew SparseRowMatrix(*this);
}


SparseRowMatrix::SparseRowMatrix() :
  rows(0),
  columns(0),
  nnz(0),
  a(0)
{
}


class matrix_sort_type : public std::binary_function<Matrix::index_type,Matrix::index_type,bool>
{
  public:
    matrix_sort_type(index_type* cols) :
      cols_(cols)
    {}
    
    bool operator()(Matrix::index_type i1, Matrix::index_type i2)
    {
      return(cols_[i1] < cols_[i2]);
    }

  private:
    index_type*      cols_;
};

void
SparseRowMatrix::order_columns()
{

  //! This code should reorder the columns of a sparse matrix so they are
  //! in ascending order. This code will purge dublicates as well by merging
  //! them together.
  
  //! Calculate the size of the buffer we need to reorder the data
  //! This way we can use the stl sorting algorithm  
  size_type max_num_cols = 0;
  for (index_type j = 0; j< nrows_; j++)
    if (rows[j+1]-rows[j] > max_num_cols)
      max_num_cols = rows[j+1]- rows[j];

  vector<index_type> order(max_num_cols);
  vector<double>     databuffer(max_num_cols);
  vector<index_type> indexbuffer(max_num_cols);

  matrix_sort_type sortmatrix(columns);
  
  index_type rr = rows[0];
  
  //! Sorting columns and removing duplicates
  for (index_type j = 0; j<nrows_; j++)
  {
    size_type num_cols = rows[j+1]-rr;
    order.resize(num_cols);
    for (index_type p=rr; p<rows[j+1]; p++) order[p-rr] = p;
    std::sort(order.begin(),order.end(),sortmatrix);

    for (index_type q=0; q<num_cols; q++)
    {
      databuffer[q] = a[order[q]];
      indexbuffer[q] = columns[order[q]];
    }
    
    index_type p = rows[j];
    index_type q = 0;
    if (q < num_cols)
    {
      a[p] = databuffer[q];
      columns[p] = indexbuffer[q];
      q++;
    }
    
    while(q < num_cols)
    {
      if (columns[p] == indexbuffer[q])
      {
        a[p] += databuffer[q];
        q++;
      }
      else
      {
        p++;
        columns[p] = indexbuffer[q];
        a[p] = databuffer[q];
        q++;
      }
    }
    p++;
    
    rr = rows[j+1];
    rows[j+1] = p;
  }
  
  nnz = rows[nrows_];
}



SparseRowMatrix::SparseRowMatrix(size_type nnrows, size_type nncols,
				 index_type* rows, index_type* columns,
				 size_type nnz, double* a_, 
         bool sort_columns) :
  Matrix(nnrows, nncols),
  rows(rows),
  columns(columns),
  nnz(nnz),
  a(a_)
{
  if (a == 0) { a = scinew double[nnz]; }
  if (sort_columns) order_columns();
  ASSERT(validate());
}


SparseRowMatrix::SparseRowMatrix(size_type nnrows, size_type nncols,
				 index_type* rows, index_type* columns,
				 size_type nnz, bool sort_columns) :
  Matrix(nnrows, nncols),
  rows(rows),
  columns(columns),
  nnz(nnz)
{
  a = scinew double[nnz];
  if (sort_columns) order_columns();
  ASSERT(validate());
}


SparseRowMatrix::SparseRowMatrix(const SparseRowMatrix& copy) :
  Matrix(copy.nrows_, copy.ncols_),
  nnz(copy.nnz)
{
  rows = scinew index_type[nrows_+1];
  columns = scinew index_type[nnz];
  a = scinew double[nnz];
  memcpy(a, copy.a, sizeof(double)*nnz);
  memcpy(rows, copy.rows, sizeof(index_type)*(nrows_+1));
  memcpy(columns, copy.columns, sizeof(index_type)*nnz);
  ASSERT(validate());
}


SparseRowMatrix::~SparseRowMatrix()
{
  if (a)
  {
    delete[] a;
  }
  if (columns)
  {
    delete[] columns;
  }
  if (rows)
  {
    delete[] rows;
  }
}


bool
SparseRowMatrix::validate()
{
  index_type i, j;

  ASSERTMSG(rows[0] == 0, "Row start is nonzero.");
  for (i = 0; i< nrows_; i++)
  {
    ASSERTMSG(rows[i] <= rows[i+1], "Malformed rows, not increasing.");
    for (j = rows[i]; j < rows[i+1]; j++)
    {
      ASSERTMSG(columns[j] >= 0 && columns[j] < ncols_, "Column out of range.");
      if (j != rows[i])
      {
        ASSERTMSG(columns[j-1] != columns[j], "Column doubled.");
        ASSERTMSG(columns[j-1] < columns[j], "Column out of order.");
      }
    }
  }
  ASSERTMSG(rows[nrows_] == nnz, "Row end is incorrect.");
  return true;
}


SparseRowMatrix *
SparseRowMatrix::sparse()
{
  return this;
}


DenseMatrix *
SparseRowMatrix::dense()
{
  DenseMatrix *dm = scinew DenseMatrix(nrows_, ncols_);
  if (nrows_ == 0) return dm;
  dm->zero();
  index_type count=0;
  index_type nextRow;
  for (index_type r=0; r<nrows_; r++)
  {
    nextRow = rows[r+1];
    while (count<nextRow)
    {
      (*dm)[r][columns[count]]=a[count];
      count++;
    }
  }
  return dm;
}


DenseColMajMatrix *
SparseRowMatrix::dense_col_maj()
{
  DenseColMajMatrix *dm = scinew DenseColMajMatrix(nrows_, ncols_);
  if (nrows_ == 0) return dm;
  dm->zero();
  index_type count = 0;
  index_type nextRow;
  for (index_type r = 0; r<nrows_; r++)
  {
    nextRow = rows[r+1];
    while (count<nextRow)
    {
      dm->iget(r, columns[count]) = a[count];
      count++;
    }
  }
  return dm;
}


ColumnMatrix *
SparseRowMatrix::column()
{
  ColumnMatrix *cm = scinew ColumnMatrix(nrows_);
  if (nrows_)
  {
    cm->zero();
    for (int i=0; i<nrows_; i++)
    {
      // If the first column entry for the row is a zero.
      if (columns[rows[i]] == 0)
      {
        (*cm)[i] = a[rows[i]];
      }
      else
      {
        (*cm)[i] = 0;
      }
    }
  }
  return cm;
}


double *
SparseRowMatrix::get_data_pointer() const
{
  return a;
}


size_type
SparseRowMatrix::get_data_size() const
{
  return nnz;
}


SparseRowMatrix *
SparseRowMatrix::transpose() const
{
  double *t_a = scinew double[nnz];
  index_type *t_columns = scinew index_type[nnz];
  index_type *t_rows = scinew index_type[ncols_+1];
  size_type t_nnz = nnz;
  size_type t_nncols = nrows_;
  size_type t_nnrows = ncols_;

  index_type *at = scinew index_type[t_nnrows+1];
  index_type i;
  for (i=0; i<t_nnrows+1;i++)
  {
    at[i] = 0;
  }
  for (i=0; i<t_nnz;i++)
  {
    at[columns[i]+1]++;
  }
  t_rows[0] = 0;
  for (i=1; i<t_nnrows+1; i++)
  {
    at[i] += at[i-1];
    t_rows[i] = at[i];
  }

  index_type c = 0;
  for (index_type r=0; r<nrows_; r++)
  {
    for (; c<rows[r+1]; c++)
    {
      index_type mcol = columns[c];
      t_columns[at[mcol]] = r;
      t_a[at[mcol]] = a[c];
      at[mcol]++;
    }
  }

  delete at;
  return scinew SparseRowMatrix(t_nnrows, t_nncols, t_rows,
                                t_columns, t_nnz, t_a);
}


index_type
SparseRowMatrix::getIdx(index_type i, index_type j)
{
  index_type row_idx=rows[i];
  index_type next_idx=rows[i+1];
  index_type l=row_idx;
  index_type h=next_idx-1;
  for (;;)
  {
    if (h<l)
    {
      return -1;
    }
    index_type m=(l+h)/2;
    if (j<columns[m])
    {
      h=m-1;
    }
    else if (j>columns[m])
    {
      l=m+1;
    }
    else
    {
      return m;
    }
  }
}


double
SparseRowMatrix::get(index_type i, index_type j) const
{
  index_type row_idx=rows[i];
  index_type next_idx=rows[i+1];
  index_type l=row_idx;
  index_type h=next_idx-1;
  for (;;)
  {
    if (h<l)
    {
      return 0.0;
    }
    index_type m=(l+h)/2;
    if (j<columns[m])
    {
      h=m-1;
    }
    else if (j>columns[m])
    {
      l=m+1;
    }
    else
    {
      return a[m];
    }
  }
}


void
SparseRowMatrix::put(index_type i, index_type j, double d)
{
  index_type row_idx=rows[i];
  index_type next_idx=rows[i+1];
  index_type l=row_idx;
  index_type h=next_idx-1;
  for (;;)
  {
    if (h<l)
    {
      ASSERTFAIL("SparseRowMatrix::put into invalid(dataless) location.");
      return;
    }
    index_type m=(l+h)/2;
    if (j<columns[m])
    {
      h=m-1;
    }
    else if (j>columns[m])
    {
      l=m+1;
    }
    else
    {
      a[m] = d;
      return;
    }
  }
}


void
SparseRowMatrix::add(index_type i, index_type j, double d)
{
  index_type row_idx=rows[i];
  index_type next_idx=rows[i+1];
  index_type l=row_idx;
  index_type h=next_idx-1;
  for (;;)
  {
    if (h<l)
    {
      ASSERTFAIL("SparseRowMatrix::add into invalid(dataless) location.");
      return;
    }
    index_type m=(l+h)/2;
    if (j<columns[m])
    {
      h=m-1;
    }
    else if (j>columns[m])
    {
      l=m+1;
    }
    else
    {
      a[m] += d;
      return;
    }
  }
}


double
SparseRowMatrix::min()
{
  double min = DBL_MAX;
  for (index_type k=0; k<nnz; k++)
    if (a[k] < min) min = a[k];
  return (min);
}

double
SparseRowMatrix::max()
{
  double max = -DBL_MAX;
  for (index_type k=0; k<nnz; k++)
    if (a[k] > max) max = a[k];
  return (max);
}


void
SparseRowMatrix::getRowNonzerosNoCopy(index_type r, size_type &size, 
                                      index_type &stride,
                                      index_type *&cols, double *&vals)
{
  size = rows[r+1] - rows[r];
  stride = 1;
  cols = columns + rows[r];
  vals = a + rows[r];
}


void
SparseRowMatrix::zero()
{
  double* ptr=a;
  for (index_type i=0;i<nnz;i++)
    *ptr++=0.0;
}


void
SparseRowMatrix::solve(ColumnMatrix&)
{
  ASSERTFAIL("SparseRowMatrix can't do a direct solve!");
}


void
SparseRowMatrix::mult(const ColumnMatrix& x, ColumnMatrix& b,
		      int& flops, int& memrefs, 
          index_type beg, index_type end,
		      int) const
{
  // Compute A*x=b
  ASSERT(x.nrows() == ncols_);
  ASSERT(b.nrows() == nrows_);
  if (beg==-1) beg = 0;
  if (end==-1) end = nrows_;
  double* xp=&x[0];
  double* bp=&b[0];
  
  
  index_type i, j;
  for(i=beg;i<end;i++)
  {
    register double sum=0;
    index_type row_idx=rows[i];
    index_type next_idx=rows[i+1];
    for(j=row_idx;j<next_idx;j++)
    {
	    sum+=a[j]*xp[columns[j]];
    }
    bp[i]=sum;
  }  

  size_type nnz=2*(rows[end]-rows[beg]);
  flops+=2*(rows[end]-rows[beg]);
  size_type nr=end-beg;
  memrefs+=2*sizeof(size_type)*nr+nnz*sizeof(size_type)+2*nnz*sizeof(double)+nr*sizeof(double);
}


void
SparseRowMatrix::mult_transpose(const ColumnMatrix& x, ColumnMatrix& b,
				int& flops, int& memrefs,
				index_type beg, index_type end, int) const
{
  // Compute At*x=b
  ASSERT(x.nrows() == nrows_);
  ASSERT(b.nrows() == ncols_);
  if (beg==-1) beg = 0;
  if (end==-1) end = nrows_;
  double* bp=&b[0];
  for (index_type i=beg; i<end; i++)
    bp[i] = 0;
  for (index_type j=0; j<nrows_; j++)
  {
    if (!x[j]) continue;
    double xj = x[j];
    index_type row_idx = rows[j];
    index_type next_idx = rows[j+1];
    index_type i=row_idx;
    for (; i<next_idx && columns[i] < beg; i++);
    for (; i<next_idx && columns[i] < end; i++)
      bp[columns[i]] += a[i]*xj;
  }
  size_type nnz=2*(rows[end]-rows[beg]);
  flops+=2*(rows[end]-rows[beg]);
  size_type nr=end-beg;
  memrefs+=2*sizeof(size_type)*nr+nnz*sizeof(size_type)+2*nnz*sizeof(double)+nr*sizeof(double);
}


void
SparseRowMatrix::sparse_mult(const DenseMatrix& x, DenseMatrix& b) const
{
  // Compute A*x=b
  ASSERT(x.nrows() == ncols_);
  ASSERT(b.nrows() == nrows_);
  ASSERT(x.ncols() == b.ncols());
  index_type i, j, k;

  for (j = 0; j < b.ncols(); j++)
  {
    for (i = 0; i < b.nrows(); i++)
    {
      double sum = 0.0;
      for (k = rows[i]; k < rows[i+1]; k++)
      {
        sum += a[k] * x.get(columns[k], j);
      }
      b.put(i, j, sum);
    }
  }
}



void
SparseRowMatrix::sparse_mult_transXB(const DenseMatrix& x,
                                     DenseMatrix& b) const
{
  // Compute A*xT=bT
  ASSERT(x.ncols() == ncols_);
  ASSERT(b.ncols() == nrows_);
  ASSERT(x.nrows() == b.nrows());
  index_type i, j, k;

  for (j = 0; j < b.nrows(); j++)
  {
    for (i = 0; i < b.ncols(); i++)
    {
      double sum = 0.0;
      for (k = rows[i]; k < rows[i+1]; k++)
      {
       sum += a[k] * x.get(j, columns[k]);
      }
      b.put(j, i, sum);
    }
  }
}


class SparseSparseElement {
public:
  index_type     row; 
  index_type     col;
  double         val;
};

inline bool operator<(const SparseSparseElement& s1, const SparseSparseElement& s2)
{
  if (s1.row < s2.row) return(true);
  if ((s1.row == s2.row)&&(s1.col < s2.col)) return(true);
  return (false);
}

inline bool operator==(const SparseSparseElement& s1, const SparseSparseElement& s2)
{
  if ((s1.row == s2.row)&&(s1.col == s2.col)) return(true);
  return (false);
}

MatrixHandle
SparseRowMatrix::sparse_sparse_mult(const SparseRowMatrix &b) const
{
  // Compute A*B=C
  ASSERT(b.nrows() == ncols_);
  
  size_type bncols = b.ncols_;
  index_type *brows = b.rows;
  index_type *bcolumns = b.columns;
  double* ba = b.a;  

  if (brows==0 || bcolumns == 0 || ba == 0 || bncols == 0)
  {
    std::cerr << "Encountered an invalid sparse matrix (B)" << std::endl;
    return (false);
  }
  
  if (rows == 0 ||columns == 0 || a == 0)
  {
    std::cerr << "Encountered an invalid sparse matrix (A)" << std::endl;
    return (false);
  }
  
  index_type k = 0;

  for (index_type r =0; r<nrows_; r++)
  {
    index_type ps = rows[r];
    index_type pe = rows[r+1];   
    for (index_type p = ps; p < pe; p++)
    {
      index_type s = columns[p];
      index_type qs = brows[s];
      index_type qe = brows[s+1];
      k += qe-qs;
    }
  }

  std::vector<SparseSparseElement> elems(k);
  
  k = 0;
  for (index_type r =0; r<nrows_; r++)
  {
    index_type ps = rows[r];
    index_type pe = rows[r+1];   
    for (index_type p = ps; p < pe; p++)
    {
      index_type s = columns[p];
      double v = a[p];
      index_type qs = brows[s];
      index_type qe = brows[s+1];
      for (index_type q=qs; q<qe; q++)
      {
        if (v*ba[q])
        {
          elems[k].row = r;
          elems[k].col = bcolumns[q];
          elems[k].val = v*ba[q];
          k++;
        }
      }
    }
  }
  
//  elems.resize(k);
        
  std::sort(elems.begin(),elems.begin()+k);
  
  index_type s = 0;
  size_type nnz = 0;
  size_type nelems = static_cast<size_type>(k);
  while (s< nelems && elems[s].val == 0.0) s++;
  
  for (index_type r=1; r< nelems; r++)
  {
    if (elems[s] == elems[r])
    {
      elems[s].val += elems[r].val;
      elems[r].val = 0.0;
    }
    else
    {
      if (elems[r].val)
      {
        nnz++;
        s = r;
      }
    }
  }
  if (nelems) if (elems[s].val) nnz++;
  
  index_type *rr = scinew index_type[nrows_+1];
  index_type *cc = scinew index_type[nnz];
  double *vv = scinew double[nnz];
  
  if ((rr == 0) || (cc == 0) || (vv == 0))
  {
    if (rr) delete[] rr;
    if (cc) delete[] cc;
    if (vv) delete[] vv;
    MatrixHandle none(0);
    return none;
  }

  rr[0] = 0;
  index_type q = 0;
  size_type kk = 0;
  for( index_type p=0; p < nrows_; p++ )
  {
    while ((kk < nelems) && (elems[kk].row == p))
    {
      if (elems[kk].val)
      {
        cc[q] = elems[kk].col; vv[q] = elems[kk].val; q++;
      } 
      kk++; 
    }
    rr[p+1] = q;
  }   
    
  MatrixHandle output = scinew SparseRowMatrix(nrows_,bncols,rr,cc,nnz,vv);
  return output;
}


void
SparseRowMatrix::print() const
{
  cerr << "Sparse RowMatrix: " << endl;
}


void SparseRowMatrix::print(std::ostream&) const
{

}


#define SPARSEROWMATRIX_VERSION 2

void
SparseRowMatrix::io(Piostream& stream)
{
  int version = stream.begin_class("SparseRowMatrix", SPARSEROWMATRIX_VERSION);
  // Do the base class first...
  Matrix::io(stream);

  if (version <2)
  {
    int r = static_cast<int>(nrows_);
    int c = static_cast<int>(ncols_);
    int n = static_cast<int>(nnz);
    stream.io(r);
    stream.io(c);
    stream.io(n);
    nrows_ = static_cast<size_type>(r);
    ncols_ = static_cast<size_type>(c);
    nnz= static_cast<size_type>(n);
  }
  else
  {
    Pio_size(stream,nrows_);
    Pio_size(stream,ncols_);
    Pio_size(stream,nnz);
  }
  
  if (stream.reading())
  {
    a = scinew double[nnz];
    columns = scinew index_type[nnz];
    rows = scinew index_type[nrows_+1];
  }
  
  stream.begin_cheap_delim();  
  Pio_index(stream,rows,nrows_+1);
  stream.end_cheap_delim();

  stream.begin_cheap_delim();
  Pio_index(stream,columns,nnz);
  stream.end_cheap_delim();

  stream.begin_cheap_delim();
  Pio(stream,a,nnz);
  stream.end_cheap_delim();

  stream.end_class();
}


SparseRowMatrix *
AddSparse(const SparseRowMatrix &a, const SparseRowMatrix &b)
{
  ASSERT(a.nrows() == b.nrows() && a.ncols() == b.ncols());

  Matrix::index_type *rows = scinew index_type[a.nrows() + 1];
  vector<Matrix::index_type> cols;
  vector<double> vals;

  Matrix::index_type r, ca, cb;

  rows[0] = 0;
  for (r = 0; r < a.nrows(); r++)
  {
    rows[r+1] = rows[r];
    ca = a.rows[r];
    cb = b.rows[r];
    for (;;)
    {
      if (ca >= a.rows[r+1] && cb >= b.rows[r+1])
      {
        break;
      }
      else if (ca >= a.rows[r+1])
      {
        cols.push_back(b.columns[cb]);
        vals.push_back(b.a[cb]);
        rows[r+1]++;
        cb++;
      }
      else if (cb >= b.rows[r+1])
      {
        cols.push_back(a.columns[ca]);
        vals.push_back(a.a[ca]);
        rows[r+1]++;
        ca++;
      }
      else if (a.columns[ca] < b.columns[cb])
      {
        cols.push_back(a.columns[ca]);
        vals.push_back(a.a[ca]);
        rows[r+1]++;
        ca++;
      }
      else if (a.columns[ca] > b.columns[cb])
      {
        cols.push_back(b.columns[cb]);
        vals.push_back(b.a[cb]);
        rows[r+1]++;
        cb++;
      }
      else
      {
        cols.push_back(a.columns[ca]);
        vals.push_back(a.a[ca] + b.a[cb]);
        rows[r+1]++;
        ca++;
        cb++;
      }
    }
  }

  Matrix::index_type *vcols = scinew Matrix::index_type[cols.size()];
  Matrix::size_type csz = static_cast<Matrix::size_type>(cols.size());
  for (Matrix::index_type i = 0; i < csz; i++)
  {
    vcols[i] = cols[i];
  }

  double *vvals = scinew double[vals.size()];
  Matrix::size_type vsz = static_cast<Matrix::size_type>(vals.size());
  for (Matrix::index_type i = 0; i < vsz; i++)
  {
    vvals[i] = vals[i];
  }

  return scinew SparseRowMatrix(a.nrows(), a.ncols(), rows,
				vcols, static_cast<Matrix::size_type>(vals.size()), vvals);
}


SparseRowMatrix *
SubSparse(const SparseRowMatrix &a, const SparseRowMatrix &b)
{
  ASSERT(a.nrows() == b.nrows() && a.ncols() == b.ncols());

  Matrix::index_type *rows = scinew Matrix::index_type[a.nrows() + 1];
  vector<Matrix::index_type> cols;
  vector<double> vals;

  Matrix::index_type r, ca, cb;

  rows[0] = 0;
  for (r = 0; r < a.nrows(); r++)
  {
    rows[r+1] = rows[r];
    ca = a.rows[r];
    cb = b.rows[r];
    for( ;; )
    {
      if (ca >= a.rows[r+1] && cb >= b.rows[r+1])
      {
        break;
      }
      else if (ca >= a.rows[r+1])
      {
        cols.push_back(b.columns[cb]);
        vals.push_back(-b.a[cb]);
        rows[r+1]++;
        cb++;
      }
      else if (cb >= b.rows[r+1])
      {
        cols.push_back(a.columns[ca]);
        vals.push_back(a.a[ca]);
        rows[r+1]++;
        ca++;
      }
      else if (a.columns[ca] < b.columns[cb])
      {
        cols.push_back(a.columns[ca]);
        vals.push_back(a.a[ca]);
        rows[r+1]++;
        ca++;
      }
      else if (a.columns[ca] > b.columns[cb])
      {
        cols.push_back(b.columns[cb]);
        vals.push_back(-b.a[cb]);
        rows[r+1]++;
        cb++;
      }
      else
      {
        cols.push_back(a.columns[ca]);
        vals.push_back(a.a[ca] - b.a[cb]);
        rows[r+1]++;
        ca++;
        cb++;
      }
    }
  }

  Matrix::index_type i;
  Matrix::index_type *vcols = scinew Matrix::index_type[cols.size()];
  for (i = 0; i < static_cast<Matrix::index_type>(cols.size()); i++)
  {
    vcols[i] = cols[i];
  }

  double *vvals = scinew double[vals.size()];
  for (i = 0; i < static_cast<Matrix::index_type>(vals.size()); i++)
  {
    vvals[i] = vals[i];
  }

  return scinew SparseRowMatrix(a.nrows(), a.ncols(), rows,
				vcols, static_cast<Matrix::size_type>(vals.size()), vvals);
}


MatrixHandle
SparseRowMatrix::submatrix(index_type r1, index_type c1, 
                            index_type r2, index_type c2)
{
  ASSERTRANGE(r1, 0, r2+1);
  ASSERTRANGE(r2, r1, nrows_);
  ASSERTRANGE(c1, 0, c2+1);
  ASSERTRANGE(c2, c1, ncols_);

  index_type i, j;
  index_type *rs = scinew index_type[r2-r1+2];
  vector<index_type> csv;
  vector<double> valsv;

  rs[0] = 0;
  for (i = r1; i <= r2; i++)
  {
    rs[i-r1+1] = rs[i-r1];
    for (j = rows[i]; j < rows[i+1]; j++)
    {
      if (columns[j] >= c1 && columns[j] <= c2)
      {
        csv.push_back(columns[j] - c1);
        valsv.push_back(a[j]);
        rs[i-r1+1]++;
      }
    }
  }

  index_type *cs = scinew index_type[csv.size()];
  double *vals = scinew double[valsv.size()];
  for (i = 0; i < static_cast<index_type>(csv.size()); i++)
  {
    cs[i] = csv[i];
    vals[i] = valsv[i];
  }

  return scinew SparseRowMatrix(r2-r1+1, c2-c1+1, rs, cs,
                           static_cast<size_type>(valsv.size()), vals);
}


SparseRowMatrix *
SparseRowMatrix::identity(size_type size)
{ 
  index_type *r = scinew index_type[size+1];
  index_type *c = scinew index_type[size];
  double *d = scinew double[size];

  index_type i;
  for (i=0; i<size; i++)
  {
    c[i] = r[i] = i;
    d[i] = 1.0;
  }
  r[i] = i;

  return scinew SparseRowMatrix(size, size, r, c, size, d);
}

} // End namespace SCIRun

