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
 *  SearchGrid.cc: Templated Mesh defined on a 3D Regular Grid
 *
 *  Written by:
 *   Michael Callahan &&
 *   Chris Moulding
 *   Department of Computer Science
 *   University of Utah
 *   January 2001
 *
 *  Copyright (C) 2001 SCI Group
 *
 */

#include <Core/Datatypes/SearchGrid.h>
#include <Core/Geometry/BBox.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Math/MusilRNG.h>
#include <Core/Math/MiscMath.h>
#include <vector>
#include <iostream>

namespace SCIRun {

using namespace std;


SearchGridBase::~SearchGridBase()
{
}


SearchGridBase::SearchGridBase(size_type x, size_type y, size_type z,
                               const Point &min, const Point &max)
  : ni_(x), nj_(y), nk_(z)
{
  transform_.pre_scale(Vector(1.0 / x, 1.0 / y, 1.0 / z));
  transform_.pre_scale(max - min);

  transform_.pre_translate(min.asVector());
  transform_.compute_imat();
}

SearchGridBase::SearchGridBase(size_type x, size_type y, size_type z,
                               const Transform &t)
  : ni_(x), nj_(y), nk_(z), transform_(t)
{
}

void
SearchGridBase::get_canonical_transform(Transform &t) 
{
  t = transform_;
  t.post_scale(Vector(ni_, nj_, nk_));
}


SearchGridConstructor::SearchGridConstructor(size_type x,
                                             size_type y,
                                             size_type z,
                                             const Point &min,
                                             const Point &max)
  : SearchGridBase(x, y, z, min, max), size_(0)
{
  bin_.resize(x * y * z);
}


SearchGridConstructor::~SearchGridConstructor()
{
}


void
SearchGridConstructor::insert(index_type val, const BBox &bbox)
{
  index_type mini, minj, mink, maxi, maxj, maxk;

  unsafe_locate(mini, minj, mink, bbox.min());
  unsafe_locate(maxi, maxj, maxk, bbox.max());

  for (index_type i = mini; i <= maxi; i++)
  {
    for (index_type j = minj; j <= maxj; j++)
    {
      for (index_type k = mink; k <= maxk; k++)
      {
        bin_[linearize(i, j, k)].push_back(val);
        size_++;
      }
    }
  }
}


void
SearchGridConstructor::remove(index_type val, const BBox &bbox)
{
  index_type mini, minj, mink, maxi, maxj, maxk;

  unsafe_locate(mini, minj, mink, bbox.min());
  unsafe_locate(maxi, maxj, maxk, bbox.max());

  for (index_type i = mini; i <= maxi; i++)
  {
    for (index_type j = minj; j <= maxj; j++)
    {
      for (index_type k = mink; k <= maxk; k++)
      {
        bin_[linearize(i, j, k)].remove(val);
        size_++;
      }
    }
  }
}


bool
SearchGridConstructor::lookup(const list<index_type> *&candidates,
                              const Point &p) const
{
  index_type i, j, k;
  if (locate(i, j, k, p))
  {
    candidates = &(bin_[linearize(i, j, k)]);
    return true;
  }
  return false;
}


void
SearchGridConstructor::lookup_ijk(const list<index_type> *&candidates,
                                  index_type i, index_type j, index_type k) const
{
  candidates = &(bin_[linearize(i, j, k)]);
}


void
SearchGridConstructor::lookup_bbox(std::set<index_type> &candidates,
                                   const BBox &bbox)
{
  index_type mini, minj, mink, maxi, maxj, maxk;

  locate(mini, minj, mink, bbox.min());
  locate(maxi, maxj, maxk, bbox.max());

  for (index_type i = mini; i <= maxi; i++)
  {
    for (index_type j = minj; j <= maxj; j++)
    {
      for (index_type k = mink; k <= maxk; k++)
      {
        const std::list<index_type> &stuff = bin_[linearize(i, j, k)];
        std::list<index_type>::const_iterator itr = stuff.begin();
        while (itr != stuff.end())
        {
          candidates.insert(*itr);
          ++itr;
        }
      }
    }
  }
}


double
SearchGridConstructor::min_distance_squared(const Point &p,
                                            index_type i, index_type j, 
                                            index_type k) const
{
  Point r;
  transform_.unproject(p, r);

  // Splat the point onto the cell.
  if (r.x() < i) { r.x(i); }
  else if (r.x() > i+1) { r.x(i+1); }

  if (r.y() < j) { r.y(j); }
  else if (r.y() > j+1) { r.y(j+1); }

  if (r.z() < k) { r.z(k); }
  else if (r.z() > k+1) { r.z(k+1); }
  
  // Project the cell intersection back to world space.
  Point q;
  transform_.project(r, q);
  
  // Return distance from point to projected cell point.
  return (p - q).length2();
}


PersistentTypeID SearchGrid::type_id("SearchGrid", "Datatype", maker);


SearchGrid::SearchGrid()
  : SearchGridBase(1, 1, 1, Point(0.0, 0.0, 0.0), Point(1.0, 1.0, 1.0)),
    vals_(0),
    vals_size_(0)
{
}


SearchGrid::SearchGrid(const SearchGridConstructor &c)
  : SearchGridBase(c.ni_, c.nj_, c.nk_, c.transform_)
{
  accum_.resize(ni_ * nj_ * nk_ + 1);
  vals_size_ = c.size_;
  vals_ = new index_type[vals_size_];
  

  index_type counter = 0;
  accum_[0] = 0;
  for (index_type i = 0; i < ni_; i++)
  {
    for (index_type j = 0; j < nj_; j++)
    {
      for (index_type k = 0; k < nk_; k++)
      {
        // NOTE: Sort by size so more likely to get hit is checked first?
        // NOTE: Quick testing showed a 3% performance gain in heavy
        // search/build ratio test.  Build time goes up.  Also makes
        // edge tests less consistent.  We currently pick lowest
        // element index on an between-element hit because of the way
        // these things are built.
        list<index_type>::const_iterator itr = c.bin_[counter].begin();
        size_type size = 0;
        while (itr != c.bin_[counter].end())
        {
          vals_[accum_[counter] + size] = *itr;
          size++;
          ++itr;
        }
        accum_[counter+1] = accum_[counter] + size;
        counter++;
      }
    }
  }
}


SearchGrid::SearchGrid(const SearchGrid &c)
  : SearchGridBase(c.ni_, c.nj_, c.nk_, c.transform_),
    accum_(c.accum_),
    vals_size_(c.vals_size_)
{
  vals_ = new index_type[vals_size_];
  for (index_type i = 0; i < vals_size_; i++)
  {
    vals_[i] = c.vals_[i];
  }
}


SearchGrid::~SearchGrid()
{
  if (vals_) { delete vals_; } vals_ = 0; vals_size_ = 0;
}


#define SEARCHGRID_VERSION 1

void
SearchGrid::io(Piostream& stream)
{
  stream.begin_class("SearchGrid", SEARCHGRID_VERSION);

  // IO data members, in order
  Pio(stream, ni_);
  Pio(stream, nj_);
  Pio(stream, nk_);

  Pio(stream, transform_);

  //Pio(stream, accum_);
  //Pio(stream, vals_);

  stream.end_class();
}


} // namespace SCIRun


