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

#include <Core/Algorithms/Fields/ClearAndChangeFieldBasis.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool ClearAndChangeFieldBasisAlgo::ClearAndChangeFieldBasis(ProgressReporter *pr, FieldHandle input, FieldHandle& output,std::string newbasis)
{
  if (input.get_rep() == 0)
  {
    pr->error("ClearAndChangeFieldBasis: No input field");
    return (false);
  }

  // no precompiled version available, so compile one

  FieldInformation fi(input);
  FieldInformation fo(input);

  if (newbasis == "NoData") fo.make_nodata();
  else if (newbasis == "Constant") fo.make_constantdata();
  else if (newbasis == "Linear") fo.make_lineardata();
  else if (newbasis == "Quadratic") fo.make_quadraticdata();
  else if (newbasis == "Cubic") fo.make_cubicdata();
  else
  {
    pr->error("ClearAndChangeFieldBasis: Unknown basis, basis needs to be NoData, Constant, Linear, Quadratic, or Cubic");
    return (false);  
  }
  
  output = CreateField(fo,input->mesh());
  VField* vfield = output->vfield();
  
  //! these calls resize non linear data automatically
  vfield->resize_values();
  vfield->clear_all_values();

  //! Copy property manager options
	output->copy_properties(input.get_rep());  
  return (true);
}

} // End namespace SCIRunAlgo

