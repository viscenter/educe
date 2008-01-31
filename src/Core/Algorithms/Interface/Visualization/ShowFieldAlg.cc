//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2007 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  License for the specific language governing rights and limitations under
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#include <iostream>
#include <Core/Events/DataManager.h>
#include <Core/Algorithms/Interface/Visualization/ShowFieldAlg.h>
#include <Core/Algorithms/Visualization/RenderField.h>

namespace SCIRun {

ShowFieldAlg* get_show_field_alg()
{
  return new ShowFieldAlg();
}

//! Algorithm Interface.
vector<size_t>
ShowFieldAlg::execute(size_t fld_id, size_t cmap_id) 
{
  vector<size_t> rval(4, 0);
  DataManager *dm = DataManager::get_dm();

  FieldHandle fld_handle = dm->get_field(fld_id);
  ColorMapHandle cmap_handle = dm->get_colormap(cmap_id);

  const TypeDescription *ftd = fld_handle->get_type_description();
  const TypeDescription *ltd = fld_handle->order_type_description();
  // description for just the data in the field

  // Get the Algorithm.
  CompileInfoHandle ci = RenderFieldBase::get_compile_info(ftd, ltd);
  DynamicAlgoHandle  dalgo;         //! the algorithm instance (return data)
  RenderFieldBase   *renderer; 
  if (!DynamicCompilation::compile(ci, dalgo)) {
    return rval;
  }
  
  renderer = (RenderFieldBase*)dalgo.get_rep();
  if (! renderer) {
    cerr << "Error: could not get algorithm in render_field" 
	 << endl;
    return rval;
  }

  MaterialHandle def_mat = new Material();
  
  def_mat->diffuse = Color(get_p_def_color_r(), 
			   get_p_def_color_g(), 
			   get_p_def_color_b());
  def_mat->transparency = get_p_def_color_a();

  MaterialHandle text_mat = new Material();

  text_mat->diffuse = Color(get_p_text_color_r(), 
			    get_p_text_color_g(), 
			    get_p_text_color_b());
  text_mat->transparency = 1.0;
  
  if (get_p_nodes_on()) {
    GeomHandle gh =
      renderer->render_mesh_nodes(fld_handle,
				  cmap_handle.get_rep(),
				  get_p_nodes_display_type(), 
				  get_p_nodes_scale(), 
				  get_p_nodes_resolution(),
				  get_p_nodes_color_type() == 0,
				  get_p_nodes_transparency());
    string name = "Nodes";
    if (get_p_nodes_transparency()) {
      name = "Transparent Nodes";
    }

    GeomHandle gmat = new GeomMaterial(gh, def_mat);
    GeomHandle geom = new GeomSwitch(new GeomColorMap(gmat, cmap_handle));
    rval[0] = dm->add_geom(geom, name);
  }
  if (get_p_edges_on()) {
    GeomHandle gh =
      renderer->render_mesh_edges(fld_handle,
				  cmap_handle.get_rep(),
				  get_p_edges_display_type(),
				  get_p_edges_scale(), 
				  get_p_edges_resolution(),
				  get_p_edges_color_type() == 0,
				  get_p_edges_transparency(),
				  get_p_approx_div());
    string name = "Edges";
    if (get_p_edges_transparency()) {
      name = "Transparent Edges";
    }
    GeomHandle gmat = new GeomMaterial(gh, def_mat);
    GeomHandle geom = new GeomSwitch(new GeomColorMap(gmat, cmap_handle));
    rval[1] = dm->add_geom(geom, name);
  }
  if (get_p_faces_on()) {
    if (get_p_faces_normals())
	  fld_handle->mesh()->synchronize(Mesh::NORMALS_E);

    GeomHandle gh =
      renderer->render_mesh_faces(fld_handle,
				  cmap_handle.get_rep(),
				  get_p_faces_normals(),
				  get_p_faces_color_type() == 0,
				  get_p_faces_transparency(),
				  get_p_faces_usetexture(),
				  get_p_approx_div());
    string name = "Faces";
    if (get_p_faces_transparency()) {
      name = "Transparent Faces";
    }
    GeomHandle gmat = new GeomMaterial(gh, def_mat);
    GeomHandle geom = new GeomSwitch(new GeomColorMap(gmat, cmap_handle));
    rval[2] = dm->add_geom(geom, name);
  }
  
  if (get_p_text_on()) {
    GeomHandle gh =
      renderer->render_text(fld_handle,
			    cmap_handle,
			    get_p_text_color_type() == 1,
			    get_p_text_color_type() == 0,
			    get_p_text_backface_cull(),
			    get_p_text_fontsize(),
			    get_p_text_precision(),
			    get_p_text_render_locations(),
			    get_p_text_show_data(),
			    get_p_text_show_nodes(),
			    get_p_text_show_edges(),
			    get_p_text_show_faces(),
			    get_p_text_show_cells(),
			    get_p_text_always_visible());

    string name = "Text Data";
    GeomHandle gmat = new GeomMaterial(gh, def_mat);
    GeomHandle geom = new GeomSwitch(new GeomColorMap(gmat, cmap_handle));
    rval[3] = dm->add_geom(geom, name);
  }
  return rval;
}

} //end namespace SCIRun

