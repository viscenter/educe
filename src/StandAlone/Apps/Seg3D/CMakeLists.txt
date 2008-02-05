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

# This is the root ITK CMakeLists file.  
CMAKE_MINIMUM_REQUIRED(VERSION 2.0)

PROJECT(SEG3D)

#-----------------------------------------------------------------------------
# Test for some required system information.
INCLUDE (CMakeBackwardCompatibilityC)

#-----------------------------------------------------------------------------
# Configure testing support.  This should be done before any
# MESSAGE(FATAL_ERROR ...) commands are invoked.
INCLUDE( CTest )
IF(BUILD_TESTING)
  ENABLE_TESTING()
  SET(BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
  MARK_AS_ADVANCED(BUILDNAME)
ENDIF(BUILD_TESTING)

#----------------------------------------------------------------------------
# Output directories.
IF(NOT BUILD_FROM_TOP_LEVEL)
  SET (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all libraries.")
  SET (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all executables.")
  #output directory for installing all include files.")
  MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)
ENDIF(NOT BUILD_FROM_TOP_LEVEL)


# DV Find SCIRun Core stuff
SET(CMAKE_MODULE_PATH ${SEG3D_SOURCE_DIR})
FIND_PACKAGE( SCIRUN )
IF(SCIRUN_FOUND)
  INCLUDE(${SCIRUN_USE_FILE})
  SET(SCIRUN_DIR . CACHE PATH "Path to SCIRun Core")
ELSE(SCIRUN_FOUND)
  MESSAGE(FATAL_ERROR "Cannot build Seg3D without SCIRUN Core. Please set SCIRUN_DIR.")
ENDIF(SCIRUN_FOUND)

IF(NOT ITK_DIR)
  SET(ITK_DIR ${SCIRUN_ITK_DIR})
ENDIF(NOT ITK_DIR)

FIND_PACKAGE( ITK )
IF(ITK_FOUND)
  INCLUDE(${ITK_USE_FILE})
ELSE(ITK_FOUND)
  MESSAGE(FATAL_ERROR "Cannot build Seg3D without ITK. Please set ITK_DIR.")
ENDIF(ITK_FOUND)

IF(WIN32)
  IF(NOT BUILD_CORE_LIBS_SHARED)
    ADD_DEFINITIONS(-DBUILD_SCIRUN_STATIC)
  ENDIF(NOT BUILD_CORE_LIBS_SHARED)
#  This defines M_PI for windows
  ADD_DEFINITIONS(-D_USE_MATH_DEFINES)
ENDIF(WIN32)


SET(Seg3D_SRCS 
  main.cc
  plugins.cc
	AutoviewTool.cc 
  BrushTool.cc 
  BrushFloodFill.cc 
  CLUTLevelsTool.cc
  CropTool.cc
  FloodfillTool.cc
  FloodFillCopyTool.cc
  ITKConnectedImageFilterTool.cc
  ITKNeighborhoodConnectedImageFilterTool.cc
  ITKConfidenceConnectedImageFilterTool.cc
  ITKCurvatureAnisotropicDiffusionImageFilterTool.cc
  ITKThresholdSegmentationLevelSetImageFilterTool.cc
  ITKOtsuImageFilterTool.cc
  ITKGradientMagnitudeImageFilterTool.cc
  ITKHoleFillImageFilterTool.cc
  ITKBinaryDilateErodeImageFilterTool.cc
  ITKThresholdImageFilterTool.cc
  ITKDiscreteGaussianImageFilterTool.cc
  KeyToolSelectorTool.cc
  LayerButton.cc
  LayerMergeTool.cc
  NrrdVolume.cc
  Painter.cc 
  PainterSignalTargets.cc 
  PanTool.cc 
  ProbeTool.cc 
  PointerToolSelectorTool.cc
  StatisticsTool.cc
  Seg3DFrame.cc
  SessionReader.cc
  SessionWriter.cc
  SliceWindow.cc
  VolumeFilter.cc
  VolumeOps.cc
  VolumeSlice.cc
  WXOpenGLContext.cc
  Seg3DwxGuiUtils.cc
  ZoomTool.cc
  GuiCode/seg3devents.cpp
  GuiCode/brushpanel.cpp
  GuiCode/croppanel.cpp
  GuiCode/itkcurvatureanistopricdiffusionfilter.cpp
  GuiCode/itkconfidenceconnectedfilter.cpp
  GuiCode/itkneighbourhoodconnectedfilter.cpp
  GuiCode/itkbinarydilateerodefilter.cpp
  GuiCode/itkthresholdsegmentationlevelsetfilter.cpp
  GuiCode/cropvolume.cpp
  GuiCode/cursorinformation.cpp
  GuiCode/fliptool.cpp
  GuiCode/resampletool.cpp
  GuiCode/medianfiltertool.cpp
  GuiCode/maskfilter.cpp
  GuiCode/optionlessfilter.cpp
  GuiCode/itkthresholdfilter.cpp
  GuiCode/itkDiscreteGaussianImageFilter.cpp
)

IF(WIN32)
  SET(Seg3D_SRCS ${Seg3D_SRCS} seg3d.rc)
ENDIF(WIN32)

SET(Seg3D_LIBS
  ${XML_LIBRARY} 
  ${M_LIBRARY} 
  ${GL_LIBRARY} 
  ${TEEM_LIBRARY}
  ${SLIVR_LIBRARY}
  Core_Matlab 
  Core_Basis 
  Core_Bundle 
  Core_Containers 
  Core_Datatypes 
  Core_Events 
  Core_Exceptions 
  Core_Geom 
  Core_Geometry 
  Core_Math 
  Core_Persistent 
  Core_Skinner 
  Core_Thread 
  Core_Util 
  Core_Volume
  ITKCommon
  ITKBasicFilters
  ITKIO 
)

IF(X11_FOUND)
  SET(Seg3D_LIBS ${Seg3D_LIBS} ${GLU_LIBRARY})
ENDIF(X11_FOUND)

ADD_EXECUTABLE(Seg3D MACOSX_BUNDLE ${Seg3D_SRCS})
TARGET_LINK_LIBRARIES(Seg3D ${Seg3D_LIBS})


IF(wxWidgets_FOUND)
  TARGET_LINK_LIBRARIES( Seg3D ${wxWidgets_LIBRARIES})
ENDIF(wxWidgets_FOUND)

IF(APPLE)
  SET_TARGET_PROPERTIES(Seg3D PROPERTIES LINK_FLAGS "-framework Carbon")
ENDIF(APPLE)


IF(APPLE)
  SET(DATA_PATH ${EXECUTABLE_OUTPUT_PATH}/Seg3D.app/Contents/MacOS/data)
ELSE(APPLE)
  IF(WIN32)
    SET(DATA_PATH ${EXECUTABLE_OUTPUT_PATH}/${BUILD_TYPE}/data)
  ELSE(WIN32)
    SET(DATA_PATH ${EXECUTABLE_OUTPUT_PATH}/data)
  ENDIF(WIN32)
ENDIF(APPLE)


ADD_CUSTOM_COMMAND(TARGET Seg3D
                   POST_BUILD
                   COMMAND mkdir
                   ARGS -p ${DATA_PATH})

#IF(NOT WIN32)
#
#ADD_CUSTOM_COMMAND(TARGET Seg3D
#                   POST_BUILD
#                   COMMAND cp
#                   ARGS ${SEG3D_SOURCE_DIR}/data/*
#                   ${DATA_PATH}/)
#                   
#ENDIF(NOT WIN32)


#IF(WIN32)
#
#ADD_CUSTOM_COMMAND(TARGET Seg3D
#                   POST_BUILD
#                   COMMAND copy
#                   ARGS ${SEG3D_SOURCE_DIR}/data/*
#                   ${DATA_PATH}/)
#                   
#ENDIF(WIN32)

ADD_CUSTOM_COMMAND(TARGET Seg3D
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND}
                   ARGS -E copy_directory 
                   ${SEG3D_SOURCE_DIR}/data/ 
                   ${DATA_PATH}/)


IF(APPLE)

  ADD_CUSTOM_COMMAND(TARGET Seg3D
                     POST_BUILD
                     COMMAND mkdir
                     ARGS -p
                     ${EXECUTABLE_OUTPUT_PATH}/Seg3D.app/Contents/Resources)

  ADD_CUSTOM_COMMAND(TARGET Seg3D
                     POST_BUILD
                     COMMAND cp
                     ARGS ${SEG3D_SOURCE_DIR}/OSX/Info.plist
                     ${EXECUTABLE_OUTPUT_PATH}/Seg3D.app/Contents/)

  ADD_CUSTOM_COMMAND(TARGET Seg3D
                     POST_BUILD
                     COMMAND cp
                     ARGS ${SEG3D_SOURCE_DIR}/OSX/Resources/*
                     ${EXECUTABLE_OUTPUT_PATH}/Seg3D.app/Contents/Resources)

  ADD_CUSTOM_COMMAND(TARGET Seg3D
                     POST_BUILD
                     COMMAND cp
                     ARGS ${SEG3D_SOURCE_DIR}/data/splash-seg3d.png
                     ${EXECUTABLE_OUTPUT_PATH}/Seg3D.app/Contents/Resources)
                     
  ADD_CUSTOM_COMMAND(TARGET Seg3D
                     POST_BUILD
                     COMMAND cp
                     ARGS ${SEG3D_SOURCE_DIR}/data/Seg3D.xrc
                     ${EXECUTABLE_OUTPUT_PATH}/Seg3D.app/Contents/Resources)

ENDIF(APPLE)
