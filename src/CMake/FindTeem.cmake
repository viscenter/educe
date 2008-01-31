#
# Find the Teem library
#

#
# This module finds the Teem include files and libraries. 
# It also determines what the name of the library is. 
# This code sets the following variables:
#
# TEEM_LIBRARY the libraries to link against when using Teem
# TEEM_INCLUDE where to find teem/nrrd.h, etc.
#

# DV Fix finding teem library in thirdparty
FIND_LIBRARY(TEEM_LIBRARY teem
             PATHS ${SCIRUN_THIRDPARTY_DIR}/lib
)

# Teem should have been configured with zlib and png support.
IF(TEEM_LIBRARY)
  SET(TEEM_LIBRARY ${TEEM_LIBRARY} ${PNG_LIBRARY} ${ZLIB_LIBRARY})
ENDIF(TEEM_LIBRARY)

FIND_PATH(TEEM_INCLUDE_DIR teem/nrrd.h
          ${SCIRUN_THIRDPARTY_DIR}/include
)

