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
//    File   : SetMatIndex.cc
//    Author : Martin Cole
//    Date   : Fri Mar 14 11:41:43 2008

#include <Core/Basis/TetLinearLgn.h>
#include <Core/Basis/Constant.h>
#include <Core/Datatypes/TetVolMesh.h>
#include <Core/Datatypes/GenericField.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/NrrdPort.h>
#include <Core/Math/MiscMath.h>

namespace SCIRun {

typedef TetVolMesh<TetLinearLgn<Point> > TVMesh;
typedef GenericField<TVMesh, ConstantBasis<char>, vector<char> > TVField; 
typedef GenericField<TVMesh, ConstantBasis<double>, vector<double> > TVFieldD;

class SetMatIndex : public Module {
public:
  SetMatIndex(GuiContext* ctx);
  virtual ~SetMatIndex() {}
  virtual void execute();
};

DECLARE_MAKER(SetMatIndex)
SetMatIndex::SetMatIndex(GuiContext* ctx) : 
  Module("SetMatIndex", ctx, Filter, "NewField", "SCIRun")
{
}


void
SetMatIndex::execute()
{
  // Get input field.
  FieldHandle itv_h;
  if (!get_input_handle("Input TetVol", itv_h)) return;
  MeshHandle mh = itv_h->mesh();
  TVFieldD *inf = dynamic_cast<TVFieldD*>(itv_h.get_rep());
  if (! inf) {
    error("Input is not a TetVolField with double data type.");
    return;
  }
  TVMesh *m = dynamic_cast<TVMesh*>(mh.get_rep());
  if (! m) {
    error("Input is not a TetVol");
    return;
  }

  //! vector of inputs
  vector<NrrdDataHandle> nrrds;
  //! Get the handles from the module
  get_dynamic_input_handles("nrrds", nrrds, false);

  if (nrrds.size() <= 0) {
    error("must have at least one nrrd input.");
    return;
  }

  VField* vfield = itv_h->vfield();
  double minv, maxv;
  vfield->minmax(minv, maxv);
  
  //! spin through once and set the count of each material 
  //! for each input material index.
  vector<vector<int> > counts;
  counts.resize((int)maxv + 1);
  vector<vector<int> >::iterator citer = counts.begin();
  while (citer != counts.end()) {
    vector<int> &v = *citer;
    v.resize(nrrds.size());
    ++citer;
  }

  NrrdDataHandle nh0 = nrrds[0];
  int x_sz = nh0->nrrd_->axis[0].size;
  int y_sz = nh0->nrrd_->axis[1].size;
  int z_sz = nh0->nrrd_->axis[2].size;
  cerr << "x_sz: " << x_sz << ", y_sz: " << y_sz 
       << ", z_sz: " << z_sz << endl;

  TVField *f = new TVField(m);
  TVMesh::Elem::iterator iter, end;
  m->begin(iter);
  m->end(end);
  TVMesh::Elem::size_type nelems;
  m->size(nelems);

  int count = 0;
  while (iter != end) {
    TVMesh::Elem::index_type idx = *iter;
    
    double tmp;
    inf->value(tmp, idx);
    const int in_idx = int(tmp);

    Point c;
    m->get_center(c, idx);
    
    //! each sample nearer the nodes.
    ++iter;

    int i0 = (int)Round(c.x());
    int j0 = (int)Round(c.y());
    int k0 = (int)Round(c.z());
    int off0 = k0 * x_sz * y_sz + j0 * x_sz + i0;
 
    //! check each input nrrd for best fit.
    float v = -99999;
    int cur = 0;
    int midx = -1;
    vector<NrrdDataHandle>::iterator ni = nrrds.begin();
    while (ni != nrrds.end()) {
      Nrrd *n = (*ni)->nrrd_;
      float *d = (float*)n->data;
      float f = *(d + off0);
 
      if (f > v) {
        midx = cur;
        v = f;
      }
      ++cur;++ni;
    }

    counts[in_idx][midx]++;

    if (count % 100 == 0)
      update_progress((double)count/(double)nelems * 0.5);
    ++count;
  }
  update_progress(0.5);


  //! reuse first element for each index to cache the dominant index.
  citer = counts.begin();
  while (citer != counts.end()) {
    vector<int> &v = *citer;
    vector<int>::iterator it = v.begin();
    int i = 0;
    int largest = -1;
    while (it != v.end()) {
      if (*it > largest) {
        largest = *it;
        v[0] = i;
      }
      ++it; ++i;
    }
    ++citer;
  }

  //! set the values to indeces matching input nrrd order.
  m->begin(iter);
  m->end(end);
  count = 0;
  while (iter != end) {
    TVMesh::Elem::index_type idx = *iter;
    
    double tmp;
    inf->value(tmp, idx);
    const int in_idx = int(tmp);

    //! each sample nearer the nodes.
    ++iter;

    f->set_value(counts[in_idx][0], idx);

    if (count % 100 == 0)
      update_progress((double)count/(double)nelems * 0.5 + 0.5);
    ++count;
  }
  update_progress(1.0);

  FieldHandle ofld(f);
  send_output_handle("Output Indexed TetVol", ofld, true);
}

} // End namespace SCIRun

