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

# CMakeLists.txt for Dataflow/Modules/Fields

SET(Dataflow_Modules_Fields_SRCS
  RegisterWithCorrespondences.cc
  SmoothVecFieldMedian.cc
  SetFieldDataToConstantValue.cc
  RemoveUnusedNodes.cc
  GeneratePolarProjection.cc
  MapFieldDataOntoNodes.cc
  MapFieldDataOntoElems.cc
  CleanupTetMesh.cc
  MatchDomainLabels.cc
  SplitFieldByDomain.cc
  AlignMeshBoundingBoxes.cc
  ProjectPointsOntoMesh.cc
  GetMeshQualityField.cc
  ApplyFilterToFieldData.cc
  ApplyMappingMatrix.cc
  ApplyTriSurfPhaseFilter.cc
  CalculateNodeNormals.cc
  BuildMappingMatrix.cc
  BuildMatrixOfSurfaceNormals.cc
  GetCentroidsFromMesh.cc
  ConvertMeshCoordinateSystem.cc
  ConvertFieldBasis.cc
  ConvertFieldDataType.cc
  ChooseField.cc
  ClipFieldByFunction.cc
  ClipFieldByFunction2.cc
  ClipFieldByFunction3.cc
  ClipFieldWithSeeds.cc
  ClipFieldByMesh.cc
  ClipFieldByFunctionCompiled.cc
  ClipFieldToFieldOrWidget.cc
  ClipLatVolByIndicesOrWidget.cc
  ClipFieldWithSeedCompiled.cc
  ConvertMatricesToMesh.cc
  CollectPointClouds.cc
  CoregisterPointClouds.cc
  DecimateTriSurf.cc
  EditMeshBoundingBox.cc
  ExtractIsosurfaceByFunctionCompiled.cc
  ExtractIsosurfaceByFunction.cc
  InterfaceWithCubit.cc
  MapFieldDataFromSourceToDestination.cc
  MaskLatVolWithTriSurf.cc
  RemoveHexVolSheet.cc
  FairMesh.cc
  GetAllSegmentationBoundaries.cc
  GetFieldBoundary.cc
  ReportFieldInfo.cc
  ReportFieldGeometryMeasures.cc
  SetFieldProperty.cc
  GetSliceFromStructuredFieldByIndices.cc
  SubsampleStructuredFieldByIndices.cc
  JoinFields.cc
  CalculateGradients.cc
  ConvertHexVolToTetVol.cc
  MergeFields.cc
  InsertHexVolSheetAlongSurface.cc
  MergeTriSurfs.cc
  ClipVolumeByIsovalue.cc
  RefineMesh.cc
  SwapFieldDataWithMatrixEntries.cc
  SwapNodeLocationsWithMatrixEntries.cc
  ConvertLatVolDataFromElemToNode.cc
  ConvertLatVolDataFromNodeToElem.cc
  CalculateLatVolGradientsAtNodes.cc
  ConvertQuadSurfToTriSurf.cc
  TransformPlanarMesh.cc
  BuildPointCloudToLatVolMappingMatrix.cc
  GenerateSinglePointProbeFromField.cc
  GeneratePointSamplesFromFieldOrWidget.cc
  CreateLatVol.cc
  CreateImage.cc
  CreateStructHex.cc
  ReportScalarFieldStats.cc
  GeneratePointSamplesFromField.cc
  SelectFieldROIWithBoxWidget.cc
  SetFieldOrMeshStringProperty.cc
  ConvertMeshToPointCloud.cc
  ConvertMeshToIrregularMesh.cc
  ConvertMeshToUnstructuredMesh.cc
  CalculateFieldData.cc
  CalculateFieldData2.cc
  CalculateFieldData3.cc
  CalculateMeshNodes.cc
  CalculateFieldDataCompiled.cc
  QueryFieldDataCompiled.cc
  TransformMeshWithFunction.cc
  TransformMeshWithTransform.cc
  CalculateVectorMagnitudes.cc
  GenerateLinearSegments.cc
  CalculateDistanceToField.cc
  CalculateDistanceToFieldBoundary.cc
  CalculateSignedDistanceToField.cc
  CalculateIsInsideField.cc
  CalculateInsideWhichField.cc
  ConvertIndicesToFieldData.cc
  CreateFieldData.cc
  GetFieldData.cc
  SetFieldData.cc
  SetMatIndex.cc
  MapFieldDataFromElemToNode.cc
  MapFieldDataFromNodeToElem.cc
  SelectAndSetFieldData.cc
  SelectAndSetFieldData3.cc
  SplitNodesByDomain.cc
  SplitFieldByConnectedRegion.cc
  GetDomainBoundary.cc
  CollectFields.cc
  ManageFieldSeries.cc
  ScaleFieldMeshAndData.cc
  TracePoints.cc
)

IF(HAVE_TETGEN)
  SET(Dataflow_Modules_Fields_SRCS ${Dataflow_Modules_Fields_SRCS}
    InterfaceWithTetGen.cc)
ELSE(HAVE_TETGEN)
  SET(Dataflow_Modules_Fields_SRCS ${Dataflow_Modules_Fields_SRCS}
    InterfaceWithTetGen-stub.cc)
ENDIF(HAVE_TETGEN)
    
IF (HAVE_CAMAL)
  SET(Dataflow_Modules_Fields_SRCS ${Dataflow_Modules_Fields_SRCS}
    InterfaceWithCamal.cc)  
ELSE(HAVE_CAMAL)    
  SET(Dataflow_Modules_Fields_SRCS ${Dataflow_Modules_Fields_SRCS}
    InterfaceWithCamal-stub.cc)  
ENDIF(HAVE_CAMAL)    

IF (HAVE_VTK)
  SET(Dataflow_Modules_Fields_SRCS ${Dataflow_Modules_Fields_SRCS}
    ReportMeshQualityMeasures.cc)  
ELSE(HAVE_VTK)
  SET(Dataflow_Modules_Fields_SRCS ${Dataflow_Modules_Fields_SRCS}
    ReportMeshQualityMeasures-stub.cc)
ENDIF(HAVE_VTK)    

IF (HAVE_MESQUITE)
  SET(Dataflow_Modules_Fields_SRCS ${Dataflow_Modules_Fields_SRCS}
    SmoothMesh.cc 
    MesquiteDomain.cc
    MesquiteMesh.cc
)  
ELSE(HAVE_MESQUITE)    
  SET(Dataflow_Modules_Fields_SRCS ${Dataflow_Modules_Fields_SRCS}
    SmoothMesh-stub.cc)  
ENDIF(HAVE_MESQUITE)    
    
SCIRUN_ADD_LIBRARY(Dataflow_Modules_Fields ${Dataflow_Modules_Fields_SRCS})

TARGET_LINK_LIBRARIES(Dataflow_Modules_Fields
  Dataflow_Network
  Dataflow_Widgets
  Dataflow_GuiInterface
  Dataflow_TkExtensions
  Core_Algorithms_Fields
  Core_Algorithms_Util
  Core_Algorithms_Visualization
  Core_Algorithms_Geometry
  Core_Algorithms_Converter
  Core_Basis
  Core_Datatypes
  Core_Exceptions
  Core_Geom
  Core_Geometry
  Core_GeomInterface
  Core_Containers
  Core_ImportExport
  Core_Math
  Core_Parser
  Core_Persistent
  Core_Thread
  Core_Util
  ${TK_LIBRARY}
  ${GL_LIBRARY}
  ${LEX_LIBRARY}
  ${M_LIBRARY}
  ${THREAD_LIBRARY}
  ${TEEM_LIBRARY}
  ${SLIVRG_LIBRARY}
)

IF (HAVE_TETGEN)
  TARGET_LINK_LIBRARIES(Dataflow_Modules_Fields
    ${TETGEN_LIBRARY})
ENDIF(HAVE_TETGEN)

IF (HAVE_CAMAL)
  TARGET_LINK_LIBRARIES(Dataflow_Modules_Fields
    ${CAMAL_LIBRARIES}
    ${F_LIBRARY})
ENDIF(HAVE_CAMAL)

IF (HAVE_MESQUITE)
  TARGET_LINK_LIBRARIES(Dataflow_Modules_Fields
    ${MESQUITE_LIBRARY})
ENDIF(HAVE_MESQUITE)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Dataflow_Modules_Fields)
ENDIF(BUILD_SHARED_LIBS)

