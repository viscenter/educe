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
#include <Core/Algorithms/Interface/ChangeFieldData/SwapFieldDataWithMatrixEntriesAlg.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace SCIRun {

SwapFieldDataWithMatrixEntriesAlg* get_swap_field_data_with_matrix_entries_alg()
{
  return new SwapFieldDataWithMatrixEntriesAlg();
}

//! Algorithm Interface.
vector<size_t>
SwapFieldDataWithMatrixEntriesAlg::execute(size_t Input_Field, 
					   size_t Input_Matrix)
{
  vector<size_t> rval(2, 0);
  DataManager *dm = DataManager::get_dm();
  FieldHandle Input, Output;
  MatrixHandle IData, OData;

  if (Input_Field == 0) { 
    return rval; 
  }
  Input = dm->get_field(Input_Field);

  if (!Input_Matrix == 0) {
    IData = dm->get_matrix(Input_Matrix);
  }

  SCIRunAlgo::FieldsAlgo algo(get_progress_reporter());

  if (! algo.GetFieldData(Input, OData)) { return rval; }
  if (IData.get_rep()) {
    if(! algo.SetFieldData(Input, Output, IData, 
			   get_p_preserve_scalar_type())) 
    {
      return rval;
    }
  } else {
    rval[0] = Input_Field;
    rval[1] = dm->add_matrix(OData);
    return rval;
  }

  rval[0] = dm->add_field(Output); 
  rval[1] = dm->add_matrix(OData);
  return rval;
}

} //end namespace SCIRun

