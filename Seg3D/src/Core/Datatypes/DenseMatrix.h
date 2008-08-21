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
 *  DenseMatrix.h:  Dense matrices
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   October 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

#ifndef CORE_DATATYPES_DENSEMATRIX_H
#define CORE_DATATYPES_DENSEMATRIX_H 1

#include <Core/Math/MiscMath.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Geometry/Transform.h>

#include <vector>

#include <Core/Datatypes/share.h>

namespace SCIRun {

using std::vector;

class SCISHARE DenseMatrix : public Matrix {

public:
  // Import definitions of size and index type to be used in this data class
  typedef SCIRun::index_type                 index_type;
  typedef SCIRun::size_type                  size_type;
  
  
  //! Constructors
  DenseMatrix();
  DenseMatrix(size_type r, size_type c);
  DenseMatrix(const DenseMatrix&);
  DenseMatrix(const Transform &t);
  DenseMatrix(double scalar);
  
  //! Destructor
  virtual ~DenseMatrix();
  
  //! Public member functions
  virtual DenseMatrix* clone();
  DenseMatrix& operator=(const DenseMatrix&);
  
  virtual DenseMatrix *dense();
  virtual SparseRowMatrix *sparse();
  virtual ColumnMatrix *column();
  virtual DenseColMajMatrix *dense_col_maj();

  virtual double*   get_data_pointer() const;
  virtual size_type get_data_size() const;

  //! slow setters/getter for polymorphic operations
  virtual void    zero();
  virtual double  get(index_type r, index_type c) const;
  virtual void    put(index_type r, index_type c, double val);
  virtual void    add(index_type r, index_type c, double val);

  virtual double min();
  virtual double max();
  virtual int compute_checksum();
  
  virtual void    getRowNonzeros(index_type r, Array1<index_type>& idx, 
                                 Array1<double>& val);
  virtual void    getRowNonzerosNoCopy(index_type r, size_type &size, 
                                       index_type &stride,
                                       index_type *&cols, double *&vals);

  virtual DenseMatrix* transpose() const;
  virtual void    mult(const ColumnMatrix& x, ColumnMatrix& b,
                      index_type beg=-1, index_type end=-1, 
                      int spVec=0) const;
  virtual void    mult_transpose(const ColumnMatrix& x, ColumnMatrix& b,
                                 index_type beg=-1, index_type end=-1, 
                                 int spVec=0) const;
  virtual void scalar_multiply(double s);
  virtual MatrixHandle submatrix(index_type r1, index_type c1, 
                                 index_type r2, index_type c2);

	void multiply(ColumnMatrix& x, ColumnMatrix& b) const;

  double  sumOfCol(index_type);
  double  sumOfRow(index_type);
  
  int     solve(ColumnMatrix&, int overwrite=0);
  int     solve(ColumnMatrix& rhs, ColumnMatrix& lhs,
		int overwrite=0);
  int     solve(vector<double>& sol, int overwrite=0);
  int     solve(const vector<double>& rhs, vector<double>& lhs,
		int overwrite=0);

  //! fast accessors
  inline double*  operator[](index_type r) 
  {
    return data[r];
  };
  inline double const*  operator[](index_type r) const
  {
    return data[r];
  };
  
  inline double* getData() 
  {
    return dataptr_;
  }

  //! return false if not invertable.
  bool invert();

  //! throws an assertion if not square
  double determinant();

 
  void mult(double s);
  
  void svd(DenseMatrix&, SparseRowMatrix&, DenseMatrix&);
  void eigenvalues(ColumnMatrix&, ColumnMatrix&);
  void eigenvectors(ColumnMatrix&, ColumnMatrix&, DenseMatrix&);

  static DenseMatrix *identity(size_type size);

  virtual void    print() const;
  virtual void    print(ostream&) const;
  
  //! Persistent representation...
  virtual string type_name() { return "DenseMatrix"; }
  virtual void io(Piostream&);
  static PersistentTypeID type_id;

  //! Friend functions
  SCISHARE friend void Mult(DenseMatrix&, const DenseMatrix&, const DenseMatrix&);

private:
  double** data;
  double*  dataptr_;

};


//! Friend functions
SCISHARE void Sub(DenseMatrix&, const DenseMatrix&, const DenseMatrix&);
SCISHARE void Add(DenseMatrix&, const DenseMatrix&, const DenseMatrix&);
SCISHARE void Add(DenseMatrix&, double, const DenseMatrix&, double, const DenseMatrix&);
SCISHARE void Add(double, DenseMatrix&, double, const DenseMatrix&);
SCISHARE void Mult_trans_X(DenseMatrix&, const DenseMatrix&, const DenseMatrix&);
SCISHARE void Mult_X_trans(DenseMatrix&, const DenseMatrix&, const DenseMatrix&);
SCISHARE void Concat_rows(DenseMatrix&, const DenseMatrix&, const DenseMatrix&); // Added by Saeed Babaeizadeh, Jan. 2006
SCISHARE void Concat_cols(DenseMatrix&, const DenseMatrix&, const DenseMatrix&); // Added by Saeed Babaeizadeh, Jan. 2006

} // End namespace SCIRun

#endif
