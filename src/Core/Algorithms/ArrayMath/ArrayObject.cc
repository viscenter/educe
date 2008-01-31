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

#include <Core/Algorithms/ArrayMath/ArrayObject.h>
#include <Core/Algorithms/Util/FieldInformation.h>

namespace SCIRunAlgo {

void ArrayObject::clear()
{
  type_ = 0;
  size_ = 0;
  
  matrix_ = 0;
  field_  = 0;
  vfield_ = 0;

  data_ = 0;
  idx_ = 0;
  
  name_   = "";
  xname_  = "";
  yname_  = "";
  zname_  = "";
  sizename_ = "";  
  
  is_scalar_ = false;
  is_vector_ = false;
  is_tensor_ = false;
  is_index_  = false;
  is_location_ = false;
  is_element_ = false;
}

bool ArrayObject::create_input_data(SCIRun::FieldHandle field, std::string name)
{
  clear();
	
  name_ = name;
  field_ = field;
  
  if (field.get_rep() == 0)
  {
    pr_->error("ArrayObject: no valid input field");
    return (false);
  }
  
  vfield_ = field->vfield();
  vmesh_ = field->vmesh();
  
  if (vfield_->is_nodata())
	{
		pr_->error("Input field does not have data");
		return (false);
	}
  
  data_ = vfield_->fdata_pointer();
  if (vfield_->is_char()) type_ = 1;
  if (vfield_->is_unsigned_char()) type_ = 2;
  if (vfield_->is_short()) type_ = 3;
  if (vfield_->is_unsigned_short()) type_ = 4;
  if (vfield_->is_int()) type_ = 5;
  if (vfield_->is_unsigned_int()) type_ = 6;
  if (vfield_->is_longlong()) type_ = 7;
  if (vfield_->is_unsigned_longlong()) type_ = 8;
  if (vfield_->is_float()) type_ = 9;
  if (vfield_->is_double()) type_ = 10;
  if (vfield_->is_vector()) type_ = 13;
  if (vfield_->is_tensor()) type_ = 16;

  if (vfield_->is_scalar()) is_scalar_ = true;
  if (vfield_->is_vector()) is_vector_ = true;
  if (vfield_->is_tensor()) is_tensor_ = true;

  if (type_ == 0) return(false);

  size_ = vfield_->num_values();
  
  return(true);
}

bool ArrayObject::create_input_data(SCIRun::MatrixHandle matrix, std::string name)
{
  clear();
  
  if (matrix.get_rep() == 0)
  {
    pr_->error("ArrayObject: no valid input matrix");
    return (false);
  }
  
  matrix_ = dynamic_cast<SCIRun::Matrix *>(matrix->dense());
  if (matrix_.get_rep())
  {
    name_   = name;
    size_   = matrix_->nrows();
    data_   = matrix_->get_data_pointer();
  }
  else
  {
    return(false);
  }
  
  SCIRun::Matrix::size_type cols = matrix_->ncols();
  double* ptr = matrix_->get_data_pointer();
  
  if (size_ == 1)
  {
    if (cols == 1)
    {
      type_ = 11; 
      constant_scalar_ = ptr[0];
      is_scalar_  = true;
    }

    if (cols == 3)
    {
      type_ = 14; 
      constant_vector_ = DataArrayMath::Vector(ptr[0],ptr[1],ptr[2]);
      is_vector_  = true;
    }
    
    if (cols == 6)
    {
      type_ = 17; 
      constant_tensor_ = DataArrayMath::Tensor(ptr[0],ptr[1],ptr[2],
                                               ptr[3],ptr[4],ptr[5]);
      is_tensor_  = true;
    }

    if (cols == 9)
    {
      type_ = 17; 
      constant_tensor_ = DataArrayMath::Tensor(ptr[0],ptr[1],ptr[2],
                                               ptr[4],ptr[5],ptr[8]);
      is_tensor_  = true;
    }
  }
  else
  {
    if (cols == 1)
    {
      type_ = 12;
      is_scalar_  = true;
    }

    if (cols == 3)
    {
      type_ = 15;
      is_vector_  = true;
    }
    
    if (cols == 6)
    {
      type_ = 18;
      is_tensor_  = true;
    }

    if (cols == 9)
    {
      type_ = 19;
      is_tensor_  = true;
    }  
  }
  
  if (type_ == 0)
  {
    pr_->error("ArrayObject: Matrix needs to have 1, 3, 6or 9 columns");
    return(false);    
  }
  return(true);
}


bool ArrayObject::create_input_index(std::string name, std::string sizename)
{
  clear();
  name_ = name;
  sizename_ = sizename;
  type_ = 0;
  size_ = 1;
  is_index_ = true;
  
  return(true);
}

bool ArrayObject::create_input_location(SCIRun::FieldHandle field, std::string locname, std::string xname, std::string yname, std::string zname,std::string location)
{
  clear();
  
  if (field.get_rep() == 0)
  {
    pr_->error("ArrayObject: no valid input field");
    return (false);
  }
  
  name_ =  locname;
  xname_ = xname;
  yname_ = yname;
  zname_ = zname;
  field_ = field;
  vfield_ = field->vfield();
  vmesh_  = field->vmesh();
  if (vfield_->basis_order() == 0)
  {
    size_ = vmesh_->num_elems();
  }   
  else
  {  
    size_ = vmesh_->num_nodes();
  }
  is_location_ = true;
  
  return(true);
}

bool ArrayObject::create_input_element(SCIRun::FieldHandle field, std::string name)
{
  clear();
  
  if (field.get_rep() == 0)
  {
    pr_->error("ArrayObject: no valid input field");
    return (false);
  }
    
  name_ = name;
  field_ = field;
  vfield_ = field->vfield();
  vmesh_  = field->vmesh();
  size_ = vfield_->num_values();
  is_element_ = true;

  return(true);
}
    
bool ArrayObject::create_output_data(SCIRun::FieldHandle& field, std::string datatype, std::string name, SCIRun::FieldHandle& ofield)
{
  clear();

  if (field.get_rep() == 0)
  {
    pr_->error("ArrayObject: no valid input field");
    return (false);
  }
	
  SCIRun::FieldInformation fi(field);

  if (datatype == "Scalar") datatype = "double";
  if (datatype != "Same as Input") fi.set_data_type(datatype);

  ofield = CreateField(fi,field->mesh());
  
  name_ = name;
  field_ = ofield;
  
  vfield_ = ofield->vfield();
  vmesh_  = field->vmesh();
  if (vfield_->is_nodata())
	{
		pr_->error("Input field does not have data");
		return (false);
	}
  
  data_ = vfield_->fdata_pointer();
  if (vfield_->is_char()) type_ = 1;
  if (vfield_->is_unsigned_char()) type_ = 2;
  if (vfield_->is_short()) type_ = 3;
  if (vfield_->is_unsigned_short()) type_ = 4;
  if (vfield_->is_int()) type_ = 5;
  if (vfield_->is_unsigned_int()) type_ = 6;
  if (vfield_->is_longlong()) type_ = 7;
  if (vfield_->is_unsigned_longlong()) type_ = 8;
  if (vfield_->is_float()) type_ = 9;
  if (vfield_->is_double()) type_ = 10;
  if (vfield_->is_vector()) type_ = 13;
  if (vfield_->is_tensor()) type_ = 16;

  if (vfield_->is_scalar()) is_scalar_ = true;
  if (vfield_->is_vector()) is_vector_ = true;
  if (vfield_->is_tensor()) is_tensor_ = true;

  if (type_ == 0) return(false);

  vfield_->resize_fdata();
  size_ = vfield_->num_values();
  
  return(true);

}


bool ArrayObject::create_output_location(SCIRun::FieldHandle& field,  std::string name, SCIRun::FieldHandle& ofield)
{
  clear();
  
  if (field.get_rep() == 0) 
  {
    pr_->error("No input data field");
    return(false);
  }
  
  name_ = name;

  SCIRun::VMesh* imesh = field->vmesh();
  
  SCIRun::FieldInformation fi(field);
  // make point editable
  if (fi.is_regularmesh())
  {
    if (fi.is_hex_element()) fi.make_structhexvolmesh();
    if (fi.is_quad_element()) fi.make_structquadsurfmesh();
    if (fi.is_crv_element()) fi.make_structcurvemesh();
  
    SCIRun::VMesh::dimension_type dims;
    imesh->get_dimensions(dims);
    
    SCIRun::MeshHandle mesh = CreateMesh(fi,dims);
    ofield = CreateField(fi,mesh);          
  }
  else
  {
    SCIRun::MeshHandle mesh = field->mesh();
    mesh.detach();
    ofield = CreateField(fi,mesh);              
  }

  field_ = ofield;
  vfield_ = ofield->vfield();
  vmesh_ = ofield->vmesh();
  
  vfield_->resize_fdata();
  
  // copy all data
  vfield_->copy_values(field->vfield());
  
  is_location_ = true;
  size_ = vmesh_->num_nodes();
  
  return(true);
}




bool ArrayObject::create_output_data(SCIRun::FieldHandle& field, std::string datatype, std::string basistype, std::string name, SCIRun::FieldHandle& ofield)
{
  clear();

  if (field.get_rep() == 0)
  {
    pr_->error("ArrayObject: no valid input field");
    return (false);
  }
	
  SCIRun::FieldInformation fi(field);
  
  if (datatype == "Scalar") datatype = "double";
  if (datatype != "Same as Input") fi.set_data_type(datatype);
  fi.set_basis_type(basistype);
  ofield = CreateField(fi,field->mesh());
  
  name_ = name;
  field_ = ofield;
  
  vfield_ = ofield->vfield();
  vmesh_ = ofield->vmesh();
  
  if (vfield_->is_nodata())
	{
		pr_->error("Input field does not have data");
		return (false);
	}
  
  data_ = vfield_->fdata_pointer();
  if (vfield_->is_char()) type_ = 1;
  if (vfield_->is_unsigned_char()) type_ = 2;
  if (vfield_->is_short()) type_ = 3;
  if (vfield_->is_unsigned_short()) type_ = 4;
  if (vfield_->is_int()) type_ = 5;
  if (vfield_->is_unsigned_int()) type_ = 6;
  if (vfield_->is_longlong()) type_ = 7;
  if (vfield_->is_unsigned_longlong()) type_ = 8;
  if (vfield_->is_float()) type_ = 9;
  if (vfield_->is_double()) type_ = 10;
  if (vfield_->is_vector()) type_ = 13;
  if (vfield_->is_tensor()) type_ = 16;

  if (vfield_->is_scalar()) is_scalar_ = true;
  if (vfield_->is_vector()) is_vector_ = true;
  if (vfield_->is_tensor()) is_tensor_ = true;

  if (type_ == 0) return(false);

  vfield_->resize_fdata();
  size_ = vfield_->num_values();
  
  return(true);
}



bool ArrayObject::create_output_data(int size, std::string datatype, std::string name, SCIRun::MatrixHandle& omatrix)
{    
  clear();
  
  if (size == 0)
  {
    pr_->error("ArrayObject: Output matrix size is 0");
    return(false);
  }
  
  name_ = name;
  size_ = size;
  
  SCIRun::Matrix::size_type ncols;
  if (datatype == "Scalar")  { type_ = 12; is_scalar_ = true; ncols = 1;}
  if (datatype == "Vector")  { type_ = 15; is_vector_ = true; ncols = 3;}
  if (datatype == "Tensor")  { type_ = 18; is_tensor_ = true; ncols = 6;}
  if (datatype == "Tensor6") { type_ = 18; is_tensor_ = true; ncols = 6;}
  if (datatype == "Tensor9") { type_ = 19; is_tensor_ = true; ncols = 9;}
  
  
  matrix_ = dynamic_cast<SCIRun::Matrix *>(scinew SCIRun::DenseMatrix(size_,ncols));
  if (matrix_.get_rep() == 0)
  {
    pr_->error("ArrayObject: Could not allocate matrix");
    return(false);
  }
  
  data_ = matrix_->get_data_pointer();
  omatrix = matrix_;
  return (true);
}

} // end namespace

