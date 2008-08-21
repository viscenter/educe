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

#include <Core/Algorithms/Converter/NrrdToField.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class NrrdToFieldAlgoT {
public:
  // Convert a nrrd to a normal Scalar Field
  template<class T>
  bool NrrdToField(ProgressReporter* pr,NrrdDataHandle input, FieldHandle& output,std::string datalocation, std::string spaceparity);

  // Convert a nrrd to a Vector Field, we need to know which axis is the 
  // vector dimension, if -1 it will try to detect dimension automatically
  // Experience tells us mostly this information is not stored in the nrrd
  // So we need a method to set it.
  template<class T>
  bool NrrdToVectorField(ProgressReporter* pr,NrrdDataHandle input, FieldHandle& output,std::string datalocation, std::string spaceparity,int vecdim = -1);

  // Convert a nrrd to a Tensor Field, we need to know which axis is the 
  // vector dimension, if -1 it will try to detect dimension automatically
  // Experience tells us mostly this information is not stored in the nrrd
  // So we need a method to set it.
  template<class T>
  bool NrrdToTensorField(ProgressReporter* pr,NrrdDataHandle input, FieldHandle& output,std::string datalocation, std::string spaceparity,int tendim = -1);
};

// Templated converter for Scalar data so we can use every type supported by the Teem library
template<class T>
bool NrrdToFieldAlgoT::NrrdToField(ProgressReporter* pr,NrrdDataHandle input, FieldHandle& output,std::string datalocation, std::string spaceparity)
{
  Nrrd *nrrd = input->nrrd_;

  if (nrrd == 0)
  {
    pr->error("NrrdToField: NrrdData does not contain Nrrd");
    return (false);      
  }

  size_t dim = nrrd->dim;
  // Assume size of 4th+ dimension is 1.  Checked elsewhere.
  if (dim > 3) dim = 3;
  std::vector<double> min(dim), max(dim);
  std::vector<int> size(dim);
  
  for (size_t p=0; p<dim; p++) 
  {
    size[p] = nrrd->axis[p].size;
    
    if (airExists(nrrd->axis[p].min)) 
    {
      min[p] = nrrd->axis[p].min;
    }
    else
    {
      min[p] = 0.0;
    }
    
    if (airExists(nrrd->axis[p].max)) 
    {
      max[p] = nrrd->axis[p].max;
    }
    else
    {
      if (airExists(nrrd->axis[p].spacing)) 
      {
        if (datalocation == "Node")
        {
          max[p] = nrrd->axis[p].spacing*(size[p]-1);
        }
        else
        {
          max[p] = nrrd->axis[p].spacing*size[p];        
        }
      }
      else
      {
        if (datalocation == "Node")
        {
          max[p] = static_cast<double>(size[p]);
        }
        else
        {
          max[p] = static_cast<double>(size[p]-1);        
        }
      }
    }
  }  

  Transform tf;
  tf.load_identity();
  bool use_tf = false;

  Vector Origin;
  std::vector<Vector> SpaceDir(3);

  if (nrrd->spaceDim > 0)
  {
    int sd = nrrd->spaceDim;

    if (datalocation == "Node")
    {
      for (size_t p=0; p< dim; p++)   
      {
        min[p] = 0.0;
        max[p] = static_cast<double>(size[p]-1);
        if (nrrd->axis[p].center != 1)
        {
          double cor = (max[p]-min[p])/(2*(size[p]-1));
          min[p] += cor;
          max[p] += cor;
        }
      } 
    }
    else
    {
      for (size_t p=0; p< dim; p++)   
      {
        min[p] = 0.0;
        max[p] = static_cast<double>(size[p]);
        if (nrrd->axis[p].center == 1)
        {
          double cor = (max[p]-min[p])/(2*size[p]);
          min[p] -= cor;
          max[p] -= cor;
        }
      }     
    }    
    
    if (sd > 0)
    {
      if (airExists(nrrd->spaceOrigin[0])) Origin.x(nrrd->spaceOrigin[0]); else Origin.x(0.0);
      for (size_t p=0; p<dim && p < 3;p++)
        if (airExists(nrrd->axis[p].spaceDirection[0])) SpaceDir[p].x(nrrd->axis[p].spaceDirection[0]); else SpaceDir[p].x(0.0);
    }

    if (sd > 1)
    {
      if (airExists(nrrd->spaceOrigin[1])) Origin.y(nrrd->spaceOrigin[1]); else Origin.y(0.0);
      for (size_t p=0; p<dim && p < 3;p++)
        if (airExists(nrrd->axis[p].spaceDirection[1])) SpaceDir[p].y(nrrd->axis[p].spaceDirection[1]); else SpaceDir[p].y(0.0);
    }
    
    if (sd > 2)
    {
      if (airExists(nrrd->spaceOrigin[2])) Origin.z(nrrd->spaceOrigin[2]); else Origin.z(0.0);
      for (size_t p=0; p<dim && p < 3;p++)
        if (airExists(nrrd->axis[p].spaceDirection[2])) SpaceDir[p].z(nrrd->axis[p].spaceDirection[2]); else SpaceDir[p].z(0.0);
    }
  
    if (dim == 1) 
    {
      SpaceDir[0].find_orthogonal(SpaceDir[1],SpaceDir[2]);
    }
    else if (dim == 2)
    {
      SpaceDir[2] = Cross(SpaceDir[0],SpaceDir[1]);
    }
 
    tf.load_basis(Point(Origin),SpaceDir[0],SpaceDir[1],SpaceDir[2]);
  
    use_tf = true;
  }

  if (spaceparity == "Make Right Hand Sided")
  {
    // The Teem documentation is not specific on how deal deal with RHS and LHS
    // Hence we interpret the information as following:
    // (1) if a patient specific orientation is given, we check the parity of
    //     the space directions and the parity of the objective and convert
    //     if needed, i.e. either coord parity or space parity is LHS, then 
    //     we mirror.
    // (2) if ScannerXYZ is given, nothing is assumed about coord parity and 
    //     space parity, as it is not clear what has been defined.
    // (3) in case SpaceLeft3DHanded is given, we assume space parity is LHS and
    //     coord parity is not of importance. 
    
    bool reverseparity = false;
    bool coordparity = true;
    
    if (use_tf) coordparity = (Dot(SpaceDir[2],Cross(SpaceDir[0],SpaceDir[1])) >= 0);
    
    if ((nrrd->space == nrrdSpaceRightAnteriorSuperior)||
        (nrrd->space == nrrdSpaceLeftPosteriorSuperior)||
        (nrrd->space == nrrdSpaceRightAnteriorSuperiorTime) ||
        (nrrd->space == nrrdSpaceLeftPosteriorSuperiorTime))
    {
      if (coordparity == false) reverseparity = true; 
    }
  
    if ((nrrd->space == nrrdSpaceLeftAnteriorSuperior)||
        (nrrd->space == nrrdSpaceLeftAnteriorSuperiorTime))
    {
      if (coordparity == true) reverseparity = true;
    }
    
    if ((nrrd->space == nrrdSpace3DLeftHanded) ||
        (nrrd->space == nrrdSpace3DLeftHandedTime))
    {
      reverseparity = true;
    }
   
    if (reverseparity)
    {
      if (use_tf)
      {
        tf.load_basis(-Point(Origin),-SpaceDir[0],-SpaceDir[1],-SpaceDir[2]);
      }
      else
      {
        use_tf = true;
        tf.load_basis(Point(0.0,0.0,0.0),Vector(-1.0,0.0,0.0),Vector(0.0,-1.0,0.0),Vector(0.0,0.0,1.0));
      }
    }
  }
  else if (spaceparity == "Invert")
  {
    if (use_tf)
    {
      tf.load_basis(-Point(Origin),-SpaceDir[0],-SpaceDir[1],-SpaceDir[2]);
    }
    else
    {
      use_tf = true;
      tf.load_basis(Point(0.0,0.0,0.0),Vector(-1.0,0.0,0.0),Vector(0.0,-1.0,0.0),Vector(0.0,0.0,1.0));
    }
  }


  T* dataptr = static_cast<T*>(nrrd->data);
  
  if (dim == 1)
  {
    int k = 0;
    
    if (datalocation == "Node")
    {
      ScanlineMesh<CrvLinearLgn<Point> >::handle_type mesh_handle = new ScanlineMesh<CrvLinearLgn<Point> >(size[0],Point(min[0],0.0,0.0),Point(max[0],0.0,0.0));
      GenericField<ScanlineMesh<CrvLinearLgn<Point> >,CrvLinearLgn<T>, vector<T> >* field = new GenericField<ScanlineMesh<CrvLinearLgn<Point> >,CrvLinearLgn<T>,  vector<T> >(mesh_handle);
      output = dynamic_cast<Field *>(field);
      ScanlineMesh<CrvLinearLgn<Point> >::Node::iterator it, it_end;
      mesh_handle->begin(it);
      mesh_handle->end(it_end);
      while (it != it_end)
      {
        field->set_value(dataptr[k++],*it);
        ++it;
      }
      
      if (use_tf)
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }      
    }
    else if (datalocation == "Element")
    {
      ScanlineMesh<CrvLinearLgn<Point> >::handle_type mesh_handle = new ScanlineMesh<CrvLinearLgn<Point> >(size[0]+1,Point(min[0],0.0,0.0),Point(max[0],0.0,0.0));
      GenericField<ScanlineMesh<CrvLinearLgn<Point> >,ConstantBasis<T>, vector<T> >* field = new GenericField<ScanlineMesh<CrvLinearLgn<Point> >,ConstantBasis<T>, vector<T> >(mesh_handle);
      output = dynamic_cast<Field *>(field);
      ScanlineMesh<CrvLinearLgn<Point> >::Elem::iterator it, it_end;
      mesh_handle->begin(it);
      mesh_handle->end(it_end);
      while (it != it_end)
      {
        field->set_value(dataptr[k++],*it);
        ++it;
      }  
      if (use_tf)
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }      
    }
    else
    {
      pr->error("NrrdToField: Data location information is not recognized");
      return (false);      
    }  
  }
  else if (dim == 2)
  {
    int k = 0;
    
    if (datalocation == "Node")
    {
      ImageMesh<QuadBilinearLgn<Point> >::handle_type mesh_handle = new ImageMesh<QuadBilinearLgn<Point> >(size[0],size[1],Point(min[0],min[1],0.0),Point(max[0],max[1],0.0));
      GenericField<ImageMesh<QuadBilinearLgn<Point> >,QuadBilinearLgn<T>, FData2d<T, ImageMesh<QuadBilinearLgn<Point> > > >* field = new GenericField<ImageMesh<QuadBilinearLgn<Point> >,QuadBilinearLgn<T>, FData2d<T, ImageMesh<QuadBilinearLgn<Point> > > >(mesh_handle);
      output = dynamic_cast<Field *>(field);
      ImageMesh<QuadBilinearLgn<Point> >::Node::iterator it, it_end;
      mesh_handle->begin(it);
      mesh_handle->end(it_end);
      while (it != it_end)
      {
        field->set_value(dataptr[k++],*it);
        ++it;
      }
      
      if (use_tf)
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }      
    }
    else if (datalocation == "Element")
    {
      ImageMesh<QuadBilinearLgn<Point> >::handle_type mesh_handle = new ImageMesh<QuadBilinearLgn<Point> >(size[0]+1,size[1]+1,Point(min[0],min[1],0.0),Point(max[0],max[1],0.0));
      GenericField<ImageMesh<QuadBilinearLgn<Point> >,ConstantBasis<T>, FData2d<T, ImageMesh<QuadBilinearLgn<Point> > > >* field = new GenericField<ImageMesh<QuadBilinearLgn<Point> >,ConstantBasis<T>, FData2d<T, ImageMesh<QuadBilinearLgn<Point> > > >(mesh_handle);
      output = dynamic_cast<Field *>(field);
      ImageMesh<QuadBilinearLgn<Point> >::Elem::iterator it, it_end;
      mesh_handle->begin(it);
      mesh_handle->end(it_end);
      while (it != it_end)
      {
        field->set_value(dataptr[k++],*it);
        ++it;
      }  
      if (use_tf)
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }      
    }
    else
    {
      pr->error("NrrdToField: Data location information is not recognized");
      return (false);      
    }
  }
  else if (dim == 3)
  {
    int k = 0;
    
    if (datalocation == "Node")
    {
      LatVolMesh<HexTrilinearLgn<Point> >::handle_type mesh_handle = new LatVolMesh<HexTrilinearLgn<Point> >(size[0],size[1],size[2],Point(min[0],min[1],min[2]),Point(max[0],max[1],max[2]));
      GenericField<LatVolMesh<HexTrilinearLgn<Point> >,HexTrilinearLgn<T>, FData3d<T, LatVolMesh<HexTrilinearLgn<Point> > > >* field = new GenericField<LatVolMesh<HexTrilinearLgn<Point> >,HexTrilinearLgn<T>, FData3d<T, LatVolMesh<HexTrilinearLgn<Point> > > >(mesh_handle);
      output = dynamic_cast<Field *>(field);
      LatVolMesh<HexTrilinearLgn<Point> >::Node::iterator it, it_end;
      mesh_handle->begin(it);
      mesh_handle->end(it_end);
      while (it != it_end)
      {
        field->set_value(dataptr[k++],*it);
        ++it;
      }
      if (use_tf) 
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }
    }
    else if (datalocation == "Element")
    {
      LatVolMesh<HexTrilinearLgn<Point> >::handle_type mesh_handle = new LatVolMesh<HexTrilinearLgn<Point> >(size[0]+1,size[1]+1,size[2]+1,Point(min[0],min[1],min[2]),Point(max[0],max[1],max[2]));
      GenericField<LatVolMesh<HexTrilinearLgn<Point> >,ConstantBasis<T>, FData3d<T, LatVolMesh<HexTrilinearLgn<Point> > > >* field = new GenericField<LatVolMesh<HexTrilinearLgn<Point> >,ConstantBasis<T>, FData3d<T, LatVolMesh<HexTrilinearLgn<Point> > > >(mesh_handle);
      output = dynamic_cast<Field *>(field);
      LatVolMesh<HexTrilinearLgn<Point> >::Elem::iterator it, it_end;
      mesh_handle->begin(it);
      mesh_handle->end(it_end);
      while (it != it_end)
      {
        field->set_value(dataptr[k++],*it);
        ++it;
      }  
      if (use_tf)
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }            
    }
    else
    {
      pr->error("NrrdToField: Data location information is not recognized");
      return (false);      
    }
  }
  else
  {
    pr->error("NrrdToField: Nrrd is not 1D, 2D or 3D");
    return (false);        
  }
  
  return (true);
}


// Templated converter for Scalar data so we can use every type supported by the Teem library
template<class T>
bool NrrdToFieldAlgoT::NrrdToVectorField(ProgressReporter* pr,NrrdDataHandle input, FieldHandle& output,std::string datalocation, std::string spaceparity, int vecdim)
{
  Nrrd *nrrd = input->nrrd_;
  double M[3][3];
  M[0][0] = 1.0; M[0][1] = 0.0; M[0][2] = 0.0;
  M[1][0] = 0.0; M[1][1] = 1.0; M[1][2] = 0.0;
  M[2][0] = 0.0; M[2][1] = 0.0; M[2][2] = 1.0;

  if (nrrd == 0)
  {
    pr->error("NrrdToVectorField: NrrdData does not contain Nrrd");
    return (false);      
  }

  size_t dim = nrrd->dim;
  std::vector<double> min(dim), max(dim);
  std::vector<int> size(dim);
  
  for (size_t p=0; p<dim; p++) 
  {
    size[p] = nrrd->axis[p].size;
    
    if (airExists(nrrd->axis[p].min)) 
    {
      min[p] = nrrd->axis[p].min;
    }
    else
    {
      min[p] = 0.0;
    }
    
    if (airExists(nrrd->axis[p].max)) 
    {
      max[p] = nrrd->axis[p].max;
    }
    else
    {
      if (airExists(nrrd->axis[p].spacing)) 
      {
        if (datalocation == "Node")
        {
          max[p] = nrrd->axis[p].spacing*(size[p]-1);
        }
        else
        {
          max[p] = nrrd->axis[p].spacing*(size[p]);        
        }
      }
      else
      {
        if (datalocation == "Node")
        {
          max[p] = static_cast<double>(size[p]-1);
        }
        else
        {
          max[p] = static_cast<double>(size[p]);        
        }
      }
    }
  }  

  if (vecdim == -1)
  {
    for (size_t p=0; p<dim; p++)
      if (size[p] == 3) vecdim = p; 
  }

  if (vecdim < 0 || vecdim > ((int)dim-1))
  {
    pr->error("NrrdToVectorField: Nrrd does not contain a dimension that can be interpreted as a vector");
    return (false);        
  }
  
  if (size[vecdim] != 3)
  {
    pr->error("NrrdToVectorField: The vector dimension does not have a size of 3");
    return (false);          
  }

  Transform tf;
  tf.load_identity();
  bool use_tf = false;

  size_t rdim = (nrrd->dim-1);
  std::vector<double> rmin(rdim), rmax(rdim);
  std::vector<int> rsize(rdim);
  
  size_t k =0;
  for (size_t p=0; p < dim; p++)   
  {
     if (p == (size_t)vecdim) continue;
     rmin[k] = min[p];
     rmax[k] = max[p];
     rsize[k] = size[p];
     k++;
  }
  
  Vector Origin;
  std::vector<Vector> SpaceDir(3);

  if (nrrd->spaceDim > 0)
  {
    int sd = nrrd->spaceDim;
    for (int q=0 ; q<(sd+1) && q < 3; q++)
    {
      if (airExists(nrrd->spaceOrigin[q])) Origin[q] = nrrd->spaceOrigin[q]; else Origin[q] = 0.0;
      int r = 0;
      for (size_t p=0; p<dim && r < 3;p++)
      {
        if (p == (size_t)vecdim) continue;
        if (airExists(nrrd->axis[p].spaceDirection[q])) SpaceDir[r][q] = nrrd->axis[p].spaceDirection[q]; else SpaceDir[r][q] = 0.0;
        r++;
      }
    }
  
    if (datalocation == "Node")
    {
      k =0;
      for (size_t p=0; p< dim; p++)   
      {
        if (p == (size_t)vecdim) continue;
        rmin[k] = 0.0;
        rmax[k] = static_cast<double>(size[p]-1);
        rsize[k] = size[p];
        if (nrrd->axis[p].center != 1)
        {
          double cor = (rmax[k]-rmin[k])/(2*(rsize[k]-1));
          rmin[k] += cor;
          rmax[k] += cor;
        }
        k++;
      } 
    }
    else
    {
      k =0;
      for (size_t p=0; p< dim; p++)   
      {
        if (p == (size_t)vecdim) continue;
        rmin[k] = 0.0;
        rmax[k] = static_cast<double>(size[p]);
        rsize[k] = size[p];
        if (nrrd->axis[p].center == 1)
        {
          double cor = (rmax[k]-rmin[k])/(2*rsize[k]);
          rmin[k] -= cor;
          rmax[k] -= cor;
        }
        k++;
      }     
    }
  
    if (rdim == 1) 
    {
      SpaceDir[0].find_orthogonal(SpaceDir[1],SpaceDir[2]);
    }
    else if (rdim == 2)
    {
      SpaceDir[2] = Cross(SpaceDir[0],SpaceDir[1]);
    }

    tf.load_basis(Point(Origin),SpaceDir[0],SpaceDir[1],SpaceDir[2]);
  
    use_tf = true;
  
  
  
    if (airExists(nrrd->measurementFrame[0][0]))
    {
      for(int p=0; p<sd;p++)
        for(int q=0; q<sd; q++)
          M[q][p] = nrrd->measurementFrame[p][q];
    }
  }

  if (spaceparity == "Make Right Hand Sided")
  {
    // The Teem documentation is not specific on how deal deal with RHS and LHS
    // Hence we interpret the information as following:
    // (1) if a patient specific orientation is given, we check the parity of
    //     the space directions and the parity of the objective and convert
    //     if needed, i.e. either coord parity or space parity is LHS, then 
    //     we mirror.
    // (2) if ScannerXYZ is given, nothing is assumed about coord parity and 
    //     space parity, as it is not clear what has been defined.
    // (3) in case SpaceLeft3DHanded is given, we assume space parity is LHS and
    //     coord parity is not of importance. 
    
    bool reverseparity = false;
    bool coordparity = true;
    
    if (use_tf) coordparity = (Dot(SpaceDir[2],Cross(SpaceDir[0],SpaceDir[1])) >= 0);
    
    if ((nrrd->space == nrrdSpaceRightAnteriorSuperior)||
        (nrrd->space == nrrdSpaceLeftPosteriorSuperior)||
        (nrrd->space == nrrdSpaceRightAnteriorSuperiorTime) ||
        (nrrd->space == nrrdSpaceLeftPosteriorSuperiorTime))
    {
      if (coordparity == false) reverseparity = true; 
    }
  
    if ((nrrd->space == nrrdSpaceLeftAnteriorSuperior)||
        (nrrd->space == nrrdSpaceLeftAnteriorSuperiorTime))
    {
      if (coordparity == true) reverseparity = true;
    }
    
    if ((nrrd->space == nrrdSpace3DLeftHanded) ||
        (nrrd->space == nrrdSpace3DLeftHandedTime))
    {
      reverseparity = true;
    }
    
    if (reverseparity)
    {
      if (use_tf)
      {
        tf.load_basis(-Point(Origin),-SpaceDir[0],-SpaceDir[1],-SpaceDir[2]);
      }
      else
      {
        use_tf = true;
        tf.load_basis(Point(0.0,0.0,0.0),Vector(-1.0,0.0,0.0),Vector(0.0,-1.0,0.0),Vector(0.0,0.0,1.0));
      }
    }
  }
  else if (spaceparity == "Invert")
  {
    if (use_tf)
    {
      tf.load_basis(-Point(Origin),-SpaceDir[0],-SpaceDir[1],-SpaceDir[2]);
    }
    else
    {
      use_tf = true;
      tf.load_basis(Point(0.0,0.0,0.0),Vector(-1.0,0.0,0.0),Vector(0.0,-1.0,0.0),Vector(0.0,0.0,1.0));
    }
  }


  T* dataptr = static_cast<T*>(nrrd->data);

  // Calculate where the data is coming from 
  size_t vector_offset = 0;
  std::vector<size_t> space_offset(rdim);
  std::vector<size_t> space_size(rdim);
  
  k = 1;
  int m =0;
  for (int p=0; p<static_cast<int>(dim);p++)
  {
    if (vecdim == p) 
    {
      vector_offset = k;
      k *= size[p];
    }
    else 
    {
      space_offset[m] = k;
      k *= size[p];
      space_size[m] = k;
      m++;
    }
  }
 
  // Build a 2d Image
  if (rdim == 1)
  {
    if (datalocation == "Node")
    {
      ScanlineMesh<CrvLinearLgn<Point> >::handle_type mesh_handle = new ScanlineMesh<CrvLinearLgn<Point> >(rsize[0],Point(rmin[0],0.0,0.0),Point(rmax[0],0.0,0.0));
      GenericField<ScanlineMesh<CrvLinearLgn<Point> >,CrvLinearLgn<Vector>, vector<Vector> >* field = new GenericField<ScanlineMesh<CrvLinearLgn<Point> >,CrvLinearLgn<Vector>, vector<Vector> >(mesh_handle);
      output = dynamic_cast<Field *>(field);

      ScanlineMesh<CrvLinearLgn<Point> >::Node::iterator it;
      mesh_handle->begin(it);
      for (size_t x=0; x<space_size[0]; x+= space_offset[0])
      {
        const double v1 = static_cast<double>(dataptr[x]);
        const double v2 = static_cast<double>(dataptr[x+vector_offset]);
        const double v3 = static_cast<double>(dataptr[x+2*vector_offset]);
        Vector v(M[0][0]*v1+M[0][1]*v2+M[0][2]*v3,M[1][0]*v1+M[1][1]*v2+M[1][2]*v3,M[2][0]*v1+M[2][1]*v2+M[2][2]*v3);
        field->set_value(v,*it);
        ++it;
      }
      
      if (use_tf)
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }      
    }
    else if (datalocation == "Element")
    {
      ScanlineMesh<CrvLinearLgn<Point> >::handle_type mesh_handle = new ScanlineMesh<CrvLinearLgn<Point> >(rsize[0]+1,Point(rmin[0],0.0,0.0),Point(rmax[0],0.0,0.0));
      GenericField<ScanlineMesh<CrvLinearLgn<Point> >,ConstantBasis<Vector>, vector<Vector> >* field = new GenericField<ScanlineMesh<CrvLinearLgn<Point> >,ConstantBasis<Vector>, vector<Vector> >(mesh_handle);
      output = dynamic_cast<Field *>(field);

      ScanlineMesh<CrvLinearLgn<Point> >::Elem::iterator it;
      mesh_handle->begin(it);
      for (size_t x=0; x<space_size[0]; x+= space_offset[0])
      {
        const double v1 = static_cast<double>(dataptr[x]);
        const double v2 = static_cast<double>(dataptr[x+vector_offset]);
        const double v3 = static_cast<double>(dataptr[x+2*vector_offset]);
        Vector v(M[0][0]*v1+M[0][1]*v2+M[0][2]*v3,M[1][0]*v1+M[1][1]*v2+M[1][2]*v3,M[2][0]*v1+M[2][1]*v2+M[2][2]*v3);
        field->set_value(v,*it);
        ++it;
      }

      if (use_tf)
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }      
    }
    else
    {
      pr->error("NrrdToVectorField: Data location information is not recognized");
      return (false);      
    }
  
  }
  else if (rdim == 2)
  {
    if (datalocation == "Node")
    {
      ImageMesh<QuadBilinearLgn<Point> >::handle_type mesh_handle = new ImageMesh<QuadBilinearLgn<Point> >(rsize[0],rsize[1],Point(rmin[0],rmin[1],0.0),Point(rmax[0],rmax[1],0.0));
      GenericField<ImageMesh<QuadBilinearLgn<Point> >,QuadBilinearLgn<Vector>, FData2d<Vector, ImageMesh<QuadBilinearLgn<Point> > > >* field = new GenericField<ImageMesh<QuadBilinearLgn<Point> >,QuadBilinearLgn<Vector>, FData2d<Vector, ImageMesh<QuadBilinearLgn<Point> > > >(mesh_handle);
      output = dynamic_cast<Field *>(field);

      ImageMesh<QuadBilinearLgn<Point> >::Node::iterator it;
      mesh_handle->begin(it);
      for (size_t y=0; y<space_size[1]; y+=space_offset[1])
      {
        for (size_t x=0; x<space_size[0]; x+= space_offset[0])
        {
          size_t a = x+y;
          const double v1 = static_cast<double>(dataptr[a]);
          const double v2 = static_cast<double>(dataptr[a+vector_offset]);
          const double v3 = static_cast<double>(dataptr[a+2*vector_offset]);
          Vector v(M[0][0]*v1+M[0][1]*v2+M[0][2]*v3,M[1][0]*v1+M[1][1]*v2+M[1][2]*v3,M[2][0]*v1+M[2][1]*v2+M[2][2]*v3);
          field->set_value(v,*it);
          ++it;
        }
      }
      
      if (use_tf)
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }
    }
    else if (datalocation == "Element")
    {
      ImageMesh<QuadBilinearLgn<Point> >::handle_type mesh_handle = new ImageMesh<QuadBilinearLgn<Point> >(rsize[0]+1,rsize[1]+1,Point(rmin[0],rmin[1],0.0),Point(rmax[0],rmax[1],0.0));
      GenericField<ImageMesh<QuadBilinearLgn<Point> >,ConstantBasis<Vector>, FData2d<Vector, ImageMesh<QuadBilinearLgn<Point> > > >* field = new GenericField<ImageMesh<QuadBilinearLgn<Point> >,ConstantBasis<Vector>, FData2d<Vector, ImageMesh<QuadBilinearLgn<Point> > > >(mesh_handle);
      output = dynamic_cast<Field *>(field);

      ImageMesh<QuadBilinearLgn<Point> >::Elem::iterator it;
      mesh_handle->begin(it);
      for (size_t y=0; y<space_size[1]; y+=space_offset[1])
      {
        for (size_t x=0; x<space_size[0]; x+= space_offset[0])
        {
          size_t a = x+y;
          const double v1 = static_cast<double>(dataptr[a]);
          const double v2 = static_cast<double>(dataptr[a+vector_offset]);
          const double v3 = static_cast<double>(dataptr[a+2*vector_offset]);
          Vector v(M[0][0]*v1+M[0][1]*v2+M[0][2]*v3,M[1][0]*v1+M[1][1]*v2+M[1][2]*v3,M[2][0]*v1+M[2][1]*v2+M[2][2]*v3);
          field->set_value(v,*it);
          ++it;
        }
      }

      if (use_tf)
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }
    }
    else
    {
      pr->error("NrrdToVectorField: Data location information is not recognized");
      return (false);      
    }
  }
  else if (rdim == 3)
  {
    if (datalocation == "Node")
    {
      LatVolMesh<HexTrilinearLgn<Point> >::handle_type mesh_handle = new LatVolMesh<HexTrilinearLgn<Point> >(rsize[0],rsize[1],rsize[2],Point(rmin[0],rmin[1],rmin[2]),Point(rmax[0],rmax[1],rmax[2]));
      GenericField<LatVolMesh<HexTrilinearLgn<Point> >,HexTrilinearLgn<Vector>, FData3d<Vector, LatVolMesh<HexTrilinearLgn<Point> > > >* field = new GenericField<LatVolMesh<HexTrilinearLgn<Point> >,HexTrilinearLgn<Vector>, FData3d<Vector, LatVolMesh<HexTrilinearLgn<Point> > > >(mesh_handle);
      output = dynamic_cast<Field *>(field);
      
      LatVolMesh<HexTrilinearLgn<Point> >::Node::iterator it;
      mesh_handle->begin(it);
      for (size_t z=0; z<space_size[2]; z+=space_offset[2])
      {
        for (size_t y=0; y<space_size[1]; y+=space_offset[1])
        {
          for (size_t x=0; x<space_size[0]; x+= space_offset[0])
          {
            size_t a = x+y+z;
            const double v1 = static_cast<double>(dataptr[a]);
            const double v2 = static_cast<double>(dataptr[a+vector_offset]);
            const double v3 = static_cast<double>(dataptr[a+2*vector_offset]);
            Vector v(M[0][0]*v1+M[0][1]*v2+M[0][2]*v3,M[1][0]*v1+M[1][1]*v2+M[1][2]*v3,M[2][0]*v1+M[2][1]*v2+M[2][2]*v3);
            field->set_value(v,*it);
            ++it;
          }
        }
      }

      if (use_tf)
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }
    }
    else if (datalocation == "Element")
    {
      LatVolMesh<HexTrilinearLgn<Point> >::handle_type mesh_handle = new LatVolMesh<HexTrilinearLgn<Point> >(rsize[0]+1,rsize[1]+1,rsize[2]+1,Point(rmin[0],rmin[1],rmin[2]),Point(rmax[0],rmax[1],rmax[2]));
      GenericField<LatVolMesh<HexTrilinearLgn<Point> >,ConstantBasis<Vector>, FData3d<Vector, LatVolMesh<HexTrilinearLgn<Point> > > >* field = new GenericField<LatVolMesh<HexTrilinearLgn<Point> >,ConstantBasis<Vector>, FData3d<Vector, LatVolMesh<HexTrilinearLgn<Point> > > >(mesh_handle);
      output = dynamic_cast<Field *>(field);


      LatVolMesh<HexTrilinearLgn<Point> >::Elem::iterator it;
      mesh_handle->begin(it);
      for (size_t z=0; z<space_size[2]; z+=space_offset[2])
      {
        for (size_t y=0; y<space_size[1]; y+=space_offset[1])
        {
          for (size_t x=0; x<space_size[0]; x+= space_offset[0])
          {
            size_t a = x+y+z;
            const double v1 = static_cast<double>(dataptr[a]);
            const double v2 = static_cast<double>(dataptr[a+vector_offset]);
            const double v3 = static_cast<double>(dataptr[a+2*vector_offset]);
            Vector v(M[0][0]*v1+M[0][1]*v2+M[0][2]*v3,M[1][0]*v1+M[1][1]*v2+M[1][2]*v3,M[2][0]*v1+M[2][1]*v2+M[2][2]*v3);
            field->set_value(v,*it);
            ++it;
          }
        }
      }
      
      if (use_tf)
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }      
    }
    else
    {
      pr->error("NrrdToVectorField: Data location information is not recognized");
      return (false);      
    }
  }
  else
  {
    pr->error("NrrdToVectorField: Nrrd is not 2D or 3D");
    return (false);        
  }
  
  return (true);
}


// Templated converter for Scalar data so we can use every type supported by the Teem library
template<class T>
bool NrrdToFieldAlgoT::NrrdToTensorField(ProgressReporter* pr,NrrdDataHandle input, FieldHandle& output,std::string datalocation, std::string spaceparity, int vecdim)
{
  Nrrd *nrrd = input->nrrd_;

  double M[3][3];
  M[0][0] = 1.0; M[0][1] = 0.0; M[0][2] = 0.0;
  M[1][0] = 0.0; M[1][1] = 1.0; M[1][2] = 0.0;
  M[2][0] = 0.0; M[2][1] = 0.0; M[2][2] = 1.0;
  
  if (nrrd == 0)
  {
    pr->error("NrrdToTensorField: NrrdData does not contain Nrrd");
    return (false);      
  }

  size_t dim = nrrd->dim;
  std::vector<double> min(dim), max(dim);
  std::vector<int> size(dim);
  
  for (size_t p=0; p<dim; p++) 
  {
    size[p] = nrrd->axis[p].size;
    
    if (airExists(nrrd->axis[p].min)) 
    {
      min[p] = nrrd->axis[p].min;
    }
    else
    {
      min[p] = 0.0;
    }
    
    if (airExists(nrrd->axis[p].max)) 
    {
      max[p] = nrrd->axis[p].max;
    }
    else
    {
      if (airExists(nrrd->axis[p].spacing)) 
      {
        if (datalocation == "Node")
        {      
          max[p] = nrrd->axis[p].spacing*(size[p]-1);
        }
        else
        {
          max[p] = nrrd->axis[p].spacing*(size[p]);        
        }
      }
      else
      {
        if (datalocation == "Node")
        {              
          max[p] = static_cast<double>(size[p]-1);
        }
        else
        {
          max[p] = static_cast<double>(size[p]);        
        }
      }
    }
  }  

  if (vecdim == -1)
  {
    for (size_t p=0; p<dim; p++)
      if ((size[p] == 6)||(size[p]==9)) vecdim = p; 
  }

  if (vecdim == -1)
  {
    for (size_t p=0; p<dim; p++)
      if ((size[p] == 7)||(size[p]==10)) vecdim = p; 
  }
  
  if (vecdim < 0 || vecdim > ((int)dim-1))
  {
    pr->error("NrrdToTensorField: Nrrd does not contain a dimension that can be interpreted as a vector");
    return (false);        
  }

  if ((size[vecdim] != 6)&&(size[vecdim] != 9)&&(size[vecdim] != 7)&&(size[vecdim] != 10))
  {
    pr->error("NrrdToTensorField: The tensor dimension does not have a size of 6, 7, 9, or 10 elements");
    return (false);          
  }

  Transform tf;
  tf.load_identity();
  bool use_tf = false;

  size_t rdim = (nrrd->dim-1);
  std::vector<double> rmin(rdim), rmax(rdim);
  std::vector<int> rsize(rdim);
  
  size_t k =0;
  for (size_t p=0; p< dim; p++)   
  {
   if (p == (size_t)vecdim) continue;
   rmin[k] = min[p];
   rmax[k] = max[p];
   rsize[k] = size[p];
   k++;
  }
  
  Vector Origin;
  std::vector<Vector> SpaceDir(3);
  
  
  if (nrrd->spaceDim > 0)
  {
    int sd = nrrd->spaceDim;
  
    for (int q=0 ; q<(sd+1) && q < 3; q++)
    {
      if (airExists(nrrd->spaceOrigin[q])) Origin[q] = nrrd->spaceOrigin[q]; else Origin[q] = 0.0;
      int r = 0;
      for (size_t p=0; p<dim && r < 3;p++)
      {
        if (p == (size_t)vecdim) continue;
        if (airExists(nrrd->axis[p].spaceDirection[q])) SpaceDir[r][q] = nrrd->axis[p].spaceDirection[q]; else SpaceDir[r][q] = 0.0;
        r++;
      }
    }
  
     if (datalocation == "Node")
    {
      k =0;
      for (size_t p=0; p< dim; p++)   
      {
        if (p == (size_t)vecdim) continue;
        rmin[k] = 0.0;
        rmax[k] = static_cast<double>(size[p]-1);
        rsize[k] = size[p];
        if (nrrd->axis[p].center != 1)
        {
          double cor = (rmax[k]-rmin[k])/(2*(rsize[k]-1));
          rmin[k] += cor;
          rmax[k] += cor;
        }
        k++;
      } 
    }
    else
    {
      k =0;
      for (size_t p=0; p< dim; p++)   
      {
        if (p == (size_t)vecdim) continue;
        rmin[k] = 0.0;
        rmax[k] = static_cast<double>(size[p]);
        rsize[k] = size[p];
        if (nrrd->axis[p].center == 1)
        {
          double cor = (rmax[k]-rmin[k])/(2*rsize[k]);
          rmin[k] -= cor;
          rmax[k] -= cor;
        }
        k++;
      }     
    } 
  
    if (rdim == 1) 
    {
      SpaceDir[0].find_orthogonal(SpaceDir[1],SpaceDir[2]);
    }
    else if (rdim == 2)
    {
      SpaceDir[2] = Cross(SpaceDir[0],SpaceDir[1]);
    }

    tf.load_basis(Point(Origin),SpaceDir[0],SpaceDir[1],SpaceDir[2]);
  
    use_tf = true;
    
    if (airExists(nrrd->measurementFrame[0][0]))
    {
      for(int p=0; p<sd;p++)
        for(int q=0; q<sd; q++)
          M[q][p] = nrrd->measurementFrame[p][q];
    }    
  }

  if (spaceparity == "Make Right Hand Sided")
  {
    // The Teem documentation is not specific on how deal deal with RHS and LHS
    // Hence we interpret the information as following:
    // (1) if a patient specific orientation is given, we check the parity of
    //     the space directions and the parity of the objective and convert
    //     if needed, i.e. either coord parity or space parity is LHS, then 
    //     we mirror.
    // (2) if ScannerXYZ is given, nothing is assumed about coord parity and 
    //     space parity, as it is not clear what has been defined.
    // (3) in case SpaceLeft3DHanded is given, we assume space parity is LHS and
    //     coord parity is not of importance. 
    
    bool reverseparity = false;
    bool coordparity = true;
    
    if (use_tf) coordparity = (Dot(SpaceDir[2],Cross(SpaceDir[0],SpaceDir[1])) >= 0);
    
    if ((nrrd->space == nrrdSpaceRightAnteriorSuperior)||
        (nrrd->space == nrrdSpaceLeftPosteriorSuperior)||
        (nrrd->space == nrrdSpaceRightAnteriorSuperiorTime) ||
        (nrrd->space == nrrdSpaceLeftPosteriorSuperiorTime))
    {
      if (coordparity == false) reverseparity = true; 
    }
  
    if ((nrrd->space == nrrdSpaceLeftAnteriorSuperior)||
        (nrrd->space == nrrdSpaceLeftAnteriorSuperiorTime))
    {
      if (coordparity == true) reverseparity = true;
    }
    
    if ((nrrd->space == nrrdSpace3DLeftHanded) ||
        (nrrd->space == nrrdSpace3DLeftHandedTime))
    {
      reverseparity = true;
    }
    
    if (reverseparity)
    {
      if (use_tf)
      {
        tf.load_basis(-Point(Origin),-SpaceDir[0],-SpaceDir[1],-SpaceDir[2]);
      }
      else
      {
        use_tf = true;
        tf.load_basis(Point(0.0,0.0,0.0),Vector(-1.0,0.0,0.0),Vector(0.0,-1.0,0.0),Vector(0.0,0.0,1.0));
      }
    }
  }
  else if (spaceparity == "Invert")
  {
    if (use_tf)
    {
      tf.load_basis(-Point(Origin),-SpaceDir[0],-SpaceDir[1],-SpaceDir[2]);
    }
    else
    {
      use_tf = true;
      tf.load_basis(Point(0.0,0.0,0.0),Vector(-1.0,0.0,0.0),Vector(0.0,-1.0,0.0),Vector(0.0,0.0,1.0));
    }
  }
  
  
  
  
  T* dataptr = static_cast<T*>(nrrd->data);

  // Calculate where the data is coming from 
  size_t vector_offset = 0;
  std::vector<size_t> space_offset(rdim);
  std::vector<size_t> space_size(rdim);
  
  k = 1;
  int m =0;
  for (int p=0; p<static_cast<int>(dim);p++)
  {
    if (vecdim == p) 
    {
      vector_offset = k;
      k *= size[p];
    }
    else 
    {
      space_offset[m] = k;
      k *= size[p];
      space_size[m] = k;
      m++;
    }
  }
 
  int t1, t2, t3 ,t4, t5, t6;
  if (size[vecdim] == 6)
  {
    // Sym matrix format
    t1 = 0; t2 = vector_offset; t3 = 2*vector_offset; 
    t4 = 3*vector_offset; t5 = 4*vector_offset; t6 = 5*vector_offset;
  }
  else if (size[vecdim] == 7)
  {
    // Masked Sym Matrix format
    t1 = vector_offset; t2 = 2*vector_offset; t3 = 3*vector_offset; 
    t4 = 4*vector_offset; t5 = 5*vector_offset; t6 = 6*vector_offset;  
  }
  else if (size[vecdim] == 9)
  {
    // Matrix format
    t1 = 0; t2 = vector_offset; t3 = 2*vector_offset; 
    t4 = 4*vector_offset; t5 = 5*vector_offset; t6 = 8*vector_offset;  
  }
  else if (size[vecdim] == 10)
  {
    // Masked Matrix format
    t1 = vector_offset; t2 = 2*vector_offset; t3 = 3*vector_offset; 
    t4 = 5*vector_offset; t5 = 6*vector_offset; t6 = 9*vector_offset;  
  }
  else
  {
    pr->error("NrrdToTensorField: Size of the Tensor axis is improper");
    return (false);          
  }
 
  const double m00 = M[0][0];
  const double m01 = M[0][1];
  const double m02 = M[0][2];
  const double m10 = M[1][0];
  const double m11 = M[1][1];
  const double m12 = M[1][2];
  const double m20 = M[2][0];
  const double m21 = M[2][1];
  const double m22 = M[2][2];
 
  // Build a 2d Image
  if (rdim == 1)
  {
    if (datalocation == "Node")
    {
      ScanlineMesh<CrvLinearLgn<Point> >::handle_type mesh_handle = new ScanlineMesh<CrvLinearLgn<Point> >(rsize[0],Point(rmin[0],0.0,0.0),Point(rmax[0],0.0,0.0));
      GenericField<ScanlineMesh<CrvLinearLgn<Point> >,CrvLinearLgn<Tensor>, vector<Tensor> >* field = new GenericField<ScanlineMesh<CrvLinearLgn<Point> >,CrvLinearLgn<Tensor>, vector<Tensor> >(mesh_handle);
      output = dynamic_cast<Field *>(field);

      ScanlineMesh<CrvLinearLgn<Point> >::Node::iterator it;
      mesh_handle->begin(it);
      
      for (size_t x=0; x<space_size[0]; x+= space_offset[0])
      {
        const double d1 = static_cast<double>(dataptr[x+t1]);
        const double d2 = static_cast<double>(dataptr[x+t2]);
        const double d3 = static_cast<double>(dataptr[x+t3]);
        const double d4 = static_cast<double>(dataptr[x+t4]);
        const double d5 = static_cast<double>(dataptr[x+t5]);
        const double d6 = static_cast<double>(dataptr[x+t6]);
        const double y0 = (d1*m00+d2*m01+d3*m02);
        const double y1 = (d2*m00+d4*m01+d5*m02);
        const double y2 = (d3*m00+d5*m01+d6*m02);
        const double y3 = (d1*m10+d2*m11+d3*m12);
        const double y4 = (d2*m10+d4*m11+d5*m12);
        const double y5 = (d3*m10+d5*m11+d6*m12);
        const double y6 = (d1*m20+d2*m21+d3*m22);
        const double y7 = (d2*m20+d4*m21+d5*m22);
        const double y8 = (d3*m20+d5*m21+d6*m22);
        
        Tensor t(m00*y0+m01*y1+m02*y2,m10*y0+m11*y1+m12*y2,m20*y0+m21*y1+m22*y2,
                 m10*y3+m11*y4+m12*y5,m20*y3+m21*y4+m22*y5,m20*y6+m21*y7+m22*y8);
        field->set_value(t,*it);
        ++it;
      }
      
      if (use_tf)
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }      
      
    }
    else if (datalocation == "Element")
    {
      ScanlineMesh<CrvLinearLgn<Point> >::handle_type mesh_handle = new ScanlineMesh<CrvLinearLgn<Point> >(rsize[0]+1,Point(rmin[0],0.0,0.0),Point(rmax[0],0.0,0.0));
      GenericField<ScanlineMesh<CrvLinearLgn<Point> >,ConstantBasis<Tensor>, vector<Tensor> >* field = new GenericField<ScanlineMesh<CrvLinearLgn<Point> >,ConstantBasis<Tensor>, vector<Tensor> >(mesh_handle);
      output = dynamic_cast<Field *>(field);

      ScanlineMesh<CrvLinearLgn<Point> >::Elem::iterator it;
      mesh_handle->begin(it);

      for (size_t x=0; x<space_size[0]; x+= space_offset[0])
      {
        const double d1 = static_cast<double>(dataptr[x+t1]);
        const double d2 = static_cast<double>(dataptr[x+t2]);
        const double d3 = static_cast<double>(dataptr[x+t3]);
        const double d4 = static_cast<double>(dataptr[x+t4]);
        const double d5 = static_cast<double>(dataptr[x+t5]);
        const double d6 = static_cast<double>(dataptr[x+t6]);
        const double y0 = (d1*m00+d2*m01+d3*m02);
        const double y1 = (d2*m00+d4*m01+d5*m02);
        const double y2 = (d3*m00+d5*m01+d6*m02);
        const double y3 = (d1*m10+d2*m11+d3*m12);
        const double y4 = (d2*m10+d4*m11+d5*m12);
        const double y5 = (d3*m10+d5*m11+d6*m12);
        const double y6 = (d1*m20+d2*m21+d3*m22);
        const double y7 = (d2*m20+d4*m21+d5*m22);
        const double y8 = (d3*m20+d5*m21+d6*m22);
        
        Tensor t(m00*y0+m01*y1+m02*y2,m10*y0+m11*y1+m12*y2,m20*y0+m21*y1+m22*y2,
                 m10*y3+m11*y4+m12*y5,m20*y3+m21*y4+m22*y5,m20*y6+m21*y7+m22*y8);
        field->set_value(t,*it);
        ++it;
      }

      if (use_tf)
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }      
    }
    else
    {
      pr->error("NrrdToTensorField: Data location information is not recognized");
      return (false);      
    }  
  }
  else if (rdim == 2)
  {    
    if (datalocation == "Node")
    {
      ImageMesh<QuadBilinearLgn<Point> >::handle_type mesh_handle = new ImageMesh<QuadBilinearLgn<Point> >(rsize[0],rsize[1],Point(rmin[0],rmin[1],0.0),Point(rmax[0],rmax[1],0.0));
      GenericField<ImageMesh<QuadBilinearLgn<Point> >,QuadBilinearLgn<Tensor>, FData2d<Tensor, ImageMesh<QuadBilinearLgn<Point> > > >* field = new GenericField<ImageMesh<QuadBilinearLgn<Point> >,QuadBilinearLgn<Tensor>, FData2d<Tensor, ImageMesh<QuadBilinearLgn<Point> > > >(mesh_handle);
      output = dynamic_cast<Field *>(field);

      ImageMesh<QuadBilinearLgn<Point> >::Node::iterator it;
      mesh_handle->begin(it);

      for (size_t y=0; y<space_size[1]; y+=space_offset[1])
      {
        for (size_t x=0; x<space_size[0]; x+= space_offset[0])
        {
          size_t a = x+y;
          const double d1 = static_cast<double>(dataptr[a+t1]);
          const double d2 = static_cast<double>(dataptr[a+t2]);
          const double d3 = static_cast<double>(dataptr[a+t3]);
          const double d4 = static_cast<double>(dataptr[a+t4]);
          const double d5 = static_cast<double>(dataptr[a+t5]);
          const double d6 = static_cast<double>(dataptr[a+t6]);
          const double y0 = (d1*m00+d2*m01+d3*m02);
          const double y1 = (d2*m00+d4*m01+d5*m02);
          const double y2 = (d3*m00+d5*m01+d6*m02);
          const double y3 = (d1*m10+d2*m11+d3*m12);
          const double y4 = (d2*m10+d4*m11+d5*m12);
          const double y5 = (d3*m10+d5*m11+d6*m12);
          const double y6 = (d1*m20+d2*m21+d3*m22);
          const double y7 = (d2*m20+d4*m21+d5*m22);
          const double y8 = (d3*m20+d5*m21+d6*m22);
          
          Tensor t(m00*y0+m01*y1+m02*y2,m10*y0+m11*y1+m12*y2,m20*y0+m21*y1+m22*y2,
                   m10*y3+m11*y4+m12*y5,m20*y3+m21*y4+m22*y5,m20*y6+m21*y7+m22*y8);
          field->set_value(t,*it);
          ++it;
        }
      }
      
      if (use_tf) 
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }      
    }
    else if (datalocation == "Element")
    {
      ImageMesh<QuadBilinearLgn<Point> >::handle_type mesh_handle = new ImageMesh<QuadBilinearLgn<Point> >(rsize[0]+1,rsize[1]+1,Point(rmin[0],rmin[1],0.0),Point(rmax[0],rmax[1],0.0));
      GenericField<ImageMesh<QuadBilinearLgn<Point> >,ConstantBasis<Tensor>, FData2d<Tensor, ImageMesh<QuadBilinearLgn<Point> > > >* field = new GenericField<ImageMesh<QuadBilinearLgn<Point> >,ConstantBasis<Tensor>, FData2d<Tensor, ImageMesh<QuadBilinearLgn<Point> > > >(mesh_handle);
      output = dynamic_cast<Field *>(field);

      ImageMesh<QuadBilinearLgn<Point> >::Elem::iterator it;
      mesh_handle->begin(it);

      for (size_t y=0; y<space_size[1]; y+=space_offset[1])
      {
        for (size_t x=0; x<space_size[0]; x+= space_offset[0])
        {
          size_t a = x+y;
          const double d1 = static_cast<double>(dataptr[a+t1]);
          const double d2 = static_cast<double>(dataptr[a+t2]);
          const double d3 = static_cast<double>(dataptr[a+t3]);
          const double d4 = static_cast<double>(dataptr[a+t4]);
          const double d5 = static_cast<double>(dataptr[a+t5]);
          const double d6 = static_cast<double>(dataptr[a+t6]);
          const double y0 = (d1*m00+d2*m01+d3*m02);
          const double y1 = (d2*m00+d4*m01+d5*m02);
          const double y2 = (d3*m00+d5*m01+d6*m02);
          const double y3 = (d1*m10+d2*m11+d3*m12);
          const double y4 = (d2*m10+d4*m11+d5*m12);
          const double y5 = (d3*m10+d5*m11+d6*m12);
          const double y6 = (d1*m20+d2*m21+d3*m22);
          const double y7 = (d2*m20+d4*m21+d5*m22);
          const double y8 = (d3*m20+d5*m21+d6*m22);
          
          Tensor t(m00*y0+m01*y1+m02*y2,m10*y0+m11*y1+m12*y2,m20*y0+m21*y1+m22*y2,
                   m10*y3+m11*y4+m12*y5,m20*y3+m21*y4+m22*y5,m20*y6+m21*y7+m22*y8);
          field->set_value(t,*it);
          ++it;
        }
      }

      if (use_tf)
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }      
    }
    else
    {
      pr->error("NrrdToTensorField: Data location information is not recognized");
      return (false);      
    }
  }
  else if (rdim == 3)
  {
    if (datalocation == "Node")
    {
      LatVolMesh<HexTrilinearLgn<Point> >::handle_type mesh_handle = new LatVolMesh<HexTrilinearLgn<Point> >(rsize[0],rsize[1],rsize[2],Point(rmin[0],rmin[1],rmin[2]),Point(rmax[0],rmax[1],rmax[2]));
      GenericField<LatVolMesh<HexTrilinearLgn<Point> >,HexTrilinearLgn<Tensor>, FData3d<Tensor, LatVolMesh<HexTrilinearLgn<Point> > > >* field = new GenericField<LatVolMesh<HexTrilinearLgn<Point> >,HexTrilinearLgn<Tensor>, FData3d<Tensor, LatVolMesh<HexTrilinearLgn<Point> > > >(mesh_handle);
      output = dynamic_cast<Field *>(field);
      
      LatVolMesh<HexTrilinearLgn<Point> >::Node::iterator it;
      mesh_handle->begin(it);

      for (size_t z=0; z<space_size[2]; z+=space_offset[2])
      {
        for (size_t y=0; y<space_size[1]; y+=space_offset[1])
        {
          for (size_t x=0; x<space_size[0]; x+= space_offset[0])
          {
            size_t a = x+y+z;
            const double d1 = static_cast<double>(dataptr[a+t1]);
            const double d2 = static_cast<double>(dataptr[a+t2]);
            const double d3 = static_cast<double>(dataptr[a+t3]);
            const double d4 = static_cast<double>(dataptr[a+t4]);
            const double d5 = static_cast<double>(dataptr[a+t5]);
            const double d6 = static_cast<double>(dataptr[a+t6]);
            const double y0 = (d1*m00+d2*m01+d3*m02);
            const double y1 = (d2*m00+d4*m01+d5*m02);
            const double y2 = (d3*m00+d5*m01+d6*m02);
            const double y3 = (d1*m10+d2*m11+d3*m12);
            const double y4 = (d2*m10+d4*m11+d5*m12);
            const double y5 = (d3*m10+d5*m11+d6*m12);
            const double y6 = (d1*m20+d2*m21+d3*m22);
            const double y7 = (d2*m20+d4*m21+d5*m22);
            const double y8 = (d3*m20+d5*m21+d6*m22);
            
            Tensor t(m00*y0+m01*y1+m02*y2,m10*y0+m11*y1+m12*y2,m20*y0+m21*y1+m22*y2,
                     m10*y3+m11*y4+m12*y5,m20*y3+m21*y4+m22*y5,m20*y6+m21*y7+m22*y8);
            field->set_value(t,*it);
            ++it;
          }
        }
      }

      if (use_tf)   
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }      
    }
    else if (datalocation == "Element")
    {
      LatVolMesh<HexTrilinearLgn<Point> >::handle_type mesh_handle = new LatVolMesh<HexTrilinearLgn<Point> >(rsize[0]+1,rsize[1]+1,rsize[2]+1,Point(rmin[0],rmin[1],rmin[2]),Point(rmax[0],rmax[1],rmax[2]));
      GenericField<LatVolMesh<HexTrilinearLgn<Point> >,ConstantBasis<Tensor>, FData3d<Tensor, LatVolMesh<HexTrilinearLgn<Point> > > >* field = new GenericField<LatVolMesh<HexTrilinearLgn<Point> >,ConstantBasis<Tensor>, FData3d<Tensor, LatVolMesh<HexTrilinearLgn<Point> > > >(mesh_handle);
      output = dynamic_cast<Field *>(field);


      LatVolMesh<HexTrilinearLgn<Point> >::Elem::iterator it;
      mesh_handle->begin(it);

      for (size_t z=0; z<space_size[2]; z+=space_offset[2])
      {
        for (size_t y=0; y<space_size[1]; y+=space_offset[1])
        {
          for (size_t x=0; x<space_size[0]; x+= space_offset[0])
          {
            size_t a = x+y+z;
            const double d1 = static_cast<double>(dataptr[a+t1]);
            const double d2 = static_cast<double>(dataptr[a+t2]);
            const double d3 = static_cast<double>(dataptr[a+t3]);
            const double d4 = static_cast<double>(dataptr[a+t4]);
            const double d5 = static_cast<double>(dataptr[a+t5]);
            const double d6 = static_cast<double>(dataptr[a+t6]);
            const double y0 = (d1*m00+d2*m01+d3*m02);
            const double y1 = (d2*m00+d4*m01+d5*m02);
            const double y2 = (d3*m00+d5*m01+d6*m02);
            const double y3 = (d1*m10+d2*m11+d3*m12);
            const double y4 = (d2*m10+d4*m11+d5*m12);
            const double y5 = (d3*m10+d5*m11+d6*m12);
            const double y6 = (d1*m20+d2*m21+d3*m22);
            const double y7 = (d2*m20+d4*m21+d5*m22);
            const double y8 = (d3*m20+d5*m21+d6*m22);
            
            Tensor t(m00*y0+m01*y1+m02*y2,m10*y0+m11*y1+m12*y2,m20*y0+m21*y1+m22*y2,
                     m10*y3+m11*y4+m12*y5,m20*y3+m21*y4+m22*y5,m20*y6+m21*y7+m22*y8);
            field->set_value(t,*it);
            ++it;
          }
        }
      }
      
      if (use_tf)   
      {
        Transform trans = mesh_handle->get_transform();
        trans.pre_trans(tf);
        mesh_handle->set_transform(trans);      
      }      
    }
    else
    {
      pr->error("NrrdToTensorField: Data location information is not recognized");
      return (false);      
    }
  }
  else
  {
    pr->error("NrrdToTensorField: Nrrd is not 2D or 3D");
    return (false);        
  }
  
  return (true);
}



bool NrrdToFieldAlgo::NrrdToField(ProgressReporter *pr, NrrdDataHandle input, FieldHandle& output,std::string datalocation,std::string fieldtype,std::string spaceparity)
{
  int datadim = -1;
  
  if (input.get_rep() == 0)
  {
    pr->error("NrrdToField: No input Nrrd");
    return (false);    
  } 

  Nrrd *nrrd = input->nrrd_;

  if (nrrd == 0)
  {
    pr->error("NrrdToField: NrrdData does not contain Nrrd");
    return (false);      
  }

  unsigned int dim = nrrd->dim;
  if (dim > 4)
  {
    pr->error("NrrdToField: Can't convert NRRDs with dimension greater than 4.");
    return false;
  }

  // Automatically detect nrrd type Element of Node based.
  if (datalocation == "Auto")
  {
    datalocation = "Node";
    for (unsigned int p=0; p<dim;p++)
    {
      if (nrrd->axis[p].center == nrrdCenterCell)
      {
        datalocation = "Element";
        break;
      }
    }
  }
  
  // Automatically detect field type based on the clues given in the nrrd
  // annotation.
  
  if (fieldtype == "Auto")
  {
    for (unsigned int p=0; p<dim;p++)
    {
      if (nrrd->axis[p].kind == nrrdKindPoint && nrrd->axis[p].size == 3)
      {
        fieldtype = "VectorField"; datadim = p;
        break;
      }
      if (nrrd->axis[p].kind == nrrdKindVector && nrrd->axis[p].size == 3)
      {
        fieldtype = "VectorField"; datadim = p;
        break;
      }
      if (nrrd->axis[p].kind == nrrdKindCovariantVector && nrrd->axis[p].size == 3)
      {
        fieldtype = "VectorField"; datadim = p;
        break;
      }
      if (nrrd->axis[p].kind == nrrdKindNormal && nrrd->axis[p].size == 3)
      {
        fieldtype = "VectorField"; datadim = p;
        break;
      }
      if (nrrd->axis[p].kind == nrrdKind3Color)
      {
        fieldtype = "VectorField"; datadim = p;
        break;
      }
      if (nrrd->axis[p].kind == nrrdKindRGBColor)
      {
        fieldtype = "VectorField"; datadim = p;
        break;
      }
      if (nrrd->axis[p].kind == nrrdKindHSVColor)
      {
        fieldtype = "VectorField"; datadim = p;
        break;
      }
      if (nrrd->axis[p].kind == nrrdKindXYZColor)
      {
        fieldtype = "VectorField"; datadim = p;
        break;
      }      
      if (nrrd->axis[p].kind == nrrdKind3Vector)
      {
        fieldtype = "VectorField"; datadim = p;
        break;
      }
      if (nrrd->axis[p].kind == nrrdKind3Gradient)
      {
        fieldtype = "VectorField"; datadim = p;
        break;
      }
      if (nrrd->axis[p].kind == nrrdKind3Normal)
      {
        fieldtype = "VectorField"; datadim = p;
        break;
      }
      if (nrrd->axis[p].kind == nrrdKind3DSymMatrix)
      {
        fieldtype = "TensorField"; datadim = p;
        break;
      }
      if (nrrd->axis[p].kind == nrrdKind3DMaskedSymMatrix)
      {
        fieldtype = "TensorField"; datadim = p;
        break;
      }
      if (nrrd->axis[p].kind == nrrdKind3DMatrix)
      {
        fieldtype = "TensorField"; datadim = p;
        break;
      }
      if (nrrd->axis[p].kind == nrrdKind3DMaskedMatrix)
      {
        fieldtype = "TensorField"; datadim = p;
        break;
      }
    }    
  }

  // If the nrrd has no clues but we have 4 dimensions we need to assign one
  // of them as tensor/vector dimension
  if (fieldtype == "Auto")
  {
    unsigned int dim = nrrd->dim;
    
    if (dim == 4)
    {
      for (unsigned int p=0; p< dim; p++)
      {
        if (nrrd->axis[p].size == 3)
        {
          fieldtype = "VectorField"; datadim = p; break;
        }
        if ((nrrd->axis[p].size == 6)||(nrrd->axis[p].size == 7)||
            (nrrd->axis[p].size == 9)||(nrrd->axis[p].size == 10))
        {
          fieldtype = "TensorField"; datadim = p; break;
        }
      }
      
      if (fieldtype == "Auto" && nrrd->axis[3].size != 1)
      {
        pr->error("NrrdToField: The nrrd is 4D, but none of the axis qualifies for being a vector, tensor, or scalar dimension.");
        return (false);
      }
    }
  }

  if (fieldtype == "Auto") fieldtype = "ScalarField";

  NrrdToFieldAlgoT algo;

  if (fieldtype == "ScalarField")
  {
    switch (nrrd->type)
    {
      case nrrdTypeChar : 
        return(algo.NrrdToField<char>(pr,input,output,datalocation,spaceparity));
      case nrrdTypeUChar : 
        return(algo.NrrdToField<unsigned char>(pr,input,output,datalocation,spaceparity));
      case nrrdTypeShort : 
        return(algo.NrrdToField<short>(pr,input,output,datalocation,spaceparity));
      case nrrdTypeUShort : 
        return(algo.NrrdToField<unsigned short>(pr,input,output,datalocation,spaceparity));              
      case nrrdTypeInt : 
        return(algo.NrrdToField<int>(pr,input,output,datalocation,spaceparity));
      case nrrdTypeUInt : 
        return(algo.NrrdToField<unsigned int>(pr,input,output,datalocation,spaceparity));
      case nrrdTypeLLong : 
        return(algo.NrrdToField<long long>(pr,input,output,datalocation,spaceparity));
      case nrrdTypeULLong : 
        return(algo.NrrdToField<unsigned long long>(pr,input,output,datalocation,spaceparity));
      case nrrdTypeFloat : 
        return(algo.NrrdToField<float>(pr,input,output,datalocation,spaceparity));
      case nrrdTypeDouble : 
        return(algo.NrrdToField<double>(pr,input,output,datalocation,spaceparity));
      default: 
        pr->error("NrrdToField: This datatype is not supported");
        return (false);
    }
  }
  else if (fieldtype == "VectorField")
  {
    switch (nrrd->type)
    {
      case nrrdTypeChar : 
        return(algo.NrrdToVectorField<char>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeUChar : 
        return(algo.NrrdToVectorField<unsigned char>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeShort : 
        return(algo.NrrdToVectorField<short>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeUShort : 
        return(algo.NrrdToVectorField<unsigned short>(pr,input,output,datalocation,spaceparity,datadim));              
      case nrrdTypeInt : 
        return(algo.NrrdToVectorField<int>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeUInt : 
        return(algo.NrrdToVectorField<unsigned int>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeLLong : 
        return(algo.NrrdToVectorField<long long>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeULLong : 
        return(algo.NrrdToVectorField<unsigned long long>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeFloat : 
        return(algo.NrrdToVectorField<float>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeDouble : 
        return(algo.NrrdToVectorField<double>(pr,input,output,datalocation,spaceparity,datadim));
      default: 
        pr->error("NrrdToVectorField: This datatype is not supported");
        return (false);
    }
  }
  else if (fieldtype == "TensorField")
  {
    switch (nrrd->type)
    {
      case nrrdTypeChar : 
        return(algo.NrrdToTensorField<char>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeUChar : 
        return(algo.NrrdToTensorField<unsigned char>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeShort : 
        return(algo.NrrdToTensorField<short>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeUShort : 
        return(algo.NrrdToTensorField<unsigned short>(pr,input,output,datalocation,spaceparity,datadim));              
      case nrrdTypeInt : 
        return(algo.NrrdToTensorField<int>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeUInt : 
        return(algo.NrrdToTensorField<unsigned int>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeLLong : 
        return(algo.NrrdToTensorField<long long>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeULLong : 
        return(algo.NrrdToTensorField<unsigned long long>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeFloat : 
        return(algo.NrrdToTensorField<float>(pr,input,output,datalocation,spaceparity,datadim));
      case nrrdTypeDouble : 
        return(algo.NrrdToTensorField<double>(pr,input,output,datalocation,spaceparity,datadim));
      default: 
        pr->error("NrrdToTensorField: This datatype is not supported");
        return (false);
    }  
  }
  else
  {
    pr->error("NrrdToField: Unknown field type");
    return (false);
  }

  return (true);
}

} // end namespace SCIRunAlgo


