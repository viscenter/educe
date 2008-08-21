//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2007 Scientific Computing and Imaging Institute,
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

#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <Core/Events/DataManager.h>
#include <Core/Algorithms/Interface/NewField/SeedIntersectionsAlg.h>

namespace SCIRun {
using namespace std;

typedef map<string, vector<Point> > seed_map_t;

void
do_seedQ(vector<Point> &pnts, size_t xi, size_t yi, size_t zi)
{
  const float f = .75;
  for (float z = -0.5; z <= 0.5; z+=f) {
    float zs = z + (float)zi;
    for (float y = -0.5; y <= 0.5; y+=f) {
      float ys = y + (float)yi;
      for (float x = -0.5; x <= 0.5; x+=f) {
        float xs = x + (float)xi;
        //ostringstream str;
        //str << xs << " " << ys << " " << zs << " 0.0 0.0 0.0 0.0";
        pnts.push_back(Point(xs, ys, zs));
      }
    }
  }
}

void
do_seed(vector<Point> &pnts, size_t xi, size_t yi, size_t zi)
{
  pnts.push_back(Point(xi, yi, zi));
}

void
seed_dbls(vector<NrrdDataHandle> &nrrds, seed_map_t &smap)
{
  // Expect all nrrds to have the same size
  NrrdDataHandle nh0 = nrrds[0];
  int x_sz = nh0->nrrd_->axis[0].size;
  int y_sz = nh0->nrrd_->axis[1].size;
  int z_sz = nh0->nrrd_->axis[2].size;
  int n = nrrds.size();
  const int skip = 4;
  for (size_t i = 0; i < n; ++i) {
    float *idat = (float*)nrrds[i]->nrrd_->data;
    for (size_t j = i+1; j < n; ++j) {
      float *jdat = (float*)nrrds[j]->nrrd_->data;
      ostringstream nm;
      nm << "d" << i << j;
      smap[nm.str()] = vector<Point>();

      for (size_t s = 0; s < z_sz; s+=skip) {
        for (size_t r = 0; r < y_sz; r+=skip) {
          for (size_t p = 0; p < x_sz; p+=skip) {
            int s1 = s; if (s1 > 0) s1 = s - rand() % skip;
            int r1 = r; if (r1 > 0) r1 = r - rand() % skip;
            int p1 = p; if (p1 > 0) p1 = p - rand() % skip;
            size_t off = s1 * x_sz * y_sz + r1 * x_sz + p1;
            if (idat[off] > 0 && jdat[off] > 0) 
            {
              // possible double junction here
              do_seed(smap[nm.str()], p, r, s);
            }
          }
        }
      }
    }
  }
}

void
seed_trips(vector<NrrdDataHandle> &nrrds, seed_map_t &smap)
{
  // Expect all nrrds to have the same size
  NrrdDataHandle nh0 = nrrds[0];
  int x_sz = nh0->nrrd_->axis[0].size;
  int y_sz = nh0->nrrd_->axis[1].size;
  int z_sz = nh0->nrrd_->axis[2].size;
  int n = nrrds.size();
  const int skip = 2;
  for (size_t i = 0; i < n; ++i) {
    float *idat = (float*)nrrds[i]->nrrd_->data;
    for (size_t j = i+1; j < n; ++j) {
      float *jdat = (float*)nrrds[j]->nrrd_->data;
      for (size_t k = j+1; k < n; ++k) {
        float *kdat = (float*)nrrds[k]->nrrd_->data;
        ostringstream nm;
        nm << "t" << i << j << k;
        smap[nm.str()] = vector<Point>();
       
        for (size_t s = 0; s < z_sz; s+=skip) {
          for (size_t r = 0; r < y_sz; r+=skip) {
            for (size_t p = 0; p < x_sz; p+=skip) {
              int s1 = s; if (s1 > 0) s1 = s - rand() % skip;
              int r1 = r; if (r1 > 0) r1 = r - rand() % skip;
              int p1 = p; if (p1 > 0) p1 = p - rand() % skip;
              size_t off = s1 * x_sz * y_sz + r1 * x_sz + p1;
              if (idat[off] > 0 && jdat[off] > 0 && 
                  kdat[off] > 0) 
              {
                // possible triple junction here
                do_seed(smap[nm.str()], p, r, s);
              }
            }
          }
        }
      }
    }
  }
}


void
seed_quads(vector<NrrdDataHandle> &nrrds, seed_map_t &smap)
{
  // Expect all nrrds to have the same size
  NrrdDataHandle nh0 = nrrds[0];
  int x_sz = nh0->nrrd_->axis[0].size;
  int y_sz = nh0->nrrd_->axis[1].size;
  int z_sz = nh0->nrrd_->axis[2].size;
  int n = nrrds.size();
  for (size_t i = 0; i < n; ++i) {
    float *idat = (float*)nrrds[i]->nrrd_->data;
    for (size_t j = i+1; j < n; ++j) {
      float *jdat = (float*)nrrds[j]->nrrd_->data;
      for (size_t k = j+1; k < n; ++k) {
        float *kdat = (float*)nrrds[k]->nrrd_->data;
        for (size_t l = k+1; l < n; ++l) {
          float *ldat = (float*)nrrds[l]->nrrd_->data;
          ostringstream nm;
          nm << "q" << i << j << k << l;
          smap[nm.str()] = vector<Point>();
          
          for (size_t s = 0; s < z_sz; ++s) {
            for (size_t r = 0; r < y_sz; ++r) {
              for (size_t p = 0; p < x_sz; ++p) {
                size_t off = s * x_sz * y_sz + r * x_sz + p;
                if (idat[off] > 0 && jdat[off] > 0 && 
                    kdat[off] > 0 && ldat[off] > 0) {
                  // possible quad junction here
                  do_seedQ(smap[nm.str()], p, r, s);
                }
              }
            }
          }
        }
      }
    }
  }
}

SeedIntersectionsAlg* get_seed_intersections_alg()
{
  return new SeedIntersectionsAlg();
}

//! Algorithm Interface.
vector<size_t>
SeedIntersectionsAlg::execute(vector<size_t> nrrd_ids)
{
  vector<size_t> rval(2, 0);
  DataManager *dm = DataManager::get_dm();
  vector<NrrdDataHandle> nrrds;
  ProgressReporter *pr = get_progress_reporter();
  vector<size_t>::iterator id_iter = nrrd_ids.begin();
  while (id_iter != nrrd_ids.end()) {
    NrrdDataHandle nrrd = dm->get_nrrd(*id_iter++);
    if (! nrrd.get_rep()) { 
      cerr << "Error: empty nrrd handle in input set." << endl;
    } else {
      nrrds.push_back(nrrd);
    }
  }

  if (nrrds.size() <= 0) {
    if (pr) pr->error("must have at least one nrrd input.");
    return rval;
  }

  NrrdDataHandle nh0 = nrrds[0];
  int x_sz = nh0->nrrd_->axis[0].size;
  int y_sz = nh0->nrrd_->axis[1].size;
  int z_sz = nh0->nrrd_->axis[2].size;
  seed_map_t seeds;
  seed_quads(nrrds, seeds);
  seed_trips(nrrds, seeds);
  seed_dbls(nrrds, seeds);

  if (get_p_gen_field()) {
    //generate point cloud field...
    cerr << "creating point clouds not implemented from SeedIntersectionsAlg" 
         << endl;
  }

  if (get_p_gen_scene_graph()) {
    //generate point cloud field...
    cerr << "creating scene graph not implemented from SeedIntersectionsAlg" 
         << endl;
  }

  if (get_p_save_ptcl_files()) 
  {
    seed_map_t::iterator iter = seeds.begin();
    while (iter != seeds.end()) {
      const string &key = (*iter).first;
      //cerr << key << endl;
      string fname(get_p_ptcl_path() + key + "_seed.ptcl");
      vector<Point> &points = (*iter).second;
      if (points.size()) {
        ofstream pts(fname.c_str());
        vector<Point>::iterator piter = points.begin();
        pts << points.size() << endl;
        while (piter != points.end()) {
          Point &p = *piter++;
          pts << p.x() << " " << p.y() << " " << p.z() << " 0.0 0.0 0.0 0.0" 
              << endl;
        }
        pts.close();
      }
      ++iter;
    }
  }

  return rval;
}

} //end namespace SCIRun

