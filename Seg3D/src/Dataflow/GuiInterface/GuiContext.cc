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
 *  GuiContext.cc:
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   April 2002
 *
 *  Copyright (C) 2002 SCI Group
 */

#include <Core/Containers/StringUtil.h>
#include <Core/Malloc/Allocator.h>
#include <Core/Math/MiscMath.h>
#include <Core/Util/Assert.h>

#include <Dataflow/GuiInterface/GuiContext.h>
#include <Dataflow/GuiInterface/GuiInterface.h>

#include <iostream>
#include <sstream>
#include <iomanip>

#include <teem/air.h>
#include <teem/nrrd.h>

using std::string;
using std::vector;
using namespace SCIRun;

GuiContext::GuiContext(GuiInterface* gui, 
		       const string& name,
		       bool save, // default = true
		       GuiContext *parent) // default = 0
  : gui_(gui),
    parent_(parent),
    name_(name),
    children_(),
    context_state_(CACHE_E),
    active_(true)
{
  context_state_ |= save?SAVE_E:0;
  if (save)
    tcl_setVarStates();
}


GuiContext::~GuiContext()
{
  dontSave();
  if (parent_) {
    for(vector<GuiContext*>::iterator iter = parent_->children_.begin();
          iter != parent_->children_.end(); ++iter) 
    {
      if (*iter == this)
      {
        parent_->children_.erase(iter);
        return;
      } 
    }
  }
}



GuiContext* 
GuiContext::subVar(const string& subname, bool saveChild)
{
  dontSave(); // Do not save intermediate nodes
  GuiContext* child = scinew GuiContext(gui_, name_+"-"+subname, saveChild, this);
  children_.push_back(child);
  return child;
}


GuiContext * 
GuiContext::find_child(const string &subname)
{
  string fullname = name_+"-"+subname;
  for (unsigned int c = 0; c < children_.size(); ++c)
    if (fullname == children_[c]->name_)
      return children_[c];
  return 0;
}

void
GuiContext::inactivate()
{
  active_ = false;
  for (unsigned int c = 0; c < children_.size(); ++c) children_[c]->inactivate();
}


bool
GuiContext::get(string& value)
{
  if((context_state_ & CACHE_E) && (context_state_ & CACHED_E))
    return true;
  context_state_ &= ~CACHED_E;
  if(!getString(name_, value))
    return false;
  context_state_ |= CACHED_E;
  return true;
}

void
GuiContext::set(const string& value)
{
  string tmp;
  if ((context_state_ & SAVE_E) && getString(name_, tmp) && tmp == value) { 
    return; 
  }
  context_state_ &= ~CACHED_E;
  setString(name_, value);
}

bool
GuiContext::get(double& value)
{
  if((context_state_ & CACHE_E) && (context_state_ & CACHED_E))
    return true;
  context_state_ &= ~CACHED_E;
  string result;
  if(!getString(name_, result))
    return false;
  
  value = atof(result.c_str());    

  if (result.size() > 2)
  {
    if (((result[0]=='n')||(result[0]=='N'))
        &&((result[1]=='a')||(result[1]=='A'))
        &&((result[2]=='n')||(result[2]=='N')))
    {
      value = (double) AIR_NAN;
    }
    else if (((result[0]=='i')||(result[0]=='I'))
        &&((result[1]=='n')||(result[1]=='N'))
        &&((result[2]=='f')||(result[2]=='F')))
    {
      value = (double) AIR_POS_INF;
    }
  }
  if (result.size() > 3)
  {
    if (((result[0]=='+'))
        &&((result[1]=='i')||(result[1]=='I'))
        &&((result[2]=='n')||(result[2]=='N'))
        &&((result[3]=='f')||(result[3]=='F')))
    {
      value = (double) AIR_POS_INF;
    }
    else if (((result[0]=='-'))
        &&((result[1]=='i')||(result[1]=='I'))
        &&((result[2]=='n')||(result[2]=='N'))
        &&((result[3]=='f')||(result[3]=='F')))
    {
      value = (double) AIR_NEG_INF;
    }
  }
  
  context_state_ |= CACHED_E;
  return true;
}

void
GuiContext::set(double value)
{
  value = MakeReal(value);
  ostringstream stream;
  // Print the number 17 digits wide with decimal
  stream << setiosflags(ios::showpoint) << setprecision(17) << value;
  // Evaluate it in TCL to pare down extra 0's at the end

  const string svalue = gui_->eval("expr "+stream.str(),this);
  if (active_ == false) return;
  
  string tmp;
  if ((context_state_ & SAVE_E) && getString(name_, tmp) && tmp == svalue) 
  { 
    return; 
  }

  context_state_ &= ~CACHED_E;
  gui_->set(name_, svalue,this);
}

bool
GuiContext::get(int& value)
{
  if ((context_state_ & CACHE_E) && (context_state_ & CACHED_E)) return true;
  context_state_ &= ~CACHED_E;
  string result;
  if(!getString(name_, result)) return false;
//  std::istringstream s(result);
//  s >> value;
//  if(!s) return false;
  
  value = atoi(result.c_str());
  context_state_ |= CACHED_E;
  return true;
}

void
GuiContext::set(int value)
{
  ostringstream val;
  val << value;
  string tmp;
  if ((context_state_ & SAVE_E) && gui_->get(name_,tmp,this) && tmp == val.str()) 
  { 
    return; 
  }
  context_state_ &= ~CACHED_E;
  setString(name_, val.str());
}


string
GuiContext::getMapKeyFromString(const string &varname)
{
  ASSERT(stringIsaMap(varname));
  ASSERT(varname.size() >= 4); // smallest possible name: ie a(b) = 4 chars
  string::size_type open_paren = varname.find_first_of("(");
  string::size_type close_paren = varname.find_last_of(")");
  ASSERT(open_paren && close_paren);
  string key = varname.substr(open_paren+1, close_paren-open_paren-1);
  if (stringIsaMap(key) || stringIsaListElement(key))
  {
    string new_key;
    if (!getString(key, new_key)) 
    {
      return key;
    }
    key = new_key;
  }
  return key;
}


string
GuiContext::getMapNameFromString(const string &varname)
{
  ASSERT(stringIsaMap(varname));
  ASSERT(varname.size() >= 4); // smallest possible name: ie a(b) = 4 chars
  string::size_type open_paren = varname.find_first_of("(");
  ASSERT(open_paren);
  return varname.substr(0, open_paren);
}

int
GuiContext::popLastListIndexFromString(string &varname)
{
  ASSERT(varname.size() >= 4); // smallest possible name: ie i[0] = 4 chars
  string::size_type open_bracket = varname.find_last_of("[");
  string::size_type close_bracket = varname.find_last_of("]");
  ASSERT(open_bracket && close_bracket);
  int i = -1;
  string_to_int(varname.substr(open_bracket+1, close_bracket-1),i);
  if (open_bracket > 0)
    varname = varname.substr(0, open_bracket);
  return i;
}

bool
GuiContext::getString(const string& varname, string& value)
{
  bool success = false;
  if (stringIsaListElement(varname))
  {
    vector<int> indexes;
    string listname = varname;
    int idx;
    while((idx = popLastListIndexFromString(listname)) != -1)
      indexes.push_back(idx);

    string list_contents;
    if (!getString(listname, list_contents)) 
    {
      return false;
    }
    
    success = gui_->extract_element_from_list (list_contents, indexes, value, this);
    return success;
  } 
  else if (stringIsaMap(varname)) 
  {
    success = gui_->get_map(getMapNameFromString(varname), 
                            getMapKeyFromString(varname), value, this);
    return success;
  }
  // else? just do a standard gui get
  success = gui_->get(varname, value, this);
  return success;
}


bool
GuiContext::setString(const string& varname, const string& value)
{
  bool success = true;
  if (stringIsaListElement(varname))
  {
    vector<int> indexes;
    string listname = varname;
    int idx;
    while((idx = popLastListIndexFromString(listname)) != -1)
      indexes.push_back(idx);

    string list_contents;
    if (!getString(listname,list_contents)) {
      //      cerr << "Cannot find list variable: " << listname;
      return false;
    }
    
    success = gui_->set_element_in_list(list_contents, indexes, value,this);
    if (!success) return (false);
    success = setString(listname, list_contents);

    return success;
  } 
  else if (stringIsaMap(varname)) 
  {
    success = gui_->set_map(getMapNameFromString(varname), getMapKeyFromString(varname), value);
    return success;
  }
  // else just do a standard gui set
  gui_->set(varname, value,this);
  return success;
}

bool
GuiContext::stringIsaListElement(const string &str)
{
  if (!str.length()) return false;
  return (str[str.length()-1] == ']');
}

bool
GuiContext::stringIsaMap(const string &str)
{
  if (!str.length()) return false;
  return (str[str.length()-1] == ')');
}

void
GuiContext::reset()
{
  gui_->lock();
  if (active_ == false)
  {
    gui_->unlock();
    return;
  }
  gui_->unlock();
  
  context_state_ &= ~CACHED_E;
  for(vector<GuiContext*>::iterator iter = children_.begin();
      iter != children_.end(); ++iter)
    (*iter)->reset();
}

string
GuiContext::getfullname()
{
  return name_;
}

GuiInterface*
GuiContext::getInterface()
{
  return gui_;
}

void
GuiContext::tcl_setVarStates()
{
  const string save_flag = (context_state_ & SAVE_E)?"1":"0";
  const string sub_flag = (context_state_ & SUBSTITUTE_DATADIR_E)?" 1":" 0";
  const string filename_flag = (context_state_ & IS_FILENAME_E)?" 1":" 0";
  gui_->execute("setVarStates \""+name_+"\" "+save_flag+sub_flag+filename_flag,this);
}

void
GuiContext::dontSave()
{
  if ((context_state_ & SAVE_E) == 0) return;
  context_state_ &= ~SAVE_E;
  tcl_setVarStates();
}

void
GuiContext::doSave()
{
  if ((context_state_ & SAVE_E) == SAVE_E) return;
  context_state_ |= SAVE_E;
  tcl_setVarStates();
}


void
GuiContext::dontSubstituteDatadir()
{
  if ((context_state_ & SUBSTITUTE_DATADIR_E) == 0) return;
  context_state_ &= ~SUBSTITUTE_DATADIR_E;
  tcl_setVarStates();
}

void
GuiContext::doSubstituteDatadir()
{
  if ((context_state_ & SUBSTITUTE_DATADIR_E) == SUBSTITUTE_DATADIR_E) return;
  context_state_ |= SUBSTITUTE_DATADIR_E;
  tcl_setVarStates();
}


void
GuiContext::unsetIsFilename()
{
  if ((context_state_ & IS_FILENAME_E) == 0) return;
  context_state_ &= ~IS_FILENAME_E;
  tcl_setVarStates();
}

void
GuiContext::setIsFilename()
{
  if ((context_state_ & IS_FILENAME_E) == IS_FILENAME_E) return;
  context_state_ |= IS_FILENAME_E;
  tcl_setVarStates();
}


