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
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateIsInsideFieldAlg.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace SCIRun {

CalculateIsInsideFieldAlg* get_calculate_is_inside_field_alg()
{
  return new CalculateIsInsideFieldAlg();
}

//! Algorithm Interface.
size_t
CalculateIsInsideFieldAlg::execute(size_t Field, size_t ObjectField)
{
  DataManager *dm = DataManager::get_dm();
  FieldHandle input, ofield;
  FieldHandle object;

  // Get the new input data:
  if (Field == 0) { return 0; }
  input = dm->get_field(Field);
  if (ObjectField == 0) { return false; }
  object = dm->get_field(ObjectField);

  ProgressReporter *pr = get_progress_reporter();

  SCIRunAlgo::FieldsAlgo algo(pr);
  if (! algo.CalculateIsInsideField(input, ofield, object, 
				    get_p_outputtype(),
				    get_p_outval(),
				    get_p_inval())) 
  {
    return 0;
  }
  // send new output if there is any:
  return dm->add_field(ofield);
}

} //end namespace SCIRun

