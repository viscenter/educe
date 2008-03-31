PROJECT(Utils)

ADD_EXECUTABLE(nrrdchunk nrrdchunk.cc)
TARGET_LINK_LIBRARIES(nrrdchunk teem png bz2 z)

ADD_EXECUTABLE(nrrdcrop nrrdcrop.cc)
TARGET_LINK_LIBRARIES(nrrdcrop teem png bz2 z)
