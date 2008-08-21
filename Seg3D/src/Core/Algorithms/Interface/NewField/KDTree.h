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
//    File   : KDTree.h
//    Author : Martin Cole
//    Date   : Thu Apr 24 12:54:15 2008

#if !defined(KDTree_h)
#define KDTree_h

#include <slivr/BBox.h>
#include <slivr/Ray.h>
#include <slivr/Plane.h>
#include <vector>

namespace SCIRun {
using namespace SLIVR;
using std::vector;

class Tri 
{
public:
  virtual void get_points(Point &p0, Point &p1, Point &p2) = 0;
};

class BuildCB {
public:
  virtual void level_callback(int d, const BBox &v) {}
};

template <class Msh>
class MeshTri : public Tri
{
public:

  MeshTri(typename Msh::Face::index_type idx, Msh *m) :
    idx_(idx),
    mesh_(m)
  {}

  virtual void get_points(Point &p0, Point &p1, Point &p2);
  

  typename Msh::Face::index_type idx_;
  Msh                           *mesh_;
};

template <class Msh>
void
MeshTri<Msh>::get_points(Point &p0, Point &p1, Point &p2)
{
  typename Msh::Node::array_type nodes;
  mesh_->get_nodes(nodes, idx_);
  mesh_->get_center(p0, nodes[0]);
  mesh_->get_center(p1, nodes[1]);
  mesh_->get_center(p2, nodes[2]);
}

class KDTree {
public:
  typedef size_t idx_type_t;

  struct Node {
    
  };
  struct KDNode {
    KDNode() :
      split_(0),
      left_c_(0),
      right_c_(0)
    {}

    virtual ~KDNode()
    {
      if (left_c_) { delete left_c_; }
      if (right_c_) { delete right_c_; }
      if (split_) { delete split_; }
    }

    virtual bool is_leaf() const { return false; }

    //! space the node covers.
    BBox                    voxel_;
    Plane                   *split_;
    
    //! left and right child nodes.
    KDNode                  *left_c_;
    KDNode                  *right_c_;
    
  };

  
  struct KDLeafNode: public KDNode {
    KDLeafNode() : 
      KDNode(),
      elems_(0)
     {}

    virtual ~KDLeafNode()
    {}
    virtual bool is_leaf() const { return true; }
    //! Each node has the intersecting element indices, 
    //! stored in the global array 
    vector<Tri*>          elems_;
  };

  struct KDLeafHits {
    KDLeafHits(KDLeafNode *n, double d) :
      ln_(n),
      dist2_(d)
    {}

    const KDLeafNode *ln_;
    double            dist2_;
  };

  KDTree(const vector<Tri*> &t, const BBox &v, BuildCB *cb = 0);
  ~KDTree();  

  void leaf_ray_intersections(const Ray &r, vector<KDLeafHits> &hits) const;

private:
  KDNode* build(const vector<Tri*> &t, const BBox &V, int depth);
  bool terminate(const vector<Tri*> &tris, const BBox &V, int depth);
  void find_plane(int depth, const BBox &vol, Plane &split);
  //void split_volume(const BBox &V, const Plane &p, BBox &V_l, BBox &V_r);
  void sort_tris(const vector<Tri*> &tris, const Plane &split, 
                 vector<Tri*> &left, vector<Tri*> &right,
                 BBox &l, BBox &r);  

  void clip_bb(const BBox &V, BBox &to_clip, const Plane &p);
  void leaf_ray_intersections_recurse(KDNode *cur, const Ray &r, double d2, 
                                      vector<KDLeafHits> &hits) const;
private:
  BBox                        bounds_;
  KDNode                     *root_;
  BuildCB                    *build_cb_;
};


} // namespace SLIVR

#endif //KDTree_h
