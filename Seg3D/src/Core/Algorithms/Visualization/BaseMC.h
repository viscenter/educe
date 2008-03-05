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
 *  BaseMC.h
 *
 *  Allen R. Sanderson
 *  SCI Institute
 *  University of Utah
 *  Feb 2008
 *
 *  Copyright (C) 2008 SCI Institute
 */


#ifndef BaseMC_h
#define BaseMC_h

#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Field.h>
#include <Core/Geom/GeomObj.h>

#include <Core/Util/TypeDescription.h>

namespace SCIRun {

class BaseMC
{
public:

  BaseMC() : build_field_(0), build_geom_(0), basis_order_(-1),
	     nnodes_(0), ncells_(0), geomHandle_(0) {}

  virtual ~BaseMC() {}

public:
  virtual void reset( int,
		      bool build_field,
		      bool build_geom,
		      bool transparency ) = 0;
  virtual FieldHandle get_field(double val) = 0;
  GeomHandle get_geom() { return geomHandle_; }

  MatrixHandle get_interpolant()
  {
    if( !build_field_ )
      return 0;

    if (basis_order_ == 0)
    {
      // The columns represent the source nodes while the rows represent
      // the destination nodes
      Matrix::size_type nrows = 0;
      Matrix::size_type ncols = nnodes_;
      
      Matrix::index_type i;

      // Count the src nodes that map to the dst nodes
      for (i=0; i<nnodes_; i++)
	if( node_map_[i] != -1 ) ++nrows;

      // Yale Sparse Row Matrix format.
      Matrix::index_type *rr = scinew Matrix::index_type[nrows+1];
      Matrix::index_type *cc = scinew Matrix::index_type[nrows];
      double *dd = scinew double[nrows];

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

      return scinew SparseRowMatrix(nrows, ncols, rr, cc, nnz, dd);
    }
    else // if (basis_order_ == 1)
    {
      // The columns represent the source nodes while the rows
      // represent the destination nodes
      const Matrix::size_type nrows = static_cast<Matrix::size_type>(edge_map_.size());
      const Matrix::size_type ncols = nnodes_;
      
      //  Yale Sparse Row Matrix format.
      Matrix::index_type *rr = scinew Matrix::index_type[nrows+1];
      Matrix::index_type *cc = scinew Matrix::index_type[nrows*2];
      double *dd = scinew double[nrows*2];
      
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
    
      return scinew SparseRowMatrix(nrows, ncols, rr, cc, nnz, dd);
    }
  };
     
  MatrixHandle get_parent_cells()
  {
    if( !build_field_ )
      return 0;

    if (basis_order_ == 0)
    {
      // The columns represent the source nodes while the rows
      // represent the destination nodes
      const Matrix::size_type nrows = static_cast<Matrix::size_type>(edge_map_.size());
      const Matrix::size_type ncols = ncells_;
      
      //  Yale Sparse Row Matrix format.
      Matrix::index_type *rr = scinew Matrix::index_type[nrows+1];
      Matrix::index_type *cc = scinew Matrix::index_type[nrows*2];
      double *dd = scinew double[nrows*2];
      
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
    
      return scinew SparseRowMatrix(nrows, ncols, rr, cc, nnz, dd);
    }
    else //if (basis_order_ == 1)
    {
      // The columns represent the source cells while the rows
      // represent the destination cells
      const Matrix::size_type nrows = cell_map_.size();
      const Matrix::size_type ncols = ncells_;

      // Yale Sparse Row Matrix format.
      Matrix::index_type *rr = scinew Matrix::index_type[nrows+1];
      Matrix::index_type *cc = scinew Matrix::index_type[nrows];
      double *dd = scinew double[nrows];

      Matrix::size_type nnz = nrows;
      Matrix::index_type i;

      for (i = 0; i < nrows; i++)
      {
	rr[i] = i;

	cc[i] = cell_map_[i];
	dd[i] = 1;
      }

      rr[i] = nrows;

      return scinew SparseRowMatrix(nrows, ncols, rr, cc, nnz, dd);
    }

  };

protected:

  struct edgepair_t
  {
    SCIRun::index_type first;
    SCIRun::index_type second;
    double dfirst;
  };

  struct edgepairless
  {
    bool operator()(const edgepair_t &a, const edgepair_t &b) const
    {
      return less(a,b);
    }
    static bool less(const edgepair_t &a, const edgepair_t &b)
    {
      return a.first < b.first || (a.first == b.first && a.second < b.second);
    }
  };

#ifdef HAVE_HASH_MAP
  struct edgepairequal
  {
    bool operator()(const edgepair_t &a, const edgepair_t &b) const
    {
      return a.first == b.first && a.second == b.second;
    }
  };

  struct edgepairhash
  {
    unsigned int operator()(const edgepair_t &a) const
    {
#if defined(__ECC) || defined(_MSC_VER)
      hash_compare<unsigned int> h;
#else
      hash<unsigned int> h;
#endif
      return h(a.first ^ a.second);
    }
# if defined(__ECC) || defined(_MSC_VER)

      // These are particularly needed by ICC's hash stuff
      static const size_t bucket_size = 4;
      static const size_t min_buckets = 8;
      
      // This is a less than function.
      bool operator()(const edgepair_t & a, const edgepair_t & b) const {
        return edgepairless::less(a,b);
      }
# endif // endif ifdef __ICC
  };

# if defined(__ECC) || defined(_MSC_VER)
  typedef hash_map<edgepair_t, SCIRun::index_type, edgepairhash> edge_hash_type;
#else
  typedef hash_map<edgepair_t,
		   SCIRun::index_type,
		   edgepairhash,
		   edgepairequal> edge_hash_type;
#endif // !defined(__ECC) && !defined(_MSC_VER)
  
#else
  typedef map<edgepair_t,
	      SCIRun::index_type,
	      edgepairless> edge_hash_type;
#endif

  edge_hash_type             edge_map_;  // Unique edge cuts when surfacing node data
  vector<SCIRun::index_type> cell_map_;  // Unique cells when surfacing node data.
  vector<SCIRun::index_type> node_map_;  // Unique nodes when surfacing cell data.

  bool build_field_;
  bool build_geom_;
  int basis_order_;

  SCIRun::size_type nnodes_;
  SCIRun::size_type ncells_;

  GeomHandle geomHandle_;
};

     
} // End namespace SCIRun

#endif // BaseMC_H
