SET (LIBRARY_OUTPUT_PATH ${DATA_PATH}/plugins CACHE INTERNAL "Single output directory for building all libraries.")

FIND_PACKAGE(OpenCV REQUIRED)
INCLUDE_DIRECTORIES( ${OPENCV_INCLUDE_DIR} )

MACRO(CREATE_PLUGIN NAME
		SOURCES LIBRARIES)
	ADD_LIBRARY(${NAME} SHARED ${SOURCES})
	IF(APPLE)
		SET_TARGET_PROPERTIES(${NAME} PROPERTIES LINK_FLAGS "-flat_namespace -undefined suppress")
	ENDIF(APPLE)
	TARGET_LINK_LIBRARIES(${NAME} ${LIBRARIES})
ENDMACRO(CREATE_PLUGIN)

SET(PLUGIN_SOURCES
	ExampleFilterPlugin
	ExamplePlugin
	PushSegmentationDown
	ScrollFilterPlugin
	ScrollUnwrapPlugin
	ScrollUnwrapMultiPlugin
	UnwrapSection
	CombineMaskXOR
)

FOREACH(source ${PLUGIN_SOURCES})
	CREATE_PLUGIN(${source} ${source}.cc ${OPENCV_LIBRARIES})
ENDFOREACH(source)
