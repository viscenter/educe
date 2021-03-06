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

# Sources of Core/ImportExport classes

# NOTE: source files are all in sub directories ColorMap, Matrix, Field,
# and Nrrd

SET(Core_ImportExport_SRCS
  ColorMap/ColorMapIEPlugin.cc
  ColorMap/ColorMapExecConverter.cc
  Matrix/MatrixExecConverter.cc 
  Matrix/MatrixIEPlugin.cc
  Field/FieldExecConverter.cc 
  Field/FieldIEPlugin.cc 
  Field/TextPointCloudString_plugin.cc
  Nrrd/NrrdIEPlugin.cc
)

## Not implemented yet...
#
#  Nrrd/NrrdExecConverter.cc 

SCIRUN_ADD_LIBRARY(Core_ImportExport ${Core_ImportExport_SRCS})

TARGET_LINK_LIBRARIES(Core_ImportExport 
  Core_Persistent 
  Core_Exceptions 
  Core_Containers 
  Core_Thread 
  Core_Geometry 
  Core_Geom 
  Core_Math 
  Core_Util 
  Core_Datatypes
  ${THREAD_LIBRARY} 
  ${Z_LIBRARY} 
  ${M_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_ImportExport)
ENDIF(BUILD_SHARED_LIBS)
