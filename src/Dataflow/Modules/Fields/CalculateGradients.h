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
 *  CalculateGradients.h:
 *
 *  Written by:
 *   Allen Sanderson
 *   School of Computering
 *   University of Utah
 *   May 2002
 *
 *  Copyright (C) 2002 SCI Group
 */

#if !defined(CalculateGradients_h)
#define CalculateGradients_h

#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Mesh.h>

#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Tensor.h>

//! for Windows support
#include <Dataflow/Modules/Fields/share.h>

namespace SCIRun {

class SCISHARE CalculateGradientsAlgo : public DynamicAlgoBase
{
public:
  virtual FieldHandle execute(FieldHandle& src) = 0;

  //! support the dynamically compiled algorithm concept
  static CompileInfoHandle get_compile_info(const TypeDescription *ftd,
					    const TypeDescription *mtd,
					    const TypeDescription *btd,
					    const TypeDescription *dtd,
					    const TypeDescription *otd);
};


template< class IFIELD, class OFIELD >
class CalculateGradientsAlgoT : public CalculateGradientsAlgo
{
public:
  //! virtual interface. 
  virtual FieldHandle execute(FieldHandle& src);
};


template< class IFIELD, class OFIELD >
FieldHandle
CalculateGradientsAlgoT<IFIELD, OFIELD>::execute(FieldHandle& field_h)
{
  IFIELD *ifield = (IFIELD *) field_h.get_rep();

  typename IFIELD::mesh_handle_type imesh = ifield->get_typed_mesh();
    
  OFIELD *ofield = scinew OFIELD(imesh);

  typename IFIELD::mesh_type::Cell::iterator in, end;
  typename OFIELD::mesh_type::Cell::iterator out;

  imesh->begin( in );
  imesh->end( end );

  ofield->get_typed_mesh()->begin( out );

  typename OFIELD::value_type gradient;
  
  // calculate element center
  typename IFIELD::mesh_type::basis_type basis = imesh->get_basis();
  
  int dim = basis.domain_dimension();
  int num = basis.number_of_vertices();
  std::vector<double> coords(dim);

  for (int j=0;j<dim;j++) coords[j] = 0.0;
  
  for (int i=0; i<num;i++) 
    for (int j=0;j<dim;j++) coords[j] += basis.unit_vertices[i][j];
  
  for (int j=0;j<dim;j++) coords[j] /= static_cast<double>(num);
  
  std::vector<typename IFIELD::value_type> grad;
  
  while (in != end) 
  {
    ifield->gradient(grad,coords,*in);
    gradient.x(static_cast<double>(grad[0]));
    gradient.y(static_cast<double>(grad[1]));
    gradient.z(static_cast<double>(grad[2]));
    ofield->set_value(gradient, *out);
    ++in; ++out;
  }

  ofield->freeze();

  return FieldHandle( ofield );
}

} // end namespace SCIRun

#endif // CalculateGradients_h