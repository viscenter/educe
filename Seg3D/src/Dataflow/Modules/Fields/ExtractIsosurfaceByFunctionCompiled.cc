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

//    File   : ExtractIsosurfaceByFunctionCompiled.h
//    Author : Michael Callahan &&
//             Allen Sanderson
//             SCI Institute
//             University of Utah
//    Date   : March 2006
//
//    Copyright (C) 2006 SCI Group


#include <Core/Algorithms/Fields/Mapping/ApplyMappingMatrix.h>
#include <Core/Algorithms/Fields/ClipFieldByFunction.h>
#include <Core/Algorithms/Fields/FieldData/ConvertFieldBasisType.h>
#include <Core/Algorithms/Fields/MarchingCubes/MarchingCubes.h>

#include <Core/Algorithms/Fields/FieldsAlgo.h>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixOperations.h>

#include <Core/Containers/StringUtil.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

#include <Dataflow/Modules/Fields/share.h>

namespace SCIRun {

class SCISHARE ExtractIsosurfaceByFunctionCompiled : public Module {
  public:
    ExtractIsosurfaceByFunctionCompiled(GuiContext *context);
    virtual ~ExtractIsosurfaceByFunctionCompiled() {}

    virtual void execute();

  private:
    FieldHandle field_transformed_handle_;
    FieldHandle field_output_handle_;

    //! GUI variables
    GuiString  gui_function_;
    GuiInt     gui_zero_checks_;

    GuiDouble  gui_slice_value_min_;
    GuiDouble  gui_slice_value_max_;
    GuiDouble  gui_slice_value_;
    GuiDouble  gui_slice_value_typed_;
    GuiInt     gui_slice_value_quantity_;
    GuiString  gui_slice_quantity_range_;
    GuiString  gui_slice_quantity_clusive_;
    GuiDouble  gui_slice_quantity_min_;
    GuiDouble  gui_slice_quantity_max_;
    GuiString  gui_slice_quantity_list_;
    GuiString  gui_slice_value_list_;
    GuiString  gui_slice_matrix_list_;
    GuiInt     gui_use_algorithm_;
    GuiString  gui_active_slice_value_selection_tab_;
    GuiString  gui_active_tab_;

    //gui_update_type_ must be declared after gui_iso_value_max_ which is
    //traced in the tcl code. If gui_update_type_ is set to Auto having it
    //last will prevent the net from executing when it is instantiated.
    GuiString  gui_update_type_;

    //! status variables
    vector< double > slicevals_;
};


DECLARE_MAKER(ExtractIsosurfaceByFunctionCompiled)


ExtractIsosurfaceByFunctionCompiled::ExtractIsosurfaceByFunctionCompiled(GuiContext *context)
  : Module("ExtractIsosurfaceByFunctionCompiled", context, Filter, "NewField", "SCIRun"),
    
    field_transformed_handle_(0),
    field_output_handle_(0),

    gui_function_(get_ctx()->subVar("function"), "result = sqrt(x*x + y*y);"),
    gui_zero_checks_(context->subVar("zero-checks"), 0),
    gui_slice_value_min_(context->subVar("slice-value-min"),  0.0),
    gui_slice_value_max_(context->subVar("slice-value-max"), 99.0),
    gui_slice_value_(context->subVar("slice-value"), 0.0),
    gui_slice_value_typed_(context->subVar("slice-value-typed"), 0.0),
    gui_slice_value_quantity_(context->subVar("slice-value-quantity"), 1),
    gui_slice_quantity_range_(context->subVar("quantity-range"), "field"),
    gui_slice_quantity_clusive_(context->subVar("quantity-clusive"), "exclusive"),
    gui_slice_quantity_min_(context->subVar("quantity-min"),   0),
    gui_slice_quantity_max_(context->subVar("quantity-max"), 100),
    gui_slice_quantity_list_(context->subVar("quantity-list"), ""),
    gui_slice_value_list_(context->subVar("slice-value-list"), "No values present."),
    gui_slice_matrix_list_(context->subVar("matrix-list"),"No matrix present - execution needed."),
    gui_use_algorithm_(context->subVar("algorithm"), 0),
    gui_active_slice_value_selection_tab_(context->subVar("active-slice-value-selection-tab"), "0"),
    gui_active_tab_(context->subVar("active_tab"), "0"),
    gui_update_type_(context->subVar("update_type"), "On Release")
{
}


void
ExtractIsosurfaceByFunctionCompiled::execute()
{
  //! Get the input field handle from the port.
  FieldHandle input_field_handle;
  get_input_handle( "Input Field", input_field_handle, true);

  //! Current
  int basis_order = input_field_handle->vfield()->basis_order();
  if( basis_order != 0 && basis_order != 1)
  {
    error( "Can not extract an isosurface with higher order data by function because currently it is not poosible to build a mapping matrix for the data which is needed." );
    return;
  }

  // update gFunction_ before get.
  get_gui()->execute(get_id() + " update_text");

  /////////////////////////////////////////////////////////////////////////////
  //! Transform Section

  if( gui_function_.changed( true ) )
    inputs_changed_ = true;

  //! Check to see if the input field has changed.
  if( inputs_changed_ || !field_transformed_handle_.get_rep() ) 
  {
    // Delete the saved field.
    field_transformed_handle_ = 0;

    string function = gui_function_.get();

    //! See if a trig function was used. If so then a secondary check
    //should be done to remove erroneous results due to a through zero
    //interpolation. I.e. 0 == 2 * Pi which can result in an
    //interpolation in between which is erroneous.
    gui_zero_checks_.set( function.find( "sin" ) != string::npos ||
			  function.find( "cos" ) != string::npos ||
			  function.find( "tan" ) != string::npos );

    FieldHandle tmp_field_handle;

    // If the basis is not linear make it so.
    if( input_field_handle->basis_order() == 0 )
    {
      SCIRunAlgo::ConvertFieldBasisTypeAlgo algo;
      algo.set_progress_reporter(this);
      algo.set_option("basistype", "linear");

      FieldHandle output_field_handle;
      MatrixHandle mapping_matrix_handle;

      if (!(algo.run(input_field_handle,tmp_field_handle,mapping_matrix_handle))) return;
    }
    else // if( input_field_handle->basis_order() == 1 )
    {
      tmp_field_handle = input_field_handle;
    }
    
    std::vector<FieldHandle> input_field_handles;
    input_field_handles.push_back( tmp_field_handle );

    unsigned int count;

    string outputDataType("double");

    // NEED TO FIX THIS ONE
    SCIRunAlgo::FieldsAlgo algo(this);
    if (!(algo.CalculateFieldDataCompiled(input_field_handles,
					  field_transformed_handle_,
					  outputDataType,
					  function,
					  count))) return;
    ///////

    if (!(field_transformed_handle_->vfield()->is_scalar())) 
    {
      error("Transformed field does not contain scalar data.");
      return;
    }

    double minval, maxval;
    field_transformed_handle_->vfield()->minmax(minval,maxval);

    // Check to see if the gui min max are different than the field.
    if( gui_slice_value_min_.get() != minval ||
        gui_slice_value_max_.get() != maxval ) 
    {

      gui_slice_value_min_.set( minval );
      gui_slice_value_max_.set( maxval );

      gui_slice_value_min_.reset();
      gui_slice_value_max_.reset();

      ostringstream str;
      str << get_id() << " set_min_max ";
      get_gui()->execute(str.str().c_str());
    }
  }

  if( !field_transformed_handle_.get_rep() ) 
  {
    error( "Can not find the transformed field for slicing" );
    return;
  } else {
    //remark( "Transformed field for slicing is present" );
  }

  /////////////////////////////////////////////////////////////////////////////
  //! Isosurface Section

  vector<double> slicevals(0);

  double qmin = gui_slice_value_min_.get();
  double qmax = gui_slice_value_max_.get();

  if (gui_active_slice_value_selection_tab_.get() == "0") 
  { // slider / typed
    const double val = gui_slice_value_.get();
    const double valTyped = gui_slice_value_typed_.get();
    if (val != valTyped) 
    {
      char s[1000];
      sprintf(s, "Typed slice value %g was out of range.  Using slice value %g instead.", valTyped, val);
      warning(s);
      gui_slice_value_typed_.set(val);
    }
    if ( qmin <= val && val <= qmax )
    {
      slicevals.push_back(val);
    }
    else 
    {
      error("Typed slice value out of range -- skipping slice surfacing.");
      return;
    }
  } 
  else if (gui_active_slice_value_selection_tab_.get() == "1") 
  { // quantity
    int num = gui_slice_value_quantity_.get();

    if (num < 1) 
    {
      error("Slice surface quantity must be at least one -- skipping slice surfacing.");
      return;
    }

    string range = gui_slice_quantity_range_.get();

    if (range == "manual") 
    {
      qmin = gui_slice_quantity_min_.get();
      qmax = gui_slice_quantity_max_.get();
    } // else we're using "field" and qmax and qmin were set above
    
    if (qmin >= qmax) 
    {
      error("Can't use quantity tab if the minimum and maximum are the same.");
      return;
    }

    string clusive = gui_slice_quantity_clusive_.get();
    ostringstream str;
    str << get_id() << " set-slice-quant-list \"";

    if (clusive == "exclusive") 
    {
      // if the min - max range is 2 - 4, and the user requests 3 slicevals,
      // the code below generates 2.333, 3.0, and 3.666 -- which is nice
      // since it produces evenly spaced slices in torroidal data.
	
      double di=(qmax - qmin)/(double)num;
      for (int i=0; i<num; i++) 
      {
        slicevals.push_back(qmin + ((double)i+0.5)*di);
        str << " " << slicevals[i];
      }
    } 
    else if (clusive == "inclusive") 
    {
      // if the min - max range is 2 - 4, and the user requests 3 slicevals,
      // the code below generates 2.0, 3.0, and 4.0.

      double di=(qmax - qmin)/(double)(num-1.0);
      for (int i=0; i<num; i++) 
      {
        slicevals.push_back(qmin + ((double)i*di));
        str << " " << slicevals[i];
      }
    }

    str << "\"";

    get_gui()->execute(str.str().c_str());

  } 
  else if (gui_active_slice_value_selection_tab_.get() == "2") 
  { // list
    istringstream vlist(gui_slice_value_list_.get());
    double val;
    while(!vlist.eof()) 
    {
      vlist >> val;
      if (vlist.fail()) 
      {
        if (!vlist.eof()) 
        {
          vlist.clear();
          warning("List of Slicevals was bad at character " +
            to_string((int)(vlist.tellg())) +
            "('" + ((char)(vlist.peek())) + "').");
        }
        break;
      }
      else if (!vlist.eof() && vlist.peek() == '%') 
      {
        vlist.get();
        val = qmin + (qmax - qmin) * val / 100.0;
      }
      slicevals.push_back(val);
    }
  } 
  else if (gui_active_slice_value_selection_tab_.get() == "3") 
  { // matrix

    MatrixHandle matrix_input_handle;
    get_input_handle( "Optional Slice values", matrix_input_handle, true );

    ostringstream str;
    str << get_id() << " set-slice-matrix-list \"";

    for (int i=0; i < matrix_input_handle->nrows(); i++) 
    {
      for (int j=0; j < matrix_input_handle->ncols(); j++) 
      {
        slicevals.push_back(matrix_input_handle->get(i, j));
        str << " " << slicevals[i];
      }
    }

    str << "\"";
    get_gui()->execute(str.str().c_str());
  } 
  else 
  {
    error("Bad active_slice_value_selection_tab value");
    return;
  }

  // See if any of the slicevals have changed.
  if( slicevals_.size() != slicevals.size()) 
  {
    slicevals_.resize( slicevals.size() );
    inputs_changed_ = true;
  }

  for( size_t i=0; i<slicevals.size(); i++ ) 
  {
    if( slicevals_[i] != slicevals[i] ) 
    {
      slicevals_[i] = slicevals[i];
      inputs_changed_ = true;
    }
  }

  if( !field_output_handle_.get_rep() || gui_zero_checks_.changed() ||
      inputs_changed_  ) 
  {
    SCIRunAlgo::MarchingCubesAlgo mc;
    mc.set_progress_reporter(this);
    mc.set_bool("build_field",true);
    if (input_field_handle->vfield()->basis_order() == 0)
    {
      mc.set_bool("build_node_interpolant",false);
      mc.set_bool("build_elem_interpolant",true);    
    }
    else
    {
      mc.set_bool("build_node_interpolant",true);
      mc.set_bool("build_elem_interpolant",false);
    }
    
    FieldHandle   field_sliced_handle = 0;
    MatrixHandle matrix_sliced_handle = 0;
    
    if(!(mc.run(field_transformed_handle_,slicevals,
              field_sliced_handle,matrix_sliced_handle))) 
    {
      error("MarchingCubes algorithm failed");
      return;
    }

    if (field_sliced_handle.get_rep())
    {
      std::string fldname;
      if (!(field_transformed_handle_->get_property("name", fldname))) 
      {      
        fldname = "Sliced Surface";
      }
      field_sliced_handle->set_property("name", fldname, false);
    }
    else
    {
      //! No field.
      warning( "No slices found" );
      return;
    }
    
    // If the basis is not constant make it so.
    if( input_field_handle->vfield()->basis_order() == 0 )
    {
      SCIRunAlgo::ConvertFieldBasisTypeAlgo algo;
      algo.set_progress_reporter(this);
      algo.set_option("basistype", "constant");
      
      FieldHandle tmp_field_handle = field_sliced_handle;
      field_sliced_handle = 0;
      MatrixHandle mapping_matrix_handle;

      if (!(algo.run(tmp_field_handle,
               field_sliced_handle,
               mapping_matrix_handle))) return;
    }

    if( !field_sliced_handle.get_rep() )
    {
      error( "Can not find the sliced field for clipping" );
      return;
    } 
    else if( !matrix_sliced_handle.get_rep() )
    {
      error( "Can not find the matrix for clipping" );
      return;
    } 
    else 
    {
      remark( "Sliced field and matrix for clipping is present" );
    }

    ///////////////////////////////////////////////////////////////////////////
    //! Apply Mapping Section 1

    //! Apply the matrix to the sliced data.
    FieldHandle field_apply_mapping_handle;
    
    SCIRunAlgo::ApplyMappingMatrixAlgo mapping_algo;
    mapping_algo.set_progress_reporter(this);
    
    if(!(mapping_algo.run(input_field_handle,field_sliced_handle,
                     matrix_sliced_handle,field_apply_mapping_handle)))
    {
      error( "Can not find the first mapped field for the second mapping" );
      return;
    }

    if( !gui_zero_checks_.get () ) 
    {
      field_output_handle_ = field_apply_mapping_handle;
      if (oport_connected("Mapping"))
      {
        MatrixHandle Mapping = matrix_sliced_handle;
        send_output_handle("Mapping",Mapping);
      }
    } 
    else 
    {

      /////////////////////////////////////////////////////////////////////////
      //! Clip By Function Section

      string mode("allnodes");
      string function = gui_function_.get();

      string::size_type pos = function.find("result");
      if( pos != string::npos ) function.erase( pos, 6);

      pos = function.find("=");
      if( pos != string::npos ) function.erase( pos, 1);

      pos = function.find_last_of(";");
      if( pos != string::npos ) function.erase( pos, 1);

      //! remove trailing white-space from the function string
      while (function.size() && isspace(function[function.size()-1]))
        function.resize(function.size()-1);

      //! Add the test to the function
      function = string("return (fabs( (") + function + string(") - v) < 1e-2);" );

      vector< FieldHandle> field_sliced_handles;

      field_sliced_handles.push_back( field_sliced_handle );

      FieldHandle field_clipped_handle;
      MatrixHandle matrix_clipped_handle;


      SCIRunAlgo::FieldsAlgo algo(this);
      
      if (!(algo.ClipFieldByFunction(field_sliced_handles,
				     field_clipped_handle,
				     matrix_clipped_handle,
				     mode,
				     function))) return;


      if( !field_clipped_handle.get_rep() ||
        !matrix_clipped_handle.get_rep() ) 
      {
        error( "Can not find the clipped field and/or matrix for the second mapping" );
        return;
      } 
      else 
      {
        //remark( "Clipped field and matrix for the second mapping are present" );
      }

      /////////////////////////////////////////////////////////////////////////
      //! Apply Mapping Section 2

      mapping_algo.run(field_apply_mapping_handle,field_clipped_handle,
                       matrix_clipped_handle,field_output_handle_);

      if (oport_connected("Mapping"))
      {
        MatrixHandle Mapping = matrix_sliced_handle*matrix_clipped_handle;
        send_output_handle("Mapping",Mapping);
      }
    }
  }

  //! Send the data downstream.
  send_output_handle( "Output Field", field_output_handle_, true );
}

} //! End namespace SCIRun
