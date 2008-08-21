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
 *  DenseColMajMatrix.h:  DenseColMaj matrices
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   October 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

#ifndef CORE_DATATYPES_DENSECOLMAJMATRIX_H
#define CORE_DATATYPES_DENSECOLMAJMATRIX_H 1

#include <Core/Datatypes/Matrix.h>
#include <Core/Geometry/Transform.h>
#include <Core/Math/MiscMath.h>

#include <vector>

#include <Core/Datatypes/share.h>

namespace SCIRun {

using std::vector;

class SCISHARE DenseColMajMatrix : public Matrix
{

public:
  // Import definitions of size and index type to be used in this data class
  typedef SCIRun::index_type                 index_type;
  typedef SCIRun::size_type                  size_type;

  //! Constructors
  DenseColMajMatrix();
  DenseColMajMatrix(size_type r, size_type c);
  DenseColMajMatrix(const DenseColMajMatrix&);

  //! Destructor
  virtual ~DenseColMajMatrix();
  
  //! Public member functions
  virtual DenseColMajMatrix* clone();
  DenseColMajMatrix& operator=(const DenseColMajMatrix&);
  
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
    
  virtual void    getRowNonzerosNoCopy(index_type r, size_type &size, 
                                       size_type &stride,
                                       index_type *&cols, double *&vals);

  virtual DenseColMajMatrix* transpose() const;
  virtual void    mult(const ColumnMatrix& x, ColumnMatrix& b,
                       index_type beg=-1, index_type end=-1, 
                       int spVec=0) const;
  virtual void    mult_transpose(const ColumnMatrix& x, ColumnMatrix& b,
                                 index_type beg=-1, index_type end=-1, 
                                 int spVec=0) const;
  virtual MatrixHandle submatrix(index_type r1, index_type c1, 
                                 index_type r2, index_type c2);


  double  sumOfCol(size_type);
  double  sumOfRow(size_type);
  

  //! fast accessors
  inline double &iget(index_type r, index_type c)
  {
    return dataptr_[c * nrows_ + r];
  };

  //! fast accessors
  inline const double &iget(index_type r, index_type c) const
  {
    return dataptr_[c * nrows_ + r];
  };

  //! Return false if not invertable.
  //bool invert();

  //! Throws an assertion if not square
  double determinant();

  //void svd(DenseColMajMatrix&, SparseRowMatrix&, DenseColMajMatrix&);
  //void eigenvalues(ColumnMatrix&, ColumnMatrix&);
  //void eigenvectors(ColumnMatrix&, ColumnMatrix&, DenseColMajMatrix&);

  static DenseColMajMatrix *identity(size_type size);

  virtual void    print() const;
  virtual void    print(ostream&) const;
  
  //! Persistent representation...
  virtual void io(Piostream&);
  static PersistentTypeID type_id;
  
private:      
  double*  dataptr_;
};


} // End namespace SCIRun

#endif
