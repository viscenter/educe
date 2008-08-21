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

#include <Core/Algorithms/Fields/MarchingCubes/BaseMC.h>

namespace SCIRun {

MatrixHandle 
BaseMC::get_interpolant()
{
  if( !build_field_ ) return (0);

/*  if (basis_order_ == 0)
  {
    // The columns represent the source nodes while the rows represent
    // the destination nodes
    Matrix::size_type nrows = 0;
    Matrix::size_type ncols = nnodes_;
    
    Matrix::index_type i;

    // Count the src nodes that map to the dst nodes
    for (i=0; i<nnodes_; i++)	if( node_map_[i] != -1 ) ++nrows;

    // Yale Sparse Row Matrix format.
    Matrix::index_type *rr = new Matrix::index_type[nrows+1];
    Matrix::index_type *cc = new Matrix::index_type[nrows];
    double *dd = new double[nrows];

    Matrix::size_type nnz = nrows;
    
    for (i=0; i<nnodes_; i++)
    {
      // Find the src nodes that map to the dst nodes
      if( node_map_[i] != -1 )
      {
        Matrix::index_type row = node_map_[i];

        rr[row] = row;
        
        cc[row] = i;
        dd[row] = 1;
      }
    }

    rr[nrows] = nnz;

    return new SparseRowMatrix(nrows, ncols, rr, cc, nnz, dd);
  }
  else // if (basis_order_ == 1)
  {
*/  
    // The columns represent the source nodes while the rows
    // represent the destination nodes
    const Matrix::size_type nrows = static_cast<Matrix::size_type>(edge_map_.size());
    const Matrix::size_type ncols = nnodes_;
    
    //  Yale Sparse Row Matrix format.
    Matrix::index_type *rr = new Matrix::index_type[nrows+1];
    Matrix::index_type *cc = new Matrix::index_type[nrows*2];
    double *dd = new double[nrows*2];
    
    edge_hash_type::iterator eiter = edge_map_.begin();
    while (eiter != edge_map_.end())
    {
      const SCIRun::index_type ei = (*eiter).second;
      
      cc[ei * 2 + 0] = (*eiter).first.first;
      cc[ei * 2 + 1] = (*eiter).first.second;
      dd[ei * 2 + 0] = 1.0 - (*eiter).first.dfirst;
      dd[ei * 2 + 1] = (*eiter).first.dfirst;
          
      ++eiter;
    }

    Matrix::size_type nnz = 0;
    Matrix::index_type i;
    for (i = 0; i < nrows; i++)
    {
      rr[i] = nnz;
      if (cc[i * 2 + 0] >= 0)
      {
        cc[nnz] = cc[i * 2 + 0];
        dd[nnz] = dd[i * 2 + 0];
        nnz++;
      }
      if (cc[i * 2 + 1] >= 0)
      {
        cc[nnz] = cc[i * 2 + 1];
        dd[nnz] = dd[i * 2 + 1];
        nnz++;
      }
    }

    rr[i] = nnz;
  
    return (new SparseRowMatrix(nrows, ncols, rr, cc, nnz, dd));
//  }
}

  
MatrixHandle 
BaseMC::get_parent_cells()
{
  if( !build_field_ ) return (0);

/*  if (basis_order_ == 0)
  {
    // The columns represent the source nodes while the rows
    // represent the destination nodes
    const Matrix::size_type nrows = static_cast<Matrix::size_type>(edge_map_.size());
    const Matrix::size_type ncols = ncells_;
    
    //  Yale Sparse Row Matrix format.
    Matrix::index_type *rr = new Matrix::index_type[nrows+1];
    Matrix::index_type *cc = new Matrix::index_type[nrows*2];
    double *dd = new double[nrows*2];
    
    edge_hash_type::iterator eiter = edge_map_.begin();
    while (eiter != edge_map_.end())
    {
      const SCIRun::index_type ei = (*eiter).second;
      
      cc[ei * 2 + 0] = (*eiter).first.first;
      cc[ei * 2 + 1] = (*eiter).first.second;
      dd[ei * 2 + 0] = 1.0 - (*eiter).first.dfirst;
      dd[ei * 2 + 1] = (*eiter).first.dfirst;
          
      ++eiter;
    }

    Matrix::size_type nnz = 0;
    Matrix::index_type i;
    for (i = 0; i < nrows; i++)
    {
      rr[i] = nnz;
      if (cc[i * 2 + 0] >= 0)
      {
        cc[nnz] = cc[i * 2 + 0];
        dd[nnz] = dd[i * 2 + 0];
        nnz++;
      }
      if (cc[i * 2 + 1] >= 0)
      {
        cc[nnz] = cc[i * 2 + 1];
        dd[nnz] = dd[i * 2 + 1];
        nnz++;
      }
    }

    rr[i] = nnz;
  
    return (new SparseRowMatrix(nrows, ncols, rr, cc, nnz, dd));
  }
  else //if (basis_order_ == 1)
  { */
    // The columns represent the source cells while the rows
    // represent the destination cells
    const Matrix::size_type nrows = cell_map_.size();
    const Matrix::size_type ncols = ncells_;

    // Yale Sparse Row Matrix format.
    Matrix::index_type *rr = new Matrix::index_type[nrows+1];
    Matrix::index_type *cc = new Matrix::index_type[nrows];
    double *dd = new double[nrows];

    Matrix::size_type nnz = nrows;
    Matrix::index_type i;

    for (i = 0; i < nrows; i++)
    {
      rr[i] = i;

      cc[i] = cell_map_[i];
      dd[i] = 1;
    }

    rr[i] = nrows;

    return (new SparseRowMatrix(nrows, ncols, rr, cc, nnz, dd));
//  }

}

} //end namespace
