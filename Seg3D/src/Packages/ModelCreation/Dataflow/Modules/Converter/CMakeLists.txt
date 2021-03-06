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

# CMakeLists.txt for Packages/ModelCreation/Dataflow/Modules/Converter

SET(Packages_ModelCreation_Dataflow_Modules_Converter_SRCS
  ConvertMatrixToString.cc
  ConvertMatrixToField.cc
  ConvertNrrdToField.cc
  ConvertMatricesToDipoleField.cc
)

SCIRUN_ADD_LIBRARY(Packages_ModelCreation_Dataflow_Modules_Converter ${Packages_ModelCreation_Dataflow_Modules_Converter_SRCS})

TARGET_LINK_LIBRARIES(Packages_ModelCreation_Dataflow_Modules_Converter
  Core_Datatypes
  Dataflow_Network
  Core_Persistent
  Core_Containers
  Core_Util
  Core_Exceptions
  Core_Thread
  Dataflow_GuiInterface
  Core_Geom
  Core_Datatypes
  Core_Geometry
  Core_GeomInterface
  Dataflow_TkExtensions
  Core_Algorithms_Converter
  ${TK_LIBRARY}
  ${GL_LIBRARY}
  ${M_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Packages_ModelCreation_Dataflow_Modules_Converter)
ENDIF(BUILD_SHARED_LIBS)
