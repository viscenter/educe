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


#include <Dataflow/Network/Module.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/TriLinearLgn.h>
#include <Core/Datatypes/TriSurfMesh.h>
#include <Core/Datatypes/GenericField.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Modules/Fields/share.h>
#include <queue>

namespace SCIRun {

typedef TriSurfMesh<TriLinearLgn<Point> > TSMesh;

struct DT_Quadric
{
private:
  double a2, ab, ac, ad;
  double b2, bc, bd;
  double c2, cd;
  double d2;

public:
  DT_Quadric()
    : a2(0.0), ab(0.0), ac(0.0), ad(0.0),
      b2(0.0), bc(0.0), bd(0.0),
      c2(0.0), cd(0.0),
      d2(0.0)
  {}

  DT_Quadric(double a, double b, double c, double d)
  {
    a2 = a*a; ab = a*b; ac = a*c; ad = a*d;
    b2 = b*b; bc = b*c; bd = b*d;
    c2 = c*c; cd = c*d;
    d2 = d*d;
  }

  DT_Quadric &operator*=(double w)
  {
    a2 *= w;
    ab *= w;
    ac *= w;
    ad *= w;
    b2 *= w;
    bc *= w;
    bd *= w;
    c2 *= w;
    cd *= w;
    d2 *= w;
    return *this;
  }

  DT_Quadric &operator+=(const DT_Quadric &a)
  {
    a2 += a.a2;
    ab += a.ab;
    ac += a.ac;
    ad += a.ad;
    b2 += a.b2;
    bc += a.bc;
    bd += a.bd;
    c2 += a.c2;
    cd += a.cd;
    d2 += a.d2;
    return *this;
  }

private:
  double cost(const Point &p) const
  {
    const double x = p.x();
    const double y = p.y();
    const double z = p.z();
    
    return
      x*x*a2 + 2*x*y*ab + 2*x*z*ac + 2*x*ad +
      y*y*b2 + 2*y*z*bc + 2*y*bd +
      z*z*c2 + 2*z*cd +
      d2;
  }

  double scaled_inverse_tensor(double i[3][3]) const
  {
    // Compute determinants, transpose as we go.
    i[0][0] = b2 * c2 - bc * bc;
    i[1][0] = bc * ac - c2 * ab;
    i[2][0] = ab * bc - ac * b2;
    i[0][1] = bc * ac - ab * c2;
    i[1][1] = c2 * a2 - ac * ac;
    i[2][1] = ac * ab - a2 * bc;
    i[0][2] = ab * bc - b2 * ac;
    i[1][2] = ac * ab - bc * a2;
    i[2][2] = a2 * b2 - ab * ab;
    const double d = i[0][0] * a2 + i[1][0] * ab + i[2][0] * ac;

    return d;
  }

public:

  // TODO: This find_best_point needs additional checks for topology
  // preservation.  Also should probably return cost at same time and
  // return midpoint if determinant is bad.
  bool find_best_point(Point &p) const
  {
    // Compute tensor inverse.
    double inv[3][3];
    double det = scaled_inverse_tensor(inv);
    if (fabs(det) < 1e-12)
      return false;

    // Multiply inverse by vector, make negative, divide by determinant.
    p.x((inv[0][0] * ad + inv[0][1] * bd + inv[0][2] * cd) / -det);
    p.y((inv[1][0] * ad + inv[1][1] * bd + inv[1][2] * cd) / -det);
    p.z((inv[2][0] * ad + inv[2][1] * bd + inv[2][2] * cd) / -det);
    return true;
  }

#define MAX_DOUBLE 1.0e24

  double find_cost_of_best_point() const
  {
    Point p;
    if (find_best_point(p)) return cost(p);
    else return MAX_DOUBLE;
  }
};


struct dtripple
{
  unsigned int v0, v1;
  double cost;
  dtripple(unsigned int a, unsigned int b) : v0(a), v1(b), cost(0.0) {}
};

static bool dtripple_compare(const dtripple &a, const dtripple &b)
{
  return a.cost < b.cost;
}


class SCISHARE DecimateTriSurf : public Module {
  public:
    DecimateTriSurf(GuiContext*);
    virtual ~DecimateTriSurf() {}
    
    virtual void execute();
};


DECLARE_MAKER(DecimateTriSurf)
DecimateTriSurf::DecimateTriSurf(GuiContext* ctx)
  : Module("DecimateTriSurf", ctx, Source, "NewField", "SCIRun")
{
}


static void
squish_nodemap(vector<Mesh::index_type> &nodemap)
{
  for (Mesh::index_type i = 0; i < static_cast<Mesh::index_type>(nodemap.size()); i++)
  {
    Mesh::index_type k = i;
    while (nodemap[k] != k)
    {
      k = nodemap[k];
    }
    nodemap[i] = k;
  }
}



void
DecimateTriSurf::execute()
{
  FieldHandle ifield, ofield;
  if (!(get_input_handle("TriSurf", ifield, true))) return;
  
  if (!inputs_changed_ && oport_cached("Decimated"))
  {
    return;
  }

  TSMesh *mesh = dynamic_cast<TSMesh *>(ifield->mesh().get_rep());
  if (!mesh)
  {
    error("Input TriSurf wasn't actually a TriSurfMesh with linear basis.");
  }

  mesh->synchronize(Mesh::NODES_E | Mesh::EDGES_E | Mesh::FACES_E);

  TSMesh::Face::size_type fsize;
  mesh->size(fsize);

  vector<DT_Quadric> qv((unsigned int)fsize);
  TSMesh::Node::array_type nodes;

  TSMesh::Node::size_type nsize;
  mesh->size(nsize);
  vector<Mesh::index_type> nodemap(nsize);
  Mesh::size_type sz = nodemap.size();
  for (Mesh::index_type i = 0; i < sz; i++)
  {
    nodemap[i] = i;
  }

  // Compute the starting vertex weights.
  TSMesh::Face::iterator fi, fie;
  mesh->begin(fi);
  mesh->end(fie);
  while (fi != fie)
  {
    mesh->get_nodes(nodes, *fi);
    
    Point p[3];
    for (int i = 0; i < 3; i++)
    {
      mesh->get_point(p[i], nodes[i]);
    }

    // Compute Qface
    Vector n = Cross(p[1] - p[0], p[2] - p[0]);
    double d = - (n.x() * p[0].x() +
                  n.y() * p[0].y() +
                  n.z() * p[0].z());

    // Compute and apply the three weights;
    double weight[3];
    for (int i = 0; i < 3; i++)
    {
      // TODO  Determine which weighting scheme works best.
      weight[i] = 1.0 / n.length();

      DT_Quadric qface(n.x(), n.y(), n.z(), d);
      qface *= weight[i];

      qv[nodes[i]] += qface;
    }

    ++fi;
  }

  vector<dtripple> candidates;
  TSMesh::Edge::size_type esize;
  mesh->size(esize);
  candidates.reserve((size_t)esize);

  TSMesh::Edge::iterator ei, eie;
  mesh->begin(ei);
  mesh->end(eie);
  while (ei != eie)
  {
    mesh->get_nodes(nodes, *ei);
    
    // Add nodes[0], nodes[1] as candidate pair.
    dtripple cp(nodes[0], nodes[1]);

    DT_Quadric q;
    q += qv[cp.v0];
    q += qv[cp.v1];

    // Compute new point and it's error.
    cp.cost = q.find_cost_of_best_point();
    
    if (cp.cost < MAX_DOUBLE)
    {
      candidates.push_back(cp);
    }

    ++ei;
  }

  mesh = mesh->clone();

  size_t counter = candidates.size() / 2;
  for (unsigned int k = 0; k < counter; k++)
  {
    update_progress(k, counter);

    // Pull out the min value from the existing candidates.
    vector<dtripple>::iterator loc =
      std::min_element(candidates.begin(), candidates.end(), dtripple_compare);

    const unsigned int v0 = (*loc).v0;
    const unsigned int v1 = (*loc).v1;

    *loc = candidates[candidates.size()-1];
    candidates.pop_back();

    if (v0 == v1) continue;

    qv[v0] += qv[v1];

    // Collapse elem v0, v1
    //  set v0 point to pnew
    Point pnew;
    qv[v0].find_best_point(pnew);
    //mesh->collapse_edge(v0, v1);
    nodemap[v1] = v0;
    mesh->set_point(pnew, v0);

    for (size_t i = 0; i < candidates.size(); i++)
    {
      // Collapse the edges in our candidates lists.
      if (candidates[i].v0 == v1) candidates[i].v0 = v0;
      if (candidates[i].v1 == v1) candidates[i].v1 = v0;

      // Recompute all the costs in the pqueue.
      if (candidates[i].v0 == v0 || candidates[i].v1 == v0)
      {
        DT_Quadric q;
        q += qv[candidates[i].v0];
        q += qv[candidates[i].v1];

        // Compute new point and it's error.
        candidates[i].cost = q.find_cost_of_best_point();
      }
    }
  }
  
  squish_nodemap(nodemap);
  mesh->collapse_edges(nodemap);
  mesh->remove_obvious_degenerate_triangles();
  mesh->remove_orphan_nodes();

  GenericField<TSMesh, ConstantBasis<double>, vector<double> > *tsfield =
    new GenericField<TSMesh, ConstantBasis<double>, vector<double> >(mesh);
  ofield = tsfield;

  send_output_handle("Decimated", ofield);
}

} // End namespace SCIRun

