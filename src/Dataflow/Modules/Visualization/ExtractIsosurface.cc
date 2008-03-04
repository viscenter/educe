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

//    File   : ExtractIsosurface.cc
//    Author : Yarden Livnat
//    Date   : Fri Jun 15 16:38:02 2001


#include <Core/Algorithms/Visualization/ExtractIsosurface.h>

#include <Core/Geom/GeomGroup.h>
#include <Core/Geom/Material.h>
#include <Core/Containers/StringUtil.h>
#include <Core/Datatypes/FieldInterface.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/ColorMapPort.h>
#include <Dataflow/Network/Ports/GeometryPort.h>

#include <Core/Algorithms/Visualization/MarchingCubes.h>
#include <Core/Algorithms/Visualization/Noise.h>
#include <Core/Algorithms/Visualization/Sage.h>
#include <Core/Algorithms/Visualization/TetMC.h>
#include <Core/Algorithms/Visualization/HexMC.h>

#ifdef _WIN32
#  undef SCISHARE
#  define SCISHARE __declspec(dllexport)
#else
#  define SCISHARE
#endif

namespace SCIRun {

class ExtractIsosurface : public Module {

public:
  ExtractIsosurface(GuiContext* ctx);
  virtual ~ExtractIsosurface();
  virtual void execute();

private:
  GeomHandle   geometry_output_handle_;

  //! GUI variables
  GuiDouble  gui_iso_value_min_;
  GuiDouble  gui_iso_value_max_;
  GuiDouble  gui_iso_value_;
  GuiDouble  gui_iso_value_typed_;
  GuiInt     gui_iso_value_quantity_;
  GuiString  gui_iso_quantity_range_;
  GuiString  gui_iso_quantity_clusive_;
  GuiDouble  gui_iso_quantity_min_;
  GuiDouble  gui_iso_quantity_max_;
  GuiString  gui_iso_quantity_list_;
  GuiString  gui_iso_value_list_;
  GuiString  gui_iso_matrix_list_;
  GuiInt     gui_use_algorithm_;
  GuiInt     gui_build_field_;
  GuiInt     gui_build_geom_;
  GuiInt     gui_transparency_;
  GuiInt     gui_np_;          
  GuiString  gui_active_isoval_selection_tab_;
  GuiString  gui_active_tab_; 
  //gui_update_type_ must be declared after gui_iso_value_max_ which is
  //traced in the tcl code. If gui_update_type_ is set to Auto having it
  //last will prevent the net from executing when it is instantiated.
  GuiString  gui_update_type_;

  GuiDouble  gui_color_r_;
  GuiDouble  gui_color_g_;
  GuiDouble  gui_color_b_;
  GuiDouble  gui_color_a_;

  //! status variables
  vector< double > isovals_;
};

DECLARE_MAKER(ExtractIsosurface)


ExtractIsosurface::ExtractIsosurface(GuiContext* context) : 
  Module("ExtractIsosurface", context, Filter, "Visualization", "SCIRun"), 

  geometry_output_handle_(0),

  gui_iso_value_min_(context->subVar("isoval-min"),  0.0),
  gui_iso_value_max_(context->subVar("isoval-max"), 99.0),
  gui_iso_value_(context->subVar("isoval"), 0.0),
  gui_iso_value_typed_(context->subVar("isoval-typed"), 0.0),
  gui_iso_value_quantity_(context->subVar("isoval-quantity"), 1),
  gui_iso_quantity_range_(context->subVar("quantity-range"), "field"),
  gui_iso_quantity_clusive_(context->subVar("quantity-clusive"), "exclusive"),
  gui_iso_quantity_min_(context->subVar("quantity-min"),   0),
  gui_iso_quantity_max_(context->subVar("quantity-max"), 100),
  gui_iso_quantity_list_(context->subVar("quantity-list"), ""),
  gui_iso_value_list_(context->subVar("isoval-list"), "No values present."),
  gui_iso_matrix_list_(context->subVar("matrix-list"),
		       "No matrix present - execution needed."),
  gui_use_algorithm_(context->subVar("algorithm"), 0),
  gui_build_field_(context->subVar("build_trisurf"), 1),
  gui_build_geom_(context->subVar("build_geom"), 1),
  gui_transparency_(context->subVar("transparency"), 0),
  gui_np_(context->subVar("np"), 1),
  gui_active_isoval_selection_tab_(context->subVar("active-isoval-selection-tab"),
				   "0"),
  gui_active_tab_(context->subVar("active_tab"), "0"),
  gui_update_type_(context->subVar("update_type"), "On Release"),
  gui_color_r_(context->subVar("color-r"), 0.4),
  gui_color_g_(context->subVar("color-g"), 0.2),
  gui_color_b_(context->subVar("color-b"), 0.9),
  gui_color_a_(context->subVar("color-a"), 0.9)
{
}


ExtractIsosurface::~ExtractIsosurface()
{
}


static MatrixHandle
append_sparse_matrices(vector<MatrixHandle> &matrices)
{
  Matrix::index_type j;

  Matrix::size_type ncols = matrices[0]->ncols();
  Matrix::size_type nrows = 0;
  Matrix::size_type nnz = 0;
  for (size_t i = 0; i < matrices.size(); i++) {
    SparseRowMatrix *sparse = matrices[i]->sparse();
    nrows += sparse->nrows();
    nnz += sparse->nnz;
  }

  Matrix::index_type *rr = scinew Matrix::index_type[nrows+1];
  Matrix::index_type *cc = scinew Matrix::index_type[nnz];
  double *dd = scinew double[nnz];

  Matrix::index_type offset = 0;
  Matrix::index_type nnzcounter = 0;
  Matrix::index_type rowcounter = 0;
  for (size_t i = 0; i < matrices.size(); i++) {
    SparseRowMatrix *sparse = matrices[i]->sparse();
    for (j = 0; j < sparse->nnz; j++) {
      cc[nnzcounter] = sparse->columns[j];
      dd[nnzcounter] = sparse->a[j];
      nnzcounter++;
    }
    const Matrix::size_type snrows = sparse->nrows();
    for (j = 0; j <= snrows; j++) {
      rr[rowcounter] = sparse->rows[j] + offset;
      rowcounter++;
    }
    rowcounter--;
    offset += sparse->rows[snrows];
  }

  return scinew SparseRowMatrix(nrows, ncols, rr, cc, nnz, dd);
}


void
ExtractIsosurface::execute()
{
  if( gui_build_field_.get() && get_oport("Surface")->nconnections() == 0)
    warning("Asking for a field to be extracted, but there is no output field port connection -- you may want to deselect the Build Output Field checkbox to increase performance.");
  else if( !gui_build_field_.get() && get_oport("Surface")->nconnections() )
      warning("An output field port connection was found but no field is being extracted");

  if( gui_build_geom_.get() && get_oport("Geometry")->nconnections() == 0)
      warning("Asking for geometry to be extracted, but there is no output geometry port connection -- you may want to deselect the Build Output Geometry checkbox to increase performance.");
  else if( !get_oport("Geometry")->nconnections() && get_oport("Geometry")->nconnections() )
      warning("An output geometry port connection was found but no geometry is being extracted");

  FieldHandle field_input_handle;
  if( !get_input_handle( "Field", field_input_handle, true ) ) return;

  // Check to see if the input field has changed.
  if( inputs_changed_ )
  {
    pair<double, double> minmax;
    field_input_handle->vfield()->minmax(minmax.first, minmax.second);


    // Check to see if the gui min max are different than the field.
    if( gui_iso_value_min_.get() != minmax.first ||
        gui_iso_value_max_.get() != minmax.second )
    {
      gui_iso_value_min_.set( minmax.first );
      gui_iso_value_max_.set( minmax.second );

      ostringstream str;
      str << get_id() << " set_min_max ";
      get_gui()->execute(str.str());
    }
  }


  // Get the optional colormap for the geometry.
  ColorMapHandle colormap_input_handle = 0;
  get_input_handle( "Optional Color Map", colormap_input_handle, false );
  
  vector<double> isovals(0);

  double qmin = gui_iso_value_min_.get();
  double qmax = gui_iso_value_max_.get();

  if (gui_active_isoval_selection_tab_.get() == "0") { // slider / typed
    const double val = gui_iso_value_.get();
    const double valTyped = gui_iso_value_typed_.get();
    if (val != valTyped) {
      char s[1000];
      sprintf(s, "Typed isovalue %g was out of range.  Using isovalue %g instead.", valTyped, val);
      warning(s);
      gui_iso_value_typed_.set(val);
    }
    if ( qmin <= val && val <= qmax )
      isovals.push_back(val);
    else {
      error("Typed isovalue out of range -- skipping isosurfacing.");
      return;
    }
  } else if (gui_active_isoval_selection_tab_.get() == "1") { // quantity
    int num = gui_iso_value_quantity_.get();

    if (num < 1) {
      error("ExtractIsosurface quantity must be at least one -- skipping isosurfacing.");
      return;
    }

    string range = gui_iso_quantity_range_.get();

    if (range == "colormap") {
      if (colormap_input_handle.get_rep() ) {
	error("No color colormap for isovalue quantity");
	return;
      }
      qmin = colormap_input_handle->getMin();
      qmax = colormap_input_handle->getMax();
    } else if (range == "manual") {
      qmin = gui_iso_quantity_min_.get();
      qmax = gui_iso_quantity_max_.get();
    } // else we're using "field" and qmax and qmin were set above
    
    if (qmin >= qmax) {
      error("Can't use quantity tab if the minimum and maximum are the same.");
      return;
    }

    string clusive = gui_iso_quantity_clusive_.get();

    ostringstream str;

    str << get_id() << " set-isoquant-list \"";

    if (clusive == "exclusive") {
      // if the min - max range is 2 - 4, and the user requests 3 isovals,
      // the code below generates 2.333, 3.0, and 3.666 -- which is nice
      // since it produces evenly spaced slices in torroidal data.
	
      double di=(qmax - qmin)/(double)num;
      for (int i=0; i<num; i++) {
	isovals.push_back(qmin + ((double)i+0.5)*di);
	str << " " << isovals[i];
      }
    } else if (clusive == "inclusive") {
      // if the min - max range is 2 - 4, and the user requests 3 isovals,
      // the code below generates 2.0, 3.0, and 4.0.

      double di=(qmax - qmin)/(double)(num-1.0);
      for (int i=0; i<num; i++) {
	isovals.push_back(qmin + ((double)i*di));
	str << " " << isovals[i];
      }
    }

    str << "\"";

    get_gui()->execute(str.str().c_str());

  } else if (gui_active_isoval_selection_tab_.get() == "2") { // list
    istringstream vlist(gui_iso_value_list_.get());
    double val;
    while(!vlist.eof()) {
      vlist >> val;
      if (vlist.fail()) {
	if (!vlist.eof()) {
	  vlist.clear();
	  warning("List of Isovals was bad at character " +
		  to_string((int)(vlist.tellg())) +
		  "('" + ((char)(vlist.peek())) + "').");
	}
	break;
      }
      else if (!vlist.eof() && vlist.peek() == '%') {
	vlist.get();
	val = qmin + (qmax - qmin) * val / 100.0;
      }
      isovals.push_back(val);
    }
  } else if (gui_active_isoval_selection_tab_.get() == "3") { // matrix

    MatrixHandle matrix_input_handle;
    if( !get_input_handle( "Optional Isovalues", matrix_input_handle, true ) )
      return;

    ostringstream str;
    
    str << get_id() << " set-isomatrix-list \"";

    for (int i=0; i < matrix_input_handle->nrows(); i++) {
      for (int j=0; j < matrix_input_handle->ncols(); j++) {
	isovals.push_back(matrix_input_handle->get(i, j));

	str << " " << isovals[i];
      }
    }

    str << "\"";

    get_gui()->execute(str.str().c_str());

  } else {
    error("Bad active_isoval_selection_tab value");
    return;
  }

  // See if any of the isovalues have changed.
  if( isovals_.size() != isovals.size() ) {
    isovals_.resize( isovals.size() );
    inputs_changed_ = true;
  }

  for( unsigned int i=0; i<isovals.size(); i++ ) {
    if( isovals_[i] != isovals[i] ) {
      isovals_[i] = isovals[i];
      inputs_changed_ = true;
    }
  }

  if( gui_use_algorithm_.changed( true ) ||
      gui_build_field_.changed( true ) ||
      gui_build_geom_.changed( true ) ||
      gui_transparency_.changed( true ) ||

      gui_np_.changed( true ) ||

      gui_color_r_.changed( true ) ||
      gui_color_g_.changed( true ) ||
      gui_color_b_.changed( true ) ) {

    inputs_changed_ = true;
  }

  // Decide if an interpolant will be computed for the output field.
  MatrixOPort *omatrix_port = (MatrixOPort *) get_oport("Node Mapping");

  const bool build_node_interp =
    gui_build_field_.get() && omatrix_port->nconnections();

  // Decide if an interpolant will be computed for the output field.
  omatrix_port = (MatrixOPort *) get_oport("Cell Mapping");

  const bool build_cell_interp =
    gui_build_field_.get() && omatrix_port->nconnections();


  if( (gui_build_field_.get() && !oport_cached("Surface")) ||
      (gui_build_geom_.get()  && !geometry_output_handle_.get_rep()) ||
      (gui_build_geom_.get()  && !oport_cached("Geometry")) ||
      (build_node_interp      && !oport_cached("Node Mapping")) ||
      (build_cell_interp      && !oport_cached("Cell Mapping")) ||
      inputs_changed_  ) {

    update_state(Executing);

    vector<FieldHandle> field_handles;
    vector<GeomHandle > geometry_handles;
    vector<MatrixHandle> node_interpolant_handles;
    vector<MatrixHandle> cell_interpolant_handles;

    const TypeDescription *td = field_input_handle->get_type_description();

    switch (gui_use_algorithm_.get()) {
    case 0:  // Marching Cubes
      {
	LockingHandle<MarchingCubesAlg> mc_alg;
	if (! mc_alg.get_rep()) {
	  CompileInfoHandle ci = MarchingCubesAlg::get_compile_info(td);
	  if (!module_dynamic_compile(ci, mc_alg)) {
	    error( "Marching Cubes can not work with this field.");
	    return;
	  }
	  int np = gui_np_.get();
	  if (np <  1 ) { np =  1; gui_np_.set(np); }
	  if (np > 32 ) { np = 32; gui_np_.set(np); }
	  mc_alg->set_np(np);
	  mc_alg->set_field( field_input_handle.get_rep() );

	  for (unsigned int iv=0; iv<isovals.size(); iv++)
	  {
	    mc_alg->search( isovals[iv],
			    gui_build_field_.get(),
			    gui_build_geom_.get(),
			    gui_transparency_.get() );
	    geometry_handles.push_back( mc_alg->get_geom() );
	    for (int i = 0 ; i < np; i++)
	    {
	      field_handles.push_back( mc_alg->get_field(i) );

	      if (build_node_interp)
		node_interpolant_handles.push_back( mc_alg->get_interpolant(i) );
	      if (build_cell_interp)
		cell_interpolant_handles.push_back( mc_alg->get_parent_cells(i) );
	    }
	  }
	  mc_alg->release();
	}
      }
      break;
    case 1:  // Noise
      {
	LockingHandle<NoiseAlg> noise_alg;
	if (! noise_alg.get_rep()) {
	  CompileInfoHandle ci =
	    NoiseAlg::get_compile_info(td,
				       field_input_handle->basis_order() == 0,
				       false);
	  if (! module_dynamic_compile(ci, noise_alg)) {
	    error( "NOISE can not work with this field.");
	    return;
	  }
	  noise_alg->set_field(field_input_handle.get_rep());

	  for (unsigned int iv=0; iv<isovals.size(); iv++) {
	    geometry_handles.push_back(noise_alg->search(isovals[iv],
							 gui_build_field_.get(),
							 gui_build_geom_.get(),
							 gui_transparency_.get() ) );
	    field_handles.push_back(noise_alg->get_field());
	    if (build_node_interp)
	      node_interpolant_handles.push_back( noise_alg->get_interpolant() );
	  }
	  noise_alg->release();
	}
      }
      break;

    case 2:  // View Dependent
      {
	LockingHandle<SageAlg> sage_alg;
	if (! sage_alg.get_rep()){
	  CompileInfoHandle ci = SageAlg::get_compile_info(td);
	  if (! module_dynamic_compile(ci, sage_alg)) {
	    error( "SAGE can not work with this field.");
	    return;
	  }
	  sage_alg->set_field(field_input_handle.get_rep());

	  for (unsigned int iv=0; iv<isovals.size(); iv++) {
	    GeomGroup *group = scinew GeomGroup;
	    GeomPoints *points = scinew GeomPoints();
	    sage_alg->search(isovals[iv], group, points);
	    geometry_handles.push_back( group );
	  }
	  sage_alg->release();
	}
      }
      break;
    default: // Error
      error("Unknown Algorithm requested.");
      return;
    }

    // Get the output field handle.
    if (gui_build_field_.get() && field_handles.size() && field_handles[0].get_rep()) {

      // Copy the name of field to the downstream field.
      string fldname;
      if (field_input_handle->get_property("name",fldname)) {
	for (unsigned int i=0; i < field_handles.size(); i++)
	  field_handles[i]->set_property("name",fldname, false);
      } else {
	for (unsigned int i=0; i < field_handles.size(); i++)
	  field_handles[i]->set_property("name", string("ExtractIsosurface"), false);
      }

      FieldHandle field_output_handle;

      // Single field.
      if (field_handles.size() == 1)
	field_output_handle = field_handles[0];

      // Multiple field_handles.
      else {

	const TypeDescription *ftd = field_handles[0]->get_type_description();
	CompileInfoHandle ci = ExtractIsosurfaceAlgo::get_compile_info(ftd);
	
	Handle<ExtractIsosurfaceAlgo> algo;
	if (!module_dynamic_compile(ci, algo)) return;
	
	field_output_handle = algo->execute(field_handles);
      }

      // Send the isosurface field downstream
      send_output_handle( "Surface", field_output_handle );

      // Get the output node interpolant handle.
      if (build_node_interp) {
	if (node_interpolant_handles[0].get_rep())
	{
	  MatrixHandle node_matrix_output_handle;

	  if (node_interpolant_handles.size() == 1)
	    node_matrix_output_handle = node_interpolant_handles[0];
	  else
	    node_matrix_output_handle = append_sparse_matrices(node_interpolant_handles);

	  send_output_handle( "Node Mapping", node_matrix_output_handle );
	}
	else
	  warning("Interpolant not computed for this input field type and data location.");
      }

      // Get the output cell interpolant handle.
      if (build_cell_interp) {
	if (cell_interpolant_handles[0].get_rep())
	{
	  MatrixHandle cell_matrix_output_handle;

	  if (cell_interpolant_handles.size() == 1)
	    cell_matrix_output_handle = cell_interpolant_handles[0];
	  else
	    cell_matrix_output_handle = append_sparse_matrices(cell_interpolant_handles);

	  send_output_handle( "Cell Mapping", cell_matrix_output_handle );
	}
	else
	  warning("Interpolant not computed for this input field type and data location.");
      }
    }

    // Merged the geometry results.
    if( gui_build_geom_.get() && isovals.size() ) {
      
      GeomGroup *geomGroup = scinew GeomGroup;

      for (unsigned int iv=0; iv<isovals.size(); iv++) {
	MaterialHandle material_handle;

	if (colormap_input_handle.get_rep())
	  material_handle = colormap_input_handle->lookup(isovals[iv]);
	else
	  material_handle = scinew Material(Color(gui_color_r_.get(),
					  gui_color_g_.get(),
					  gui_color_b_.get()));

	if (geometry_handles[iv].get_rep()) 
	  geomGroup->add(scinew GeomMaterial( geometry_handles[iv],
					      material_handle ));
      }

      if( geomGroup->size() ) {
	geometry_output_handle_ = GeomHandle( geomGroup );
      } else {
	delete geomGroup;
	geometry_output_handle_  = 0;
      }

    } else {
      geometry_output_handle_  = 0;
    }

    string fldname;
    if (!field_input_handle->get_property("name", fldname))
      fldname = string("ExtractIsosurface");

    send_output_handle( "Geometry", geometry_output_handle_, fldname );
  }
}

} // End namespace SCIRun
