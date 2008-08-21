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

# CMakeLists.txt for main

SET (EXECUTABLE_OUTPUT_PATH ${SCIRUN_CORE_BINARY_DIR}/ CACHE INTERNAL "Output directory for SCIRun executable")

SET(SCIRUN_SRCS ${SCIRUN_CORE_SOURCE_DIR}/main/main.cc)

IF(WIN32)
  SET(SCIRUN_SRCS ${SCIRUN_SRCS} ${SCIRUN_CORE_SOURCE_DIR}/main/scirun.rc)
ENDIF(WIN32)

MESSAGE(STATUS "scirun=${SCIRUN_SRCS}")

# scirun executable
ADD_EXECUTABLE(scirun ${SCIRUN_SRCS})
SCIRUN_INSTALL_EXECUTABLE(scirun)

SET(SCIRUN_LIBS
  Core_Basis         
  Core_Containers    
  Core_Events
  Core_Exceptions 
  Core_Geom          
  Dataflow_GuiInterface  
  Core_ICom          
  Core_Init         
  Core_Services      
  Core_SystemCall    
  Core_Thread        
  Dataflow_TkExtensions  
  Core_Util          
  Core_Volume        
  Core_XMLUtil       
  Dataflow_Network   
  Dataflow_TCLThread
  ${GLEW_LIBRARY}
)

TARGET_LINK_LIBRARIES(scirun ${SCIRUN_LIBS}
)

IF(CMAKE_SYSTEM_NAME MATCHES Darwin)
    TARGET_LINK_LIBRARIES(scirun Core_Datatypes Core_ImportExport Core_Persistent)
ENDIF(CMAKE_SYSTEM_NAME MATCHES Darwin)
