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


//    File   : MapFieldDataFromSourceToDestination.h
//    Author : Michael Callahan
//    Date   : June 2001

#if !defined(MapFieldDataFromSourceToDestination_h)
#define MapFieldDataFromSourceToDestination_h

#include <Core/Thread/Barrier.h>
#include <Core/Thread/Thread.h>
#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <Core/Geometry/BBox.h>
#include <sci_hash_map.h>
#include <float.h> // for DBL_MAX

//! for Windows support
#include <Dataflow/Modules/Fields/share.h>

namespace SCIRun {

class SCISHARE MapFieldDataFromSourceToDestinationAlgo : public DynamicAlgoBase
{
public:
  virtual FieldHandle execute(ProgressReporter *reporter,
                              FieldHandle src, MeshHandle dst,
			      int basis_order,
			      const string &basis, bool source_to_single_dest,
			      bool exhaustive_search, double dist, int np) = 0;

  //! support the dynamically compiled algorithm concept
  static CompileInfoHandle get_compile_info(const TypeDescription *fsrc,
					    const TypeDescription *lsrc,
					    const TypeDescription *fdst,
					    const TypeDescription *ldst,
					    const string &fout);
};


template <class FSRC, class LSRC, class FOUT, class LDST>
class MapFieldDataFromSourceToDestinationAlgoT : public MapFieldDataFromSourceToDestinationAlgo
{
  typedef pair<typename LDST::index_type, 
    vector<typename LSRC::index_type> > dst_src_pair;
    
  struct IndexHash {
    //! These are needed by the hash_map particularly
    // ANSI C++ allows us to initialize these variables in the
    // declaration.  However there may be compilers which will complain
    // about it.
    static const size_t bucket_size = 4;
    static const size_t min_buckets = 8;

    //! This is the hash function
    size_t operator()(const Field::index_type &f) const {
      return (static_cast<size_t>(f));
    }

    //!  This should return less than rather than equal to.
    bool operator()(const Field::index_type f1, const Field::index_type f2) const {
      return f1 < f2;
    }
  };    
    
    
#ifdef HAVE_HASH_MAP
#  if defined(__ECC) || defined(_MSC_VER)
  typedef hash_map<Field::index_type,
		   dst_src_pair,IndexHash> hash_type;
#  else
  typedef hash_map<Field::index_type,
		   dst_src_pair,
		   IndexHash > hash_type;
#  endif
#else
  typedef map<Field::index_type,
	      dst_src_pair,
	      IndexHash > hash_type;
#endif

  typedef struct _DIData 
  {
    FieldHandle src_fieldH;
    MeshHandle dst_meshH;
    int basis_order;
    string basis;
    bool source_to_single_dest;
    bool exhaustive_search;
    double dist;
    int np;
    FieldHandle out_fieldH;
    hash_type dstmap;
    Barrier barrier;
    Mutex maplock;
    ProgressReporter *reporter;
    
    _DIData() : barrier("MapFieldDataFromSourceToDestination Barrier"), maplock("MapFieldDataFromSourceToDestination Map Lock") {}
  } DIData;

public:
  //! virtual interface. 
  virtual FieldHandle execute(ProgressReporter *reporter,
                              FieldHandle src, MeshHandle dst,
			      int basis_order,
			      const string &basis, bool source_to_single_dest,
			      bool exhaustive_search, double dist, int np);

private:
  double find_closest_src_loc(typename LSRC::index_type &index,
			      typename FSRC::mesh_type *mesh, 
			      const Point &p) const;
  double find_closest_dst_loc(typename LDST::index_type &index,
			      typename FOUT::mesh_type *mesh, 
			      const Point &p) const;
  void parallel_execute(int proc, DIData *d);
};

template <class FSRC, class LSRC, class FOUT, class LDST>
double
MapFieldDataFromSourceToDestinationAlgoT<FSRC, LSRC, FOUT, 
                   LDST>::find_closest_src_loc(typename LSRC::index_type &index,
                                               typename FSRC::mesh_type *mesh, 
                                               const Point &p) const
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

template <class FSRC, class LSRC, class FOUT, class LDST>
double
MapFieldDataFromSourceToDestinationAlgoT<FSRC, LSRC, FOUT, LDST>::find_closest_dst_loc(typename LDST::index_type &index, typename FOUT::mesh_type *mesh, const Point &p) const
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

  return sqrt(mindist);
}

template <class FSRC, class LSRC, class FOUT, class LDST>
FieldHandle
MapFieldDataFromSourceToDestinationAlgoT<FSRC, LSRC, 
                   FOUT, LDST>::execute(ProgressReporter *reporter,
                                        FieldHandle src_fieldH, 
                                        MeshHandle dst_meshH, 
                                        int basis_order, 
                                        const string &basis, 
                                        bool source_to_single_dest, 
                                        bool exhaustive_search, 
                                        double dist, 
                                        int np)
{
  DIData d;
  d.src_fieldH=src_fieldH;
  d.dst_meshH=dst_meshH;
  d.basis_order=basis_order;
  d.basis=basis;
  d.source_to_single_dest=source_to_single_dest;
  d.exhaustive_search=exhaustive_search;
  d.dist=dist;
  d.np=np;
  d.reporter = reporter;

  if ((basis == "constant") && source_to_single_dest)
  {
    typename FSRC::mesh_type *src_mesh =
      dynamic_cast<typename FSRC::mesh_type *>(src_fieldH->mesh().get_rep());
    typename LSRC::size_type src_size0;
    src_mesh->size(src_size0);
    const Field::size_type src_size = static_cast<Field::size_type>(src_size0);
    reporter->update_progress(0, src_size);
  }
  else
  {
    typename FOUT::mesh_type *dst_mesh =
      dynamic_cast<typename FOUT::mesh_type *>(dst_meshH.get_rep());
    typename LDST::size_type dst_size0;
    dst_mesh->size(dst_size0);
    const Field::size_type dst_size = static_cast<Field::size_type>(dst_size0);
    reporter->update_progress(0, dst_size);
  }

  typename FOUT::mesh_type *dst_mesh = 
    dynamic_cast<typename FOUT::mesh_type *>(dst_meshH.get_rep());
  FOUT *out_field = scinew FOUT(dst_mesh);  
  d.out_fieldH = out_field;

  Thread::parallel(this, 
                   &MapFieldDataFromSourceToDestinationAlgoT<FSRC, LSRC, FOUT, LDST>::parallel_execute,
                   np, &d);

  return out_field;
}

template <class FSRC, class LSRC, class FOUT, class LDST>
void
MapFieldDataFromSourceToDestinationAlgoT<FSRC, LSRC, FOUT, LDST>::parallel_execute(int proc,
                                                             DIData *d)
{
  FieldHandle src_fieldH = d->src_fieldH;
  MeshHandle dst_meshH = d->dst_meshH;
  int basis_order = d->basis_order;
  const string& basis = d->basis;
  bool source_to_single_dest = d->source_to_single_dest;
  bool exhaustive_search = d->exhaustive_search;
  double dist = d->dist;
  int np = d->np;
  FieldHandle out_fieldH = d->out_fieldH;
  
  FSRC *src_field = dynamic_cast<FSRC *>(src_fieldH.get_rep());
  typename FSRC::mesh_type *src_mesh = src_field->get_typed_mesh().get_rep();
  typename FOUT::mesh_type *dst_mesh = 
    dynamic_cast<typename FOUT::mesh_type *>(dst_meshH.get_rep());
  FOUT *out_field = dynamic_cast<FOUT *>(out_fieldH.get_rep());

  src_mesh->synchronize(Mesh::LOCATE_E);
  dst_mesh->synchronize(Mesh::LOCATE_E);
  BBox src_search_bbox, dst_search_bbox;
  if (proc == 0) 
  {
    if (basis_order > 0) 
    {
      src_mesh->synchronize(Mesh::NODES_E);
    } 
    else if (basis_order == 0 && src_mesh->dimensionality() == 3) 
    {
      src_mesh->synchronize(Mesh::CELLS_E);
    } 
    else if (basis_order == 0 && src_mesh->dimensionality() == 2) 
    {
      src_mesh->synchronize(Mesh::FACES_E);
    } 
    else if (basis_order == 0 && src_mesh->dimensionality() == 1) 
    {
      src_mesh->synchronize(Mesh::EDGES_E);
    }
    src_search_bbox=src_mesh->get_bounding_box();
    dst_search_bbox=dst_mesh->get_bounding_box();
    if (exhaustive_search && dist>0) 
    {
      src_search_bbox.extend(src_search_bbox.min()-Vector(dist, dist, dist));
      src_search_bbox.extend(src_search_bbox.max()+Vector(dist, dist, dist));
      dst_search_bbox.extend(dst_search_bbox.min()-Vector(dist, dist, dist));
      dst_search_bbox.extend(dst_search_bbox.max()+Vector(dist, dist, dist));
    }
  }

  d->barrier.wait(np);
  int count=0;

  if ((basis == "constant") && source_to_single_dest) 
  {
    // For each source location, we will map it to a single destination
    //   location.  This is different from our other interpolation
    //   methods in that here, many destination locations are likely to
    //   have no source location mapped to them at all.
    // Note: it is possible that multiple sources will be mapped to the
    //   same destination, which is fine -- but we will flag it as a
    //   remark, just to let the user know.  
    // Also: if a source is outside of the destination volume,
    //   and the "exhaustive search" option is not selected,
    //   that source will not be mapped to any destination.
    if (proc == 0) 
    {
      typename LDST::iterator itr, end_itr;
      dst_mesh->begin(itr);
      dst_mesh->end(end_itr);
      typename FOUT::value_type val(0);
      while (itr != end_itr) 
      {
        out_field->set_value(val, *itr);
        ++itr;
      }
    }
    
    d->barrier.wait(np);

    typename LSRC::iterator itr, end_itr;
    src_mesh->begin(itr);
    src_mesh->end(end_itr);
    typename LDST::size_type sz;
    dst_mesh->size(sz);

    while (itr != end_itr) 
    {
      if (count%np != proc) 
      {
        ++itr;
        ++count;
        continue;
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
        int max_idx=0;
        for (size_t i=1; i<locs.size(); i++) 
        {
          if (weights[i] > max_weight) 
          {
            max_idx = i;
            max_weight = weights[i];
          }
        }
        Field::index_type uint_idx = static_cast<Field::index_type>(locs[max_idx]);
        d->maplock.lock();
        typename hash_type::iterator dst_iter = d->dstmap.find(uint_idx);
        if (dst_iter != d->dstmap.end()) 
        {
          dst_iter->second.second.push_back(*itr);
        } 
        else 
        {
          vector<typename LSRC::index_type> v;
          v.push_back(*itr);
          d->dstmap[uint_idx] = dst_src_pair(locs[max_idx], v);
        }
        d->maplock.unlock();
      }
      if (exhaustive_search && failed) 
      {
        if (dist<=0 || dst_search_bbox.inside(p)) 
        {
          typename LDST::index_type index;
          double dd=find_closest_dst_loc(index, dst_mesh, p);
          if (dist<=0 || dd<dist) {
            Field::index_type uint_idx = static_cast<Field::index_type>(index);
            d->maplock.lock();
            typename hash_type::iterator dst_iter = d->dstmap.find(uint_idx);
            if (dst_iter != d->dstmap.end()) 
            {
              dst_iter->second.second.push_back(*itr);
            } 
            else 
            {
              vector<typename LSRC::index_type> v;
              v.push_back(*itr);
              d->dstmap[uint_idx] = dst_src_pair(index, v);
            }
            d->maplock.unlock();
          }
        }
      }
      ++itr;
      ++count;
    }
        
    d->barrier.wait(np);
    typename hash_type::iterator dst_iter = d->dstmap.begin();
    count=0;
    while (dst_iter != d->dstmap.end()) 
    {
      if (count%np != proc) 
      {
        ++dst_iter;
        ++count;
        continue;
      }
      vector<pair<typename LSRC::index_type, double> > v;
      Field::size_type n = static_cast<Field::size_type>(dst_iter->second.second.size());
      double invn=1./((double)n);
      if (n) 
      {
        typename FOUT::value_type val =
          (typename FOUT::value_type)(src_field->value(dst_iter->second.second[0])*invn);
        for (Field::index_type i=1; i<n; i++) 
        {
          val += 
            (typename FOUT::value_type)(src_field->value(dst_iter->second.second[i])*invn);
        }
        out_field->set_value(val, dst_iter->second.first);
      }
      ++dst_iter;
      ++count;
    }
  } 
  else 
  { // linear (or constant, with each src mapping to many dests)

    typename LDST::iterator itr, end_itr;
    dst_mesh->begin(itr);
    dst_mesh->end(end_itr);
    bool linear(basis == "linear");

    while (itr != end_itr) 
    {
      if (count%np != proc) 
      {
        ++itr;
        ++count;
        continue;
      }
      d->reporter->increment_progress();

      typename LSRC::array_type locs;
      double weights[MESH_WEIGHT_MAXSIZE];
      Point p;
      dst_mesh->get_center(p, *itr);
      bool failed = true;
      const int nw = src_mesh->get_weights(p, locs, weights);
      typename FOUT::value_type val;

      if (nw > 0)	
      {
        failed = false;
        if (linear) 
        {
          if (locs.size())
            val = (typename FOUT::value_type)(src_field->value(locs[0])*weights[0]);
          for (size_t i = 1; i < locs.size(); i++) {
            val +=(typename FOUT::value_type)(src_field->value(locs[i])*weights[i]);
          }
        } 
        else 
        {
          double max_weight=weights[0];
          Field::index_type max_idx=0;
          for (Field::index_type i=1; i<static_cast<Field::index_type>(locs.size()); i++) 
          {
            if (weights[i] > max_weight) 
            {
              max_idx = i;
              max_weight = weights[i];
            }
          }
          val = (typename FOUT::value_type)(src_field->value(locs[max_idx]));
        }
      }

      if (exhaustive_search && failed) 
      {
        if (dist<=0 || src_search_bbox.inside(p)) 
        {
          typename LSRC::index_type index;
          double d=find_closest_src_loc(index, src_mesh, p);
          if (dist<=0 || d<dist)
            {
              failed = false;
              val = (typename FOUT::value_type)(src_field->value(index));
            }
        }
      }
      if (failed) val = typename FOUT::value_type(0);
      out_field->set_value(val, *itr);
      ++itr;
      ++count;

    }
  }
}


} // end namespace SCIRun

#endif // MapFieldDataFromSourceToDestination_h
