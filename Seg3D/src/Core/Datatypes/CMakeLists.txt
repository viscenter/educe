#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2004 Scientific Computing and Imaging Institute,
#  University of Utah.
# 
#  
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
# 
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
# 
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#

# Sources of Core/Datatypes classes

SET(Core_Datatypes_SRCS
  Bundle.cc
  Clipper.cc
  Color.cc
  ColorMap.cc		    	
  ColumnMatrix.cc	    	
  Datatype.cc
  DenseColMajMatrix.cc	    	
  DenseMatrix.cc	    	
  Field.cc
  FieldRNG.cc
  FieldInformation.cc
  VFData.cc	
  VFDataT_0.cc	
  VFDataT_1.cc	
  VFDataT_2.cc	
  Image.cc	
  Material.cc	    	
  Matrix.cc		    	
  MatrixOperations.cc	    	
  Mesh.cc		    	
  VMesh.cc		    	
  NrrdData.cc
  NrrdToITK.cc
  PropertyManager.cc	    	
  SearchGrid.cc           	
  SparseRowMatrix.cc	    	
  String.cc                     
  TypeName.cc		    	
  PointCloudMesh.cc
  CurveMesh.cc
  TriSurfMesh.cc
  QuadSurfMesh.cc
  TetVolMesh.cc
  PrismVolMesh.cc
  HexVolMesh.cc
  LatVolMesh.cc
  ImageMesh.cc
  ScanlineMesh.cc
  cd_templates.cc	    	
  cd_templates_fields_0.cc    	
  cd_templates_fields_1.cc    	
  cd_templates_fields_2.cc    	
  cd_templates_fields_3.cc    	
  cd_templates_fields_4.cc    	
  cd_templates_fields_5.cc	
  cd_templates_fields_6.cc	
)

IF(ITK_FOUND)
  SET(Core_Datatypes_SRCS ${Core_Datatypes_SRCS} 
    ITKDatatype.cc)
ENDIF(ITK_FOUND)

SCIRUN_ADD_LIBRARY(Core_Datatypes ${Core_Datatypes_SRCS})

TARGET_LINK_LIBRARIES(Core_Datatypes 
  Core_Basis        
  Core_Containers   
  Core_Exceptions
  Core_Geometry     
  Core_Math         
  Core_Persistent   
  Core_Thread       
  Core_Util
  ${GL_LIBRARY}
  ${M_LIBRARY}
  ${CBLAS_LIBRARY}
  ${F_LIBRARY} 
  ${TEEM_LIBRARY}
  ${PNG_LIBRARY}
  ${Z_LIBRARY}
  ${SLIVRV_LIBRARY}
)

IF(APPLE_LEOPARD)
  SET_TARGET_PROPERTIES(Core_Datatypes PROPERTIES LINK_FLAGS  "${GL_LINK}")
ENDIF(APPLE_LEOPARD)


IF(ITK_FOUND)
  TARGET_LINK_LIBRARIES(Core_Datatypes
    ITKCommon)
ENDIF(ITK_FOUND)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Datatypes)
ENDIF(BUILD_SHARED_LIBS)
