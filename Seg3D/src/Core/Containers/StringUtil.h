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

#ifndef CORE_CONTAINERS_STRINGUTIL_H
#define CORE_CONTAINERS_STRINGUTIL_H 1

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
bool multiple_from_string(const string &str, std::vector<T> &values)
{
  values.clear();
  
  string data = str;
  for (size_t j=0; j<data.size(); j++) 
    if ((data[j] == '\t')||(data[j] == '\r')||(data[j] == '\n')||(data[j]=='"')) data[j] = ' ';
    
  std::vector<std::string> nums;
  for (size_t p=0;p<data.size();)
  { 
    while((data[p] == ' ')&&(p<data.size())) p++;
    if (p >= data.size()) break;

    std::string::size_type next_space = data.find(' ',p);
    if (next_space == std::string::npos) next_space = data.size();
    T value;
    if(from_string(data.substr(p,next_space-p),value)) values.push_back(value);
    p = next_space;

    if (p >= data.size()) break;
  }
  if (values.size() > 0) return (true);
  return (false);
}

template <class T>
bool from_string(const string &str, T &value)
{
  std::istringstream iss(str+"  ");
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

SCISHARE bool from_string(const string &str, double &value);
SCISHARE bool from_string(const string &str, float &value);
SCISHARE bool from_string(const string &str, int &value);
SCISHARE bool from_string(const string &str, unsigned int &value);
SCISHARE bool from_string(const string &str, long &value);
SCISHARE bool from_string(const string &str, unsigned long &value);
SCISHARE bool from_string(const string &str, long long &value);
SCISHARE bool from_string(const string &str, unsigned long long &value);


inline bool string_to_bool(const string &str, bool &result) 
{ return(from_string(str,result)); }
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
