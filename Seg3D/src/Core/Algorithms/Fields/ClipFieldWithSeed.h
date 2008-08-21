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
 *  ClipFieldWithSeed.cc:  Clip out parts of a field using a seed(s)
 *
 *  Written by:
 *   Allen R. Sanderson
 *   SCI Institute
 *   University of Utah
 *   July 2007
 *
 *  Copyright (C) 2007 SCI Institute
 */

#if !defined(ClipFieldWithSeedAlgo_h)
#define ClipFieldWithSeedAlgo_h

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


class SCISHARE ClipFieldWithSeedAlgo : public DynamicAlgoBase
{
public:
  bool ClipFieldWithSeed(ProgressReporter *pr,
			 FieldHandle& field_input_handle,
			 FieldHandle& seed_input_handle,
			 FieldHandle& field_output_handle,
			 MatrixHandle& matrix_output_handle,
			 string mode,
			 string function );

protected:
  CompileInfoHandle get_compile_info(FieldHandle& field_input_handle,
				     FieldHandle& seed_input_handle,
				     string clipfunction,
				     int hashoffset);

  virtual bool execute(ProgressReporter *pr,
                       FieldHandle& field_input_handle,
                       FieldHandle& seed_input_handle,
                       FieldHandle& field_output_handle,
                       MatrixHandle& matrix_output_handle,
		       int mode )
  { return false; };

  virtual string identify() { return string(""); };
};



template <class IFIELD, class SFIELD, class OFIELD>
class ClipFieldWithSeedAlgoT : public ClipFieldWithSeedAlgo
{

public:
  virtual bool execute(ProgressReporter *pr,
                       FieldHandle& field_input_handle,
                       FieldHandle& seed_input_handle,
                       FieldHandle& field_output_handle,
                       MatrixHandle& matrix_output_handle,
		       int mode );

  virtual bool function(double fx, double fy, double fz,
                        double sx, double sy, double sz,
                        const typename IFIELD::value_type &fv,
                        const typename SFIELD::value_type &sv )
  { return false; };

  virtual string identify() { return string(""); };
};


template< class IFIELD, class SFIELD, class OFIELD >
bool ClipFieldWithSeedAlgoT< IFIELD, SFIELD, OFIELD >::
execute(ProgressReporter *pr,
	FieldHandle& field_input_handle,
	FieldHandle& seed_input_handle,
	FieldHandle& field_output_handle,
	MatrixHandle& matrix_output_handle,
	int mode )
{
  IFIELD *ifield = dynamic_cast<IFIELD *>(field_input_handle.get_rep());
  SFIELD *sfield = dynamic_cast<SFIELD *>(seed_input_handle.get_rep());
  
  typename IFIELD::mesh_type *iMesh =
    dynamic_cast<typename IFIELD::mesh_type *>(ifield->mesh().get_rep());
  
  typename SFIELD::mesh_type *sMesh =
    dynamic_cast<typename SFIELD::mesh_type *>(sfield->mesh().get_rep());
  
  typename OFIELD::mesh_type *oMesh = new typename OFIELD::mesh_type();
  oMesh->copy_properties(iMesh);
  
#ifdef HAVE_HASH_MAP

#if defined(__ECC) || defined(_MSC_VER)
  typedef hash_map<unsigned int, typename IFIELD::mesh_type::Node::index_type> hash_type;
#else
  typedef hash_map<unsigned int,
    typename IFIELD::mesh_type::Node::index_type,
    hash<unsigned int>,
    equal_to<unsigned int> > hash_type;
#endif
  
#else
  typedef map<unsigned int,
    typename IFIELD::mesh_type::Node::index_type,
    less<unsigned int> > hash_type;
#endif
  
  hash_type nodemap;
    
  vector<typename IFIELD::mesh_type::Elem::index_type> elemmap;

  typedef typename IFIELD::mesh_type mesh_type_val;
  const bool element_data_valid =
    (ifield->order_type_description()->get_name() == 
     mesh_type_val::elem_type_description()->get_name());

  const bool node_data_valid =
    (ifield->basis_order() == 1 || iMesh->dimensionality() == 0);

  typename IFIELD::mesh_type::Elem::size_type prsizetmp;
  iMesh->size(prsizetmp);
  const Field::size_type prsize = (Field::size_type)prsizetmp;
  Field::size_type prcounter = 0;

  typename IFIELD::mesh_type::Elem::iterator ibi, iei;
  iMesh->begin(ibi);
  iMesh->end(iei);

  while (ibi != iei)
  {
    pr->update_progress(prcounter, prsize);

    typename SFIELD::mesh_type::Node::iterator sbi, sei;
    sMesh->begin(sbi);
    sMesh->end(sei);

    while (sbi != sei)
    {
      Point s;
      sMesh->get_center(s, *sbi);

      typename SFIELD::value_type sval = typename SFIELD::value_type();
      if (element_data_valid) sfield->value(sval, *sbi);

      bool inside = false;

      if (mode == 0)
      {
        Point p;
        iMesh->get_center(p, *ibi);

        typename IFIELD::value_type fval = typename IFIELD::value_type();
        if (element_data_valid) ifield->value(fval, *ibi);

        inside = function(p.x(), p.y(), p.z(),
			  s.x(), s.y(), s.z(), fval, sval);
      }
      else
      {
        typename IFIELD::mesh_type::Node::array_type onodes;
        iMesh->get_nodes(onodes, *ibi);
        inside = true;
        for (size_t i = 0; i < onodes.size(); i++)
        {
          Point p;
          iMesh->get_center(p, onodes[i]);

          typename IFIELD::value_type fval = typename IFIELD::value_type();
          if (node_data_valid) ifield->value(fval, onodes[i]);

          if (!function(p.x(), p.y(), p.z(),
			  s.x(), s.y(), s.z(), fval, sval))
          {
            inside = false;
            break;
          }
        }
      }

      if (inside)
      {
        typename IFIELD::mesh_type::Node::array_type onodes;
        iMesh->get_nodes(onodes, *ibi);

        typename IFIELD::mesh_type::Node::array_type nnodes(onodes.size());

        for (size_t i = 0; i<onodes.size(); i++)
        {
          if (nodemap.find((Field::index_type)onodes[i]) == nodemap.end())
          {
            Point np;
            iMesh->get_center(np, onodes[i]);

            const typename OFIELD::mesh_type::Node::index_type nodeindex =
              oMesh->add_point(np);

            nodemap[(Field::index_type)onodes[i]] = nodeindex;
            nnodes[i] = nodeindex;
          }
          else
          {
            nnodes[i] = nodemap[(Field::index_type)onodes[i]];
          }
        }

        oMesh->add_elem(nnodes);
        elemmap.push_back(*ibi);
      }
      
      ++sbi;
    }
    
    ++ibi;
  }

  OFIELD *oField = new OFIELD(oMesh);
  oField->copy_properties(field_input_handle.get_rep());

  if (node_data_valid)
  {
    IFIELD *iField =
      dynamic_cast<IFIELD *>(field_input_handle.get_rep());

    typename hash_type::iterator hitr = nodemap.begin();
    
    const Matrix::size_type nrows = nodemap.size();
    const Matrix::size_type ncols = iField->fdata().size();
    Matrix::index_type *rr = new Matrix::index_type[nrows+1];
    Matrix::index_type *cc = new Matrix::index_type[nrows];
    double *d = new double[nrows];
    
    typedef typename IFIELD::mesh_type mesh_type_val;
    
    while (hitr != nodemap.end())
    {
      typename IFIELD::value_type val;
      
      ifield->value(val, (typename IFIELD::mesh_type::Node::index_type)((*hitr).first));
      oField->set_value(val, (typename IFIELD::mesh_type::Node::index_type)((*hitr).second));
      
      cc[(*hitr).second] = (*hitr).first;
      
      ++hitr;
    }

    for (Matrix::size_type i=0; i<nrows; i++)
    {
      rr[i] = i;
      d[i] = 1.0;
    }
    rr[nrows] = nrows;
    
    matrix_output_handle =
      new SparseRowMatrix(nrows, ncols, rr, cc, nrows, d);
  }
  else if (field_input_handle->order_type_description()->get_name() ==
	   mesh_type_val::elem_type_description()->get_name())
  {
    const Matrix::size_type nrows = elemmap.size();
    const Matrix::size_type ncols = ifield->fdata().size();
    Matrix::index_type *rr = new Matrix::index_type[nrows+1];
    Matrix::index_type *cc = new Matrix::index_type[nrows];
    double *d = new double[nrows];
    
    for (size_t i=0; i<elemmap.size(); i++)
    {
      typename IFIELD::value_type val;
      
      ifield->value(val, (typename IFIELD::mesh_type::Elem::index_type) elemmap[i]);
      oField->set_value(val, (typename IFIELD::mesh_type::Elem::index_type) i);
	
      cc[i] = elemmap[i];
    }

    for (Matrix::index_type i=0; i<nrows; i++)
    {
      rr[i] = i;
      d[i] = 1.0;
    }
    rr[nrows] = nrows;
    
    matrix_output_handle =
      new SparseRowMatrix(nrows, ncols, rr, cc, nrows, d);
  }
  else
  {
    pr->warning("Unable to copy data at this field data location.");
    pr->warning("No interpolant computed for field data location.");
    matrix_output_handle = 0;
  }
  
  field_output_handle = (FieldHandle) oField;
  
  return true;
};


} // end namespace SCIRun

#endif // ClipFieldWithSeedAlgo_h
