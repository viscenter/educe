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

# Sources of Core/Algorithms/Fields classes
SET(Core_Algorithms_Fields_SRCS
  ApplyErodeFilterToFieldData.cc
  ApplyDilateFilterToFieldData.cc
  ApplyMappingMatrix.cc
  ApplyTriSurfPhaseFilter.cc
  CalculateFieldDataCompiled.cc
  CalculateIsInsideField.cc
  CalculateLinkBetweenOppositeFieldBoundaries.cc
  ClearAndChangeFieldBasis.cc
  ClipAtIndeces.cc
  ClipBySelectionMask.cc
  ClipFieldByFunction.cc
  ClipFieldWithSeed.cc
  ConvertMappingMatrixToFieldData.cc
  ConvertMeshToTetVol.cc
  ConvertMeshToTriSurf.cc
  ConvertMeshToUnstructuredMesh.cc
  ConvertMeshToIrregularMesh.cc
  CollectPointClouds.cc
  CreateLinkBetweenMeshAndCompGridByDomain.cc
  CreateLinkBetweenMeshAndCompGrid.cc
  CreateMeshFromNrrd.cc
  CreateFieldFromMesh.cc
  DistanceField.cc
  FieldsAlgo.cc
  FindClosestNodeByValue.cc
  FindClosestNode.cc
  GatherFields.cc
  GetBoundingBox.cc
  GetDomainBoundary.cc
  GetFieldNodes.cc
  GetFieldDataMinMax.cc
  GetFieldInfo.cc
  GetFieldMeasure.cc
  GetMeshQualityField.cc
  MapCurrentDensityOntoField.cc
  Mapping.cc
  MergeFields.cc
  MergeMeshes.cc
  ProjectPointsOntoMesh.cc
  RemoveUnusedNodes.cc
  ScaleFieldMeshAndData.cc
  SetFieldNodes.cc
  SplitNodesByDomain.cc
  SplitByConnectedRegion.cc
  ConvertMeshToPointCloud.cc
  TracePoints.cc
  TransformMeshWithTransform.cc
  TransformMeshWithFunction.cc
)

SET(Core_Algorithms_Fields_SRCS ${Core_Algorithms_Fields_SRCS}
  FieldData/CalculateGradients.cc
  FieldData/ConvertFieldDataType.cc
  FieldData/ConvertFieldBasisType.cc
  FieldData/ConvertIndicesToFieldData.cc
  FieldData/GetFieldData.cc
  FieldData/SetFieldData.cc
  Mapping/MapFieldDataFromElemToNode.cc
  Mapping/MapFieldDataFromNodeToElem.cc
  MeshDerivatives/GetCentroids.cc
  MeshDerivatives/GetFieldBoundary.cc
  RefineMesh/RefineMesh.cc
)

SCIRUN_ADD_LIBRARY(Core_Algorithms_Fields ${Core_Algorithms_Fields_SRCS})

TARGET_LINK_LIBRARIES(Core_Algorithms_Fields 
  Core_Datatypes 
  Core_Util 
  Core_Containers 
  Core_Exceptions 
  Core_Thread 
  Core_Geom 
  Core_Geometry 
  Core_Algorithms_Converter
  Core_Algorithms_Util 
  Core_Persistent 
  Core_Basis 
  Core_Bundle
  ${TEEM_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Algorithms_Fields)
ENDIF(BUILD_SHARED_LIBS)

