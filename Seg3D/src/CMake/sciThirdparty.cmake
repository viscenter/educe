#
# SEARCH_FOR_SYSTEM_3P( SCIRun_Version
#                       Compiler
#                       Compiler_Version
#                       Bits )
#
# Search for the SCIRun Thirdparty on the SCI file system.
#

MACRO(SEARCH_FOR_SYSTEM_3P version compiler compiler_version bits)

    MESSAGE( "search_for_system_3p: ${CMAKE_SYSTEM_NAME}" )

    IF(NOT WIN32) # There is not a system wide Thirdparty build for Windows.

        # Determine system
        IF( CMAKE_SYSTEM_NAME STREQUAL "IRIX" )
            SET(SCI_ARCH "IRIX64")
        ELSEIF( CMAKE_SYSTEM_NAME STREQUAL "Darwin" )
            EXECUTE_PROCESS( COMMAND uname -p OUTPUT_VARIABLE processor )
            # Uname returns a \n at the end, so we have to cut it off... :(
            STRING( LENGTH ${processor} proc_str_len ) 
            MATH( EXPR proc_str_len "${proc_str_len} - 1" )
            STRING( SUBSTRING ${processor} 0 ${proc_str_len} processor ) 
            SET(SCI_ARCH "Darwin-${processor}")
        ELSE( CMAKE_SYSTEM_NAME STREQUAL "IRIX" )
            SET(SCI_ARCH ${CMAKE_SYSTEM_NAME})
        ENDIF( CMAKE_SYSTEM_NAME STREQUAL "IRIX") 

        # First check if a thirdparty path exists on the sci system
        # based on the version, system architecture, compiler version, 
        # and number of bits
        SET( SCI_3P_DIR_ATTEMPT "/usr/sci/projects/SCIRun/Thirdparty/${version}/${SCI_ARCH}/${compiler}-${compiler_version}-${bits}bit")

        MESSAGE( "SCI_3P_DIR_ATTEMPT is ${SCI_3P_DIR_ATTEMPT}" )

        IF( EXISTS ${SCI_3P_DIR_ATTEMPT}/SCIRUN_THIRDPARTY_VERSION )

            SET( SCIRUN_THIRDPARTY_DIR ${SCI_3P_DIR_ATTEMPT} CACHE PATH "Path to SCIRun's Thirdparty" FORCE )

        ELSE( EXISTS ${SCI_3P_DIR_ATTEMPT}/SCIRUN_THIRDPARTY_VERSION )

            SET( SCIRUN_THIRDPARTY_DIR NOTFOUND CACHE PATH "Path to SCIRun's Thirdparty" FORCE )

        ENDIF( EXISTS ${SCI_3P_DIR_ATTEMPT}/SCIRUN_THIRDPARTY_VERSION )

    ENDIF(NOT WIN32)

ENDMACRO(SEARCH_FOR_SYSTEM_3P)
