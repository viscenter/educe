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
 *  SearchGrid.h: Specialized compact regular mesh used for searching.
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   August 2004
 *
 *  Copyright (C) 2004 SCI Group
 *
 */

#ifndef SCI_project_SearchGrid_h
#define SCI_project_SearchGrid_h 1

#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Types.h>
#include <Core/Geometry/BBox.h>
#include <Core/Geometry/Transform.h>
#include <list>
#include <vector>
#include <set>

#include <Core/Datatypes/share.h>

namespace SCIRun {

class SCISHARE SearchGridBase
{
public:
  typedef SCIRun::index_type  index_type;
  typedef SCIRun::size_type   size_type;

  SearchGridBase(size_type x, size_type y, size_type z,
                 const Point &min, const Point &max);

  SearchGridBase(size_type x, size_type y, size_type z, const Transform &t);

  virtual ~SearchGridBase();
  
  //! get the mesh statistics
  inline size_type get_ni() const 
    { return ni_; }
  inline size_type get_nj() const 
    { return nj_; }
  inline size_type get_nk() const 
    { return nk_; }

  inline void transform(const Transform &t) 
    { transform_.pre_trans(t);}

  inline const Transform &get_transform() const 
    { return transform_; }
  
  void get_canonical_transform(Transform &t);
  
  inline Transform &set_transform(const Transform &trans) 
    { transform_ = trans; return transform_; }

  inline bool locate(index_type &i, index_type &j, 
                     index_type &k, const Point &) const;
                     
  inline void unsafe_locate(index_type &i, index_type &j, 
                            index_type &k, const Point &) const;

protected:

  index_type linearize(index_type i, index_type j, index_type k) const
  {
    // k inner loops
    return ((i * nj_) + j) * nk_ + k;
  }

  index_type ni_, nj_, nk_;

  Transform transform_;
};


bool
SearchGridBase::locate(index_type &i, index_type &j, 
                       index_type &k, const Point &p) const
{
  const Point r = transform_.unproject(p);
  
  const double rx = floor(r.x());
  const double ry = floor(r.y());
  const double rz = floor(r.z());

  // Clamp in double space to avoid overflow errors.
  if (rx < 0.0      || ry < 0.0      || rz < 0.0    ||
      rx >= ni_     || ry >= nj_     || rz >= nk_   )
  {
    return false;
  }

  i = static_cast<index_type>(rx);
  j = static_cast<index_type>(ry);
  k = static_cast<index_type>(rz);
  return (true);
}


void
SearchGridBase::unsafe_locate(index_type &i, index_type &j,
                              index_type &k, const Point &p) const
{
  Point r;
  transform_.unproject(p, r);
  
  i = static_cast<index_type>(r.x());
  j = static_cast<index_type>(r.y());
  k = static_cast<index_type>(r.z());
}




class SCISHARE SearchGridConstructor : public Datatype, public SearchGridBase
{
  friend class SearchGrid;
  
public:
  typedef SCIRun::index_type  index_type;
  typedef SCIRun::size_type   size_type;

  SearchGridConstructor(size_type x, size_type y, size_type z,
                        const Point &min, const Point &max);
  virtual ~SearchGridConstructor();

  void insert(index_type val, const BBox &bbox);
  void remove(index_type val, const BBox &bbox);
  
  bool lookup(const std::list<index_type> *&candidates, const Point &p) const;
  void lookup_ijk(const std::list<index_type> *&candidates,
                  size_type i, size_type j, size_type k) const;
                  
  void lookup_bbox(std::set<index_type> &candidates, const BBox &bbox);
  double min_distance_squared(const Point &p, size_type i, 
                              size_type j, size_type k) const;

  virtual void io(Piostream&) {}

protected:
  std::vector<std::list<index_type> > bin_;
  size_type size_;
};



class SCISHARE SearchGrid : public Datatype, public SearchGridBase
{
public:
  typedef SCIRun::index_type  index_type;
  typedef SCIRun::size_type   size_type;

  SearchGrid(const SearchGridConstructor &c);
  SearchGrid(const SearchGrid &c);
  virtual ~SearchGrid();

  inline bool lookup(index_type **begin, index_type **end, const Point &p) const;
  inline void lookup(index_type **begin, index_type **end, const index_type idx) const;
  inline void lookup_bbox(std::vector<index_type>& indices, const BBox &bbox) const;
  inline bool lookup_point(index_type& index, const Point &point) const;

  virtual void io(Piostream&);
  static PersistentTypeID type_id;

protected:
  SearchGrid();

  std::vector<index_type> accum_;
  index_type *vals_;
  size_type   vals_size_;

  // Returns a SearchGrid
  static Persistent *maker() { return new SearchGrid(); }
};


// Inline functions for added speed

bool
SearchGrid::lookup(index_type **begin, index_type **end, const Point &p) const
{
  index_type i, j, k;
  if (locate(i, j, k, p))
  {
    const index_type index = linearize(i, j, k);
    *begin = vals_ + accum_[index];
    *end = vals_ + accum_[index+1];
    return true;
  }
  return false;
}

void
SearchGrid::lookup(index_type **begin, index_type **end, index_type idx) const
{
  *begin = vals_ + accum_[idx];
  *end = vals_ + accum_[idx+1];
}

void
SearchGrid::lookup_bbox(std::vector<index_type> &indices,
                                   const BBox &bbox) const
{
  index_type mini, minj, mink, maxi, maxj, maxk;

  locate(mini, minj, mink, bbox.min());
  locate(maxi, maxj, maxk, bbox.max());

  indices.resize((maxi-mini+1)*(maxj-minj+1)*(maxk-mink+1));

  index_type p=0;
  for (index_type i = mini; i <= maxi; i++)
  {
    for (index_type j = minj; j <= maxj; j++)
    {
      for (index_type k = mink; k <= maxk; k++)
      {
        indices[p] = linearize(i,j,k);
      }
    }
  }
}

bool
SearchGrid::lookup_point(index_type& index, const Point &p) const
{
  index_type i, j, k;
  if (locate(i, j, k, p))
  {
    index = linearize(i, j, k);
    return (true);
  }
  return (false);
}



} // namespace SCIRun

#endif // SCI_project_SearchGrid_h
