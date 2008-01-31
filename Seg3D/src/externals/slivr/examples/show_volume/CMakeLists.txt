PROJECT(SLIVR_EXAMPLES)

# Force static builds
SET(BUILD_SHARED_LIBS OFF)

# Set which components of wx we need, as there can be different parts
# compiled.

SET(wxWidgets_INCLUDE_DIRS_NO_SYSTEM 1)
SET(wxWidgets_USE_LIBS core base gl)
FIND_PACKAGE(wxWidgets)

IF(NOT wxWidgets_FOUND)
  MESSAGE(FATAL_ERROR "Required wxWidget libraries not found")
ENDIF(NOT wxWidgets_FOUND)
INCLUDE(${wxWidgets_USE_FILE})


IF(APPLE)
  ADD_DEFINITIONS(-DEX_DATA_PATH="\\"./wxgladetest.app/data/\\"")
  INSTALL(FILES tooth.nhdr tooth.raw DESTINATION bin/examples/wxgladetest.app/data)
ELSE(APPLE)
  ADD_DEFINITIONS(-DEX_DATA_PATH="\\"../data/\\"")
  INSTALL(FILES tooth.nhdr tooth.raw DESTINATION bin/data)
ENDIF(APPLE)



# and for each of your dependant executable/library targets:
# MACOSX_BUNDLE : add this to make a MAC bundle.
ADD_EXECUTABLE(wxgladetest WIN32 MACOSX_BUNDLE
               main.cpp MainWindow.cpp OGLCanvas.cc)

TARGET_LINK_LIBRARIES(wxgladetest slivrV slivrG 
		      ${wxWidgets_LIBRARIES} ${TEEM_LIBRARIES} libGLEW.a)
INCLUDE_DIRECTORIES(.)
INSTALL(TARGETS wxgladetest RUNTIME DESTINATION bin/examples)

