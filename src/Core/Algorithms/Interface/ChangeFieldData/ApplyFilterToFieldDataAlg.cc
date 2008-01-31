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
#include <Core/Algorithms/Interface/ChangeFieldData/ApplyFilterToFieldDataAlg.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace SCIRun {

ApplyFilterToFieldDataAlg* get_apply_filter_to_field_data_alg()
{
  return new ApplyFilterToFieldDataAlg();
}

//! Algorithm Interface.
size_t
ApplyFilterToFieldDataAlg::execute(size_t fld_id)
{
  DataManager *dm = DataManager::get_dm();

  string method = get_p_method();
  string ed_method = get_p_ed_method();
  int ed_iterations = get_p_ed_iterations();

  FieldHandle input, ofield;
  ProgressReporter *pr = get_progress_reporter();

  if (fld_id == 0) {
    return 0;
  }
  input = dm->get_field(fld_id);
  SCIRunAlgo::FieldsAlgo algo(pr);

  if (method == "erode/dilate")
  {
    SCIRunAlgo::FieldsAlgo::FilterSettings settings;
    settings.num_iterations_ = ed_iterations;
    if (ed_method == "erode")
    {
      if(! algo.ApplyErodeFilterToFieldData(input, ofield, settings))
	return 0;
    }
    else  if (ed_method == "dilate")
    {
      if(! algo.ApplyDilateFilterToFieldData(input, ofield, settings))
	return 0;
    }
    else
    {
      if(! algo.ApplyErodeFilterToFieldData(input, ofield, settings))
	return 0;
      if(! algo.ApplyDilateFilterToFieldData(input, ofield, settings))
	return 0;
    }
  }
  return dm->add_field(ofield);
}

} //end namespace SCIRun

