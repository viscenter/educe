# Get a list of the files in the directory

SET(ON_THE_FLY_DIRECTORY ${SCIRUN_CORE_BINARY_DIR}/on-the-fly-libs )
#MESSAGE("ON_THE_FLY_DIRECTORY = ${ON_THE_FLY_DIRECTORY}")
FILE(GLOB file_list "${ON_THE_FLY_DIRECTORY}/*.cc")

# MESSAGE("Found these files:\n")
# FOREACH(file ${file_list})
#   MESSAGE("${file}")
# ENDFOREACH(file)

# Now add a library with all the files:
LIST(LENGTH file_list num_files)
# MESSAGE("num_files = ${num_files}")
IF(num_files)
  ADD_LIBRARY(DynamicModules ${file_list})
  SCIRUN_INSTALL_LIBRARY(DynamicModules)
  TARGET_LINK_LIBRARIES(DynamicModules ${SCIRUN_LIBRARY_LIST})
  # We need to make the name of the library consistent across systems
  # to make loading it dynamically easier.  Also, the on-the-fly-libs
  # Makefile is setup to ignore libraries that start out with an
  # underscore.
  SET_TARGET_PROPERTIES(DynamicModules
    PROPERTIES PREFIX "_"
    )
ENDIF(num_files)
