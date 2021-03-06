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



/*
 *  StringUtil.h: some useful string functions
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   April 2001
 *
 *  Copyright (C) 2001 SCI Group
 */

#ifndef SCI_Core_StringUtil_h
#define SCI_Core_StringUtil_h 1

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

#include <Core/Containers/share.h>

namespace SCIRun {
  using std::string;
  using std::vector;

template <class T>
bool from_string(const string &str, T &value)
{
  std::istringstream iss(str+" ");
  iss.exceptions(std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit);
  try
  {
    iss >> value;
    return (true);
  }
  catch (...)
  {
    return (false);
  }
}


inline bool string_to_int(const string &str, int &result) 
{ return(from_string(str,result)); }
inline bool string_to_double(const string &str, double &result) 
{ return(from_string(str,result)); }
inline bool string_to_unsigned_long(const string &str, unsigned long &result) 
{ return(from_string(str,result)); }


template <class T>
string to_string(T val) 
{  
  std::ostringstream oss; oss << val; return (oss.str()); 
}
 
SCISHARE string string_toupper(string);
SCISHARE string string_tolower(string);

//////////
// Remove directory name
inline string basename(const string &path)
{ return (path.substr(path.rfind('/')+1)); }

//////////
// Return directory name
inline string pathname(const string &path)
{ return (path.substr(0, path.rfind('/')+1)); }

// Split a string into multiple parts, separated by the character sep
SCISHARE vector<string> split_string(const string& str, char sep);

/////////
// C++ify a string, turn newlines into \n, use \t, \r, \\ \", etc.
SCISHARE string string_Cify(const string &str);

//////////
// Unsafe cast from string to char *, used to export strings to C functions.
SCISHARE char* ccast_unsafe(const string &str);

// replaces all occurances of 'substr' in 'str' with 'replacement'
SCISHARE string replace_substring(string str, 
                                  const string &substr, 
                                  const string &replacement);

// Returns true if 'str' ends with the string 'substr'
SCISHARE bool ends_with(const string &str, const string &substr);


} // End namespace SCIRun

#endif
