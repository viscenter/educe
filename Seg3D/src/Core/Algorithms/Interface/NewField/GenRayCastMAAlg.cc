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
//    File   : GenRayCastMAAlg.cc
//    Author : Martin Cole
//    Date   : Wed May  7 13:29:14 2008

#include <iostream>
#include <Core/Events/DataManager.h>
#include <Core/Algorithms/Interface/NewField/GenRayCastMAAlg.h>
#include <Core/Algorithms/Interface/NewField/KDTree.h>
#include <Core/Basis/TriLinearLgn.h>
#include <Core/Thread/Thread.h>
#include <Core/Thread/Runnable.h>
#include <Core/Datatypes/TriSurfMesh.h>
#include <Core/Datatypes/GenericField.h>
#include <Core/Geom/GeomQuads.h>
#include <Core/Geom/GeomPoint.h>
//#include <Dataflow/Modules/Fields/SetMatIndex.h>

#include <fstream>

namespace SCIRun {
using namespace SLIVR;

typedef TriSurfMesh<TriLinearLgn<Point> >               TSMesh;
typedef TriLinearLgn<double>                            DatBasis;
typedef GenericField<TSMesh, DatBasis, vector<double> > TSField;

class GenMA : public GenRayCastMAAlg
{
public:
  GenMA();
  virtual ~GenMA();

  virtual vector<size_t> execute(size_t field_id0);
  void show_bin(int d, const BBox &bin);
  void add_point_set(const vector<Point> &pnts);
  void clear_points() 
  {
    pnt_lock_.lock();
    ma_points_.clear();
    pnt_lock_.unlock();
  }

protected:
  Mutex                    pnt_lock_;
  vector<Point>            ma_points_;
  //! output port
  GeomFastQuads           *faces_;
};


class MACast : public Runnable
{
public:
  MACast(const KDTree &kd, const vector<Ray> &rays, GenMA *m) :
    kd_(kd),
    rays_(rays),
    dcount_(0),
    mod_(m)
  {}

  virtual ~MACast() {}

  virtual void run();
  vector<Point>& get_ma_points() { return ma_points_; }
  size_t done_count() const { return dcount_; }
private:

  bool find_hits(const Ray &r, vector<Tri*> tris, vector<double> &hits) const;
  double get_nearest_tri_intersection(const Ray &r, const KDTree &kd) const;
  bool add_ma_point(const Ray &r, const KDTree &kd, Point &ma) const;

  const KDTree                    &kd_;
  const vector<Ray>               &rays_;
  vector<Point>                    ma_points_;
  size_t                           dcount_;
  GenMA                          *mod_;
};

class MyBuildCB : public BuildCB {
public:
  MyBuildCB(GenMA *mod) : mod_(mod) {}
  virtual void level_callback(int d, const BBox &v) 
  { mod_->show_bin(d, v); }
    
private:
  GenMA                 *mod_;
};

GenMA::GenMA() :
  GenRayCastMAAlg(),
  pnt_lock_("GenMA point lock"),
  faces_(0)
{
}

GenMA::~GenMA()
{
}

void 
GenMA::add_point_set(const vector<Point> &pnts)
{
  pnt_lock_.lock();
  ma_points_.insert(ma_points_.end(), pnts.begin(), pnts.end());
  pnt_lock_.unlock();
}

void
GenMA::show_bin(int depth, const BBox &bin)
{
  static MaterialHandle mh0;
  static MaterialHandle mh1;
  static MaterialHandle mh2;
  static MaterialHandle mh3;
  static MaterialHandle mh4;
  static MaterialHandle mh5;


  if (mh0.get_rep() == 0) {
    mh0 = new Material(Color(8.0, 2.0, 2.0));
    mh0->transparency = 0.45;
    mh1 = new Material(Color(2.0, 8.0, 2.0));
    mh1->transparency = 0.45;
    mh2 = new Material(Color(2.0, 2.0, 8.0));
    mh2->transparency = 0.45;
    mh3 = new Material(Color(8.0, 8.0, 2.0));
    mh3->transparency = 0.45;
    mh4 = new Material(Color(8.0, 2.0, 8.0));
    mh4->transparency = 0.45;
    mh5 = new Material(Color(2.0, 8.0, 8.0));
    mh5->transparency = 0.45;
  }
  if (! bin.valid() )
    cerr << bin.min() << "   " << bin.max() << endl;
  Point mn, mx;
  mn = bin.min();
  mx = bin.max();
  
  Point p0(mn);
  Point p1(mx.x(), mn.y(), mn.z());
  Point p2(mx.x(), mx.y(), mn.z());
  Point p3(mn.x(), mx.y(), mn.z());
  Point p4(mn.x(), mn.y(), mx.z());
  Point p5(mx.x(), mn.y(), mx.z());
  Point p6(mx);
  Point p7(mn.x(), mx.y(), mx.z());

  faces_->add(p0, mh0, p1, mh0, p2, mh0, p3, mh0);
  faces_->add(p1, mh1, p0, mh1, p4, mh1, p5, mh1);
  faces_->add(p0, mh2, p3, mh2, p7, mh2, p4, mh2);
  faces_->add(p3, mh3, p2, mh3, p6, mh3, p7, mh3);
  faces_->add(p4, mh4, p7, mh4, p6, mh4, p5, mh4);
  faces_->add(p1, mh5, p5, mh5, p6, mh5, p2, mh5);
}

void
MACast::run()
{
  ma_points_.clear();
  vector<Ray>::const_iterator iter = rays_.begin();
  while (iter != rays_.end()) {
    const Ray &r = *iter++;
    Point ma;
    if (add_ma_point(r, kd_, ma)) {
      ma_points_.push_back(ma);
    }
    if(mod_->get_progress_reporter() && ++dcount_ % 100 == 0) {
      mod_->get_progress_reporter()->increment_progress();
    }
  }
  //cerr << "finished: " << this << endl;
  mod_->add_point_set(ma_points_);
}

struct tritor
{
  bool operator()(Tri *t00, Tri *t01) {
    const MeshTri<TSMesh> *t0 = (MeshTri<TSMesh>*)t00;
    const MeshTri<TSMesh> *t1 = (MeshTri<TSMesh>*)t01;

    if (t0->idx_ < t1->idx_) return true;
    return false;
  }
};

bool
MACast::find_hits(const Ray &r, vector<Tri*> tris, vector<double> &hits) const
{
  bool rval = false;
  double t,u,v;
  const bool bf_cull = false;
  vector<Tri*>::iterator iter = tris.begin();
  while (iter != tris.end())
  {
    
    Tri *tri = *iter++;
    Point p0,p1,p2;
    tri->get_points(p0, p1, p2);

    if (RayTriangleIntersection(t, u, v, bf_cull, r.origin(), r.direction(),
                                p2, p1, p0))
    {
      MeshTri<TSMesh> *mtri = (MeshTri<TSMesh>*)tri;     
      //Point hit = r.origin() + r.direction() * t;
      hits.push_back(t);
      rval = true;
      //       cerr << "intersected a triangle: " << p0 << p1 << p2 << endl;
      //       cerr << "at: " << hit << endl;
      //       cerr << "face idx is: " << mtri->idx_ << endl;
    }
  }
  return rval;
}

double
MACast::get_nearest_tri_intersection(const Ray &r, const KDTree &kd) const
{
  vector<KDTree::KDLeafHits> hits;
  kd.leaf_ray_intersections(r, hits);

  if (hits.size() == 0) {
    return 0.0L;
  }

  vector<double> rhits(10, 0.0);
  // nodes are sorted nearest first.
  vector<KDTree::KDLeafHits>::iterator hiter = hits.begin();
  while (hiter != hits.end()) {
    const KDTree::KDLeafNode *node = (*hiter).ln_;
    ++hiter;
    rhits.clear();
    
    if (! find_hits(r, node->elems_, rhits)) continue;
    
    // use nearest non zero parameter in ray dir or opposite.
    sort(rhits.begin(), rhits.end());
    
    vector<double>::iterator iter = rhits.begin();
    double neg = 0.0;
    double pos = 0.0;
    bool found = false;
    while (iter != rhits.end()) {
      const double d = *iter++;
      if (fabs(d) < 0.000001) {
        continue;
      } else {
        return d;
      }
    }
  }
}

bool
MACast::add_ma_point(const Ray &r, const KDTree &kd, Point &ma) const
{
  
  // just add closest point.
  double t = 0.5 * get_nearest_tri_intersection(r, kd);
  if (fabs(t) < 0.000001) {
    return false;
  }
  //  cerr << "norm :" << r.direction().length() << endl;
  //cerr << "t :" << t << endl;
  ma = r.origin() + r.direction() * t;


  const Vector ax[] = {Vector(1, 0, 0), Vector(0, 1, 0), Vector(0, 0, 1)};

  double maxax = r.direction().x();
  int axidx = 2;
  if (r.direction().y() > maxax) {
    axidx = 0;
    maxax = r.direction().y();
  }
  if (r.direction().z() > maxax) {
    axidx = 1;
  }

  Vector dir = ax[axidx];
  Vector v0 = Cross(r.direction(), dir);
  v0.normalize();
  Vector v1 = -v0;
  Vector v2 = Cross(r.direction(), v1);
  v2.normalize();
  Vector v3 = -v2;

  Ray r0(ma, v0);
  Ray r1(ma, v1);
  Ray r2(ma, v2);
  Ray r3(ma, v3);
  
  double t0 = get_nearest_tri_intersection(r0, kd);
  double t1 = get_nearest_tri_intersection(r1, kd);
  double t2 = get_nearest_tri_intersection(r2, kd);
  double t3 = get_nearest_tri_intersection(r3, kd);
  vector<double> from_ma;
  from_ma.push_back(fabs(t0));
  from_ma.push_back(fabs(t1));
  from_ma.push_back(fabs(t2));
  from_ma.push_back(fabs(t3));
  sort(from_ma.begin(), from_ma.end());
  double min_param = from_ma[0];
  if (min_param == 0.0L) {
    min_param = from_ma[1];
  }
  if (min_param == 0.0L) {
    min_param = from_ma[2];
  }
  if (min_param == 0.0L) {
    min_param = from_ma[3];
  }
  if (min_param == 0.0L) {
    return false;
  }
  
  if (min_param / fabs(t) < 0.2) {
    // reject.
//     cout << "rejecting medial axis point: " 
//          << min_param / fabs(t) << endl;
//     cout << "min_param: " << min_param << endl;
//     cout << "t: " << t << endl;
    return false;
  }
  return true;
}



vector<size_t>
GenMA::execute(size_t field_id)
{
  ProgressReporter *pr = get_progress_reporter();
  DataManager *dm = DataManager::get_dm();
  FieldHandle fld_h = dm->get_field(field_id);

  vector<size_t> rval(3, 0);
  //cerr << fld_h.get_rep() << endl;
  //get the triangles
  TSField* ts = dynamic_cast<TSField*>(fld_h.get_rep());
  if (!ts) {
    if (pr) pr->error("Input Field is not a TriSurf.");
    cerr << "ts == 0" << endl;
    return rval;
  }

  TSMesh *m = (TSMesh*)ts->get_typed_mesh().get_rep();
  //m->orient_faces();

  BBox bb = m->get_bounding_box();
  vector<Tri*> tris;
  TSMesh::Face::iterator fi, fend;
  m->begin(fi);
  m->end(fend);
  
  while (fi != fend) {
    tris.push_back(new MeshTri<TSMesh>(*fi, m));
    ++fi;
  }

  MyBuildCB *cb = 0;
  if (get_p_gen_geom_points()) {
    faces_ = new GeomTranspQuads;
    GeomHandle gh = faces_;
    cb = new MyBuildCB(this);
  }

  KDTree kd(tris, bb, cb);
  clear_points();

  GeomPoints    *points = 0;
  if (get_p_gen_geom_points()) {
    points = new GeomPoints();
    //GeomHandle ph = points;
  }

  // generate medial axis approximation points.
  m->synchronize(Mesh::NORMALS_E);
  TSMesh::Node::iterator ni, nend;
  m->begin(ni);
  m->end(nend);

  size_t count = 0;  
  const int n_threads = get_p_num_threads();
  vector<vector<Ray> > rays(n_threads);

  while (ni != nend) {
    Vector n;
    Point p;
    m->get_normal(n, *ni);
    n.normalize();
    m->get_point(p, *ni);
    ++ni;

    rays[count++ % n_threads].push_back(Ray(p,-n));
  }
  int max = (int)count / 100;
  //cerr << max << endl;
  if (pr) pr->update_progress(0, max);
  vector<Thread*> threads(n_threads);
  vector<MACast*> casters(n_threads);
  for (int i = 0; i < n_threads; i++)
  {
    ostringstream ss;
    ss << "MA_Ray_Caster_#" << i << endl;
    casters[i] = new MACast(kd, rays[i], this);
    threads[i] = new Thread(casters[i], ss.str().c_str());
  }

  for (int i = 0; i < n_threads; i++) {
    threads[i]->join();
  }
  cerr << "generated "<< ma_points_.size() << " ma points" << endl;

  if (get_p_save_ptcl_file() || get_p_gen_geom_points()) {
    ofstream fp(get_p_ptcl_filename().c_str());
    if (get_p_save_ptcl_file() && ma_points_.size()) {
      fp << ma_points_.size() << endl;
    }
    vector<Point>::iterator iter = ma_points_.begin();
    while (iter != ma_points_.end()) {
      Point &p = *iter++;
      if (get_p_save_ptcl_file()) {
        fp << p.x() << " " << p.y() << " " << p.z() 
           << " 0.0 0.0 0.0 0.0" << endl;
      }
      if (get_p_gen_geom_points()) {
        points->add(p);
      }
    }
    fp.close();
  }
  if (pr) pr->update_progress(1.0);
  return rval;
}

GenRayCastMAAlg* get_gen_ray_cast_m_a_alg()
{
  return new GenMA();
}


vector<size_t> 
GenRayCastMAAlg::execute(size_t field_id0)
{
}

} //end namespace SCIRun
