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
 *  ClipFieldByFunction.cc:  Clip out parts of a field.
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   March 2001
 *
 *  Copyright (C) 2001 SCI Group
 */

#include <Core/Containers/StringUtil.h>
#include <Core/Algorithms/Fields/ClipFieldByFunction.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

#include <sci_hash_map.h>

#include <iostream>


namespace SCIRun {


bool ClipFieldByFunctionAlgo::
ClipFieldByFunction(ProgressReporter *pr,
		    vector< FieldHandle >& field_input_handles,
		    FieldHandle& field_output_handle,
		    MatrixHandle& matrix_output_handle,
		    string mode,
		    string function )
{
  // Check to see if all of the fields have similar meshes
  if( !SCIRunAlgo::FieldsAlgo::SimilarMeshes( pr, field_input_handles ) )
    return (false);

  if (!field_input_handles[0]->mesh()->is_editable()) {
    pr->error("Not an editable mesh type.");
    pr->error("(Try passing Field through an Unstructure module first).");
    return (false);
  }

  int cMode = 0;
  if (mode == "cell")               cMode =  0;
  else if (mode == "onenode")       cMode =  1;
  else if (mode == "majoritynodes") cMode =  2;
  else if (mode == "allnodes")      cMode = -1;

  if (!(field_input_handles[0]->basis_order() == 0 && cMode == 0 ||
	field_input_handles[0]->basis_order() == 1 && cMode != 0) &&
      field_input_handles[0]->mesh()->dimensionality() > 0)
  {
    pr->warning("Basis doesn't match clip location, value will always be zero.");
  }

  int hoffset = 0;
  Handle<ClipFieldByFunctionAlgo> algo;
  
  while (1) {
    CompileInfoHandle ci = get_compile_info(field_input_handles,
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
			 field_input_handles,
			 field_output_handle,
			 matrix_output_handle,
			 cMode) );
}


CompileInfoHandle
ClipFieldByFunctionAlgo::
get_compile_info( vector<FieldHandle>& field_input_handles,
		  string function,
		  int hashoffset)
{
#if defined(__ECC) || defined(_MSC_VER)
  hash_compare<const char*> H;
#else
  hash<const char*> H;
#endif
  unsigned int hashval = H(function.c_str()) + hashoffset;

  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  const string template_name("ClipFieldByFunctionInstanceAlgoT" +
			     to_string(hashval));
  static const string base_class_name("ClipFieldByFunctionAlgo");

//  string filename("");
  string datatypename("");
  string name("");

  for( unsigned int i=0; i<field_input_handles.size(); i++ ) {
    FieldHandle fHandle = field_input_handles[i];

    const TypeDescription *ftd = fHandle->get_type_description();

//    filename += ftd->get_filename() + ".";
    name += ftd->get_name() + ", ";

    TypeDescription::td_vec *tdv = 
	fHandle->get_type_description(Field::FDATA_TD_E)->get_sub_type();

    datatypename += (*tdv)[0]->get_name();
  }

  const TypeDescription *oftd = field_input_handles[0]->get_type_description();

  CompileInfo *rval = 
    new CompileInfo(template_name + "." +

		       // The file name gets too long and because a hash
		       // number is used there should not be a conflict
		       // with other instances so skip adding it.

//		       filename + "." +

		       oftd->get_filename() + "." +

		       to_filename(datatypename) + ".",

                       base_class_name,
                       template_name,
                       name +
		       oftd->get_name() );

  // Add in the include path to compile this obj
  string class_declaration = "template < ";

  for( unsigned int i=0; i<field_input_handles.size(); i++ )
    class_declaration += "class IFIELD" + to_string(i) + ", ";
  
  class_declaration += string("") + 
    "class OFIELD >\n" +
    "class " + template_name + " : public " + base_class_name + "\n" +
    "{\n" +
    "\n" +
    "public:\n" +
    "  virtual bool execute(ProgressReporter *pr,\n" +
    "                       vector< FieldHandle > field_input_handles,\n" +
    "                       FieldHandle& field_output_handle,\n" +
    "                       MatrixHandle& matrix_output_handle,\n" +
    "                       int mode )\n" +
    "  {\n";
  
  for( unsigned int i=0; i<field_input_handles.size(); i++ )
    class_declaration += "    IFIELD" + to_string(i) +
      " *iField" + to_string(i) +
      " = dynamic_cast<IFIELD" + to_string(i) +
      " *>(field_input_handles[" + to_string(i) +
      "].get_rep());\n";

  class_declaration += string("") + 
    "\n" +
    "    typename IFIELD0::mesh_type *iMesh =\n" +
    "      dynamic_cast<typename IFIELD0::mesh_type *>(iField0->mesh().get_rep());\n" +
    "\n" +
    "    typename OFIELD::mesh_type *oMesh = new typename OFIELD::mesh_type();\n" +
    "    oMesh->copy_properties(iMesh);\n" +
    "\n" +
    "#ifdef HAVE_HASH_MAP\n" +
    "\n" +
    "#if defined(__ECC) || defined(_MSC_VER)\n" +
    "    typedef hash_map<unsigned int, typename IFIELD0::mesh_type::Node::index_type> hash_type;\n" +
    "#else\n" +
    "    typedef hash_map<unsigned int,\n" +
    "      typename IFIELD0::mesh_type::Node::index_type,\n" +
    "      hash<unsigned int>,\n" +
    "      equal_to<unsigned int> > hash_type;\n" +
    "#endif\n" +
    "\n" +
    "#else\n" +
    "    typedef map<unsigned int,\n" +
    "      typename IFIELD0::mesh_type::Node::index_type,\n" +
    "      less<unsigned int> > hash_type;\n" +
    "#endif\n" +
    "\n" +
    "    hash_type nodemap;\n" +
    "    \n" +
    "    vector<typename IFIELD0::mesh_type::Elem::index_type> elemmap;\n" +
    "\n" +
    "    typedef typename IFIELD0::mesh_type mesh_type_val;\n" +
    "    const bool element_data_valid =\n" +
    "      (iField0->order_type_description()->get_name() == \n" +
    "       mesh_type_val::elem_type_description()->get_name());\n" +
    "\n" +
    "    const bool node_data_valid =\n" +
    "      (iField0->basis_order() == 1 || iMesh->dimensionality() == 0);\n" +
    "\n" +
    "    typename IFIELD0::mesh_type::Elem::size_type prsizetmp;\n" +
    "    iMesh->size(prsizetmp);\n" +
    "    const unsigned int prsize = (unsigned int)prsizetmp;\n" +
    "    unsigned int prcounter = 0;\n" +
    "\n" +
    "    typename IFIELD0::mesh_type::Elem::iterator ibi, iei;\n" +
    "    iMesh->begin(ibi);\n" +
    "    iMesh->end(iei);\n" +
    "\n" +
    "    while (ibi != iei)\n" +
    "    {\n" +
    "      pr->update_progress(prcounter, prsize);\n" +
    "\n" +
    "      bool inside = false;\n" +
    "\n" +
    "      if (mode == 0)\n" +
    "      {\n" +
    "        Point p;\n" +
    "        iMesh->get_center(p, *ibi);\n" +
    "\n";

  for( unsigned int i=0; i<field_input_handles.size(); i++ )
    class_declaration +=
      "        typename IFIELD" + to_string(i) +
      "::value_type val" + to_string(i) +
      " = typename IFIELD" + to_string(i) +
      "::value_type();\n" +
      "        if (element_data_valid) iField" + to_string(i) +
      "->value(val" + to_string(i) +
      ", *ibi);\n" +
      "\n";

  class_declaration += string("") +
    "        inside = function(p.x(), p.y(), p.z()";

  for( unsigned int i=0; i<field_input_handles.size(); i++ )
    class_declaration += ", val" + to_string(i);

  class_declaration += string("") + 
    ");\n" +
    "      }\n" +
    "      else if (mode == 1)\n" +
    "      {\n" +
    "        typename IFIELD0::mesh_type::Node::array_type onodes;\n" +
    "        iMesh->get_nodes(onodes, *ibi);\n" +
    "\n" +
    "        inside = false;\n" +
    "        int counter = 0;\n" +
    "        for (unsigned int i=0; i<onodes.size(); i++)\n" +
    "        {\n" +
    "          Point p;\n" +
    "          iMesh->get_center(p, onodes[i]);\n" +
    "\n";

  for( unsigned int i=0; i<field_input_handles.size(); i++ )
    class_declaration +=
      "          typename IFIELD" + to_string(i) +
      "::value_type val" + to_string(i) +
      " = typename IFIELD" + to_string(i) +
      "::value_type();\n" +
      "          if (node_data_valid) iField" + to_string(i) +
      "->value(val" + to_string(i) +
      ", onodes[i]);\n" +
      "\n";

  class_declaration += string("") +
    "          if (function(p.x(), p.y(), p.z()";

  for( unsigned int i=0; i<field_input_handles.size(); i++ )
    class_declaration += ", val" + to_string(i);

  class_declaration += string("") + 
    "))\n" +
    "          {\n" +
    "            counter++;\n" +
    "            if (counter >= mode)\n" +
    "            {\n" +
    "              inside = true;\n" +
    "              break;\n" +
    "            }\n" +
    "          }\n" +
    "        }\n" +
    "      }\n" +
    "      else if (mode == 2)\n" +
    "      {\n" +
    "        typename IFIELD0::mesh_type::Node::array_type onodes;\n" +
    "        iMesh->get_nodes(onodes, *ibi);\n" +
    "\n" +
    "        unsigned sz = onodes.size() / 2;\n" +
    "\n" +
    "        inside = false;\n" +
    "        unsigned counter = 0;\n" +
    "        for (unsigned int i = 0; i < onodes.size(); i++)\n" +
    "        {\n" +
    "          Point p;\n" +
    "          iMesh->get_center(p, onodes[i]);\n" +
    "\n";

  for( unsigned int i=0; i<field_input_handles.size(); i++ )
    class_declaration +=
      "          typename IFIELD" + to_string(i) +
      "::value_type val" + to_string(i) +
      " = typename IFIELD" + to_string(i) +
      "::value_type();\n" +
      "          if (node_data_valid) iField" + to_string(i) +
      "->value(val" + to_string(i) +
      ", onodes[i]);\n" +
      "\n";

  class_declaration += string("") +
    "          if (function(p.x(), p.y(), p.z()";

  for( unsigned int i=0; i<field_input_handles.size(); i++ )
    class_declaration += ", val" + to_string(i);

  class_declaration += string("") + 
    "))\n" +
    "          {\n" +
    "            counter++;\n" +
    "            if (counter >= sz)\n" +
    "            {\n" +
    "              inside = true;\n" +
    "              break;\n" +
    "            }\n" +
    "          }\n" +
    "        }\n" +
    "      }\n" +
    "      else\n" +
    "      {\n" +
    "        typename IFIELD0::mesh_type::Node::array_type onodes;\n" +
    "        iMesh->get_nodes(onodes, *ibi);\n" +
    "        inside = true;\n" +
    "        for (unsigned int i = 0; i < onodes.size(); i++)\n" +
    "        {\n" +
    "          Point p;\n" +
    "          iMesh->get_center(p, onodes[i]);\n" +
    "\n";

  for( unsigned int i=0; i<field_input_handles.size(); i++ )
    class_declaration +=
      "          typename IFIELD" + to_string(i) +
      "::value_type val" + to_string(i) +
      " = typename IFIELD" + to_string(i) +
      "::value_type();\n" +
      "          if (node_data_valid) iField" + to_string(i) +
      "->value(val" + to_string(i) +
      ", onodes[i]);\n" +
      "\n";

  class_declaration += string("") +
    "          if (!function(p.x(), p.y(), p.z()";

  for( unsigned int i=0; i<field_input_handles.size(); i++ )
    class_declaration += ", val" + to_string(i);

  class_declaration += string("") + 
    "))\n" +
    "          {\n" +
    "            inside = false;\n" +
    "            break;\n" +
    "          }\n" +
    "        }\n" +
    "      }\n" +
    "\n" +
    "      if (inside)\n" +
    "      {\n" +
    "        typename IFIELD0::mesh_type::Node::array_type onodes;\n" +
    "        iMesh->get_nodes(onodes, *ibi);\n" +
    "\n" +
    "        typename IFIELD0::mesh_type::Node::array_type nnodes(onodes.size());\n" +
    "\n" +
    "        for (unsigned int i = 0; i<onodes.size(); i++)\n" +
    "        {\n" +
    "          if (nodemap.find((unsigned int)onodes[i]) == nodemap.end())\n" +
    "          {\n" +
    "            Point np;\n" +
    "            iMesh->get_center(np, onodes[i]);\n" +
    "\n" +
    "            const typename OFIELD::mesh_type::Node::index_type nodeindex =\n" +
    "              oMesh->add_point(np);\n" +
    "\n" +
    "            nodemap[(unsigned int)onodes[i]] = nodeindex;\n" +
    "            nnodes[i] = nodeindex;\n" +
    "          }\n" +
    "          else\n" +
    "          {\n" +
    "            nnodes[i] = nodemap[(unsigned int)onodes[i]];\n" +
    "          }\n" +
    "        }\n" +
    "\n" +
    "        oMesh->add_elem(nnodes);\n" +
    "        elemmap.push_back(*ibi);\n" +
    "      }\n" +
    "      \n" +
    "      ++ibi;\n" +
    "    }\n" +
    "\n" +
    "    OFIELD *oField = new OFIELD(oMesh);\n" +
    "    oField->copy_properties(field_input_handles[0].get_rep());\n" +
    "\n" +
    "    if (node_data_valid)\n" +
    "    {\n" +
    "      IFIELD0 *iField =\n" +
    "        dynamic_cast<IFIELD0 *>(field_input_handles[0].get_rep());\n" +
    "\n" +
    "      typename hash_type::iterator hitr = nodemap.begin();\n" +
    "\n" +
    "      const size_type nrows = nodemap.size();\n" +
    "      const size_type ncols = iField->fdata().size();\n" +
    "      index_type *rr = new index_type[nrows+1];\n" +
    "      index_type *cc = new index_type[nrows];\n" +
    "      double *d = new double[nrows];\n" +
    "\n" +
    "      typedef typename IFIELD0::mesh_type mesh_type_val;\n" +
    "\n" +
    "      while (hitr != nodemap.end())\n" +
    "      {\n" +
    "        typename IFIELD0::value_type val;\n" +
    "\n" +
    "        iField0->value(val, (typename IFIELD0::mesh_type::Node::index_type)((*hitr).first));\n" +
    "        oField->set_value(val, (typename IFIELD0::mesh_type::Node::index_type)((*hitr).second));\n" +
    "\n" +
    "        cc[(*hitr).second] = (*hitr).first;\n" +
    "\n" +
    "        ++hitr;\n" +
    "      }\n" +
    "\n" +
    "      for (unsigned int i=0; i<nrows; i++)\n" +
    "      {\n" +
    "        rr[i] = i;\n" +
    "        d[i] = 1.0;\n" +
    "      }\n" +
    "      rr[nrows] = nrows;\n" +
    "\n" +
    "      matrix_output_handle =\n" +
    "        new SparseRowMatrix(nrows, ncols, rr, cc, nrows, d);\n" +
    "    }\n" +
    "    else if (field_input_handles[0]->order_type_description()->get_name() ==\n" +
    "	   mesh_type_val::elem_type_description()->get_name())\n" +
    "    {\n" +
    "      const size_type nrows = elemmap.size();\n" +
    "      const size_type ncols = iField0->fdata().size();\n" +
    "      index_type *rr = new index_type[nrows+1];\n" +
    "      index_type *cc = new index_type[nrows];\n" +
    "      double *d = new double[nrows];\n" +
    "\n" +
    "      for (unsigned int i=0; i<elemmap.size(); i++)\n" +
    "      {\n" +
    "        typename IFIELD0::value_type val;\n" +
    "\n" +
    "        iField0->value(val, (typename IFIELD0::mesh_type::Elem::index_type) elemmap[i]);\n" +
    "        oField->set_value(val, (typename IFIELD0::mesh_type::Elem::index_type) i);\n" +
    "\n" +
    "        cc[i] = elemmap[i];\n" +
    "      }\n" +
    "\n" +
    "      for (unsigned int i=0; i<nrows; i++)\n" +
    "      {\n" +
    "        rr[i] = i;\n" +
    "        d[i] = 1.0;\n" +
    "      }\n" +
    "      rr[nrows] = nrows;\n" +
    "\n" +
    "      matrix_output_handle =\n" +
    "        new SparseRowMatrix(nrows, ncols, rr, cc, nrows, d);\n" +
    "    }\n" +
    "    else\n" +
    "    {\n" +
    "      pr->warning(\"Unable to copy data at this field data location.\");\n" +
    "      pr->warning(\"No interpolant computed for field data location.\");\n" +
    "      matrix_output_handle = 0;\n" +
    "    }\n" +
    "  \n" +
    "    field_output_handle = (FieldHandle) oField;\n" +
    "  \n" +
    "    return true;\n" +
    "\n" +
    "  }\n" +
    "\n\n" +
    "  virtual bool function(double x, double y, double z";

  for( unsigned int i=0; i<field_input_handles.size(); i++ )
    class_declaration +=  string("") +
      ",\n" + 
      "                        const typename IFIELD" + to_string(i) +
      "::value_type &v" + to_string(i);

  class_declaration += string("") + 
    ")\n" +
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

  rval->add_include(include_path);
  rval->add_post_include(class_declaration);
  for( unsigned int i=0; i<field_input_handles.size(); i++ ) {
    const TypeDescription *ftd = field_input_handles[i]->get_type_description();
    ftd->fill_compile_info(rval);
  }

  return rval;
}

} // End namespace SCIRun

