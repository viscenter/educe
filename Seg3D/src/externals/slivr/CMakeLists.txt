IF(NOT SCIRUN_SLIVR_BUILD)
  # -- SCIRun Volume Rendering library. --
  PROJECT(SLIVR)

  # -- Version --
  SET(SLIVR_VERSION_MAJOR "0")
  SET(SLIVR_VERSION_MINOR "1")
  SET(SLIVR_VERSION_PATCH "0")

  # -- shared or static lib --
  OPTION(BUILD_SHARED_LIBS "Build shared libraries" ON)

  # -- VERBOSE_MAKEFILE --
  OPTION(VERBOSE_MAKEFILE "Visible compile output?" ON)

  IF(VERBOSE_MAKEFILE)
    SET(CMAKE_VERBOSE_MAKEFILE ON)
  ENDIF(VERBOSE_MAKEFILE)
  
  # -- CMAKE_BUILD_TYPE --
  IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
  ENDIF(NOT CMAKE_BUILD_TYPE)
  
  
  
  # -- OpenGL --
  FIND_PACKAGE(OpenGL)
  IF(OPENGL_LIBRARIES AND OPENGL_INCLUDE_DIR)
    SET(GL_LIBRARY ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} CACHE INTERNAL "OpenGL Libs")
    SET(GL_INCLUDE ${OPENGL_INCLUDE_DIR})
  ENDIF(OPENGL_LIBRARIES AND OPENGL_INCLUDE_DIR)
  
  # if we are a debug build then define our debug opengl code.
  if(CMAKE_BUILD_TYPE STREQUAL Debug)
    ADD_DEFINITIONS(-DOGL_DBG)
  endif(CMAKE_BUILD_TYPE STREQUAL Debug)
  
  
  # -- GLEW --	       
  
  FIND_LIBRARY(GLEW_LIBRARY GLEW DEFAULT_PATH)
  FIND_PATH(GLEW_INCLUDE  GL/glew.h  DEFAULT_PATH)
  
  IF(GLEW_LIBRARY AND GLEW_INCLUDE)
     SET(HAVE_GLEW TRUE)
  ENDIF(GLEW_LIBRARY AND GLEW_INCLUDE)

  IF(NOT HAVE_GLEW)
    MESSAGE(FATAL_ERROR "GLEW not found")
  ENDIF(NOT HAVE_GLEW)
  
  # -- Teem --
  FIND_PACKAGE(TEEM REQUIRED)
  
  INCLUDE_DIRECTORIES(
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE}	
    ${TEEM_INCLUDE_DIRS}
    ${SLIVR_SOURCE_DIR}/..
    )
ENDIF(NOT SCIRUN_SLIVR_BUILD)

# -- Source files the lib depends upon. --
# none of the following depend on GLEW
SET(SLIVR_GEOMETRY_SRCS
  BBox.cc
  Plane.cc
  Point.cc
  Ray.cc
  Transform.cc
  Vector.cc
  )

# -- Source files the lib depends upon. --
# the following may depend on GLEW
SET(SLIVR_VOLUME_SRCS
  Color.cc
  ColorMap.cc
  ColorMap2.cc
  CM2Shader.cc
  CM2Widget.cc
  ShaderProgramARB.cc
  SliceRenderer.cc
  Texture.cc
  TextureBrick.cc
  TextureRenderer.cc
  VideoCardInfo.c
  VolShader.cc
  VolumeRenderer.cc
  )

IF(NOT SCIRUN_SLIVR_BUILD)
  # -- Source files the lib depends upon. --
  SET(SLIVR_SRCS ${SLIVR_GEOMETRY_SRCS} ${SLIVR_VOLUME_SRCS} )
  
  SET(SLIVR_INSTALL_HEADER_FILES
    BBox.h
    CM2Shader.h
    Color.h
    ColorMap.h
    ColorMap2.h
    CM2Shader.h
    CM2Widget.h
    GLinfo.h
    Plane.h
    Point.h
    Ray.h
    ShaderProgramARB.h
    share.h
    SliceRenderer.h
    Texture.h
    TextureBrick.h
    TextureRenderer.h
    Transform.h
    Utils.h
    Vector.h
    VideoCardInfo.h
    VolShader.h
    VolumeRenderer.h
    )
  
  
  SET(LIBRARY_OUTPUT_PATH ${SLIVR_BINARY_DIR}/lib 
    CACHE INTERNAL "Single output directory for building all libraries.")
  
  SET (EXECUTABLE_OUTPUT_PATH ${SLIVR_BINARY_DIR}/bin 
    CACHE INTERNAL "Single output directory for building all executables.")
  
  LINK_DIRECTORIES(${GLEW_LIBRARY} ${TEEM_LIBRARY_DIRS})

  # -- Specify the library target slivr. -- 
  # -- specify separately so we can optionallyturn off slivrV in SCIRun 
  add_library(slivrG ${SLIVR_GEOMETRY_SRCS})
  add_library(slivrV ${SLIVR_VOLUME_SRCS})

  TARGET_LINK_LIBRARIES(slivrV ${GLEW_LIBRARY})
  
  INSTALL(FILES ${SLIVR_INSTALL_HEADER_FILES} DESTINATION include/slivr
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
  
  INSTALL(TARGETS slivrV slivrG LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
  
  INSTALL(FILES ${SLIVR_BINARY_DIR}/SLIVRConfig.cmake DESTINATION lib/slivr)

  # build the test programs
  #ADD_SUBDIRECTORY(tests)

  # optionally build the example programs
  OPTION(BUILD_EXAMPLES "Build example programs" OFF)	
  IF(BUILD_EXAMPLES)
    ADD_SUBDIRECTORY(examples/show_volume)
  ENDIF(BUILD_EXAMPLES)	   


  OPTION(BUILD_DISTRIBUTED "Build the distributed test app (X only)" OFF)
  IF(BUILD_DISTRIBUTED)
    ADD_SUBDIRECTORY(distributed)
  ENDIF(BUILD_DISTRIBUTED)	 


  SET(SLIVR_LIBRARY_DIR ${CMAKE_INSTALL_PREFIX}/lib)
  SET(SLIVR_LIBRARY slivr)
  SET(SLIVR_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include)
  
  CONFIGURE_FILE("${SLIVR_SOURCE_DIR}/SLIVRConfig.cmake.in"
    "${SLIVR_BINARY_DIR}/SLIVRConfig.cmake" @ONLY IMMEDIATE)
  
ELSE(NOT SCIRUN_SLIVR_BUILD)

  # -- Specify the library target slivr. --
  SCIRUN_ADD_LIBRARY(slivrG ${SLIVR_GEOMETRY_SRCS})
  
  IF(NOT MINIMAL_CORE)
    SCIRUN_ADD_LIBRARY(slivrV ${SLIVR_VOLUME_SRCS})
    TARGET_LINK_LIBRARIES(slivrV slivrG 
                          ${SCI_GLEW_LIBRARY} 
                          ${GL_LIBRARY} 
                          ${TEEM_LIBRARY})
    IF(APPLE_LEOPARD)
      SET_TARGET_PROPERTIES(slivrV slivrG PROPERTIES LINK_FLAGS "${GL_LINK}")
    ENDIF(APPLE_LEOPARD)

  ENDIF(NOT MINIMAL_CORE)
ENDIF(NOT SCIRUN_SLIVR_BUILD)

