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

# CMakeLists.txt for sr_py

SET(sr_py_SRCS
  api.cc
  sr_py_wrap.cc	
)

INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

ADD_LIBRARY(_sr_py MODULE ${sr_py_SRCS})
SET_TARGET_PROPERTIES(_sr_py PROPERTIES PREFIX "")

TARGET_LINK_LIBRARIES(_sr_py
  Core_Util
  Core_Init
  Core_Basis
  Core_Events
  Core_Events_CM2View
  Core_Events_OGLView
  Core_Datatypes
  Core_Geom
  Core_Geometry
  Core_Exceptions
  Core_Algorithms_Interface
  ${PYTHON_LIBRARIES}
)

IF(HAVE_TETGEN)
  ADD_DEFINITIONS(-DHAVE_TETGEN)
ENDIF(HAVE_TETGEN)

ADD_CUSTOM_COMMAND(TARGET _sr_py POST_BUILD COMMAND 
                   ${CMAKE_COMMAND} ARGS -E copy ${SCIRUN_CORE_SOURCE_DIR}/sr_py/sr_py.py ${LIBRARY_OUTPUT_PATH})

IF (WIN32)
  # for windows, python wants a .pyd extension
  ADD_CUSTOM_COMMAND(TARGET _sr_py POST_BUILD COMMAND 
                     ${CMAKE_COMMAND} ARGS -E copy ${LIBRARY_OUTPUT_PATH}/${BUILD_TYPE}/_sr_py.dll ${LIBRARY_OUTPUT_PATH}/${BUILD_TYPE}/_sr_py.pyd)
  ADD_CUSTOM_COMMAND(TARGET _sr_py POST_BUILD COMMAND 
                     ${CMAKE_COMMAND} ARGS -E copy ${SCIRUN_CORE_SOURCE_DIR}/sr_py/sr_py.py ${LIBRARY_OUTPUT_PATH}/${BUILD_TYPE})
ELSE(WIN32)
  ADD_CUSTOM_COMMAND(TARGET _sr_py POST_BUILD COMMAND 
                     ${CMAKE_COMMAND} ARGS -E copy ${SCIRUN_CORE_SOURCE_DIR}/sr_py/sr_py.py ${LIBRARY_OUTPUT_PATH})
ENDIF(WIN32)

#ADD_CUSTOM_COMMAND(TARGET _sr_py POST_BUILD COMMAND python ${SCIRUN_CORE_SOURCE_DIR}/sr_py/fix.py)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD__sr_py)
ENDIF(BUILD_SHARED_LIBS)