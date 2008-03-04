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

#include <Core/Algorithms/Util/AlgoParamList.h>
#include <ctype.h>

namespace SCIRunAlgo {

bool 
AlgoParamList::set_bool(std::string key, bool value)
{
  BoolParam* param = dynamic_cast<BoolParam*>(parameters_[key]);
  if (param) { param->value() = value; return (true); }
  return (false); 
}

bool
AlgoParamList::get_bool(std::string key, bool& value)
{
  BoolParam* param = dynamic_cast<BoolParam*>(parameters_[key]);
  if (param) { value = param->value(); return (true); }
  else { return (false); }
}

bool 
AlgoParamList::set_int(std::string key, int value)
{
  IntegerParam* param = dynamic_cast<IntegerParam*>(parameters_[key]);
  if (param) 
  { 
    if ((!param->limit()) ||
        (param->limit() && ((value < param->min())||(value > param->max()))))
    {
      param->value() = value; return (true); 
    }
  }
  return (false); 
}

bool
AlgoParamList::get_int(std::string key, int& value)
{
  IntegerParam* param = dynamic_cast<IntegerParam*>(parameters_[key]);
  if (param) { value = param->value(); return (true); }
  else { return (false); }
}

bool
AlgoParamList::get_int_limits(std::string key, int& min, int& max)
{
  IntegerParam* param = dynamic_cast<IntegerParam*>(parameters_[key]);
  if (param) { min = param->min(); max = param->max(); return (true); }
  else { return (false); }
}

bool 
AlgoParamList::set_scalar(std::string key, double value)
{
  ScalarParam* param = dynamic_cast<ScalarParam*>(parameters_[key]);
  if (param) 
  { 
    if ((!param->limit()) ||
        (param->limit() && ((value < param->min())||(value > param->max()))))
    {
      param->value() = value; return (true); 
    }
  }
  return (false); 
}

bool
AlgoParamList::get_scalar(std::string key, double& value)
{
  ScalarParam* param = dynamic_cast<ScalarParam*>(parameters_[key]);
  if (param) { value = param->value(); return (true); }
  else { return (false); }
}

bool
AlgoParamList::get_scalar_limits(std::string key, double& min, double& max)
{
  ScalarParam* param = dynamic_cast<ScalarParam*>(parameters_[key]);
  if (param) { min = param->min(); max = param->max(); return (true); }
  else { return (false); }
}

bool
AlgoParamList::set_color(std::string key, SCIRun::Color value)
{
  ColorParam* param = dynamic_cast<ColorParam*>(parameters_[key]);
  if (param) 
  {
    param->value() = value; return (true); 
  }
  return (false); 
}

bool
AlgoParamList::get_color(std::string key, SCIRun::Color& value)
{
  ColorParam* param = dynamic_cast<ColorParam*>(parameters_[key]);
  if (param) { value = param->value(); return (true); }
  else { return (false); }
}


bool
AlgoParamList::set_vector(std::string key, SCIRun::Vector value)
{
  VectorParam* param = dynamic_cast<VectorParam*>(parameters_[key]);
  if (param) 
  {
    param->value() = value; return (true); 
  }
  return (false); 
}

bool
AlgoParamList::get_vector(std::string key, SCIRun::Vector& value)
{
  VectorParam* param = dynamic_cast<VectorParam*>(parameters_[key]);
  if (param) { value = param->value(); return (true); }
  else { return (false); }
}

bool
AlgoParamList::get_point(std::string key, SCIRun::Point& value)
{
  PointParam* param = dynamic_cast<PointParam*>(parameters_[key]);
  if (param) { value = param->value(); return (true); }
  else { return (false); }
}

bool
AlgoParamList::set_point(std::string key, SCIRun::Point value)
{
  PointParam* param = dynamic_cast<PointParam*>(parameters_[key]);
  if (param) 
  {
    param->value() = value; return (true); 
  }
  return (false); 
}

bool
AlgoParamList::set_option(std::string key, std::string value)
{
  OptionParam* param = dynamic_cast<OptionParam*>(parameters_[key]);
  for (size_t j=0; j<value.size(); j++) value[j] = tolower(value[j]);
  
  if (param) 
  {
    std::vector<std::string>::iterator it = param->options().begin();
    std::vector<std::string>::iterator it_end = param->options().end();
    while (it != it_end)
    {
      if ((*it) == value) { param->value() = value; return (true); }
      ++it;
    }
  }
  return (false);
}

bool
AlgoParamList::get_option(std::string key, std::string& value)
{
  OptionParam* param = dynamic_cast<OptionParam*>(parameters_[key]);
  if (param) { value = param->value(); return (true); }
  else { return (false); }
}

bool 
AlgoParamList::set_string(std::string key, std::string value)
{
  StringParam* param = dynamic_cast<StringParam*>(parameters_[key]);
  if (param) { param->value() = value; return (true); }
  else { return (false); }
}

bool
AlgoParamList::get_string(std::string key, std::string& value)
{
  StringParam* param = dynamic_cast<StringParam*>(parameters_[key]);
  if (param) { value = param->value(); return (true); }
  else { return (false); }
}

bool 
AlgoParamList::set_filename(std::string key, std::string value)
{
  FilenameParam* param = dynamic_cast<FilenameParam*>(parameters_[key]);
  if (param) { param->value() = value; return (true); }
  else { return (false); }
}

bool
AlgoParamList::get_filename(std::string key, std::string& value)
{
  FilenameParam* param = dynamic_cast<FilenameParam*>(parameters_[key]);
  if (param) { value = param->value(); return (true); }
  else { return (false); }
}


AlgoParamList::~AlgoParamList()
{
  parameters_type::iterator it, it_end;
  it = parameters_.begin();
  it_end = parameters_.end();
  while(it != it_end) { delete (*it).second; ++it; }
}

void
AlgoParamList::add_bool(std::string key, bool defval)
{
  parameters_[key] = new BoolParam(defval);
}

void
AlgoParamList::add_int(std::string key, int defval)
{
  parameters_[key] = new IntegerParam(defval);
}

void
AlgoParamList::add_int(std::string key, int defval, int min, int max)
{
  parameters_[key] = new IntegerParam(defval,min,max);
}

void
AlgoParamList::add_scalar(std::string key, double defval)
{
  parameters_[key] = new ScalarParam(defval);
}

void
AlgoParamList::add_scalar(std::string key, double defval, double min, double max)
{
  parameters_[key] = new ScalarParam(defval,min,max);
}

void
AlgoParamList::add_option(std::string key, std::string defval, std::string options)
{
  std::vector<std::string> opts;
  for (size_t j=0; j<options.size(); j++) options[j] = tolower(options[j]);
  while(1)
  {
    size_t loc = options.find('|');
    if (loc >= options.size())
    {
      opts.push_back(options);
      break;
    }
    opts.push_back(options.substr(0,loc));
    options = options.substr(loc+1);
  }
  
  parameters_[key] = new OptionParam(defval,opts);
}

void
AlgoParamList::add_string(std::string key, std::string defval)
{
  parameters_[key] = new StringParam(defval);
}

void
AlgoParamList::add_filename(std::string key, std::string defval)
{
  parameters_[key] = new FilenameParam(defval);
}

}
