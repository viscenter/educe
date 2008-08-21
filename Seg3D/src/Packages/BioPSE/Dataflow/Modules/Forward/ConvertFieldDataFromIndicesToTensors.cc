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
 *  ConvertFieldDataFromIndicesToTensors: Change a Field of indices (ints) into a Field or Tensors,
 *                      where the Tensor values are looked up in the
 *                      conductivity_table for each index
 *
 *  Written by:
 *   David Weinstein
 *   Department of Computer Science
 *   University of Utah
 *   December 2002
 *
 *  Copyright (C) 2002 SCI Group
 */

#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Geometry/Tensor.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>

#include <vector>
#include <string>

namespace BioPSE {

using namespace SCIRun;

class ConvertFieldDataFromIndicesToTensors : public Module {
public:
  ConvertFieldDataFromIndicesToTensors(GuiContext *context);
  virtual ~ConvertFieldDataFromIndicesToTensors() {}
  
  virtual void execute();
};


DECLARE_MAKER(ConvertFieldDataFromIndicesToTensors)

ConvertFieldDataFromIndicesToTensors::ConvertFieldDataFromIndicesToTensors(GuiContext *context)
  : Module("ConvertFieldDataFromIndicesToTensors", context, Filter, "Forward", "BioPSE")
{
}

void
ConvertFieldDataFromIndicesToTensors::execute()
{
  FieldHandle ifieldH;
  get_input_handle("IndexField", ifieldH);

  std::vector<std::pair<string, Tensor> > conds;
  if (!ifieldH->get_property("conductivity_table", conds)) 
  {
    error("Error - input field does not have a conductivity_table property.");
    return;
  }

  if (!(ifieldH->has_virtual_interface()))
  {
    error("This module only needs a field with a virtual interface");
    return;
  }

  VField* src = ifieldH->vfield();
  
  FieldInformation fi(ifieldH);
  fi.make_tensor();
  
  FieldHandle ofieldH = CreateField(fi,ifieldH->mesh());
  VField* dst = ofieldH->vfield();

  src->get_property("conductivity_table", conds);

  VField::size_type num_values = dst->num_values();
  Tensor null_tensor(0);
  for (VField::index_type idx=0; idx<num_values;idx++)
  {
    int index;
    src->get_value(index,idx);
    if (idx >= 0 && idx < conds.size())
    {
      dst->set_value(conds[index].second,idx);
    }
    else
    {
      dst->set_value(null_tensor,idx);
    }
  }
  
  send_output_handle("TensorField", ofieldH);
}

} // End namespace BioPSE
