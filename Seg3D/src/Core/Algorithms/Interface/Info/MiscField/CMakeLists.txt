# 
# For more information, please see: http://software.sci.utah.edu
# 
# The MIT License
# 
# Copyright (c) 2007 Scientific Computing and Imaging Institute,
# University of Utah.
# 
# License for the specific language governing rights and limitations under
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
# 
#   File   : CMakeLists.txt
#   Author : Martin Cole
#   Date   : Fri Oct 26 11:39:00 2007

SET(Algo_SRCS
  BuildMappingMatrix
  BuildMatrixOfSurfaceNormals
  BuildPointCloudToLatVolMappingMatrix
  ChooseField
  CollectPointClouds
  CoregisterPointClouds
  ManageFieldSeries
  ReportFieldGeometryMeasures
  ReportFieldInfo
  ReportScalarFieldStats
  ReportSearchGridInfo
  SelectFieldROIWithBoxWidget
  SetFieldOrMeshStringProperty
  SetFieldProperty
)

SET(cat_dir MiscField)
SET(atarg ai_${cat_dir})
SET(htemp_dir ${CMAKE_SOURCE_DIR}/scripts)
SET(alg_top Core/Algorithms/Interface)
SET(xml2h ${htemp_dir}/write_C_interface.py)
SET(dest_arg ${CMAKE_BINARY_DIR}/include/${alg_top})

ADD_CUSTOM_TARGET(${atarg} ALL)

SET(results)
FOREACH(file ${Algo_SRCS})
  SET(src ${file}Info.xml)
  SET(dst ${dest_arg}/${cat_dir}/${file}Alg.h)

  ADD_CUSTOM_COMMAND(
    SOURCE ${src}
    TARGET ${atarg}
    COMMAND ${PYTHON_EXECUTABLE}
    ARGS ${xml2h} ${CMAKE_CURRENT_SOURCE_DIR}/${src} ${dest_arg} ${htemp_dir}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src} ${xml2h}
    COMMENT "Building ${dst} from ${src}"
    OUTPUTS ${dst}
    )
  SET(results ${results} ${dst})

ENDFOREACH(file)

ADD_CUSTOM_COMMAND(
  SOURCE    ${atarg}
  TARGET    ${atarg}
  DEPENDS   ${results} 
  )