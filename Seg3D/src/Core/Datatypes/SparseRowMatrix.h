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
 *  SparseRowMatrix.h:  Sparse Row Matrices
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   November 1994
 *
 *  Copyright (C) 1994 SCI Group
 */


#ifndef CORE_DATATYPES_SPARSEROWMATRIX_H
#define CORE_DATATYPES_SPARSEROWMATRIX_H 1

#include <Core/Datatypes/Matrix.h>
#include <Core/Containers/Array1.h>

#include <Core/Datatypes/share.h>

namespace SCIRun {

class SCISHARE SparseRowMatrix : public Matrix {
private:
  SparseRowMatrix(); // This is only used by the maker function.

public:
  //! Public data
  index_type* rows;
  index_type* columns;
  size_type nnz;
  double* a;

  bool validate();
  void order_columns();

  //! Constructors
  // Here's what the arguements for the constructor should be:
  //   r   = number of rows
  //   c   = number of columns
  //   rr  = row accumulation buffer containing r+1 entries where
  //         rr[N+1]-rr[N] is the number of non-zero entries in row N
  //   cc  = column number for each nonzero data entry.  Sorted by
  //         row/col orderand corresponds with the spaces in the rr array.
  //   nnz = number of non zero entries.
  //   d   = non zero data values.
  //   sort_columns = do we need to sort columns of the matrix
  SparseRowMatrix(size_type r, size_type c, index_type *rr, index_type *cc, 
                  size_type nnz, double *data, bool sort_columns = false);

  SparseRowMatrix(size_type r, size_type c, index_type *rr, index_type *cc, 
                  size_type nnz, bool sort_columns = false);

  SparseRowMatrix(const SparseRowMatrix&);

  virtual SparseRowMatrix* clone();
  SparseRowMatrix& operator=(const SparseRowMatrix&);

  //! Destructor
  virtual ~SparseRowMatrix();

  virtual DenseMatrix *dense();
  virtual SparseRowMatrix *sparse();
  virtual ColumnMatrix *column();
  virtual DenseColMajMatrix *dense_col_maj();

  virtual double*   get_data_pointer() const;
  virtual size_type get_data_size() const;

  index_type getIdx(index_type, index_type);
  size_type get_nnz() { return nnz; }
  
  virtual double*     get_val() { return a; }
  virtual index_type* get_row() { return rows; }
  virtual index_type* get_col() { return columns; }

  virtual void zero();
  virtual double get(index_type, index_type) const;
  virtual void put(index_type row, index_type col, double val);
  virtual void add(index_type row, index_type col, double val);
  
  virtual double min();
  virtual double max();
    
  virtual void getRowNonzerosNoCopy(index_type r, size_type &size, 
                                    size_type &stride,
                                    index_type *&cols, double *&vals);

  //! 
  virtual SparseRowMatrix *transpose() const;
  virtual void mult(const ColumnMatrix& x, ColumnMatrix& b,
		    int& flops, int& memrefs, index_type beg=-1, index_type end=-1,
		    int spVec=0) const;
  virtual void mult_transpose(const ColumnMatrix& x, ColumnMatrix& b,
			      int& flops, int& memrefs, index_type beg=-1, 
			      index_type end=-1, int spVec=0) const;
  virtual MatrixHandle submatrix(index_type r1, index_type c1, 
                                 index_type r2, index_type c2);

  void sparse_mult(const DenseMatrix& x, DenseMatrix& b) const;
  void sparse_mult_transXB(const DenseMatrix& x, DenseMatrix& b) const;
  MatrixHandle sparse_sparse_mult(const SparseRowMatrix &x) const;
  void solve(ColumnMatrix&);

  static SparseRowMatrix *identity(size_type size);

  virtual void print() const;
  virtual void print(std::ostream&) const;
 
  //! Persistent representation...
  virtual string type_name() { return "SparseRowMatrix"; }
  virtual void io(Piostream&);
  static PersistentTypeID type_id;

  friend SparseRowMatrix *AddSparse(const SparseRowMatrix &a,
						 const SparseRowMatrix &b);
  friend SparseRowMatrix *SubSparse(const SparseRowMatrix &a,
						 const SparseRowMatrix &b);


  static Persistent *maker();
};

} // End namespace SCIRun

#endif
