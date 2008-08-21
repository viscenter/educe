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
 *  RenderFieldData.h: Rendering alogrithms for data
 *
 *  Written by:
 *   Allen R. Sanderson
 *   Martin Cole
 *   SCI Institute
 *   University of Utah
 *   April 2007
 *
 *  Copyright (C) 2007 SCI Institute
 */

#if !defined(Visualization_RenderField_h)
#define Visualization_RenderField_h

#include <Core/Algorithms/Visualization/RenderFieldState.h>
#include <Core/Basis/QuadBilinearLgn.h>
#include <Core/Containers/StringUtil.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/ImageMesh.h>
#include <Core/Datatypes/TetVolMesh.h>
#include <Core/Geom/GeomMaterial.h>
#include <Core/Geom/GeomGroup.h>
#include <Core/Geom/GeomSwitch.h>
#include <Core/Geom/GeomGlyph.h>
#include <Core/Geom/GeomLine.h>
#include <Core/Geom/GeomPoint.h>
#include <Core/Geom/GeomTriangles.h>
#include <Core/Geom/GeomQuads.h>
#include <Core/Geom/GeomText.h>
#include <Core/Geom/GeomDL.h>
#include <Core/Geom/GeomColorMap.h>
#include <Core/Geom/GeomTexRectangle.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Tensor.h>
#include <Core/Math/MiscMath.h>
#include <Core/Math/Trig.h>
#include <Core/Math/TrigTable.h>
#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <sci_hash_map.h>
#include <sstream>
#include <iomanip>
#include <iostream>

#if defined(_WIN32) && !defined(uint)
  // for some reason, this isn't defined...
#define uint unsigned
#endif


#include <Core/Algorithms/Visualization/share.h>

namespace SCIRun {

typedef ImageMesh<QuadBilinearLgn<Point> > IMesh;

//! RenderFieldBase supports the dynamically loadable algorithm concept.
//! when dynamically loaded the user will dynamically cast to a 
//! RenderFieldBase from the DynamicAlgoBase they will have a pointer to.
class SCISHARE RenderFieldBase :
  public DynamicAlgoBase, public RenderStateBase
{
public:

#if !defined(_MSC_VER) && !defined(__ECC)

  struct equal_str
  {
    bool operator()(const string &s1, const string &s2) const
    {
      return s1 == s2;
    }
  };
  
  struct str_hasher
  {
    size_t operator()(const string s) const
    {
      hash<const char*> H;
      return H(s.c_str());
    }
  };
#endif

  virtual GeomHandle render_mesh_nodes(FieldHandle field_handle, 
				       ColorMapHandle colormap_handle,
				       const string &display_mode,
				       double scale,
				       int resolution,
				       bool use_def_color,
				       bool use_transparency)
  {
    unsigned int render_state = 0;
    switch_flag( render_state, IS_ON, 1 );
    switch_flag( render_state, USE_DEFAULT_COLOR, use_def_color );
    switch_flag( render_state, USE_TRANSPARENCY,  use_transparency );
    switch_flag( render_state, USE_COLORMAP,      colormap_handle.get_rep() );

    if (field_handle->basis_order() == -1 || colormap_handle.get_rep() == 0)
      switch_flag( render_state, USE_DEFAULT_COLOR, true );
    
    return render_mesh_nodes( field_handle,
			      display_mode, scale, resolution, render_state);
  }
  
  virtual GeomHandle render_mesh_edges(FieldHandle field_handle,
				       ColorMapHandle colormap_handle,
				       const string &display_mode,
				       double scale,
				       int resolution,
				       bool use_def_color,
				       bool use_transparency,
				       unsigned int approx_div)
  {
    unsigned int render_state = 0;
    switch_flag( render_state, IS_ON, 1 );
    switch_flag( render_state, USE_DEFAULT_COLOR, use_def_color );
    switch_flag( render_state, USE_TRANSPARENCY,  use_transparency );
    switch_flag( render_state, USE_COLORMAP,      colormap_handle.get_rep() );

    if (field_handle->basis_order() == -1 || colormap_handle.get_rep() == 0)
      switch_flag( render_state, USE_DEFAULT_COLOR, true );

    return render_mesh_edges( field_handle, display_mode,
			      scale, resolution, render_state, approx_div);

  }

  virtual GeomHandle render_mesh_faces(FieldHandle field_handle, 
				       ColorMapHandle colormap_handle,
				       bool use_normals,
				       bool use_def_color,
				       bool use_transparency,
				       bool use_texture,
				       unsigned int approx_div)
  {
    unsigned int render_state = 0;
    switch_flag( render_state, IS_ON, 1 );
    switch_flag( render_state, USE_DEFAULT_COLOR, use_def_color );
    switch_flag( render_state, USE_TRANSPARENCY,  use_transparency );
    switch_flag( render_state, USE_COLORMAP,      colormap_handle.get_rep() );
    switch_flag( render_state, USE_NORMALS, use_normals );
    switch_flag( render_state, USE_TEXTURE, use_texture );

    if (field_handle->basis_order() == -1 || colormap_handle.get_rep() == 0)
      switch_flag( render_state, USE_DEFAULT_COLOR, true );

    return render_mesh_faces(field_handle, colormap_handle,
			     render_state, approx_div);
  }


  virtual GeomHandle render_mesh_nodes(FieldHandle field_handle, 
				       const string &display_mode,
				       double scale, int resolution,
				       unsigned int render_state) = 0;

  virtual GeomHandle render_mesh_edges(FieldHandle field_handle,
				       const string &display_mode,
				       double scale, int resolution,
				       unsigned int render_state,
				       unsigned int approx_div) = 0;

  virtual GeomHandle render_mesh_faces(FieldHandle field_handle, 
				       ColorMapHandle color_handle,
				       unsigned int render_state,
				       unsigned int approx_div) = 0;

  virtual GeomHandle render_text(FieldHandle field_handle,
				 ColorMapHandle color_handle,
				 bool use_colormap,
				 bool use_default_color,
				 bool backface_cull_p,
				 int  fontsize,
				 int  precision,
				 bool render_locations,
				 bool render_data,
				 bool render_nodes,
				 bool render_edges,
				 bool render_faces,
				 bool render_cells,
				 bool always_visible) = 0;

  RenderFieldBase();
  virtual ~RenderFieldBase();

  //! support the dynamically compiled algorithm concept
  static CompileInfoHandle get_compile_info(const TypeDescription *ftd,
					    const TypeDescription *ltd);
  
  GeomHandle node_switch_;
  GeomHandle edge_switch_;
  GeomHandle face_switch_;
};


template <class FIELD, class Loc>
class RenderField : public RenderFieldBase
{
public:
  //! virtual interface. 
  virtual GeomHandle render_mesh_nodes(FieldHandle field_handle, 
				       const string &display_mode,
				       double scale,
				       int resolution,
				       unsigned int render_state );

  virtual GeomHandle render_mesh_edges(FieldHandle field_handle,
				       const string &display_mode,
				       double scale,
				       int resolution,
				       unsigned int render_state,
				       unsigned int approx_div);

  virtual GeomHandle render_mesh_faces(FieldHandle field_handle, 
				       ColorMapHandle color_handle,
				       unsigned int render_state,
				       unsigned int approx_div);

  virtual GeomHandle render_text(FieldHandle field_handle,
				 ColorMapHandle color_handle,
				 bool use_color_map,
				 bool use_default_color,
				 bool backface_cull_p,
				 int fontsize,
				 int precision,
				 bool render_locations,
				 bool render_data,
				 bool render_nodes,
				 bool render_edges,
				 bool render_faces,
				 bool render_cells,
				 bool always_visible);

protected:
  GeomHandle render_nodes(FieldHandle field_handle,
			  const string &display_mode,
			  double scale,
			  int resolution,
			  unsigned int render_state);


  GeomHandle render_edges(FieldHandle field_handle,
			  const string &display_mode,
			  double scale,
			  int resolution,
			  unsigned int render_state,
			  unsigned int approx_div);

  GeomHandle render_edges_linear(FieldHandle field_handle,
				 const string &display_mode,
				 double scale,
				 int resolution,
				 unsigned int render_state);

  GeomHandle render_faces(FieldHandle field_handle,
			  unsigned int render_state,
			  unsigned int approx_div);

  GeomHandle render_faces_linear(FieldHandle field_handle,
				 unsigned int render_state);         

  virtual GeomHandle render_faces_texture(FieldHandle field_handle,
					  ColorMapHandle color_handle,
					  unsigned int render_state)
  {
    return 0;
  }


  GeomHandle render_text_data(FieldHandle field_handle,
			      ColorMapHandle color_handle,
			      bool use_color_map,
			      bool use_default_color,
			      bool backface_cull_p,
			      int fontsize,
			      int precision,
			      bool always_visible);

  GeomHandle render_text_data_nodes(FieldHandle field_handle,
				    ColorMapHandle color_handle,
				    bool use_color_map,
				    bool use_default_color,
				    bool backface_cull_p,
				    int fontsize,
				    int precision,
				    bool always_visible);

  GeomHandle render_text_nodes(FieldHandle field_handle,
			       ColorMapHandle color_handle,
			       bool use_color_map,
			       bool use_default_color,
			       bool backface_cull_p,
			       int fontsize,
			       int precision,
			       bool render_locations,
			       bool always_visible);

  GeomHandle render_text_edges(FieldHandle field_handle,
			       ColorMapHandle color_handle,
			       bool use_color_map,
			       bool use_default_color,
			       int fontsize,
			       int precision,
			       bool render_locations,
			       bool always_visible);

  GeomHandle render_text_faces(FieldHandle field_handle,
			       ColorMapHandle color_handle,
			       bool use_color_map,
			       bool use_default_color,
			       int fontsize,
			       int precision,
			       bool render_locations,
			       bool always_visible);

  GeomHandle render_text_cells(FieldHandle field_handle,
			       ColorMapHandle color_handle,
			       bool use_color_map,
			       bool use_default_color,
			       int fontsize,
			       int precision,
			       bool render_locations,
			       bool always_visible);

  void add_edge_geom(GeomLines *lines,
		     GeomGlyphBase *glyphs,
		     const Point &p0, const Point &p1,
		     const double radius,
		     const int resolution,
		     bool cylinders_p,
		     unsigned int color_scheme,
		     double scol0,
		     double scol1,
		     MaterialHandle& vcol0,
		     MaterialHandle& vcol1);

  void add_face_geom(GeomFastTriangles *faces,
		     GeomFastQuads *qfaces,
		     const vector<Point>  &points,
		     const vector<Vector> &normals,
		     bool with_normals,
		     unsigned int color_scheme,
		     vector<double> &scols,
		     vector<MaterialHandle> &vcols );
};



class SCISHARE RenderFieldV : public RenderFieldBase
{
public:
  //! virtual interface. 
  virtual GeomHandle render_mesh_nodes(FieldHandle field_handle, 
				       const string &display_mode,
				       double scale,
				       int resolution,
				       unsigned int render_state );

  virtual GeomHandle render_mesh_edges(FieldHandle field_handle,
				       const string &display_mode,
				       double scale,
				       int resolution,
				       unsigned int render_state,
				       unsigned int approx_div);

  virtual GeomHandle render_mesh_faces(FieldHandle field_handle, 
				       ColorMapHandle color_handle,
				       unsigned int render_state,
				       unsigned int approx_div);

  virtual GeomHandle render_text(FieldHandle field_handle,
				 ColorMapHandle color_handle,
				 bool use_color_map,
				 bool use_default_color,
				 bool backface_cull_p,
				 int fontsize,
				 int precision,
				 bool render_locations,
				 bool render_data,
				 bool render_nodes,
				 bool render_edges,
				 bool render_faces,
				 bool render_cells,
				 bool always_visible);

protected:
  GeomHandle render_nodes(FieldHandle field_handle,
			  const string &display_mode,
			  double scale,
			  int resolution,
			  unsigned int render_state);


  GeomHandle render_edges(FieldHandle field_handle,
			  const string &display_mode,
			  double scale,
			  int resolution,
			  unsigned int render_state,
			  unsigned int approx_div);

  GeomHandle render_edges_linear(FieldHandle field_handle,
				 const string &display_mode,
				 double scale,
				 int resolution,
				 unsigned int render_state);

  GeomHandle render_faces(FieldHandle field_handle,
			  unsigned int render_state,
			  unsigned int approx_div);

  GeomHandle render_faces_linear(FieldHandle field_handle,
				 unsigned int render_state);         

  virtual GeomHandle render_faces_texture(FieldHandle field_handle,
					  ColorMapHandle color_handle,
					  unsigned int render_state)
  {
    return 0;
  }

  GeomHandle render_text_data(FieldHandle field_handle,
			      ColorMapHandle color_handle,
			      bool use_color_map,
			      bool use_default_color,
			      bool backface_cull_p,
			      int fontsize,
			      int precision,
			      bool always_visible);

  GeomHandle render_text_data_nodes(FieldHandle field_handle,
				    ColorMapHandle color_handle,
				    bool use_color_map,
				    bool use_default_color,
				    bool backface_cull_p,
				    int fontsize,
				    int precision,
				    bool always_visible);

  GeomHandle render_text_nodes(FieldHandle field_handle,
			       ColorMapHandle color_handle,
			       bool use_color_map,
			       bool use_default_color,
			       bool backface_cull_p,
			       int fontsize,
			       int precision,
			       bool render_locations,
			       bool always_visible);

  GeomHandle render_text_edges(FieldHandle field_handle,
			       ColorMapHandle color_handle,
			       bool use_color_map,
			       bool use_default_color,
			       int fontsize,
			       int precision,
			       bool render_locations,
			       bool always_visible);

  GeomHandle render_text_faces(FieldHandle field_handle,
			       ColorMapHandle color_handle,
			       bool use_color_map,
			       bool use_default_color,
			       int fontsize,
			       int precision,
			       bool render_locations,
			       bool always_visible);

  GeomHandle render_text_cells(FieldHandle field_handle,
			       ColorMapHandle color_handle,
			       bool use_color_map,
			       bool use_default_color,
			       int fontsize,
			       int precision,
			       bool render_locations,
			       bool always_visible);

  void add_edge_geom(GeomLines *lines,
		     GeomGlyphBase *glyphs,
		     const Point &p0, const Point &p1,
		     const double radius,
		     const int resolution,
		     bool cylinders_p,
		     unsigned int color_scheme,
		     double scol0,
		     double scol1,
		     MaterialHandle& vcol0,
		     MaterialHandle& vcol1);

  void add_face_geom(GeomFastTriangles *faces,
		     GeomFastQuads *qfaces,
		     const vector<Point>  &points,
		     const vector<Vector> &normals,
		     bool with_normals,
		     unsigned int color_scheme,
		     vector<double> &scols,
		     vector<MaterialHandle> &vcols );
};




template <class FIELD, class Loc>
GeomHandle
RenderField<FIELD, Loc>::render_mesh_nodes(FieldHandle field_handle, 
					   const string &display_mode,
					   double scale,
					   int resolution,
					   unsigned int render_state)
{
  return render_nodes(field_handle, display_mode,
		      scale, resolution, render_state );
}
  

template <class FIELD, class Loc>
GeomHandle
RenderField<FIELD, Loc>::render_mesh_edges(FieldHandle field_handle,
					   const string &display_mode,
					   double scale,
					   int resolution,
					   unsigned int render_state,
					   unsigned int approx_div)
{
  FIELD *fld = dynamic_cast<FIELD*>(field_handle.get_rep());
  
  typename FIELD::mesh_handle_type mesh = fld->get_typed_mesh();
  bool do_linear = (fld->basis_order() < 2 && 
		    mesh->get_basis().polynomial_order() < 2 &&
		    approx_div == 1);

  if (do_linear) {
    return render_edges_linear(field_handle, display_mode,
			       scale, resolution, render_state);
  } else {
    return render_edges(field_handle, display_mode,
			scale, resolution, render_state, approx_div);
  }
}


template <class FIELD, class Loc>
GeomHandle
RenderField<FIELD, Loc>::render_mesh_faces(FieldHandle field_handle, 
					   ColorMapHandle colormap_handle,
					   unsigned int render_state,
					   unsigned int approx_div)
{
  FIELD *fld = dynamic_cast<FIELD*>(field_handle.get_rep());

  // If an ImageMesh, it can be rendered as a single polygon
  // with a textured face, providing the flag is true.
  const TypeDescription *mtd = fld->get_type_description(Field::MESH_TD_E);
  
  if((mtd->get_name().find("ImageMesh") != string::npos) &&
     get_flag(render_state, USE_TEXTURE))
  {
    return render_faces_texture(field_handle, colormap_handle, render_state);
  }

  else
  {
    typename FIELD::mesh_handle_type mesh = fld->get_typed_mesh();
    bool do_linear = (fld->basis_order() < 2 && 
		      mesh->get_basis().polynomial_order() < 2 &&
		      approx_div == 1);

    if (do_linear)
      return render_faces_linear(field_handle, render_state );
    else
      return render_faces(field_handle, render_state, approx_div);
  }
}


template <class FIELD, class Loc>
GeomHandle
RenderField<FIELD, Loc>::render_nodes(FieldHandle field_handle, 
				      const string &display_mode,
				      double scale,
				      int resolution,
				      unsigned int render_state )
{
  FIELD *fld = dynamic_cast<FIELD*>(field_handle.get_rep());
  typename FIELD::mesh_handle_type mesh = fld->get_typed_mesh();

  GeomPoints    *points;
  GeomGlyphBase *glyphs;
  GeomHandle data_switch;

  // 0 Points 1 Spheres
  bool points_p  = (display_mode == "Points");
  bool spheres_p = (display_mode == "Spheres");

  if (points_p) // Points
  {
    if (get_flag(render_state, USE_TRANSPARENCY))
      points = new GeomTranspPoints();
    else
      points = new GeomPoints();

    data_switch = new GeomDL(points);

    points->setPointSize(resolution/5.0);
  }
  else if (spheres_p) // Spheres
  {
    if (get_flag(render_state, USE_TRANSPARENCY))
      glyphs = new GeomTranspGlyph();
    else
      glyphs = new GeomGlyph();

    data_switch = new GeomDL(glyphs->getObj());
  }

  unsigned int color_scheme = 0;
  double scol;
  MaterialHandle vcol(0);

  if( fld->basis_order() < 0 ||
      (fld->basis_order() == 0 && mesh->dimensionality() != 0) ||
      get_flag(render_state, USE_DEFAULT_COLOR) )
  {
    color_scheme = 0; // Default color
  }
  else if (get_flag(render_state, USE_COLORMAP))
  {
    // Color map lookup using either a scalar value
    // the vector magnitude, the tensor.
    color_scheme = 1;
  }
  else // if (fld->basis_order() >= 1)
  {
    color_scheme = 2; // Value become RGB
    vcol = new Material(Color(1.0, 1.0, 1.0));
    if (get_flag(render_state, USE_TRANSPARENCY))
      vcol->transparency = 0.75;
    else
      vcol->transparency = 1.0;
  }

  mesh->synchronize(Mesh::NODES_E);
  typename FIELD::mesh_type::Node::iterator iter;      mesh->begin(iter);  
  typename FIELD::mesh_type::Node::iterator iter_end;  mesh->end(iter_end);  
  while (iter != iter_end) 
  {
    Point p;
    mesh->get_point(p, *iter);

    unsigned int n_idx = *iter;

    if( color_scheme ) 
    {
      typename FIELD::value_type val;
      fld->value(val, *iter);

      value_to_color( color_scheme, val, scol, vcol );
    }

    if (points_p)
    {  
      if (color_scheme == 0)
        points->add(p);
      else if (color_scheme == 1)
        points->add(p, scol);
      else //if (color_scheme == 2)
        points->add(p, vcol);
    }
    else // if (spheres_p)
    {
      if (color_scheme == 0)
        glyphs->add_sphere(p, scale, resolution, resolution);
      else if (color_scheme == 1)
        glyphs->add_sphere(p, scale, scol, resolution, resolution);
      else //if (color_scheme == 2)
        glyphs->add_sphere(p, scale, vcol, resolution, resolution);
    }

    ++iter;
  }

  return data_switch;
}


template <class FIELD, class Loc>
void
RenderField<FIELD, Loc>::add_edge_geom(GeomLines *lines,
				       GeomGlyphBase *glyphs,
				       const Point &p0, const Point &p1,
				       const double radius,
				       const int resolution,
				       bool cylinders_p,
				       unsigned int color_scheme,
				       double scol0,
				       double scol1,
				       MaterialHandle& vcol0,
				       MaterialHandle& vcol1)
{
  Vector vec = (Vector) (p1 - p0);

  if (color_scheme == 0)
  {
    if (cylinders_p)
      glyphs->add_cylinder(p0, vec, radius, radius, vec.length(), resolution );
    else
      lines->add(p0, p1);
  }
  else if (color_scheme == 1)
  {
    if (cylinders_p)
      glyphs->add_cylinder(p0, vec, radius, radius, vec.length(),
			   scol0, scol1, resolution);
    else
      lines->add(p0, scol0, p1, scol1);
  }
  else if (color_scheme == 2)
  {
    if ( cylinders_p )
      glyphs->add_cylinder(p0, vec, radius, radius, vec.length(),
			   vcol0, vcol1, resolution);
    else
      lines->add(p0, vcol0, p1, vcol1);
  }
}

//#define DEBUG_PRINT
#if defined(DEBUG_PRINT)
inline
void
print_coords(const char *pre, vector<double> &c0)
{
  cout << pre;
  vector<double>::iterator dbg_iter = c0.begin();
  while (dbg_iter != c0.end()) {
    cout << *dbg_iter++ << " ";
  }
  cout << endl;
}
#endif

template <class FIELD, class Loc>
GeomHandle
RenderField<FIELD, Loc>::render_edges(FieldHandle field_handle,
				      const string &edge_display_mode,
				      double scale,
				      int resolution,
				      unsigned int render_state,
				      unsigned div)
{
  FIELD *fld = dynamic_cast<FIELD *>(field_handle.get_rep());
  typename FIELD::mesh_handle_type mesh = fld->get_typed_mesh();

  GeomLines*     lines;
  GeomGlyphBase* glyphs;
  GeomHandle data_switch;

  // 0 Lines 1 Cylinders
  bool lines_p = (edge_display_mode == "Lines");
  bool cylinders_p = (edge_display_mode == "Cylinders");
  
  if (lines_p) // Lines
  {
    if (get_flag(render_state, USE_TRANSPARENCY))
      lines = new GeomTranspLines;
    else
      lines = new GeomLines;

    data_switch = new GeomDL(lines);

    lines->setLineWidth(resolution/5.0);
  }
  else if (cylinders_p) // Cylinders
  {
    if (get_flag(render_state, USE_TRANSPARENCY))
      glyphs = new GeomTranspGlyph();
    else
      glyphs = new GeomGlyph();

    data_switch = new GeomDL(glyphs->getObj());
  }

  unsigned int color_scheme = 0;
  double scol0, scol1;
  MaterialHandle vcol0(0);
  MaterialHandle vcol1(0);

  if (fld->basis_order() < 0 ||
      (fld->basis_order() == 0 && mesh->dimensionality() != 1) ||
      get_flag(render_state, USE_DEFAULT_COLOR))
  {
    color_scheme = 0; // Default color
  }
  else if (get_flag(render_state, USE_COLORMAP))
  {
    // Color map lookup using either a scalar value
    // the vector magnitude, the tensor.
    color_scheme = 1;
  }
  else // if (fld->basis_order() >= 1)
  {
    color_scheme = 2; // Values become RGB
    vcol0 = new Material(Color(1.0, 1.0, 1.0));
    vcol1 = new Material(Color(1.0, 1.0, 1.0));
    if (get_flag(render_state, USE_TRANSPARENCY))
      vcol0->transparency = vcol1->transparency = 0.75;
    else
      vcol0->transparency = vcol1->transparency = 1.0;
  }

#if defined(_MSC_VER) || defined(__ECC)
  typedef hash_set<string> edge_ht_t;
#else
  typedef hash_set<string, str_hasher, equal_str> edge_ht_t;
#endif
  edge_ht_t rendered_edges; 

  mesh->synchronize(Mesh::EDGES_E | Mesh::FACES_E | Mesh::CELLS_E);
  typename FIELD::mesh_type::Elem::iterator eiter; mesh->begin(eiter);  
  typename FIELD::mesh_type::Elem::iterator eiter_end; mesh->end(eiter_end);  
  while (eiter != eiter_end) 
  {  
    typename FIELD::mesh_type::Edge::array_type edges;
    mesh->get_edges(edges, *eiter);

    typename FIELD::mesh_type::Edge::array_type::iterator edge_iter;
    edge_iter = edges.begin();
    int ecount = 0;
    while (edge_iter != edges.end()) 
    {
      typename FIELD::mesh_type::Node::array_type nodes;
      typename FIELD::mesh_type::Edge::index_type eidx = *edge_iter++;

      Point cntr;
      mesh->get_center(cntr, eidx);
      ostringstream pstr;  
      pstr << setiosflags(ios::scientific);
      pstr << setprecision(7); 
      pstr << cntr.x() << cntr.y() << cntr.z();
      
      typename edge_ht_t::const_iterator it = rendered_edges.find(pstr.str());

      if (it != rendered_edges.end()) 
      {
        ++ecount;
        continue;
      } 
      else 
      {
        rendered_edges.insert(pstr.str());
      }
      // following print is useful for debugging edge ordering
//       cerr << "elem: " << *eiter << " count " << ecount 
// 	   << " edge" << eidx << std::endl;
      vector<vector<double> > coords;
      mesh->pwl_approx_edge(coords, *eiter, ecount++, div);
      vector<vector<double> >::iterator coord_iter = coords.begin();
      do 
      {
        vector<double> &c0 = *coord_iter++;
        if (coord_iter == coords.end()) break;
        vector<double> &c1 = *coord_iter;
        Point p0, p1;      

        // get the geometry at the approx.
        mesh->interpolate(p0, c0, *eiter);
        mesh->interpolate(p1, c1, *eiter);

        // get the field variables values at the approx (if they exist)
        if (color_scheme) 
        {
          typename FIELD::value_type val0, val1;

          fld->interpolate(val0, c0, *eiter);
          fld->interpolate(val1, c1, *eiter);

          value_to_color( color_scheme, val0, scol0, vcol0 );
          value_to_color( color_scheme, val1, scol1, vcol1 );
        }

        add_edge_geom(lines, glyphs, p0, p1,
		      scale, resolution, cylinders_p,
		      color_scheme, scol0, scol1, vcol0, vcol1);
	
      } 
      while (coords.size() > 1 && coord_iter != coords.end()); 
    }
    
    ++eiter;
  }
  
  return data_switch;
}


template <class FIELD, class Loc>
GeomHandle
RenderField<FIELD, Loc>::render_edges_linear(FieldHandle field_handle,
					     const string &display_mode,
					     double scale,
					     int resolution,
					     unsigned int render_state) 
{
  FIELD *fld = dynamic_cast<FIELD *>(field_handle.get_rep());
  typename FIELD::mesh_handle_type mesh = fld->get_typed_mesh();

  GeomLines*     lines;
  GeomGlyphBase* glyphs;
  GeomHandle data_switch;
  
  // 0 Lines 1 Cylinders
  bool lines_p     = (display_mode == "Lines");
  bool cylinders_p = (display_mode == "Cylinders");

  if (lines_p) // Lines
  {
    if (get_flag(render_state, USE_TRANSPARENCY))
    {
      lines = new GeomTranspLines;
      data_switch = lines;
    }
    else
    {
      lines = new GeomLines;
      data_switch = new GeomDL(lines);
    }

    lines->setLineWidth(resolution/5.0);
  }
  else if (cylinders_p) // Cylinders
  {
    if (get_flag(render_state, USE_TRANSPARENCY))
    {
      glyphs = new GeomTranspGlyph();
      data_switch = glyphs->getObj();
    }
    else
    {
      glyphs = new GeomGlyph();
      data_switch = new GeomDL(glyphs->getObj());
    }
  }

  unsigned int color_scheme = 0;
  double scol0, scol1;
  MaterialHandle vcol0(0);
  MaterialHandle vcol1(0);

  if (fld->basis_order() < 0 ||
      (fld->basis_order() == 0 && mesh->dimensionality() != 1) ||
      get_flag(render_state, USE_DEFAULT_COLOR))
  {
    color_scheme = 0; // Default color
  }
  else if (get_flag(render_state, USE_COLORMAP))
  {
    // Color map lookup using either a scalar value
    // the vector magnitude, the tensor.
    color_scheme = 1;
  }
  else //if (fld->basis_order() >= 0)
  {
    color_scheme = 2; // Vector values become RGB
    vcol0 = new Material(Color(1.0, 1.0, 1.0));
    vcol1 = new Material(Color(1.0, 1.0, 1.0));
    if (get_flag(render_state, USE_TRANSPARENCY))
      vcol0->transparency = vcol1->transparency = 0.75;
    else
      vcol0->transparency = vcol1->transparency = 1.0;
  }

  mesh->synchronize(Mesh::EDGES_E);
  typename FIELD::mesh_type::Edge::iterator eiter; mesh->begin(eiter);  
  typename FIELD::mesh_type::Edge::iterator eiter_end; mesh->end(eiter_end);  
  while (eiter != eiter_end) 
  {  
    typename FIELD::mesh_type::Node::array_type nodes;
    mesh->get_nodes(nodes, *eiter);
      
    Point p0, p1;
    mesh->get_point(p0, nodes[0]);
    mesh->get_point(p1, nodes[1]);

    typename FIELD::value_type val0, val1;

    if( color_scheme )
    {
      if (fld->basis_order() == 1)
      {
        fld->value(val0, nodes[0]);
        fld->value(val1, nodes[1]);
      }
      else //if (mesh->dimensionality() == 1)
      {
        fld->value(val0, *eiter);

        val1 = val0;
      }

      value_to_color( color_scheme, val0, scol0, vcol0 );
      value_to_color( color_scheme, val1, scol1, vcol1 );
    }

    add_edge_geom(lines, glyphs, p0, p1,
		  scale, resolution, cylinders_p,
		  color_scheme, scol0, scol1, vcol0, vcol1);

    ++eiter;
  }

  return data_switch;
}


template <class FIELD, class Loc>
void
RenderField<FIELD, Loc>::add_face_geom(GeomFastTriangles *faces,
				       GeomFastQuads *qfaces,
				       const vector<Point>  &points,
				       const vector<Vector> &normals,
				       bool with_normals,
				       unsigned int color_scheme,
				       vector<double> &scols,
				       vector<MaterialHandle> &vcols )
{
  if (color_scheme == 0)
  {
    if (points.size() == 4)
    {
      if (with_normals)
      {
        qfaces->add(points[0], normals[0],
          points[1], normals[1],
          points[2], normals[2],
          points[3], normals[3]);
      }
      else
      {
        qfaces->add(points[0], points[1], points[2], points[3]);
      }
    }
    else
    {
      for (unsigned i = 2; i < points.size(); i++)
      {
        if (with_normals)
        {
          faces->add(points[0],   normals[0],
               points[i-1], normals[i-1],
               points[i],   normals[i]);
        }
        else
        {
          faces->add(points[0], points[i-1], points[i]);
        }
      }
    }    
  }
  else if (color_scheme == 1)
  {
    if (points.size() == 4)
    {
      if (with_normals)
      {
        qfaces->add(points[0], normals[0], scols[0],
              points[1], normals[1], scols[1],
              points[2], normals[2], scols[2],
              points[3], normals[3], scols[3]);
      }
      else
      {
        qfaces->add(points[0], scols[0],
              points[1], scols[1],
              points[2], scols[2],
              points[3], scols[3]);
      }
    }
    else
    {
      for (unsigned i = 2; i < points.size(); i++)
      {
        if (with_normals)
        {
          faces->add(points[0],   normals[0],   scols[0],
               points[i-1], normals[i-1], scols[i-1],
               points[i],   normals[i],   scols[i]);
        }
        else
        {
          faces->add(points[0],   scols[0],
               points[i-1], scols[i-1],
               points[i],   scols[i]);
        }
      }
    }
  }
  else if (color_scheme == 2)
  {
    if (points.size() == 4)
    {
      if (with_normals)
      {
        qfaces->add(points[0], normals[0], vcols[0],
              points[1], normals[1], vcols[1],
              points[2], normals[2], vcols[2],
              points[3], normals[3], vcols[3]);
      }
      else
      {
        qfaces->add(points[0], vcols[0],
              points[1], vcols[1],
              points[2], vcols[2],
              points[3], vcols[3]);
      }
    }
    else
    {
      for (unsigned i = 2; i < points.size(); i++)
      {
        if (with_normals)
        {
          faces->add(points[0],   normals[0],   vcols[0],
               points[i-1], normals[i-1], vcols[i-1],
               points[i],   normals[i],   vcols[i]);
        }
        else
        {
          faces->add(points[0],   vcols[0],
               points[i-1], vcols[i-1],
               points[i],   vcols[i]);
        }
      }
    }
  }

}


template <class FIELD, class Loc>
GeomHandle 
RenderField<FIELD, Loc>::render_faces(FieldHandle field_handle,
				      unsigned int render_state,
				      unsigned int div)
{
  FIELD *fld = dynamic_cast<FIELD *>(field_handle.get_rep());
  typename FIELD::mesh_handle_type mesh = fld->get_typed_mesh();

  const bool with_normals = (get_flag(render_state, USE_NORMALS)
			     && mesh->has_face_normals());

  GeomFastTriangles* tfaces;
  GeomFastQuads* qfaces;
  GeomGroup *grp = new GeomGroup();
  GeomHandle face_switch = new GeomDL(grp);

  if (get_flag(render_state, USE_TRANSPARENCY))
  {
    tfaces = new GeomTranspTriangles;
    qfaces = new GeomTranspQuads;
  }
  else
  {
    tfaces = new GeomFastTriangles;
    qfaces = new GeomFastQuads;
  }

  grp->add(tfaces);
  grp->add(qfaces);

  unsigned int color_scheme = 0;
  vector<typename FIELD::value_type> vals(20);
  vector<double> scols(20);
  vector<MaterialHandle> vcols(20, (Material *) 0);

  if( fld->basis_order() < 0 ||
      get_flag(render_state, USE_DEFAULT_COLOR) )
  {
    color_scheme = 0; // Default color
  }
  else if (get_flag(render_state, USE_COLORMAP))
  {
    // Color map lookup using either a scalar value
    // the vector magnitude, the tensor.
    color_scheme = 1;
  }
  else // if( fld->basis_order() >= 0 )
  {
    color_scheme = 2; // Values become RGB
  }

  if( color_scheme >= 2 )
  {
    for (unsigned int i=0; i<20; i++)
    {
      vcols[i] = new Material(Color(1.0, 1.0, 1.0));

      if (get_flag(render_state, USE_TRANSPARENCY))
        vcols[i]->transparency = 0.75;
      else
        vcols[i]->transparency = 1.0;
    }
  }

#if defined(_MSC_VER) || defined(__ECC)
  typedef hash_set<string> face_ht_t;
#else
  typedef hash_set<string, str_hasher, equal_str> face_ht_t;
#endif
  face_ht_t rendered_faces; 
  
  mesh->synchronize(Mesh::FACES_E | Mesh::EDGES_E | Mesh::CELLS_E);
  typename FIELD::mesh_type::Elem::iterator eiter; mesh->begin(eiter);  
  typename FIELD::mesh_type::Elem::iterator eiter_end; mesh->end(eiter_end);  
  while (eiter != eiter_end) 
  {  
    typename FIELD::mesh_type::Face::array_type face_indecies;
    mesh->get_faces(face_indecies, *eiter);

    typename FIELD::mesh_type::Face::array_type::iterator face_iter;
    face_iter = face_indecies.begin();
    int fcount = 0;
    while (face_iter != face_indecies.end()) 
    {
      typename FIELD::mesh_type::Node::array_type nodes;
      typename FIELD::mesh_type::Face::index_type fidx = *face_iter++;

      Point cntr;
      mesh->get_center(cntr, fidx);
      ostringstream pstr;
      pstr << setiosflags(ios::scientific);
      pstr << setprecision(7); 
      pstr << cntr.x() << cntr.y() << cntr.z();
      
      typename face_ht_t::const_iterator it = rendered_faces.find(pstr.str());

      if (it != rendered_faces.end()) 
      {
        ++fcount;
        continue;
      } 
      else 
      {
        rendered_faces.insert(pstr.str());
      }

      //coords organized as scanlines of quad/tri strips.
      vector<vector<vector<double> > > coords;
      mesh->pwl_approx_face(coords, *eiter, fcount, div);

      const int face_sz = mesh->get_basis().vertices_of_face();
      vector<vector<vector<double> > >::iterator coord_iter = coords.begin();
      while (coord_iter != coords.end()) 
      {
        vector<vector<double> > &sl = *coord_iter++;
        vector<vector<double> >::iterator sliter = sl.begin();

        for (unsigned int i = 0; i < sl.size() - 2; i++) 
        {
          //while(sliter != sl.end()) {
          if (face_sz == 3) 
          {  // TRI STRIPS
            vector<vector<double> >::iterator it0,it1;
            
            vector<double> &c0 = !i%2 ? sl[i] : sl[i+1];
            vector<double> &c1 = !i%2 ? sl[i+1] : sl[i];
            vector<double> &c2 = sl[i+2];

#if defined(DEBUG_PRINT)
            print_coords("c0: ", c0);
            print_coords("c1: ", c1);
            print_coords("c2: ", c2);
#endif
            vector<Point> pnts(face_sz);
            vector<Vector> norms(face_sz);
            vector<typename FIELD::value_type> vals(face_sz);
          
            mesh->interpolate(pnts[0], c0, *eiter);
            mesh->interpolate(pnts[1], c1, *eiter);
            mesh->interpolate(pnts[2], c2, *eiter);

            if (with_normals) 
            {	      
              mesh->get_normal(norms[0], c0, *eiter, fcount);
              mesh->get_normal(norms[1], c1, *eiter, fcount);
              mesh->get_normal(norms[2], c2, *eiter, fcount);
            }

#if defined(DEBUG_PRINT)
            cout << "p0: " << pnts[0] << std::endl;
            cout << "p1: " << pnts[1] << std::endl;
            cout << "p2: " << pnts[2] << std::endl;
#endif	  
            // get the field variables values at the approx (if they exist)
            if (color_scheme) 
            {
              fld->interpolate(vals[0], c0, *eiter);
              fld->interpolate(vals[1], c1, *eiter);
              fld->interpolate(vals[2], c2, *eiter);

              value_to_color( color_scheme, vals[0], scols[0], vcols[0] );
              value_to_color( color_scheme, vals[1], scols[1], vcols[1] );
              value_to_color( color_scheme, vals[2], scols[2], vcols[2] );
	    }

	    add_face_geom(tfaces, qfaces, pnts, norms, with_normals,
			  color_scheme, scols, vcols);
	  }
	  else 
          { // QUADS

	    vector<double> &c0 = *sliter++;
	    if (sliter == sl.end()) break;
	    vector<double> &c1 = *sliter++;
	    if (sliter == sl.end()) break;
	    vector<double> &c2 = *sliter;
	    vector<double> &c3 = *(sliter + 1);
	    
	    vector<Point> pnts(face_sz);
	    vector<Vector> norms(face_sz);
	    vector<typename FIELD::value_type> vals(face_sz);
            
	    // get the geometry at the approx.
	    mesh->interpolate(pnts[0], c2, *eiter);
	    mesh->interpolate(pnts[1], c3, *eiter);
	    mesh->interpolate(pnts[2], c1, *eiter);
	    mesh->interpolate(pnts[3], c0, *eiter);

	    if (with_normals) 
            {	      
	      mesh->get_normal(norms[0], c2, *eiter, fcount);
	      mesh->get_normal(norms[1], c3, *eiter, fcount);
	      mesh->get_normal(norms[2], c1, *eiter, fcount);
	      mesh->get_normal(norms[3], c0, *eiter, fcount);
	    }

#if defined(DEBUG_PRINT)
	    print_coords("c0: ", c0);
	    print_coords("c1: ", c1);
	    print_coords("c2: ", c2);
	    print_coords("c3: ", c3);
#endif
	      
	    // get the field variables values at the approx (if they exist)
	    if (color_scheme) 
            {
	      fld->interpolate(vals[0], c2, *eiter);
	      fld->interpolate(vals[1], c3, *eiter);
	      fld->interpolate(vals[2], c1, *eiter);
	      fld->interpolate(vals[3], c0, *eiter);
	      
	      value_to_color( color_scheme, vals[0], scols[0], vcols[0] );
	      value_to_color( color_scheme, vals[1], scols[1], vcols[1] );
	      value_to_color( color_scheme, vals[2], scols[2], vcols[2] );
	      value_to_color( color_scheme, vals[3], scols[3], vcols[3] );
	    }
	    
	    add_face_geom(tfaces, qfaces, pnts, norms, with_normals,
			  color_scheme, scols, vcols);
          }
        }
      }
      ++fcount;
    }
    ++eiter;
  }

  return face_switch;
}

template <class FIELD, class Loc>
GeomHandle 
RenderField<FIELD, Loc>::render_faces_linear(FieldHandle field_handle,
					     unsigned int render_state)
{
  FIELD *fld = dynamic_cast<FIELD *>(field_handle.get_rep());
  typename FIELD::mesh_handle_type mesh = fld->get_typed_mesh();

  const bool with_normals = (get_flag(render_state, USE_NORMALS)
			     && mesh->has_normals());

  GeomFastTriangles* tfaces = 0;
  GeomFastQuads* qfaces = 0;
  GeomFastTrianglesTwoSided* ttfaces = 0;
  GeomFastQuadsTwoSided* tqfaces =0 ;

  GeomGroup *grp = new GeomGroup();
  GeomHandle face_switch = new GeomDL(grp);

  // Special case for cell centered data
  if ((fld->basis_order() == 0) && (mesh->dimensionality() == 3))
  {
    if (get_flag(render_state, USE_TRANSPARENCY))
    {
      ttfaces = new GeomTranspTrianglesTwoSided;
      tqfaces = new GeomTranspQuadsTwoSided;
      grp->add(ttfaces);
      grp->add(tqfaces);
    }
    else
    {
      ttfaces = new GeomFastTrianglesTwoSided;
      tqfaces = new GeomFastQuadsTwoSided;
      grp->add(ttfaces);
      grp->add(tqfaces);
    }
  }
  else
  {
    if (get_flag(render_state, USE_TRANSPARENCY))
    {
      tfaces = new GeomTranspTriangles;
      qfaces = new GeomTranspQuads;
      
      grp->add(tfaces);
      grp->add(qfaces);
    }
    else
    {
      tfaces = new GeomFastTriangles;
      qfaces = new GeomFastQuads;
    
      grp->add(tfaces);
      grp->add(qfaces);
    }
  }

  unsigned int color_scheme = 0;
  vector<typename FIELD::value_type> vals(20);
  vector<MaterialHandle> vcols(20, (Material *)NULL);
  vector<double> scols(20);

  if( fld->basis_order() < 0 ||
      get_flag(render_state, USE_DEFAULT_COLOR) )
  {
    color_scheme = 0; // Default color
  }
  else if (get_flag(render_state, USE_COLORMAP))
  {
    // Color map lookup using either a scalar value,
    // the vector magnitude, the tensor.
    color_scheme = 1;
  }
  else //if ( fld->basis_order() >= 0 )
  {
    color_scheme = 2; // Values become RGB

    for (unsigned int i=0; i<20; i++)
    {
      vcols[i] = new Material(Color(1.0, 1.0, 1.0));

      if (get_flag(render_state, USE_TRANSPARENCY))
        vcols[i]->transparency = 0.75;
      else
        vcols[i]->transparency = 1.0;
    }
  }

  if (with_normals) mesh->synchronize(Mesh::NORMALS_E);

  mesh->synchronize(Mesh::FACES_E);
  typename FIELD::mesh_type::Face::iterator fiter; mesh->begin(fiter);  
  typename FIELD::mesh_type::Face::iterator fiter_end; mesh->end(fiter_end);  
  typename FIELD::mesh_type::Node::array_type nodes;

  while (fiter != fiter_end) 
  {
    mesh->get_nodes(nodes, *fiter); 
 
    vector<Point> points(nodes.size());
    vector<Vector> normals(nodes.size());

    for (unsigned int i=0; i<nodes.size(); i++)
      mesh->get_point(points[i], nodes[i]);

    if (with_normals) 
    {
      for (unsigned int i=0; i<nodes.size(); i++)
        mesh->get_normal(normals[i], nodes[i]);
    }

    // Element data (Cells) so two sided faces.
    if (fld->basis_order() == 0 && mesh->dimensionality() == 3)
    {
      typename FIELD::mesh_type::Elem::array_type cells;
      mesh->get_elems(cells,*fiter);
      
      fld->value(vals[0], cells[0]);

      if (cells.size() > 1)
        fld->value(vals[1], cells[1]);
      else
        vals[1] = vals[0];

      if (color_scheme == 0)
      {
        if (nodes.size() == 4)
        {
          if (with_normals)
            tqfaces->add(points[0], normals[0],
             points[1], normals[1],
             points[2], normals[2],
             points[3], normals[3]);
          else
            tqfaces->add(points[0], points[1], points[2], points[3]);
        }
        else
        {
          for (unsigned int i=2; i<nodes.size(); i++)
          {
            if (with_normals)
              ttfaces->add(points[0],   normals[0],
               points[i-1], normals[i-1],
               points[i],   normals[i]);
            else
              ttfaces->add(points[0], points[i-1], points[i]);
          }
        }
      }
      else if (color_scheme == 1)
      {
        to_double(vals[0], scols[0]);
        to_double(vals[1], scols[1]);

        if (nodes.size() == 4)
        {
          if (with_normals)
            tqfaces->add(points[0], normals[0], scols[0], scols[1],
                 points[1], normals[1], scols[0], scols[1],
                 points[2], normals[2], scols[0], scols[1],
                 points[3], normals[3], scols[0], scols[1]);
          else
            tqfaces->add(points[0], scols[0], scols[1],
                 points[1], scols[0], scols[1],
                 points[2], scols[0], scols[1],
                 points[3], scols[0], scols[1]);
        }
        else
        {
          for (unsigned int i=2; i<nodes.size(); i++)
          {
            if (with_normals)
              ttfaces->add(points[0],   normals[0],   scols[0], scols[1],
                   points[i-1], normals[i-1], scols[0], scols[1],
                   points[i],   normals[i],   scols[0], scols[1]);
            else
              ttfaces->add(points[0],   scols[0], scols[1],
                   points[i-1], scols[0], scols[1],
                   points[i],   scols[0], scols[1]);
          }
        }
      }
      else //if (color_scheme >= 2)
      {
        to_color(vals[0], vcols[0]->diffuse);
        to_color(vals[1], vcols[1]->diffuse);

        if (nodes.size() == 4)
        {
          if (with_normals)
            tqfaces->add(points[0], normals[0], vcols[0], vcols[1],
                 points[1], normals[1], vcols[0], vcols[1],
                 points[2], normals[2], vcols[0], vcols[1],
                 points[3], normals[3], vcols[0], vcols[1]);
          else
            tqfaces->add(points[0], vcols[0], vcols[1],
                 points[1], vcols[0], vcols[1],
                 points[2], vcols[0], vcols[1],
                 points[3], vcols[0], vcols[1]);
        }
        else
        {
          for (unsigned int i=2; i<nodes.size(); i++)
          {
            if (with_normals)
              ttfaces->add(points[0],   normals[0],   vcols[0], vcols[1],
                   points[i-1], normals[i-1], vcols[0], vcols[1],
                   points[i],   normals[i],   vcols[0], vcols[1]);
            else
              ttfaces->add(points[0],   vcols[0], vcols[1],
                   points[i-1], vcols[0], vcols[1],
                   points[i],   vcols[0], vcols[1]);
          }
        }
      }
    }
    else if (color_scheme == 0)
    {
      if (nodes.size() == 4)
      {
        if (with_normals)
          qfaces->add(points[0], normals[0],
                points[1], normals[1],
                points[2], normals[2],
                points[3], normals[3]);
        else
          qfaces->add(points[0], points[1], points[2], points[3]);
      }
      else
      {
        for (unsigned int i=2; i<nodes.size(); i++)
        {
          if (with_normals)
            tfaces->add(points[0],   normals[0],
                points[i-1], normals[i-1],
                points[i],   normals[i]);
          else
            tfaces->add(points[0], points[i-1], points[i]);
        }
      }
    }

    // Data at nodes
    else if (fld->basis_order() == 1)
    {
      for (unsigned int i=0; i<nodes.size(); i++)
      {
        fld->value(vals[i], nodes[i]);
        value_to_color( color_scheme, vals[i], scols[i], vcols[i] );
      }

      if (color_scheme == 1)
      {
        if (nodes.size() == 4)
        {
          if (with_normals)
            qfaces->add(points[0], normals[0], scols[0],
                        points[1], normals[1], scols[1],
                        points[2], normals[2], scols[2],
                        points[3], normals[3], scols[3]);
          else
            qfaces->add(points[0], scols[0],
                        points[1], scols[1],
                        points[2], scols[2],
                        points[3], scols[3]);
        }
        else
        {
          for (unsigned int i=2; i<nodes.size(); i++)
          {
            if (with_normals)
              tfaces->add(points[0],   normals[0],   scols[0],
                  points[i-1], normals[i-1], scols[i-1],
                  points[i],   normals[i],   scols[i]);
            else
              tfaces->add(points[0],   scols[0],
                  points[i-1], scols[i-1],
                  points[i],   scols[i]);
          }
        }
      }
      else //if (color_scheme >= 2)
      {
        if (nodes.size() == 4)
        {
          if (with_normals)
            qfaces->add(points[0], normals[0], vcols[0],
                        points[1], normals[1], vcols[1],
                        points[2], normals[2], vcols[2],
                        points[3], normals[3], vcols[3]);
          else
            qfaces->add(points[0], vcols[0],
                        points[1], vcols[1],
                        points[2], vcols[2],
                        points[3], vcols[3]);
        }
        else
        {
          for (unsigned int i=2; i<nodes.size(); i++)
          {
            if (with_normals)
              tfaces->add(points[0],   normals[0],   vcols[0],
                  points[i-1], normals[i-1], vcols[i-1],
                  points[i],   normals[i],   vcols[i]);
            else
              tfaces->add(points[0],   vcols[0],
                  points[i-1], vcols[i-1],
                  points[i],   vcols[i]);
          }
        }
      }
    }

    // Element data (faces)
    else if (fld->basis_order() == 0 && mesh->dimensionality() == 2)
    {
      typename FIELD::value_type val;
      fld->value(val, *fiter);

      value_to_color( color_scheme, val, scols[0], vcols[0] );

      if (color_scheme == 1)
      {
        if (nodes.size() == 4)
        {
          if (with_normals)
            qfaces->add(points[0], normals[0], scols[0],
                        points[1], normals[1], scols[0],
                        points[2], normals[2], scols[0],
                        points[3], normals[3], scols[0]);
          else
            qfaces->add(points[0], scols[0],
                        points[1], scols[0],
                        points[2], scols[0],
                        points[3], scols[0]);
        }
        else
        {
          for (unsigned int i=2; i<nodes.size(); i++)
          {
            if (with_normals)
              tfaces->add(points[0],   normals[0],   scols[0],
                  points[i-1], normals[i-1], scols[0],
                  points[i],   normals[i],   scols[0]);
            else
              tfaces->add(points[0],   scols[0],
                  points[i-1], scols[0],
                  points[i],   scols[0]);
          }
        }
      }
      else //if (color_scheme >= 2)
      {
        if (nodes.size() == 4)
        {
          if (with_normals)
            qfaces->add(points[0], normals[0], vcols[0],
                        points[1], normals[1], vcols[0],
                        points[2], normals[2], vcols[0],
                        points[3], normals[3], vcols[0]);
          else
            qfaces->add(points[0], vcols[0],
                        points[1], vcols[0],
                        points[2], vcols[0],
                        points[3], vcols[0]);
        }
        else
        {
          for (unsigned int i=2; i<nodes.size(); i++)
          {
            if (with_normals)
              tfaces->add(points[0],   normals[0],   vcols[0],
                  points[i-1], normals[i-1], vcols[0],
                  points[i],   normals[i],   vcols[0]);
            else
              tfaces->add(points[0],   vcols[0],
                  points[i-1], vcols[0],
                  points[i],   vcols[0]);
          }
        }
      }
    }


    ++fiter;     
  }

  return face_switch;
}


template <class FIELD, class Loc>
class RenderFieldImage : public RenderField<FIELD, Loc>
{
protected:
  virtual GeomHandle render_faces_texture(FieldHandle field_handle, 
					  ColorMapHandle colormap_handle,
					  unsigned int render_state);
};


class SCISHARE RenderFieldImageV : public RenderFieldV
{
protected:
  virtual GeomHandle render_faces_texture(FieldHandle field_handle, 
					  ColorMapHandle colormap_handle,
					  unsigned int render_state);
};



template <class FIELD, class Loc>
GeomHandle
RenderFieldImage<FIELD, Loc>::render_faces_texture(FieldHandle field_handle,
						   ColorMapHandle colormap_handle,
						   unsigned int render_state)
{
  FIELD *fld = dynamic_cast<FIELD *>(field_handle.get_rep());
  typename FIELD::mesh_handle_type mesh = fld->get_typed_mesh();
  GeomHandle texture_face;
  float tex_coords[8];
  float pos_coords[12];
  const int colorbytes = 4;

  GeomTexRectangle *tr = new GeomTexRectangle();
  texture_face = tr;

  IMesh *im = dynamic_cast<IMesh *> (mesh.get_rep());

  // Set up the texture parameters, power of 2 dimensions.
  int width = Pow2(im->get_ni());
  int height = Pow2(im->get_nj());

  // Use for the texture coordinates 
  double tmin_x, tmax_x, tmin_y, tmax_y;

  // Create texture array 
  unsigned char * texture = new unsigned char[colorbytes*width*height];

  //***************************************************
  // we need to find the corners of the square in space
  // use the node indices to grab the corner points
  typename FIELD::mesh_type::Node::index_type ll(im, 0, 0);
  typename FIELD::mesh_type::Node::index_type lr(im, im->get_ni() - 1, 0);
  typename FIELD::mesh_type::Node::index_type ul(im, 0, im->get_nj() - 1);
  typename FIELD::mesh_type::Node::index_type ur(im, im->get_ni() - 1, 
					       im->get_nj() - 1);
  
  Point p1, p2, p3, p4;
  im->get_center(p1, ll);
  pos_coords[0] = p1.x();
  pos_coords[1] = p1.y();
  pos_coords[2] = p1.z();

  im->get_center(p2, lr);
  pos_coords[3] = p2.x();
  pos_coords[4] = p2.y();
  pos_coords[5] = p2.z();

  im->get_center(p3, ur);
  pos_coords[6] = p3.x();
  pos_coords[7] = p3.y();
  pos_coords[8] = p3.z();

  im->get_center(p4, ul);
  pos_coords[9] = p4.x();
  pos_coords[10] = p4.y();
  pos_coords[11] = p4.z();

  vector<typename FIELD::value_type> vals(20);
  vector<double> scols(20);


  if ( fld->basis_order() == 1)
  {
    tr->interpolate(true);

    tmin_x = 0.5/(double)width;
    tmax_x = (im->get_ni()- 0.5)/(double)width;
    tmin_y = 0.5/(double)height;
    tmax_y = (im->get_nj()-0.5)/(double)height;

    tex_coords[0] = tmin_x; tex_coords[1] = tmin_y;
    tex_coords[2] = tmax_x; tex_coords[3] = tmin_y;
    tex_coords[4] = tmax_x; tex_coords[5] = tmax_y;
    tex_coords[6] = tmin_x; tex_coords[7] = tmax_y;

    typename FIELD::mesh_type::Node::iterator niter; mesh->begin(niter);  
    typename FIELD::mesh_type::Node::iterator niter_end; mesh->end(niter_end);  
    typename FIELD::mesh_type::Node::array_type nodes;
    while(niter != niter_end )
    {
      // Convert data values to double.
      typename FIELD::value_type val;
      double dval;      
      fld->value(val, *niter);
      to_double(val, dval);

      // Compute index into texture array.
      const int idx = (niter.i_ * colorbytes) + (niter.j_ * width *colorbytes);
      
      // Compute the ColorMap index and retreive the color.
      const double cmin = colormap_handle->getMin();
      const double cmax = colormap_handle->getMax();
      const double index = Clamp((dval - cmin)/(cmax - cmin), 0.0, 1.0);
      float r,g,b,a;
      colormap_handle->get_color(index, r, g, b, a);
      const float zro = 0.0;
      const float one = 1.0;
      
      // Fill the texture.
      texture[idx  ] = (unsigned char)(Clamp(r, zro, one) * 255.);
      texture[idx+1] = (unsigned char)(Clamp(g, zro, one) * 255.);
      texture[idx+2] = (unsigned char)(Clamp(b, zro, one) * 255.);
      texture[idx+3] = (unsigned char)(Clamp(a, zro, one) * 255.);
      ++niter;
    }
  }
  else if( fld->basis_order() == 0)
  {
     tr->interpolate( false );
     tmin_x = 0.0;
     tmax_x = (im->get_ni()-1)/(double)width;
     tmin_y = 0.0;
     tmax_y = (im->get_nj()-1)/(double)height;
     tex_coords[0] = tmin_x; tex_coords[1] = tmin_y;
     tex_coords[2] = tmax_x; tex_coords[3] = tmin_y;
     tex_coords[4] = tmax_x; tex_coords[5] = tmax_y;
     tex_coords[6] = tmin_x; tex_coords[7] = tmax_y;
      
     typename FIELD::mesh_type::Face::iterator fiter; mesh->begin(fiter);  
     typename FIELD::mesh_type::Face::iterator fiter_end; mesh->end(fiter_end);  

     while (fiter != fiter_end)
     {
       typename FIELD::value_type val;
       double dval;
       fld->value(val, *fiter);
       to_double(val, dval);

       // Compute index into texture array.
       const int idx = (fiter.i_ * colorbytes) + (fiter.j_ * width *colorbytes);
       // Compute the ColorMap index and retreive the color.
       const double cmin = colormap_handle->getMin();
       const double cmax = colormap_handle->getMax();
       const double index = Clamp((dval - cmin)/(cmax - cmin), 0.0, 1.0);
       float r,g,b,a;
       colormap_handle->get_color(index, r, g, b, a);
       const float zro = 0.0;
       const float one = 1.0;

       // Fill the texture.
       texture[idx  ] = (unsigned char)(Clamp(r, zro, one) * 255);
       texture[idx+1] = (unsigned char)(Clamp(g, zro, one) * 255);
       texture[idx+2] = (unsigned char)(Clamp(b, zro, one) * 255);
       texture[idx+3] = (unsigned char)(Clamp(a, zro, one) * 255);
       ++fiter;
     }
   }

  // Set normal for lighting.
  Vector normal = Cross( p2 - p1, p4 - p1 );
  normal.normalize();
  float n[3];
  n[0] = normal.x(); n[1] = normal.y(); n[2] = normal.z();
  tr->set_normal( n );

  tr->set_transparency( get_flag(render_state,
				 RenderStateBase::USE_TRANSPARENCY) );
  tr->set_coords(tex_coords, pos_coords);
  tr->set_texture( texture, colorbytes, width, height );

  delete [] texture;
  return texture_face;
}



template <class FIELD, class Loc>
GeomHandle 
RenderField<FIELD, Loc>::render_text(FieldHandle field_handle,
				     ColorMapHandle colormap_handle,
				     bool use_colormap,
				     bool use_default_color,
				     bool backface_cull_p,
				     int fontsize,
				     int precision,
				     bool render_locations,
				     bool render_data,
				     bool render_nodes,
				     bool render_edges,
				     bool render_faces,
				     bool render_cells,
				     bool always_visible)
{
  GeomGroup *texts = new GeomGroup;
  GeomHandle text_switch = new GeomSwitch(texts);

  if (render_data)
  {
    texts->add(render_text_data(field_handle, colormap_handle, 
				use_colormap,
				use_default_color,
				backface_cull_p,
				fontsize, precision, always_visible));
  }
  if (render_nodes)
  {
    texts->add(render_text_nodes(field_handle, colormap_handle, 
				 use_colormap,
				 use_default_color,
				 backface_cull_p,
				 fontsize, precision, 
				 render_locations, always_visible));
  }
  if (render_edges)
  {
    texts->add(render_text_edges(field_handle, colormap_handle, 
				 use_colormap,
				 use_default_color,
				 fontsize, precision, 
				 render_locations, always_visible));
  }
  if (render_faces)
  {
    texts->add(render_text_faces(field_handle, colormap_handle, 
				 use_colormap,
				 use_default_color,
				 fontsize, precision, 
				 render_locations, always_visible));
  }
  if (render_cells)
  {
    texts->add(render_text_cells(field_handle, colormap_handle, 
				 use_colormap,
				 use_default_color,
				 fontsize, precision, 
				 render_locations, always_visible));
  }
  return text_switch;
}


template <class FIELD, class Loc>
GeomHandle 
RenderField<FIELD, Loc>::render_text_data(FieldHandle field_handle,
					  ColorMapHandle colormap_handle, 
					  bool use_color_map,
					  bool use_default_color,
					  bool backface_cull_p,
					  int fontsize,
					  int precision,
					  bool always_visible)
{
  if (backface_cull_p && field_handle->basis_order() == 1)
  {
    return render_text_data_nodes(field_handle, colormap_handle, use_color_map,
				  use_default_color,
				  backface_cull_p, fontsize,
				  precision, always_visible);
  }

  FIELD *fld = dynamic_cast<FIELD *>(field_handle.get_rep());
  ASSERT(fld);

  typename FIELD::mesh_handle_type mesh = fld->get_typed_mesh();

  GeomTexts *texts = new GeomTexts();
  if (always_visible) texts->set_always_visible();
  GeomHandle text_switch = new GeomSwitch(new GeomDL(texts));
  texts->set_font_index(fontsize);

  std::ostringstream buffer;
  buffer.precision(precision);

  unsigned int color_scheme = 0;
  double scol;
  Color vcol;

  if( use_default_color )
    color_scheme = 0;
  else if( use_color_map )
    color_scheme = 1;
  else {
    color_scheme = 2;
  }

  typename Loc::iterator iter, end;
  mesh->begin(iter);
  mesh->end(end);
  Point p;
  while (iter != end)
  {
    typename FIELD::value_type val;

    if (fld->value(val, *iter))
    {
      mesh->get_center(p, *iter);

      buffer.str("");
      to_buffer(val, buffer);

      if (color_scheme == 0 )
      {
        texts->add(buffer.str(), p);
      }
      else if (color_scheme == 2 )
      {
        to_color(val, vcol);
        texts->add(buffer.str(), p, vcol);
      }
      else
      {
        to_double(val, scol);
        // Compute the ColorMap index and retreive the color.
        const double cmin = colormap_handle->getMin();
        const double cmax = colormap_handle->getMax();
        const double index = Clamp((scol - cmin)/(cmax - cmin), 0.0, 1.0);
        float r,g,b,a;
        colormap_handle->get_color(index, r, g, b, a);
        const Color c(r, g, b);

        texts->add(buffer.str(), p, c);
      }
    }
    ++iter;
  }

  return text_switch;
}


template <class FIELD, class Loc>
GeomHandle 
RenderField<FIELD, Loc>::render_text_data_nodes(FieldHandle field_handle,
						ColorMapHandle colormap_handle,
						bool use_color_map,
						bool use_default_color,
						bool backface_cull_p,
						int fontsize,
						int precision,
						bool always_visible)
{
  FIELD *fld = dynamic_cast<FIELD *>(field_handle.get_rep());
  ASSERT(fld);

  typename FIELD::mesh_handle_type mesh = fld->get_typed_mesh();

  GeomTexts *texts = 0;
  GeomTextsCulled *ctexts = 0;
  GeomHandle text_switch = 0;
  const bool culling_p = backface_cull_p && mesh->has_normals();
  if (culling_p)
  {
    mesh->synchronize(Mesh::NORMALS_E);
    ctexts = new GeomTextsCulled();
    text_switch = new GeomSwitch(ctexts);
    ctexts->set_font_index(fontsize);
  }
  else
  {
    texts = new GeomTexts();
    if (always_visible) texts->set_always_visible();
    text_switch = new GeomSwitch(new GeomDL(texts));
    texts->set_font_index(fontsize);
  }

  std::ostringstream buffer;
  buffer.precision(precision);

  unsigned int color_scheme = 0;
  double scol;
  Color vcol;

  if( fld->basis_order() < 0 ||
      (fld->basis_order() == 0 && mesh->dimensionality() != 0) ||
      use_default_color )
    color_scheme = 0;
  else if( use_color_map )
    color_scheme = 1;
  else
    color_scheme = 2;

  typename FIELD::mesh_type::Node::iterator iter, end;
  mesh->begin(iter);
  mesh->end(end);
  Point p;
  Vector n;
  while (iter != end) 
  {
    typename FIELD::value_type val;
    if (fld->value(val, *iter)) 
    {
      mesh->get_center(p, *iter);
      
      buffer.str("");
      to_buffer(val, buffer);

      if (color_scheme == 0)
      {
        if (culling_p)
        {
          mesh->get_normal(n, *iter);
          ctexts->add(buffer.str(), p, n);
        }
        else
        {
          texts->add(buffer.str(), p);
        }
      }
      else if (color_scheme == 2)
      {
        to_color( val, vcol );
        if (culling_p)
        {
          mesh->get_normal(n, *iter);
          ctexts->add(buffer.str(), p, n, vcol);
        }
        else
        {
          texts->add(buffer.str(), p, vcol);
        }
      }
      else
      {
        to_double(val, scol);	
        // Compute the ColorMap index and retreive the color.
        const double cmin = colormap_handle->getMin();
        const double cmax = colormap_handle->getMax();
        const double index = Clamp((scol - cmin)/(cmax - cmin), 0.0, 1.0);
        float r,g,b,a;
        colormap_handle->get_color(index, r, g, b, a);
        const Color c(r, g, b);

        if (culling_p)
        {
          mesh->get_normal(n, *iter);
          ctexts->add(buffer.str(), p, n, c);
        }
        else
        {
          texts->add(buffer.str(), p, c);
        }
      }
    }
    ++iter;
  }

  return text_switch;
}


template <class FIELD, class Loc>
GeomHandle 
RenderField<FIELD, Loc>::render_text_nodes(FieldHandle field_handle,
					   ColorMapHandle colormap_handle,
					   bool use_color_map,
					   bool use_default_color,
					   bool backface_cull_p,
					   int fontsize,
					   int precision,
					   bool render_locations,
					   bool always_visible)
{
  FIELD *fld = dynamic_cast<FIELD *>(field_handle.get_rep());
  ASSERT(fld);

  typename FIELD::mesh_handle_type mesh = fld->get_typed_mesh();
  GeomTexts *texts = 0;
  GeomTextsCulled *ctexts = 0;
  GeomHandle text_switch = 0;

  const bool culling_p = backface_cull_p && mesh->has_normals();
  if (culling_p)
  {
    mesh->synchronize(Mesh::NORMALS_E);
    ctexts = new GeomTextsCulled();
    text_switch = new GeomSwitch(ctexts);
    ctexts->set_font_index(fontsize);
  }
  else
  {
    texts = new GeomTexts();
    if (always_visible) texts->set_always_visible();
    text_switch = new GeomSwitch(new GeomDL(texts));
    texts->set_font_index(fontsize);
  }

  unsigned int color_scheme = 0;
  double scol;
  Color vcol;

  if( (fld->basis_order() <  0) ||
      (fld->basis_order() == 0 && mesh->dimensionality() != 0) ||
      use_default_color )
    color_scheme = 0;
  else if( use_color_map )
    color_scheme = 1;
  else
    color_scheme = 2;

  ostringstream buffer;
  buffer.precision(precision);
  typename FIELD::mesh_type::Node::iterator iter, end;
  mesh->begin(iter);
  mesh->end(end);
  Point p;
  Vector n;
  while (iter != end)
  {
    mesh->get_center(p, *iter);

    buffer.str("");
    if (render_locations)
    {
      buffer << p;
    }
    else
    {
      (*iter).str_render(buffer);
    }

    if (color_scheme == 0)
    {
      if (culling_p)
      {
        mesh->get_normal(n, *iter);
        ctexts->add(buffer.str(), p, n);
      }
      else
      {
        texts->add(buffer.str(), p);
      }
    }
    else if (color_scheme == 2)
    {
      typename FIELD::value_type val;
      fld->value(val, *iter);
      to_color( val, vcol );
      if (culling_p)
      {
        mesh->get_normal(n, *iter);
        ctexts->add(buffer.str(), p, n, vcol);
      }
      else
      {
        texts->add(buffer.str(), p, vcol);
      }
    }
    else
    {
      typename FIELD::value_type val;
      fld->value(val, *iter);
      to_double(val, scol);
      // Compute the ColorMap index and retreive the color.
      const double cmin = colormap_handle->getMin();
      const double cmax = colormap_handle->getMax();
      const double index = Clamp((scol - cmin)/(cmax - cmin), 0.0, 1.0);
      float r,g,b,a;
      colormap_handle->get_color(index, r, g, b, a);
      const Color c(r, g, b);

      if (culling_p)
      {
        mesh->get_normal(n, *iter);
        ctexts->add(buffer.str(), p, n, c);
      }
      else
      {
        texts->add(buffer.str(), p, c);
      }
    }

    ++iter;
  }
  return text_switch;
}


template <class FIELD, class Loc>
GeomHandle 
RenderField<FIELD, Loc>::render_text_edges(FieldHandle field_handle,
					   ColorMapHandle colormap_handle,
					   bool use_color_map,
					   bool use_default_color,
					   int fontsize,
					   int precision,
					   bool render_locations,
					   bool always_visible)
{
  FIELD *fld = dynamic_cast<FIELD *>(field_handle.get_rep());
  ASSERT(fld);

  typename FIELD::mesh_handle_type mesh = fld->get_typed_mesh();
  mesh->synchronize(Mesh::EDGES_E);

  GeomTexts *texts = new GeomTexts;
  if (always_visible) texts->set_always_visible();
  GeomHandle text_switch = new GeomSwitch(new GeomDL(texts));
  texts->set_font_index(fontsize);

  ostringstream buffer;
  buffer.precision(precision);

  unsigned int color_scheme = 0;
  double scol;
  Color vcol;

  if( fld->basis_order() != 0 || mesh->dimensionality() != 1 ||
      use_default_color )
    color_scheme = 0;
  else if( use_color_map )
    color_scheme = 1;
  else
    color_scheme = 2;

  typename FIELD::mesh_type::Edge::iterator iter, end;
  mesh->begin(iter);
  mesh->end(end);
  Point p;
  while (iter != end)
  {
    mesh->get_center(p, *iter);

    buffer.str("");
    if (render_locations)
    {
      buffer << p;
    }
    else
    {
      buffer << (int)(*iter);
    }

    if (color_scheme == 0)
    {
      texts->add(buffer.str(), p);
    }
    else if (color_scheme == 2)
    {
      typename FIELD::value_type val;
      fld->value(val, *iter);
      to_color(val, vcol);
      texts->add(buffer.str(), p, vcol);
    }
    else
    {
      typename FIELD::value_type val;
      fld->value(val, *iter);
      to_double(val, scol);
      // Compute the ColorMap index and retreive the color.
      const double cmin = colormap_handle->getMin();
      const double cmax = colormap_handle->getMax();
      const double index = Clamp((scol - scol)/(cmax - cmin), 0.0, 1.0);
      float r,g,b,a;
      colormap_handle->get_color(index, r, g, b, a);
      const Color c(r, g, b);

      texts->add(buffer.str(), p, c);
    }
 
    ++iter;
  }
  return text_switch;
}


template <class FIELD, class Loc>
GeomHandle 
RenderField<FIELD, Loc>::render_text_faces(FieldHandle field_handle,
					   ColorMapHandle colormap_handle,
					   bool use_color_map,
					   bool use_default_color,
					   int fontsize,
					   int precision,
					   bool render_locations,
					   bool always_visible)
{
  FIELD *fld = dynamic_cast<FIELD *>(field_handle.get_rep());
  ASSERT(fld);

  typename FIELD::mesh_handle_type mesh = fld->get_typed_mesh();
  mesh->synchronize(Mesh::FACES_E);

  GeomTexts *texts = new GeomTexts;
  if (always_visible) texts->set_always_visible();
  GeomHandle text_switch = new GeomSwitch(new GeomDL(texts));
  texts->set_font_index(fontsize);

  ostringstream buffer;
  buffer.precision(precision);

  unsigned int color_scheme = 0;
  double scol;
  Color vcol;

  if( fld->basis_order() != 0 || mesh->dimensionality() != 2 ||
      use_default_color )
    color_scheme = 0;
  else if( use_color_map )
    color_scheme = 1;
  else
    color_scheme = 2;

  typename FIELD::mesh_type::Face::iterator iter, end;
  mesh->begin(iter);
  mesh->end(end);
  Point p;
  while (iter != end)
  {
    mesh->get_center(p, *iter);

    buffer.str("");
    if (render_locations)
    {
      buffer << p;
    }
    else
    {
      (*iter).str_render(buffer);
    }

    if (color_scheme == 0)
    {
      texts->add(buffer.str(), p);
    }
    else if (color_scheme == 2)
    {
      typename FIELD::value_type val;
      fld->value(val, *iter);
      to_color(val, vcol);
      texts->add(buffer.str(), p, vcol);
    }
    else
    {
      typename FIELD::value_type val;
      fld->value(val, *iter);
      double dval = 0.0;
      to_double(val, dval);
      // Compute the ColorMap index and retreive the color.
      const double cmin = colormap_handle->getMin();
      const double cmax = colormap_handle->getMax();
      const double index = Clamp((dval - cmin)/(cmax - cmin), 0.0, 1.0);
      float r,g,b,a;
      colormap_handle->get_color(index, r, g, b, a);
      const Color c(r, g, b);

      texts->add(buffer.str(), p, c);
    }

    ++iter;
  }
  return text_switch;
}


template <class FIELD, class Loc>
GeomHandle 
RenderField<FIELD, Loc>::render_text_cells(FieldHandle field_handle,
					   ColorMapHandle colormap_handle,
					   bool use_color_map,
					   bool use_default_color,
					   int fontsize,
					   int precision,
					   bool render_locations,
					   bool always_visible)
{
  FIELD *fld = dynamic_cast<FIELD *>(field_handle.get_rep());
  ASSERT(fld);

  typename FIELD::mesh_handle_type mesh = fld->get_typed_mesh();
  mesh->synchronize(Mesh::CELLS_E);

  GeomTexts *texts = new GeomTexts;
  if (always_visible) texts->set_always_visible();
  GeomHandle text_switch = new GeomSwitch(new GeomDL(texts));
  texts->set_font_index(fontsize);

  ostringstream buffer;
  buffer.precision(precision);

  unsigned int color_scheme = 0;
  double scol;
  Color vcol;

  if( fld->basis_order() != 0 ||
      use_default_color )
    color_scheme = 0;
  else if( use_color_map )
    color_scheme = 1;
  else {
    color_scheme = 2;
  }

  typename FIELD::mesh_type::Cell::iterator iter, end;
  mesh->begin(iter);
  mesh->end(end);
  Point p;
  while (iter != end)
  {
    mesh->get_center(p, *iter);

    buffer.str("");
    if (render_locations)
    {
      buffer << p;
    }
    else
    {
      (*iter).str_render(buffer);
    }

    if (color_scheme == 0)
    {
      texts->add(buffer.str(), p);
    }
    else if (color_scheme == 2)
    {
      typename FIELD::value_type val;
      fld->value(val, *iter);
      to_color(val, vcol);
      texts->add(buffer.str(), p, vcol);
    }
    else
    {
      typename FIELD::value_type val;
      fld->value(val, *iter);
      to_double(val, scol);
      // Compute the ColorMap index and retreive the color.
      const double cmin = colormap_handle->getMin();
      const double cmax = colormap_handle->getMax();
      const double index = Clamp((scol - cmin)/(cmax - cmin), 0.0, 1.0);

      float r,g,b,a;
      colormap_handle->get_color(index, r, g, b, a);
      const Color c(r, g, b);

      texts->add(buffer.str(), p, c);
    }

    ++iter;
  }
  return text_switch;
}

} // end namespace SCIRun

#endif // Visualization_RenderField_h
