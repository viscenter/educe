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

#include <Core/Algorithms/Fields/Mapping/ApplyMappingMatrix.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/SparseRowMatrix.h>

namespace SCIRunAlgo {

using namespace SCIRun;

//! Internal function to this algorithm: no need for this function to be
//! public. It is called from the algorithm class only.

template <class DATA> 
bool 
ApplyMappingMatrixT(ApplyMappingMatrixAlgo* algo,
                    VField* input, VField* output,
                    SparseRowMatrix* mapping);

//! This is the basic algorithm behind the mapping algorithm

template <class DATA> 
bool 
ApplyMappingMatrixT(ApplyMappingMatrixAlgo* algo,
                    VField* input, VField* output,
                    SparseRowMatrix* mapping)
{
  double* vals = mapping->a;
  Matrix::index_type* rows = mapping->rows;
  Matrix::index_type* columns = mapping->columns;
  Matrix::size_type m = mapping->nrows();

  int cnt = 0; 
  for(Matrix::index_type idx=0; idx<m;idx++)
  { 
    DATA val(0);
    Matrix::index_type rr = rows[idx];
    Matrix::size_type  ss = rows[idx+1]-rows[idx];
    input->get_weighted_value(val,&columns[rr],&vals[rr],ss);
    output->set_value(val,idx);
    cnt++; if (cnt==400) algo->update_progress(idx,m);
  }

  //! Algorithm succeeded
  algo->algo_end(); return (true);
}


//! Actual Algorithm class

bool 
ApplyMappingMatrixAlgo::
run(FieldHandle& isrc, FieldHandle& idst, MatrixHandle& mapping, FieldHandle& output)
{
  algo_start("ApplyMappingMatrix",true);
  
  //! safety check
  if (isrc.get_rep() == 0)
  {
    error("No input source field");
    algo_end(); return (false);
  }

  if (!(isrc->has_virtual_interface()))
  {
    error("Source input field has not virtual interface");
    algo_end(); return (false);
  }

  //! safety check
  if (idst.get_rep() == 0)
  {
    error("No input source field");
    algo_end(); return (false);
  }

  if (!(idst->has_virtual_interface()))
  {
    error("Source input field has not virtual interface");
    algo_end(); return (false);
  }

  if (mapping.get_rep() == 0)
  {
    error("No mapping source field");
    algo_end(); return (false);
  }

  SparseRowMatrix* matrix = mapping->sparse();
  if (matrix == 0)
  {
    error("Mapping matrix needs to be sparse");
    algo_end(); return (false);
  }
  

  VField* ifsrc =  isrc->vfield();
  VField* ifdst =  idst->vfield();
  //VMesh*  imsrc =  isrc->vmesh();
  VMesh*  imdst =  idst->vmesh();
  

  //! Get information about field types
  FieldInformation fi(isrc);
  FieldInformation fo(idst);
  fo.set_data_type(fi.get_data_type());
  
  Matrix::size_type m = mapping->nrows();
  Matrix::size_type n = mapping->ncols();
  
  Field::size_type dst_num_nodes = imdst->num_nodes();
  Field::size_type dst_num_elems = imdst->num_elems();
  Field::size_type dst_num_values = ifdst->num_values();
  Field::size_type src_num_values = ifsrc->num_values();

  if (dst_num_values == m)
  {
    // do nothing
  }
  if (m == dst_num_nodes)
  {
    fo.make_lineardata();
  }
  else if (m == dst_num_elems)
  {
    fo.make_constantdata();
  }
  else
  {
    error("The number of columns in the matrix does not match number of nodes or elements in the destination field");
    algo_end(); return (false);
  }
  
  if (src_num_values != n)
  {
    cerr << "n="<<n<<"\n";
    cerr << "num_values="<<src_num_values<<"\n";
    error("The number of columns in the matrix does not match number of values in the source field");
    algo_end(); return (false);
  }

  //! Create output field
  output = CreateField(fo,idst->mesh());
 
  VField* ofield = output->vfield();
  ofield->resize_values();
  
  //! Check whether output field was created
  if (output.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return(false);
  } 
  
  //! Simple table to deal with the various data type formats
  //! Note that not every data type is handled, all char, shorts etc,
  //! are automatically handled by the int, and unsigned int case, by
  //! casting the data on input (these should be the less frequently
  //! used datatypes and hence have no specific algorithm in place).
  //! Similarly floats are casted to doubles.

  if (isrc->vfield()->is_char()) 
    return (ApplyMappingMatrixT<char>(this,ifsrc,ofield,matrix));
  if (isrc->vfield()->is_unsigned_char()) 
    return (ApplyMappingMatrixT<unsigned char>(this,ifsrc,ofield,matrix));

  if (isrc->vfield()->is_short()) 
    return (ApplyMappingMatrixT<short>(this,ifsrc,ofield,matrix));
  if (isrc->vfield()->is_unsigned_short()) 
    return (ApplyMappingMatrixT<unsigned short>(this,ifsrc,ofield,matrix));

  if (isrc->vfield()->is_int()) 
    return (ApplyMappingMatrixT<int>(this,ifsrc,ofield,matrix));
  if (isrc->vfield()->is_unsigned_int()) 
    return (ApplyMappingMatrixT<unsigned int>(this,ifsrc,ofield,matrix));
  
  if (isrc->vfield()->is_longlong()) 
    return (ApplyMappingMatrixT<long long>(this,ifsrc,ofield,matrix));
  if (isrc->vfield()->is_unsigned_longlong()) 
    return (ApplyMappingMatrixT<unsigned long long>(this,ifsrc,ofield,matrix));

  if (isrc->vfield()->is_float()) 
    return (ApplyMappingMatrixT<float>(this,ifsrc,ofield,matrix));
  if (isrc->vfield()->is_double()) 
    return (ApplyMappingMatrixT<double>(this,ifsrc,ofield,matrix));

  if (isrc->vfield()->is_vector()) 
    return (ApplyMappingMatrixT<Vector>(this,ifsrc,ofield,matrix));
  if (isrc->vfield()->is_tensor()) 
    return (ApplyMappingMatrixT<Tensor>(this,ifsrc,ofield,matrix));


  error("Encountered an unknown data type");
  algo_end(); return (false);
}

} // namespace SCIRunAlgo
