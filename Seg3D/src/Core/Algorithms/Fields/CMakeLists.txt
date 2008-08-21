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
  CalculateFieldDataCompiled.cc
  ClearAndChangeFieldBasis.cc
  ClipAtIndeces.cc
  ClipBySelectionMask.cc
  ClipFieldByFunction.cc
  ClipFieldWithSeed.cc
  CollectPointClouds.cc
  FieldsAlgo.cc
  FindClosestNodeByValue.cc
  FindClosestNode.cc
  GetFieldMeasure.cc
  MapCurrentDensityOntoField.cc
  Mapping.cc
  RegisterWithCorrespondences.cc
  TransformMeshWithFunction.cc
)

SET(Core_Algorithms_Fields_SRCS ${Core_Algorithms_Fields_SRCS}
  Cleanup/CleanupTetMesh.cc
  Cleanup/RemoveUnusedNodes.cc
  ClipMesh/ClipMeshByIsovalue.cc
  ClipMesh/ClipMeshBySelection.cc
  ConvertMeshType/ConvertMeshToPointCloudMesh.cc
  ConvertMeshType/ConvertMeshToTetVolMesh.cc
  ConvertMeshType/ConvertMeshToTriSurfMesh.cc
  ConvertMeshType/ConvertMeshToUnstructuredMesh.cc
  ConvertMeshType/ConvertMeshToIrregularMesh.cc
  ConvertMeshType/ConvertLatVolDataFromElemToNode.cc
  ConvertMeshType/ConvertLatVolDataFromNodeToElem.cc
  CreateMesh/CreateMeshFromNrrd.cc
  DistanceField/CalculateDistanceField.cc
  DistanceField/CalculateIsInsideField.cc
  DistanceField/CalculateInsideWhichField.cc
  DistanceField/CalculateSignedDistanceField.cc
  DomainFields/GetDomainBoundary.cc
  DomainFields/MatchDomainLabels.cc
  DomainFields/SplitNodesByDomain.cc
  DomainFields/SplitFieldByDomain.cc
  FieldData/CalculateGradients.cc
  FieldData/CalculateLatVolGradientsAtNodes.cc
  FieldData/CalculateVectorMagnitudes.cc
  FieldData/ConvertFieldDataType.cc
  FieldData/ConvertFieldBasisType.cc
  FieldData/ConvertIndicesToFieldData.cc
  FieldData/GetFieldData.cc
  FieldData/ConvertMappingMatrixToFieldData.cc
  FieldData/SetFieldData.cc
  FieldData/SetFieldDataToConstantValue.cc
  FieldData/SmoothVecFieldMedian.cc
  FilterFieldData/DilateFieldData.cc
  FilterFieldData/ErodeFieldData.cc
  FilterFieldData/TriSurfPhaseFilter.cc
  Mapping/ApplyMappingMatrix.cc
  Mapping/BuildMappingMatrix.cc
  Mapping/MapFieldDataFromElemToNode.cc
  Mapping/MapFieldDataFromNodeToElem.cc
  Mapping/MapFieldDataFromSourceToDestination.cc
  MarchingCubes/BaseMC.cc
  MarchingCubes/EdgeMC.cc
  MarchingCubes/HexMC.cc
  MarchingCubes/MarchingCubes.cc
  MarchingCubes/mcube2.cc
  MarchingCubes/PrismMC.cc
  MarchingCubes/QuadMC.cc
  MarchingCubes/TetMC.cc
  MarchingCubes/TriMC.cc
  MarchingCubes/UHexMC.cc
  MergeFields/AppendFields.cc
  MergeFields/JoinFields.cc
  MeshData/GetMeshNodes.cc
  MeshData/SetMeshNodes.cc
  MeshData/GetSurfaceNodeNormals.cc
  MeshData/GetSurfaceElemNormals.cc
  MeshData/GetMeshQualityField.cc
  MeshDerivatives/GetCentroids.cc
  MeshDerivatives/GetFieldBoundary.cc
  MeshDerivatives/GetBoundingBox.cc
  MeshDerivatives/SplitByConnectedRegion.cc
  RefineMesh/RefineMesh.cc
  SampleField/GeneratePointSamplesFromField.cc
  SmoothMesh/FairMesh.cc
  StreamLines/StreamLineIntegrators.cc
  StreamLines/GenerateStreamLines.cc
  TransformMesh/AlignMeshBoundingBoxes.cc
  TransformMesh/TransformMeshWithTransform.cc
  TransformMesh/ConvertMeshCoordinateSystem.cc
  TransformMesh/ScaleFieldMeshAndData.cc
  TransformMesh/TransformPlanarMesh.cc
  TransformMesh/GeneratePolarProjection.cc
  TransformMesh/ProjectPointsOntoMesh.cc
  TracePoints/TracePoints.cc
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
  Core_Algorithms_Math
  Core_Algorithms_Util 
  Core_Persistent 
  Core_Basis 
  ${TEEM_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Algorithms_Fields)
ENDIF(BUILD_SHARED_LIBS)

