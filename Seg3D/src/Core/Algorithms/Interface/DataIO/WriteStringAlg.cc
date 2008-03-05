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
#include <fstream>
#include <Core/Events/DataManager.h>
#include <Core/Algorithms/Interface/DataIO/WriteStringAlg.h>
#include <Core/Algorithms/Interface/DataIO/DataIOHelper.h>

using namespace SCIRun;

WriteStringAlg* SCIRun::get_write_string_alg()
{
  return new WriteStringAlg();
}

//! Algorithm Interface.
void
WriteStringAlg::execute(string in_string, string filename)
{
  // Do some initial error checking before we do any work.
  if (filename == "")
  {
    cerr << "No filename specified.\n";
    return;
  }
  //determine if we should increment an index in the file name
  if (get_p_increment()) {
    DataIOHelper::numberFilename(filename, get_p_current());
    set_p_current(get_p_current()+1);
  }
  ofstream outfile(filename.c_str());
  if (!outfile) {
    cerr << "Could not open file for writing " << filename << "\n";
    return;
  } else {
    outfile << in_string;
  }
  outfile.close();
}

