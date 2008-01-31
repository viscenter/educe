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

#include <Core/Algorithms/Fields/SetFieldData.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool SetFieldDataAlgo::SetFieldData(ProgressReporter *pr,
				    FieldHandle& field_input_handle,
				    FieldHandle& field_output_handle,
				    MatrixHandle& matrix_input_handle,
				    bool keepscalartype)
{
  if (!(field_input_handle.get_rep()))
  {
    pr->error("SetFieldData: No input field was provided");
    return (false);  
  }

  if (!(matrix_input_handle.get_rep()))
  {
    pr->error("SetFieldData: No input matrix was provided");
    return (false);    
  }

  FieldInformation fi(field_input_handle);

  // Check whether we have a virtual interface (when every field is
  // converted, we can remove this one)
  if (!(field_input_handle->has_virtual_interface()))
  {
    pr->error("SetFieldData: The input field has no virtual interface");
    return (false);  
  }  

  // Get the virtual interface
  VMesh* imesh = field_input_handle->vmesh();
  VMesh::size_type numnodes = imesh->num_nodes();
  VMesh::size_type numelems = imesh->num_elems();

  
  int numvals;
  bool found = false;

  // try to see whether the matrix dimensions fit the field size
  if ((matrix_input_handle->nrows() == numnodes)||
      (matrix_input_handle->nrows() == numelems))
  {
    found = true;
    
    // do we have a scalar, vector, or tensor
    if (matrix_input_handle->ncols() == 1) 
    {
      fi.make_scalar();
      if (keepscalartype == false) fi.make_double();

    }
    else if (matrix_input_handle->ncols() == 3) 
    {
      fi.make_vector();
    }
    else if ((matrix_input_handle->ncols() == 6)||
	     (matrix_input_handle->ncols() == 9))
    {
      fi.make_tensor();

    } else
      found = false;
    
    if (found)
    {
      numvals = matrix_input_handle->nrows();
      if (numnodes != numelems)
      {
        if (numvals == numnodes) fi.make_lineardata();
        else if (numvals == numelems) fi.make_constantdata();
        else found = false;
      }
      else if ((!(fi.is_lineardata()))&&(!(fi.is_constantdata())))
      {
        if (numvals == numnodes) fi.make_lineardata();
        else found = false;
      }
    }
  }
  else if ((!found)&&((matrix_input_handle->ncols() == numnodes)||
		      (matrix_input_handle->ncols() == numelems)))
  {
    found = true;
    
     // do we have a scalar, vector, or tensor  ?
    if (matrix_input_handle->nrows() == 1) 
      { fi.make_scalar(); if (keepscalartype == false) fi.make_double(); }
    else if (matrix_input_handle->nrows() == 3) 
      { fi.make_vector(); }
    else if ((matrix_input_handle->nrows() == 6)||
	     (matrix_input_handle->nrows() == 9)) 
      { fi.make_tensor(); }
    else found = false;
    
    if (found)
    {
      numvals = matrix_input_handle->ncols();
      if (numnodes != numelems)
      {
        if (numvals == numnodes) fi.make_lineardata();
        else if (numvals == numelems) fi.make_constantdata();
        else found = false;
      }
      else if ((!(fi.is_lineardata()))&&(!(fi.is_constantdata())))
      {
        if (numvals == numnodes) fi.make_lineardata();
        else found = false;
      }
    }
  }
  else
  {
    // Do we have a constant that has to be fitted in every field position ?
    if (matrix_input_handle->nrows() == 1)
    {
      found = true;
      if (matrix_input_handle->ncols() == 1) 
        { fi.make_scalar(); if (keepscalartype == false) fi.make_double(); }
      else if (matrix_input_handle->ncols() == 3) 
        { fi.make_vector(); }
      else if ((matrix_input_handle->ncols() == 6)||
	       (matrix_input_handle->ncols() == 9)) 
        {  fi.make_tensor(); }    
      else found = false;
    }
    else if (matrix_input_handle->ncols() == 1)
    {
      found = true;
      if (matrix_input_handle->nrows() == 1) 
        { fi.make_scalar(); if (keepscalartype == false) fi.make_double(); }
      else if (matrix_input_handle->nrows() == 3) 
        { fi.make_vector(); }
      else if ((matrix_input_handle->nrows() == 6)||
	       (matrix_input_handle->nrows() == 9))
        { fi.make_tensor(); }
      else found = false;
    }
  }
  
  if (!found)
  {
    pr->error("SetFieldData: Matrix dimensions do not match any of the fields dimensions");
    return (false);
  }
  
  field_output_handle = CreateField(fi,field_input_handle->mesh());
  field_output_handle->copy_properties(field_input_handle.get_rep());

  if ((field_output_handle.get_rep() == 0) ||
      (!(field_output_handle->has_virtual_interface())))
  {
    pr->error("SetFieldData: Could not create output field and output interface");
    return (false);  
  }  

  
  VField* ofield = field_output_handle->vfield();
  
  // Convert the matrix to a dense matrix if it is not
  MatrixHandle densematrix;
  if (!(matrix_input_handle->is_dense()) &&
      !(matrix_input_handle->is_column()))
  {
    // store data in a new handle so it deallocates automatically
    densematrix = matrix_input_handle->sparse();
  }
  else
  {
    // handle copy
    densematrix = matrix_input_handle;
  }
  
  double* matrixdata = densematrix->get_data_pointer();
  int nrows = densematrix->nrows();
  int ncols = densematrix->ncols(); 
  
  if (fi.is_scalar())
  {
    if (((nrows == 1)&&(ncols == numvals))||((ncols == 1)&&(nrows == numvals)))
    {
      ofield->set_values(matrixdata,numvals);
    }
    else if ((nrows == 1)&&(ncols == 1))
    {
      ofield->set_all_values(matrixdata[0]);
    }
    else
    {
      pr->error("SetFieldData: Internal error (data not scalar)");
      return (false);        
    }
  }
  else if (fi.is_vector())
  {
    // Handle Vector values
    if ((ncols == 3)&&(nrows == numvals))
    {
      int k =0;
      for (VMesh::index_type i=0; i< numvals; i++)
      {
        Vector v(matrixdata[k],matrixdata[k+1],matrixdata[k+2]);
        ofield->set_value(v,i);
        k += 3;
      }
    }
    else if ((nrows == 3)&&(ncols == numvals))
    {
      for (VMesh::index_type i=0; i< numvals; i++)
      {
        Vector v(matrixdata[i],matrixdata[i+numvals],matrixdata[i+2*numvals]);
        ofield->set_value(v,i);
      }    
    }
    else if (((nrows == 1)&&(ncols == 3))||((ncols == 1)&&(nrows == 3)))
    {
      Vector v(matrixdata[0],matrixdata[1],matrixdata[2]);
      ofield->set_all_values(v);
    }
    else
    {
      pr->error("SetFieldData: Internal error (data not vector)");
      return (false);        
    }
  }
  else if (fi.is_tensor())
  {
    // Fill field with Tensor values
    // Handle 6 by n data 
    if ((ncols == 6)&&(nrows == numvals))
    {
      int k = 0;
      for (VMesh::index_type i=0; i< numvals; i++)
      {
        Tensor v(matrixdata[k],matrixdata[k+1],matrixdata[k+2],
                  matrixdata[k+3],matrixdata[k+4],matrixdata[k+5]);
        ofield->set_value(v,i);
        k += 6;
      }
    }
    else if ((nrows == 6)&&(ncols == numvals))
    {
      for (VMesh::index_type i=0; i< numvals; i++)
      {
        Tensor v(matrixdata[i],matrixdata[i+numvals],
                  matrixdata[i+2*numvals],matrixdata[i+3*numvals],
                  matrixdata[i+4*numvals],matrixdata[i+5*numvals]);
        ofield->set_value(v,i);
      }
    }
    else if (((nrows == 1)&&(ncols == 6))||((ncols == 1)&&(nrows == 6)))
    {
      Tensor v(matrixdata[0],matrixdata[1],matrixdata[2],
                  matrixdata[3],matrixdata[4],matrixdata[5]);
      ofield->set_all_values(v);
    }
    // Handle 9 by n data 
    else if ((ncols == 9)&&(nrows == numvals))
    {
      int k = 0;
      for (VMesh::index_type i=0; i< numvals; i++)
      {
        Tensor v(matrixdata[k],matrixdata[k+1],matrixdata[k+2],
                  matrixdata[k+4],matrixdata[k+5],matrixdata[k+8]);
        ofield->set_value(v,i);
        k += 9;
      }
    }
    else if ((nrows == 9)&&(ncols == numvals))
    {
      for (VMesh::index_type i=0; i< numvals; i++)
      {
        Tensor v(matrixdata[i],matrixdata[i+numvals],
                  matrixdata[i+2*numvals],matrixdata[i+4*numvals],
                  matrixdata[i+5*numvals],matrixdata[i+8*numvals]);
        ofield->set_value(v,i);
      }
    }
    else if (((nrows == 1)&&(ncols == 9))||((ncols == 1)&&(nrows == 9)))
    {
      Tensor v(matrixdata[0],matrixdata[1],matrixdata[2],
                  matrixdata[4],matrixdata[5],matrixdata[8]);
      ofield->set_all_values(v);
    }    else
    {
      pr->error("SetFieldData: Internal error (data not tensor)");
      return (false);        
    }  
  }

  return  (true);
}


bool SetFieldDataAlgo::SetFieldData(ProgressReporter *pr,
				    FieldHandle& field_input_handle,
				    FieldHandle& field_output_handle,
				    NrrdDataHandle& nrrddata_input_handle,
				    bool keepscalartype)
{
  FieldInformation fi(field_input_handle);

  VMesh* imesh = field_input_handle->vmesh();
  VMesh::size_type numnodes = imesh->num_nodes();
  VMesh::size_type numelems = imesh->num_elems();

  std::string output_datatype = "";   

  VMesh::size_type numvals = 0;

  // If structured see if the nrrd looks like the mesh
  VMesh::dimension_type dims;

  imesh->get_dimensions( dims );
  if( dims.size() > 1 ) 
  {
    if( nrrddata_input_handle->nrrd_->dim   == dims.size() ||
      nrrddata_input_handle->nrrd_->dim-1 == dims.size() ) 
    {

      numvals = 1;

      // count number of entries, disregarding vector or tensor
      // components
      for (int d=nrrddata_input_handle->nrrd_->dim-1, m=dims.size()-1; m>=0; d--, m--) 
      {
        numvals *= nrrddata_input_handle->nrrd_->axis[d].size;
      }

      // These are secondary checks and are not really needed but if
      // the data is structured then it should match the mesh.

      // Node check
      if( numvals == numnodes ) 
      {
        fi.make_lineardata();

        // check dimensions
        for (int d=nrrddata_input_handle->nrrd_->dim-1, m=dims.size()-1; m>=0; d--, m--) 
        {
          if (static_cast<Mesh::size_type>(nrrddata_input_handle->nrrd_->axis[d].size) != dims[m]) 
          {
            numvals = 0;
            break;
          }
        }
      }

      // Element check
      else if( numvals == numelems ) 
      {
        fi.make_constantdata();

        // check dimensions
        for (int d=nrrddata_input_handle->nrrd_->dim-1, m=dims.size()-1; m>=0; d--, m--) 
        {
          if (static_cast<Mesh::size_type>(nrrddata_input_handle->nrrd_->axis[d].size) != dims[m]-1) 
          {
            numvals = 0;
            break;
          }
        }
      }
      // No match
      else 
      {
        numvals = 0;
      }
	

      if( numvals ) 
      {
        if( nrrddata_input_handle->nrrd_->dim == dims.size() ||
            nrrddata_input_handle->nrrd_->axis[0].size == 1 ) 
        {
          output_datatype = "Scalar";
    
          if (keepscalartype == false) fi.make_double();
    
        } 
        else if ( nrrddata_input_handle->nrrd_->axis[0].size == 3 ) 
        {
          output_datatype = "Vector";
          fi.make_vector();
        } 
        else if ( nrrddata_input_handle->nrrd_->axis[0].size == 6 ||
                  nrrddata_input_handle->nrrd_->axis[0].size == 7 ||
		              nrrddata_input_handle->nrrd_->axis[0].size == 9 ) 
        {
          output_datatype = "Tensor";
          fi.make_tensor();
        }
      }
    }

  }

  // If unstructured or a single list
  if( output_datatype == "" ) 
  {
    if( nrrddata_input_handle->nrrd_->dim == 1 &&
        (static_cast<Mesh::size_type>(nrrddata_input_handle->nrrd_->axis[0].size) == numnodes ||
         static_cast<Mesh::size_type>(nrrddata_input_handle->nrrd_->axis[0].size) == numelems) ) 
    {

      numvals = nrrddata_input_handle->nrrd_->axis[0].size;

      output_datatype = "Scalar";
    } 
    else if( nrrddata_input_handle->nrrd_->dim == 2 &&
	           (static_cast<Mesh::size_type>(nrrddata_input_handle->nrrd_->axis[1].size) == numnodes ||
		          static_cast<Mesh::size_type>(nrrddata_input_handle->nrrd_->axis[1].size) == numelems) ) 
    {

      numvals = nrrddata_input_handle->nrrd_->axis[1].size;

      if( nrrddata_input_handle->nrrd_->axis[0].size == 1 ) 
      {
        output_datatype = "Scalar";
	    } 
      else if ( nrrddata_input_handle->nrrd_->axis[0].size == 3 ) 
      {
        output_datatype = "Vector";
        fi.make_vector();
      } 
      else if ( nrrddata_input_handle->nrrd_->axis[0].size == 6 )
      {
        output_datatype = "Tensor6";
        fi.make_tensor();        
      }
      else if ( nrrddata_input_handle->nrrd_->axis[0].size == 7 )
      {
        output_datatype = "Tensor7";
        fi.make_tensor();        
      }
      else if ( nrrddata_input_handle->nrrd_->axis[0].size == 9 )
      {
        output_datatype = "Tensor9";
        fi.make_tensor();
      }
    }

    if (numvals == numnodes) fi.make_lineardata();
    if (numvals == numelems) fi.make_constantdata();
  }

  if (output_datatype == "")
  {
    pr->error("SetFieldData: NrrdData dimensions do not match any of the fields dimensions");
    return (false);
  }

  if (output_datatype == "Scalar" && !keepscalartype)
  {
    fi.make_double();
    switch(nrrddata_input_handle->nrrd_->type)
    {
      case nrrdTypeChar:
        fi.make_char();
        break;
      case nrrdTypeUChar:
        fi.make_unsigned_char();
        break;
      case nrrdTypeShort:
        fi.make_short();
        break;
      case nrrdTypeUShort:
        fi.make_unsigned_short();
        break;
      case nrrdTypeInt:
        fi.make_int();
        break;
      case nrrdTypeUInt:
        fi.make_unsigned_int();
        break;
      case nrrdTypeFloat:
        fi.make_float();
        break;
      case nrrdTypeDouble:
        fi.make_double();
        break;
    }    
  }
  else if (output_datatype == "Scalar")
  {
    if (!fi.is_scalar()) fi.make_double();
  }
  
  field_output_handle = CreateField(fi,field_input_handle->mesh());

  if (field_output_handle.get_rep() == 0)
  {
    pr->error("SetFieldData: Could not allocate output field");
    return (false);
  }

  switch(nrrddata_input_handle->nrrd_->type)
    {
    case nrrdTypeChar:
      return(SetFieldDataV<char>(field_output_handle,
				 nrrddata_input_handle,
				 output_datatype));
    case nrrdTypeUChar:
      return(SetFieldDataV<unsigned char>(field_output_handle,
					  nrrddata_input_handle,
					  output_datatype));
    case nrrdTypeShort:
      return(SetFieldDataV<short>(field_output_handle,
				  nrrddata_input_handle,
				  output_datatype));
    case nrrdTypeUShort:
      return(SetFieldDataV<unsigned short>(field_output_handle,
					   nrrddata_input_handle,
					   output_datatype));
    case nrrdTypeInt:
      return(SetFieldDataV<int>(field_output_handle,
				nrrddata_input_handle,
				output_datatype));
    case nrrdTypeUInt:
      return(SetFieldDataV<unsigned int>(field_output_handle,
					 nrrddata_input_handle,
					 output_datatype));
    case nrrdTypeFloat:
      return(SetFieldDataV<float>(field_output_handle,
				  nrrddata_input_handle,
				  output_datatype));
    case nrrdTypeDouble:
      return(SetFieldDataV<double>(field_output_handle,
				   nrrddata_input_handle,
				   output_datatype));
  }

  pr->error("SetFieldData: Nrrd datatype is not supported");
  return (false);
}

} // namespace SCIRunAlgo

