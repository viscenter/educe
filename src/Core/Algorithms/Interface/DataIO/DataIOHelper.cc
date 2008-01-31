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

#include <Core/Algorithms/Interface/DataIO/DataIOHelper.h>
#include <iostream>
#include <sstream>

using namespace SCIRun;
using namespace std;

std::string DataIOHelper::streamTypeFromExportType(const std::string& export_type)
{
  const string::size_type loc = export_type.find(" (");
  const string ft = export_type.substr(0, loc);

  if (ft == "SCIRun Field ASCII")
    return string("Text");
  else
    return string("Binary");
}

std::string DataIOHelper::streamTypeFromFileType(const std::string& file_type)
{
  if (file_type == "Binary")
    return string("Binary");
  else
    return string("Text");
}

bool DataIOHelper::numberFilename(std::string& filename, int number)
{
  const string::size_type loc2 = filename.find("%d");
  if(loc2 == string::npos) {
    //warn the user if they try to use 'Increment' incorrectly
    cerr << "To use the increment function, there must be a '%d' in the file name.\n";
    // NOTE(bigler): should we turn p_increment_ off?
    return false;
  } else {
    ostringstream number_str;
    number_str << number;
    filename.replace(loc2, 2, number_str.str());
    return true;
  }
}



