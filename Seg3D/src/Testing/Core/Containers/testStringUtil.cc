/*
  For more information, please see: http://software.sci.utah.edu
 
  The MIT License
 
  Copyright (c) 2004 Scientific Computing and Imaging Institute,
  University of Utah.
 
  
  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#include <Core/Containers/StringUtil.h>
#include <iostream>

int main() {
  const std::string int_string = "123";
  int int_result = 0;
  if (!SCIRun::string_to_int(int_string, int_result)) {
    std::cerr << "StringUtil string_to_int failed\n";
    return -1;
  } else if (int_result != 123) {
    std::cerr << "StringUtil string_to_int returned ";
    std::cerr << "incorrect results: " << int_result << "\n";
    return -1;
  }

  const std::string double_string = "1.123456789";
  double double_result = 0;
  if (!SCIRun::string_to_double(double_string, double_result)) {
    std::cerr << "StringUtil string_to_double failed\n";
    return -1;
  } else if (double_result != 1.123456789) {
    std::cerr << "StringUtil string_to_double returned ";
    std::cerr << "incorrect results: " << double_result << "\n";
    return -1;
  }

  const std::string unsigned_long_string = "429496729";
  unsigned long unsigned_long_result = 0;
  if (!SCIRun::string_to_unsigned_long(unsigned_long_string, 
				       unsigned_long_result)) {
    std::cerr << "StringUtil string_to_unsigned_long failed\n";
    return -1;
  } else if (unsigned_long_result != 429496729) {
    std::cerr << "StringUtil string_to_unsigned_long returned ";
    std::cerr << "incorrect results: " << unsigned_long_result << "\n";
    return -1;
  }

  
  return 0;
} 
