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
 *  TransformMeshWithFunction: Mesh transform operation
 *
 *  Written by:
 *   Allen R. Sanderson
 *   Michael Callahan
 *   SCI Institute
 *   University of Utah
 *   March 2007
 *
 *  Copyright (C) 2006 SCI Group
 */

#if !defined(TransformMeshWithFunction_h)
#define TransformMeshWithFunction_h

#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Tensor.h>
#include <Core/Datatypes/Field.h>

//! for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRun {

class SCISHARE TransformMeshWithFunctionAlgo : public DynamicAlgoBase
{
public:
  bool TransformMeshWithFunction(ProgressReporter *pr,
				 FieldHandle& field_input_handle,
				 FieldHandle& field_output_handle,
				 string function );

protected:
  CompileInfoHandle get_compile_info(const TypeDescription *field_td,
				     string function,
				     int hashoffset);
  
  virtual bool execute(ProgressReporter *pr,
		       FieldHandle& field_input_handle,
		       FieldHandle& field_output_handle ) { return false; };

  virtual string identify() { return string(""); };
};


template <class FIELD>
class TransformMeshWithFunctionAlgoT : public TransformMeshWithFunctionAlgo
{
public:
  virtual void function( Point &result,
			 double x, double y, double z,
			 typename FIELD::value_type &val ) = 0;

  //! virtual interface. 
  virtual bool execute( ProgressReporter *pr,
			FieldHandle& field_input_handle,
			FieldHandle& field_output_handle );
};


template <class FIELD>
bool
TransformMeshWithFunctionAlgoT<FIELD>::execute(ProgressReporter *pr,
			       FieldHandle& field_input_handle,
			       FieldHandle& field_output_handle)
{
  FIELD *iField = dynamic_cast<FIELD *>(field_input_handle.get_rep());
  FIELD *oField = iField->clone();

  oField->mesh_detach();
  typename FIELD::mesh_handle_type oMesh = oField->get_typed_mesh();

  typename FIELD::mesh_type::Node::iterator ibi, iei;
  oMesh->begin(ibi);
  oMesh->end(iei);

  while (ibi != iei)
  {
    Point p;
    oMesh->get_center(p, *ibi);

    typename FIELD::value_type val;
    oField->value(val, *ibi);

    Point result = p;
    function(result, p.x(), p.y(), p.z(), val);
    oMesh->set_point(result, *ibi);

    ++ibi;
  }

  field_output_handle = (FieldHandle) oField;

  return true;
}


} // end namespace SCIRun

#endif // TransformMeshWithFunction_h
