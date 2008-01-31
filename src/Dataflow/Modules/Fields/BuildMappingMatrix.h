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


//    File   : BuildMappingMatrix.h
//    Author : Michael Callahan
//    Date   : Jan 2005

#if !defined(BuildMappingMatrix_h)
#define BuildMappingMatrix_h

#include <Core/Thread/Thread.h>
#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Geometry/BBox.h>
#include <sci_hash_map.h>
#include <sci_algorithm.h>
#include <float.h> // for DBL_MAX

//! For Windows
#include <Dataflow/Modules/Fields/share.h>

namespace SCIRun {

class SCISHARE BuildMappingMatrixAlgo : public DynamicAlgoBase
{
public:
  virtual MatrixHandle execute(ProgressReporter *reporter,
                               MeshHandle src, MeshHandle dst,
			       int interp_basis,
			       bool source_to_single_dest,
			       bool exhaustive_search,
			       double esearch_dist, int np) = 0;

  //! support the dynamically compiled algorithm concept
  static CompileInfoHandle get_compile_info(const TypeDescription *msrc,
					    const TypeDescription *lsrc,
					    const TypeDescription *mdst,
					    const TypeDescription *ldst,
					    const TypeDescription *fdst);

protected:

  static bool pair_less(const pair<int, float> &a, const pair<int, float> &b)
  {
    return a.first < b.first;
  }
};


template <class MSRC, class LSRC, class MDST, class LDST>
class BuildMappingMatrixAlgoT : public BuildMappingMatrixAlgo
{
public:
  //! virtual interface. 
  virtual MatrixHandle execute(ProgressReporter *reporter,
                               MeshHandle src, MeshHandle dst,
                               int interp_basis,
                               bool source_to_single_dest,
                               bool exhaustive_search,
                               double esearch_dist, int np);

private:

  typedef struct BIData__ {
    MeshHandle src_meshH;
    MeshHandle dst_meshH;
    int interp_basis;
    bool source_to_single_dest;
    bool exhaustive_search;
    double esearch_dist;
    Matrix::size_type sprocsize;
    Matrix::size_type dprocsize;

    Matrix::index_type *rowdata;
    vector<Matrix::index_type> *coldatav;
    vector<double> *datav;

    vector<Matrix::index_type> *dstmap;
    Mutex maplock;
  
    ProgressReporter *reporter;

    BIData__() :
      maplock("BuildInterp Map Lock")
    {}

  } BIData;

  double find_closest_src_loc(typename LSRC::index_type &index,
			      MSRC *mesh, const Point &p) const;
  double find_closest_dst_loc(typename LDST::index_type &index,
			      MDST *mesh, const Point &p) const;
  void parallel_execute(int proc, BIData *d);
};


// Returns dist * dist, don't bother to do sqrt.
template <class MSRC, class LSRC, class MDST, class LDST>
double
BuildMappingMatrixAlgoT<MSRC, LSRC, MDST, LDST>::find_closest_src_loc(typename LSRC::index_type &index, MSRC *mesh, const Point &p) const
{
  double mindist = DBL_MAX;

  typename LSRC::iterator itr, eitr;
  mesh->begin(itr);
  mesh->end(eitr);
  while (itr != eitr)
  {
    Point c;
    mesh->get_center(c, *itr);
    const double dist = (p - c).length2();
    if (dist < mindist)
    {
      mindist = dist;
      index = *itr;
    }
    ++itr;
  }
  return sqrt(mindist);
}


// Returns dist * dist, don't bother to do sqrt.
template <class MSRC, class LSRC, class MDST, class LDST>
double
BuildMappingMatrixAlgoT<MSRC, LSRC, MDST, LDST>::find_closest_dst_loc(typename LDST::index_type &index, MDST *mesh, const Point &p) const
{
  double mindist = DBL_MAX;
  
  typename LDST::iterator itr, eitr;
  mesh->begin(itr);
  mesh->end(eitr);
  while (itr != eitr)
  {
    Point c;
    mesh->get_center(c, *itr);
    const double dist = (p - c).length2();
    if (dist < mindist)
    {
      mindist = dist;
      index = *itr;
    }
    ++itr;
  }
  return mindist;
}


template <class MSRC, class LSRC, class MDST, class LDST>
MatrixHandle
BuildMappingMatrixAlgoT<MSRC, LSRC, MDST, 
		       LDST>::execute(ProgressReporter *reporter,
                                      MeshHandle src_meshH, 
				      MeshHandle dst_meshH, 
				      int interp_basis,
				      bool source_to_single_dest, 
				      bool exhaustive_search, 
				      double esearch_dist, 
				      int np)
{
  Matrix::index_type i, j;

  // Set up the data for the parallel run.
  BIData d;
  d.src_meshH = src_meshH;
  d.dst_meshH = dst_meshH;
  d.interp_basis = interp_basis;
  d.source_to_single_dest = source_to_single_dest;
  d.exhaustive_search = exhaustive_search;
  d.esearch_dist = esearch_dist;

  MSRC *src_mesh = dynamic_cast<MSRC *>(src_meshH.get_rep());
  MDST *dst_mesh = dynamic_cast<MDST *>(dst_meshH.get_rep());

  typename LSRC::size_type src_size0;
  src_mesh->size(src_size0);
  typename LDST::size_type dst_size0;
  dst_mesh->size(dst_size0);

  const Matrix::size_type src_size = static_cast<Matrix::size_type>(src_size0);
  const Matrix::size_type dst_size = static_cast<Matrix::size_type>(dst_size0);

  // Divide up the data amongst the processors in contiguous blocks.
  d.rowdata = scinew Matrix::index_type[(dst_size)+1];
  d.rowdata[0] = 0;
  d.coldatav = scinew vector<Matrix::index_type>[np];
  d.datav = scinew vector<double>[np];

  d.sprocsize = (src_size%np)?(src_size/np+1):(src_size / np);
  d.dprocsize = (dst_size%np)?(dst_size/np+1):(dst_size / np);

  d.reporter = reporter;
  if ((interp_basis == 0) && source_to_single_dest)
  {
    reporter->update_progress(0, src_size);
  }
  else
  {
    reporter->update_progress(0, dst_size);
  }

  if ((interp_basis == 0) && source_to_single_dest)
  {
    d.dstmap = scinew vector<Matrix::index_type>[dst_size];
  }

  Thread::parallel(this, 
                   &BuildMappingMatrixAlgoT<MSRC, LSRC, MDST, LDST>::parallel_execute,
                   np, &d);

  // Collect the data back into a sparse row matrix.
  // This is for source_to_single_dest.
  if ((interp_basis == 0) && source_to_single_dest)
  {
    for (i = 0; i < dst_size; i++)
    {
      const Matrix::index_type ds = static_cast<Matrix::index_type>(d.dstmap[i].size());
      d.rowdata[i+1] = d.rowdata[i] + ds;
    }
    const Matrix::index_type dsize = d.rowdata[i];

    Matrix::index_type *coldata = scinew Matrix::index_type[dsize];
    double *data = scinew double[dsize];
    Matrix::index_type c = 0;
    for (i = 0; i < dst_size; i++)
    {
      for (j = 0; j < d.dstmap[i].size(); j++)
      {
        coldata[c] = d.dstmap[i][j];
        data[c] = 1.0 / d.dstmap[i].size();
        ++c;
      }
    }

    delete [] d.dstmap;

    SparseRowMatrix *matrix =
      scinew SparseRowMatrix(dst_size, src_size,
			     d.rowdata, coldata, dsize, data);
    return matrix;
  }

  // Collect the data back into a sparse row matrix.
  // All but source_to_single_dest go through here.
  Matrix::size_type dsize = 0;
  for (j = 0; j < (Matrix::index_type)np; j++)
  {
    dsize += d.coldatav[j].size();
  }
  
  Matrix::index_type *coldata = scinew Matrix::index_type[dsize];
  double *data = scinew double[dsize];
  Matrix::size_type c = 0;
  for (j = 0; j < (Matrix::index_type)np; j++)
  {
    for (i = 0; i < d.coldatav[j].size(); i++)
    {
      coldata[c] = d.coldatav[j][i];
      data[c] = d.datav[j][i];
      ++c;
    }
  }

  Matrix::index_type off = d.rowdata[d.dprocsize];
  for (i = d.dprocsize+1; i < dst_size+1; i++)
  {
    d.rowdata[i] += off;
    if ((i % d.dprocsize) == 0) { off = d.rowdata[i]; }
  }

  delete [] d.coldatav;
  delete [] d.datav;

  SparseRowMatrix *matrix =
    scinew SparseRowMatrix(dst_size, src_size,
			   d.rowdata, coldata, dsize, data);

  return matrix;
}


template <class MSRC, class LSRC, class MDST, class LDST>
void
BuildMappingMatrixAlgoT<MSRC, LSRC, MDST, LDST>::parallel_execute(int proc,
                                                                  BIData *d)
{
  const int interp_basis = d->interp_basis;
  const bool source_to_single_dest = d->source_to_single_dest;
  const bool exhaustive_search = d->exhaustive_search;
  const double dist = d->esearch_dist;
  Matrix::index_type *rowdata = d->rowdata + proc * d->dprocsize;
  vector<Matrix::index_type> &coldatav = d->coldatav[proc];
  vector<double> &datav = d->datav[proc];

  MSRC *src_mesh = dynamic_cast<MSRC *>(d->src_meshH.get_rep());
  MDST *dst_mesh = dynamic_cast<MDST *>(d->dst_meshH.get_rep());
  BBox src_search_bbox=src_mesh->get_bounding_box();
  BBox dst_search_bbox=dst_mesh->get_bounding_box();
  src_mesh->synchronize(Mesh::LOCATE_E);
  dst_mesh->synchronize(Mesh::LOCATE_E);
  if (exhaustive_search && dist>0) {
    src_search_bbox.extend(src_search_bbox.min()-Vector(dist, dist, dist));
    src_search_bbox.extend(src_search_bbox.max()+Vector(dist, dist, dist));
    dst_search_bbox.extend(dst_search_bbox.min()-Vector(dist, dist, dist));
    dst_search_bbox.extend(dst_search_bbox.max()+Vector(dist, dist, dist));
  }

  Matrix::index_type count = 0;
  if ((interp_basis == 0) && source_to_single_dest)
  {
    // For each source location, we will map it to a single destination
    //   location.  This is different from our other interpolation
    //   methods in that here, many destination locations are likely to
    //   have no source location mapped to them at all.
    // Note: it is possible that multiple source will be mapped to the
    //   same destination, which is fine -- but we will flag it as a
    //   remark, just to let the user know.  
    // Also: if a source is outside of the destination volume,
    //   and the "exhaustive search" option is not selected,
    //   that source will not be mapped to any destination.
    typename LSRC::iterator itr, end_itr;
    src_mesh->begin(itr);
    src_mesh->end(end_itr);

    while (itr != end_itr)
    {
      if (count < proc * d->sprocsize)
      {
        ++itr;
        ++count;
        continue;
      }
      if (count >= (proc + 1) * d->sprocsize)
      {
        break;
      }
      d->reporter->increment_progress();

      typename LDST::array_type locs;
      double weights[MESH_WEIGHT_MAXSIZE];
      Point p;
      src_mesh->get_center(p, *itr);
      bool failed = true;
      const int nw = dst_mesh->get_weights(p, locs, weights);
      if (nw > 0)
      {
        failed = false;
        double max_weight=weights[0];
        Matrix::index_type max_idx=0;
        for (Matrix::index_type i=1; i<locs.size(); i++)
        {
          if (weights[i] > max_weight)
          {
            max_idx = i;
            max_weight = weights[i];
          }
        }
        Matrix::index_type uint_idx = static_cast<Matrix::index_type>(locs[max_idx]);
        d->maplock.lock();
        d->dstmap[uint_idx].push_back(static_cast<Matrix::index_type>(*itr));
        d->maplock.unlock();
      }
      if (exhaustive_search && failed)
      {
        if (dist <= 0 || dst_search_bbox.inside(p)) {
          typename LDST::index_type index;
          const double dd = find_closest_dst_loc(index, dst_mesh, p);
          if (dist <= 0 || dd < dist * dist)
            {
              Matrix::index_type uint_idx = index;
              d->maplock.lock();
              d->dstmap[uint_idx].push_back(static_cast<Matrix::index_type>(*itr));
              d->maplock.unlock();
            }
        }
      }
      ++itr;
      ++count;
    }
  }
  else
  { // linear (or constant, with each src mapping to many dests)
    typename LDST::iterator itr, end_itr;
    dst_mesh->begin(itr);
    dst_mesh->end(end_itr);
    const bool linear = (interp_basis == 1);
    Matrix::index_type rcount = 0;
    while (itr != end_itr)
    {
      if (count < proc * d->dprocsize)
      {
        ++itr;
        ++count;
        continue;
      }
      if (count >= (proc + 1) * d->dprocsize)
      {
        break;
      }
      d->reporter->increment_progress();

      typename LSRC::array_type locs;
      double weights[MESH_WEIGHT_MAXSIZE];
      Point p;
      dst_mesh->get_center(p, *itr);
      const int nw = src_mesh->get_weights(p, locs, weights);
      const Matrix::index_type lastrdata = rcount?rowdata[rcount]:0;
      rowdata[rcount+1] = lastrdata; // init in case we fall through
      if (nw > 0)
      {
        if (linear)
        {
          vector<pair<typename LSRC::index_type, double> > v;
          for (size_t i = 0; i < locs.size(); i++)
          {
            v.push_back(pair<typename LSRC::index_type, double>
            (locs[i], weights[i]));
          }

          rowdata[rcount+1] = lastrdata + v.size();
          std::sort(v.begin(), v.end(), pair_less);
          for (size_t i = 0; i < locs.size(); i++)
          {
            coldatav.push_back(static_cast<Matrix::index_type>(v[i].first));
            datav.push_back(v[i].second);
          }
        }
        else
        {
          double max_weight = weights[0];
          Matrix::index_type max_idx = 0;
          for (size_t i=1; i<locs.size(); i++)
          {
            if (weights[i] > max_weight)
            {
              max_idx = static_cast<Matrix::index_type>(i);
              max_weight = weights[i];
            }
          }
          rowdata[rcount+1] = lastrdata + 1;
          coldatav.push_back((Matrix::index_type)(locs[max_idx]));
          datav.push_back(1.0);
        }
      }
      else if (exhaustive_search)
      {
        if (dist <= 0 || dst_search_bbox.inside(p)) 
        {
          typename LSRC::index_type index;
          const double dd = find_closest_src_loc(index, src_mesh, p);
          if (dist <= 0 || dd < dist * dist)
            {
              rowdata[rcount+1] = lastrdata + 1;
              coldatav.push_back((Matrix::index_type)index);
              datav.push_back(1.0);
            }
        }
      }
      ++itr;
      ++count;
      ++rcount;
    }
  }
}


} // end namespace SCIRun

#endif // BuildMappingMatrix_h
