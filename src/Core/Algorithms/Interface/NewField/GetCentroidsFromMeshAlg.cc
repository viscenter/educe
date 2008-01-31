//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2007 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  License for the specific language governing rights and limitations under
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#include <iostream>
#include <Core/Events/DataManager.h>
#include <Core/Algorithms/Interface/NewField/GetCentroidsFromMeshAlg.h>
//#include <Dataflow/Modules/Fields/GetCentroidsFromMesh.h>

namespace SCIRun {

GetCentroidsFromMeshAlg* get_get_centroids_from_mesh_alg()
{
  return new GetCentroidsFromMeshAlg();
}

//! Algorithm Interface.
size_t
GetCentroidsFromMeshAlg::execute(size_t field_id)
{
  //DataManager *dm = DataManager::get_dm();
  std::cerr << "WARNING: get_centroids_from_mesh not implemented." 
	    << std::endl;
  //   FieldHandle ifieldhandle = fields_[fld_id];
  //   ProgressReporter pr;


  //   const TypeDescription *ftd = ifieldhandle->get_type_description();
  //   CompileInfoHandle ci = GetCentroidsFromMeshAlgo::get_compile_info(ftd);
  //   Handle<GetCentroidsFromMeshAlgo> algo;
  //   if (!DynamicCompilation::compile(ci, algo, &pr)) return false;

  //   FieldHandle ofieldhandle(algo->execute(&pr, ifieldhandle));

  //   output = add_field(ofieldhandle);
  return 0;
}

} //end namespace SCIRun

