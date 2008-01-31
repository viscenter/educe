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

//    File   : GetSliceFromStructuredFieldByIndices.h
//    Author : Michael Callahan &&
//             Allen Sanderson
//             SCI Institute
//             University of Utah
//    Date   : March 2006
//
//    Copyright (C) 2006 SCI Group


#include <Dataflow/Modules/Fields/GetSliceFromStructuredFieldByIndices.h>
#include <Dataflow/Modules/Fields/SubsampleStructuredFieldByIndices.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Datatypes/DenseMatrix.h>


namespace SCIRun {

class GetSliceFromStructuredFieldByIndices : public Module {
public:
  GetSliceFromStructuredFieldByIndices(GuiContext *context);

  virtual ~GetSliceFromStructuredFieldByIndices();

  virtual void execute();

private:
  GuiInt               gui_axis_;
  GuiInt               gui_dims_;

  GuiInt               gui_dim_i_;
  GuiInt               gui_dim_j_;
  GuiInt               gui_dim_k_;

  GuiInt               gui_index_i_;
  GuiInt               gui_index_j_;
  GuiInt               gui_index_k_;

  //! gui_update_type_ must be declared after all gui vars because !
  //! some are traced in the tcl code. If gui_update_type_ is set to
  //! Auto having it last will prevent the net from executing when it
  //! is instantiated.

  GuiString            gui_update_type_;
  GuiInt               gui_continuous_;
};


DECLARE_MAKER(GetSliceFromStructuredFieldByIndices)


GetSliceFromStructuredFieldByIndices::GetSliceFromStructuredFieldByIndices(GuiContext *context)
  : Module("GetSliceFromStructuredFieldByIndices", context, Filter, "NewField", "SCIRun"),
    
    gui_axis_(context->subVar("axis"), 2),
    gui_dims_(context->subVar("dims"), 3),

    gui_dim_i_(context->subVar("dim-i"), 1),
    gui_dim_j_(context->subVar("dim-j"), 1),
    gui_dim_k_(context->subVar("dim-k"), 1),

    gui_index_i_(context->subVar("index-i"), 1),
    gui_index_j_(context->subVar("index-j"), 1),
    gui_index_k_(context->subVar("index-k"), 1),

    gui_update_type_(context->subVar("update_type"), "Manual"),
    gui_continuous_(context->subVar("continuous"), 0)
{
}


GetSliceFromStructuredFieldByIndices::~GetSliceFromStructuredFieldByIndices()
{
}


void
GetSliceFromStructuredFieldByIndices::execute()
{
  FieldHandle field_in_handle = 0;
  MatrixHandle matrix_handle = 0;

  //! Get the input field handle from the port.
  if( !get_input_handle( "Input Field",  field_in_handle,  true  ) ) return;

  //! Get the optional matrix handle from the port. Note if a matrix is
  //! present it is sent down stream. Otherwise it will be created.
  get_input_handle( "Input Matrix", matrix_handle, false );

  // Because the field slicer is index based it can only work on
  // structured data. For unstructured data SamplePlane should be used.
  if( !(field_in_handle->mesh()->topology_geometry() & Mesh::STRUCTURED) ) 
  {
    error( field_in_handle->get_type_description(Field::MESH_TD_E)->get_name() );
    error( "Only availible for topologically structured data." );
    error( "For topologically unstructured data use SamplePlane." );
    return;
  }

  // For cell based data the max index is one less than the dimension in each direction
  unsigned int offset;

  if( field_in_handle->basis_order() == 0)
    offset = 1;
  else /*if( field_in_handle->basis_order() == 1)*/
    offset = 0;

  // Get the dimensions of the mesh.
  vector<Field::size_type> dims;

  field_in_handle->mesh()->get_dim( dims );
  
  bool update_dims = false;

  if( dims.size() >= 1 ) {
    //! Check to see if the gui dimensions are different than the field.
    if( gui_dim_i_.get() != static_cast<int>(dims[0]-offset) )
    {
      gui_dim_i_.set( static_cast<int>(dims[0]-offset) );
      update_dims = true;
    }
  }

  if( dims.size() >= 2 ) {
    //! Check to see if the gui dimensions are different than the field.
    if( gui_dim_j_.get() != static_cast<int>(dims[1]-offset) ) 
    {
      gui_dim_j_.set( static_cast<int>(dims[1]-offset) );
      update_dims = true;
    }
  }

  if( dims.size() >= 3 ) {
    //! Check to see if the gui dimensions are different than the field.
    if( gui_dim_k_.get() != static_cast<int>(dims[2]-offset) ) 
    {
      gui_dim_k_.set( static_cast<int>(dims[2]-offset) );
      update_dims = true;
    }
  }

  //! Check to see if the gui dimensions are different than the field.
  //! This is last because the GUI var has a callback on it.
  if( gui_dims_.get() != static_cast<int>(dims.size()) ) 
  {
    gui_dims_.set( dims.size() );
    update_dims = true;
  }

  //! If the gui dimensions are different than the field then update the gui.
  if( update_dims ) 
  {
    //! Dims has callback on it, so it must be set it after i, j, and k.
    ostringstream str;
    str << get_id() << " set_size ";
    get_gui()->execute(str.str().c_str());

    reset_vars();
  }

  //! An input matrix is present so use the values in it to override
  //! the variables set in the gui.
  //! Column 0 selected axis to slice.
  //! Column 1 index of the axis to slice.
  //! Column 2 dimensions of the data.
  if( matrix_handle.get_rep() ) 
  {
    if( (matrix_handle->nrows() == 1 && matrix_handle->ncols() == 1) ) 
    {
      //! Check to see what index has been selected.
      if( gui_axis_.get() == 0 )
      {
        gui_index_i_.set( (int) matrix_handle->get(0, 0) );
        gui_index_i_.reset();
      }

      if( gui_axis_.get() == 1 )
      {
        gui_index_j_.set( (int) matrix_handle->get(0, 0) );
        gui_index_j_.reset();
      }

      if( gui_axis_.get() == 2 )
      {
        gui_index_k_.set( (int) matrix_handle->get(0, 0) );
        gui_index_k_.reset();
      }

      ostringstream str;
      str << get_id() << " update_index ";
      
      get_gui()->execute(str.str().c_str());
	
      reset_vars();
	
      inputs_changed_ = true;
    }
   
    //! The matrix is optional. If present make sure it is a 3x3 matrix.
    //! The row indices is the axis index. The column is the data.
    else if( (matrix_handle->nrows() == 3 &&
	      matrix_handle->ncols() == 3) ) 
    {
      //! Sanity check. Make sure the gui dimensions match the matrix
      //! dimensions.
      if( gui_dim_i_.get() != matrix_handle->get(0, 2) ||
          gui_dim_j_.get() != matrix_handle->get(1, 2) ||
          gui_dim_k_.get() != matrix_handle->get(2, 2) ) 
      {
        ostringstream str;
        str << "The dimensions of the matrix slicing do match the field. "
            << " Expected "
            << gui_dim_i_.get() << " "
            << gui_dim_j_.get() << " "
            << gui_dim_k_.get()
            << " Got "
            << matrix_handle->get(0, 2) << " "
            << matrix_handle->get(1, 2) << " "
            << matrix_handle->get(2, 2);
	
        error( str.str() );
        return;
      }

      //! Check to see what axis has been selected. Only one should be
      //! selected.
      for (Matrix::index_type i=0; i < matrix_handle->nrows(); i++) 
      {
        if( matrix_handle->get(i, 0) == 1 && gui_axis_.get() != i ) 
        {
          gui_axis_.set( i );	
          inputs_changed_ = true;
        }
      }
      
      //! Check to see what index has been selected and if it matches
      //! the gui index.
      if( gui_index_i_.get() != matrix_handle->get(0, 1) ||
          gui_index_j_.get() != matrix_handle->get(1, 1) ||
          gui_index_k_.get() != matrix_handle->get(2, 1) ) 
      {
        gui_index_i_.set( (int) matrix_handle->get(0, 1) );
        gui_index_j_.set( (int) matrix_handle->get(1, 1) );
        gui_index_k_.set( (int) matrix_handle->get(2, 1) );
        
        gui_index_i_.reset();
        gui_index_j_.reset();
        gui_index_k_.reset();

        ostringstream str;
        str << get_id() << " update_index ";
        
        get_gui()->execute(str.str().c_str());
        
        reset_vars();
        
        inputs_changed_ = true;
      }
    }
    else
    {
      error( "Input matrix is not a 1x1 or a 3x3 matrix" );
      return;
    }
  }

  //! If no data or an input change recreate the field. I.e Only
  //! execute when neeed.
  if( inputs_changed_  ||
      !oport_cached("Output Field") ||
      !oport_cached("Output Matrix") ||
      gui_axis_.changed( true ) ||
      (gui_axis_.get() == 0 && gui_index_i_.changed( true )) ||
      (gui_axis_.get() == 1 && gui_index_j_.changed( true )) ||
      (gui_axis_.get() == 2 && gui_index_k_.changed( true )) ) 
  {

    // Update the state. Other state changes are handled in either
    // getting handles or in the calling method Module::do_execute.
    update_state(Executing);

    //! Create the new field via a dynamically compiled algorithm.
    const TypeDescription *ftd = field_in_handle->get_type_description();
    const TypeDescription *ttd =
      field_in_handle->get_type_description(Field::FDATA_TD_E);

    if( field_in_handle->basis_order() == 0 )
    {
      //! Create the new field via a dynamically compiled algorithm.
      const TypeDescription *ftd = field_in_handle->get_type_description();
      const bool geom_irreg =
	(field_in_handle->mesh()->topology_geometry() & Mesh::IRREGULAR);

      CompileInfoHandle ci =
	SubsampleStructuredFieldByIndicesAlgo::get_compile_info(ftd,geom_irreg);

      Handle<SubsampleStructuredFieldByIndicesAlgo> workalgo;
      if (!module_dynamic_compile(ci, workalgo)) return;

      unsigned int start_i = 0;
      unsigned int start_j = 0;
      unsigned int start_k = 0;

      unsigned int stop_i = gui_dim_i_.get();
      unsigned int stop_j = gui_dim_i_.get();
      unsigned int stop_k = gui_dim_i_.get();

      unsigned int stride_i = 1;
      unsigned int stride_j = 1;
      unsigned int stride_k = 1;

      unsigned int wrap_i = 0;
      unsigned int wrap_j = 0;
      unsigned int wrap_k = 0;
    
      //! Get the index for the axis selected.
      if (gui_axis_.get() == 0) 
      {
	start_i = gui_index_i_.get();
	stop_i  = gui_index_i_.get() + 1;
      } 
      else if (gui_axis_.get() == 1) 
      {
	start_j = gui_index_j_.get();
	stop_j  = gui_index_j_.get() + 1;
      } 
      else 
      {
	start_k = gui_index_k_.get();
	stop_k  = gui_index_k_.get() + 1;
      }

      FieldHandle field_out_handle;

      workalgo->execute(field_in_handle, field_out_handle,
 			start_i,  start_j,  start_k,
 			stop_i,   stop_j,   stop_k,
 			stride_i, stride_j, stride_k,
 			wrap_i,   wrap_j,   wrap_k);

      // Send the data downstream
      send_output_handle( "Output Field", field_out_handle );
    }
    else /*if( field_in_handle->basis_order() == 1)*/
    {
      CompileInfoHandle ci =
	GetSliceFromStructuredFieldByIndicesAlgo::get_compile_info(ftd,ttd);
      
      Handle<GetSliceFromStructuredFieldByIndicesAlgo> algo;
      if (!module_dynamic_compile(ci, algo)) return;

      FieldHandle field_out_handle = algo->execute(field_in_handle, gui_axis_.get());

      //! Get the index for the axis selected.
      unsigned int index;
      if (gui_axis_.get() == 0) 
      {
	index = gui_index_i_.get();
      } 
      else if (gui_axis_.get() == 1) 
      {
	index = gui_index_j_.get();
      } 
      else 
      {
	index = gui_index_k_.get();
      }

      //! Now the new field is defined so slice the field via a
      //! dynamically compiled algorithm.
      const TypeDescription *iftd = field_in_handle->get_type_description();
      const TypeDescription *oftd = field_out_handle->get_type_description();
      const bool geom_irreg =
	(field_in_handle->mesh()->topology_geometry() & Mesh::IRREGULAR);
      
      ci =
	GetSliceFromStructuredFieldByIndicesWorkAlgo::get_compile_info(iftd,oftd,geom_irreg);
      Handle<GetSliceFromStructuredFieldByIndicesWorkAlgo> workalgo;
      
      if (!module_dynamic_compile(ci, workalgo)) return;
      
      workalgo->execute( field_in_handle, field_out_handle,
			 index, gui_axis_.get() );
      
      // Send the data downstream.
      send_output_handle( "Output Field", field_out_handle );
    }

    //! Create the output matrix with the axis selected, index, and
    //! dimensions.
    if( matrix_handle == 0 ) 
    {
      DenseMatrix *selected = scinew DenseMatrix(3,3);

      for (int i=0; i < 3; i++)
        selected->put(i, 0, (double) (gui_axis_.get() == i) );

      selected->put(0, 1, gui_index_i_.get() );
      selected->put(1, 1, gui_index_j_.get() );
      selected->put(2, 1, gui_index_k_.get() );

      selected->put(0, 2, gui_dim_i_.get() );
      selected->put(1, 2, gui_dim_j_.get() );
      selected->put(2, 2, gui_dim_k_.get() );

      matrix_handle = MatrixHandle(selected);

      // Send the data downstream.
      send_output_handle( "Output Matrix", matrix_handle );
    }
  }
}


CompileInfoHandle
GetSliceFromStructuredFieldByIndicesAlgo::get_compile_info(const TypeDescription *ftd,
				  const TypeDescription *ttd)
{
  //! use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string template_class_name("GetSliceFromStructuredFieldByIndicesAlgoT");
  static const string base_class_name("GetSliceFromStructuredFieldByIndicesAlgo");

  TypeDescription::td_vec *tdv = ttd->get_sub_type();
  string odat = (*tdv)[0]->get_name();
  CompileInfo *rval = 
    scinew CompileInfo(template_class_name + "." +
		       ftd->get_filename() + "." +
		       ttd->get_filename() + ".",
                       base_class_name, 
                       template_class_name, 
                       ftd->get_name() + ", " +
		       odat );

  //! Add in the include path to compile this obj
  rval->add_include(include_path);
  rval->add_basis_include("Core/Basis/QuadBilinearLgn.h");
  rval->add_basis_include("Core/Basis/CrvLinearLgn.h");
  rval->add_basis_include("Core/Basis/Constant.h");
  rval->add_basis_include("Core/Basis/NoData.h");

  rval->add_mesh_include("Core/Datatypes/StructCurveMesh.h");
  rval->add_mesh_include("Core/Datatypes/StructQuadSurfMesh.h");
  rval->add_mesh_include("Core/Datatypes/PointCloudMesh.h");

  ftd->fill_compile_info(rval);
  return rval;
}

CompileInfoHandle
GetSliceFromStructuredFieldByIndicesWorkAlgo::get_compile_info(const TypeDescription *iftd,
				      const TypeDescription *oftd,
				      bool geometry_irregular)
{
  //! use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string template_class_name("GetSliceFromStructuredFieldByIndicesWorkAlgoT");
  static const string base_class_name("GetSliceFromStructuredFieldByIndicesWorkAlgo");

  CompileInfo *rval = 
    scinew CompileInfo(template_class_name + "." +
		       iftd->get_filename() + "." +
		       oftd->get_filename() + ".",
                       base_class_name, 
                       template_class_name, 
                       iftd->get_name() + ", " +
		       oftd->get_name() );

  //! Add in the include path to compile this obj
  if(geometry_irregular)
    rval->add_pre_include( "#define SET_POINT_DEFINED 1");

  rval->add_include(include_path);  

  iftd->fill_compile_info(rval);
  return rval;
}

} //! End namespace SCIRun
