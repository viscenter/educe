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
#include <Core/Algorithms/Interface/ChangeMesh/ConvertMeshToPointCloudAlg.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace SCIRun {

ConvertMeshToPointCloudAlg* get_convert_mesh_to_point_cloud_alg()
{
  return new ConvertMeshToPointCloudAlg();
}

//! Algorithm Interface.
size_t
ConvertMeshToPointCloudAlg::execute(size_t Field)
{
  DataManager *dm = DataManager::get_dm();
  FieldHandle ifield, ofield;
  if (Field == 0) { return 0; }
  ifield = dm->get_field(Field);

  SCIRunAlgo::FieldsAlgo algo(get_progress_reporter());
  if (! algo.ConvertMeshToPointCloud(ifield, ofield, 
				     get_p_datalocation())) 
  {
    return 0;
  }

  return dm->add_field(ofield);
}

} //end namespace SCIRun

