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



#include <Core/Util/notset.h>
#include <Core/Util/Assert.h>
#include <Core/XMLUtil/XMLUtil.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/IntVector.h>
#include <Core/Containers/StringUtil.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#ifndef _WIN32
#include <strings.h>
#endif
#include <stdio.h>
using namespace std;

namespace SCIRun {

 
xmlAttrPtr
get_attribute_by_name(const xmlNodePtr p, const char *name) 
{
  xmlAttr *cur = p->properties;
  
  while (cur != 0) {
    if (cur->type == XML_ATTRIBUTE_NODE && 
        !strcmp(to_char_ptr(cur->name), name)) {
      return cur;
    }
    cur = cur->next;
  }
  return 0;
} 

bool
get_attributes(vector<xmlNodePtr> &attr, xmlNodePtr p) 
{
  attr.clear();
  xmlAttr *cur = p->properties;
  
  while (cur != 0) {
    if (cur->type == XML_ATTRIBUTE_NODE) 
    {
      attr.push_back(cur->children);
    }
    cur = cur->next;
  }
  return attr.size() > 0;
} 




string get_serialized_attributes(xmlNode* d)
{
  string fullstr;

  vector<xmlNodePtr> attr;
  get_attributes(attr, d);
  vector<xmlNodePtr>::iterator iter = attr.begin();
  while(iter != attr.end()) {
    xmlNodePtr n = *iter++;
    ostringstream strm;
    strm << " " << fullstr << n->name << "=\"" << n->content << "\"";
    fullstr = strm.str();
  }

  return fullstr;
}

string get_serialized_children(xmlNode* d)
{
  string fullstr;

  for (xmlNode *n = d->children; n != 0; n = n->next) {
    string str;
    if (n->type == XML_TEXT_NODE) {
      str = string(to_char_ptr(n->content));
    } else if (n->type == XML_ELEMENT_NODE) {
      ostringstream strm;
      strm << "<" << n->name << get_serialized_attributes(n) << ">" 
	   << get_serialized_children(n) << "</" << n->name << ">";
      str = strm.str();
    } else {
      ASSERTFAIL("unexpected node type, in XMLUtil.cc");
    }
    fullstr = fullstr + str;
  }
  return fullstr;
}

namespace XMLUtil {

bool node_is_element(const xmlNodePtr p, const char *name) {
  return (p->type == XML_ELEMENT_NODE && 
          !strcmp(name, xmlChar_to_char(p->name)));
}

bool node_is_dtd(const xmlNodePtr p, const string &name) {
  return (p->type == XML_DTD_NODE &&
	  name == xmlChar_to_char(p->name));
}

bool node_is_comment(const xmlNodePtr p)
{
  //  ASSERT(0);
  return true;
}

bool maybe_get_att_as_int(const xmlNodePtr p, 
      			     const string &name, 
			     int &val)
{
  string str;
  return (maybe_get_att_as_string(p, name, str) &&
	  string_to_int(str, val));
}


bool
maybe_get_att_as_double(const xmlNodePtr p, 
			   const string &name, 
			   double &val)
{
  string str;
  return (maybe_get_att_as_string(p, name, str) &&
	  string_to_double(str, val));
}


bool
maybe_get_att_as_string(const xmlNodePtr p, 
			   const string &name, 
			   string &val)
{
  xmlAttrPtr attr = get_attribute_by_name(p, name.c_str());
  if (!attr)
    return false;
  val = xmlChar_to_string(attr->children->content);
  return true;
}


const char *
maybe_get_att_as_const_char_str(const xmlNodePtr p, 
                                const char *name)
{
  xmlAttrPtr attr = get_attribute_by_name(p, name);
  return attr ? to_char_ptr(attr->children->content) : 0;
}

string node_att_as_string(const xmlNodePtr p, const string &name)
{
  xmlAttrPtr attr = get_attribute_by_name(p, name.c_str());
  if (!attr)
    throw "Attribute "+name+" does not exist!";
  return xmlChar_to_string(attr->children->content);
}

const char *
node_att_as_const_char_str(const xmlNodePtr p, const char *name)
{
  xmlAttrPtr attr = get_attribute_by_name(p, name);
  if (!attr)
    throw "Attribute "+string(name)+" does not exist!";
  return to_char_ptr(attr->children->content);
}



int node_att_as_int(const xmlNodePtr p, const string &name)
{
  int val = 0;
  string str = node_att_as_string(p, name);
  if (!string_to_int(str, val))
    throw "Attribute "+name+" value: "+str+" cannot convert to int";

  return val;
}

double node_att_as_double(const xmlNodePtr p, const string &name)
{
  double val = 0;
  string str = node_att_as_string(p, name);
  if (!string_to_double(str, val))
    throw "Attribute "+name+" value: "+str+" cannot convert to double";

  return val;
}

} // end namespace XMLUtil

} // End namespace SCIRun
