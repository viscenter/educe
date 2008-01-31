PROJECT(SLIVR_DISTRIBUTED)

# Force static builds
SET(BUILD_SHARED_LIBS OFF)

# and for each of your dependant executable/library targets:
# MACOSX_BUNDLE : add this to make a MAC bundle.
ADD_EXECUTABLE(voldist WIN32 MACOSX_BUNDLE
  OGLContext.cc
  main.cc)

TARGET_LINK_LIBRARIES(voldist slivrV slivrG 
  ${TEEM_LIBRARIES} libGLEW.a X11 GL GLU)

INCLUDE_DIRECTORIES(.)
INSTALL(TARGETS voldist RUNTIME DESTINATION bin/distributed)

