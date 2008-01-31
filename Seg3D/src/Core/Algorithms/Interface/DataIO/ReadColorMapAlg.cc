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
#include <Core/Algorithms/Interface/DataIO/ReadColorMapAlg.h>
#include <Core/Algorithms/Interface/DataIO/DataIOHelper.h>

namespace SCIRun {

ReadColorMapAlg* get_read_colormap_alg()
{
  return new ReadColorMapAlg();
}

//! Algorithm Interface.
  //! Algorithm Interface.
vector<size_t>
ReadColorMapAlg::execute(string filename)
{
  vector<size_t> result;
  ColorMapHandle handle;
  if (DataIOHelper::readFile(filename, handle, get_progress_reporter())) {
    // Errors
    return result;
  }
  
  DataManager *dm = DataManager::get_dm();
  result.push_back( dm->add_colormap(handle) );
  return result;
}

} //end namespace SCIRun

