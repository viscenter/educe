#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2004 Scientific Computing and Imaging Institute,
#  University of Utah.
# 
#  
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
# 
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
# 
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#
PROJECT(SCIRUN_STANDALONE_CONVERT)

SET (EXECUTABLE_OUTPUT_PATH ${SCIRUN_CORE_BINARY_DIR}/convert CACHE INTERNAL "Single output directory for building all executables.")
MARK_AS_ADVANCED(EXECUTABLE_OUTPUT_PATH)

SET(StandAlone_Convert_Libs
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

SET(ConvertApps 
  CurveFieldToText
  HexVolFieldToText   
  HexVolFieldToExotxt
  HexVolToVtk
  PointCloudFieldToText
  PointCloudFieldToText
  StructCurveFieldToText
  StructHexVolFieldToText
  StructQuadSurfFieldToText
  TetVolFieldToText
  TriSurfFieldToText
  TriSurfFieldToExotxt
  ColumnMatrixToText
  DenseMatrixToText
  SparseRowMatrixToText
  ColorMapToText
  TextToCurveField
  TextToHexVolField
  TextToHexTricubicHmt
  TextToPointCloudField
  TextToPointCloudString
  TextToQuadSurfField
  TextToStructCurveField
  TextToStructHexVolField
  TextToStructQuadSurfField
  TextToTetVolField
  TextToTriSurfField
  MtoTriSurfField
  VTKtoTriSurfField
  VTKtoHexVolField
  TextToColumnMatrix
  TextToDenseMatrix
  TextToColorMap
  TriSurfToVtk
  TriSurfToOBJ
  TriSurfToTetgen
  TriSurfToM
  TetgenToTetVol
  vff2nrrd
  PictToNrrd
  UCSDToHexVol
  UCSD_HO_edge_sfToHexVol
  VistaToNrrd
  gmvToPts
  geoToPts
  SampleHexTricubicHmt
  Lin2QuadMesh
)

FOREACH( convert ${ConvertApps})
  ADD_EXECUTABLE(${convert} ${SCIRUN_CORE_SOURCE_DIR}/StandAlone/convert/${convert}.cc)
  TARGET_LINK_LIBRARIES(${convert} ${StandAlone_Convert_Libs})
  IF(WIN32)
    # copy from <dir>/convert/debug-or-release to <dir>/convert
    ADD_CUSTOM_COMMAND(TARGET ${convert} POST_BUILD COMMAND ${CMAKE_COMMAND}
                      ARGS  -E copy ${EXECUTABLE_OUTPUT_PATH}/${BUILD_TYPE}/${convert}.exe
                      ${EXECUTABLE_OUTPUT_PATH}/${convert}.exe)

  ENDIF(WIN32)
  INSTALL(TARGETS ${convert}
    RUNTIME DESTINATION bin/convert
    )
ENDFOREACH( convert)


