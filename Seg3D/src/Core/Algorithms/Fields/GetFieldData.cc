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

#include <Core/Algorithms/Fields/GetFieldData.h>
#include <Core/Datatypes/DenseMatrix.h>

//! Namespace used for SCIRun Algorithmic layer
namespace SCIRunAlgo {

using namespace SCIRun;

//! Function call to convert data from Field into Matrix data
bool GetFieldDataAlgo::GetFieldData(ProgressReporter *pr,
				    FieldHandle& field_input_handle,
				    MatrixHandle& matrix_output_handle)
{
  //! Check whether we have a field.
  if (field_input_handle.get_rep() == 0)
  {
    pr->error("GetFieldData: No input source field");
    return (false);
  }
  
  //! Construct a class with all the type information of this field
  FieldInformation fi(field_input_handle);

  //! Check whether we have data
  if (fi.is_nodata())
  {
    pr->error("GetFieldData: Field does not contain any data");
    return (false);
  }
  
  //! Depending on the data type select a sub algorithm
  if (fi.is_scalar())
    return(GetScalarFieldDataV(pr,field_input_handle,matrix_output_handle));

  else if (fi.is_vector())
    return(GetVectorFieldDataV(pr,field_input_handle,matrix_output_handle));

  else if (fi.is_tensor())
    return(GetTensorFieldDataV(pr,field_input_handle,matrix_output_handle));

  pr->error("GetFieldData: Unknown data type");
  return (false);
}


bool GetFieldDataAlgo::GetScalarFieldDataV(ProgressReporter *pr,
					   FieldHandle& field_input_handle,
					   MatrixHandle& matrix_output_handle)
{
  //! Obtain virtual interface
  VField* field = field_input_handle->vfield();
  
  //! Obtain the number values in a field
  VMesh::size_type size = field->num_values();
  
  //! Create output object
  matrix_output_handle = scinew DenseMatrix(size,1);
  if (matrix_output_handle.get_rep() == 0)
  {
    pr->error("GetFieldData: Could not allocate output matrix");
    return (false);
  }
  double* dataptr = matrix_output_handle->get_data_pointer();

  //! Copy al the data from the field to the matrix:
  
  //! We do all data handling in doubles
  double val;
  for (VMesh::index_type i=0; i<size; i++)
  {
    //! Get and cast data value
    field->get_value(val,i);
    //! Insert data into matrix
    dataptr[i] = val;
  }
  
  return (true);
}


bool GetFieldDataAlgo::GetVectorFieldDataV(ProgressReporter *pr,
					   FieldHandle& field_input_handle,
					   MatrixHandle& matrix_output_handle)
{
  VField* field = field_input_handle->vfield();
  
  VMesh::size_type size = field->num_values();
  
  matrix_output_handle = scinew DenseMatrix(size,3);
  if (matrix_output_handle.get_rep() == 0)
  {
    pr->error("GetFieldData: Could not allocate output matrix");
    return (false);
  }
  double* dataptr = matrix_output_handle->get_data_pointer();

  Vector val;
  int k = 0;
  for (VMesh::index_type i=0; i<size; i++)
  {
    field->get_value(val,i);
    dataptr[k] = val.x();
    dataptr[k+1] = val.y();
    dataptr[k+2] = val.z();
    k+=3;
  }
  return (true);
}


bool GetFieldDataAlgo::GetTensorFieldDataV(ProgressReporter *pr,
					   FieldHandle& field_input_handle,
					   MatrixHandle& matrix_output_handle)
{
  VField* field = field_input_handle->vfield();
  
  VMesh::size_type size = field->num_values();
  
  matrix_output_handle = scinew DenseMatrix(size,6);
  if (matrix_output_handle.get_rep() == 0)
  {
    pr->error("GetFieldData: Could not allocate matrix_output_handle matrix");
    return (false);
  }
  double* dataptr = matrix_output_handle->get_data_pointer();

  Tensor val;
  int k = 0;
  for (VMesh::index_type i=0; i<size; i++)
  {
    field->get_value(val,i);
    dataptr[k] = static_cast<double>(val.mat_[0][0]);
    dataptr[k+1] = static_cast<double>(val.mat_[0][1]);
    dataptr[k+2] = static_cast<double>(val.mat_[0][2]);
    dataptr[k+3] = static_cast<double>(val.mat_[1][1]);
    dataptr[k+4] = static_cast<double>(val.mat_[1][2]);
    dataptr[k+5] = static_cast<double>(val.mat_[2][2]);    
    k+=6;
  }
  return (true);
}

} // namespace SCIRunAlgo
