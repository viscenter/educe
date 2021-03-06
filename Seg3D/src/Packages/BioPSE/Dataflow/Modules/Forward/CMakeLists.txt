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

# CMakeLists.txt for Packages/BioPSE/Dataflow/Modules/Forward

SET(Packages_BioPSE_Dataflow_Modules_Forward_SRCS
  CreateAnisoSphereParameters.cc
  ApplyFEMCurrentSource.cc
  ApplyFEMElectrodeSource.cc
  ApplyFEMVoltageSource.cc
  BuildBEMSrcToInfTransferMatrix.cc
  CalculateCurrentDensity.cc
  SetWireElectrodePotential.cc
  SetWireElectrodeProperties.cc
  CreateDiscreteAnisoSpheres.cc
  SimulateDipoleInSphereAnalytically.cc
  SimulateDipoleInAnisoSpheresAnalytically.cc
  SimulateForwardMagneticField.cc
  GetActivationTimes.cc
  ConvertFieldDataFromIndicesToTensors.cc
  InsertElectrodes.cc
  InsertVoltageSource.cc
  IntegrateCurrent.cc
  BuildBEMatrix.cc
  BuildFEMatrix.cc
  ConvertFieldDataFromTensorsToIndices.cc
  CreateElectrodeParameterMatrices.cc
  SimulateEITAnalytically.cc
  SetEITGround.cc
  CreateTrigCurrentPattern.cc
  BuildEITMatrix.cc
)

SCIRUN_ADD_LIBRARY(Packages_BioPSE_Dataflow_Modules_Forward ${Packages_BioPSE_Dataflow_Modules_Forward_SRCS})

TARGET_LINK_LIBRARIES(Packages_BioPSE_Dataflow_Modules_Forward
  Packages_BioPSE_Core_Algorithms_Forward
  Packages_BioPSE_Core_Algorithms_NumApproximation
  Packages_BioPSE_Core_Datatypes
  Core_Basis
  Core_Containers
  Core_Datatypes
  Core_Exceptions
  Core_Geom
  Core_Geometry
  Core_GeomInterface
  Dataflow_GuiInterface
  Core_Persistent
  Core_Thread
  Dataflow_TkExtensions
  Core_Util
  Dataflow_Network
  Core_Algorithms_Math
  ${TK_LIBRARY}
  ${GL_LIBRARY}
  ${M_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Packages_BioPSE_Dataflow_Modules_Forward)
ENDIF(BUILD_SHARED_LIBS)
