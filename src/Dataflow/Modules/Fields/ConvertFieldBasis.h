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



//    File   : ConvertFieldBasis.h
//    Author : McKay Davis
//    Date   : July 2002


#if !defined(ConvertFieldBasis_h)
#define ConvertFieldBasis_h

#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/SparseRowMatrix.h>

#include <algorithm>

//! for Windows support
#include <Dataflow/Modules/Fields/share.h>

namespace SCIRun {


class SCISHARE ConvertFieldBasisAlgo : public DynamicAlgoBase
{
public:

  virtual FieldHandle execute(ProgressReporter *reporter,
			      FieldHandle fsrc_h,
			      int basis_order,
			      MatrixHandle &interp) = 0;

  //! support the dynamically compiled algorithm concept
  static CompileInfoHandle get_compile_info(const TypeDescription *fsrc,
                                            const string &fdst);
};


template <class FSRC, class FDST>
class ConvertFieldBasisAlgoT : public ConvertFieldBasisAlgo
{
public:

  virtual FieldHandle execute(ProgressReporter *reporter,
			      FieldHandle fsrc_h,
			      int basis_order,
			      MatrixHandle &interp);
};


template <class FSRC, class FDST>
FieldHandle
ConvertFieldBasisAlgoT<FSRC, FDST>::execute(ProgressReporter *reporter,
                                           FieldHandle fsrc_h,
                                           int basis_order,
                                           MatrixHandle &interp)
{
  FSRC *fsrc = dynamic_cast<FSRC *>(fsrc_h.get_rep());
  typename FSRC::mesh_handle_type mesh = fsrc->get_typed_mesh();

  // Create the field with the new mesh and data location.
  FDST *fout = scinew FDST(fsrc->get_typed_mesh());
  fout->resize_fdata();

  if (fsrc->basis_order() > 0)
  {
    typename FSRC::mesh_type::Node::size_type nodesize;
    mesh->size(nodesize);
    const int ncols = nodesize;

    Matrix::size_type nrows = 0;
    Matrix::index_type *rr = 0;
    Matrix::size_type nnz = 0;
    Matrix::index_type *cc = 0;
    double *d = 0;
    typename FSRC::mesh_type::Node::array_type tmparray;

    if (basis_order == 0 && mesh->dimensionality())
    {
      typename FSRC::mesh_type::Elem::size_type osize;
      mesh->size(osize);
      nrows = osize;

      rr = scinew Matrix::index_type[nrows+1];
      rr[0] = 0;
      size_t counter = 0;
      
      std::vector<Matrix::index_type> cc_tmp;
      std::vector<double> d_tmp;
      
      typename FSRC::mesh_type::Elem::iterator itr, eitr;
      mesh->begin(itr);
      mesh->end(eitr);
      
      while (itr != eitr)
      {
        mesh->get_nodes(tmparray, *itr);
        for (size_t i = 0; i < tmparray.size(); i++)
        {
          cc_tmp.push_back(tmparray[i]);
          d_tmp.push_back(1.0/tmparray.size());
        }

        ++itr;
        ++counter;
        rr[counter] = rr[counter-1] + tmparray.size();
      }
            
      cc = scinew Matrix::index_type[cc_tmp.size()];
      d = scinew double[d_tmp.size()];
      for (size_t i=0; i< cc_tmp.size(); i++) 
      {
        cc[i] = cc_tmp[i];
        d[i] = d_tmp[i];
      }      
      
      nnz = cc_tmp.size();
    } 

    if (rr && cc)
    {
      for (Matrix::index_type i = 0; i < nrows; i++)
      {
        std::sort(cc + rr[i], cc + rr[i+1]);
      }
      interp = scinew SparseRowMatrix(nrows, ncols, rr, cc, nnz, d);
    }
    else if (rr)
    {
      delete rr;
    }
  }
  try {
    if (basis_order == 1 && fsrc->basis_order() == 0 && mesh->dimensionality())
    {
      mesh->synchronize(Mesh::NODE_NEIGHBORS_E);

      typename FSRC::mesh_type::Elem::size_type nsize;
      mesh->size(nsize);
      const Matrix::size_type ncols = static_cast<Matrix::size_type>(nsize);

      typename FSRC::mesh_type::Node::size_type osize;
      mesh->size(osize);
      const Matrix::size_type nrows = static_cast<Matrix::size_type>(osize);

      Matrix::index_type *rr = scinew Matrix::index_type[nrows + 1];
      vector<Matrix::index_type> cctmp;
      vector<double> dtmp;

      typename FSRC::mesh_type::Elem::array_type tmparray;
      typename FSRC::mesh_type::Node::iterator itr, eitr;
      mesh->begin(itr);
      mesh->end(eitr);
      rr[0] = 0;
      Matrix::index_type counter = 0;
      while (itr != eitr)
      {
        mesh->get_elems(tmparray, *itr);
        for (size_t i = 0; i < tmparray.size(); i++)
        {
          cctmp.push_back(tmparray[i]);
          dtmp.push_back(1.0 / tmparray.size()); // Weight by distance?
        }

        ++itr;
        ++counter;
        rr[counter] = rr[counter-1] + tmparray.size();
      }

      const Matrix::size_type nnz = static_cast<Matrix::size_type>(cctmp.size());
      Matrix::index_type *cc = scinew Matrix::index_type[nnz];
      double *d = scinew double[nnz];
      for (Matrix::index_type i = 0; i < nnz; i++)
      {
        cc[i] = cctmp[i];
        d[i] = dtmp[i];
      }

      for (Matrix::index_type i = 0; i < nrows; i++)
      {
        std::sort(cc + rr[i], cc + rr[i+1]);
      }
      
      interp = scinew SparseRowMatrix(nrows, ncols, rr, cc, nnz, d);
    }    
  } catch (...)
  {
  }

  fout->copy_properties(fsrc);

  return fout;
}


} // end namespace SCIRun

#endif // ConvertFieldBasis_h
