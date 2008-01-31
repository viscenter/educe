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

/*
 *  CreateMeshFromNrrd.cc:
 *
 *  Create a Field from Nrrd Daata.  * Incoming Nrrds may consist of
 *  Data, Points, Connections and a * Field mesh to associate it with.
 *
 *  Written by:
 *    Allen Sanderson, Darby Van Uitert
 *    School of Computing
 *    University of Utah
 *    March 2004
 *
 *  Copyright (C) 2001 SCI Institute
 */

#if !defined(CreateMeshFromNrrd_h)
#define CreateMeshFromNrrd_h

#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>

#include <Core/Basis/TriLinearLgn.h>
#include <Core/Basis/CrvLinearLgn.h>
#include <Core/Basis/TetLinearLgn.h>
#include <Core/Basis/PrismLinearLgn.h>
#include <Core/Basis/CrvLinearLgn.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/HexTrilinearLgn.h>
#include <Core/Basis/QuadBilinearLgn.h>

#include <Core/Datatypes/PointCloudMesh.h>
#include <Core/Datatypes/CurveMesh.h>
#include <Core/Datatypes/TriSurfMesh.h>
#include <Core/Datatypes/QuadSurfMesh.h>

#include <Core/Datatypes/TetVolMesh.h>
#include <Core/Datatypes/PrismVolMesh.h>
#include <Core/Datatypes/HexVolMesh.h>

#include <Core/Datatypes/LatVolMesh.h>
#include <Core/Datatypes/ImageMesh.h>
#include <Core/Datatypes/ScanlineMesh.h>

#include <Core/Datatypes/StructHexVolMesh.h>
#include <Core/Datatypes/StructQuadSurfMesh.h>
#include <Core/Datatypes/StructCurveMesh.h>
#include <Core/Math/MiscMath.h>

#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Tensor.h>

#include <Core/Containers/StringUtil.h>

#include <Core/Datatypes/NrrdData.h>

#include <teem/ten.h>

#include <iostream>

#include <Core/Algorithms/Fields/share.h>

namespace SCIRun {

class SCISHARE CreateMeshFromNrrdAlgo : public DynamicAlgoBase
{
public:
  bool CreateMeshFromNrrd( ProgressReporter *pr,
                           NrrdDataHandle nrrd_point_handle,
                           NrrdDataHandle nrrd_connection_handle,
                           FieldHandle& field_output_handle,
                           string QuadOrTet,
                           string StructOrUnstruct );

protected:
  //! support the dynamically compiled algorithm concept
  static CompileInfoHandle get_compile_info( const std::string topoStr,
                                             const TypeDescription *mtd,
                                             const unsigned int ptype,
                                             const unsigned int ctype);
protected:
  typedef CurveMesh<CrvLinearLgn<Point> >             CMesh;
  typedef TriSurfMesh<TriLinearLgn<Point> >           TSMesh;
  typedef LatVolMesh<HexTrilinearLgn<Point> >         LVMesh;
  typedef ImageMesh<QuadBilinearLgn<Point> >          IMesh;
  typedef QuadSurfMesh<QuadBilinearLgn<Point> >       QSMesh;
  typedef ScanlineMesh<CrvLinearLgn<Point> >          SLMesh;
  typedef PointCloudMesh<ConstantBasis<Point> >       PCMesh;
  typedef PrismVolMesh<PrismLinearLgn<Point> >        PVMesh;
  typedef TetVolMesh<TetLinearLgn<Point> >            TVMesh;
  typedef HexVolMesh<HexTrilinearLgn<Point> >         HVMesh;
  typedef StructCurveMesh<CrvLinearLgn<Point> >       SCMesh;
  typedef StructQuadSurfMesh<QuadBilinearLgn<Point> > SQSMesh;
  typedef StructHexVolMesh<HexTrilinearLgn<Point> >   SHVMesh;


  enum {UNKNOWN=0,UNSTRUCTURED=1,STRUCTURED=2,IRREGULAR=4,REGULAR=8};
};


/////////////// Regular Mesh ///////////////
class RegularCreateMeshFromNrrdAlgo : public DynamicAlgoBase
{
public:
  virtual void execute(MeshHandle& mHandle,
                       NrrdDataHandle dataH, int data_size);
};


template< class MESH, class DNTYPE, class CNTYPE>
class RegularCreateMeshFromNrrdAlgoT : 
  public RegularCreateMeshFromNrrdAlgo
{
public:
  virtual void execute(MeshHandle& mHandle,
                       NrrdDataHandle dataH, int data_size) = 0;
};

template< class MESH, class DNTYPE, class CNTYPE>
void
RegularCreateMeshFromNrrdAlgoT< MESH, DNTYPE, CNTYPE>::
execute(MeshHandle& mHandle, NrrdDataHandle dataH, int data_size)
{
  Point minpt, maxpt;

  DNTYPE *ptr = (DNTYPE *)(dataH->nrrd_->data);

  int rank = data_size;

  float xVal = 0, yVal = 0, zVal = 0;

  if( rank >= 1 ) xVal = ptr[0];
  if( rank >= 2 ) yVal = ptr[1];
  if( rank >= 3 ) zVal = ptr[2];

  minpt = Point( xVal, yVal, zVal );

  xVal = 0; yVal = 0; zVal = 0;

  if( rank >= 1 ) xVal = ptr[rank + 0];
  if( rank >= 2 ) yVal = ptr[rank + 1];
  if( rank >= 3 ) zVal = ptr[rank + 2];

  maxpt = Point( xVal, yVal, zVal );

  MESH *imesh = (MESH *) mHandle.get_rep();

  vector<unsigned int> array;
  imesh->get_dim(array);

  Transform trans;

  if( array.size() == 1 )
    trans.pre_scale(Vector(1.0 / (array[0]-1.0),
                           1.0,
                           1.0));
  
  else if( array.size() == 2 )
    trans.pre_scale(Vector(1.0 / (array[0]-1.0),
                           1.0 / (array[1]-1.0),
                           1.0));
  
  else  if( array.size() == 3 )
    trans.pre_scale(Vector(1.0 / (array[0]-1.0),
                           1.0 / (array[1]-1.0),
                           1.0 / (array[2]-1.0)));
  
  trans.pre_scale(maxpt - minpt);
  
  trans.pre_translate(minpt.asVector());
  trans.compute_imat();

  imesh->set_transform(trans);
}


/////////////// Structured Mesh ///////////////
class StructuredCreateMeshFromNrrdAlgo : public CreateMeshFromNrrdAlgo
{
public:
  virtual void execute(MeshHandle& mHandle,
                       NrrdDataHandle pointsH,
                       int idim, int jdim, int kdim) = 0;
};


template< class MESH, class PNTYPE, class CNTYPE >
class StructuredCreateMeshFromNrrdAlgoT : 
  public StructuredCreateMeshFromNrrdAlgo
{
public:

  virtual void execute(MeshHandle& mHandle,
                       NrrdDataHandle pointsH,
                       int idim, int jdim, int kdim);
};


template< class MESH, class PNTYPE, class CNTYPE >
void
StructuredCreateMeshFromNrrdAlgoT< MESH, PNTYPE, CNTYPE >::
execute(MeshHandle& mHandle,
        NrrdDataHandle pointsH,
        int idim, int jdim, int kdim)
{
  MESH *imesh = (MESH *) mHandle.get_rep();
  typename MESH::Node::iterator inodeItr;
  
  imesh->begin( inodeItr );
  
  register int i, j, k;
  int rank = pointsH->nrrd_->axis[0].size;
  
  PNTYPE *ptr = (PNTYPE *)(pointsH->nrrd_->data);
  for( i=0; i<idim; i++ ) {
    for( j=0; j<jdim; j++ ) {
      for( k=0; k<kdim; k++ ) {
        
        int index = (i * jdim + j) * kdim + k;
        
        float xVal = 0, yVal = 0, zVal = 0;
        
        // Mesh
        if( rank >= 1 ) xVal = ptr[index*rank + 0];
        if( rank >= 2 ) yVal = ptr[index*rank + 1];
        if( rank >= 3 ) zVal = ptr[index*rank + 2];
        
        imesh->set_point(Point(xVal, yVal, zVal), *inodeItr);
        
        ++inodeItr;
      }
    }
  }
}


/////////////// Unstructured Mesh ///////////////
class UnstructuredCreateMeshFromNrrdAlgo : 
  public CreateMeshFromNrrdAlgo
{
public:
  virtual void execute(MeshHandle& mHandle,
                       NrrdDataHandle pointsH,
                       NrrdDataHandle connectH,
                       unsigned int connectivity, int which) = 0;
};

template< class MESH, class PNTYPE, class CNTYPE >
class UnstructuredCreateMeshFromNrrdAlgoT : 
  public UnstructuredCreateMeshFromNrrdAlgo
{
public:
  virtual void execute(MeshHandle& mHandle,
                       NrrdDataHandle pointsH,
                       NrrdDataHandle connectH,
                       unsigned int connectivity, int which);
};


template< class MESH, class PNTYPE, class CNTYPE >
void
UnstructuredCreateMeshFromNrrdAlgoT< MESH, PNTYPE, CNTYPE >::
execute(MeshHandle& mHandle,
        NrrdDataHandle pointsH,
        NrrdDataHandle connectH,
        unsigned int connectivity, int which)
{
  MESH *imesh = (MESH *) mHandle.get_rep();
  typename MESH::Node::iterator inodeItr;
  bool single_element = false;
  if (connectH != 0 && connectH->nrrd_->dim == 1)
    single_element = true;

  imesh->begin( inodeItr );

  int npts = pointsH->nrrd_->axis[1].size;
  int rank = pointsH->nrrd_->axis[0].size;

  PNTYPE *pPtr = (PNTYPE *)(pointsH->nrrd_->data);
  
  for( int index=0; index<npts; index++ ) {
    float xVal = 0, yVal = 0, zVal = 0;
    
    // Mesh
    if( rank >= 1 ) xVal = pPtr[index*rank + 0];
    if( rank >= 2 ) yVal = pPtr[index*rank + 1];
    if( rank >= 3 ) zVal = pPtr[index*rank + 2];
    imesh->add_point( Point(xVal, yVal, zVal) );
  }

  if( connectivity > 0 ) {

    CNTYPE *cPtr = cPtr = (CNTYPE *)(connectH->nrrd_->data);

    int nelements = 0;
    if (single_element)
      nelements = 1;
    else if (which == 0) {
      // p x n
      nelements = connectH->nrrd_->axis[1].size;
    } else {
      // n x p
      nelements = connectH->nrrd_->axis[0].size;
    }

    typename MESH::Node::array_type array(connectivity);

    if (which == 0) {
      // p x n
      for( int i=0; i<nelements; i++ ) {
        for( unsigned int j=0; j<connectivity; j++ ) {
          array[j] = (int) cPtr[i*connectivity+j];
        }
        imesh->add_elem( array );
      }
    } else {
      // n x p
      for( int i=0; i<nelements; i++ ) {
        for( unsigned int j=0; j<connectivity; j++ ) {
          array[j] = (int) cPtr[j*connectivity+i];
        }
        imesh->add_elem( array );
      }
    }
  }
}

} // end namespace SCITeem

#endif // CreateMeshFromNrrd_h
