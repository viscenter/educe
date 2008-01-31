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
 *  CalculateFieldDataCompiled: Field data operations
 *
 *  Written by:
 *   Allen R. Sanderson
 *   Michael Callahan
 *   SCI Institute
 *   University of Utah
 *   February 2006
 *
 *  Copyright (C) 2006 SCI Group
 */


#include <Core/Algorithms/Fields/TransformMeshWithFunction.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>
#include <Core/Containers/HashTable.h>
#include <Core/Containers/StringUtil.h>
#include <iostream>

namespace SCIRun {


bool TransformMeshWithFunctionAlgo::
TransformMeshWithFunction(ProgressReporter *pr,
			  FieldHandle& field_input_handle,
			  FieldHandle& field_output_handle,
			  string function )
{
  const TypeDescription *ftd = field_input_handle->get_type_description();

  int hoffset = 0;
  Handle<TransformMeshWithFunctionAlgo> algo;
  
  while (1) {
    CompileInfoHandle ci = get_compile_info(ftd,function, hoffset);
    
    if(!(SCIRun::DynamicCompilation::compile(ci,algo,pr))) {
      pr->compile_error(ci->filename_);
      SCIRun::DynamicLoader::scirun_loader().cleanup_failed_compile(ci);  
      return(false);
    }
    
    if (algo->identify() == function)
      break;
    
    hoffset++;
  }

  return( algo->execute(pr,
			field_input_handle,
			field_output_handle) );

}



CompileInfoHandle
TransformMeshWithFunctionAlgo::get_compile_info(const TypeDescription *field_td,
						string function,
						int hashoffset)

{
  unsigned int hashval = Hash(function, 0x7fffffff) + hashoffset;

  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  const string template_name("TransformMeshWithFunctionInstance" + to_string(hashval));
  static const string base_class_name("TransformMeshWithFunctionAlgo");

  CompileInfo *rval = 
    scinew CompileInfo(template_name + "." +
		       field_td->get_filename() + ".",
                       base_class_name, 
                       template_name, 
                       field_td->get_name());

  // Code for the function.
  string class_declaration =
    string("template <class FIELD>\n") +
    "class " + template_name + " : public TransformMeshWithFunctionAlgoT<FIELD>\n" +
    "{\n" +
    "  virtual void function(Point &result, double x, double y, double z,\n" +
    "                        typename FIELD::value_type &v)\n" +
    "  {\n" +
    "    " + function + "\n" +
    "  }\n" +
    "\n" +
    "  virtual string identify()\n" +
    "  { return string(\"" + string_Cify(function) + "\"); }\n" +
    "};\n";

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  rval->add_post_include(class_declaration);
  field_td->fill_compile_info(rval);
  return rval;
}


} // End namespace SCIRun
