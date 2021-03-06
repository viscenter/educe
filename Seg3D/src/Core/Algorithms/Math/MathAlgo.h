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


#ifndef CORE_ALGORITHMS_MATH_MATHALGO_H
#define CORE_ALGORITHMS_MATH_MATHALGO_H 1

#include <Core/Algorithms/Util/AlgoLibrary.h>

#include <Core/Bundle/Bundle.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <sgi_stl_warnings_off.h>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <sgi_stl_warnings_on.h>

#include <Core/Algorithms/Math/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SparseElement; 
typedef std::vector<SparseElement> SparseElementVector;

class SCISHARE MathAlgo : public AlgoLibrary {

  public:
    MathAlgo(ProgressReporter* pr); // normal case

    // Build the FEMMatrix using a variable number of processes
    bool BuildFEMatrix(FieldHandle field, MatrixHandle& matrix, int num_proc, MatrixHandle ConductivityTable = 0, MatrixHandle GeomToComp = 0, MatrixHandle CompToGeom = 0);
    
    // CreateFEDirichletBC()
    bool CreateFEDirichletBC(MatrixHandle FEin, MatrixHandle RHSin, MatrixHandle BC, MatrixHandle& FEout, MatrixHandle& RHSout);
    
    // Resize a matrix, Dense or Sparse
    bool ResizeMatrix(MatrixHandle input, MatrixHandle& output, Matrix::size_type m, Matrix::size_type n);
    
    // Build a sparse matrix based on coordinates of elements
    bool CreateSparseMatrix(SparseElementVector& input, MatrixHandle& output, Matrix::size_type m, Matrix::size_type n);

    // Recursive CutHill-mcKee
    bool ReverseCuthillmcKee(MatrixHandle input,MatrixHandle& output,MatrixHandle& mapping,bool calcmapping = true);

    // CutHill-mcKee
    bool CuthillmcKee(MatrixHandle input,MatrixHandle& output,MatrixHandle& mapping,bool calcmapping = true);
    
    // Apply an operation on a row by row basis
    bool ApplyRowOperation(MatrixHandle input, MatrixHandle& output, std::string operation); 

    // Apply an operation on a column by column basis
    bool ApplyColumnOperation(MatrixHandle input, MatrixHandle& output, std::string operation); 
    
    bool IdentityMatrix(Matrix::size_type n,MatrixHandle& output); 
    
    bool MatrixSelectRows(MatrixHandle input, MatrixHandle& output, std::vector<Matrix::index_type> rows);
    bool MatrixSelectColumns(MatrixHandle input, MatrixHandle& output, std::vector<Matrix::index_type> columns);
        
    bool MatrixAppendRows(MatrixHandle input,MatrixHandle& output,MatrixHandle Rows,std::vector<Matrix::index_type>& newrows);
    bool MatrixAppendColumns(MatrixHandle input,MatrixHandle& output,MatrixHandle Columns,std::vector<Matrix::index_type>& newcolumns);

    ///////////////////////////////////////////////
    // Newer versions of algorithms
    
    bool FindNonZeroMatrixRows(MatrixHandle input, std::vector<Matrix::index_type>& rows);
    bool FindNonZeroMatrixColumns(MatrixHandle input, std::vector<Matrix::index_type>& columns);
    bool FindZeroMatrixRows(MatrixHandle input, std::vector<Matrix::index_type>& rows);
    bool FindZeroMatrixColumns(MatrixHandle input, std::vector<Matrix::index_type>& columns);
    
    bool SelectMatrixRows(MatrixHandle input, MatrixHandle& output, std::vector<Matrix::index_type> rows);
    bool SelectMatrixRows(MatrixHandle input, MatrixHandle& output, MatrixHandle rows);

    bool SelectMatrixColumns(MatrixHandle input, MatrixHandle& output, std::vector<Matrix::index_type> columns);
    bool SelectMatrixColumns(MatrixHandle input, MatrixHandle& output, MatrixHandle columns);

    bool SelectSubMatrix(MatrixHandle input, MatrixHandle& output, std::vector<Matrix::index_type> rows, std::vector<Matrix::index_type> columns);
    
    bool GetSortOrderMatrixRows(MatrixHandle input, std::vector<Matrix::index_type>& order, bool asc = true);
    bool GetSortOrderMatrixRows(MatrixHandle input, MatrixHandle& order, bool asc = true);
    bool SortMatrixRows(MatrixHandle input, MatrixHandle& output, bool asc = true);

    bool GetSortOrderMatrixColumns(MatrixHandle input, std::vector<Matrix::index_type>& order, bool asc = true);
    bool GetSortOrderMatrixColumns(MatrixHandle input, MatrixHandle& order, bool asc = true);
    bool SortMatrixColumns(MatrixHandle input, MatrixHandle& output, bool asc = true);
    
    bool ConvertMappingMatrixIntoMappingOrder(MatrixHandle input, std::vector<Matrix::index_type>& output);    
    bool ConvertMappingMatrixIntoMappingOrder(MatrixHandle input, MatrixHandle& output);
    
    bool ConvertMappingOrderIntoMappingMatrix(MatrixHandle input, MatrixHandle& output, Matrix::size_type ncols = 0);
    bool ConvertMappingOrderIntoMappingMatrix(std::vector<Matrix::index_type>& input, MatrixHandle& output, Matrix::size_type ncols = 0);

    bool ConvertMappingOrderIntoTransposeMappingMatrix(MatrixHandle input, MatrixHandle& output, Matrix::size_type nrows = 0);
    bool ConvertMappingOrderIntoTransposeMappingMatrix(std::vector<Matrix::index_type>& input, MatrixHandle& output, Matrix::size_type nrows = 0);
};





// helper classes

class SparseElement {
public:
  Matrix::index_type     row;
  Matrix::index_type     col;
  double  val;
};

inline bool operator==(const SparseElement& s1,const SparseElement& s2)
{
  if ((s1.row == s2.row)&&(s1.col == s2.col)) return (true);
  return (false);
}    

inline bool operator<(const SparseElement& s1, const SparseElement& s2)
{
  if (s1.row < s2.row) return(true);
  if (s1.row == s2.row) if (s1.col < s2.col) return(true);
  return (false);
}


} // end SCIRun namespace
#endif
