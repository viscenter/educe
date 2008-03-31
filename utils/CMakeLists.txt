PROJECT(Utils)

ADD_EXECUTABLE(nrrdchunk nrrdchunk.cc)
TARGET_LINK_LIBRARIES(nrrdchunk teem)
