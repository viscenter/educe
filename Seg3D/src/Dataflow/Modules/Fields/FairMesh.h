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
//    File   : FairMesh.h
//    Author : Martin Cole
//    Date   : Wed Mar 14 09:30:53 2007

#if !defined(Modules_FairMesh_h)
#define Modules_FairMesh_h

#include <Core/Util/Timer.h>
#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <Core/Util/ProgressReporter.h>
#include <Core/Datatypes/Matrix.h>
#include <math.h>

//! for Windows support
#include <Dataflow/Modules/Fields/share.h>

namespace SCIRun {

class SCISHARE SurfaceFairingAlgoBase : public DynamicAlgoBase
{
public:
  SurfaceFairingAlgoBase() :
    lambda_(0.6307),
    pass_band_freq_(0.1)
  {}
  void lambda(double l) { lambda_ = l; }
  void pass_band_freq(double k) { pass_band_freq_ = k; }
  
  //! cache off nbor indices for efficiency.
  virtual void generate_neighborhoods(FieldHandle fh, string method) = 0;
  virtual FieldHandle iterate(ProgressReporter *reporter, FieldHandle fh, 
			      int steps, string method) = 0;


  static CompileInfoHandle get_compile_info(const TypeDescription *td);
protected:
  double lambda_;
  double pass_band_freq_;
  double mu() { return 1./(pass_band_freq_ - (1./lambda_)); }
  
  virtual void generate_displacements(FieldHandle fh, vector<Vector> &disp,
				      string method) = 0;

};


template <class FLD>
class SurfaceFairingAlgo : public SurfaceFairingAlgoBase
{
public:
  typedef typename FLD::mesh_type::Node::index_type node_idx_t;

  virtual void generate_neighborhoods(FieldHandle fh, string method);
  virtual FieldHandle iterate(ProgressReporter *reporter, FieldHandle fh, 
			      int steps, string method);

protected:
  void desbrun_displacements(FieldHandle fh, vector<Vector> &disp);
  void fast_displacements(FieldHandle fh, vector<Vector> &disp);

  void ordered_neighborhoods(FieldHandle fh);
  void fast_neighborhoods(FieldHandle fh);

  virtual void generate_displacements(FieldHandle fh, vector<Vector> &disp,
				      string method);

  //! cached node neighborhood indices, in order.
  vector<vector<node_idx_t> > neighborhoods_;
};

template <class Fld>
void
SurfaceFairingAlgo<Fld>::fast_neighborhoods(FieldHandle fh)
{
  // empty any previously calculated displacements.
  Fld *fld = dynamic_cast<Fld*>(fh.get_rep());
  typename Fld::mesh_handle_type mesh = fld->get_typed_mesh();
  mesh->synchronize(Mesh::NODE_NEIGHBORS_E);
  typename Fld::mesh_type::Node::size_type sz;
  mesh->size(sz);
  neighborhoods_.clear();
  neighborhoods_.resize(sz);
  
  typename Fld::mesh_type::Node::iterator niter;  mesh->begin(niter);
  typename Fld::mesh_type::Node::iterator niter_end;  mesh->end(niter_end);  
  int i = 0;  
  
  vector<typename Fld::mesh_type::Node::index_type> nodes;

  while (niter != niter_end) {
    typename Fld::mesh_type::Node::index_type ni = *niter; ++niter;
    //! for each edge in the neighborhood, store the vector, and 
    //! calculate the total edge length.

    vector<node_idx_t> &nhood = neighborhoods_[i++];
    mesh->get_neighbors(nhood, ni);
  }
}



template <class Fld>
void
SurfaceFairingAlgo<Fld>::ordered_neighborhoods(FieldHandle fh)
{
  // empty any previously calculated displacements.
  Fld *fld = dynamic_cast<Fld*>(fh.get_rep());
  typename Fld::mesh_handle_type mesh = fld->get_typed_mesh();
  mesh->synchronize(Mesh::NODE_NEIGHBORS_E | Mesh::ELEM_NEIGHBORS_E | 
		    Mesh::EDGES_E);
  typename Fld::mesh_type::Node::size_type sz;
  mesh->size(sz);
  neighborhoods_.clear();
  neighborhoods_.resize(sz);

  typename Fld::mesh_type::Node::iterator niter;  mesh->begin(niter);
  typename Fld::mesh_type::Node::iterator niter_end;  mesh->end(niter_end);  
  int i = 0;  
  while (niter != niter_end) 
  {
    typename Fld::mesh_type::Node::index_type ni = *niter; ++niter;

    vector<typename Fld::mesh_type::Elem::index_type> elems;
    mesh->get_elems(elems, ni);
    typename vector<typename Fld::mesh_type::Elem::index_type>::iterator elit;
    elit = elems.begin();

    vector<node_idx_t> &nhood = neighborhoods_[i++];

    // Keep the set of nodes to make sure we don't inf loop in non manifold..
    vector<node_idx_t> nhood_set;
    mesh->get_neighbors(nhood_set, ni);
    vector<bool> seen(nhood_set.size(), false);

    nhood.clear();
    bool found = false;
    typename Fld::mesh_type::Face::index_type start_face;
    while (!found && elit != elems.end()) 
    {
      //! find a starting face, one that contains the current node.
      typename Fld::mesh_type::Face::array_type faces;
      mesh->get_faces(faces, *elit);

      typename Fld::mesh_type::Face::array_type::iterator fiter;
      fiter = faces.begin();
      while (!found && fiter != faces.end()) 
      {
        // see if current node is in this face.
        typename Fld::mesh_type::Node::array_type fnodes;
        mesh->get_nodes(fnodes, *fiter);
        typename Fld::mesh_type::Node::array_type::iterator fniter;
        fniter = fnodes.begin();
        while (fniter != fnodes.end()) 
        {
          if (ni == *fniter) 
          {
            found = true;
            start_face = *fiter;
            break;
          }	  
          ++fniter;
        }
        ++fiter;
      }
      ++elit;
    }
    // have a starting face, now walk faces and record the neighbors.
    //walk the faces in consistent order around the node.
    typename Fld::mesh_type::Face::index_type cur_face;
    typename Fld::mesh_type::Edge::index_type cur_edge = -1;
    cur_face = start_face;

    bool stop_walking = false;
    do 
    {
      typename Fld::mesh_type::Edge::array_type edges;
      mesh->get_edges(edges, cur_face);
      typename Fld::mesh_type::Edge::array_type::iterator eiter;
      eiter = edges.begin();
 

      while (eiter != edges.end()) 
      {
        //! find edge to walk across.
        typename Fld::mesh_type::Node::index_type add_node_idx;
        typename Fld::mesh_type::Node::array_type enodes;
        mesh->get_nodes(enodes, *eiter);
        if ((enodes[0] == ni || enodes[1] == ni) && (cur_edge != *eiter)) 
        {
          cur_edge = *eiter;
          // cache the neighbor node in walk order.
          if (enodes[0] == ni) 
          { 
            add_node_idx = enodes[1];
          } 
          else 
          {
            add_node_idx = enodes[0];
          }
          // if we reach the same node twice, we are non-manifold.
          bool found = false;
          for (int i = 0; i < nhood_set.size(); i++) 
          {
            if (!seen[i] && (nhood_set[i] == add_node_idx)) 
            {
              seen[i] = true; // mark it seen.
              found = true;   // found node for the first time.
            }
          }
          if (found) 
          {
            nhood.push_back(add_node_idx);
          } 
          else 
          {
            // non manifold case...
            nhood.clear();
            stop_walking = true;
            cerr << "found non-manifold, use fast weighting..." << endl;
          }	  
          break;
        }
        ++eiter;
      }
      // walk to next face
      mesh->get_neighbor(cur_face, cur_face, cur_edge);
    } while (!stop_walking && cur_face != start_face);
  }
}


template <class Fld>
void
SurfaceFairingAlgo<Fld>::generate_neighborhoods(FieldHandle fh, 
						string method)
{
  if (method == "desbrun") {
    ordered_neighborhoods(fh);
  } else {
    fast_neighborhoods(fh);
  }
}


template <class Fld>
void
SurfaceFairingAlgo<Fld>::generate_displacements(FieldHandle fh, 
						vector<Vector> &disp, 
						string method)
{
  if (method == "desbrun") {
    desbrun_displacements(fh, disp);
  } else {
    fast_displacements(fh, disp);
  }
}

//equal weighting.
template <class Fld>
void
SurfaceFairingAlgo<Fld>::fast_displacements(FieldHandle fh, 
					    vector<Vector> &disp)
{
  Fld *fld = dynamic_cast<Fld*>(fh.get_rep());
  typename Fld::mesh_handle_type mesh = fld->get_typed_mesh();
  typename Fld::mesh_type::Node::iterator niter;  mesh->begin(niter);
  typename Fld::mesh_type::Node::iterator niter_end;  mesh->end(niter_end);  
  int i = 0;
  while (niter != niter_end) {
    typename Fld::mesh_type::Node::index_type ni = *niter; ++niter;
    Point p0;
    mesh->get_center(p0, ni);
    //! for each edge in the neighborhood, equally weight the displacent. 
    vector<node_idx_t> &nodes = neighborhoods_[i];   
    typename vector<node_idx_t>::iterator it;
    it = nodes.begin();
    Vector d(0.0, 0.0, 0.0);
    double w = 1.0 / nodes.size();
    while (it != nodes.end()) {
      typename Fld::mesh_type::Node::index_type nbor_idx = *it; ++it;
      Point np;
      mesh->get_center(np, nbor_idx);
      d += w * (np - p0);
    }

    //! set the displacement vector for this node.
    disp[i] = d;
    ++i;
  }
}

//! Use Desbrun weights. 
//! "Implicit Fairing of Irregular Meshes using Diffusion and Curvature Flow"
template <class Fld>
void
SurfaceFairingAlgo<Fld>::desbrun_displacements(FieldHandle fh, 
					       vector<Vector> &disp)
{
  Fld *fld = dynamic_cast<Fld*>(fh.get_rep());
  typename Fld::mesh_handle_type mesh = fld->get_typed_mesh();
  typename Fld::mesh_type::Node::iterator niter;  mesh->begin(niter);
  typename Fld::mesh_type::Node::iterator niter_end;  mesh->end(niter_end);  
  int i = 0;
  while (niter != niter_end) {
    typename Fld::mesh_type::Node::index_type ni = *niter; ++niter;
    Point p0;
    mesh->get_center(p0, ni);
    //! for each edge in the neighborhood generate displacement.
    vector<node_idx_t> &nodes = neighborhoods_[i]; 
    Vector d(0.0, 0.0, 0.0);

    if (nodes.size() == 0) {
      disp[i] = d;
      i++;
      continue;
    }
  
    double totw = 0.0;
    for (int j = 0; j < nodes.size(); j++) {
      int nprev = j - 1;
      int nnext = j + 1;
      if (j == 0) { nprev = nodes.size() - 1; }
      if (j == nodes.size() - 1) { nnext = 0; }

      Point p1;
      mesh->get_center(p1, (node_idx_t)nodes[j]);
      Vector evec = p1 - p0;
 
      Point pprev;
      mesh->get_center(pprev, (node_idx_t)nodes[nprev]);
      Point pnext;
      mesh->get_center(pnext, (node_idx_t)nodes[nnext]);

      //opposite this edge previous dir.
      Vector prev1 = p0 - pprev;
      Vector prev2 = p1 - pprev;
      double cos_alpha = Dot(prev1, prev2) / (prev1.length() * prev2.length());
      double alpha = acos(cos_alpha);
      double cot_alpha = cos_alpha / sin(alpha);

      //opposite this edge next dir.
      Vector next1 = p1 - pnext;
      Vector next2 = p0 - pnext;
      double cos_beta = Dot(next1, next2) / (next1.length() * next2.length());
      double beta = acos(cos_beta);
      double cot_beta = cos_beta / sin(beta);

      double w = cot_alpha + cot_beta;
      totw += w;
      d += w * evec;
    }
    //! set the displacement vector for this node.
    disp[i] = d * (1.0 / totw);
    ++i;
  }
}


template <class Fld>
FieldHandle
SurfaceFairingAlgo<Fld>::iterate(ProgressReporter *reporter, FieldHandle fh,
				 int steps, string method)
{
  if (fh.get_rep() == 0) 
  {
    reporter->error("Error: No input field.");
    return fh;
  }

  FieldHandle ofh = fh;
  //! make our own copy of the field (and mesh)
  ofh.detach();
  ofh->mesh_detach(); //! this new field will become the output.


  Fld *fld = dynamic_cast<Fld*>(ofh.get_rep());
  typename Fld::mesh_handle_type mesh = fld->get_typed_mesh();
  typename Fld::mesh_type::Node::size_type sz;
  mesh->size(sz);
  vector<Vector> disp(sz);
  steps = steps;
  for (int i = 0; i < steps; i++) 
  {
    generate_displacements(ofh, disp, method);
    //! Apply one shrink step.
    typename Fld::mesh_type::Node::iterator niter;  mesh->begin(niter);
    typename Fld::mesh_type::Node::iterator niter_end;  mesh->end(niter_end);  
    while (niter != niter_end) 
    {
      //! Indices do not change, no nodes are added or removed.
      typename Fld::mesh_type::Node::index_type ni = *niter; ++niter;
      const Vector &d = disp[ni];
      Point p;
      mesh->get_center(p, ni);
      //! displace the point.
      if (i % 2 == 0) 
      {
        mesh->set_point(p + lambda_ * d, ni);
      } 
      else 
      {
        mesh->set_point(p + mu() * d, ni);
      }
    }
    reporter->update_progress((double(i) / (double)steps));
  }
  return ofh;
}

} // namespace SCIRun
#endif
