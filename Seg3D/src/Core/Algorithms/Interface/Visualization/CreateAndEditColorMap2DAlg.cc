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
#include <Core/Thread/RecursiveMutex.h>
#include <Core/Events/DataManager.h>
#include <Core/Algorithms/Interface/Visualization/CreateAndEditColorMap2DAlg.h>
#include <Core/Events/CM2View/CM2View.h>
#include <Core/Events/CursorChangeEvent.h>
#include <Core/Events/ToolManager.h>
#include <Core/Events/BaseTool.h>

namespace SCIRun {

size_t
CreateAndEditColorMap2DAlg::execute(size_t, size_t)
{
  //DataManager *dm = DataManager::get_dm();
  std::cerr << "WARNING: CreateAndEditColorMap2DAlg::execute not implemented." << std::endl;
  return 0;
}

class CreateAndEditColorMap2DPriv;

class HandleCursorChangeTool : public BaseTool
{
public:
  HandleCursorChangeTool(string name, CreateAndEditColorMap2DPriv *m) :
    BaseTool(name),
    mod_(m)
  {}
  virtual ~HandleCursorChangeTool() {}

  propagation_state_e process_event(event_handle_t event);

private:
  CreateAndEditColorMap2DPriv *mod_;

};

//! Tools for widget manipulation and event handling.
class WidgetsChangedTool : public BaseTool
{
public:
  WidgetsChangedTool(string name, CreateAndEditColorMap2DPriv *m) :
    BaseTool(name),
    mod_(m)
  {}
  virtual ~WidgetsChangedTool() {}

  virtual propagation_state_e process_event(event_handle_t event);
private:
    CreateAndEditColorMap2DPriv *mod_;
};

class StopHandlingEvent : public BaseEvent
{
public:
  StopHandlingEvent(string t) :
    BaseEvent(t)
  {}

  virtual StopHandlingEvent *clone() { return new StopHandlingEvent(""); }
};

class EventHandler : public Runnable 
{
public:
  EventHandler(CreateAndEditColorMap2DPriv *m) :
    mod_(m)
  {}
  virtual void          run();
private:
  CreateAndEditColorMap2DPriv *mod_;
};


class CreateAndEditColorMap2DPriv : public CreateAndEditColorMap2DAlg
{
  friend class EventHandler;
  friend class WidgetsChangedTool;
public:
  CreateAndEditColorMap2DPriv();
  virtual ~CreateAndEditColorMap2DPriv();
  virtual size_t execute(size_t colormap2_id0, size_t nrrd_histo);
private:
  void                         update_view_widgets();
  void                         clear_widgets();
  void                         set_widgets(const vector<SLIVR::CM2Widget*> &w,
					   bool updating);
  void                         wlock() { wid_mutex_.lock(); }
  void                         wunlock() { wid_mutex_.unlock(); }

  //! We get events from the CM2View to update widgets.
  ColorMap2Handle		   merge_cmap2_;
  ToolManager                      tm_;
  EventManager::event_mailbox_t   *events_;
  vector<SLIVR::CM2Widget*>	   widgets_;
  NrrdDataHandle                   histo_;
  RecursiveMutex                   wid_mutex_;
};




CreateAndEditColorMap2DAlg* get_create_and_edit_colormap2d_alg()
{
  return new CreateAndEditColorMap2DPriv();
}


CreateAndEditColorMap2DPriv::CreateAndEditColorMap2DPriv() : 
  CreateAndEditColorMap2DAlg(),
  merge_cmap2_(0),
  tm_("CAECM2 tool manager"),
  widgets_(),
  histo_(0),
  wid_mutex_("CreateAndEditColorMap2D widget mutex")
{
  widgets_.push_back(scinew RectangleCM2Widget());
  events_ = EventManager::register_event_messages("EM:" + 
                                                  get_p_cm2view_targ());

  tool_handle_t cct = new HandleCursorChangeTool("HandleCursorChangeTool", 
						 this);
  tm_.add_tool(cct, 100);

  tool_handle_t wct = new WidgetsChangedTool("WidgetsChangedTool", this);
  tm_.add_tool(wct, 101);

  // launch the event handler.
  string tid = "EventHandler_" + get_p_cm2view_targ();
  Thread *vt = scinew Thread(new EventHandler(this), tid.c_str());
  vt->detach(); // runs until thread exits.
}

CreateAndEditColorMap2DPriv::~CreateAndEditColorMap2DPriv()
{}

void
CreateAndEditColorMap2DPriv::update_view_widgets()
{
  vector<SLIVR::CM2Widget*> a;
  if (merge_cmap2_.get_rep()) {
    a = merge_cmap2_->widgets();
  }
  wlock();
  SetWidgetsEvent *e = new SetWidgetsEvent(widgets_, a, get_p_cm2view_targ());
  wunlock();
  event_handle_t event = e;
  EventManager::add_event(event);
}
void 
CreateAndEditColorMap2DPriv::clear_widgets()
{
  for (size_t i = 0; i < widgets_.size(); ++i) {
    if (widgets_[i]) delete widgets_[i];
  }
  widgets_.clear();
}

void 
CreateAndEditColorMap2DPriv::set_widgets(const vector<SLIVR::CM2Widget*> &w,
                                         bool updating)
{
  clear_widgets();
  widgets_ = w;
  //notify vol renderer about widget change

}


//! Algorithm Interface.
size_t
CreateAndEditColorMap2DPriv::execute(size_t colormap2_id0, size_t nrrd_histo)
{
  DataManager *dm = DataManager::get_dm();
  size_t rval = 0;

  if (colormap2_id0) {
    merge_cmap2_ = dm->get_colormap2(colormap2_id0);
  } 

  update_view_widgets();


  ProgressReporter *pr = get_progress_reporter();
  if (nrrd_histo)  {
    histo_ = dm->get_nrrd(nrrd_histo);
    if(histo_->nrrd_->dim != 2 && histo_->nrrd_->dim != 3) {
      if (pr) pr->error("Invalid input histogram dimension.");
      return rval;
    }
    //! tell the CM2View about the histo    
    event_handle_t event = new SetHistoEvent(histo_->nrrd_, get_p_histo(),
                                             get_p_cm2view_targ());
    EventManager::add_event(event);
  } else {
    event_handle_t event = new SetHistoEvent(0, get_p_histo(),
					     get_p_cm2view_targ());
    EventManager::add_event(event);
  }

  wlock();
  //copy the widgets to send.
  vector<SLIVR::CM2Widget*>	wds;
  pair<float, float> vr = widgets_[0]->get_value_range();
  deep_copy_widgets(widgets_, wds);
  wunlock();

  // If there are input widgets, insert copies in the widget list.
  if (merge_cmap2_.get_rep()) {
    vector<SLIVR::CM2Widget*>	mwds;
    deep_copy_widgets(merge_cmap2_->widgets(), mwds);
    vector<SLIVR::CM2Widget*>::iterator iter = mwds.begin();
    while (iter != mwds.end()) {
      SLIVR::CM2Widget *w = *iter++;
      wds.push_back(w);
    }
  }

  rval = dm->add_colormap2(new ColorMap2(wds, true, 0, vr));
  return rval;
}

void          
EventHandler::run() {
  for (;;) {
    event_handle_t e = mod_->events_->receive();
    StopHandlingEvent *quit = dynamic_cast<StopHandlingEvent*>(e.get_rep());
    if (quit) break;
    mod_->tm_.propagate_event(e);
  }
}

BaseTool::propagation_state_e 
HandleCursorChangeTool::process_event(event_handle_t event)
{
  CursorChangeEvent *cce = 
    dynamic_cast<CursorChangeEvent*>(event.get_rep());
  if (! cce) {
    return CONTINUE_E;
  }
  //mod_->set_window_cursor(cce->get_cursor_name());
  return STOP_E;
}

BaseTool::propagation_state_e 
WidgetsChangedTool::process_event(event_handle_t event)
{
  SetWidgetsEvent *swe = dynamic_cast<SetWidgetsEvent*>(event.get_rep());
  if (! swe) {
    return CONTINUE_E;
  }
  mod_->wlock();
  mod_->set_widgets(swe->widgets_, swe->updating_);
  mod_->wunlock();
  return CONTINUE_E;
}

} //end namespace SCIRun

