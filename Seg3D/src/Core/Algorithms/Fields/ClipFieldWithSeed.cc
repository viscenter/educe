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
 *  ClipFieldWithSeed.cc:  Clip out parts of a field using a seed(s)
 *
 *  Written by:
 *   Allen R. Sanderson
 *   SCI Institute
 *   University of Utah
 *   July 2007
 *
 *  Copyright (C) 2007 SCI Institute
 */

#include <Core/Containers/StringUtil.h>
#include <Core/Algorithms/Fields/ClipFieldWithSeed.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>
#include <Core/Containers/HashTable.h>
#include <iostream>


namespace SCIRun {

bool ClipFieldWithSeedAlgo::
ClipFieldWithSeed(ProgressReporter *pr,
		  FieldHandle& field_input_handle,
		  FieldHandle& seed_input_handle,
		  FieldHandle& field_output_handle,
		  MatrixHandle& matrix_output_handle,
		  string mode,
		  string function )
{
  if (!field_input_handle->mesh()->is_editable()) {
    pr->error("Not an editable mesh type.");
    pr->error("(Try passing Field through an Unstructure module first).");
    return (false);
  }

  int cMode = -1;
  if (mode == "cell")          cMode =  0;
  else if (mode == "allnodes") cMode = -1;

  if (!(field_input_handle->basis_order() == 0 && cMode == 0 ||
	field_input_handle->basis_order() == 1 && cMode != 0) &&
      field_input_handle->mesh()->dimensionality() > 0)
  {
    pr->warning("Basis doesn't match clip location, value will always be zero.");
  }

  int hoffset = 0;
  Handle<ClipFieldWithSeedAlgo> algo;
  
  while (1) {
    CompileInfoHandle ci = get_compile_info(field_input_handle,
					    seed_input_handle,
					    function, hoffset);
    
    if(!(SCIRun::DynamicCompilation::compile(ci,algo,pr))) {
      pr->compile_error(ci->filename_);
      SCIRun::DynamicLoader::scirun_loader().cleanup_failed_compile(ci);  
      return(false);
    }
    
    if (algo->identify() == function)
      break;
    
    hoffset++;
  }

  return( algo->execute( pr,
			 field_input_handle,
			 seed_input_handle,
			 field_output_handle,
			 matrix_output_handle,
			 cMode) );
}


CompileInfoHandle
ClipFieldWithSeedAlgo::
get_compile_info( FieldHandle& field_input_handle,
		  FieldHandle& seed_input_handle,
		  string function,
		  int hashoffset)
{
  unsigned int hashval = Hash(function, 0x7fffffff) + hashoffset;

  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  const string template_name("ClipFieldWithSeedInstanceAlgoT" +
			     to_string(hashval));
  static const string base_class_name("ClipFieldWithSeedAlgo");

  const TypeDescription *ftd = field_input_handle->get_type_description();
  const TypeDescription *std = seed_input_handle->get_type_description();

  TypeDescription::td_vec *tdv = 
    field_input_handle->get_type_description(Field::FDATA_TD_E)->get_sub_type();

  string datatypename = (*tdv)[0]->get_name();

  const TypeDescription *oftd = field_input_handle->get_type_description();


  CompileInfo *rval = 
    scinew CompileInfo(template_name + "." +

		       ftd->get_filename() + "." +
		       std->get_filename() + "." +
		       oftd->get_filename() + "." +

		       to_filename(datatypename) + ".",

                       base_class_name,
                       template_name,
		       ftd->get_name() + ", " +
		       std->get_name() + ", " +
		       oftd->get_name() );

  string class_declaration = string("") +
    "template <class IFIELD, class SFIELD, class OFIELD >\n" +
    "class " + template_name + " : public " + base_class_name + "T< IFIELD, SFIELD, OFIELD >\n" +
    "{\n" +
    "\n" +
    "public:\n" +
    "  virtual bool function(double fx, double fy, double fz,\n" +
    "                        double sx, double sy, double sz,\n" +
    "                        const typename IFIELD::value_type &fv,\n" +
    "                        const typename SFIELD::value_type &sv )\n" +
    "  {\n" +
    "    " + function + "\n" +
    "  }\n" +

    "\n\n" +

    "  virtual string identify()\n" +
    "  {\n" +
    "    return string(\"" + string_Cify(function) + "\");\n" +
    "  }\n" +
    "};\n";

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  rval->add_post_include(class_declaration);
  ftd->fill_compile_info(rval);
  std->fill_compile_info(rval);

  return rval;
}

} // End namespace SCIRun

