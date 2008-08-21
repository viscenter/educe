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

#include <Core/Datatypes/Field.h>
#include <Core/Algorithms/Fields/FieldData/SetFieldDataToConstantValue.h>

#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool 
SetFieldDataToConstantValueAlgo::
run(FieldHandle input, FieldHandle& output)
{
  algo_start("SetFieldDataToConstantValue",false);
  if (!(input.get_rep()))
  {
    error("No input field was provided");
    algo_end(); return (false);  
  }

  FieldInformation fi(input);

  VMesh* imesh = input->vmesh();
  VMesh::size_type numnodes = imesh->num_nodes();
  VMesh::size_type numelems = imesh->num_elems();


  std::string data_type = get_option("data_type");
  if (data_type != "same as input")
  {
    fi.set_data_type(data_type);
  }
  else if (!(fi.is_scalar()))
  {
    fi.make_double();
  }

  std::string basis_order = get_option("basis_order");
  if (basis_order != "same as input")
  {
    fi.set_basis_type(basis_order);
  }

  output = CreateField(fi,input->mesh());
  
  if (output.get_rep() == 0)
  {
    error("Could not allocate output field");
    algo_end(); return (false);
  }

  double new_value = get_scalar("value");
  
  output->vfield()->resize_values();
  output->vfield()->set_all_values(new_value);
  
  algo_end();
  return (true);
}

} // namespace SCIRunAlgo

