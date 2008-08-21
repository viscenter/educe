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



#include <vector>


#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Tensor.h>
#include <Core/Basis/NoData.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/CrvLinearLgn.h>
#include <Core/Basis/HexTrilinearLgn.h>
#include <Core/Basis/QuadBilinearLgn.h>
#include <Core/Datatypes/LatVolMesh.h>
#include <Core/Datatypes/ImageMesh.h>
#include <Core/Datatypes/ScanlineMesh.h>
#include <Core/Datatypes/GenericField.h>

#include <Core/Algorithms/Converter/FieldToNrrd.h>


namespace SCIRunAlgo {

using namespace SCIRun;

class FieldToNrrdAlgoT {
public:
  bool FieldToNrrd(ProgressReporter* pr,FieldHandle input, NrrdDataHandle& output);

  // Converters for node centered data
  template<class T>
  bool ScalarFieldToNrrd(ProgressReporter* pr,FieldHandle input, NrrdDataHandle& output, int datatype);
  bool VectorFieldToNrrd(ProgressReporter* pr,FieldHandle input, NrrdDataHandle& output);
  bool TensorFieldToNrrd(ProgressReporter* pr,FieldHandle input, NrrdDataHandle& output);

};


// Templated converter for Scalar data so we can use every type supported by the Teem library

template<class T>
bool FieldToNrrdAlgoT::ScalarFieldToNrrd(ProgressReporter* pr,FieldHandle input, NrrdDataHandle& output,int datatype)
{
  output = new NrrdData();
  output->nrrd_ = nrrdNew();
  
  if (output->nrrd_ == 0)
  {
    pr->error("FieldToNrrd: Could not create new Nrrd");
    return (false);      
  }

  // Get a pointer to make program more readable
  Nrrd* nrrd = output->nrrd_;

  typedef GenericField<LatVolMesh<HexTrilinearLgn<Point> >,HexTrilinearLgn<T>, FData3d<T,LatVolMesh<HexTrilinearLgn<Point> > > > LVN;
  typedef GenericField<ImageMesh<QuadBilinearLgn<Point> >,QuadBilinearLgn<T>, FData2d<T,ImageMesh<QuadBilinearLgn<Point> > > > IMN;
  typedef GenericField<ScanlineMesh<CrvLinearLgn<Point> >,CrvLinearLgn<T>, vector<T> > SLN;
  typedef GenericField<LatVolMesh<HexTrilinearLgn<Point> >,ConstantBasis<T>, FData3d<T,LatVolMesh<HexTrilinearLgn<Point> > > > LVE;
  typedef GenericField<ImageMesh<QuadBilinearLgn<Point> >,ConstantBasis<T>, FData2d<T,ImageMesh<QuadBilinearLgn<Point> > > > IME;
  typedef GenericField<ScanlineMesh<CrvLinearLgn<Point> >,ConstantBasis<T>, vector<T> > SLE;
  
  int nrrddim = 0;
  int nrrdcenter = nrrdCenterNode;
  size_t dim[3];
  Transform tf;

  LVN* LVN_Field = dynamic_cast<LVN*>(input.get_rep());
  if (LVN_Field)
  {
    // LatVolField with data on nodes;
    typename LVN::mesh_type* mesh = LVN_Field->get_typed_mesh().get_rep();
    typename LVN::mesh_type::Node::size_type sz;
    
    mesh->size(sz);
    nrrddim = 3; 
    dim[0] = static_cast<size_t>(sz.i_);
    dim[1] = static_cast<size_t>(sz.j_);
    dim[2] = static_cast<size_t>(sz.k_);
    nrrdAlloc_nva(nrrd,datatype,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    typename LVN::mesh_type::Node::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
    T* data = reinterpret_cast<T*>(nrrd->data);
    while (it != it_end)
    {
      T val;
      LVN_Field->value(val,*it);
      data[k] = val; k++;
      ++it;
    }
    
    nrrdcenter = nrrdCenterNode;
    tf = mesh->get_transform();
  }

  LVE* LVE_Field = dynamic_cast<LVE*>(input.get_rep());
  if (LVE_Field)
  {
    // LatVolField with data on nodes;
    typename LVE::mesh_type* mesh = LVE_Field->get_typed_mesh().get_rep();
    typename LVE::mesh_type::Elem::size_type sz;
    
    mesh->size(sz);
    nrrddim = 3; 
    dim[0] = static_cast<size_t>(sz.i_);
    dim[1] = static_cast<size_t>(sz.j_);
    dim[2] = static_cast<size_t>(sz.k_);
    nrrdAlloc_nva(nrrd,datatype,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    typename LVE::mesh_type::Elem::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
    
    T* data = reinterpret_cast<T*>(nrrd->data);
    while (it != it_end)
    {
      T val;
      LVE_Field->value(val,*it);
      data[k] = val; k++;
      ++it;
    }
    
    nrrdcenter = nrrdCenterCell;
    tf = mesh->get_transform();
  }


  IMN* IMN_Field = dynamic_cast<IMN*>(input.get_rep());
  if (IMN_Field)
  {
    // LatVolField with data on nodes;
    typename IMN::mesh_type* mesh = IMN_Field->get_typed_mesh().get_rep();
    typename IMN::mesh_type::Node::size_type sz;
    
    mesh->size(sz);
    nrrddim = 2; 
    dim[0] = static_cast<size_t>(sz.i_);
    dim[1] = static_cast<size_t>(sz.j_);
    nrrdAlloc_nva(nrrd,datatype,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    typename IMN::mesh_type::Node::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;

    T* data = reinterpret_cast<T*>(nrrd->data);
    while (it != it_end)
    {
      T val;
      IMN_Field->value(val,*it);
      data[k] = val; k++;
      ++it;

    }
    
    nrrdcenter = nrrdCenterNode;
    tf = mesh->get_transform();
  }

  IME* IME_Field = dynamic_cast<IME*>(input.get_rep());
  if (IME_Field)
  {
    // LatVolField with data on nodes;
    typename IME::mesh_type* mesh = IME_Field->get_typed_mesh().get_rep();
    typename IME::mesh_type::Elem::size_type sz;
    
    mesh->size(sz);
    nrrddim = 2;
    dim[0] = static_cast<size_t>(sz.i_);
    dim[1] = static_cast<size_t>(sz.j_);
    nrrdAlloc_nva(nrrd,datatype,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    typename IME::mesh_type::Elem::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;

    T* data = reinterpret_cast<T*>(nrrd->data);    
    while (it != it_end)
    {
      T val;
      IME_Field->value(val,*it);
      data[k] = val; k++;
      ++it;

    }
    
    nrrdcenter = nrrdCenterCell;
    tf = mesh->get_transform();
  }

  SLN* SLN_Field = dynamic_cast<SLN*>(input.get_rep());
  if (SLN_Field)
  {
    // LatVolField with data on nodes;
    typename SLN::mesh_type* mesh = SLN_Field->get_typed_mesh().get_rep();
    typename SLN::mesh_type::Node::size_type sz;
    
    mesh->size(sz);
    nrrddim = 1; 
    dim[0] = static_cast<size_t>(sz);
    nrrdAlloc_nva(nrrd,datatype,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    typename SLN::mesh_type::Node::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
    
    T* data = reinterpret_cast<T*>(nrrd->data);   
    while (it != it_end)
    {
      T val;
      SLN_Field->value(val,*it);
      data[k] = val; k++;
      ++it;

    }
    
    nrrdcenter = nrrdCenterNode;
    tf = mesh->get_transform();
  }

  SLE* SLE_Field = dynamic_cast<SLE*>(input.get_rep());
  if (SLE_Field)
  {
    // LatVolField with data on nodes;
    typename SLE::mesh_type* mesh = SLE_Field->get_typed_mesh().get_rep();
    typename SLE::mesh_type::Elem::size_type sz;
    
    mesh->size(sz);
    nrrddim = 1;
    dim[0] = static_cast<size_t>(sz);    
    nrrdAlloc_nva(nrrd,datatype,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    typename SLE::mesh_type::Elem::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
 
    T* data = reinterpret_cast<T*>(nrrd->data);      
    while (it != it_end)
    {
      T val;
      SLE_Field->value(val,*it);
      data[k] = val; k++;
      ++it;

    }
    
    nrrdcenter = nrrdCenterCell;
    tf = mesh->get_transform();
  }

  if (nrrddim)
  {
    // Set spacing information
    
    int centerdata[NRRD_DIM_MAX];
    for (int p=0;p<NRRD_DIM_MAX;p++)
    {
      centerdata[p] = nrrdcenter;
    }
    nrrdAxisInfoSet_nva(nrrd,nrrdAxisInfoCenter,centerdata); 

    int kind[NRRD_DIM_MAX];
    for (int p=0;p<NRRD_DIM_MAX;p++)
    {
      kind[p] = nrrdKindSpace;
    }
    nrrdAxisInfoSet_nva(nrrd,nrrdAxisInfoKind,kind);

    nrrd->spaceDim = 3;
    
    double Trans[16];
    tf.get(Trans);
    for (int p=0;p<3;p++)
    {
      nrrd->spaceOrigin[p] = Trans[3+4*p];
      for (int q=0;q<nrrddim;q++)
        nrrd->axis[q].spaceDirection[p] = Trans[q+4*p];
    }
    
    for (int p=0;p<3;p++)
      for (int q=0;q<3;q++)
      {
        if (p==q) nrrd->measurementFrame[p][q] = 1.0;
        else nrrd->measurementFrame[p][q] = 0.0;
      }
    
    nrrd->space = nrrdSpace3DRightHanded;

    return (true);
  }

  return (false);  
}



bool FieldToNrrdAlgoT::VectorFieldToNrrd(ProgressReporter* pr,FieldHandle input, NrrdDataHandle& output)
{
  output = new NrrdData();
  output->nrrd_ = nrrdNew();
  
  if (output->nrrd_ == 0)
  {
    pr->error("FieldToNrrd: Could not create new Nrrd");
    return (false);      
  }

  // Get a pointer to make program more readable
  Nrrd* nrrd = output->nrrd_;

  typedef GenericField<LatVolMesh<HexTrilinearLgn<Point> >,HexTrilinearLgn<Vector>, FData3d<Vector,LatVolMesh<HexTrilinearLgn<Point> > > > LVN;
  typedef GenericField<ImageMesh<QuadBilinearLgn<Point> >,QuadBilinearLgn<Vector>, FData2d<Vector,ImageMesh<QuadBilinearLgn<Point> > > > IMN;
  typedef GenericField<ScanlineMesh<CrvLinearLgn<Point> >,CrvLinearLgn<Vector>, vector<Vector> > SLN;
  typedef GenericField<LatVolMesh<HexTrilinearLgn<Point> >,ConstantBasis<Vector>, FData3d<Vector,LatVolMesh<HexTrilinearLgn<Point> > > > LVE;
  typedef GenericField<ImageMesh<QuadBilinearLgn<Point> >,ConstantBasis<Vector>, FData2d<Vector,ImageMesh<QuadBilinearLgn<Point> > > > IME;
  typedef GenericField<ScanlineMesh<CrvLinearLgn<Point> >,ConstantBasis<Vector>, vector<Vector> > SLE;
  
  int nrrddim = 0;
  int nrrdcenter = nrrdCenterNode;
  size_t dim[4];
  Transform tf;

  LVN* LVN_Field = dynamic_cast<LVN*>(input.get_rep());
  if (LVN_Field)
  {
    // LatVolField with data on nodes;
    LVN::mesh_type* mesh = LVN_Field->get_typed_mesh().get_rep();
    LVN::mesh_type::Node::size_type sz;
    
    mesh->size(sz);
    nrrddim = 4; dim[0] = 3;
    dim[1] = static_cast<size_t>(sz.i_);
    dim[2] = static_cast<size_t>(sz.j_);
    dim[3] = static_cast<size_t>(sz.k_);
    nrrdAlloc_nva(nrrd,nrrdTypeDouble,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    LVN::mesh_type::Node::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
 
    double* data = reinterpret_cast<double*>(nrrd->data);      
    while (it != it_end)
    {
      Vector v;
      LVN_Field->value(v,*it);
      data[k] = v.x(); k++;
      data[k] = v.y(); k++;
      data[k] = v.z(); k++;
      ++it;
    }
    
    nrrdcenter = nrrdCenterNode;
    tf = mesh->get_transform();
  }

  LVE* LVE_Field = dynamic_cast<LVE*>(input.get_rep());
  if (LVE_Field)
  {
    // LatVolField with data on nodes;
    LVE::mesh_type* mesh = LVE_Field->get_typed_mesh().get_rep();
    LVE::mesh_type::Elem::size_type sz;
    
    mesh->size(sz);
    nrrddim = 4; dim[0] = 3; 
    dim[1] = static_cast<size_t>(sz.i_);
    dim[2] = static_cast<size_t>(sz.j_);
    dim[3] = static_cast<size_t>(sz.k_);
    nrrdAlloc_nva(nrrd,nrrdTypeDouble,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    LVE::mesh_type::Elem::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
    double* data = reinterpret_cast<double*>(nrrd->data);      
     
    while (it != it_end)
    {
      Vector v;
      LVE_Field->value(v,*it);
      data[k] = v.x(); k++;
      data[k] = v.y(); k++;
      data[k] = v.z(); k++;
      ++it;
    }
    
    nrrdcenter = nrrdCenterCell;
    tf = mesh->get_transform();
  }


  IMN* IMN_Field = dynamic_cast<IMN*>(input.get_rep());
  if (IMN_Field)
  {
    // LatVolField with data on nodes;
    IMN::mesh_type* mesh = IMN_Field->get_typed_mesh().get_rep();
    IMN::mesh_type::Node::size_type sz;
    
    mesh->size(sz);
    nrrddim = 3; dim[0] = 3; 
    dim[1] = static_cast<size_t>(sz.i_);
    dim[2] = static_cast<size_t>(sz.j_);
    nrrdAlloc_nva(nrrd,nrrdTypeDouble,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    IMN::mesh_type::Node::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
    double* data = reinterpret_cast<double*>(nrrd->data);      
     
    while (it != it_end)
    {
      Vector v;
      IMN_Field->value(v,*it);
      data[k] = v.x(); k++;
      data[k] = v.y(); k++;
      data[k] = v.z(); k++;
      ++it;
    }
    
    nrrdcenter = nrrdCenterNode;
    tf = mesh->get_transform();
  }

  IME* IME_Field = dynamic_cast<IME*>(input.get_rep());
  if (IME_Field)
  {
    // LatVolField with data on nodes;
    IME::mesh_type* mesh = IME_Field->get_typed_mesh().get_rep();
    IME::mesh_type::Elem::size_type sz;
    
    mesh->size(sz);
    nrrddim = 3; dim[0] = 3; 
    dim[1] = static_cast<size_t>(sz.i_);
    dim[2] = static_cast<size_t>(sz.j_);
    nrrdAlloc_nva(nrrd,nrrdTypeDouble,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    IME::mesh_type::Elem::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
    
    double* data = reinterpret_cast<double*>(nrrd->data);      
    while (it != it_end)
    {
      Vector v;
      IME_Field->value(v,*it);
      data[k] = v.x(); k++;
      data[k] = v.y(); k++;
      data[k] = v.z(); k++;
      ++it;
    }
    
    nrrdcenter = nrrdCenterCell;
    tf = mesh->get_transform();
  }

  SLN* SLN_Field = dynamic_cast<SLN*>(input.get_rep());
  if (SLN_Field)
  {
    // LatVolField with data on nodes;
    SLN::mesh_type* mesh = SLN_Field->get_typed_mesh().get_rep();
    SLN::mesh_type::Node::size_type sz;
    
    mesh->size(sz);
    nrrddim = 2; dim[0] = 3; 
    dim[1] = static_cast<size_t>(sz);
    nrrdAlloc_nva(nrrd,nrrdTypeDouble,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    SLN::mesh_type::Node::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
 
    double* data = reinterpret_cast<double*>(nrrd->data);      
    while (it != it_end)
    {
      Vector v;
      SLN_Field->value(v,*it);
      data[k] = v.x(); k++;
      data[k] = v.y(); k++;
      data[k] = v.z(); k++;
      ++it;
    }
    
    nrrdcenter = nrrdCenterNode;
    tf = mesh->get_transform();
  }

  SLE* SLE_Field = dynamic_cast<SLE*>(input.get_rep());
  if (SLE_Field)
  {
    // LatVolField with data on nodes;
    SLE::mesh_type* mesh = SLE_Field->get_typed_mesh().get_rep();
    SLE::mesh_type::Elem::size_type sz;
    
    mesh->size(sz);
    nrrddim = 2; dim[0] = 3; 
    dim[1] = static_cast<size_t>(sz);
    nrrdAlloc_nva(nrrd,nrrdTypeDouble,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    SLE::mesh_type::Elem::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
    double* data = reinterpret_cast<double*>(nrrd->data);      
     
    while (it != it_end)
    {
      Vector v;
      SLE_Field->value(v,*it);
      data[k] = v.x(); k++;
      data[k] = v.y(); k++;
      data[k] = v.z(); k++;
      ++it;
    }
    
    nrrdcenter = nrrdCenterCell;
    tf = mesh->get_transform();
  }

  if (nrrddim)
  {
    // Set spacing information
    
    int centerdata[NRRD_DIM_MAX];
    for (int p=0;p<NRRD_DIM_MAX;p++)
    {
      centerdata[p] = nrrdcenter;
    }
    nrrdAxisInfoSet_nva(nrrd,nrrdAxisInfoCenter,centerdata); 

    int kind[NRRD_DIM_MAX];
    for (int p=0;p<NRRD_DIM_MAX;p++)
    {
      if (p==0) kind[p] = nrrdKind3Vector;
      else kind[p] = nrrdKindSpace;
    }
    nrrdAxisInfoSet_nva(nrrd,nrrdAxisInfoKind,kind);

    nrrd->spaceDim = 3;
    
    double Trans[16];
    tf.get(Trans);
    nrrd->axis[0].spaceDirection[0] = 0.0;
    nrrd->axis[0].spaceDirection[1] = 0.0;
    nrrd->axis[0].spaceDirection[2] = 0.0;
    
    for (int p=0;p<3;p++)
    {
      nrrd->spaceOrigin[p] = Trans[3+4*p];
      for (int q=0;q<nrrddim;q++)
        nrrd->axis[q+1].spaceDirection[p] = Trans[q+4*p];
    }
    
    for (int p=0;p<3;p++)
      for (int q=0;q<3;q++)
      {
        if (p==q) nrrd->measurementFrame[p][q] = 1.0;
        else nrrd->measurementFrame[p][q] = 0.0;
      }
    
    nrrd->space = nrrdSpace3DRightHanded;

    return (true);
  }

  return (false);  
}


bool FieldToNrrdAlgoT::TensorFieldToNrrd(ProgressReporter* pr,FieldHandle input, NrrdDataHandle& output)
{
  output = new NrrdData();
  output->nrrd_ = nrrdNew();
  
  if (output->nrrd_ == 0)
  {
    pr->error("FieldToNrrd: Could not create new Nrrd");
    return (false);      
  }

  // Get a pointer to make program more readable
  Nrrd* nrrd = output->nrrd_;

  typedef GenericField<LatVolMesh<HexTrilinearLgn<Point> >,HexTrilinearLgn<Tensor>, FData3d<Tensor,LatVolMesh<HexTrilinearLgn<Point> > > > LVN;
  typedef GenericField<ImageMesh<QuadBilinearLgn<Point> >,QuadBilinearLgn<Tensor>, FData2d<Tensor,ImageMesh<QuadBilinearLgn<Point> > > > IMN;
  typedef GenericField<ScanlineMesh<CrvLinearLgn<Point> >,CrvLinearLgn<Tensor>, vector<Tensor> > SLN;
  typedef GenericField<LatVolMesh<HexTrilinearLgn<Point> >,ConstantBasis<Tensor>, FData3d<Tensor,LatVolMesh<HexTrilinearLgn<Point> > > > LVE;
  typedef GenericField<ImageMesh<QuadBilinearLgn<Point> >,ConstantBasis<Tensor>, FData2d<Tensor,ImageMesh<QuadBilinearLgn<Point> > > > IME;
  typedef GenericField<ScanlineMesh<CrvLinearLgn<Point> >,ConstantBasis<Tensor>, vector<Tensor> > SLE;
  
  int nrrddim = 0;
  int nrrdcenter = nrrdCenterNode;
  size_t dim[4];
  Transform tf;

  LVN* LVN_Field = dynamic_cast<LVN*>(input.get_rep());
  if (LVN_Field)
  {
    // LatVolField with data on nodes;
    LVN::mesh_type* mesh = LVN_Field->get_typed_mesh().get_rep();
    LVN::mesh_type::Node::size_type sz;
    
    mesh->size(sz);
    nrrddim = 4; dim[0] = 6; 
    dim[1] = static_cast<size_t>(sz.i_);
    dim[2] = static_cast<size_t>(sz.j_);
    dim[3] = static_cast<size_t>(sz.k_);
    nrrdAlloc_nva(nrrd,nrrdTypeDouble,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    LVN::mesh_type::Node::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
    
    double* data = reinterpret_cast<double*>(nrrd->data);      
    while (it != it_end)
    {
      Tensor t;
      LVN_Field->value(t,*it);
      data[k] = t.mat_[0][0]; k++;
      data[k] = t.mat_[0][1]; k++;
      data[k] = t.mat_[0][2]; k++;
      data[k] = t.mat_[1][1]; k++;
      data[k] = t.mat_[1][2]; k++;
      data[k] = t.mat_[2][2]; k++;
      ++it;

    }
    
    nrrdcenter = nrrdCenterNode;
    tf = mesh->get_transform();
  }

  LVE* LVE_Field = dynamic_cast<LVE*>(input.get_rep());
  if (LVE_Field)
  {
    // LatVolField with data on nodes;
    LVE::mesh_type* mesh = LVE_Field->get_typed_mesh().get_rep();
    LVE::mesh_type::Elem::size_type sz;
    
    mesh->size(sz);
    nrrddim = 4; dim[0] = 6;
    dim[1] = static_cast<size_t>(sz.i_);
    dim[2] = static_cast<size_t>(sz.j_);
    dim[3] = static_cast<size_t>(sz.k_);
    nrrdAlloc_nva(nrrd,nrrdTypeDouble,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    LVE::mesh_type::Elem::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
    double* data = reinterpret_cast<double*>(nrrd->data);      
     
    while (it != it_end)
    {
      Tensor t;
      LVE_Field->value(t,*it);
      data[k] = t.mat_[0][0]; k++;
      data[k] = t.mat_[0][1]; k++;
      data[k] = t.mat_[0][2]; k++;
      data[k] = t.mat_[1][1]; k++;
      data[k] = t.mat_[1][2]; k++;
      data[k] = t.mat_[2][2]; k++;
      ++it;
    }
    
    nrrdcenter = nrrdCenterCell;
    tf = mesh->get_transform();
  }


  IMN* IMN_Field = dynamic_cast<IMN*>(input.get_rep());
  if (IMN_Field)
  {
    // LatVolField with data on nodes;
    IMN::mesh_type* mesh = IMN_Field->get_typed_mesh().get_rep();
    IMN::mesh_type::Node::size_type sz;
    
    mesh->size(sz);
    nrrddim = 3; dim[0] = 6; 
    dim[1] = static_cast<size_t>(sz.i_);
    dim[2] = static_cast<size_t>(sz.j_);
    nrrdAlloc_nva(nrrd,nrrdTypeDouble,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    IMN::mesh_type::Node::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
    double* data = reinterpret_cast<double*>(nrrd->data);      
     
    while (it != it_end)
    {
      Tensor t;
      IMN_Field->value(t,*it);
      data[k] = t.mat_[0][0]; k++;
      data[k] = t.mat_[0][1]; k++;
      data[k] = t.mat_[0][2]; k++;
      data[k] = t.mat_[1][1]; k++;
      data[k] = t.mat_[1][2]; k++;
      data[k] = t.mat_[2][2]; k++;
      ++it;
    }
    
    nrrdcenter = nrrdCenterNode;
    tf = mesh->get_transform();
  }

  IME* IME_Field = dynamic_cast<IME*>(input.get_rep());
  if (IME_Field)
  {
    // LatVolField with data on nodes;
    IME::mesh_type* mesh = IME_Field->get_typed_mesh().get_rep();
    IME::mesh_type::Elem::size_type sz;
    
    mesh->size(sz);
    nrrddim = 3; dim[0] = 6; 
    dim[1] = static_cast<size_t>(sz.i_);
    dim[2] = static_cast<size_t>(sz.j_);
    nrrdAlloc_nva(nrrd,nrrdTypeDouble,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    IME::mesh_type::Elem::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
    double* data = reinterpret_cast<double*>(nrrd->data);      
     
    while (it != it_end)
    {
      Tensor t;
      IME_Field->value(t,*it);
      data[k] = t.mat_[0][0]; k++;
      data[k] = t.mat_[0][1]; k++;
      data[k] = t.mat_[0][2]; k++;
      data[k] = t.mat_[1][1]; k++;
      data[k] = t.mat_[1][2]; k++;
      data[k] = t.mat_[2][2]; k++;
      ++it;
    }
    
    nrrdcenter = nrrdCenterCell;
    tf = mesh->get_transform();
  }

  SLN* SLN_Field = dynamic_cast<SLN*>(input.get_rep());
  if (SLN_Field)
  {
    // LatVolField with data on nodes;
    SLN::mesh_type* mesh = SLN_Field->get_typed_mesh().get_rep();
    SLN::mesh_type::Node::size_type sz;
    
    mesh->size(sz);
    nrrddim = 2; dim[0] = 6; 
    dim[1] = static_cast<size_t>(sz);
    nrrdAlloc_nva(nrrd,nrrdTypeDouble,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    SLN::mesh_type::Node::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
    double* data = reinterpret_cast<double*>(nrrd->data);      
    
    while (it != it_end)
    {
      Tensor t;
      SLN_Field->value(t,*it);
      data[k] = t.mat_[0][0]; k++;
      data[k] = t.mat_[0][1]; k++;
      data[k] = t.mat_[0][2]; k++;
      data[k] = t.mat_[1][1]; k++;
      data[k] = t.mat_[1][2]; k++;
      data[k] = t.mat_[2][2]; k++;
      ++it;
    }
    
    nrrdcenter = nrrdCenterNode;
    tf = mesh->get_transform();
  }

  SLE* SLE_Field = dynamic_cast<SLE*>(input.get_rep());
  if (SLE_Field)
  {
    // LatVolField with data on nodes;
    SLE::mesh_type* mesh = SLE_Field->get_typed_mesh().get_rep();
    SLE::mesh_type::Elem::size_type sz;
    
    mesh->size(sz);
    nrrddim = 2; dim[0] = 6;
    dim[1] = static_cast<size_t>(sz);
    nrrdAlloc_nva(nrrd,nrrdTypeDouble,nrrddim,dim);
  
    if (nrrd->data == 0)
    {
      pr->error("FieldToNrrd: Could not allocate enough space for new Nrrd");
      return (false);      
    }  

    SLE::mesh_type::Elem::iterator it, it_end;
    mesh->begin(it);
    mesh->end(it_end);
    size_t k = 0;
    double* data = reinterpret_cast<double*>(nrrd->data);      
     
    while (it != it_end)
    {
      Tensor t;
      SLE_Field->value(t,*it);
      data[k] = t.mat_[0][0]; k++;
      data[k] = t.mat_[0][1]; k++;
      data[k] = t.mat_[0][2]; k++;
      data[k] = t.mat_[1][1]; k++;
      data[k] = t.mat_[1][2]; k++;
      data[k] = t.mat_[2][2]; k++;
      ++it;
    }
    
    nrrdcenter = nrrdCenterCell;
    tf = mesh->get_transform();
  }

  if (nrrddim)
  {
    // Set spacing information
    
    int centerdata[NRRD_DIM_MAX];
    for (int p=0;p<NRRD_DIM_MAX;p++)
    {
      centerdata[p] = nrrdcenter;
    }
    nrrdAxisInfoSet_nva(nrrd,nrrdAxisInfoCenter,centerdata); 

    int kind[NRRD_DIM_MAX];
    for (int p=0;p<NRRD_DIM_MAX;p++)
    {
      if (p==0) kind[p] = nrrdKind3DSymMatrix;
      else kind[p] = nrrdKindSpace;
    }
    nrrdAxisInfoSet_nva(nrrd,nrrdAxisInfoKind,kind);

    nrrd->spaceDim = 3;
    
    double Trans[16];
    tf.get(Trans);
    nrrd->axis[0].spaceDirection[0] = 0.0;
    nrrd->axis[0].spaceDirection[1] = 0.0;
    nrrd->axis[0].spaceDirection[2] = 0.0;
    
    for (int p=0;p<3;p++)
    {
      nrrd->spaceOrigin[p] = Trans[3+4*p];
      for (int q=0;q<nrrddim;q++)
        nrrd->axis[q+1].spaceDirection[p] = Trans[q+4*p];
    }
    
    for (int p=0;p<3;p++)
      for (int q=0;q<3;q++)
      {
        if (p==q) nrrd->measurementFrame[p][q] = 1.0;
        else nrrd->measurementFrame[p][q] = 0.0;
      }
    
    nrrd->space = nrrdSpace3DRightHanded;

    return (true);
  }

  return (false);  
}

bool FieldToNrrdAlgo::FieldToNrrd(ProgressReporter *pr, FieldHandle input, NrrdDataHandle& output)
{

  if (input.get_rep() == 0)
  {
    pr->error("FieldToNrrd: No input Field");
    return (false);    
  } 

  FieldInformation fi(input);
  if (!(fi.is_scanline()||fi.is_image()||fi.is_latvol()))
  {
    pr->error("FieldToNrrd: The input field is not a ScanlineMesh, not an ImageMesh, nor a LatVolMesh.");
    pr->error("FieldToNrrd: Fields other than these three types cannot be converted. In order to store the field as a nrrd, one has to interpolate the field onto a regular grid.");
    return (false);
  }

  if (fi.is_nodata())
  {
    pr->error("FieldToNrrd: This Field cannot be converted as it does not contains any data");
    return (false);
  }

  if (fi.is_nonlineardata())
  {
    pr->error("FieldToNrrd: Non linear nrrdTypeDoubles are not supported by the Nrrd format.");
    return (false);
  }
  
  if (!fi.is_linearmesh())
  {
    pr->error("FieldToNrrd: Non linear nrrdTypeDoubles are not supported by the Nrrd format.");
    return (false);  
  }
  
  FieldToNrrdAlgoT algo;

  if (fi.is_scalar())
  {
    if (fi.get_data_type() == "double")  return(algo.ScalarFieldToNrrd<double>(pr,input,output,nrrdTypeDouble));
    if (fi.get_data_type() == "float")  return(algo.ScalarFieldToNrrd<float>(pr,input,output,nrrdTypeFloat));
    if (fi.get_data_type() == "char")  return(algo.ScalarFieldToNrrd<char>(pr,input,output,nrrdTypeChar));
    if (fi.get_data_type() == "unsigned char")  return(algo.ScalarFieldToNrrd<unsigned char>(pr,input,output,nrrdTypeUChar));
    if (fi.get_data_type() == "short")  return(algo.ScalarFieldToNrrd<short>(pr,input,output,nrrdTypeShort));
    if (fi.get_data_type() == "unsigned short")  return(algo.ScalarFieldToNrrd<unsigned short>(pr,input,output,nrrdTypeUShort));
    if (fi.get_data_type() == "int")  return(algo.ScalarFieldToNrrd<int>(pr,input,output,nrrdTypeInt));
    if (fi.get_data_type() == "unsigned int")  return(algo.ScalarFieldToNrrd<unsigned int>(pr,input,output,nrrdTypeUInt));
    if (fi.get_data_type() == "long long")  return(algo.ScalarFieldToNrrd<long long>(pr,input,output,nrrdTypeLLong));
    if (fi.get_data_type() == "unsigned long")  return(algo.ScalarFieldToNrrd<unsigned long long>(pr,input,output,nrrdTypeULLong));
    pr->error("FieldToNrrd: The field type is not supported by nrrd format, hence we cannot convert it");
    return (false);
  }
  
  if (fi.is_vector())
  {
    return(algo.VectorFieldToNrrd(pr,input,output));
  }
  
  if (fi.is_tensor())
  {
    return(algo.TensorFieldToNrrd(pr,input,output));
  }
  
  pr->error("FieldToNrrd: Unknown Field type encountered, cannot convert Field into Nrrd");
  return (false);
}

} // end namespace SCIRunAlgo


