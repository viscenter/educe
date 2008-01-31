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

#include <Core/Algorithms/ArrayMath/ArrayEngine.h>

namespace SCIRunAlgo {

ArrayEngine::ArrayEngine(SCIRun::ProgressReporter *pr) :
  pr_(pr),
  free_pr_(false)
{
  if (pr_ == 0)
  {
    pr_ = scinew SCIRun::ProgressReporter;
    free_pr_ = true;
  }
}

ArrayEngine::~ArrayEngine()
{
  if (free_pr_) delete pr_;
}

bool ArrayEngine::computesize(ArrayObjectList& Input, int& size)
{
  size = 1;
  
  for (size_t p=0; p < Input.size(); p++)
  {
    if (!(Input[p].is_valid())) 
    { 
      return(false);
    }
    
    if (Input[p].size() == 0)
    {
      return(false);
    }
    
    if (size == 1)
    {
      size = Input[p].size();
    }
    else
    {
      if ((Input[p].size() != 1)&&(Input[p].size() != size))
      {
        return(false);
      }      
    }
  }

  return(true);
}


bool ArrayEngine::engine(ArrayObjectList& Input, ArrayObjectList& Output, std::string function)
{
  // Do some up front checking on whether the input data is correct
  
  int n = 1;
  
  for (size_t p=0; p < Input.size(); p++)
  {
    if (!(Input[p].is_valid())) 
    { 
      std::ostringstream oss;
      oss << "Input object " << p+1 << " does not seem to be a valid object";
      pr_->error(oss.str());
      return(false);     
    }
    
    if (Input[p].size() == 0)
    {
      std::ostringstream oss;
      oss << "Input object " << p+1 << " is empty";
      pr_->error(oss.str());
      return(false);
    }
    
    if (n == 1)
    {
      n = Input[p].size();
    }
    else
    {
      if ((Input[p].size() != 1)&&(Input[p].size() != n))
      {
        std::ostringstream oss;
        oss << "The size of input object " << p+1 << " is not one or does not match the size of the other objects";
        pr_->error(oss.str());
        return(false);
      }      
    }
  }

  if (Output.size() < 1)
  {
    pr_->error("No output matrix/field is given");
    return(false);
  }

  for (size_t p=0; p < Output.size(); p++)
  {
    if (!Output[p].is_valid())
    {
        std::ostringstream oss;
        oss << "Output object " << p << " does not seem to be a valid object";
        pr_->error(oss.str());
        return(false);     
    }
    if ((n != 1)&&(Output[p].size() != n))
    {
        std::ostringstream oss;
        oss << "The size of output object " << p << " is not equal to the number of elements in input";
        pr_->error(oss.str());
        return(false);        
    }
  }

  // Remove white spaces from function
  while (function.size() && isspace(function[function.size()-1])) function.resize(function.size()-1);

  int hoffset = 0;
  SCIRun::Handle<DataArrayMath::ArrayEngineAlgo> algo;
    
  while (1) 
  {
    SCIRun::CompileInfoHandle ci = DataArrayMath::ArrayEngineAlgo::get_compile_info(Input,Output,function,hoffset);
    if (!SCIRun::DynamicCompilation::compile(ci, algo, pr_)) 
    {
      pr_->compile_error(ci->filename_);
      SCIRun::DynamicLoader::scirun_loader().cleanup_failed_compile(ci);
      return(false);
    }

    if (algo->identify() == function) 
    {
      break;
    }
    hoffset++;
  }

  algo->function(Input,Output,n,pr_);
  return(true);
}

} // end namespace

namespace DataArrayMath {

SCIRun::CompileInfoHandle ArrayEngineAlgo::get_compile_info(
        SCIRunAlgo::ArrayObjectList& Input, SCIRunAlgo::ArrayObjectList& Output,
        std::string function, int hashoffset)
{
  unsigned int hashval = SCIRun::Hash(function, 0x7fffffff) + hashoffset;

  // name of include_path
  static const std::string include_path(SCIRun::TypeDescription::cc_to_h(__FILE__));
  // name of the basis class
  static const std::string base_class_name("ArrayEngineAlgo");

  // Unique filename
  std::string inputtype;
  for (size_t p = 0; p < Input.size(); p++)
  {
      if (Input[p].is_scalar())   { inputtype += "S"; continue; }
      if (Input[p].is_tensor())   { inputtype += "T"; continue; }
      if (Input[p].is_vector())   { inputtype += "V"; continue; }    
      if (Input[p].is_element())  { inputtype += "E"; continue; }    
      if (Input[p].is_location()) { inputtype += "L"; continue; }    
      if (Input[p].is_index())    { inputtype += "I"; continue; }    
      inputtype += "U";
  }

  std::string outputtype;
  for (size_t p = 0; p < Output.size(); p++)
  {
    if (Output[p].is_scalar())   { outputtype += "S"; continue; }
    if (Output[p].is_tensor())   { outputtype += "T"; continue; }
    if (Output[p].is_vector())   { outputtype += "V"; continue; }
    if (Output[p].is_location()) { outputtype += "L"; continue; }    
    outputtype += "U";
  }

  std::string template_name("ALGOArrayEngine_" + SCIRun::to_string(hashval) + "_" + inputtype + "_" + outputtype);

  SCIRun::CompileInfo *ci = scinew SCIRun::CompileInfo(template_name+".","DataArrayMath::"+base_class_name,"DataArrayMath::"+template_name,"double");

  // Code for the function.
  std::string fcn;
  
  fcn = std::string("namespace DataArrayMath {\ntemplate <class DATATYPE>\n") +
    "class " + template_name + " : public ArrayEngineAlgo\n" +
    "{\nvirtual void function(SCIRunAlgo::ArrayObjectList& input_," + 
    "SCIRunAlgo::ArrayObjectList& output_, int size_, SCIRun::ProgressReporter* pr_)\n  {\n";
  fcn += "    DATATYPE dummy_ = 0.0; dummy_ += 1.0;\n\n";   // Make compiler happy
  
  for (size_t p = 0; p< Input.size(); p++)
  {
    if (Input[p].is_scalar())       fcn += "    DataArrayMath::Scalar " + Input[p].get_name() + ";\n";
    if (Input[p].is_vector())       fcn += "    DataArrayMath::Vector " + Input[p].get_name() + ";\n";
    if (Input[p].is_tensor())       fcn += "    DataArrayMath::Tensor " + Input[p].get_name() + ";\n";
    if (Input[p].is_location())   { fcn += "    DataArrayMath::Scalar " + Input[p].get_x_name() + ";\n";
                                    fcn += "    DataArrayMath::Scalar " + Input[p].get_y_name() + ";\n";
                                    fcn += "    DataArrayMath::Scalar " + Input[p].get_z_name() + ";\n";
                                    fcn += "    DataArrayMath::Vector " + Input[p].get_name() + ";\n"; }
    if (Input[p].is_index())      { fcn += "    DataArrayMath::Scalar " + Input[p].get_name() + ";\n";                                   
                                    fcn += "    DataArrayMath::Scalar " + Input[p].get_size_name() + " = static_cast<DataArrayMath::Scalar>(size_);\n"; }
    if (Input[p].is_element())    { std::ostringstream oss; oss << p;
                                    fcn += "    DataArrayMath::Element " + Input[p].get_name() + ";\n";
                                    fcn += "    input_["+oss.str()+"].get_element("+Input[p].get_name() +");\n";  }                                  
  }

  for (size_t p = 0; p< Output.size(); p++)
  {
    if (Output[p].is_scalar())      fcn += "    DataArrayMath::Scalar " + Output[p].get_name() + ";\n";
    if (Output[p].is_vector())      fcn += "    DataArrayMath::Vector " + Output[p].get_name() + ";\n";
    if (Output[p].is_tensor())      fcn += "    DataArrayMath::Tensor " + Output[p].get_name() + ";\n";
    if (Output[p].is_location())    fcn += "    DataArrayMath::Vector " + Output[p].get_name() + ";\n";
  }
  
  fcn += "\n\n";
  fcn += "int cnt_ = 0;";
  fcn += "    for (int p_ = 0; p_ < size_; p_++)\n    {\n";
  
  for (size_t p = 0; p< Input.size(); p++)
  {
    std::ostringstream oss; oss << p;
    if (Input[p].is_scalar())        fcn += "      input_[" + oss.str() + "].get_next_scalar(" + Input[p].get_name() + ");\n";
    if (Input[p].is_vector())        fcn += "      input_[" + oss.str() + "].get_next_vector(" + Input[p].get_name() + ");\n";
    if (Input[p].is_tensor())        fcn += "      input_[" + oss.str() + "].get_next_tensor(" + Input[p].get_name() + ");\n";
    if (Input[p].is_location())    { fcn += "      input_[" + oss.str() + "].get_next_location(" + Input[p].get_name() + ");\n"; 
                                     fcn += "      " + Input[p].get_x_name() + "=" + Input[p].get_name() + ".x();\n";
                                     fcn += "      " + Input[p].get_y_name() + "=" + Input[p].get_name() + ".y();\n";
                                     fcn += "      " + Input[p].get_z_name() + "=" + Input[p].get_name() + ".z();\n"; }
    if (Input[p].is_index())         fcn += "      " + Input[p].get_name() + "= static_cast<double>(p_);\n";
  }
  
  fcn += "\n\n";
  fcn += "      " + function + " \n\n\n";

  for (size_t p = 0; p< Input.size(); p++)
  {
    std::ostringstream oss; oss << p;
    if (Input[p].is_element())        fcn += "      " + Input[p].get_name() + ".next();\n";
  }

  for (size_t p = 0; p< Output.size(); p++)
  {
    std::ostringstream oss; oss << p;
    if (Output[p].is_scalar())       fcn += "     output_[" + oss.str() + "].set_next_scalar(" + Output[p].get_name() + ");\n";
    if (Output[p].is_vector())       fcn += "     output_[" + oss.str() + "].set_next_vector(" + Output[p].get_name() + ");\n";
    if (Output[p].is_tensor())       fcn += "     output_[" + oss.str() + "].set_next_tensor(" + Output[p].get_name() + ");\n";
    if (Output[p].is_location())     fcn += "     output_[" + oss.str() + "].set_next_location(" + Output[p].get_name() + ");\n"; 

  }
  
  fcn += std::string("    cnt_++; if (cnt_ == 200) { cnt_ = 0; pr_->update_progress(p_,size_); }\n");
  
  fcn += std::string("    }\n  }\n\n") +
    "  virtual std::string identify()\n" +
    "  { return std::string(\"" + std::string(SCIRun::string_Cify(function)) + 
    "\"); }\n};\n\n}\n";

  // Add in the include path to compile this obj
  ci->add_include(include_path);
  ci->add_post_include(fcn);

  return(ci);
}


} // End namespace SCIRun
