#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2007 Scientific Computing and Imaging Institute,
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

IF(NOT BUILD_SHARED_LIBS)
  SET(EXECUTABLE_OUTPUT_PATH ${SCIRUN_CORE_BINARY_DIR}/StaticHelper)
  ADD_EXECUTABLE(ExtractSymbolsFromXML ExtractSymbolsFromXML.cc)
  TARGET_LINK_LIBRARIES(ExtractSymbolsFromXML
      Core_XMLUtil
      Core_Util
    )

  # Get the full path to the ExtractSymbolsFromXML executable
  GET_TARGET_PROPERTY(ExtractSymbolsFromXML_exe ExtractSymbolsFromXML LOCATION)
  #MESSAGE("ExtractSymbolsFromXML_exe = ${ExtractSymbolsFromXML_exe}")
  # Get the path to the executable
  GET_FILENAME_COMPONENT(binary_path ${ExtractSymbolsFromXML_exe} PATH)
  SET(Loader_cc "${binary_path}/Loader.cc")
  SET_SOURCE_FILES_PROPERTIES(
    ${Loader_cc}
    PROPERTIES GENERATED TRUE
    )
  ADD_CUSTOM_COMMAND(
    OUTPUT ${Loader_cc}
    COMMAND ${ExtractSymbolsFromXML_exe} -o ${Loader_cc}
    # Don't try and add a MAIN_DEPENDENCY of the executable here as
    # CMake will barf on you about

#    MAIN_DEPENDENCY ExtractSymbolsFromXML
    DEPENDS ExtractSymbolsFromXML
    COMMENT "Generating static loader file: ${Loader_cc}"
    )
  SCIRUN_ADD_LIBRARY(StaticHelper Loader.h ${Loader_cc})

  TARGET_LINK_LIBRARIES(StaticHelper
    Dataflow_Modules_Bundle
    Dataflow_Modules_Converters
    Dataflow_Modules_DataArrayMath
    Dataflow_Modules_DataIO
    Dataflow_Modules_Examples
    Dataflow_Modules_Fields
    Dataflow_Modules_Math
    Dataflow_Modules_Render
    Dataflow_Modules_String
    Dataflow_Modules_Time
    Dataflow_Modules_Visualization
    )

  TARGET_LINK_LIBRARIES(StaticHelper
    Packages_BioPSE_Dataflow_Modules_DataIO
    Packages_BioPSE_Dataflow_Modules_Forward
    Packages_BioPSE_Dataflow_Modules_Inverse
    Packages_BioPSE_Dataflow_Modules_LeadField
    Packages_BioPSE_Dataflow_Modules_Modeling
    Packages_BioPSE_Dataflow_Modules_NeuroFEM
    Packages_BioPSE_Dataflow_Modules_Visualization
    )
  
  TARGET_LINK_LIBRARIES(StaticHelper
    Packages_MatlabInterface_Dataflow_Modules_DataIO
    Packages_MatlabInterface_Dataflow_Modules_Interface
    )

  TARGET_LINK_LIBRARIES(StaticHelper
    Packages_Teem_Dataflow_Modules_Converters
    Packages_Teem_Dataflow_Modules_DataIO
    Packages_Teem_Dataflow_Modules_Misc
    Packages_Teem_Dataflow_Modules_Tend
    Packages_Teem_Dataflow_Modules_Unu
    )

ELSE(NOT BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_StaticHelper)
  SCIRUN_ADD_LIBRARY(StaticHelper Loader.h LoaderShared.cc)
ENDIF(NOT BUILD_SHARED_LIBS)

