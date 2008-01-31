
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

PROJECT(SCIRUN_TESTING)

IF(SCIRUN_CORE_BINARY_DIR)
 # We are building inside the tree
 SET (EXECUTABLE_OUTPUT_PATH ${SCIRUN_CORE_BINARY_DIR} CACHE INTERNAL
 "Single output directory for building all testing executables.")
 SET(CXX_TEST_PATH ${EXECUTABLE_OUTPUT_PATH})
ELSE(SCIRUN_CORE_BINARY_DIR)
 MESSAGE(FATAL_ERROR "Stand alone testing not currently supported.")
ENDIF(SCIRUN_CORE_BINARY_DIR)

# INITIAL VALUES FOR BUILDS

SET(CTEST_CONTINUOUS_DURATION 800)
SET(CTEST_CONTINUOUS_MINIMUM_INTERVAL 60)


# ----------------------------------------------------------------------
# Where to upload results
# ----------------------------------------------------------------------

SET(CTEST_PROJECT_NAME "SCIRUN_CORE")
SET(CTEST_DROP_METHOD "xmlrpc")
SET(CTEST_DROP_SITE "http://dashboards.sci.utah.edu:8081")
SET(CTEST_DROP_LOCATION "SCIRUN_CORE")

# ----------------------------------------------------------------------
# Different testing categories
# ----------------------------------------------------------------------

OPTION(RUN_UNIT_TESTS "Run individual module unit tests (X must be running)" ON)
OPTION(RUN_SAMPLE_TESTS "Run the regression test networks (X must be running)" ON)
OPTION(RUN_OPTIONAL_TESTS "Run the optional test networks (X must be running)" OFF)

# Directory of baseline images
SET(BASELINE_IMAGES_DIR "${SCIRUN_TESTING_BINARY_DIR}/../../testing/BaseLineImages" CACHE PATH "Path to baseline images")
SET(TEST_IMAGES_DIR "${SCIRUN_TESTING_BINARY_DIR}/../../testing/TestImages" CACHE PATH "Path to test images")
SET(DIFF_IMAGES_DIR "${SCIRUN_TESTING_BINARY_DIR}/../../testing/DiffImages" CACHE PATH "Path to difference images")

IF(EXISTS "${SCIRUN_TESTING_BINARY_DIR}/../../../SCIRunData")
  SET(SCIRUN_DATADIR "${SCIRUN_TESTING_BINARY_DIR}/../../../SCIRunData" CACHE PATH "Where to find SCIRunData" FORCE)
  SET(CTEST_CUSTOM_PRE_TEST "${SVNCOMMAND} update ${SCIRUN_TESTING_BINARY_DIR}/../../../SCIRunData" CACHE STRING "run svn update on data" FORCE)
  MESSAGE(STATUS "Found SCIRunData for regression testing")
ENDIF (EXISTS "${SCIRUN_TESTING_BINARY_DIR}/../../../SCIRunData")

# ----------------------------------------------------------------------
# Finding scirun executable
# ----------------------------------------------------------------------
IF(RUN_UNIT_TESTS OR RUN_SAMPLE_TESTS)
 # Run unit module tests or samples only if Dataflow built (which includes
 # the scirun executable)
 IF(BUILD_DATAFLOW)
   # Not determining correct path to SCIRun on Windows
   # so we need to come up with it by hand right now
   IF(WIN32)
     SET(EXE_LOC ${EXECUTABLE_OUTPUT_PATH}/${BUILD_TYPE}/scirun.exe CACHE PATH "Path to SCIRun executable")
   ELSE(WIN32)
     #GET_TARGET_PROPERTY(EXE_LOC scirun LOCATION)
     SET(EXE_LOC ${EXECUTABLE_OUTPUT_PATH}/scirun CACHE PATH "Path to SCIRun executable")
   ENDIF(WIN32)
   MESSAGE(STATUS "Using executable: ${EXE_LOC} to test networks")
 ELSE(BUILD_DATAFLOW)
   MESSAGE(FATAL_ERROR "Currently must build dataflow when running regression test networks.")
 ENDIF(BUILD_DATAFLOW)
ENDIF(RUN_UNIT_TESTS OR RUN_SAMPLE_TESTS)

MARK_AS_ADVANCED(FORCE EXE_LOC)

IF(SCIRUN_DATADIR)
  SET(DATADIR_LOCATION "${SCIRUN_DATADIR}" CACHE INTERNAL "Find default SCIRunData" FORCE)
  SET(SET_DATADIR "-d" CACHE INTERNAL "" FORCE)
ELSE(SCIRUN_DATADIR)
  SET(DATADIR_LOCATION "" CACHE INTERNAL "Find default SCIRunData" FORCE)
  SET(SET_DATADIR "" CACHE INTERNAL "" FORCE)
ENDIF(SCIRUN_DATADIR)

MARK_AS_ADVANCED(FORCE SET_DATADIR)

# ----------------------------------------------------------------------
# Module tests
# ----------------------------------------------------------------------
SET(srns_with_viewscene "" CACHE INTERNAL "srns with ViewScene module")
IF(RUN_UNIT_TESTS)
 # Run all the possible .srn files in src/nets/Modules
  FILE(GLOB_RECURSE scirun_srns "${SCIRUN_CORE_SOURCE_DIR}/nets/Modules/*.srn")
 FOREACH(srn ${scirun_srns})
   GET_FILENAME_COMPONENT(srn_name ${srn} NAME_WE)
   SET(SRN_ROOT_NAMES "${SRN_ROOT_NAMES} ${srn_name}" CACHE INTERNAL "List of root net names that correspond with image names")
   ADD_TEST(".Test.Modules.${srn_name}_srn" ${EXE_LOC} ${srn} --regression --nosplash -R ${TEST_IMAGES_DIR} ${SET_DATADIR} ${DATADIR_LOCATION} )

   FILE(READ ${srn} temp_srn)
   STRING(REGEX MATCH "ViewScene" contains_ViewScene ${temp_srn})
   IF(contains_ViewScene)
     SET(srns_with_viewscene "${srns_with_viewscene} ${srn_name}" CACHE INTERNAL "srns with ViewScene module")
   ENDIF(contains_ViewScene)
 ENDFOREACH(srn ${scirun_srns})

 # Run all the possible .srn files for BioPSE
 FILE(GLOB_RECURSE biopse_srns "${SCIRUN_CORE_SOURCE_DIR}/Packages/BioPSE/nets/Modules*.srn")
 FOREACH(srn ${biopse_srns})
  GET_FILENAME_COMPONENT(srn_name ${srn} NAME_WE)
  ADD_TEST(".Test.Modules.${srn_name}_srn" ${EXE_LOC} ${srn} --regression --nosplash -R ${TEST_IMAGES_DIR} ${SET_DATADIR} ${DATADIR_LOCATION})

   FILE(READ ${srn} temp_srn)
   STRING(REGEX MATCH "ViewScene" contains_ViewScene ${temp_srn})
   IF(contains_ViewScene)
     SET(srns_with_viewscene "${srns_with_viewscene} ${srn_name}" CACHE INTERNAL "srns with ViewScene module")
   ENDIF(contains_ViewScene)
  ENDFOREACH(srn ${biopse_srns})

# Run all the possible .srn files for Teem
 FILE(GLOB_RECURSE biopse_srns "${SCIRUN_CORE_SOURCE_DIR}/Packages/Teem/nets/Modules/*.srn")
 FOREACH(srn ${biopse_srns})
  GET_FILENAME_COMPONENT(srn_name ${srn} NAME_WE)
  ADD_TEST(".Test.Modules.${srn_name}_srn" ${EXE_LOC} ${srn} --regression --nosplash -R ${TEST_IMAGES_DIR} ${SET_DATADIR} ${DATADIR_LOCATION})

   FILE(READ ${srn} temp_srn)
   STRING(REGEX MATCH "ViewScene" contains_ViewScene ${temp_srn})
   IF(contains_ViewScene)
     SET(srns_with_viewscene "${srns_with_viewscene} ${srn_name}" CACHE INTERNAL "srns with ViewScene module")
   ENDIF(contains_ViewScene)
  ENDFOREACH(srn ${biopse_srns})
  
  
 # Run all the possible .srn files for MatlabInterface
 FILE(GLOB_RECURSE matlab_srns "${SCIRUN_CORE_SOURCE_DIR}/Packages/MatlabInterface/nets/Modules/*.srn")
 FOREACH(srn ${matlab_srns})
  GET_FILENAME_COMPONENT(srn_name ${srn} NAME_WE)
  ADD_TEST(".Test.Modules.${srn_name}_srn" ${EXE_LOC} ${srn} --regression --nosplash -R ${TEST_IMAGES_DIR} ${SET_DATADIR} ${DATADIR_LOCATION})
 
  FILE(READ ${srn} temp_srn)
  STRING(REGEX MATCH "ViewScene" contains_ViewScene ${temp_srn})
  IF(contains_ViewScene)
    SET(srns_with_viewscene "${srns_with_viewscene} ${srn_name}" CACHE INTERNAL "srns with ViewScene module")
  ENDIF(contains_ViewScene)
 ENDFOREACH(srn ${matlab_srns})

ENDIF(RUN_UNIT_TESTS)

IF(RUN_SAMPLE_TESTS)
 # Run all the possible .srn files in src/nets/Samples
 FILE(GLOB_RECURSE scirun_srns "${SCIRUN_CORE_SOURCE_DIR}/nets/Samples/*.srn")
 FOREACH(srn ${scirun_srns})
   GET_FILENAME_COMPONENT(srn_name ${srn} NAME_WE)
   SET(SRN_ROOT_NAMES "${SRN_ROOT_NAMES} ${srn_name}" CACHE INTERNAL "List of root net names that correspond with image names")
   ADD_TEST(".Test.Samples.${srn_name}_srn" ${EXE_LOC} ${srn} --regression --nosplash -R ${TEST_IMAGES_DIR} ${SET_DATADIR} ${DATADIR_LOCATION})
   FILE(READ ${srn} temp_srn)
   STRING(REGEX MATCH "ViewScene" contains_ViewScene ${temp_srn})
   IF(contains_ViewScene)
     SET(srns_with_viewscene "${srns_with_viewscene} ${srn_name}" CACHE INTERNAL "srns with ViewScene module")
   ENDIF(contains_ViewScene)
 ENDFOREACH(srn ${scirun_srns})

  # Run all the possible .srn files for BioPSE
 FILE(GLOB_RECURSE matlab_srns "${SCIRUN_CORE_SOURCE_DIR}/Packages/BioPSE/nets/Samples/*.srn")
 FOREACH(srn ${matlab_srns})
  GET_FILENAME_COMPONENT(srn_name ${srn} NAME_WE)
  ADD_TEST(".Test.Samples.${srn_name}_srn" ${EXE_LOC} ${srn} --regression --nosplash -R ${TEST_IMAGES_DIR} ${SET_DATADIR} ${DATADIR_LOCATION})
 
  FILE(READ ${srn} temp_srn)
  STRING(REGEX MATCH "ViewScene" contains_ViewScene ${temp_srn})
  IF(contains_ViewScene)
    SET(srns_with_viewscene "${srns_with_viewscene} ${srn_name}" CACHE INTERNAL "srns with ViewScene module")
  ENDIF(contains_ViewScene)
 ENDFOREACH(srn ${matlab_srns}) 

  # Run all the possible .srn files for Teem
 FILE(GLOB_RECURSE matlab_srns "${SCIRUN_CORE_SOURCE_DIR}/Packages/Teem/nets/Samples/*.srn")
 FOREACH(srn ${matlab_srns})
  GET_FILENAME_COMPONENT(srn_name ${srn} NAME_WE)
  ADD_TEST(".Test.Samples.${srn_name}_srn" ${EXE_LOC} ${srn} --regression --nosplash -R ${TEST_IMAGES_DIR} ${SET_DATADIR} ${DATADIR_LOCATION})
 
  FILE(READ ${srn} temp_srn)
  STRING(REGEX MATCH "ViewScene" contains_ViewScene ${temp_srn})
  IF(contains_ViewScene)
    SET(srns_with_viewscene "${srns_with_viewscene} ${srn_name}" CACHE INTERNAL "srns with ViewScene module")
  ENDIF(contains_ViewScene)
 ENDFOREACH(srn ${matlab_srns}) 
 
  # Run all the possible .srn files for MatlabInterface
 FILE(GLOB_RECURSE matlab_srns "${SCIRUN_CORE_SOURCE_DIR}/Packages/MatlabInterface/nets/Samples/*.srn")
 FOREACH(srn ${matlab_srns})
  GET_FILENAME_COMPONENT(srn_name ${srn} NAME_WE)
  ADD_TEST(".Test.Samples.${srn_name}_srn" ${EXE_LOC} ${srn} --regression --nosplash -R ${TEST_IMAGES_DIR} ${SET_DATADIR} ${DATADIR_LOCATION})
 
  FILE(READ ${srn} temp_srn)
  STRING(REGEX MATCH "ViewScene" contains_ViewScene ${temp_srn})
  IF(contains_ViewScene)
    SET(srns_with_viewscene "${srns_with_viewscene} ${srn_name}" CACHE INTERNAL "srns with ViewScene module")
  ENDIF(contains_ViewScene)
 ENDFOREACH(srn ${matlab_srns})
  
ENDIF(RUN_SAMPLE_TESTS)

IF(RUN_OPTIONAL_TESTS)
 # Run all the possible .srn files in src/nets/Optional
 FILE(GLOB_RECURSE scirun_srns "${SCIRUN_CORE_SOURCE_DIR}/nets/Optional/*.srn")
 FOREACH(srn ${scirun_srns})
   GET_FILENAME_COMPONENT(srn_name ${srn} NAME_WE)
   SET(SRN_ROOT_NAMES "${SRN_ROOT_NAMES} ${srn_name}" CACHE INTERNAL "List of root net names that correspond with image names")
   ADD_TEST("${srn_name}_srn" ${EXE_LOC} ${srn} --regression --nosplash -R ${TEST_IMAGES_DIR} ${SET_DATADIR} ${DATADIR_LOCATION})
   FILE(READ ${srn} temp_srn)
   STRING(REGEX MATCH "ViewScene" contains_ViewScene ${temp_srn})
   IF(contains_ViewScene)
     SET(srns_with_viewscene "${srns_with_viewscene} ${srn_name}" CACHE INTERNAL "srns with ViewScene module")
   ENDIF(contains_ViewScene)
 ENDFOREACH(srn ${scirun_srns})

  # Run all the possible .srn files for BioPSE
 FILE(GLOB_RECURSE matlab_srns "${SCIRUN_CORE_SOURCE_DIR}/Packages/BioPSE/nets/Optional/*.srn")
 FOREACH(srn ${matlab_srns})
  GET_FILENAME_COMPONENT(srn_name ${srn} NAME_WE)
  ADD_TEST("${srn_name}_srn" ${EXE_LOC} ${srn} --regression --nosplash -R ${TEST_IMAGES_DIR} ${SET_DATADIR} ${DATADIR_LOCATION})
 
  FILE(READ ${srn} temp_srn)
  STRING(REGEX MATCH "ViewScene" contains_ViewScene ${temp_srn})
  IF(contains_ViewScene)
    SET(srns_with_viewscene "${srns_with_viewscene} ${srn_name}" CACHE INTERNAL "srns with ViewScene module")
  ENDIF(contains_ViewScene)
 ENDFOREACH(srn ${matlab_srns}) 

  # Run all the possible .srn files for Teem
 FILE(GLOB_RECURSE matlab_srns "${SCIRUN_CORE_SOURCE_DIR}/Packages/Teem/nets/Optional/*.srn")
 FOREACH(srn ${matlab_srns})
  GET_FILENAME_COMPONENT(srn_name ${srn} NAME_WE)
  ADD_TEST("${srn_name}_srn" ${EXE_LOC} ${srn} --regression --nosplash -R ${TEST_IMAGES_DIR} ${SET_DATADIR} ${DATADIR_LOCATION})
 
  FILE(READ ${srn} temp_srn)
  STRING(REGEX MATCH "ViewScene" contains_ViewScene ${temp_srn})
  IF(contains_ViewScene)
    SET(srns_with_viewscene "${srns_with_viewscene} ${srn_name}" CACHE INTERNAL "srns with ViewScene module")
  ENDIF(contains_ViewScene)
 ENDFOREACH(srn ${matlab_srns}) 
 
  # Run all the possible .srn files for MatlabInterface
 FILE(GLOB_RECURSE matlab_srns "${SCIRUN_CORE_SOURCE_DIR}/Packages/MatlabInterface/nets/Optional/*.srn")
 FOREACH(srn ${matlab_srns})
  GET_FILENAME_COMPONENT(srn_name ${srn} NAME_WE)
  ADD_TEST("${srn_name}_srn" ${EXE_LOC} ${srn} --regression --nosplash -R ${TEST_IMAGES_DIR} ${SET_DATADIR} ${DATADIR_LOCATION})
 
  FILE(READ ${srn} temp_srn)
  STRING(REGEX MATCH "ViewScene" contains_ViewScene ${temp_srn})
  IF(contains_ViewScene)
    SET(srns_with_viewscene "${srns_with_viewscene} ${srn_name}" CACHE INTERNAL "srns with ViewScene module")
  ENDIF(contains_ViewScene)
 ENDFOREACH(srn ${matlab_srns})
  
ENDIF(RUN_OPTIONAL_TESTS)


# Always build the Teem based image compare program
ADD_EXECUTABLE(testRegressionImages TestRegressionImagesTeem.cc)
TARGET_LINK_LIBRARIES(testRegressionImages ${TEEM_LIBRARY})

FILE(MAKE_DIRECTORY "${TEST_IMAGES_DIR}")
FILE(MAKE_DIRECTORY "${BASELINE_IMAGES_DIR}")
FILE(MAKE_DIRECTORY "${DIFF_IMAGES_DIR}")

SEPARATE_ARGUMENTS(srns_with_viewscene)
FOREACH(snapshot ${srns_with_viewscene})
  # Try to run the test if there was a ViewScene module, regardless
  # of if the files exists or not because we want this test to fail
  # if we don't have the proper files so we can make sure to have a
  # gold-standard image/ or test image for future runs
  ADD_TEST(".Test.Image.${snapshot}_diff" ${EXECUTABLE_OUTPUT_PATH}/testRegressionImages "${TEST_IMAGES_DIR}/${snapshot}-window0.png" "${BASELINE_IMAGES_DIR}/${snapshot}-window0.png" "${DIFF_IMAGES_DIR}/${snapshot}_diff.png")
ENDFOREACH(snapshot ${srns_with_viewscene})

MARK_AS_ADVANCED(CLEAR RUN_UNIT_TESTS RUN_SAMPLE_TESTS BASELINE_IMAGES_DIR TEST_IMAGES_DIR DIFF_IMAGES_DIR)




