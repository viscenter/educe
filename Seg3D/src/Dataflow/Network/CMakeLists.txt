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

# CMakeLists.txt for Dataflow/Network


SET(Dataflow_Network_SRCS
  ComponentNode.cc
  Connection.cc
  GenFiles.cc
  Module.cc
  ModuleHelper.cc
  Network.cc
  NetworkEditor.cc
  NetworkIO.cc
  PackageDB.cc
  SkeletonFiles.cc
  Scheduler.cc
  Ports/BundlePort.cc
  Ports/ColorMapPort.cc
  Ports/ColorMap2Port.cc
  Ports/FieldPort.cc
  Ports/GeometryPort.cc
  Ports/ImagePort.cc
  Ports/MatrixPort.cc
  Ports/NrrdPort.cc
  Ports/Port.cc
  Ports/PathPort.cc
  Ports/StringPort.cc
  Ports/TexturePort.cc
  Ports/TimePort.cc
)

IF(ITK_FOUND)
  SET(Dataflow_Network_SRCS ${Dataflow_Network_SRCS} Ports/ITKDatatypePort.cc)
ENDIF(ITK_FOUND)

INCLUDE_DIRECTORIES(${SLIVR_INCLUDE_DIR})

SCIRUN_ADD_LIBRARY(Dataflow_Network ${Dataflow_Network_SRCS})

TARGET_LINK_LIBRARIES(Dataflow_Network
  Dataflow_Comm
  Core_Datatypes
  Core_Exceptions
  Core_Containers
  Core_Geom
  Core_GeomInterface
  Core_Geometry
  Dataflow_GuiInterface
  Core_Util
  Core_Thread
  Core_Volume
  Core_XMLUtil
  Core_OS
  StaticHelper
  ${TCL_LIBRARY}
  ${XML2_LIBRARY}
  ${SLIVR_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Dataflow_Network)
ENDIF(BUILD_SHARED_LIBS)
