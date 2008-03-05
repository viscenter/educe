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
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateGradientsAlg.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/VField.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRun {

CalculateGradientsAlg* get_calculate_gradients_alg()
{
  return new CalculateGradientsAlg();
}

//! Algorithm Interface.
  //! Algorithm Interface.
size_t
CalculateGradientsAlg::execute(size_t field_id)
{
  DataManager *dm = DataManager::get_dm();
  FieldHandle input;

  input = dm->get_field(field_id);
  // get_input_handle( "Input Field", input, true );

  FieldInformation fi(input);
    
  if ((!(fi.is_scalar()))||(!(fi.field_basis_order() > 0)))
  {
    if (progress_) {
      progress_->error( "This module only works on scalar data that is located on the nodes of the mesh");
    }
    return 0;   
  }
    
  fi.make_constantdata();
  fi.make_vector();
  FieldHandle output = CreateField(fi,input->mesh());
  if (output.get_rep() == 0)
  {
    if (progress_) {
      progress_->error( "CalculateGradient: Could not create output field");
    }
    return 0;
  }

  VMesh* mesh = output->vmesh();
  VField* field = output->vfield();
  VField* ifield = input->vfield();
    
  // Calculate center coordinates
  VMesh::coords_array_type ca;
  mesh->get_element_vertices(ca);
    
  VMesh::coords_type center;
  (ca[0].size());
  for (size_t k=0; k<ca.size(); k++) 
  {
    for (size_t p=0; p<center.size();p++) center[p] += ca[k][p];
  }
  for (size_t p=0; p<center.size();p++) center[p] *= 1.0/static_cast<double>(ca.size());
    
  field->resize_values();
    
  VMesh::Elem::iterator it, it_end;
  mesh->begin(it);
  mesh->end(it_end);
  
  while (it != it_end)
  {
    StackVector<double,3> grad;
    ifield->gradient(grad,center,*it);
      
    Vector v(grad[0],grad[1],grad[2]);
    field->set_value(v,*it);
    ++it;
  }
  
  // Store the data in the DataManager.
  return dm->add_field(output);
  //send_output_handle( "Output CalculateGradients", output,true);
}

} //end namespace SCIRun

