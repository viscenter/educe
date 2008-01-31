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
#include <Core/Algorithms/Interface/NewField/GetDomainBoundaryAlg.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>
#include <Core/Algorithms/Converter/ConverterAlgo.h>

namespace SCIRun {

GetDomainBoundaryAlg* get_get_domain_boundary_alg()
{
  return new GetDomainBoundaryAlg();
}

//! Algorithm Interface.
size_t
GetDomainBoundaryAlg::execute(size_t fld_id, size_t min_mat_id, 
			      size_t max_mat_id)
{
  DataManager *dm = DataManager::get_dm();
  FieldHandle field = dm->get_field(fld_id);
  MatrixHandle ElemLink;
  MatrixHandle min;
  MatrixHandle max;
  if (min_mat_id) {
    min = dm->get_matrix(min_mat_id);
  }
  if (max_mat_id) {
    max = dm->get_matrix(max_mat_id);
  }
  FieldHandle ofield;
  ProgressReporter *pr = get_progress_reporter();

  if (field->is_property("ElemLink")) {
    field->get_property("ElemLink",ElemLink);
  }

  SCIRunAlgo::ConverterAlgo calgo(pr);
  bool userange = get_p_userange();
  double minrange = get_p_minrange();
  double maxrange = get_p_maxrange();

  if (min.get_rep()) {
    calgo.MatrixToDouble(min, minrange);
  }
  if (max.get_rep()) {
    calgo.MatrixToDouble(max, maxrange);
  }

  // In case we have a one value range, use the range code but for one value:
  if (get_p_usevalue()) { 
    userange = true; 
    minrange = get_p_value(); 
    maxrange = get_p_value(); 
  }

  // The innerworks of the module:
  SCIRunAlgo::FieldsAlgo algo(pr);
  if (! algo.GetDomainBoundary(field, ofield, ElemLink, 
			       minrange, maxrange, userange, 
			       get_p_includeouterboundary(),
			       get_p_innerboundaryonly(), 
			       get_p_noinnerboundary(),
			       get_p_disconnect()))
  {
    return 0;
  }
  // send new output if there is any:
  return dm->add_field(ofield);
}

} //end namespace SCIRun

