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


#include <Core/Algorithms/Fields/CalculateFieldDataCompiled.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>
#include <Core/Containers/HashTable.h>
#include <Core/Containers/StringUtil.h>
#include <iostream>

namespace SCIRun {


bool CalculateFieldDataCompiledAlgo::
CalculateFieldDataCompiled(ProgressReporter *pr,
			   vector< FieldHandle >& field_input_handles,
			   FieldHandle& field_output_handle,
			   string outputDataType,
			   string function,
			   unsigned int& count )
{
  // Check to see if all of the fields have similar meshes
  if( !SCIRunAlgo::FieldsAlgo::SimilarMeshes( pr, field_input_handles ) )
    return (false);

  FieldHandle fHandle0 = field_input_handles[0];

  // This is to catch a backwards compatibility error - this is needed
  // because a single module has replaced the other three versions.

  if (outputDataType.find("input") == 0 ) {
    outputDataType = "port 0 input";
  }

  // Change the output type to the input type.
  // Currently only the port 0 input is possible via the menu.
  for( size_t i=0; i<field_input_handles.size(); i++ ) {
    FieldHandle fHandle = field_input_handles[i];
    
    if (outputDataType ==
	string("port ") + to_string(i) + string(" input") ) {
      TypeDescription::td_vec *tdv = 
	fHandle->get_type_description(Field::FDATA_TD_E)->get_sub_type();
      outputDataType = (*tdv)[0]->get_name();
    }
  }

  const TypeDescription *ltd = fHandle0->order_type_description();

  const string oftn = 
    fHandle0->get_type_description(Field::FIELD_NAME_ONLY_E)->get_name() + "<" +
    fHandle0->get_type_description(Field::MESH_TD_E)->get_name() + ", " +
    fHandle0->get_type_description(Field::BASIS_TD_E)->get_similar_name(outputDataType, 
									0, "<", " >, ") +
    fHandle0->get_type_description(Field::FDATA_TD_E)->get_similar_name(outputDataType,
							  0, "<", " >") + " >";
  int hoffset = 0;
  Handle<CalculateFieldDataCompiledAlgo> algo;
  
  while (1) {
    CompileInfoHandle ci = get_compile_info(field_input_handles,
					    oftn, ltd,
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

  return( algo->execute( pr, field_input_handles, field_output_handle, count) );
}


CompileInfoHandle
CalculateFieldDataCompiledAlgo::
get_compile_info( vector< FieldHandle >& field_input_handles,
		  string ofieldtypename,
		  const TypeDescription *loc_td,
		  string function,
		  int hashoffset)
{
  unsigned int hashval = Hash(function, 0x7fffffff) + hashoffset;

  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  const string template_name("CalculateFieldDataCompiledInstanceAlgoT" +
			     to_string(hashval));
  static const string base_class_name("CalculateFieldDataCompiledAlgo");

//  string filename("");
  string datatypename("");
  string name("");

  for( size_t i=0; i<field_input_handles.size(); i++ ) 
  {
    FieldHandle fHandle = field_input_handles[i];

    const TypeDescription *ftd = fHandle->get_type_description();

//    filename += ftd->get_filename() + ".";
    name += ftd->get_name() + ", ";

    TypeDescription::td_vec *tdv = 
	fHandle->get_type_description(Field::FDATA_TD_E)->get_sub_type();

    datatypename += (*tdv)[0]->get_name();
  }

  CompileInfo *rval = 
    scinew CompileInfo(template_name + "." +

		       // The file name gets too long for the file
		       // system. But because the meshes should be the
		       // same the data type can be used instead.

//		       filename + "." +

		       loc_td->get_filename() + "." +

		       to_filename(datatypename) + "." +

		       to_filename(ofieldtypename) + ".",

                       base_class_name, 
                       template_name, 
                       name +
		       ofieldtypename + ", " +
		       loc_td->get_name());

  // Code for the function. This is rather ugly cause it must be built
  // based on the number of inputs. But it allows for dynamic inputs
  // which is more important and reduced module bloat.

  string class_declaration = "template < ";

  for( size_t i=0; i<field_input_handles.size(); i++ )
    class_declaration += "class IFIELD" + to_string(i) + ", ";
  
  class_declaration += string("") + 
    "class OFIELD, class LOC >\n" +
    "class " + template_name + " : public " + base_class_name + "\n" +
    "{\n" +
    "\n" +
    "public:\n" +
    "  virtual bool execute(ProgressReporter *pr,\n" +
    "                       vector<FieldHandle>& field_input_handles,\n" +
    "                       FieldHandle& field_output_handle,\n" +
    "                       unsigned int& count)\n" +
    "  {\n";
  
  for( size_t i=0; i<field_input_handles.size(); i++ )
    class_declaration += "    IFIELD" + to_string(i) +
      " *iField" + to_string(i) +
      " = dynamic_cast<IFIELD" + to_string(i) +
      " *>(field_input_handles[" + to_string(i) +
      "].get_rep());\n";

  class_declaration += string("") + 
    "\n" +
    "    typename IFIELD0::mesh_handle_type iMesh = iField0->get_typed_mesh();\n" +
    "\n" +
    "    OFIELD *oField;\n" +
    "    bool valid_output_handle = (count && field_output_handle.get_rep());\n" +

    "\n" +
    "    if( valid_output_handle ) \n" +
    "    {\n" +
    "      oField = dynamic_cast<OFIELD *> (field_output_handle->clone());\n" +
    "      oField->thaw();\n" +
    "\n" +
    "    } else {\n" +
    "      oField = scinew OFIELD(iMesh);\n" +
    "    }\n" +
    "\n" +
    "    typename LOC::iterator ibi, iei;\n" +
    "    iMesh->begin(ibi);\n" +
    "    iMesh->end(iei);\n" +
    "\n" +
    "    while (ibi != iei)\n" +
    "    {\n" +
    "      Point p;\n" +
    "      iMesh->get_center(p, *ibi);\n" +
    "\n";

  for( unsigned int i=0; i<field_input_handles.size(); i++ )
    class_declaration +=
      "      typename IFIELD" + to_string(i) +
      "::value_type val" + to_string(i) + ";\n" +
      "      iField" + to_string(i) +
      "->value(val" + to_string(i) +
      ", *ibi);\n" +
      "\n";

  class_declaration += string("") + 
    "      typename OFIELD::value_type result;\n" +
    "\n" +
    "      if( valid_output_handle )\n" +
    "        oField->value(result, *ibi);\n" +
    "      else\n" +
    "        result = typename OFIELD::value_type();\n" +
    "\n" +
    "      function(result, p.x(), p.y(), p.z(), ";

  for( unsigned int i=0; i<field_input_handles.size(); i++ )
    class_declaration += "val" + to_string(i) + ", "; 

  class_declaration += string("") + 
    "count );\n" +
    "\n" +
    "      oField->set_value(result, *ibi);\n" +
    "\n" +
    "      ++ibi;\n" +
    "    }\n" +
    "\n" +
    "    field_output_handle = (FieldHandle) oField;\n" +
    "\n" +
    "    count++;\n" +
    "\n" +
    "    return true;\n" +
    "  }\n" +
    "\n\n" +
    "  virtual void function(typename OFIELD::value_type &result,\n" +
    "                        double x, double y, double z,\n";

  for( size_t i=0; i<field_input_handles.size(); i++ )
    class_declaration +=  string("") +
      "                        const typename IFIELD" + to_string(i) +
      "::value_type &v" + to_string(i) + ",\n";

  class_declaration += string("") + 
    "unsigned int count)\n" +
    "  {\n";

  if( field_input_handles.size() == 1 ) {
    class_declaration += string("") + 
      "    // This for backwards compatibility\n" +
      "    const typename IFIELD0::value_type &v = v0;\n";
  }

  class_declaration += string("") + 
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
  for( unsigned int i=0; i<field_input_handles.size(); i++ ) {
    const TypeDescription *ftd = field_input_handles[i]->get_type_description();
    ftd->fill_compile_info(rval);
  }

  rval->add_data_include("Core/Geometry/Vector.h");
  rval->add_data_include("Core/Geometry/Tensor.h");
  return rval;
}

} // End namespace SCIRun
