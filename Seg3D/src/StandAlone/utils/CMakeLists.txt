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
PROJECT(SCIRUN_STANDALONE_UTILS)

FILE(MAKE_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}/utils")

SET(StandAlone_Utils_Libs
  Core_Exceptions
  Core_Datatypes
  Core_Util
  Core_Containers
  Core_Persistent
  Core_Init
  Core_Thread
  Core_Geometry
  Core_Math
  Core_ImportExport
  Core_Basis
  Core_Geom
  ${TEEM_LIBRARY}
  ${FREETYPE_LIBRARY}
  ${XML2_LIBRARY}
  ${M_LIBRARY}
  ${F_LIBRARY}
  ${PNG_LIBRARY}
  ${ZLIB_LIBRARY}
 )

# Helper macro to install utilty executables
MACRO(SCIRUN_INSTALL_UTIL executable)
  INSTALL(TARGETS ${executable}
    RUNTIME DESTINATION bin/util
    )
ENDMACRO(SCIRUN_INSTALL_UTIL)

# Helper function to add the executable and the intall at the same time
MACRO(SCIRUN_ADD_UTILITY_EXECUTABLE executable)
  ADD_EXECUTABLE(${executable} ${ARGN})
  SCIRUN_INSTALL_UTIL(${executable})
ENDMACRO(SCIRUN_ADD_UTILITY_EXECUTABLE)

IF(WIN32)
  LINK_DIRECTORIES("${EXECUTABLE_OUTPUT_PATH}/lib/${BUILD_TYPE}")
ENDIF(WIN32)

SCIRUN_ADD_UTILITY_EXECUTABLE(utils/AddTri ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/AddTri.cc)
TARGET_LINK_LIBRARIES(utils/AddTri ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(utils/FieldBin1Test ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/FieldBin1Test.cc)
TARGET_LINK_LIBRARIES(utils/FieldBin1Test ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(utils/FieldTextToBin ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/FieldTextToBin.cc)
TARGET_LINK_LIBRARIES(utils/FieldTextToBin ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(utils/GenerateMPMData ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/GenerateMPMData.cc)
TARGET_LINK_LIBRARIES(utils/GenerateMPMData ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(utils/OrientFaces ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/OrientFaces.cc)
TARGET_LINK_LIBRARIES(utils/OrientFaces ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(utils/PCA-example ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/PCA-example.cc)
TARGET_LINK_LIBRARIES(utils/PCA-example ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(utils/RemoveConnectedFaces ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/RemoveConnectedFaces.cc)
TARGET_LINK_LIBRARIES(utils/RemoveConnectedFaces ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(utils/RemoveFaces ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/RemoveFaces.cc)
TARGET_LINK_LIBRARIES(utils/RemoveFaces ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(utils/RemoveOrphanNodes ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/RemoveOrphanNodes.cc)
TARGET_LINK_LIBRARIES(utils/RemoveOrphanNodes ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(utils/SwapFaces ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/SwapFaces.cc)
TARGET_LINK_LIBRARIES(utils/SwapFaces ${StandAlone_Utils_Libs})

#SCIRUN_ADD_UTILITY_EXECUTABLE(utils/TestBasis ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/TestBasis.cc)
#TARGET_LINK_LIBRARIES(utils/TestBasis ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(utils/UnitElementMesh ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/UnitElementMesh.cc)
TARGET_LINK_LIBRARIES(utils/UnitElementMesh ${StandAlone_Utils_Libs})
