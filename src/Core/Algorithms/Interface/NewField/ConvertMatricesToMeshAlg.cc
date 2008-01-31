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
#include <Core/Algorithms/Interface/NewField/ConvertMatricesToMeshAlg.h>
//#include <Dataflow/Modules/Fields/ConvertMatricesToMesh.h>

namespace SCIRun {

ConvertMatricesToMeshAlg* get_convert_matrices_to_mesh_alg()
{
  return new ConvertMatricesToMeshAlg();
}

//! Algorithm Interface.
size_t
ConvertMatricesToMeshAlg::execute(size_t Mesh_Elements, size_t Mesh_Positions,
				  size_t Mesh_Normals)
{
  //DataManager *dm = DataManager::get_dm();
  std::cerr << "WARNING: convert_matrices_to_mesh not implemented." << std::endl;

  //   ProgressReporter pr;
  //   MatrixHandle elementshandle;
  //   MatrixHandle positionshandle;
  //   MatrixHandle normalshandle;

  //   if (Mesh_Elements == 0) return false;
  //   elementshandle = mats_[Mesh_Elements];
  //   if (Mesh_Elements == 0) return false;
  //   positionshandle = mats_[Mesh_Positions];
  //   if (!Mesh_Normals == 0)
  //     normalshandle = mats_[Mesh_Normals];
  //   else
  //   {
  //     cerr << "No input normals connected, not used.";
  //   }

  //   CompileInfoHandle ci =
  //     ConvertMatricesToMeshAlgo::get_compile_info(fieldbasetype,
  //                                                 datatype);
  //   Handle<ConvertMatricesToMeshAlgo> algo;
  //   if (!DynamicCompilation::compile(ci, algo, false, &pr))
  //   {
  //     cerr << "Could not compile algorithm.";
  //     return false;
  //   }

  //   FieldHandle result_field =
  //     algo->execute(&pr, elementshandle, positionshandle,
  // 		  normalshandle, 1);

  //   if (!result_field.get_rep())
  //   {
  //     return false;
  //   }

  //   output = add_field(result_field);
  return 0;
}

} //end namespace SCIRun

