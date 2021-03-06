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

# This is the root SCIRun CMakeLists file.
CMAKE_MINIMUM_REQUIRED(VERSION 2.2)

PROJECT(SCIRUN_DATAFLOW)

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
  SET (LIBRARY_OUTPUT_PATH ${SCIRUN_DATAFLOW_BINARY_DIR}/lib CACHE INTERNAL "Single output directory for building all libraries.")
  SET (EXECUTABLE_OUTPUT_PATH ${SCIRUN_DATAFLOW_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all executables.")
  #output directory for installing all include files.")
  MARK_AS_ADVANCED(LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH)
ENDIF(NOT BUILD_FROM_TOP_LEVEL)


# DV Find SCIRun Core stuff
SET(CMAKE_MODULE_PATH ${SCIRUN_DATAFLOW_SOURCE_DIR})
FIND_PACKAGE( SCIRUN )
IF(SCIRUN_FOUND)
  INCLUDE(${SCIRUN_USE_FILE})
  SET(SCIRUN_DIR ${SCIRUN_CORE_BINARY_DIR} CACHE PATH "Path to SCIRun Core")
ELSE(SCIRUN_FOUND)
  MESSAGE(FATAL_ERROR "Cannot build SCIRun Dataflow without SCIRUN Core. Please set SCIRUN_DIR.")
ENDIF(SCIRUN_FOUND)

# DV Fix when fixing separate building option
IF(WITH_ITK)
  IF(NOT ITK_DIR)
    SET(ITK_DIR ${SCIRUN_ITK_DIR})
  ENDIF(NOT ITK_DIR)

  FIND_PACKAGE( ITK )
  IF(ITK_FOUND)
    INCLUDE(${ITK_USE_FILE})
  ELSE(ITK_FOUND)
    MESSAGE(FATAL_ERROR "SCIRun Core build with the Insight Toolkit. Please set ITK_DIR.")
  ENDIF(ITK_FOUND)
ENDIF(WITH_ITK)

# TCL include directories
SET(TCL_SRC_PATH ${SCIRUN_THIRDPARTY_DIR}/src/tcl CACHE PATH "Path to tcl source")
SET(TCL_INCLUDE1 ${TCL_SRC_PATH}/generic)
IF(CMAKE_SYSTEM_NAME MATCHES Darwin)
  SET(TCL_INCLUDE2 ${TCL_SRC_PATH}/mac ${TCL_SRC_PATH}/unix)
ELSE(CMAKE_SYSTEM_NAME MATCHES Darwin)
  SET(TCL_INCLUDE2 ${TCL_SRC_PATH}/unix)
ENDIF(CMAKE_SYSTEM_NAME MATCHES Darwin)

FIND_PATH(TCL_INCLUDE tcl.h
           ${SCIRUN_THIRDPARTY_DIR}/include
           /usr/local/include
           NO_DEFAULT_PATH)
MARK_AS_ADVANCED(TCL_SRC_PATH TCL_INCLUDE1 TCL_INCLUDE2 TCL_INCLUDE)
INCLUDE_DIRECTORIES(${TCL_INCLUDE1} ${TCL_INCLUDE2})

# TK include directories
SET(TK_SRC_PATH ${SCIRUN_THIRDPARTY_DIR}/src/tk CACHE PATH "Path to tk source")
SET(TK_INCLUDE1 ${TK_SRC_PATH}/generic)
IF(CMAKE_SYSTEM_NAME MATCHES Darwin)
  SET(TK_INCLUDE2 ${TK_SRC_PATH}/mac ${TK_SRC_PATH}/unix)
ELSE(CMAKE_SYSTEM_NAME MATCHES Darwin)
  SET(TK_INCLUDE2 ${TK_SRC_PATH}/unix)
ENDIF(CMAKE_SYSTEM_NAME MATCHES Darwin)

FIND_PATH(TK_INCLUDE tk.h
           ${SCIRUN_THIRDPARTY_DIR}/include
           /usr/local/include
           NO_DEFAULT_PATH)
MARK_AS_ADVANCED(TK_INCLUDE TK_SRC_PATH TK_INCLUDE1 TK_INCLUDE2)

INCLUDE_DIRECTORIES(${TK_INCLUDE1} ${TK_INCLUDE2})

# Create on-the-fly-libs stuff
# DV Fix - these shouldn't necesarily point to thirdparty
GET_DIRECTORY_PROPERTY(OTFL_INCLUDES_LIST INCLUDE_DIRECTORIES)
SET(OTFL_INCLUDES "-I${SCIRUN_CORE_SOURCE_DIR}/Packages")
FOREACH(next_ITEM ${OTFL_INCLUDES_LIST})
   SET(OTFL_INCLUDES "${OTFL_INCLUDES} -I${next_ITEM}")
ENDFOREACH(next_ITEM ${OTFL_INCLUDES_LIST})

SET(scirun_obj_dataflow_dir ${SCIRUN_DATAFLOW_BINARY_DIR})

SET(IS_OSX "no")
SET(IS_WIN "no")
SET(IS_AIX "no")

# CFLAGS is only used in the on-the-fly-libs Makefile.
SET(OTFL_CFLAGS "${CMAKE_CXX_FLAGS}")

SET(SO_OR_A_FILE "so" CACHE STRING "library extension")
SET(SOFLAGS "${OTFL_CFLAGS} -shared -L${LIBRARY_OUTPUT_PATH}")

IF(CMAKE_SYSTEM_NAME MATCHES Darwin)
  SET(SO_OR_A_FILE "dylib" CACHE STRING "library extension" FORCE)
  SET(IS_OSX "yes")
  SET(SOFLAGS "${OTFL_CFLAGS}  -dynamic -dynamiclib -L${LIBRARY_OUTPUT_PATH}")
ENDIF(CMAKE_SYSTEM_NAME MATCHES Darwin)
IF(CMAKE_SYSTEM_NAME MATCHES AIX)
  SET(SO_OR_A_FILE "a" CACHE STRING "library extension" FORCE)
  SET(IS_AIX "yes")
  SET(SOFLAGS "${OTFL_CFLAGS} -shared -L${LIBRARY_OUTPUT_PATH}")
ENDIF(CMAKE_SYSTEM_NAME MATCHES AIX)
IF(WIN32 AND NOT CYGWIN)
  SET(SO_OR_A_FILE "dll" CACHE STRING "library extension" FORCE)
  SET(IS_WIN "yes")
  SET(SOFLAGS "${OTFL_CFLAGS} -shared -LIBPATH:${LIBRARY_OUTPUT_PATH}")
ENDIF(WIN32 AND NOT CYGWIN)

MARK_AS_ADVANCED(SO_OR_A_FILE)

# Add the dynamic libary creation flags to the CFLAGS (-fPIC, etc).
SET(OTFL_CFLAGS "${OTFL_CFLAGS} ${CMAKE_SHARED_LIBRARY_CXX_FLAGS}")

IF(WIN32)
  CONFIGURE_FILE (
    "${SCIRUN_CORE_SOURCE_DIR}/on-the-fly-libs/cmake_win32make.in"
    "${SCIRUN_CORE_BINARY_DIR}/on-the-fly-libs/win32make.bat"
  )
ELSE(WIN32)
  CONFIGURE_FILE (
    "${SCIRUN_CORE_SOURCE_DIR}/on-the-fly-libs/cmake_Makefile.in"
    "${SCIRUN_CORE_BINARY_DIR}/on-the-fly-libs/Makefile"
  )
ENDIF(WIN32)

# Add correct flags for win32
IF(WIN32)
  IF(NOT BUILD_CORE_LIBS_SHARED)
    ADD_DEFINITIONS(-DBUILD_SCIRUN_STATIC)
  ENDIF(NOT BUILD_CORE_LIBS_SHARED)
#  This defines M_PI for windows
  ADD_DEFINITIONS(-D_USE_MATH_DEFINES)
  ADD_DEFINITIONS(-D_WIN32_WINNT=0x0500)
  # win32 l3 warnings are a little ridiculous
  SET(CMAKE_CXX_WARNING_LEVEL 1)
  IF(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    STRING(REGEX REPLACE "/W[0-4]" "/W1"
            CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  ELSE(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W1")
  ENDIF(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
ENDIF(WIN32)

ADD_SUBDIRECTORY(Comm)
ADD_SUBDIRECTORY(Constraints)
ADD_SUBDIRECTORY(GuiInterface)
ADD_SUBDIRECTORY(Modules)
ADD_SUBDIRECTORY(Network)
ADD_SUBDIRECTORY(TkExtensions)
ADD_SUBDIRECTORY(TCLThread)
ADD_SUBDIRECTORY(Widgets)

# createTclIndex and tclIndex files
# DV Fix for each packages, create their tclIndex file
SET(CREATETCLINDEX ${SCIRUN_DATAFLOW_BINARY_DIR}/createTclIndex CACHE INTERNAL "createTclIndex script file")
SET(TCLSH ${SCIRUN_THIRDPARTY_DIR}/bin/itclsh${ITCL_VERSION})
SET(TCL_INIT_DIR ${SCIRUN_THIRDPARTY_DIR}/lib/tcl/)
SET(ITCL_INIT_DIR ${SCIRUN_THIRDPARTY_DIR}/lib/itcl/)
MESSAGE(STATUS "Creating createTclIndex using ${TCLSH}, (version ${ITCL_VERSION})")

IF(WIN32) 
  # On Windows, we would just make a batch file to do the same thing as the createTCLIndex sh script,
  # but the << operator is not accepted.  So we must create a file per tcl index
  set (CREATE_TCL_PACKAGE ${SCIRUN_CORE_SOURCE_DIR}/Dataflow/GUI)
  CONFIGURE_FILE (
    "${SCIRUN_CORE_SOURCE_DIR}/scripts/cmake_win32createTclIndex.in"
    "${SCIRUN_DATAFLOW_BINARY_DIR}/createDataflowTclIndex.tcl"
  )
  
  ADD_CUSTOM_TARGET(create_tclIndex_SCIRun ALL
    COMMAND ${TCLSH} createCoreTclIndex.tcl
    COMMAND ${TCLSH} createDataflowTclIndex.tcl
  )  
ELSE(WIN32)
  CONFIGURE_FILE (
    "${SCIRUN_CORE_SOURCE_DIR}/scripts/cmake_createTclIndex.in"
    "${CREATETCLINDEX}"
  )
  # Change permissions on createTclIndex file and generate
  # tclIndex files for Core/GUI and Dataflow/GUI
  ADD_CUSTOM_TARGET(create_tclIndex_SCIRun ALL
    chmod a+x ${CREATETCLINDEX}
    COMMAND ${CREATETCLINDEX} ${SCIRUN_CORE_SOURCE_DIR}/Dataflow/GUI
  )
ENDIF(WIN32)
# Packages enabled when building core

# Replace comma's from LoadPackage to be semi-colons to make a list
STRING(REGEX REPLACE "[,]" ";" PACKAGE_LIST ${LOAD_PACKAGE})
SET(INCLUDED_PACKAGE_INCLUDES FALSE)

FOREACH( pkg ${PACKAGE_LIST})
  # only do anything for packages other than SCIRun
  IF(NOT ${pkg} MATCHES "SCIRun" AND NOT HANDLED_PACKAGE_${pkg})
    # add SCIRun/src/Packages to the include line once
    IF(NOT ${INCLUDED_PACKAGE_INCLUDES})
      INCLUDE_DIRECTORIES(${SCIRUN_CORE_SOURCE_DIR}/Packages)
      SET(INCLUDED_PACKAGE_INCLUDES TRUE)
    ENDIF(NOT ${INCLUDED_PACKAGE_INCLUDES})

    # Add the package subdirectory in CMake
    ADD_SUBDIRECTORY(${SCIRUN_CORE_SOURCE_DIR}/Packages/${pkg} ${pkg})

    IF(WIN32) 
      # On Windows, we would just make a batch file to do the same thing as the createTCLIndex sh script,
      # but the << operator is not accepted.  So we must create a file per tcl index
      set (CREATE_TCL_PACKAGE ${SCIRUN_CORE_SOURCE_DIR}/Packages/${pkg}/Dataflow/GUI)
      CONFIGURE_FILE (
        "${SCIRUN_CORE_SOURCE_DIR}/scripts/cmake_win32createTclIndex.in"
        "${SCIRUN_DATAFLOW_BINARY_DIR}/create${pkg}TclIndex.tcl"
      )
      set (CREATE_TCL_PACKAGE ${SCIRUN_CORE_SOURCE_DIR}/Dataflow/GUI)
      
      ADD_CUSTOM_TARGET(create_tclIndex_${pkg} ALL
        COMMAND ${TCLSH} create${pkg}TclIndex.tcl
      )  
    ELSE(WIN32)

      # Create the tclIndex file
      ADD_CUSTOM_TARGET(create_tclIndex_${pkg} ALL
        chmod a+x ${CREATETCLINDEX}
        COMMAND ${CREATETCLINDEX} ${SCIRUN_CORE_SOURCE_DIR}/Packages/${pkg}/Dataflow/GUI 
      )
    ENDIF(WIN32)

    # Install the relevant source directories
    SCIRUN_INSTALL_PACKAGE(${pkg})
    
  ENDIF(NOT ${pkg} MATCHES "SCIRun" AND NOT HANDLED_PACKAGE_${pkg})
ENDFOREACH( pkg )

SET(SCIRUN_SRCS ${SCIRUN_CORE_SOURCE_DIR}/main/main.cc)

IF(WIN32)
  SET(SCIRUN_SRCS ${SCIRUN_SRCS} ${SCIRUN_CORE_SOURCE_DIR}/main/scirun.rc)
ENDIF(WIN32)

# scirun executable
ADD_EXECUTABLE(scirun ${SCIRUN_SRCS})
SCIRUN_INSTALL_EXECUTABLE(scirun)

ADD_EXECUTABLE(scirunremote
${SCIRUN_CORE_SOURCE_DIR}/main/scirunremote.cc)
SCIRun_INSTALL_EXECUTABLE(scirunremote)

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

IF(NOT WIN32)
# don't add malloc to windows version
  SET(SCIRUN_LIBS ${SCIRUN_LIBS})
ENDIF(NOT WIN32)

TARGET_LINK_LIBRARIES(scirun ${SCIRUN_LIBS}
)

TARGET_LINK_LIBRARIES(scirunremote ${SCIRUN_LIBS}
)

IF(CMAKE_SYSTEM_NAME MATCHES Darwin)
    TARGET_LINK_LIBRARIES(scirun Core_Datatypes Core_ImportExport Core_Persistent)
ENDIF(CMAKE_SYSTEM_NAME MATCHES Darwin)

# Determine if we should delete the contents of the on-the-fly-libs directory
SET(PURGE_ON_THE_FLY_DIRECTORY ON CACHE BOOL "Deletes contents of the on-the-fly directory upon configuration")
MARK_AS_ADVANCED(PURGE_ON_THE_FLY_DIRECTORY)



IF (NOT WIN32)

  # If we purge the directory, delete everything, else run the make command in there.
  # Split the rm command as it can process no more than approximately 1000 files.
  # This directory tends to have far more files.
  IF(PURGE_ON_THE_FLY_DIRECTORY)
    ADD_CUSTOM_TARGET(otfl-delete ALL
      COMMAND rm -f ALGO*.cc
      COMMAND rm -f ALGO*.d
      COMMAND rm -f ALGO*.o
      COMMAND rm -f ALGO*.so
      COMMAND rm -f ALGO*.dylib
      COMMAND rm -f ALGO*.log
      COMMAND rm -f *.cc
      COMMAND rm -f *.d
      COMMAND rm -f *.o
      COMMAND rm -f *.so
      COMMAND rm -f *.dylib
      COMMAND rm -f *.log
      WORKING_DIRECTORY ${SCIRUN_CORE_BINARY_DIR}/on-the-fly-libs 

      COMMENT "Purging files in ${SCIRUN_CORE_BINARY_DIR}/on-the-fly-libs"
      )
    # Make sure this target is run if we do 'make scirun'
    ADD_DEPENDENCIES(scirun otfl-delete)

  ELSE(PURGE_ON_THE_FLY_DIRECTORY)
    
    # Always run make in the on-the-fly-libs
    ADD_CUSTOM_TARGET(otfl-build ALL
      COMMAND ${CMAKE_BUILD_TOOL}
      WORKING_DIRECTORY ${SCIRUN_CORE_BINARY_DIR}/on-the-fly-libs
      COMMENT "Running ${CMAKE_BUILD_TOOL} in ${SCIRUN_CORE_BINARY_DIR}/on-the-fly-libs"
      )
  ENDIF(PURGE_ON_THE_FLY_DIRECTORY)

ELSE(NOT WIN32)  

  # If we purge the directory, delete everything, else run the make command in there.
  IF(PURGE_ON_THE_FLY_DIRECTORY)
    ADD_CUSTOM_TARGET(otfl-delete ALL
      COMMAND del *.cc
      COMMAND del *.dll      
      COMMAND del *.lib
      COMMAND del *.dll.manifest
      COMMAND del *.exp
      COMMAND del *.obj
      COMMAND del *.log
      WORKING_DIRECTORY ${SCIRUN_CORE_BINARY_DIR}/on-the-fly-libs 

      COMMENT "Purging files in ${SCIRUN_CORE_BINARY_DIR}/on-the-fly-libs"
      )
    # Make sure this target is run if we do 'make scirun'
    ADD_DEPENDENCIES(scirun otfl-delete)  
  ELSE(PURGE_ON_THE_FLY_DIRECTORY)

  ENDIF(PURGE_ON_THE_FLY_DIRECTORY)
  
ENDIF(NOT WIN32)

# Create powerapps
STRING (REGEX MATCH "Teem" WITH_TEEM_PACKAGE ${LOAD_PACKAGE})
STRING (REGEX MATCH "BioPSE" WITH_BIOPSE_PACKAGE ${LOAD_PACKAGE})

IF(WITH_TEEM_PACKAGE MATCHES "Teem")
  # BioTensor
  IF(NOT WIN32)
    CONFIGURE_FILE(${SCIRUN_CORE_SOURCE_DIR}/Packages/Teem/nets/PowerApps/cmake_BioTensor ${EXECUTABLE_OUTPUT_PATH}/BioTensor)
    ADD_CUSTOM_TARGET(create_BioTensor ALL
      chmod +x ${EXECUTABLE_OUTPUT_PATH}/BioTensor
      )
  ELSE(NOT WIN32)
    CONFIGURE_FILE(${SCIRUN_CORE_SOURCE_DIR}/Packages/Teem/nets/PowerApps/cmake_winBioTensor ${EXECUTABLE_OUTPUT_PATH}/${BUILD_TYPE}/BioTensor.bat)
  ENDIF(NOT WIN32)
  
  # BioImage
  IF(NOT WIN32)  
    CONFIGURE_FILE(${SCIRUN_CORE_SOURCE_DIR}/Packages/Teem/nets/PowerApps/cmake_BioImage ${EXECUTABLE_OUTPUT_PATH}/BioImage)
    ADD_CUSTOM_TARGET(create_BioImage ALL
      chmod +x ${EXECUTABLE_OUTPUT_PATH}/BioImage
      )
  ELSE(NOT WIN32)
    CONFIGURE_FILE(${SCIRUN_CORE_SOURCE_DIR}/Packages/Teem/nets/PowerApps/cmake_winBioImage ${EXECUTABLE_OUTPUT_PATH}/${BUILD_TYPE}/BioImage.bat)
  ENDIF(NOT WIN32)
ENDIF(WITH_TEEM_PACKAGE MATCHES "Teem")

IF(WITH_BIOPSE_PACKAGE MATCHES "BioPSE")
  # BioFEM
  IF(NOT WIN32)
    CONFIGURE_FILE(${SCIRUN_CORE_SOURCE_DIR}/Packages/BioPSE/nets/PowerApps/cmake_BioFEM ${EXECUTABLE_OUTPUT_PATH}/BioFEM)
    ADD_CUSTOM_TARGET(create_BioFEM ALL
      chmod +x ${EXECUTABLE_OUTPUT_PATH}/BioFEM
      )
  ELSE(NOT WIN32)
    CONFIGURE_FILE(${SCIRUN_CORE_SOURCE_DIR}/Packages/BioPSE/nets/PowerApps/cmake_winBioFEM ${EXECUTABLE_OUTPUT_PATH}/${BUILD_TYPE}/BioFEM.bat)
  ENDIF(NOT WIN32)
ENDIF(WITH_BIOPSE_PACKAGE MATCHES "BioPSE")


