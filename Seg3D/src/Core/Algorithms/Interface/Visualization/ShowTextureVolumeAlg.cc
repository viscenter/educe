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

#include <slivr/VideoCardInfo.h>
#include <slivr/ShaderProgramARB.h>
#include <slivr/Utils.h>
#include <Core/Geom/GeomSticky.h>
#include <Core/Volume/VolumeRenderer.h>
#include <Core/Volume/CM2Widget.h>
#include <Core/Volume/Texture.h>
#include <Core/Events/DataManager.h>
#include <Core/Algorithms/Interface/Visualization/ShowTextureVolumeAlg.h>
#include <Core/Events/CM2View/CM2View.h>
#include <Core/Events/ToolManager.h>
#include <Core/Events/BaseTool.h>


namespace SCIRun {

using SLIVR::Plane;
using SLIVR::ShaderProgramARB;

vector<size_t>
ShowTextureVolumeAlg::execute(size_t texture_id0, 
			      size_t colormap_id1, size_t colormap2_id2) 
{
  //Base class execute should never be called.
  ASSERTFAIL("Base class execute should not be called.(ShowTextureVolumeAlg)")
}

class ShowTextureVolumeAlgPriv;

//! Tools for widget manipulation and event handling.
class WidgetUpdateTool : public BaseTool
{
public:
  WidgetUpdateTool(string name, ShowTextureVolumeAlgPriv *m) :
    BaseTool(name),
    mod_(m)
  {}
  virtual ~WidgetUpdateTool() {}

  virtual propagation_state_e process_event(event_handle_t event);
private:
    ShowTextureVolumeAlgPriv *mod_;
};

class STVEventHandler : public Runnable 
{
public:
  STVEventHandler(ShowTextureVolumeAlgPriv *m) :
    mod_(m)
  {}
  virtual void          run();
private:
  ShowTextureVolumeAlgPriv *mod_;
};

class StopHandlingEvent : public BaseEvent
{
public:
  StopHandlingEvent(string t) :
    BaseEvent(t)
  {}

  virtual StopHandlingEvent *clone() { return new StopHandlingEvent(""); }
};

class ShowTextureVolumeAlgPriv : public ShowTextureVolumeAlg 
{
  friend class STVEventHandler;
  friend class WidgetUpdateTool;
public:
  ShowTextureVolumeAlgPriv();
  virtual ~ShowTextureVolumeAlgPriv() 
  {
    if (volren_) {
      delete volren_;
    }
    vector<Plane*>::iterator iter = clipping_planes_.begin();
    while (iter != clipping_planes_.end()) {
      delete *iter++;
    }
    clipping_planes_.clear();
  }

  virtual
  vector<size_t> execute(size_t texture_id0, size_t colormap_id1, 
			 size_t colormap2_id2);
  void set_widgets(vector<SLIVR::CM2Widget*> w, bool updating);
private:
  VolumeRenderer           *volren_;
  vector<Plane *>           clipping_planes_;
  ToolManager                      tm_;
  EventManager::event_mailbox_t   *events_;
};


ShowTextureVolumeAlg* get_show_texture_volume_alg() 
{
  return new ShowTextureVolumeAlgPriv();
}

ShowTextureVolumeAlgPriv::ShowTextureVolumeAlgPriv() : 
  ShowTextureVolumeAlg(),
  volren_(0),
  tm_("STVA tool manager")
{
  events_ = EventManager::register_event_messages(get_p_id());

  tool_handle_t wct = new WidgetUpdateTool("WidgetUpdateTool", this);
  tm_.add_tool(wct, 101);

  // launch the event handler.
  string tid = "EventHandler_ShowTextureVolume";
  Thread *vt = new Thread(new STVEventHandler(this), tid.c_str());
  vt->detach(); // runs until thread exits.
}

void 
ShowTextureVolumeAlgPriv::set_widgets(vector<SLIVR::CM2Widget*> w, 
                                      bool updating)
{
  if (volren_) {
    pair<float, float> vr = w[0]->get_value_range();

    vector<SLIVR::ColorMap2*> cmap2ptrs;
    cmap2ptrs.push_back(new ColorMap2(w, updating, false, vr));

    volren_->lock(); //stop rendering while state changes.
    volren_->set_colormap2(cmap2ptrs);
    volren_->set_colormap2_dirty();
    volren_->unlock();
  }
}

vector<size_t>
ShowTextureVolumeAlgPriv::execute(size_t texture_id0, 
				  size_t colormap_id1, size_t colormap2_id2)
{
  DataManager *dm = DataManager::get_dm();
  vector<size_t> rval(2, 0);

  //! get the inputs.
  if (! texture_id0) return rval;
  TextureHandle tex = dm->get_texture(texture_id0);

  bool shading_state = tex->nb(0) == 1;

  ColorMapHandle cmap1;
  bool c1 = false;
  if (colormap_id1) {
    cmap1 = dm->get_colormap(colormap_id1);
    c1 = true;
  }  


  vector<Plane *> cm2_planes(0);
  vector<int> cmap2_generation;
  vector<ColorMap2Handle> cmap2;
  bool c2 = false;
  if (colormap2_id2) {
    cmap2.push_back(dm->get_colormap2(colormap2_id2));
    c2 = true;
  }  

  //! \todo {fix the alg version of dynamic input ports.}
//   port_range_type range = get_iports("ColorMap2");
//   port_map_type::iterator pi = range.first;

//   vector<ColorMap2Handle> cmap2;
  
//   while (pi != range.second) 
//   {
//     ColorMap2IPort *cmap2_iport = 
//       dynamic_cast<ColorMap2IPort*>(get_iport(pi->second));
//     ColorMap2Handle cmap2H;
//     if (cmap2_iport && cmap2_iport->get(cmap2H) && cmap2H.get_rep()) 
//     {
//       cmap2_generation.push_back(cmap2H->generation);
//       cmap2.push_back(cmap2H);
//       for (unsigned int w = 0; w < cmap2H->widgets().size(); ++w) 
//       {
//         ClippingCM2Widget *clip = 
//           dynamic_cast<ClippingCM2Widget *>(cmap2H->widgets()[w]);
//         if (clip) 
//         {
//           cm2_planes.push_back(&clip->plane());
//         }
//       }
//     }
//     ++pi;
//   }

  ProgressReporter *pr = get_progress_reporter();

  if (c2)
  {
    if (!ShaderProgramARB::shaders_supported())
    {
      if (pr) {
	pr->warning("ColorMap2 usage requires shader support.");
      }
      cmap2.clear();
      c2 = false;
    }
  }

  // copy the pointers which are valid as long as we have a handle reference.
  vector<SLIVR::ColorMap2*> cmap2ptrs(cmap2.size(), 0);
  cmap2ptrs.clear();
  vector<ColorMap2Handle>::iterator iter = cmap2.begin();
  while (iter != cmap2.end()) 
  {
    ColorMap2Handle h = *iter++;
    cmap2ptrs.push_back(h.get_rep());
  }

  if(!volren_) 
  {
    //texhandle_copy_ = tex; // maintain the ref_cnt.
    volren_ = new VolumeRenderer((SLIVR::Texture*)tex.get_rep(), 
				 (SLIVR::ColorMap*)cmap1.get_rep(), 
				 cmap2ptrs, clipping_planes_, 
				 int(video_card_memory_size()*1024*1024*0.8));
         
    rval[0] = dm->add_geom(volren_, "VolumeRenderer Transparent");
    volren_->lock(); //stop rendering while state changes.
  } 
  else 
  {
    volren_->lock(); //stop rendering while state changes.
                     // don't trigger a gui lock while locked.

    //texhandle_copy_ = tex; // maintain the ref_cnt.
    volren_->set_texture(tex.get_rep());
    if(c1) {
      volren_->set_colormap1(cmap1.get_rep());
    } else if (!c1) {
      volren_->set_colormap1(0);
    }

    volren_->set_colormap2(cmap2ptrs);
  }
  
  volren_->set_interp(get_p_interp_mode());
  switch(get_p_render_style()) 
  {
  case 0:
    volren_->set_mode(VolumeRenderer::MODE_OVER);
    break;
  case 1:
    volren_->set_mode(VolumeRenderer::MODE_MIP);
    break;
  default:
    pr->warning("Unsupported blend mode.  Using over.");
    volren_->set_mode(VolumeRenderer::MODE_OVER);
    break;
  }

  volren_->set_gradient_range(get_p_gradient_min(), get_p_gradient_max());
  volren_->set_sampling_rate(get_p_sampling_rate_hi());
  volren_->set_interactive_rate(get_p_sampling_rate_lo());
  volren_->set_adaptive(get_p_adaptive());
  volren_->set_colormap2_width(1 << get_p_cmap2_width());
  volren_->set_slice_alpha(get_p_alpha_scale());
  volren_->set_stencil(get_p_use_stencil());
  volren_->invert_opacity(get_p_invert_opacity());
  volren_->set_blend_num_bits(get_p_blend_res());
  volren_->set_shading(get_p_shading());
  volren_->set_material(get_p_ambient(), get_p_diffuse(), 
			get_p_specular(), get_p_shine());
  volren_->set_light(get_p_light());

  volren_->unlock(); //done changing state, allow rendering.


  //! \todo{enable the plane widgets in this alg}
//   // Keep track of the widgets until we can lock again.
//   vector<Plane*> to_delete;
//   // Always one fewer widget than colormap2Ds.
//   int ncmps = cmap2.size();
//   p_num_clipping_planes_.set(ncmps ? ncmps - 1 : 0);
//   if (p_num_clipping_planes_.get() != (int)widget_.size()) 
//   {
//     BBox bbox;
//     tex->get_bounds(bbox);
    
//     while (p_num_clipping_planes_.get() != (int)widget_.size())
//     {
//       if ((int)widget_.size() < p_num_clipping_planes_.get()) {
// 	// add a widget.
// 	push_back_new_widget(bbox);
//       } else {
// 	// remove a widget.
// 	Plane *p = remove_last_widget();
// 	if (p) to_delete.push_back(p);
//       }
//     }
//   }
  //Sync the planes in the Volume Renderer.
  volren_->lock();
  volren_->set_planes(clipping_planes_);
  volren_->set_colormap2_dirty();
//   for (size_t i = 0; i < to_delete.size(); ++i) {
//     delete to_delete[i];
//   }
  volren_->unlock();

  // Sync all of the widget switches with the checkbox.
//   for (unsigned int w = 0; w < widget_switch_.size(); ++w) 
//     ((GeomSwitch*)widget_switch_[w].get_rep())->
//       set_state(p_show_clipping_widgets_.get());


  return rval;
}

void          
STVEventHandler::run() {
  for (;;) {
    event_handle_t e = mod_->events_->receive();
    StopHandlingEvent *quit = dynamic_cast<StopHandlingEvent*>(e.get_rep());
    if (quit) break;
    mod_->tm_.propagate_event(e);
  }
}

BaseTool::propagation_state_e 
WidgetUpdateTool::process_event(event_handle_t event)
{
  SetWidgetsEvent *swe = dynamic_cast<SetWidgetsEvent*>(event.get_rep());
  if (! swe) {
    return CONTINUE_E;
  }
  mod_->set_widgets(swe->widgets_, swe->updating_);
  return CONTINUE_E;
}

} //end namespace SCIRun

