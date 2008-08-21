/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2004 Scientific Coutputputing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without linputitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whoutput the
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

#include <Core/Algorithms/Math/ReorderMatrix/ReorderMatrix.h>
#include <Core/Algorithms/Math/ReorderMatrix/ReverseCutHillMcKee.h>
#include <Core/Algorithms/Math/ReorderMatrix/CutHillMcKee.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool 
ReorderMatrixAlgo::run(MatrixHandle input, 
                       MatrixHandle& output,
                       MatrixHandle& mapping)
{
  std::string method = get_option("method");
  if (method == "cuthillmckee")
  {
    CutHillMcKeeAlgo algo;
    algo.set_progress_reporter(this);
    algo.set_bool("build_mapping",get_bool("build_mapping"));
    return(algo.run(input,output,mapping));
  }
  else if (method == "reversecuthillmckee")
  {
    ReverseCutHillMcKeeAlgo algo;
    algo.set_progress_reporter(this);
    algo.set_bool("build_mapping",get_bool("build_mapping"));
    return(algo.run(input,output,mapping));
  }
  
  return (false);
}

} // end namespace SCIRunAlgo
