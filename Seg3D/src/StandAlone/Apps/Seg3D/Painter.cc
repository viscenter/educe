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
 *  Painter.cc
 *
 *  Written by:
 *   McKay Davis
 *   School of Computing
 *   University of Utah
 *   November, 2005
 *
 *  Copyright (C) 2005 SCI Group
 */
#include <sci_comp_warn_fixes.h>
#include <stdlib.h>
#include <math.h>
#include <map>
#include <typeinfo>
#include <iostream>
#include <sci_gl.h>
#include <sci_algorithm.h>
#include <Core/Containers/Array3.h>
#include <Core/Datatypes/Field.h> 
#include <Core/Exceptions/GuiException.h>
#include <Core/Geom/Material.h>
#include <Core/Geom/ColorMappedNrrdTextureObj.h>
#include <Core/Geom/GeomSwitch.h>
#include <Core/Skinner/GeomSkinnerVarSwitch.h>
#include <Core/Geom/GeomCull.h>
#include <Core/Geom/GeomGroup.h>
#include <Core/Geom/TexSquare.h>
#include <Core/Geom/OpenGLViewport.h>
#include <Core/Geom/FreeType.h>
#include <Core/Malloc/Allocator.h>
#include <Core/Math/MiscMath.h>
#include <Core/Math/MinMax.h>
#include <Core/Thread/CleanupManager.h>
#include <Core/Thread/Runnable.h>
#include <Core/Thread/Mutex.h>
#include <Core/Util/Environment.h>
#include <Core/Volume/CM2Widget.h>
#include <Core/Geom/TextRenderer.h>
#include <Core/Geom/FontManager.h>
#include <Core/Geom/GeomColorMappedNrrdTextureObj.h>
#include <Core/Skinner/Variables.h>
#include <Core/Events/EventManager.h>
#include <Core/Events/SceneGraphEvent.h>
#include <Core/Util/FileUtils.h>
#include <StandAlone/Apps/Seg3D/Painter.h>
#include <StandAlone/Apps/Seg3D/PointerToolSelectorTool.h>
#include <StandAlone/Apps/Seg3D/KeyToolSelectorTool.h>
#include <StandAlone/Apps/Seg3D/Seg3DFrame.h>
#include <StandAlone/Apps/Seg3D/BrushTool.h>
#include <StandAlone/Apps/Seg3D/ITKCurvatureAnisotropicDiffusionImageFilterTool.h>

#include <StandAlone/Apps/Seg3D/ITKNeighborhoodConnectedImageFilterTool.h>

#include <StandAlone/Apps/Seg3D/ITKThresholdImageFilterTool.h>
#include <StandAlone/Apps/Seg3D/ITKDiscreteGaussianImageFilterTool.h>

// needed for Painter::maker
#if defined(_WIN32) && !defined(BUILD_SCIRUN_STATIC)
#  define SCISHARE __declspec(dllexport)
#else
#  define SCISHARE
#endif


namespace SCIRun {

Seg3DFrame* Painter::global_seg3dframe_pointer_ = NULL;
static Painter *global_painter_pointer = NULL;


Painter::Painter(Skinner::Variables *variables, VarContext* ctx) :
  Parent(variables),
  cur_window_(0),
  tm_("Painter"),
  pointer_pos_(),
  windows_(),
  volumes_(),
  current_volume_(0),
  mask_volume_(0),
  current_vrender_target_(0),
  current_vrender_target_deferred_(false),
  volume_lock_("Volume")
{
  tm_.add_tool(new PointerToolSelectorTool(this), 50);
  tm_.add_tool(new KeyToolSelectorTool(this), 51);

  event_handle_t event;
  InitializeSignalCatcherTargets(event);

  global_painter_pointer = this;
}


Painter::~Painter()
{
}


void
Painter::redraw_all()
{
  for (SliceWindows::iterator i = windows_.begin(); i != windows_.end(); ++i) {
    (*i)->mark_redraw();
  }
}


void
Painter::extract_all_window_slices()
{
  for (SliceWindows::iterator i = windows_.begin(); i != windows_.end(); ++i)
  {
    (*i)->extract_slices();
  }
}



void
Painter::get_data_from_layer_buttons()  
{
  volume_lock_.lock();
  event_handle_t event = 0;
  for (unsigned int i = 0; i < layer_buttons_.size(); ++i) {
    layer_buttons_[i]->update_from_gui(event);
  }
  volume_lock_.unlock();
}


void
Painter::rebuild_layer_buttons()  
{
  get_data_from_layer_buttons();
  unsigned int bpos = 0;  
  volume_lock_.lock();
  for (int i = volumes_.size()-1; i >= 0 ; --i) {
    build_layer_button(bpos, volumes_[i]);
  }
  for (; bpos < layer_buttons_.size(); ++bpos) {
    layer_buttons_[bpos]->visible_ = false;
    layer_buttons_[bpos]->volume_ = 0;
  }
  volume_lock_.unlock();
  EventManager::add_event(new WindowEvent(WindowEvent::REDRAW_E));
}


void
Painter::build_layer_button(unsigned int &bpos, NrrdVolumeHandle &volume)  
{
  // TODO: Allocate the layer buttons dynamically instead of
  // statically declaring 100 of them in the .skin file (when the
  // layer UI moves to wxwidgets).
  if (bpos >= layer_buttons_.size())
  {
    set_status("Layer button overflow.");
    return;
  }

  volume->lock.lock();
  LayerButton *button = layer_buttons_[bpos];
  button->visible_ = true;
  button->layer_name_ = volume->name_;
  button->layer_visible_ = volume->tmp_visible_;
  button->volume_ = volume;
  volume->button_ = button;
  volume->set_geom_switch(button->layer_visible_);
  
  button->indent_ = volume->depth() * 10 + 5;

  if (volume->label_) {
    Color c = volume->get_label_color();
    button->color_ = Skinner::Color(c.r(), c.g(), c.b(), 1.0);
    button->color_is_selectable_ = true;
  }
  else
  {
    button->color_ = Skinner::Color(0.0, 0.0, 0.0, 0.0);
    button->color_is_selectable_ = false;
  }

  if (volume == current_volume_ && volume == mask_volume_)
  {
    button->background_color_ = Skinner::Color(0.6, 0.3, 0.6, 0.75);
  }
  else if (volume == current_volume_)
  {
    button->background_color_ = Skinner::Color(0.3, 0.3, 0.6, 0.75);
  }
  else if (volume == mask_volume_)
  {
    button->background_color_ = Skinner::Color(0.6, 0.3, 0.3, 0.75);
  }
  else
  {
    button->background_color_ = Skinner::Color(0.0, 0.0, 0.0, 0.0);
  }

  button->active_render_target_ = (volume == current_vrender_target_);

  bpos++;  

  volume->lock.unlock();
}


void
Painter::build_volume_list(NrrdVolumes &volumes)
{
  // TODO:  Get rid of this and use volumes_ directly.
  // Make a copy of volumes_.  Probably should just return volumes directly.
  volume_lock_.lock();
  for (unsigned int i = 0; i < volumes_.size(); ++i) {
    volumes.push_back(volumes_[i]);
  }
  volume_lock_.unlock();
}


void
Painter::opacity_down()
{
  if (current_volume_.get_rep()) {
    current_volume_->opacity_ = 
      Clamp(current_volume_->opacity_-0.05, 0.0, 1.0);
    redraw_all();
  }
}


void
Painter::opacity_up()
{
  if (current_volume_.get_rep()) {
    current_volume_->opacity_ = 
      Clamp(current_volume_->opacity_+0.05, 0.0, 1.0);
    redraw_all();
  }
}


void
Painter::current_layer_down()
{
  if (!current_volume_.get_rep()) return;

  volume_lock_.lock();
  int pos;
  int i;
  for (i = 0; i < (int)layer_buttons_.size(); i++)
  {
    if (layer_buttons_[i]->volume_ == current_volume_)
    {
      pos = i-1;
    }
    if (layer_buttons_[i]->volume_.get_rep() == 0) break;
  }
  if (pos == -1) pos = i-1;
  current_volume_ = layer_buttons_[pos]->volume_;
  volume_lock_.unlock();

  rebuild_layer_buttons();
}


void
Painter::current_layer_up()
{
  if (!current_volume_.get_rep()) return;

  volume_lock_.lock();
  int pos = 0;
  int i;
  for (i = 0; i < (int)layer_buttons_.size(); i++)
  {
    if (layer_buttons_[i]->volume_ == current_volume_)
    {
      pos = i+1;
    }
    if (layer_buttons_[i]->volume_.get_rep() == 0) break;
  }
  if (pos == i) pos = 0;
  current_volume_ = layer_buttons_[pos]->volume_;
  volume_lock_.unlock();

  rebuild_layer_buttons();
}


void
Painter::set_probe()
{
  for (SliceWindows::iterator i = windows_.begin(); i != windows_.end(); ++i) {
    (*i)->set_probe();
  }
  redraw_all();
}


void
Painter::set_all_slices_tex_dirty()
{
  volume_lock_.lock();
  for (NrrdVolumes::iterator iter = volumes_.begin(); 
       iter != volumes_.end(); ++iter) {
    (*iter)->set_slices_dirty();
  }
  volume_lock_.unlock();
}


NrrdVolumeHandle
Painter::find_volume_by_name(const string &name, NrrdVolumeHandle parent)
{
  NrrdVolumeHandle retval = 0;
  volume_lock_.lock();
  for (size_t i = 0; i < volumes_.size(); ++i) {
    if (volumes_[i]->name_ == name) {
      retval = volumes_[i]; break;
    }
  }
  volume_lock_.unlock();
  return retval;
}


string
Painter::unique_layer_name(string base)
{
  string::size_type pos = base.find_last_not_of(" 0123456789");
  base = base.substr(0, pos+1);
  int i = 0;
  string name = base;
  while (find_volume_by_name(name).get_rep())
    name = base + " "+to_string(++i);
  return name;
}


string
Painter::unique_layer_name_from_filename(string filename)
{
  string tmp = changeExtension(filename, "");
  string base = tmp.substr(0, tmp.size()-1);
 
  int i = 1;
  string name = base;
  while (find_volume_by_name(name).get_rep())
    name = base + "(" + to_string(++i) + ")";
  return name;
}


Skinner::Drawable *
Painter::maker(Skinner::Variables *vars) 
{
  return new Painter(vars, 0);
}


NrrdVolumeHandle
Painter::make_layer(string name, NrrdDataHandle &nrrdh, unsigned int mask)
{
  volume_lock_.lock();
  NrrdVolume *vol = new NrrdVolume(this, unique_layer_name(name), nrrdh, mask);
  volumes_.push_back(vol);
  current_volume_ = vol;
  rebuild_layer_buttons();
  volume_lock_.unlock();  
  return vol;
}


NrrdVolumeHandle
Painter::copy_current_layer(string suff)
{
  if (!current_volume_.get_rep()) return 0;
  NrrdDataHandle nrrdh = current_volume_->nrrd_handle_;
  nrrdh.detach(); // Copies the layer memory to the new layer
  return make_layer(current_volume_->name_+suff,nrrdh,current_volume_->label_);
}


void
Painter::show_visible_item(const string &id, const string &group)
{
  // Find the top of the drawable tree.
  Drawable *ptmp = this;
  while (ptmp->get_parent()) ptmp = ptmp->get_parent();

  // Grab all the callbacks with the name we care about in the tree.
  Skinner::Callbacks_t callbacks;
  Parent *parent = dynamic_cast<Parent *>(ptmp);
  if (parent)
    parent->find_callbacks(callbacks, "VisibilityGroup::show_VisibleItem");

  // Set the callback's variables.
  Skinner::Callbacks_t::iterator itr = callbacks.begin();
  while (itr != callbacks.end())
  {
    (*itr)->variables_->insert("id", id);
    (*itr)->variables_->insert("group", group);

    ++itr;
  }

  // Call all the callbacks.  Only the ones with the correct variables
  // will do anything, the others will just return.
  Skinner::Signal *s =
    new Skinner::Signal("VisibilityGroup::show_VisibleItem", this, NULL);
  event_handle_t event(s);

  SignalThrower::throw_signal(callbacks, event);
}


vector<string>
Painter::get_filename_series(string filename)
{
  filename = substituteTilde(filename);
  convertToUnixPath(filename);

  pair<string,string> dfile = split_filename(filename);
  if (validDir(filename)) {
    dfile.first = filename;
    dfile.second = "";
  }

  vector<string> files;
  try {
    typedef itk::DICOMSeriesFileNames series_t;
    series_t::Pointer names = series_t::New();
    names->SetDirectory(dfile.first);
    names->SetFileNameSortingOrder(series_t::SortByImageNumber);
    files = names->GetFileNames();
  } catch (...) {
  }
  if (files.empty()) {
    files = GetFilenamesInSequence(dfile.first, dfile.second);
    sort (files.begin(), files.end());
    vector<string> files2;
    for (vector<string>::size_type i = 0; i < files.size(); ++i)
    {
      const string filename = dfile.first + "/" + files[i];
      if (validFile(filename)) {
        files2.push_back(filename);
      }
    }
    files = files2;
  }

  return files;
}


BaseTool::propagation_state_e
Painter::process_event(event_handle_t &event)
{
  ThrowSkinnerSignalEvent *tsse =
    dynamic_cast<ThrowSkinnerSignalEvent *>(event.get_rep());
  if (tsse)
  {
    tsse->set_vars(get_vars());
    throw_signal(tsse->get_name());
#ifdef __WX_GTK__
    // Finish the event for threaded synchronous throw.
    tsse->up();
#endif
    return STOP_E;
  }
  else
  {
    return Parent::process_event(event);
  }
}


void
Painter::ThrowSkinnerSignal(const string &name, bool sync)
{
  ThrowSkinnerSignalEvent *tsse = new ThrowSkinnerSignalEvent(name);
  ThrowSkinnerSignal(tsse, sync);
}


void
Painter::ThrowSkinnerSignal(ThrowSkinnerSignalEvent *tsse, bool sync)
{
  if (sync)
  {
#ifdef __WX_GTK__
    wxMutexGuiLeave();
    event_handle_t event(tsse);
    EventManager::add_event(event);

    // Wait for the event to finish.
    tsse->down();
    wxMutexGuiEnter();
#else
    // Throw in this thread, backwards compatable (thread unsafe)
    tsse->set_vars(global_painter_pointer->get_vars());
    global_painter_pointer->throw_signal(tsse->get_name());
#endif
  }
  else
  {
    // Asynchronous throw.
    event_handle_t event(tsse);
    EventManager::add_event(event);
  }
}


void
Painter::update_progress(int progress)
{
  wxPanel *panel = Painter::global_seg3dframe_pointer_->CurrentToolPanel();
  if (panel)
  {
    wxCommandEvent wxevent(wxEVT_SET_PROGRESS, wxID_ANY);
    wxevent.SetInt(progress);

    wxPostEvent(panel, wxevent);
  }
}


void
Painter::start_progress()
{
  update_progress(-1);
}


void
Painter::finish_progress()
{
  update_progress(101);
}


NrrdVolumeHandle
Painter::create_new_label(NrrdVolumeHandle &likethis, const string &name)
{
  NrrdVolumeHandle result;
  volume_lock_.lock();

  // See if it fits in the current label.
  if (likethis->label_)
  {
    result = likethis->create_child_label_volume();
    if (result.get_rep() && name != "")
    {
      result->name_ = unique_layer_name(name);
    }
  }
  
  // Search for a space amongst the current volumes.
  if (!result.get_rep())
  {
    for (size_t i = 0; i < volumes_.size(); i++)
    {
      if (volumes_[i]->label_ &&
          VolumeOps::compare_nrrd_info(likethis->nrrd_handle_,
                                       volumes_[i]->nrrd_handle_))
      {
        result = volumes_[i]->create_child_label_volume();
      }
      if (result.get_rep())
      {
        if (name != "")
        {
          result->name_ = unique_layer_name(name);
        }
        break;
      }
    }
  }

  // If we reused space make sure it is clear.
  if (result.get_rep())
  {
    VolumeOps::bit_clear(result->nrrd_handle_, result->label_);
  }

  // If no space was found just make a new one.
  if (!result.get_rep())
  {
    NrrdDataHandle nrrdh = 
      VolumeOps::create_clear_nrrd(likethis->nrrd_handle_, LabelNrrdType);
    string newname = unique_layer_name(name);
    if (newname == "")
    {
      newname = unique_layer_name(likethis->name_ + " Label 1");
    }
    result = new NrrdVolume(this, newname, nrrdh, 1);
  }
  volumes_.push_back(result);
  current_volume_ = result;

  volume_lock_.unlock();

  return result;
}


} // end namespace SCIRun
