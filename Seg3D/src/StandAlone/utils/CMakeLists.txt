SET (EXECUTABLE_OUTPUT_PATH ${SCIRUN_CORE_BINARY_DIR}/utils CACHE INTERNAL "Single output directory for building all executables.")
MARK_AS_ADVANCED(EXECUTABLE_OUTPUT_PATH)

SET(StandAlone_Utils_Libs
  Core_Exceptions
  Core_Datatypes
  Core_Util
  Core_Containers
  Core_Persistent
  Core_Init
  Core_Thread
  Core_Geometry
  Core_Math
  Core_ImportExport
  Core_Basis
  Core_Geom
  ${TEEM_LIBRARY}
  ${FREETYPE_LIBRARY}
  ${XML2_LIBRARY}
  ${M_LIBRARY}
  ${F_LIBRARY}
  ${PNG_LIBRARY}
  ${ZLIB_LIBRARY}
 )

# Helper macro to install utilty executables
MACRO(SCIRUN_INSTALL_UTIL executable)
  INSTALL(TARGETS ${executable}
    RUNTIME DESTINATION bin/util
    )
ENDMACRO(SCIRUN_INSTALL_UTIL)

# Helper function to add the executable and the intall at the same time
MACRO(SCIRUN_ADD_UTILITY_EXECUTABLE executable)
  ADD_EXECUTABLE(${executable} ${ARGN})
  SCIRUN_INSTALL_UTIL(${executable})
ENDMACRO(SCIRUN_ADD_UTILITY_EXECUTABLE)

SCIRUN_ADD_UTILITY_EXECUTABLE(AddTri ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/AddTri.cc)
TARGET_LINK_LIBRARIES(AddTri ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(FieldBin1Test ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/FieldBin1Test.cc)
TARGET_LINK_LIBRARIES(FieldBin1Test ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(FieldTextToBin ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/FieldTextToBin.cc)
TARGET_LINK_LIBRARIES(FieldTextToBin ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(GenerateMPMData ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/GenerateMPMData.cc)
TARGET_LINK_LIBRARIES(GenerateMPMData ${StandAlone_Utils_Libs})


SCIRUN_ADD_UTILITY_EXECUTABLE(OrientFaces ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/OrientFaces.cc)
TARGET_LINK_LIBRARIES(OrientFaces ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(PCA-example ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/PCA-example.cc)
TARGET_LINK_LIBRARIES(PCA-example ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(RemoveConnectedFaces ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/RemoveConnectedFaces.cc)
TARGET_LINK_LIBRARIES(RemoveConnectedFaces ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(RemoveFaces ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/RemoveFaces.cc)
TARGET_LINK_LIBRARIES(RemoveFaces ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(RemoveOrphanNodes ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/RemoveOrphanNodes.cc)
TARGET_LINK_LIBRARIES(RemoveOrphanNodes ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(SwapFaces ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/SwapFaces.cc)
TARGET_LINK_LIBRARIES(SwapFaces ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(TestBasis ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/TestBasis.cc)
TARGET_LINK_LIBRARIES(TestBasis ${StandAlone_Utils_Libs})

SCIRUN_ADD_UTILITY_EXECUTABLE(UnitElementMesh ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/utils/UnitElementMesh.cc)
TARGET_LINK_LIBRARIES(UnitElementMesh ${StandAlone_Utils_Libs})
