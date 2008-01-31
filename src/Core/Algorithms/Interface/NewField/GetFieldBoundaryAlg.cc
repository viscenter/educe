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
#include <Core/Algorithms/Interface/NewField/GetFieldBoundaryAlg.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace SCIRun {

GetFieldBoundaryAlg* get_get_field_boundary_alg()
{
  return new GetFieldBoundaryAlg();
}

//! Algorithm Interface.
vector<size_t>
GetFieldBoundaryAlg::execute(size_t fld_id)
{
  vector<size_t> rval(2, 0);
  DataManager *dm = DataManager::get_dm();
  FieldHandle field = dm->get_field(fld_id);

  FieldHandle ofield;
  MatrixHandle mapping;

  ProgressReporter *pr = get_progress_reporter();

  // Entry point to algorithm
  SCIRunAlgo::FieldsAlgo algo(pr);
  if (!(algo.GetFieldBoundary(field, ofield, mapping))) {
    return rval;
  }

  // Send Data flow objects downstream
  rval[0] = dm->add_field(ofield);
  rval[1] = dm->add_matrix(mapping);
  return rval;
}

} //end namespace SCIRun

