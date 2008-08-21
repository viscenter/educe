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


#include <Core/Algorithms/Math/SortMatrix/SortMatrixRows.h>
#include <Core/Algorithms/Math/SelectMatrix/SelectMatrixRows.h>

#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>

// STL Fucntions we need
#include <algorithm>
#include <vector>

namespace SCIRunAlgo {

using namespace SCIRun;


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


bool 
SortMatrixRowsAlgo::
get_sortorder(MatrixHandle input, std::vector<Matrix::index_type>& order)
{
  algo_start("SortMatrixRows",false);
  bool asc = get_bool("ascending");
  
  if (input.get_rep() == 0)
  {
    error("No input matrix");
    algo_end(); return (false); 
  }

  if (!(input->is_dense()||input->is_column()))
  {
    error("This function has only been implemented for DenseMatrix and ColumnMatrix");
    algo_end(); return (false);
  }

  Matrix::size_type nrows = input->nrows();
  order.resize(nrows);
  for (Matrix::index_type p=0; p<nrows; p++) order[p] = p;
  
  if (input->is_dense())
  {
    DenseMatrix* dm = input->as_dense();
    if (asc == true)
      std::sort(order.begin(),order.end(),SortAscDenseMatrix(dm));
    else
      std::sort(order.begin(),order.end(),SortDescDenseMatrix(dm));      
  }

  if (input->is_column())
  {
    ColumnMatrix* cm = input->as_column();
    if (asc == true)
      std::sort(order.begin(),order.end(),SortAscColumnMatrix(cm));
    else
      std::sort(order.begin(),order.end(),SortDescColumnMatrix(cm));      
  }

  algo_end(); return (true);
}



bool 
SortMatrixRowsAlgo::
get_sortorder(MatrixHandle input, MatrixHandle& output)
{
  algo_start("SortMatrixRows",false);

  if (input.get_rep() == 0)
  {
    error("No input matrix");
    algo_end(); return (false); 
  }
  
  Matrix::size_type nrows = input->nrows();
  std::vector<Matrix::index_type> order;
  if (!(get_sortorder(input,order))) 
  {
    algo_end(); return (false);
  }
  
  output = new DenseMatrix(nrows,1);
  if (output.get_rep() == 0)
  {
    error("Could not allocate output matrix");
    algo_end(); return (false);
  }

  double* data = output->get_data_pointer();
  for (Matrix::index_type p=0; p<nrows; p++) data[p] = static_cast<double>(order[p]);
  
  algo_end(); return (true);
}


bool 
SortMatrixRowsAlgo::
run(MatrixHandle input, MatrixHandle& output)
{
  algo_start("SortMatrixRows",false);
  
  std::vector<Matrix::index_type> order;
  if(!(get_sortorder(input,order))) 
  {
    algo_end(); return (false);
  }
  
  SelectMatrixRowsAlgo algo;
  algo.set_progress_reporter(this);
  if(!(algo.run(input,output,order)))
  {
    algo_end(); return (false);
  }

  algo_end(); return(true);
}

} // end namespace SCIRunAlgo
