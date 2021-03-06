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


//    File   : ClipLatVolByIndicesOrWidget.h
//    Author : Michael Callahan
//    Date   : August 2001

#if !defined(ClipLatVolByIndicesOrWidget_h)
#define ClipLatVolByIndicesOrWidget_h

#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <Core/Datatypes/Clipper.h>
#include <Core/Datatypes/NrrdData.h>
#include <sci_hash_map.h>
#include <algorithm>

//! for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRun {

class SCISHARE ClipLatVolByIndicesOrWidgetAlgo : public DynamicAlgoBase
{
public:
  typedef LatVolMesh<HexTrilinearLgn<Point> > LVMesh;
  virtual FieldHandle execute(FieldHandle fieldh,
			      const Point &a, const Point &b,
			      NrrdDataHandle nrrdh) = 0;

  //! support the dynamically compiled algorithm concept
  static CompileInfoHandle get_compile_info(const TypeDescription *fsrc);
};


template <class FIELD>
class ClipLatVolByIndicesOrWidgetAlgoT : public ClipLatVolByIndicesOrWidgetAlgo
{
public:
  //! virtual interface. 
  virtual FieldHandle execute(FieldHandle fieldh,
			      const Point &a, const Point &b,
			      NrrdDataHandle nrrdh);
};


template <class FIELD>
FieldHandle
ClipLatVolByIndicesOrWidgetAlgoT<FIELD>::execute(FieldHandle fieldh,
				 const Point &a, const Point &b,
				 NrrdDataHandle nrrdh)
{
  FIELD *lv = dynamic_cast<FIELD *>(fieldh.get_rep());
  LVMesh *omesh = lv->get_typed_mesh().get_rep();

  LVMesh::Node::index_type ns, ne;
  omesh->locate(ns, a);
  omesh->locate(ne, b);

  const int onx = omesh->get_ni();
  const int ony = omesh->get_nj();
  const int onz = omesh->get_nk();

  int si = (int)(ns.i_);
  int sj = (int)(ns.j_);
  int sk = (int)(ns.k_);
  int ei = (int)(ne.i_);
  int ej = (int)(ne.j_);
  int ek = (int)(ne.k_);

  if (si < 0) { si = 0; } 
  if (sj < 0) { sj = 0; } 
  if (sk < 0) { sk = 0; } 
  if (ei < 0) { ei = 0; } 
  if (ej < 0) { ej = 0; } 
  if (ek < 0) { ek = 0; } 

  if (si >= onx) { si = onx - 1; } 
  if (sj >= ony) { sj = ony - 1; } 
  if (sk >= onz) { sk = onz - 1; } 
  if (ei >= onx) { ei = onx - 1; } 
  if (ej >= ony) { ej = ony - 1; } 
  if (ek >= onz) { ek = onz - 1; } 

  LVMesh::Node::index_type s, e;

  if (si < ei) { s.i_ = si; e.i_ = ei; }
  else         { s.i_ = ei; e.i_ = si; }
  if (sj < ej) { s.j_ = sj; e.j_ = ej; }
  else         { s.j_ = ej; e.j_ = sj; }
  if (sk < ek) { s.k_ = sk; e.k_ = ek; }
  else         { s.k_ = ek; e.k_ = sk; }

  const int nx = e.i_ - s.i_ + 1;
  const int ny = e.j_ - s.j_ + 1;
  const int nz = e.k_ - s.k_ + 1;

  if (nx < 2 || ny < 2 || nz < 2) return 0;

  Point bmin(0.0, 0.0, 0.0);
  Point bmax(1.0, 1.0, 1.0);
  LVMesh *mesh = scinew LVMesh(nx, ny, nz, bmin, bmax);

  Transform trans = omesh->get_transform();
  trans.post_translate(Vector(s.i_, s.j_, s.k_));

  mesh->get_transform().load_identity();
  mesh->transform(trans);

  FIELD *fld = scinew FIELD(mesh);
  fld->copy_properties(lv);

  if (lv->basis_order() == 1)
  {
    LVMesh::Node::iterator si, ei;
    mesh->begin(si); mesh->end(ei);
    LVMesh::Node::size_type ns;
    omesh->size(ns);
    size_t dim[NRRD_DIM_MAX];
    dim[0] = ns;
    nrrdAlloc_nva(nrrdh->nrrd_, nrrdTypeUChar, 1, dim);
    unsigned char *mask = (unsigned char *)nrrdh->nrrd_->data;
    memset(mask, 0, dim[0]*sizeof(unsigned char));
    while (si != ei)
    {
      LVMesh::Node::index_type idx = *si;
      idx.i_ += s.i_;
      idx.j_ += s.j_;
      idx.k_ += s.k_;
      idx.mesh_ = omesh;
      mask[(unsigned int)idx] = 1;
      typename FIELD::value_type val;
      lv->value(val, idx);
      fld->set_value(val, *si);
    
      ++si;
    }
  }
  else if (lv->basis_order() == 0)
  {
    LVMesh::Cell::iterator si, ei;
    mesh->begin(si); mesh->end(ei);
    LVMesh::Cell::size_type ns;
    omesh->size(ns);
    size_t dim[NRRD_DIM_MAX];
    dim[0] = ns;
    nrrdAlloc_nva(nrrdh->nrrd_, nrrdTypeUChar, 1, dim);
    unsigned char *mask = (unsigned char *)nrrdh->nrrd_->data;
    memset(mask, 0, dim[0]*sizeof(unsigned char));
    while (si != ei)
    {
      LVMesh::Cell::index_type idx = *si;
      idx.i_ += s.i_;
      idx.j_ += s.j_;
      idx.k_ += s.k_;
      idx.mesh_ = omesh;
      mask[(unsigned int)idx] = 1;
      typename FIELD::value_type val;
      lv->value(val, idx);
      fld->set_value(val, *si);
    
      ++si;
    }
  }


  return fld;
}


} // end namespace SCIRun

#endif // ClipLatVolByIndicesOrWidget_h
