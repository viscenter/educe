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
#include <Core/Algorithms/Interface/Converters/ConvertNrrdToFieldAlg.h>
#include <Core/Algorithms/Converter/ConverterAlgo.h>

namespace SCIRun {

ConvertNrrdToFieldAlg* get_convert_nrrd_to_field_alg()
{
  return new ConvertNrrdToFieldAlg();
}

//! Algorithm Interface.
size_t
ConvertNrrdToFieldAlg::execute(size_t Nrrd)
{
  DataManager *dm = DataManager::get_dm();
  // Define local handles of data objects:
  NrrdDataHandle inrrd;
  FieldHandle ofield;

  inrrd = dm->get_nrrd(Nrrd);
  if (inrrd.get_rep() == 0) { return 0; }


  SCIRunAlgo::ConverterAlgo algo(get_progress_reporter());
  if (! algo.NrrdToField(inrrd, ofield, get_p_datalocation(), 
                         get_p_fieldtype(), get_p_convertparity())) 
  {
    return 0;
  }

  // send new output if there is any:
  return dm->add_field(ofield);
}

} //end namespace SCIRun

