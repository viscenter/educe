//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2007 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  License for the specific language governing rights and limitations under
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#include <iostream>
#include <Core/Events/DataManager.h>
#include <Core/Algorithms/Interface/ChangeMesh/ScaleFieldMeshAndDataAlg.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>
#include <Core/Algorithms/Converter/ConverterAlgo.h>

namespace SCIRun {

ScaleFieldMeshAndDataAlg* get_scale_field_mesh_and_data_alg()
{
  return new ScaleFieldMeshAndDataAlg();
}

//! Algorithm Interface.
size_t
ScaleFieldMeshAndDataAlg::execute(size_t Field,size_t GeomScaleFactor,
				  size_t DataScaleFactor)
{
  DataManager *dm = DataManager::get_dm();
  FieldHandle input, ofield;
  MatrixHandle DataScale,GeomScale;

  if (Field == 0) { return 0; }
  input = dm->get_field(Field);
  if (!DataScaleFactor == 0) {
    DataScale = dm->get_matrix(DataScaleFactor);
  }
  if (!GeomScaleFactor == 0) {
    GeomScale = dm->get_matrix(GeomScaleFactor);
  }
  ProgressReporter *pr = get_progress_reporter();
  SCIRunAlgo::ConverterAlgo calgo(pr);
  SCIRunAlgo::FieldsAlgo    falgo(pr);

  double ddatascale = get_p_datascale();
  double dgeomscale = get_p_geomscale();
  if (DataScale.get_rep())
  {
    ddatascale = 1.0;
    calgo.MatrixToDouble(DataScale, ddatascale);
  }

  if (GeomScale.get_rep())
  {
    dgeomscale = 1.0;
    calgo.MatrixToDouble(GeomScale, dgeomscale);
  }

  if(! falgo.ScaleFieldMeshAndData(input, ofield, ddatascale, dgeomscale,
                                   get_p_usegeomcenter())) 
  {
    return false;
  }
  return dm->add_field(ofield);
}

} //end namespace SCIRun

