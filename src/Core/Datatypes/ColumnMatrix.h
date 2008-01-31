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
 *  ColumnMatrix.h: for RHS and LHS
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   July 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

#ifndef CORE_DATATYPES_COLUMNMATRIX_H
#define CORE_DATATYPES_COLUMNMATRIX_H 1

#include <Core/Util/FancyAssert.h>
#include <Core/Containers/LockingHandle.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Datatype.h>

#include <Core/Datatypes/share.h>

namespace SCIRun {

class SCISHARE ColumnMatrix : public Matrix {

public:
  // Import definitions of size and index type to be used in this data class
  typedef SCIRun::index_type                 index_type;
  typedef SCIRun::size_type                  size_type;

  ColumnMatrix(size_type rows = 0);
  ColumnMatrix(const ColumnMatrix&);
  ColumnMatrix& operator=(const ColumnMatrix&);
  virtual ColumnMatrix* clone();
  virtual ~ColumnMatrix();

  virtual DenseMatrix *dense();
  virtual SparseRowMatrix *sparse();
  virtual ColumnMatrix *column();
  virtual DenseColMajMatrix *dense_col_maj();

  virtual double *get_data_pointer() const;
  virtual size_type get_data_size() const;

  inline double& operator[](int r) const
  {
    ASSERTRANGE(r, 0, nrows_)
    return data[r];
  }
  double* get_data() const {return data;}
  void set_data(double* d) {data = d;} 
  double  get(index_type r) const      
    { ASSERTRANGE(r, 0, nrows_); return data[r]; };
  void    put(index_type r, double val) 
    { ASSERTRANGE(r, 0, nrows_); data[r] = val; };

  void resize(size_type);

  virtual void zero();
  virtual double get(index_type, index_type) const;
  virtual void put(index_type row, index_type col, double val);
  virtual void add(index_type row, index_type col, double val);
  
  virtual void getRowNonzeros(index_type r, 
                              Array1<index_type>& idx, 
                              Array1<double>& val);
  virtual void getRowNonzerosNoCopy(index_type r, size_type &size, 
                                    index_type &stride,
                                    index_type *&cols, double *&vals);

  virtual Matrix *transpose() const;
  virtual void mult(const ColumnMatrix& x, ColumnMatrix& b,
		    int& flops, int& memrefs, index_type beg=-1, index_type end=-1, 
		    int spVec=0) const;
  virtual void mult_transpose(const ColumnMatrix& x, ColumnMatrix& b,
			      int& flops, int& memrefs,
			      index_type beg=-1, index_type end=-1, int spVec=0) const;
  virtual void scalar_multiply(double s);
  virtual MatrixHandle submatrix(index_type r1, index_type c1, 
                                 index_type r2, index_type c2);


  int solve(ColumnMatrix&);
  int solve(vector<double>& sol);
  double sumOfCol(index_type);

  DenseMatrix exterior(const ColumnMatrix &) const;
  double vector_norm() const;
  double vector_norm(int& flops, int& memrefs) const;
  double vector_norm(int& flops, int& memrefs, 
                     index_type beg, index_type end) const;

  virtual void print();
  virtual void print() const;
  virtual void print(std::ostream&) const;
  
  // Persistent representation...
  virtual string type_name() { return "ColumnMatrix"; }
  virtual void io(Piostream&);
  static PersistentTypeID type_id;


  SCISHARE friend void Mult(ColumnMatrix&, const ColumnMatrix&, double s);
  SCISHARE friend void Mult(ColumnMatrix&, const ColumnMatrix&, const ColumnMatrix&);
  SCISHARE friend void Mult(ColumnMatrix&, const ColumnMatrix&, const ColumnMatrix&,
                            int& flops, int& memrefs);
  SCISHARE friend void Mult(ColumnMatrix&, const ColumnMatrix&, const ColumnMatrix&,
				int& flops, int& memrefs, index_type beg, index_type end);
  SCISHARE friend void Sub(ColumnMatrix&, const ColumnMatrix&, const ColumnMatrix&);
  SCISHARE friend void Sub(ColumnMatrix&, const ColumnMatrix&, const ColumnMatrix&,
			       int& flops, int& memrefs);
  SCISHARE friend double Dot(const ColumnMatrix&, const ColumnMatrix&);
  SCISHARE friend double Dot(const ColumnMatrix&, const ColumnMatrix&,
				 int& flops, int& memrefs);
  SCISHARE friend double Dot(const ColumnMatrix&, const ColumnMatrix&,
				 int& flops, int& memrefs, index_type beg, index_type end);
  SCISHARE friend void ScMult_Add(ColumnMatrix&, double s, const ColumnMatrix&,
				      const ColumnMatrix&);
  SCISHARE friend void ScMult_Add(ColumnMatrix&, double s, const ColumnMatrix&,
				      const ColumnMatrix&, int& flops, int& memrefs);
  SCISHARE friend void ScMult_Add(ColumnMatrix&, double s, const ColumnMatrix&,
				      const ColumnMatrix&, int& flops, int& memrefs,
				      index_type beg, index_type end);
  
  SCISHARE friend void Copy(ColumnMatrix&, const ColumnMatrix&);
  SCISHARE friend void Copy(ColumnMatrix&, const ColumnMatrix&, 
				int& flops, int& refs);
  SCISHARE friend void Copy(ColumnMatrix&, const ColumnMatrix&, 
                            int& flops, int& refs,
                            index_type beg, index_type end);
  SCISHARE friend void AddScMult(ColumnMatrix&, const ColumnMatrix&, 
                                 double s, const ColumnMatrix&);
  SCISHARE friend void Add(ColumnMatrix&, const ColumnMatrix&, 
                           const ColumnMatrix&);
  SCISHARE friend void Add(ColumnMatrix&, const ColumnMatrix&, 
                           const ColumnMatrix&, const ColumnMatrix&);
                           
private:
  double* data;
                           
};


} // End namespace SCIRun

#endif
