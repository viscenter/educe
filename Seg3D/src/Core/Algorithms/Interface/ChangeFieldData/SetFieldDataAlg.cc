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
#include <Core/Algorithms/Interface/ChangeFieldData/SetFieldDataAlg.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace SCIRun {

SetFieldDataAlg* get_set_field_data_alg()
{
  return new SetFieldDataAlg();
}

//! Algorithm Interface.
size_t
SetFieldDataAlg::execute(size_t Field, size_t Matrix_Data, size_t Nrrd_Data) 
{
  DataManager *dm = DataManager::get_dm();
  FieldHandle Input, Output;

  MatrixHandle matrixHandle;
  NrrdDataHandle nrrdHandle;

  if (Field == 0) { return 0; }
  Input = dm->get_field(Field);
  if (!Matrix_Data == 0) {
    matrixHandle = dm->get_matrix(Matrix_Data);
  }
  if (!Nrrd_Data == 0) {
    nrrdHandle = dm->get_nrrd(Nrrd_Data);
  }
  SCIRunAlgo::FieldsAlgo algo(get_progress_reporter());

  if(matrixHandle.get_rep()) {
    if(! algo.SetFieldData(Input, Output, matrixHandle, 
			   get_p_keepscalartype())) 
    {
      return 0;
    }
  } else if(nrrdHandle.get_rep()) {
    if(! algo.SetFieldData(Input, Output, nrrdHandle,
			   get_p_keepscalartype())) 
    {
      return 0;
    }
  } else {
    std::cerr << "No Matrix or Nrrd input" << std::endl;
    return 0;
  }
  return dm->add_field(Output);
}

} //end namespace SCIRun

