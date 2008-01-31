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


//    File   : ClipField.h
//    Author : Michael Callahan
//    Date   : August 2001

#if !defined(ClipField_h)
#define ClipField_h

#include <Core/Algorithms/Fields/share.h>
#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Tensor.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/SparseRowMatrix.h>

#include <math.h>
#include <algorithm>

//! for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRun {


class SCISHARE ClipFieldByFunctionAlgo : public DynamicAlgoBase
{
public:
  bool ClipFieldByFunction(ProgressReporter *pr,
			   vector< FieldHandle >& field_input_handles,
			   FieldHandle& field_output_handle,
			   MatrixHandle& matrix_output_handle,
			   string mode,
			   string function );

protected:
  CompileInfoHandle get_compile_info(vector<FieldHandle>& field_input_handles,
				     string clipfunction,
				     int hashoffset);

  virtual bool execute(ProgressReporter *reporter,
		       vector< FieldHandle > field_input_handles,
		       FieldHandle& field_output_handle,
		       MatrixHandle& matrix_output_handle,
		       int mode ) { return false; };

  virtual string identify() { return string(""); };
};


  // NOTE: The ClipFieldByFunctionAlgoT is completely built
  // when an instance is specified. This is done in the
  // get_compile_info method. It is ugly but it allows for a dynamic
  // input which is more important than pretty code.


} // end namespace SCIRun

#endif // ClipField_h
