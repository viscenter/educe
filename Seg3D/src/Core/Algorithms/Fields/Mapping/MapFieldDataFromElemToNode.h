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


#ifndef CORE_ALGORITHMS_FIELDS_FIELDDATAELEMTONODE_H
#define CORE_ALGORITHMS_FIELDS_FIELDDATAELEMTONODE_H 1

//! STL datatypes needed
#include <algorithm>

//! Datatypes used
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>

//! Base for algorithm
#include <Core/Algorithms/Util/AlgoBase.h>

//! for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SCISHARE MapFieldDataFromElemToNodeAlgo : public AlgoBase
{
  public:
    // Algorithm defaults
    MapFieldDataFromElemToNodeAlgo()
    {
      //! Add option for how to do map data from elements to node
      add_option("method","average","interpolate|average|min|max|sum|median");
    }

  public:
    // Algorithm Functions
    bool run(FieldHandle input, FieldHandle& output);   
};

} // namespace SCIRunAlgo

#endif
