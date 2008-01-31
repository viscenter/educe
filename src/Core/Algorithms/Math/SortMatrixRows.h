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

#ifndef CORE_ALGORITHMS_MATH_SORTMATRIXROWS_H
#define CORE_ALGORITHMS_MATH_SORTMATRIXROWS_H 1

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>

// STL Fucntions we need
#include <algorithm>
#include <vector>

#include <Core/Algorithms/Math/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SCISHARE SortMatrixRowsAlgo 
{
  public:
    static bool GetSortOrderMatrixRows(ProgressReporter *pr,MatrixHandle input, std::vector<Matrix::index_type>& output, bool asc=true); 
    static bool GetSortOrderMatrixRows(ProgressReporter *pr,MatrixHandle input, MatrixHandle& output, bool asc=true); 
    static bool SortMatrixRows(ProgressReporter *pr,MatrixHandle input, MatrixHandle& output, bool asc=true);
    
  
    class SortAscDenseMatrix : public std::binary_function<Matrix::index_type,Matrix::index_type,bool>
    {
      public:
        SortAscDenseMatrix(DenseMatrix* mat)
        {
          m_ = static_cast<Matrix::index_type>(mat->nrows());
          n_ = static_cast<Matrix::index_type>(mat->ncols());
          data_ = mat->getData();
        }
        
        bool operator()(Matrix::index_type i1, Matrix::index_type i2)
        {
          for (Matrix::index_type p=0; p<n_; p++)
          {
            double val1 = data_[n_*i1+p];
            double val2 = data_[n_*i2+p];
            if (val1 == val2) continue;
            return (val1 < val2);
          }
          return (false);        
        }
    
      private:
        Matrix::index_type m_;
        Matrix::index_type n_;
        double*      data_;
    };
 
     class SortDescDenseMatrix : public std::binary_function<Matrix::index_type,Matrix::index_type,bool>
    {
      public:
        SortDescDenseMatrix(DenseMatrix* mat)
        {
          m_ = static_cast<Matrix::index_type>(mat->nrows());
          n_ = static_cast<Matrix::index_type>(mat->ncols());
          data_ = mat->getData();
        }
        
        bool operator()(Matrix::index_type i1, Matrix::index_type i2)
        {
          for (Matrix::index_type p=0; p<n_; p++)
          {
            double val1 = data_[n_*i1+p];
            double val2 = data_[n_*i2+p];
            if (val1 == val2) continue;
            return (val1 > val2);
          }
          return (false);        
        }
    
      private:
        Matrix::index_type m_;
        Matrix::index_type n_;
        double*      data_;
    };
 

    class SortAscColumnMatrix : public std::binary_function<Matrix::index_type,Matrix::index_type,bool>
    {
      public:
        SortAscColumnMatrix(ColumnMatrix* mat)
        {
          data_ = mat->get_data();
        }
        
        bool operator()(Matrix::index_type i1, Matrix::index_type i2)
        {
          return (data_[i1] < data_[i2]);
        }
  
      private:
        double*      data_;
    };    
    
    class SortDescColumnMatrix : public std::binary_function<Matrix::index_type,Matrix::index_type,bool>
    {
      public:
        SortDescColumnMatrix(ColumnMatrix* mat)
        {
          data_ = mat->get_data();
        }
        
        bool operator()(Matrix::index_type i1, Matrix::index_type i2)
        {
          return (data_[i1] > data_[i2]);
        }
  
      private:
        double*      data_;
    };    
    
};

} // end namespace SCIRunAlgo

#endif
