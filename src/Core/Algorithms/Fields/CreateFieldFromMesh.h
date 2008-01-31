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
 *  CreateFieldFromMesh.cc:
 *
 *  Create a Field from a mesh.
 *
 *  Written by:
 *   Allen Sanderson
     Darby Van Uitert
 *   School of Computing
 *   University of Utah
 *   March 2004
 *
 *  Copyright (C) 2001 SCI Institute
 */

#if !defined(CreateFieldFromMesh_h)
#define CreateFieldFromMesh_h

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

#include <Core/Datatypes/NrrdData.h>

#include <iostream>

//! for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRun {

class SCISHARE CreateFieldFromMeshAlgo : public DynamicAlgoBase
{
public:
  virtual bool CreateFieldFromMesh(ProgressReporter *pr,
				   MeshHandle mHandle,
				   const TypeDescription *btd,
				   const TypeDescription *dtd,
				   FieldHandle& fhandle);

  //! support the dynamically compiled algorithm concept
  static CompileInfoHandle get_compile_info(MeshHandle mHandle,
					    const TypeDescription *btd,
					    const TypeDescription *dtd,
					    int rank);

  virtual bool execute(ProgressReporter *pr,
		       MeshHandle mHandle,
		       FieldHandle& fhandle) { return false; };
};


template< class FIELD, class MESH >
class CreateFieldFromMeshAlgoT : public CreateFieldFromMeshAlgo
{
public:
  virtual bool execute(ProgressReporter *pr,
		       MeshHandle mHandle,
		       FieldHandle& fhandle);
};


template< class FIELD, class MESH >
bool
CreateFieldFromMeshAlgoT<FIELD, MESH>::execute(ProgressReporter *pr,
					       MeshHandle mHandle,
					       FieldHandle& fHandle)
{
  MESH *imesh = (MESH *) mHandle.get_rep();
  FIELD *ifield = (FIELD *) scinew FIELD((MESH *) imesh);

  fHandle = (FieldHandle) ifield;

  return true;
}

} // end namespace SCITeem

#endif // NrrdFieldConverter_h
