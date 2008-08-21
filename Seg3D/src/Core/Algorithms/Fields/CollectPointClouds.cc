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
 *  CollectPointClouds.cc:  Collect a (time) series of point clouds
 *  turing them into a set of curves.
 *
 *  Written by:
 *   Allen R. Sanderson
 *   SCI Institute
 *   University of Utah
 *   July 2007
 *
 *  Copyright (C) 2007 SCI Institute
 */

#include <Core/Algorithms/Fields/CollectPointClouds.h>

namespace SCIRun {

bool CollectPointCloudsAlgo::
CollectPointClouds(ProgressReporter *pr,
		   FieldHandle& field_input_handle,
		   FieldHandle& field_output_handle,
		   int num_fields,
		   int num_nodes,
		   unsigned int& count)
{
  static const string template_name("CollectPointCloudsAlgoT");
  static const string base_class_name("CollectPointCloudsAlgo");

  if (field_input_handle.get_rep() == 0)
  {
    pr->error("CollectPointClouds: No input field");
    return (false);
  }

  FieldInformation fi(field_input_handle);
  FieldInformation fo(field_input_handle);
  
  // If the input mesh is not a PointCloud report an error.
  if (fi.get_mesh_type() != "PointCloudMesh")
  {
    pr->error("Mesh is not a PointCloud");
    return (false);
  }
  
  // Define the output type of the data
  fo.set_mesh_type("CurveMesh");
  fo.set_mesh_basis_type("CrvLinearLgn");
  fo.set_basis_type("CrvLinearLgn");
  
  CompileInfoHandle ci = new CompileInfo(
    template_name + "." +
    fi.get_field_filename() + "." + 
    fo.get_field_filename() + ".",
    base_class_name, 
    template_name, 
    fi.get_field_name() + "," + fo.get_field_name());

  ci->add_include(TypeDescription::cc_to_h(__FILE__));

  ci->add_namespace("SCIRun");
  
  fi.fill_compile_info(ci);
  fo.fill_compile_info(ci);
  
  if (dynamic_cast<RegressionReporter *>(pr)) ci->keep_library_ = false;    
  
  Handle<CollectPointCloudsAlgo> algo;

  if(!(SCIRun::DynamicCompilation::compile(ci,algo,pr)))
  {
    pr->compile_error(ci->filename_);
    
    SCIRun::DynamicLoader::scirun_loader().cleanup_failed_compile(ci);  
    return(false);
  }

  return(algo->CollectPointClouds(pr,
				  field_input_handle,
				  field_output_handle,
				  num_fields,
				  num_nodes,
				  count));
}

} // End namespace SCIRunAlgo
