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
 *  CalculateGradients.cc:  Unfinished modules
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   March 2001
 *
 *  Copyright (C) 2001 SCI Group
 */

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Containers/Handle.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/VField.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/FieldInformation.h>


namespace SCIRun {

class CalculateGradients : public Module
{
public:
  CalculateGradients(GuiContext* ctx);
  virtual void execute();

};


DECLARE_MAKER(CalculateGradients)

CalculateGradients::CalculateGradients(GuiContext* ctx)
  : Module("CalculateGradients", ctx, Filter, "ChangeFieldData", "SCIRun")
{
}

void
CalculateGradients::execute()
{
  FieldHandle input;

  get_input_handle( "Input Field", input, true );

  if( inputs_changed_ || !oport_cached("Output CalculateGradients") )
  {
    FieldInformation fi(input);
    
    if ((!(fi.is_scalar()))||(!(fi.field_basis_order() > 0)))
    {
      error( "This module only works on scalar data that is located on the nodes of the mesh");
      return;   
    }
    
    fi.make_constantdata();
    fi.make_vector();
    FieldHandle output = CreateField(fi,input->mesh());
    if (output.get_rep() == 0)
     {
      error( "CalculateGradient: Could not create output field");
      return;
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
  
    // Send the data downstream
    send_output_handle( "Output CalculateGradients", output,true);
  }
}

} // End namespace SCIRun
