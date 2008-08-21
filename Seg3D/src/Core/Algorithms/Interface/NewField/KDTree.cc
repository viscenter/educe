//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  License for the specific language governing rights and limitations under
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  
//    File   : KDTree.cc
//    Author : Martin Cole
//    Date   : Thu Apr 24 12:55:09 2008

#include <Core/Algorithms/Interface/NewField/KDTree.h>
#include <iostream>

namespace SCIRun {
using namespace std;

KDTree::KDTree(const vector<Tri*> &b, const BBox &v, BuildCB *cb) :
  bounds_(v),
  root_(0),
  build_cb_(cb)
{
  //cerr << v.min() << "   " << v.max() << endl;
  root_ = build(b, v, 0);
}

KDTree::~KDTree() 
{
  //cerr << "KDTree destroyed." << endl;
}

void
print_bbox(string s, const BBox &b) {
  cerr << s << b.min() << " to " << b.max() << endl;
}

struct LeafSortFtor
{
  bool operator()(const KDTree::KDLeafHits &n0, const KDTree::KDLeafHits &n1) {
    return n0.dist2_ < n1.dist2_;
  }
};

void
KDTree::leaf_ray_intersections(const Ray &r, vector<KDLeafHits> &hits) const
{
  hits.clear();
  leaf_ray_intersections_recurse(root_, r, 0.0, hits);

  if (hits.size()) 
    //sort the hits to be closest first.
    sort(hits.begin(), hits.end(), LeafSortFtor());
}


void
KDTree::leaf_ray_intersections_recurse(KDNode *cur, const Ray &r, double d2,
                                       vector<KDLeafHits> &hits) const
{

  if (cur->is_leaf()) {
    KDLeafNode *l = (KDLeafNode*)cur;
    hits.push_back(KDLeafHits(l, d2));
    return;
  }
  
  Point hit;
  if (cur->left_c_ && 
      cur->left_c_->voxel_.valid() &&
      cur->left_c_->voxel_.intersect(r.origin(), r.direction(), hit))
  {
    
    leaf_ray_intersections_recurse(cur->left_c_, r,
                                   (hit - r.origin()).length2(), 
                                   hits); 
  }

  if (cur->right_c_ && 
      cur->right_c_->voxel_.valid() &&
      cur->right_c_->voxel_.intersect(r.origin(), r.direction(), hit))
  {
    leaf_ray_intersections_recurse(cur->right_c_, r,
                                   (hit - r.origin()).length2(),
                                   hits); 
  }

}

KDTree::KDNode* 
KDTree::build(const vector<Tri*> &tris, const BBox &V, int depth) 
{
  if (terminate(tris, V, depth)) {
    KDLeafNode *n = new KDLeafNode();
    n->voxel_ = V;
    n->elems_ = tris;
    if (n->elems_.size() == 0) {
      n->voxel_.set_valid(false);
    }
    if (build_cb_ && tris.size()) { 
      build_cb_->level_callback(-1, V); 
    }
    //cerr << "LEAF" << endl;
    return n;
  }
  //if (build_cb_) { build_cb_->level_callback(depth, V); }
  Plane *p = new Plane();
  find_plane(depth, V, *p);

  vector<Tri*> tris_l;
  vector<Tri*> tris_r;
  BBox V_l;
  BBox V_r; 
  sort_tris(tris, *p, tris_l, tris_r,  V_l, V_r);

  clip_bb(V, V_l, *p);
  clip_bb(V, V_r, *p);

  KDNode *n = new KDNode();
  n->voxel_ = V;
  n->split_ = p;
  n->left_c_  = build(tris_l, V_l, depth + 1);
  n->right_c_ = build(tris_r, V_r, depth + 1);
  //cerr << "NODE" << endl;

  return n;
}

bool
KDTree::terminate(const vector<Tri*> &tris, const BBox &V, int depth)
{
  const double eps = .001;
  if (! V.valid()) return true;

  Vector d = V.diagonal();
  if (d.x() > eps && d.y() > eps && d.z() > eps) {
    return tris.size() < 10 || depth > 10;
  } else {
    return true;
  }
}


void
KDTree::find_plane(int depth, const BBox &vol, Plane &split)
{
  Vector d = vol.diagonal();
  Point mid = vol.min() + d * 0.5;
  
  //split longest dimension.
  double le = vol.longest_edge();
  if (d.x() ==  le) {
    //y,z plane
    split = Plane(mid, Vector(1.0, 0.0, 0.0));
  } else if (d.y() == le) {
    // x,z plane
    split = Plane(mid, Vector(0.0, 1.0, 0.0));
  } else {
    // x,y plane
    split = Plane(mid, Vector(0.0, 0.0, 1.0));
  }
  
//   switch(depth % 3) {
//   case 0:
//     {
//       // x,y plane
//       split = Plane(mid, Vector(0.0, 0.0, 1.0));
//     }
//   break;
//   case 1:
//     {
//       // x,z plane
//       split = Plane(mid, Vector(0.0, 1.0, 0.0));
//     }
//   break;
//   case 2:
//     {
//       //y,z plane
//       split = Plane(mid, Vector(1.0, 0.0, 0.0));
//     }
//   break;
//   };
}

void
KDTree::clip_bb(const BBox &V, BBox &c, const Plane &p )
{
  Point mn;
  Point mx;

  if (V.min().x() > c.min().x()) {
    mn.x(V.min().x());
  } else {
    mn.x(c.min().x());
  }

  if (V.min().y() > c.min().y()) {
    mn.y(V.min().y());
  } else {
    mn.y(c.min().y());
  }

  if (V.min().z() > c.min().z()) {
    mn.z(V.min().z());
  } else {
    mn.z(c.min().z());
  }

  if (V.max().x() < c.max().x()) {
    mx.x(V.max().x());
  } else {
    mx.x(c.max().x());
  }

  if (V.max().y() < c.max().y()) {
    mx.y(V.max().y());
  } else {
    mx.y(c.max().y());
  }

  if (V.max().z() < c.max().z()) {
    mx.z(V.max().z());
  } else {
    mx.z(c.max().z());
  }

  c = BBox(mn, mx);
}

void
KDTree::sort_tris(const vector<Tri*> &tris, const Plane &split, 
                  vector<Tri*> &left_tris, vector<Tri*> &right_tris,
                  BBox &l, BBox &r)
{
  vector<Tri*>::const_iterator iter = tris.begin();
  while (iter != tris.end()) {
    Tri *t = *iter++;

    bool left = false;
    bool right = false;

    Point p0,p1,p2;
    t->get_points(p0, p1, p2);

    if (split.eval_point(p0) < 0.0) {
      left = true;
    } else {
      right = true;
    }

    if (split.eval_point(p1) < 0.0) {
      left = true;
    } else {
      right = true;
    }

    if (split.eval_point(p2) < 0.0) {
      left = true;
    } else {
      right = true;
    }
     
    if (left) {
      left_tris.push_back(t);
      l.extend(p0);
      l.extend(p1);
      l.extend(p2);
    }

    if (right) {
      right_tris.push_back(t);
      r.extend(p0);
      r.extend(p1);
      r.extend(p2);
    }

  }
  Point h;
  if (l.valid()) {
    split.Intersect(l.min(), l.diagonal(), h);
  } else {
    split.Intersect(r.max(), -r.diagonal(), h);
  }

  //axis aligned planes...
  Point tmp;
  if (split.normal().x() > 0.9) {
    if (l.valid()) {
      tmp = l.max();
      tmp.x(h.x());
      l = BBox(l.min(), tmp);
    }
    
    if (r.valid()) {
      tmp = r.min();
      tmp.x(h.x());
      r = BBox(tmp, r.max());
    }
  } else if (split.normal().y() > 0.9) {
    if (l.valid()) {
      tmp = l.max();
      tmp.y(h.y());
      l = BBox(l.min(), tmp);
    }

    if (r.valid()) {
      tmp = r.min();
      tmp.y(h.y());
      r = BBox(tmp, r.max());
    }
  } else if (split.normal().z() > 0.9) {
    if (l.valid()) {
      tmp = l.max();
      tmp.z(h.z());
      l = BBox(l.min(), tmp);
    }

    if (r.valid()) {
      tmp = r.min();
      tmp.z(h.z());
      r = BBox(tmp, r.max());
    }
  }
}

} // namespace SLIVR
