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
 *  ShowFieldGlyphs.cc
 *
 *  Written by:
 *   Allen R. Sanderson
 *   Marty Cole
 *   School of Computing
 *   University of Utah
 *   March 15, 2007
 *
 *  Copyright (C) 2007 SCI Institute
 */


#include <Core/Datatypes/Field.h>
#include <Core/Geom/GeomMaterial.h>
#include <Core/Geom/GeomSwitch.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>
#include <Core/Algorithms/Visualization/RenderField.h>
#include <Core/Algorithms/Visualization/RenderFieldGlyphs.h>
#include <Core/Algorithms/Util/FieldInformation.h>


#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/ColorMapPort.h>
#include <Dataflow/Network/Ports/GeometryPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>

#include <Core/Basis/QuadBilinearLgn.h>
#include <Core/Datatypes/ImageMesh.h>
#include <Core/Datatypes/GenericField.h>


#include <typeinfo>
#include <iostream>

namespace SCIRun {

using namespace SCIRunAlgo;

class ShowFieldGlyphs : public Module, public RenderStateBase
{
public:
  ShowFieldGlyphs(GuiContext* ctx);
  virtual ~ShowFieldGlyphs();

  virtual void execute();
  virtual void tcl_command(GuiArgs& args, void* userdata);

  virtual void post_read();

protected:
  //! input ports
  int                      mesh_generation_;
  bool                     primary_field_present_;
  bool                     secondary_field_present_;
  bool                     tertiary_field_present_;
  bool                     color_map_present_;

  //! output port
  GeometryOPort           *ogeom_;

  //! Scene graph ID's
  int                      data_id_;
  int                      text_id_;

  //! top level nodes for switching on and off.

  //! Options for rendering data.
    GuiInt                   glyphs_on_;                 // Display the Glyphs boolean
    GuiInt                   glyph_alpha_;               // Type of alpha selection for Alpha mapping
    GuiInt                   glyph_value_;               // Selected Glyph type in choosing glyph
    GuiInt                   glyphs_transparency_;       // Display using transparency 
                                                         // set if glyph_alpha_ is not zero

    GuiInt                   glyphs_normalize_;          // Normalize data before scaling
    GuiInt                   glyphs_color_type_;         // Data decomposition type to use
    GuiInt                   glyphs_color_type_field_;   // Field port to take the RGB data from
    GuiInt                   glyphs_resolution_;         // Resolution of glyphs
    GuiInt                   glyphs_display_type_;       //
    GuiString                glyph_image_;               // For storing the base_64 encoded gifs of glyphs
    GuiInt                   colormap_color_;            // Type of color selection for Colormapping 
                                                         // 0 - default color, 1 - 3 Colormap ports
                                                         // 4+ Use RGB data from selection in 
                                                         // glyphs_color_type_ and glyphs_color_type_field_

    GuiDouble                glyphs_scale_;              // Scaling for glyphs

    GuiString                scalar_glyphs_;             // List of Scalar glyph names for display
    GuiString                scalar_glyphs_values_;      // List of Scalar glyph names for data use

    GuiString                vector_glyphs_;             // List of Vector glyph names for display
    GuiString                vector_glyphs_values_;      // List of Vector glyph names for data use

    GuiString                tensor_glyphs_;             // List of Tensor glyph names for display
    GuiString                tensor_glyphs_values_;      // List of Tensor glyph names for data use

    GuiString                glyph_parameter0_;          // Data type code for parameter 0
    GuiString                glyph_parameter1_;          // Data type code for parameter 1
    GuiString                glyph_parameter2_;          // Data type code for parameter 2
    GuiString                glyph_parameter3_;          // Data type code for parameter 3
    GuiString                glyph_parameter4_;          // Data type code for parameter 4
    GuiString                glyph_parameter5_;          // Data type code for parameter 5
    GuiString                glyph_parameter6_;          // Data type code for parameter 6
    GuiString                glyph_parameter7_;          // Data type code for parameter 7
							
    GuiString                glyph_parameter0_field_;    // Field number for parameter 0
    GuiString                glyph_parameter1_field_;    // Field number for parameter 1
    GuiString                glyph_parameter2_field_;    // Field number for parameter 2
    GuiString                glyph_parameter3_field_;    // Field number for parameter 3
    GuiString                glyph_parameter4_field_;    // Field number for parameter 4
    GuiString                glyph_parameter5_field_;    // Field number for parameter 5
    GuiString                glyph_parameter6_field_;    // Field number for parameter 6
    GuiString                glyph_parameter7_field_;    // Field number for parameter 7

    GuiInt                   vectors_bidirectional_;     // Draw vector glyphs bidirectional

    GuiDouble                tensors_emphasis_;          // Emphasis value for superquadric

    GuiInt                   primary_has_data_;          // Does Primary port have data 
    GuiInt                   primary_has_scalar_data_;   // Does Primary port have scalar data 
    GuiInt                   primary_has_vector_data_;   // Does Primary port have vector data 
    GuiInt                   primary_has_tensor_data_;   // Does Primary port have tensor data 

    GuiInt                   secondary_has_data_;        // Does Secondary port have data
    GuiInt                   secondary_has_scalar_data_; // Does Secondary port have scalar data
    GuiInt                   secondary_has_vector_data_; // Does Secondary port have vector data
    GuiInt                   secondary_has_tensor_data_; // Does Secondary port have tensor data

    GuiInt                   tertiary_has_data_;         // Does Tertiary port have data
    GuiInt                   tertiary_has_scalar_data_;  // Does Tertiary port have scalar data
    GuiInt                   tertiary_has_vector_data_;  // Does Tertiary port have vector data
    GuiInt                   tertiary_has_tensor_data_;  // Does Tertiary port have tensor data

    GuiString                glyph_parameters_;          // 

  //! Options for rendering text.
  GuiInt                   text_on_;
  GuiInt                   text_color_type_;
  GuiDouble                text_color_r_;
  GuiDouble                text_color_g_;
  GuiDouble                text_color_b_;
  GuiInt                   text_backface_cull_;
  GuiInt                   text_always_visible_;
  GuiInt                   text_fontsize_;
  GuiInt                   text_precision_;
  GuiInt                   text_render_locations_;
  GuiInt                   text_show_data_;
  GuiInt                   text_show_nodes_;
  GuiInt                   text_show_edges_;
  GuiInt                   text_show_faces_;
  GuiInt                   text_show_cells_;
  MaterialHandle           text_material_;

  string                   field_data_basis_type_;

  //! default color and material
  GuiDouble                def_color_r_;
  GuiDouble                def_color_g_;
  GuiDouble                def_color_b_;
  GuiDouble                def_color_a_;
  MaterialHandle           def_material_;



    GuiString                active_tab_; //! for saving nets state
    GuiString                interactive_mode_; //need
    GuiInt                   showProgress_; //need

  GuiString                gui_field_name_;
  GuiInt                   gui_field_name_override_;

    // Unused varables for remove as they are removed from rest of the code
    // I have found that these variables need to be removed in pairs
    // Otherwise when you run scirun it crashes on launch with a
    // "corrupted doubly linked list" error from glib
  GuiDouble                glyphs_scaleNV_;            // Unused

  GuiInt                   scalars_has_data_;
  GuiInt                   scalars_on_;
  GuiString                scalars_display_type_;
  GuiInt                   scalars_transparency_;
  GuiInt                   scalars_normalize_;
  GuiInt                   scalars_color_type_;
  GuiInt                   scalars_resolution_;

  GuiInt                   vectors_has_data_;
  GuiInt                   vectors_on_;
  GuiString                vectors_display_type_;
  GuiInt                   vectors_transparency_;
  GuiInt                   vectors_normalize_;
  GuiInt                   vectors_color_type_;
  GuiInt                   vectors_resolution_;

  GuiInt                   tensors_has_data_;
  GuiInt                   tensors_on_;
  GuiString                tensors_display_type_;
  GuiInt                   tensors_transparency_;
  GuiInt                   tensors_normalize_;
  GuiInt                   tensors_color_type_;
  GuiInt                   tensors_resolution_;

  GuiDouble                scalars_scale_;
  GuiDouble                scalars_scaleNV_;
  GuiDouble                vectors_scale_;
  GuiDouble                vectors_scaleNV_;
  GuiDouble                tensors_scale_;
  GuiDouble                tensors_scaleNV_;
  GuiDouble                secondary_scale_;
  GuiDouble                secondary_scaleNV_;
  GuiDouble                tertiary_scale_;
  GuiDouble                tertiary_scaleNV_;

  GuiInt                   secondary_on_;
  GuiString                secondary_display_type_;
  GuiInt                   secondary_color_type_;
  GuiInt                   secondary_alpha_;
  GuiInt                   secondary_value_;

  GuiInt                   tertiary_on_;
  GuiString                tertiary_display_type_;
  GuiInt                   tertiary_color_type_;
  GuiInt                   tertiary_alpha_;
  GuiInt                   tertiary_value_;
    //End of Unused Variables


  //! Refinement resolution for cylinders and spheres
  GuiInt                   approx_div_;

  LockingHandle<RenderFieldBase>        text_renderer_;
  LockingHandle<RenderScalarFieldBase>  scalar_renderer_;
  LockingHandle<RenderVectorFieldBase>  vector_renderer_;
  LockingHandle<RenderTensorFieldBase>  tensor_renderer_;

  // variables related to default scale factor usage.
  GuiInt                  gui_use_default_size_;
  double                  cur_mesh_scale_factor_;

  unsigned int            render_state_[6];

  enum toggle_type_e {
    SCALAR    = 0,
    VECTOR    = 1,
    TENSOR    = 2,
    SECONDARY = 3,
    TERTIARY  = 4,
    TEXT      = 5,
    GLYPH     = 6,
    ALL     = 7
  };

//Enumeration of the glyph types used in Glyph Menu when a glyph is selected.
    enum Glyph_Types {
	// Scalars
	POINT = 1,
	SPHERE = 2,
	BOX = 3,
	AXES = 4,
	// Vectors
	LINE = 16,
	NEEDLE = 32,
	ARROW = 48,
	CONE = 64,
	COMET = 80,
	TORUS = 96,
	HELIX = 112,
	//Tensor
	TBOX = 256,
	CBOX = 512,
	ELLIPSOID = 768,
	SUPERQUADRIC = 1024,
    };


//Scructure to be passed to the renderer.
    struct Glyph_State
    {
        Glyph_Types glyph_type;
	bool bidirectional;
	bool transparency;
	bool normalize;
	float glyph_resolution;
	MaterialHandle def_material;
    };

//Scructure to be passed to the renderer.
    struct Parameter
    {
	short field_num; //Primary 0, Secondary 1, Tertiary 2
	char  parameter_type[5];
	float value;     // scale
    };



  void maybe_execute(toggle_type_e dis_type);
  void set_default_display_values();
};

ShowFieldGlyphs::ShowFieldGlyphs(GuiContext* ctx) :
  Module("ShowFieldGlyphs", ctx, Filter, "Visualization", "SCIRun"),
  mesh_generation_(-1),
  secondary_field_present_(false),
  tertiary_field_present_(false),
  color_map_present_(false),
  ogeom_(0),
  data_id_(0),
  text_id_(0),

    glyphs_on_(get_ctx()->subVar("glyphs_on"), 0), 
    glyph_alpha_(get_ctx()->subVar("glyphs_alpha"), 0),
    glyph_value_(get_ctx()->subVar("glyphs_value"), 0), 
    glyphs_transparency_(get_ctx()->subVar("glyphs_transparency"), 0),
								      

    glyphs_normalize_(get_ctx()->subVar("glyphs_normalize"), 0), 
    glyphs_color_type_(get_ctx()->subVar("glyphs_color_type"), 1),
    glyphs_color_type_field_(get_ctx()->subVar("glyphs_color_type_field"), 1),
    glyphs_resolution_(get_ctx()->subVar("glyphs_resolution"), 13), 
    glyphs_display_type_(get_ctx()->subVar("glyphs_display_type"), 0), 
    glyph_image_(get_ctx()->subVar("glyph_image"), " "), 
    colormap_color_(get_ctx()->subVar("colormap_color"),0),

    glyphs_scale_(get_ctx()->subVar("glyphs_scale"), 1), 

    scalar_glyphs_(get_ctx()->subVar("scalar_glyphs"), "Points Spheres Test Axes"), 
    scalar_glyphs_values_(get_ctx()->subVar("scalar_glyphs_values"), "points spheres boxes axes"), 

    vector_glyphs_(get_ctx()->subVar("vector_glyphs"), "Lines Needles Arrows Cones Comets Disks Rings Springs"), 
    vector_glyphs_values_(get_ctx()->subVar("vector_glyphs_values"), "lines needles arrows cones comets disks rings springs"), 

    tensor_glyphs_(get_ctx()->subVar("tensor_glyphs"), "Boxes {Colored Boxes} Ellipsoids Superquadrics"), 
    tensor_glyphs_values_(get_ctx()->subVar("tensor_glyphs_values"), "tboxes cboxes ellipsoids superquadrics"), 

    glyph_parameter0_(get_ctx()->subVar("glyph_parameter0"), "Spheres"), //My added
    glyph_parameter1_(get_ctx()->subVar("glyph_parameter1"), "Spheres"), //My added
    glyph_parameter2_(get_ctx()->subVar("glyph_parameter2"), "Spheres"), //My added
    glyph_parameter3_(get_ctx()->subVar("glyph_parameter3"), "Spheres"), //My added
    glyph_parameter4_(get_ctx()->subVar("glyph_parameter4"), "Spheres"), //My added
    glyph_parameter5_(get_ctx()->subVar("glyph_parameter5"), "Spheres"), //My added
    glyph_parameter6_(get_ctx()->subVar("glyph_parameter6"), "Spheres"), //My added
    glyph_parameter7_(get_ctx()->subVar("glyph_parameter7"), "Spheres"), 
								       

    glyph_parameter0_field_(get_ctx()->subVar("glyph_parameter0_field"), "Spheres"), //My added
    glyph_parameter1_field_(get_ctx()->subVar("glyph_parameter1_field"), "Spheres"), //My added
    glyph_parameter2_field_(get_ctx()->subVar("glyph_parameter2_field"), "Spheres"), //My added
    glyph_parameter3_field_(get_ctx()->subVar("glyph_parameter3_field"), "Spheres"), //My added
    glyph_parameter4_field_(get_ctx()->subVar("glyph_parameter4_field"), "Spheres"), //My added
    glyph_parameter5_field_(get_ctx()->subVar("glyph_parameter5_field"), "Spheres"), //My added
    glyph_parameter6_field_(get_ctx()->subVar("glyph_parameter6_field"), "Spheres"), //My added
    glyph_parameter7_field_(get_ctx()->subVar("glyph_parameter7_field"), "Spheres"), //My added

    vectors_bidirectional_(get_ctx()->subVar("vectors_bidirectional"), 0), //need

    tensors_emphasis_(get_ctx()->subVar("tensors_emphasis"), 0.825), //need

    primary_has_data_(get_ctx()->subVar("primary_has_data"), 0), //My added
    primary_has_scalar_data_(get_ctx()->subVar("primary_has_Scalar_data"), 0), //My added
    primary_has_vector_data_(get_ctx()->subVar("primary_has_Vector_data"), 0), //My added
    primary_has_tensor_data_(get_ctx()->subVar("primary_has_Tensor_data"), 0), //My added
    
    secondary_has_data_(get_ctx()->subVar("secondary_has_data"), 0), //need
    secondary_has_scalar_data_(get_ctx()->subVar("secondary_has_Scalar_data"), 0), //My added
    secondary_has_vector_data_(get_ctx()->subVar("secondary_has_Vector_data"), 0), //My added
    secondary_has_tensor_data_(get_ctx()->subVar("secondary_has_Tensor_data"), 0), //My added

    tertiary_has_data_(get_ctx()->subVar("tertiary_has_data"), 0), //need
    tertiary_has_scalar_data_(get_ctx()->subVar("tertiary_has_Scalar_data"), 0), //My added
    tertiary_has_vector_data_(get_ctx()->subVar("tertiary_has_Vector_data"), 0), //My added
    tertiary_has_tensor_data_(get_ctx()->subVar("tertiary_has_Tensor_data"), 0), //My added

    glyph_parameters_(get_ctx()->subVar("glyph_paramters"), ""), //My added

  text_on_(get_ctx()->subVar("text_on"), 0),
  text_color_type_(get_ctx()->subVar("text_color_type"), 0),
  text_color_r_(get_ctx()->subVar("text_color-r"), 1.0),
  text_color_g_(get_ctx()->subVar("text_color-g"), 1.0),
  text_color_b_(get_ctx()->subVar("text_color-b"), 1.0),
  text_backface_cull_(get_ctx()->subVar("text_backface_cull"), 0),
  text_always_visible_(get_ctx()->subVar("text_always_visible"), 0),
  text_fontsize_(get_ctx()->subVar("text_fontsize"), 0),
  text_precision_(get_ctx()->subVar("text_precision"),3),
  text_render_locations_(get_ctx()->subVar("text_render_locations"), 0),
  text_show_data_(get_ctx()->subVar("text_show_data"),1),
  text_show_nodes_(get_ctx()->subVar("text_show_nodes"), 0),
  text_show_edges_(get_ctx()->subVar("text_show_edges"), 0),
  text_show_faces_(get_ctx()->subVar("text_show_faces"), 0),
  text_show_cells_(get_ctx()->subVar("text_show_cells"), 0),
  text_material_(new Material(Color(0.75, 0.75, 0.75))),

  field_data_basis_type_("none"),

  def_color_r_(get_ctx()->subVar("def_color-r"), 0.5),
  def_color_g_(get_ctx()->subVar("def_color-g"), 0.5),
  def_color_b_(get_ctx()->subVar("def_color-b"), 0.5),
  def_color_a_(get_ctx()->subVar("def_color-a"), 0.5),
  def_material_(new Material(Color(0.5, 0.5, 0.5))),

  active_tab_(get_ctx()->subVar("active_tab"), "Text"),
  interactive_mode_(get_ctx()->subVar("interactive_mode"), "Interactive"),
  showProgress_(get_ctx()->subVar("show_progress"), 0),

  gui_field_name_(get_ctx()->subVar("field_name"), ""),
  gui_field_name_override_(get_ctx()->subVar("field_name_override"), 0),

  approx_div_(get_ctx()->subVar("approx_div"), 1),

 ///////////////////////////
 // Unused varables for remove as they are removed from rest of the code
 // I have found that these variables need to be removed in pairs
 // Otherwise when you run scirun it crashes on launch with a
 // "corrupted doubly linked list" error from glib
 ////////////////////////////
  glyphs_scaleNV_(get_ctx()->subVar("glyphs_scaleNV"), 0), 
  scalars_has_data_(get_ctx()->subVar("scalars_has_data"), 0),
  scalars_on_(get_ctx()->subVar("scalars_on"), 0),
  scalars_display_type_(get_ctx()->subVar("scalars_display_type"), "Spheres"),
  scalars_transparency_(get_ctx()->subVar("scalars_transparency"), 0),
  scalars_normalize_(get_ctx()->subVar("scalars_normalize"), 0),
  scalars_color_type_(get_ctx()->subVar("scalars_color_type"), 1),
  scalars_scale_(get_ctx()->subVar("scalars_scale")),
  scalars_scaleNV_(get_ctx()->subVar("scalars_scaleNV")),
  scalars_resolution_(get_ctx()->subVar("scalars_resolution"), 6),

  vectors_has_data_(get_ctx()->subVar("vectors_has_data"), 0),
  vectors_on_(get_ctx()->subVar("vectors_on"), 0),
  vectors_display_type_(get_ctx()->subVar("vectors_display_type"), "Arrows"),
  vectors_transparency_(get_ctx()->subVar("vectors_transparency"), 0),
  vectors_normalize_(get_ctx()->subVar("vectors_normalize"), 0),
  vectors_color_type_(get_ctx()->subVar("vectors_color_type"), 1),
  vectors_scale_(get_ctx()->subVar("vectors_scale")),
  vectors_scaleNV_(get_ctx()->subVar("vectors_scaleNV")),
  vectors_resolution_(get_ctx()->subVar("vectors_resolution"), 6),

  tensors_has_data_(get_ctx()->subVar("tensors_has_data"), 0),
  tensors_on_(get_ctx()->subVar("tensors_on"), 0),
  tensors_display_type_(get_ctx()->subVar("tensors_display_type"), "Colored Boxes"),
  tensors_transparency_(get_ctx()->subVar("tensors_transparency"), 0),
  tensors_normalize_(get_ctx()->subVar("tensors_normalize"), 0),
  tensors_color_type_(get_ctx()->subVar("tensors_color_type"), 1),
  tensors_scale_(get_ctx()->subVar("tensors_scale")),
  tensors_scaleNV_(get_ctx()->subVar("tensors_scaleNV")),
  tensors_resolution_(get_ctx()->subVar("tensors_resolution"), 6),

  secondary_on_(get_ctx()->subVar("secondary_on"), 0),
  secondary_display_type_(get_ctx()->subVar("secondary_display_type"), "Major Radius"),
  secondary_color_type_(get_ctx()->subVar("secondary_color_type"), 0),
  secondary_alpha_(get_ctx()->subVar("secondary_alpha"), 0),
  secondary_value_(get_ctx()->subVar("secondary_value"), 1),
  secondary_scale_(get_ctx()->subVar("secondary_scale")),
  secondary_scaleNV_(get_ctx()->subVar("secondary_scaleNV")),

  tertiary_on_(get_ctx()->subVar("tertiary_on"), 0),
  tertiary_display_type_(get_ctx()->subVar("tertiary_display_type"), "Minor Radius"),
  tertiary_color_type_(get_ctx()->subVar("tertiary_color_type"), 0),
  tertiary_alpha_(get_ctx()->subVar("tertiary_alpha"), 0),
  tertiary_value_(get_ctx()->subVar("tertiary_value"), 1),
  tertiary_scale_(get_ctx()->subVar("tertiary_scale")),
  tertiary_scaleNV_(get_ctx()->subVar("tertiary_scaleNV")),
  /////////////////////////////////
  // End of Unused Variables
  /////////////////////////////////

  text_renderer_(0),
  scalar_renderer_(0),
  vector_renderer_(0),
  tensor_renderer_(0),

  gui_use_default_size_(get_ctx()->subVar("use_default_size"), 0),
  cur_mesh_scale_factor_(1.0)
{
  render_state_[SCALAR] = 0;
  render_state_[VECTOR] = 0;
  render_state_[TENSOR] = 0;
  render_state_[SECONDARY] = 0;
  render_state_[TERTIARY] = 0;
  render_state_[GLYPH] = 0;
  render_state_[TEXT] = 0;
}


ShowFieldGlyphs::~ShowFieldGlyphs()
{
}


void
ShowFieldGlyphs::execute()
{
  bool update_algo = false;

  ogeom_ = (GeometryOPort *) get_oport("Scene Graph");

  FieldHandle pfld_handle;
  if( !get_input_handle("Primary Data", pfld_handle, !in_power_app() ) )
    return;

  primary_field_present_ = true;
 
  // Update the field name but only if the user does not enter an
  // overriding name.
  if (!gui_field_name_override_.get())
  {
    string fname("");

    if ( !pfld_handle->get_property("name", fname))
      pfld_handle->mesh()->get_property("name", fname);
       
    if( fname != gui_field_name_.get() )
    {
      gui_field_name_.set(fname);
      gui_field_name_.reset();
    }
  }

  // See if the mesh has changed because this is not check with the ports.
  if( mesh_generation_ != pfld_handle->mesh()->generation )
  {
    mesh_generation_ = pfld_handle->mesh()->generation;
    inputs_changed_ = true;
    update_algo = true;
  }

  // Get the optional secondary field that will be used for the color and
  // secondary scaling.
  FieldHandle sfld_handle;
  if( get_input_handle("Secondary Data", sfld_handle, false) )
  {
    if( !secondary_field_present_ )
    {
      secondary_field_present_ = true;
      inputs_changed_ = true;
      update_algo = true;
    }
  }
  else 
  {
    if( secondary_field_present_ )
    {
      secondary_field_present_ = false;
      inputs_changed_ = true;
      update_algo = true;
    }

    sfld_handle = pfld_handle;
  }

  // Get the optional tertiary field that will be used for the color and
  // tertiary scaling.
  FieldHandle tfld_handle;
  if( get_input_handle("Tertiary Data", tfld_handle, false) )
  {
    if( !tertiary_field_present_ )
    {
      tertiary_field_present_ = true;
      inputs_changed_ = true;
      update_algo = true;
    }
  }
  else 
  {
    if( tertiary_field_present_ )
    {
      tertiary_field_present_ = false;
      inputs_changed_ = true;
      update_algo = true;
    }

    tfld_handle = pfld_handle;
  }

  // See if the algorithm needs to be recreated due the field or mesh
  // changing. This check will be made if colormap changes also.
  if( inputs_changed_ ) 
  {
    // Set inital colors here.
    def_material_->diffuse =
      Color(def_color_r_.get(), def_color_g_.get(), def_color_b_.get());
    def_material_->transparency = def_color_a_.get();

    text_material_->diffuse =
      Color(text_color_r_.get(), text_color_g_.get(), text_color_b_.get());
    text_material_->transparency = 1.0;
    
    // Get the information on the types of the fields
    FieldInformation pfi(pfld_handle);

    if (!pfi.is_svt() )
    {
      error("No Scalar, Vector, or Tensor data found in the data field.");
      return;
    }

    // Make sure both fields have similar meshes and data locations.
    if( sfld_handle != pfld_handle || tfld_handle != pfld_handle )
    {
      // Get the information on the types of the fields
      FieldInformation sfi(sfld_handle);
      FieldInformation tfi(tfld_handle);
      
      if (!sfi.is_svt() )
      {
        error("No Scalar, Vector, or Tensor data found in the secondary field.");
        return;
      }

      if (!tfi.is_svt() )
      {
        error("No Scalar, Vector, or Tensor data found in the tertiary field.");
        return;
      }

	  secondary_has_scalar_data_.set(sfi.is_scalar());
	  secondary_has_vector_data_.set(sfi.is_vector());
	  secondary_has_tensor_data_.set(sfi.is_tensor());
	  tertiary_has_scalar_data_.set(tfi.is_scalar());
	  tertiary_has_vector_data_.set(tfi.is_vector());
	  tertiary_has_tensor_data_.set(tfi.is_tensor());

      vector< FieldHandle > field_input_handles;
	
      field_input_handles.push_back( pfld_handle );
      field_input_handles.push_back( sfld_handle );
      field_input_handles.push_back( tfld_handle );
      
      if( !FieldsAlgo::SimilarMeshes( this, field_input_handles ) )
        return;
    }
    else
    {
      if( !secondary_field_present_ ) 
      {
        secondary_on_.set(0);
        secondary_on_.reset();
      }

      if( !tertiary_field_present_ ) {
        tertiary_on_.set(0);
        tertiary_on_.reset();
      }

    }

    primary_has_data_.set(primary_field_present_);
    primary_has_scalar_data_.set(pfi.is_scalar());
    primary_has_vector_data_.set(pfi.is_vector());
    primary_has_tensor_data_.set(pfi.is_tensor());

    secondary_has_data_.set(secondary_field_present_);
    secondary_has_data_.reset();

    tertiary_has_data_.set(tertiary_field_present_);
    tertiary_has_data_.reset();

    primary_has_data_.reset();
    primary_has_scalar_data_.reset();
    primary_has_vector_data_.reset();
    primary_has_tensor_data_.reset();

    secondary_has_data_.reset();
    secondary_has_scalar_data_.reset();
    secondary_has_vector_data_.reset();
    secondary_has_tensor_data_.reset();

    tertiary_has_data_.reset();
    tertiary_has_scalar_data_.reset();
    tertiary_has_vector_data_.reset();
    tertiary_has_tensor_data_.reset();


    const TypeDescription *data_type_description =
      pfld_handle->get_type_description(Field::BASIS_TD_E);

    if( field_data_basis_type_ != data_type_description->get_name() )
    {
      field_data_basis_type_ = data_type_description->get_name();
      update_algo = true;
    }

    // If the field basis type or the mesh has changed update the algorithm.
    if (update_algo)
    {
      if ( 0 &&
	  pfld_handle->has_virtual_interface() &&
	  sfld_handle->has_virtual_interface() &&
	  tfld_handle->has_virtual_interface())
      {
        text_renderer_ = new RenderFieldV;
        scalar_renderer_ = new RenderScalarFieldV;
        vector_renderer_ = new RenderVectorFieldV;
        tensor_renderer_ = new RenderTensorFieldV;
        
        BBox bbox = pfld_handle->vmesh()->get_bounding_box();
     
        if (bbox.valid()) 
        {  
          Vector diag = bbox.diagonal(); 
          cur_mesh_scale_factor_ = diag.length();
        } 
        else 
        {
          cur_mesh_scale_factor_ = 1.0;
        }
      
        gui_use_default_size_.reset();
        if (gui_use_default_size_.get() || 
            sci_getenv_p("SCIRUN_USE_DEFAULT_SETTINGS")) 
        {
          set_default_display_values();
        }        
      }
      else
      {
        const TypeDescription *pftd = pfld_handle->get_type_description();
        const TypeDescription *sftd = sfld_handle->get_type_description();
        const TypeDescription *tftd = tfld_handle->get_type_description();
        const TypeDescription *pltd = pfld_handle->order_type_description();
        // Get the Algorithms.
        CompileInfoHandle ci = RenderFieldBase::get_compile_info(pftd, pltd);
        if (!module_dynamic_compile(ci, text_renderer_))
        {
          return;
        }

        FieldInformation fi(pfld_handle);
        
        if ( fi.is_scalar())
        {
          CompileInfoHandle dci =
            RenderScalarFieldBase::get_compile_info(pftd, sftd, tftd, pltd);
          if (!module_dynamic_compile(dci, scalar_renderer_))
          {
            return;
          }
        }

        else if ( fi.is_vector())
        {
          CompileInfoHandle dci =
            RenderVectorFieldBase::get_compile_info(pftd, sftd, tftd, pltd);
          if (!module_dynamic_compile(dci, vector_renderer_))
          {
            return;
          }
        }

        else if ( fi.is_tensor())
        {
          CompileInfoHandle dci =
            RenderTensorFieldBase::get_compile_info(pftd, sftd, tftd, pltd);
          if (!module_dynamic_compile(dci, tensor_renderer_))
          {
            return;
          }
        }
      

        // set new scale defaults based on input.
        BBox bbox = pfld_handle->mesh()->get_bounding_box();
     
        if (bbox.valid()) 
        {  
          Vector diag = bbox.diagonal(); 
          cur_mesh_scale_factor_ = diag.length();
        } 
        else 
        {
          cur_mesh_scale_factor_ = 1.0;
        }
      
        gui_use_default_size_.reset();
        if (gui_use_default_size_.get() || 
            sci_getenv_p("SCIRUN_USE_DEFAULT_SETTINGS")) 
        {
          set_default_display_values();
        }
      }
    }
  }

  // If the colormap gets connected or disconnected then a redraw may be need.
  // Do this after the algorithm checks so the check does not happen

  bool need_primary_color_map = (scalars_color_type_.get() == 1 ||
				 vectors_color_type_.get() == 1 ||
				 tensors_color_type_.get() == 1 ||
				 text_color_type_.get() == 1);

  bool need_secondary_color_map = (secondary_on_.get() == 1 &&
				   secondary_color_type_.get() == 1);

  bool need_tertiary_color_map = (tertiary_on_.get() == 1 &&
				  tertiary_color_type_.get() == 1);

  ColorMapHandle cmap_handle;

  // Check the tertiary first as it overrides the secondary.
  if( need_tertiary_color_map )
  {
    if( get_input_handle("Tertiary ColorMap", cmap_handle, true) )
    {
      if( !color_map_present_ )
      {
	color_map_present_ = true;
	inputs_changed_ = true;
      }
    }
    else if( color_map_present_ )
    {
      color_map_present_ = false;
      inputs_changed_ = true;
    }
  }

  // Check the secondary first as it overrides the primary.
  else if( need_secondary_color_map )
  {
    if( get_input_handle("Secondary ColorMap", cmap_handle, true) )
    {
      if( !color_map_present_ )
      {
	color_map_present_ = true;
	inputs_changed_ = true;
      }
    }
    else if( color_map_present_ )
    {
      color_map_present_ = false;
      inputs_changed_ = true;
    }
  }

  else if( need_primary_color_map )
  {
    if( get_input_handle("Primary ColorMap", cmap_handle, true) )
    {
      if( !color_map_present_ )
      {
	// If colormap is added and the current selection is for the
	// default assume that the user wants to use the colormap.
	if( scalars_color_type_.get() == 0 ||
	    vectors_color_type_.get() == 0 ||
	    tensors_color_type_.get() == 0 )
	  remark("Detected a colormap, using it instead of the defult color.");

	if( scalars_color_type_.get() == 0 )
	  scalars_color_type_.set(1);

	if( vectors_color_type_.get() == 0 )
	  vectors_color_type_.set(1);

	if( tensors_color_type_.get() == 0 )
	  tensors_color_type_.set(1);

	scalars_color_type_.reset();
	vectors_color_type_.reset();
	tensors_color_type_.reset();

	color_map_present_ = true;
	inputs_changed_ = true;
      }
    }
    else if( color_map_present_ )
    {
      color_map_present_ = false;
      inputs_changed_ = true;
    }
  }

  if( !color_map_present_  &&
      (need_primary_color_map ||
       need_secondary_color_map ||
       need_tertiary_color_map) )
  {
    warning("No Colormap present using default color.");

    if(scalars_color_type_.get() == 1)
      scalars_color_type_.set(0);
    if(vectors_color_type_.get() == 1)
      vectors_color_type_.set(0);
    if(tensors_color_type_.get() == 1)
      tensors_color_type_.set(0);
    if(secondary_color_type_.get() == 1)
      secondary_color_type_.set(0);
    if(tertiary_color_type_.get() == 1)
      tertiary_color_type_.set(0);
    if(text_color_type_.get() == 1)
      text_color_type_.set(0);

    scalars_color_type_.reset();
    vectors_color_type_.reset();
    tensors_color_type_.reset();
    secondary_color_type_.reset();
    tertiary_color_type_.reset();
    text_color_type_.reset();
  }

  // Do this after the algorithm checks so the check does not happen
  // unless needed.
  if( gui_field_name_.changed() )
    inputs_changed_ = true;

  // Major input change so everything is dirty
//   if( inputs_changed_ )
//   {
//     set_flag( render_state_[SCALAR], DIRTY);
//     set_flag( render_state_[VECTOR], DIRTY);
//     set_flag( render_state_[TENSOR], DIRTY);
//     set_flag( render_state_[TEXT  ], DIRTY);
//   }

//   // check to see if there is something to do.
//   if (!get_flag(render_state_[SCALAR], DIRTY) && 
//       !get_flag(render_state_[VECTOR], DIRTY) && 
//       !get_flag(render_state_[TENSOR], DIRTY) && 
//       !get_flag(render_state_[TEXT], DIRTY))
//   {
//     return;
//   }

//   switch_flag( render_state_[SCALAR], IS_ON, scalars_on_.get() );
//   switch_flag( render_state_[VECTOR], IS_ON, vectors_on_.get() );
//   switch_flag( render_state_[TENSOR], IS_ON, tensors_on_.get() );
//   switch_flag( render_state_[SECONDARY], IS_ON, secondary_on_.get() );
//   switch_flag( render_state_[TERTIARY], IS_ON, tertiary_on_.get() );
//   switch_flag( render_state_[TEXT  ], IS_ON, text_on_.get() );

//   switch_flag( render_state_[SCALAR],    USE_COLORMAP, color_map_present_ );
//   switch_flag( render_state_[VECTOR],    USE_COLORMAP, color_map_present_ );
//   switch_flag( render_state_[TENSOR],    USE_COLORMAP, color_map_present_ );
//   switch_flag( render_state_[SECONDARY], USE_COLORMAP, color_map_present_ );
//   switch_flag( render_state_[TERTIARY], USE_COLORMAP, color_map_present_ );
//   switch_flag( render_state_[TEXT  ],    USE_COLORMAP, color_map_present_ );

//   switch_flag( render_state_[SCALAR], USE_TRANSPARENCY, scalars_transparency_.get() );
//   switch_flag( render_state_[VECTOR], USE_TRANSPARENCY, vectors_transparency_.get() );
//   switch_flag( render_state_[TENSOR], USE_TRANSPARENCY, tensors_transparency_.get() );

//   switch_flag( render_state_[SCALAR], USE_DEFAULT_COLOR, scalars_color_type_.get() == 0 );
//   switch_flag( render_state_[VECTOR], USE_DEFAULT_COLOR, vectors_color_type_.get() == 0 );
//   switch_flag( render_state_[TENSOR], USE_DEFAULT_COLOR, tensors_color_type_.get() == 0 );
//   switch_flag( render_state_[TEXT  ], USE_DEFAULT_COLOR,    text_color_type_.get() == 0 );
//   switch_flag( render_state_[SECONDARY], USE_DEFAULT_COLOR, secondary_color_type_.get() == 0 );
//   switch_flag( render_state_[TERTIARY], USE_DEFAULT_COLOR, tertiary_color_type_.get() == 0 );

//   switch_flag( render_state_[SCALAR], USE_COLORMAP, scalars_color_type_.get() == 1 );
//   switch_flag( render_state_[VECTOR], USE_COLORMAP, vectors_color_type_.get() == 1 );
//   switch_flag( render_state_[TENSOR], USE_COLORMAP, tensors_color_type_.get() == 1 );
//   switch_flag( render_state_[TEXT  ], USE_COLORMAP,    text_color_type_.get() == 1 );
//   switch_flag( render_state_[SECONDARY], USE_COLORMAP, secondary_color_type_.get() == 1 );
//   switch_flag( render_state_[TERTIARY], USE_COLORMAP, tertiary_color_type_.get() == 1 );

//   switch_flag( render_state_[SCALAR], USE_COLOR_CONVERT, scalars_color_type_.get() == 2 );
//   switch_flag( render_state_[VECTOR], USE_COLOR_CONVERT, vectors_color_type_.get() == 2 );
//   switch_flag( render_state_[TENSOR], USE_COLOR_CONVERT, tensors_color_type_.get() == 2 );
//   switch_flag( render_state_[TEXT  ], USE_COLOR_CONVERT,    text_color_type_.get() == 2 );
//   switch_flag( render_state_[SECONDARY], USE_COLOR_CONVERT, secondary_color_type_.get() == 2 );
//   switch_flag( render_state_[TERTIARY], USE_COLOR_CONVERT, tertiary_color_type_.get() == 2 );

//   switch_flag( render_state_[SCALAR], NORMALIZE_DATA, scalars_normalize_.get() );
//   switch_flag( render_state_[VECTOR], NORMALIZE_DATA, vectors_normalize_.get() );
//   switch_flag( render_state_[TENSOR], NORMALIZE_DATA, tensors_normalize_.get() );

//   switch_flag( render_state_[VECTOR], BIDIRECTIONAL ,vectors_bidirectional_.get() );

//   switch_flag( render_state_[SECONDARY], USE_ALPHA, secondary_alpha_.get() );
//   switch_flag( render_state_[SECONDARY], USE_VALUE, secondary_value_.get() );
//   switch_flag( render_state_[SECONDARY], USE_MAJOR_RADIUS,
// 	       secondary_display_type_.get() == "Major Radius" );
//   switch_flag( render_state_[SECONDARY], USE_MINOR_RADIUS,
// 	       secondary_display_type_.get() == "Minor Radius" );
//   switch_flag( render_state_[SECONDARY], USE_PITCH,
// 	       secondary_display_type_.get() == "Pitch" );

//   switch_flag( render_state_[TERTIARY], USE_ALPHA, tertiary_alpha_.get() );
//   switch_flag( render_state_[TERTIARY], USE_VALUE, tertiary_value_.get() );
//   switch_flag( render_state_[TERTIARY], USE_MAJOR_RADIUS,
// 	       tertiary_display_type_.get() == "Major Radius" );
//   switch_flag( render_state_[TERTIARY], USE_MINOR_RADIUS,
// 	       tertiary_display_type_.get() == "Minor Radius" );
//   switch_flag( render_state_[TERTIARY], USE_PITCH,
// 	       tertiary_display_type_.get() == "Pitch" );

  string fname = clean_fieldname(gui_field_name_.get());
  if (fname != "" && fname[fname.size()-1] != ' ') { fname = fname + " "; }

//   if(get_flag(render_state_[SCALAR], (IS_ON|HAS_DATA|DIRTY))) 
//   {
//     GeomHandle data_geometry =
//       scalar_renderer_->render_data(pfld_handle,
// 				    sfld_handle,
// 				    tfld_handle,
// 				    scalars_display_type_.get(),
// 				    scalars_scale_.get(),
// 				    secondary_scale_.get(),
// 				    tertiary_scale_.get(),
// 				    scalars_resolution_.get(),
// 				    render_state_[SCALAR],
// 				    render_state_[SECONDARY],
// 				    render_state_[TERTIARY]);

//     GeomHandle gmat = new GeomMaterial(data_geometry, def_material_);
//     GeomHandle geom = new GeomSwitch(new GeomColorMap(gmat, cmap_handle));
//     if (data_id_) ogeom_->delObj(data_id_);
//     data_id_ = ogeom_->addObj(geom, fname +
// 			      (scalars_transparency_.get()?"Transparent Scalars":"Scalars"));
//   }  
//   else if(get_flag(render_state_[VECTOR], (IS_ON|HAS_DATA|DIRTY))) 
//   {
//     GeomHandle data_geometry =
//       vector_renderer_->render_data(pfld_handle,
// 				    sfld_handle,
// 				    tfld_handle,
// 				    def_material_,
// 				    vectors_display_type_.get(),
// 				    vectors_scale_.get(),
// 				    secondary_scale_.get(),
// 				    tertiary_scale_.get(),
// 				    vectors_resolution_.get(),
// 				    render_state_[VECTOR],
// 				    render_state_[SECONDARY],
// 				    render_state_[TERTIARY]);

//     GeomHandle gmat = new GeomMaterial(data_geometry, def_material_);
//     GeomHandle geom = new GeomSwitch(new GeomColorMap(gmat, cmap_handle));
//     const string vdname = vectors_transparency_.get() ?
//       "Transparent Vectors":
//       ((vectors_display_type_.get()=="Needles" ||
// 	vectors_display_type_.get()=="Comets" ) ?
//        "Transparent Vectors":"Vectors");

//     if (data_id_) ogeom_->delObj(data_id_);
//     data_id_ = ogeom_->addObj(geom, fname + vdname);
//   }
//   else if(get_flag(render_state_[TENSOR], (IS_ON|HAS_DATA|DIRTY))) 
//   {
//     GeomHandle data_geometry =
//       tensor_renderer_->render_data(pfld_handle,
// 				    sfld_handle,
// 				    tfld_handle,
// 				    tensors_display_type_.get(),
// 				    tensors_scale_.get(),
// 				    secondary_scale_.get(),
// 				    tensors_resolution_.get(),
// 				    tensors_emphasis_.get(),
// 				    render_state_[TENSOR],
// 				    render_state_[SECONDARY],
// 				    render_state_[TERTIARY]);

//     GeomHandle gmat = new GeomMaterial(data_geometry, def_material_);
//     GeomHandle geom = new GeomSwitch(new GeomColorMap(gmat, cmap_handle));
//     const string tdname = tensors_transparency_.get() ?
//       "Transparent Tensors":"Tensors";

//     if (data_id_) ogeom_->delObj(data_id_);
//     data_id_ = ogeom_->addObj(geom, fname + tdname);
//   }
//   else
      if(get_flag(render_state_[GLYPH], (IS_ON|HAS_DATA|DIRTY))) 
  {
      int numberParameters = 10;
      struct Glyph_State state;

      state.bidirectional = vectors_bidirectional_.get();
      state.transparency = glyph_alpha_.get();
      state.normalize = glyphs_normalize_.get();
      state.glyph_resolution = glyphs_transparency_.get();
      state.def_material = def_material_;

      FieldHandle fieldHandleArray[3] = {pfld_handle,sfld_handle,tfld_handle};
      ColorMapHandle colormapHandleArray[3];
      struct Parameter parameterArray[numberParameters];


      /*NEW   GeomHandle data_geometry =
      glyph_renderer_->render_data(fieldHandleArray[],
                                   state,
				   colormapHandleArray[],
				   parameterArray[]);

      //OLD!!!!   GeomHandle data_geometry =
      //glyph_renderer_->render_data(fieldHandleArray[],
//				    glyphs_display_type_.get(),
//				    glyphs_scale_.get(),
//				    glyphs_resolution_.get(),
//				    tensors_emphasis_.get(),
//				    render_state_[TENSOR],
//				    render_state_[SECONDARY],
//				    render_state_[TERTIARY]);
//      
//    GeomHandle gmat = new GeomMaterial(data_geometry, def_material_);
//    GeomHandle geom = new GeomSwitch(new GeomColorMap(gmat, cmap_handle));
      */    
      const string gdname = glyphs_transparency_.get() ?
      "Transparent Glyphs":"Glyphs";

//    if (data_id_) ogeom_->delObj(data_id_);
//    data_id_ = ogeom_->addObj(geom, fname + gdname);
  }

  if (get_flag(render_state_[TEXT], (IS_ON|DIRTY)))
  {
    GeomHandle text_geometry =
      text_renderer_->render_text(pfld_handle,
				  cmap_handle,
				  text_color_type_.get() == 1,
				  text_color_type_.get() == 0,
				  text_backface_cull_.get(),
				  text_fontsize_.get(),
				  text_precision_.get(),
				  text_render_locations_.get(),
				  text_show_data_.get(),
				  text_show_nodes_.get(),
				  text_show_edges_.get(),
				  text_show_faces_.get(),
				  text_show_cells_.get(),
				  text_always_visible_.get());
    
    GeomHandle gmat = new GeomMaterial(text_geometry, text_material_);
    GeomHandle geom = new GeomSwitch(new GeomColorMap(gmat, cmap_handle));
    if (text_id_) ogeom_->delObj(text_id_);
    text_id_ = ogeom_->addObj(geom, fname +
			      (text_backface_cull_.get()?"Culled Text Data":"Text Data"));
  }

//   clear_flag( render_state_[SCALAR], DIRTY);
//   clear_flag( render_state_[VECTOR], DIRTY);
//   clear_flag( render_state_[TENSOR], DIRTY);
  clear_flag( render_state_[GLYPH], DIRTY);
  clear_flag( render_state_[TEXT], DIRTY);

  ogeom_->flushViews();
}

void
ShowFieldGlyphs::set_default_display_values() 
{
  double fact = cur_mesh_scale_factor_;
  glyphs_scaleNV_.set(fact * 0.0735);

  set_flag( render_state_[GLYPH], DIRTY);
}

void
ShowFieldGlyphs::maybe_execute(toggle_type_e dis_type)
{
  bool do_execute = false;

  interactive_mode_.reset();
  if (interactive_mode_.get() == "Interactive") {
    switch(dis_type) {
    case GLYPH :
      do_execute = glyphs_on_.get();
	break;
    case TEXT :
      do_execute = text_on_.get();
	break;
    case ALL :
    default:
      do_execute = true;
	break;
    }
  }

  if (do_execute) {
    want_to_execute();
  }
}


void
ShowFieldGlyphs::tcl_command(GuiArgs& args, void* userdata) {
  if(args.count() < 2){
    args.error("ShowFieldGlyphs needs a minor command");
    return;
  }

  if (args[1] == "glyphs_scale") {
      //Handles changes to scaling of the glyph with the slider
      // Reread the values
      glyphs_display_type_.reset();
      glyphs_scale_.reset();
      
      set_flag( render_state_[GLYPH], DIRTY);
      maybe_execute(GLYPH);

  } else if (args[1] == "glyphs_resolution") {
      // handles changes to the glyph resolution slider
      // Reread the values
      glyphs_display_type_.reset();
      glyphs_resolution_.reset();
      
      if (!(glyphs_display_type_.get() == TBOX ||
	    glyphs_display_type_.get() == CBOX)) {
	  set_flag( render_state_[GLYPH], DIRTY);
	  maybe_execute(GLYPH);
      }

  } else if (args[1] == "tensors_emphasis") {
      // Reread the values
      glyphs_display_type_.reset();
      tensors_emphasis_.reset();
      
      set_flag( render_state_[GLYPH], DIRTY);
      maybe_execute(GLYPH);


  } else if (args[1] == "default_color_change") {
    def_color_r_.reset();
    def_color_g_.reset();
    def_color_b_.reset();
    def_color_a_.reset();
    def_material_->diffuse =
      Color(def_color_r_.get(), def_color_g_.get(), def_color_b_.get());
    def_material_->transparency = def_color_a_.get();
    if (ogeom_) ogeom_->flushViews();

  } else if (args[1] == "text_color_change") {
    text_color_r_.reset();
    text_color_g_.reset();
    text_color_b_.reset();
    text_material_->diffuse =
      Color(text_color_r_.get(), text_color_g_.get(), text_color_b_.get());
    if (ogeom_) ogeom_->flushViews();

  } else if (args[1] == "toggle_display_glyphs"){
// may need to be changed, I don't know what GeomSwitch does.
    // Toggle the GeomSwitch.
    glyphs_on_.reset();
    if (glyphs_on_.get())
    {
      set_flag( render_state_[GLYPH], DIRTY);
      maybe_execute(GLYPH);
    }
    else if (data_id_)
    {
      ogeom_->delObj(data_id_);
      ogeom_->flushViews();
      data_id_ = 0;
    }

  } else if (args[1] == "toggle_display_text"){
    // Toggle the GeomSwitch.
    text_on_.reset();
    if ((text_on_.get()) && (text_id_ == 0))
    {
      set_flag( render_state_[TEXT], DIRTY);
      maybe_execute(TEXT);
    }
    else if (!text_on_.get() && text_id_)
    {
      ogeom_->delObj(text_id_);
      ogeom_->flushViews();
      text_id_ = 0;
    }

  } else if (args[1] == "rerender_all" ) {
    set_flag( render_state_[GLYPH], DIRTY);
    maybe_execute(ALL);

  } else if (args[1] == "rerender_glyphs") {
    set_flag( render_state_[GLYPH], DIRTY);
    maybe_execute(GLYPH);

  } else if (args[1] == "rerender_text"){
    set_flag( render_state_[TEXT], DIRTY);
    maybe_execute(TEXT);

  } else if (args[1] == "execute_policy"){

  } else if (args[1] == "calcdefs") {
    set_default_display_values();
    maybe_execute(ALL);

  } else if (args[1] == "get_glyphs"){
//This passes over to the TCL code the types of Glyphs available,
// This can be moved to the other glyph code to keep all the 
// glyph functions together, and a function call executed here to
// load the TCL variables.
//The reason there are *_glyphs_ and *_glyphs_values_ is to allow the use of
// a space in the "Colored Boxes" glyph display name. The *_values_ are 
// used for the logic inside the TCL code.

       scalar_glyphs_.set("Points Spheres Boxes Axes");
       scalar_glyphs_values_.set("points spheres boxes axes");
       vector_glyphs_.set("Lines Needles Arrows Cones Comets Disks Rings Springs");
       vector_glyphs_values_.set("lines needles arrows cones comets disks rings springs");
       tensor_glyphs_.set("Boxes {Colored Boxes} Ellipsoids Superquadrics");
       tensor_glyphs_values_.set("tboxes cboxes ellipsoids superquadrics");

  } else if (args[1] == "get_parameters"){
//This passes over to the TCL code the default parameters for the selected glyphs,
// The glyph_parameters format is; parameter name and then the default code for the parameter.
// This is repeated for each parameter for the glyph. For parameter names that have
// spaces in them, the name needs to be inclosed in curly braces {}.
// This can be moved to the other glyph code to keep all the 
// glyph functions together, and a function call executed here to
// load the TCL variables.

      int glyph_number = 0;
      glyph_number = atoi(args[2].c_str());
      //  glyph_parameters_.set(getGlyphParameter( glyph_number));
      switch (glyph_number)
       {
	   case 1:    glyph_parameters_.set("Scale 1001"); break; //points
	   case 2:    glyph_parameters_.set("Scale 1001"); break; //spheres
	   case 3:    glyph_parameters_.set("Scale 1001"); break; //boxes
	   case 4:    glyph_parameters_.set("Scale 1001"); break; //axes
	   case 16:   glyph_parameters_.set("Orientation 2010 Length 2001"); break; //lines
	   case 32:   glyph_parameters_.set("Orientation 2010 Length 2001"); break; //needles
	   case 48:   glyph_parameters_.set("Orientation 2010 Length 2001"); break; //arrows
	   case 64:   glyph_parameters_.set("Orientation 2010 Length 2001"); break; //cones
	   case 80:   glyph_parameters_.set("Orientation 2010 Length 2001"); break; //comets
	   case 96:   glyph_parameters_.set("Orientation 2010 Diameter 2001 Thickness 1003"); break; //disks
	   case 112:  glyph_parameters_.set("Orientation 2010 Diameter 2001 {Ring Diameter} 1003"); break; //rings
	   case 128:  glyph_parameters_.set("Orientation 2010 Length 2001 {Major Radius} 1002 {Minor Radius} 1003 Pitch 1003 Thickness 1003"); break; //springs
	   case 256:  glyph_parameters_.set("Tensor 4100"); break; //tboxes
	   case 512:  glyph_parameters_.set("Tensor 4100"); break; //cboxes
	   case 768:  glyph_parameters_.set("Tensor 4100"); break; //ellipsoids
	   case 1024: glyph_parameters_.set("Tensor 4100"); break; //superquadrics
	  default : glyph_parameters_.set(""); break; //no parameters
       }

  } else if (args[1] == "get_image"){
//This passes over to the TCL code the image for the selected image as a Base-64 encoded string,
// Glyphs are Gif images of size 192x132 that are then encoded are Base-64 encoded, and then copied into the code.
// Nathan has all the master non-encoded images.
// I used the website "http://www.motobit.com/util/base64-decoder-encoder.asp" to encode the files.
// This can be moved to the other glyph code to keep all the 
// glyph functions together, and a function call executed here to
// load the TCL variables.


      glyph_value_.reset();
      printf ("inside glyph_image %d\n", glyph_value_.get());
      cout << "glyph_value_ has the value of " << glyph_value_.get() << endl;

      int glyph_number = 0;
      glyph_number = atoi(args[2].c_str());

      //  glyph_image_.set(getGlyphImage( glyph_number));
      switch (glyph_number)
       {
	   case 1:   //points
	       glyph_image_.set("R0lGODlhwACEAMQAAL6+vrW1tbKysqmpqaGhoZ2dnZmZmYqKioiIiIeHh3V1dXR0dGhoaGdnZ2ZmZmNjY1tbW1VVVf///wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACH5BAEAABIALAAAAADAAIQAAAX/ICCOZGmeaKqubOu+cCzPdG3feK7vfO//wKBwSCwaj8ikcslsOp/QqHRKrVqv2Kx2y+16v+CweEwum8/otHrNbrvf8Lh8Tq/b7/i8fs/v+/+AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKlMAQUJCg4QERAOCgkFAapHAwgMEb6/wBEMCAO5QgIHD8HLwA8HAsY+BAvM1cALBNE7Bg3W3r4NBto4Bsrf3w/i4zQE3efnDdnrMQLU7+8L0PMvB/f+B/tcDDDn79yDYgFXICjoD0FCFQF6MXzHANfDEwUm+itw8UQCjfcSdDShAOQ7BSNLsTgwec5BShKxWHqD8HKEzHM1RcS8yYxmzpU8mbnMWTLoMpQ5PxoNJjJnxqXAOOaMCNVXxZwiFlZ1iBXAQKgHu4rotxSgWAD1jOY7K6Idz3hsR5STmS4uCW4mw9ktMU0jtr0mkBE0+Awwil0SrQ1DaDgFK1ewZNGyZbGx5cuYM2vezLmz58+gQ4seTbq06dOoU6tezbq169ewY8ueTbu27du4c+vezbu379/AgwsfTvxsCAA7");
	      break;
	   case 2:    //spheres
	       glyph_image_.set("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");
	       break;
	   case 3:    //boxes
	   case 256:  //tboxes 
	   case 512:  //cboxes 
	       glyph_image_.set("R0lGODlhwACEAKIAAL6+vqKiopqamnx8fDAwMAAAAP///wAAACH5BAEAAAYALAAAAADAAIQAAAP/CLrc/jDKSau9OOvNu/9gKI5kaZ5oqq5s675wLM90bd94ru987//AoHBILBqPyKRyyWw6n9CodEqtWq/YrHarK3i/4LB4TC6bwxCCes1uu9fcSyFAr9vv+Lx+z6974zVzfYOEhXkFAwWAM4KGjo96iImLMY2Ql5CSk5QulpifhJqbnCueoKeRA6qqiqQqpqixfqurra4nsLKxorW3uLrAs7S9viS5wZ+8tLbFIMfIl8rLzSHP0I/S09Qe1teG2drbG93eocPnrOLj5ajgw8zqFeTsfO7v8XL0oPb3+BTz+g6hG5jOnwSAAe/wOwfPIIMzECNKNEOwYkOHCgoI2Mix/6PHjyBDihzJcSFDjA80klzJsiVIk/1QPnRJs+ZImOFkZrTJs6cAnMQy/Hl4sYpKn0hZAi0oFN5QLUeTSg25dFTTBU+hTt36siLBov9sZc0SlSvXqmDDZkw7pazZqWg7/BlL9u3buHK/AHJrFyleDmD29j3rdSBbtYIHwy2M7vAEunUVS/0L2LEUvpJrUtYAOXJmn5vl6F2E+XPL0BZGkzYNmvFJnVhZ90SNsbTsm65jwrZ9m2runDp59/5I26Hw4R2LGzyOfKNyf8ybP8cXHfn0eNWHX1eXvfd2cd1vf98WXvZ4auVZn2+W3vT6Yu0/v/cVP/P8W/Ul33eVX/F+Uv/9DfYfJwH2NSAlBdpF2xsMNohFgnf9xhRsAEBoFlCWYdfcab9lqOGGK8Hk4Ycg4sbYiCSW6Fthh5EBnoomWiTPGC/CuOJX43QGoI03NpYjip7xmByOnOnIn5DEGfYjdUh6JA2QqnHX5JC6iQYkFxYSBpyVaJA3ZUlbchmYl1/+FBSFcpRpplVopqYmIldSmOVWRrYZwUR45gmRnXE06GcbfAYq6KCEFmrooYgSIISiiEbAKBCPNupApD5QKukClvKQ6aWb6tBpo5/iEOqho85IVAilFpoqYnVesOqgrx7USqsWxBqorXdG+QGudvIKAa2uXvqArynFOaywk44wJqo0yDZA7KkVGsvAs7BRu9a1zDaLaTXMADuBtTKBi4K42pZr7rnopqvuuuy26+678MYr76AJAAA7");
	       break;
	   case 4:    //axes
	       glyph_image_.set("R0lGODlhwACEAJEAAL6+vn9/f////wAAACH5BAEAAAIALAAAAADAAIQAAAL/hI+py+0Po5y02ouz3rz7D4biSJbmiabqyrbuC8fyTNf2jef6zvf+DwwKh8Si8YhMKpfMpvMJjUqn1Kr1is1qt9yu9wsOi8fksvmMTqvX7Lb7vQ3IA3DqnF6X3vP6OT+69/cUKNhEWLh0iJikuHjU6FgEGTk0SRlkefmTqdnD2bnzCZojOnpTalqDmjqzyhrj+voSK9tCW7tyi5uiu3vS61sCHDwyTBxifPyRrNzB3LzxDJ0hPX1RbU1xh5ftsP0NHi4+Tl6+TWmerr6ujs7+Dv/uHk9fH97t7Yc/q79v2+8vF8CAvAYS/GXwoLCECosxbIjsIcRlEic6q2gxGsaMWtQ2crzm8WMFbB9JcjSZEaVFlRNZQnTZEKZCmQdpErQZEKc/nft44vPZDWg2odaITjMKDanIpUybOn0KNarUqVSrWr2KNavWrVy7ev0KNqzYsWTLmj2LNm2NAgA7");
	       break;
	   case 16:   //lines
	       glyph_image_.set("R0lGODlhwACEAJEAAL6+vn9/f////wAAACH5BAEAAAIALAAAAADAAIQAAALohI+py+0Po5y02ouz3rz7D4biSJbmiabqyrbuC8fyTNf2jef6zvf+DwwKh8Si8YhMKpfMpvMJjUqn1Kr1is1qt9yu9wsOi8fksvmMTqvX7Lb7DY/L5/S6/Y7P6/f8vv8PGCg4SFhoeIiYqLjI2Oj4CBkpOUlZaXmJmam5ydnp+QkaehhAWmp6ipqqusra6voKyxoVS1tre4srC5XL2+v7O/srPEysKnqMnKy8zNzs/AwdLT1NXW19jZ2tvc3d7f0NHi4+Tl5ufo6err7O3u7+Dh8vP09fb3+Pn6+/z9/v/w8woMCBBPkVAAA7");
	       break;
	   case 32:   //needles
	       glyph_image_.set("R0lGODlhwACEAOYAAL6+vqenp6WlpaSkpKOjo6KioqGhoaCgoJ+fn56enp2dnZycnJubm5mZmZiYmJeXl5aWlpWVlZSUlJOTk5KSkpGRkZCQkI+Pj46Ojo2NjYyMjIuLi4qKiomJiYiIiIeHh4aGhoWFhYSEhIODg4KCgoGBgYCAgH9/f35+fn19fXx8fHt7e3p6enl5eXh4eHd3d3Z2dnV1dXR0dHNzc3JycnFxcXBwcG9vb25ubm1tbWxsbGtra2pqamlpaWhoaGdnZ2ZmZmVlZWRkZGNjY2JiYmFhYWBgYF9fX15eXl1dXVxcXFtbW1paWllZWVhYWP///wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACH5BAEAAE8ALAAAAADAAIQAAAf/gACCg4SFhoeIiYqLjI2Oj5CRkpOUlZaXmJmam5ydnp+goaKjpKWmp6ipqqusra6vsLGys7S1tre4ubq7vL2+v8DBwsPExcbHyMnKy8zNzs/Q0dLT1NXW19jZ2tvc3d7f4OHi4+Tl5ufo6err7O3u7/Dx8vP09fb3+Pn6+/z9/v8AAwocSLCgwYMIEypcyLChw4cQI0qcSLGixYsYM2rcSEsAgY8FQh5AgECBggULGDRo4OCBSwgSJEygQKGChQsXMGTIoGHDhg4dPHj4EKJoiBEjSJQoceIEChQpVKhYsaKFCxcvXsDYKkPGDBpga9iwcQMHjhw5dqjtwcPHj7dA/4IEETJkCJEiRIgYOXIEiZIkSpQwYdKkcJNmHkEWMDAygUmUKlm6fABTZoXLOHPy9MkhqFAQRkOQSLq06dOoU6tezbo1xozXNWKLvVEWhw4danf06OEWrlwhee8WKcL3SODjhp0oRzygOQGRJB+nXNnyZYQIEmjatGBB5+afnokaRaqUhOmnUqWycGFVK9fXYGmIJWs2B+61vN/+iDs3+HDifAEW2BKGNeHEYcwEIMCCAhTw3AEjlXQSAyo5YCFlEGBn2WXdYYCBBj1twAEHQ30AAmghiEAeCSaY4BQKKkTFAgstWIVVa6/BFhttZp11X1s+8AZEXELQ5Z8ReyGRBFtgSyxBWGEcRSnllFRWaeWVWGap5ZZcdunll2CGKeaYZJZp5plopqnmmmy26eabcMYp55x01mnnnXjmqeeefPbp55+ABirooIQWauihiCaq6KKMNuroo5BGakwgADs=");
	       break;
	   case 48:   //arrows
	       glyph_image_.set("R0lGODlhwACEAMQAAL6+vri4uLOzs62traioqKKiop2dnZeXl5KSko2NjYeHh4KCgnx8fHd3d3FxcWxsbGZmZv///wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACH5BAEAABEALAAAAADAAIQAAAX/ICCOZGmeaKqubOu+cCzPdG3feK7vfO//wKBwSCwaj8ikcslsOp/QqHRKrVqv2Kx2y+16v+CweEwum8/otHrNbrvf8Lh8Tq/b7/i8fs/v+/+AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrqkIDgoGAq9MCBC5EA0JBQG2R7i6ww0IBMBDwsPLEAsHA8g+yszMDwu00TnT1NyyBr/ZM9vc5Lu94TDj5esMxugr6uvy1s9bCPf4+fr7/P3+/wADCuTHQJ5Bct5qWTnIsKHDhxAjPnTQC1wUiRgzatzIUYEUjiBDigRJ0dcUBihTiapcybKly5cwY8qc2dLByGEJ34mIh5EeNJ0keD5sdwzoCaEGeRUwCg+iN4tMVSAdZg1b1BdTGdS7KiNesaJcaUxTCjWs2Jxm06pdy7at27dw48qdS7eu3bt48+rdy7ev37+AAwseTLiw4cOIEytezLix48eQI0ueTLmy5cuYM2vezLmz58+gGYUAADs=");
	       break;
	   case 64:   //cones
	       glyph_image_.set("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");
	       break;
	   case 80:   //comets
	       glyph_image_.set("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");
	       break;
	   case 96:   //disks
	       glyph_image_.set("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");
	       break; 
	   case 112:  //rings
	       glyph_image_.set("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");
	       break; 
	   case 128:  //springs
	       glyph_image_.set("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");
	       break;
// 	   case 256:  //tboxes 
// 	       glyph_image_.set("");
// 	       break;
// 	   case 512:   //cboxes
// 	       glyph_image_.set("");
// 	       break;
	   case 768:  //ellipsoids 
	       glyph_image_.set("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");
	       break;
	   case 1024:  //superquadrics 
	       glyph_image_.set("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");
 	       break;
	   default : //blank
	       glyph_image_.set("R0lGODlhwACEAIAAAL6+vv///yH5BAEAAAEALAAAAADAAIQAAALHhI+py+0Po5y02ouz3rz7D4biSJbmiabqyrbuC8fyTNf2jef6zvf+DwwKh8Si8YhMKpfMpvMJjUqn1Kr1is1qt9yu9wsOi8fksvmMTqvX7Lb7DY/L5/S6/Y7P6/f8vv8PGCg4SFhoeIiYqLjI2Oj4CBkpOUlZaXmJmam5ydnp+QkaKjpKWmp6ipqqusra6voKGys7S1tre4ubq7vL2+v7CxwsPExcbHyMnKy8zNzs/AwdLT1NXW19jZ2tvc3d7f0NHi4+Tk5ZAAA7");
       }
  } else {
    Module::tcl_command(args, userdata);
  }
}

void
ShowFieldGlyphs::post_read()
{
//This will need to be updated, if it can since the whole structure of the GUI has changed.

  char names[36][2][30] = { {"vectors-on", "vectors_on" },
		            {"normalize-vectors", "vectors_normalize" },
		            {"has_vector_data", "vectors_has_data" },
		            {"bidirectional", "vectors_bidirectional" },
// Special case	            {"vectors-usedefcolor", "vectors_color_type" },

		            {"tensors-on", "tensors_on" },
		            {"has_tensor_data", "tensors_has_data" },
// Special case	            {"tensors-usedefcolor","tensors_color_type"  },
		            {"tensors-emphasis", "tensors_emphasis" },

		            {"scalars-on", "scalars_on" },
		            {"scalars-transparency", "scalars_transparency" },
// Special case	            {"scalars-usedefcolor", "scalars_color_type" },
		            {"has_scalar_data", "scalars_has_data" },

			    {"text-on", "text_on" },
			    {"text-use-default-color", "text_color_type" },
			    {"text-color-r", "text_color-r" },
			    {"text-color-g", "text_color-g" },
			    {"text-color-b", "text_color-b" },
			    {"text-backface-cull", "text_backface_cull" },
			    {"text-always_visible", "text_always_visible" },
			    {"text-fontsize", "text_fontsize" },
			    {"text-precision","text_precision"  },
			    {"text-render_locations", "text_render_locations" },
			    {"text-show-data",  "text_show_data" },
			    {"text-show-nodes", "text_show_nodes" },
			    {"text-show-edges", "text_show_edges" },
			    {"text-show-faces", "text_show_faces" },
			    {"text-show-cells", "text_show_cells" },
		       
			    {"def-color-r", "def_color-r" },
			    {"def-color-g", "def_color-g" },
			    {"def-color-b", "def_color-b" },
// Special case		    {"def-color-a", "def_color-a" },

		            {"data_display_type", "vectors_display_type" },
		            {"tensor_display_type", "tensors_display_type" },
		            {"scalar_display_type", "scalars_display_type" },

// No Change		    {"scalars_scale",   "scalars_scale" },
// No Change		    {"scalars_scaleNV", "scalars_scaleNV" },
// No Change		    {"vectors_scale",   "vectors_scale" },
// No Change		    {"vectors_scaleNV", "vectors_scaleNV" },
// No Change		    {"tensors_scale",   "tensors_scale" },
// No Change		    {"tensors_scaleNV", "tensors_scaleNV"  },

// No Change		    {"active_tab", "active_tab" },
// No Change		    {"interactive_mode", "interactive_mode" },
// No Change		    {"show_progress", "show_progress" },
			    {"field-name","field_name"  },
			    {"field-name-override","field_name_override"  },
//		            {"field-name-update", },
		            {"data-resolution", "vectors_resolution" },
			    {"approx-div", "approx_div" },
			    {"use-default-size", "use_default_size" } };

  // Get the module name
  const string modName = get_ctx()->getfullname() + "-";

  string val;

  for( unsigned int i=0; i<36; i++ )
  {
    // Get the current values for the old names
    if( get_gui()->get(modName+names[i][0], val, get_ctx()) )
    {
      // Set the current values for the new names
      get_gui()->set(modName+names[i][1], val, get_ctx());
    }
  }

  // Special cases for the old default color vars
  if( get_gui()->get(modName+"scalars-usedefcolor", val, get_ctx()) )
  {
    if( val == string("1") )
      get_gui()->set(modName+"scalars_color_type", "0", get_ctx());
  }

  if( get_gui()->get(modName+"vectors-usedefcolor", val, get_ctx()) )
  {
    if( val == string("1") )
      get_gui()->set(modName+"vectors_color_type", "0", get_ctx());
  }

  if( get_gui()->get(modName+"tensors-usedefcolor", val, get_ctx()) )
  {
    if( val == string("1") )
      get_gui()->set(modName+"tensors_color_type", "0", get_ctx());
  }

  if( get_gui()->get(modName+"def-color-a", val, get_ctx()) )
  {
    double dval = atof( val.c_str() );

    dval = dval * dval * dval * dval;

    val = to_string( dval );

    get_gui()->set(modName+"def_color-a", val, get_ctx());
  }
}

DECLARE_MAKER(ShowFieldGlyphs)
} // End namespace SCIRun
