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

# CMakeLists.txt for Packages/BioPSE/Core/Algorithms/NumApproximation

SET(Packages_BioPSE_Core_Algorithms_NumApproximation_SRCS
  BuildBEMatrix.cc
)

SCIRUN_ADD_LIBRARY(Packages_BioPSE_Core_Algorithms_NumApproximation ${Packages_BioPSE_Core_Algorithms_NumApproximation_SRCS})

TARGET_LINK_LIBRARIES(Packages_BioPSE_Core_Algorithms_NumApproximation
  Core_Datatypes
  Core_Persistent
  Core_Exceptions
  Core_Containers
  Core_Thread
  Core_Geometry
  Core_Geom
  Dataflow_GuiInterface
  Core_Math
  Core_Util
  Core_Basis
  ${TK_LIBRARY}
  ${GL_LIBRARY}
  ${M_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Packages_BioPSE_Core_Algorithms_NumApproximation)
ENDIF(BUILD_SHARED_LIBS)
