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
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateSignedDistanceToFieldAlg.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace SCIRun {

CalculateSignedDistanceToFieldAlg* get_calculate_signed_distance_to_field_alg()
{
  return new CalculateSignedDistanceToFieldAlg();
}

//! Algorithm Interface.
size_t
CalculateSignedDistanceToFieldAlg::execute(size_t Field,size_t ObjectField)
{
  DataManager *dm = DataManager::get_dm();
  // define data handles:
  FieldHandle input, ofield;
  FieldHandle object;

  if (Field == 0) { return 0; }
  input = dm->get_field(Field);
  if (ObjectField == 0) { return 0; }
  object = dm->get_field(ObjectField);

  ProgressReporter *pr = get_progress_reporter();

  // Entry point to core of SCIRun:
  SCIRunAlgo::FieldsAlgo algo(pr);

  // Actual algorithm:
  if(! algo.SignedDistanceField(input, ofield, object)) { return 0; }

  // Send data downstream:
  return dm->add_field(ofield);
}

} //end namespace SCIRun

