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
 *  ClipFieldToFieldOrWidget.cc:  Rotate and flip field to get it into "standard" view
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   March 2001
 *
 *  Copyright (C) 2001 SCI Group
 */

#include <Core/Util/DynamicCompilation.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/GeometryPort.h>
#include <Core/Thread/CrowdMonitor.h>
#include <Dataflow/Widgets/BoxWidget.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInterface.h>
#include <Core/Datatypes/Clipper.h>
#include <Dataflow/Modules/Fields/ClipFieldToFieldOrWidget.h>
#include <Core/Containers/StringUtil.h>
#include <iostream>
#include <stack>

namespace SCIRun {

using std::stack;

class ClipFieldToFieldOrWidget : public Module
{
private:
  BoxWidget *box_;
  CrowdMonitor widget_lock_;
  BBox last_bounds_;
  GuiString clip_location_;
  GuiString clip_mode_;
  GuiInt    autoexec_;
  GuiInt    autoinvert_;
  GuiString exec_mode_;
  GuiDouble center_x_;
  GuiDouble center_y_;
  GuiDouble center_z_;
  GuiDouble right_x_;
  GuiDouble right_y_;
  GuiDouble right_z_;
  GuiDouble down_x_;
  GuiDouble down_y_;
  GuiDouble down_z_;
  GuiDouble in_x_;
  GuiDouble in_y_;
  GuiDouble in_z_;
  GuiDouble scale_;
  bool      first_pass_;
  int  last_input_generation_;
  int  last_clip_generation_;
  ClipperHandle clipper_;
  stack<ClipperHandle> undo_stack_;
  int widgetid_;
  FieldHandle ofield_;

  bool bbox_similar_to(const BBox &a, const BBox &b);
  // check to see if any of the Points are not the default of -1
  bool points_differ();

public:
  ClipFieldToFieldOrWidget(GuiContext* ctx);
  virtual ~ClipFieldToFieldOrWidget();

  virtual void execute();
  virtual void widget_moved(bool, BaseWidget*);
};


DECLARE_MAKER(ClipFieldToFieldOrWidget)

ClipFieldToFieldOrWidget::ClipFieldToFieldOrWidget(GuiContext* ctx)
  : Module("ClipFieldToFieldOrWidget", ctx, Filter, "NewField", "SCIRun"),
    widget_lock_("ClipFieldToFieldOrWidget widget lock"),
    clip_location_(get_ctx()->subVar("clip-location"), "cell"),
    clip_mode_(get_ctx()->subVar("clipmode"), "replace"),
    autoexec_(get_ctx()->subVar("autoexecute"), 0),
    autoinvert_(get_ctx()->subVar("autoinvert"), 0),
    exec_mode_(get_ctx()->subVar("execmode"), "0"),
    center_x_(get_ctx()->subVar("center_x"), -1.0),
    center_y_(get_ctx()->subVar("center_y"), -1.0),
    center_z_(get_ctx()->subVar("center_z"), -1.0),
    right_x_(get_ctx()->subVar("right_x"), -1.0),
    right_y_(get_ctx()->subVar("right_y"), -1.0),
    right_z_(get_ctx()->subVar("right_z"), -1.0),
    down_x_(get_ctx()->subVar("down_x"), -1.0),
    down_y_(get_ctx()->subVar("down_y"), -1.0),
    down_z_(get_ctx()->subVar("down_z"), -1.0),
    in_x_(get_ctx()->subVar("in_x"), -1.0),
    in_y_(get_ctx()->subVar("in_y"), -1.0),
    in_z_(get_ctx()->subVar("in_z"), -1.0),
    scale_(get_ctx()->subVar("scale"), -1.0),
    first_pass_(true),
    last_input_generation_(0),
    last_clip_generation_(0),
    widgetid_(0),
    ofield_(0)
{
  box_ = scinew BoxWidget(this, &widget_lock_, 1.0, false, false);
  box_->Connect((GeometryOPort *)get_oport("Selection Widget"));
}


ClipFieldToFieldOrWidget::~ClipFieldToFieldOrWidget()
{
  delete box_;
}


static bool
check_ratio(double x, double y, double lower, double upper)
{
  if (fabs(x) < 1e-6)
  {
    if (!(fabs(y) < 1e-6))
    {
      return false;
    }
  }
  else
  {
    const double ratio = y / x;
    if (ratio < lower || ratio > upper)
    {
      return false;
    }
  }
  return true;
}


bool
ClipFieldToFieldOrWidget::bbox_similar_to(const BBox &a, const BBox &b)
{
  return 
    a.valid() &&
    b.valid() &&
    check_ratio(a.min().x(), b.min().x(), 0.5, 2.0) &&
    check_ratio(a.min().y(), b.min().y(), 0.5, 2.0) &&
    check_ratio(a.min().z(), b.min().z(), 0.5, 2.0) &&
    check_ratio(a.min().x(), b.min().x(), 0.5, 2.0) &&
    check_ratio(a.min().y(), b.min().y(), 0.5, 2.0) &&
    check_ratio(a.min().z(), b.min().z(), 0.5, 2.0);
}

bool
ClipFieldToFieldOrWidget::points_differ()
{
  reset_vars();
  if (center_x_.get() != -1 || center_y_.get() != -1 || center_z_.get() != -1 ||
      right_x_.get() != -1 || right_y_.get() != -1 || right_z_.get() != -1 ||
      down_x_.get() != -1 || down_y_.get() != -1 || down_z_.get() != -1 ||
      in_x_.get() != -1 || in_y_.get() != -1 || in_z_.get() != -1) {
    return true;
  }
  return false;
}


void
ClipFieldToFieldOrWidget::execute()
{
  // Get input field.
  FieldHandle ifieldhandle;
  if (!get_input_handle("Input Field", ifieldhandle)) return;
  if (!ifieldhandle->mesh()->is_editable())
  {
    error("Not an editable mesh type (try passing Field through an Unstructure module first).");
    return;
  }

  bool do_clip_p = false;

  FieldHandle cfieldhandle;
  if (get_input_handle("Clip Field", cfieldhandle, false) &&
      cfieldhandle->generation != last_clip_generation_)
  {
    last_clip_generation_ = cfieldhandle->generation;

    const TypeDescription *ftd = cfieldhandle->mesh()->get_type_description();
    CompileInfoHandle ci = ClipFieldToFieldOrWidgetMeshAlgo::get_compile_info(ftd);
    Handle<ClipFieldToFieldOrWidgetMeshAlgo> algo;
    if (!DynamicCompilation::compile(ci, algo, this)) return;

    clipper_ = algo->execute(cfieldhandle->mesh());
    do_clip_p = true;
  }

  // Update the widget.
  const BBox bbox = ifieldhandle->mesh()->get_bounding_box();
  if (first_pass_ && points_differ())
  {
    Point center(center_x_.get(), center_y_.get(), center_z_.get());
    Point right(right_x_.get(), right_y_.get(), right_z_.get());
    Point down(down_x_.get(), down_y_.get(), down_z_.get());
    Point in(in_x_.get(), in_y_.get(), in_z_.get());

    box_->SetScale(scale_.get());
    box_->SetPosition(center, right, down, in);

    GeomGroup *widget_group = scinew GeomGroup;
    widget_group->add(box_->GetWidget());

    GeometryOPort *ogport=0;
    ogport = (GeometryOPort*)get_oport("Selection Widget");
    widgetid_ = ogport->addObj(widget_group, "ClipFieldToFieldOrWidget Selection Widget",
			       &widget_lock_);
    ogport->flushViews();

    last_bounds_ = bbox;
    // Force clipper to sync with new widget.
    if (clipper_.get_rep() && !clipper_->mesh_p()) { clipper_ = 0; }
    first_pass_ = false;
  }
  else if (!bbox_similar_to(last_bounds_, bbox) || exec_mode_.get() == "reset")
  {
    Point bmin = bbox.min();
    Point bmax = bbox.max();

    // Fix degenerate boxes.
    const double size_estimate = Max((bmax-bmin).length() * 0.01, 1.0e-5);
    if (fabs(bmax.x() - bmin.x()) < 1.0e-6)
    {
      bmin.x(bmin.x() - size_estimate);
      bmax.x(bmax.x() + size_estimate);
    }
    if (fabs(bmax.y() - bmin.y()) < 1.0e-6) 
    {
      bmin.y(bmin.y() - size_estimate);
      bmax.y(bmax.y() + size_estimate);
    }
    if (fabs(bmax.z() - bmin.z()) < 1.0e-6)
    {
      bmin.z(bmin.z() - size_estimate);
      bmax.z(bmax.z() + size_estimate);
    }

    const Point center = bmin + Vector(bmax - bmin) * 0.25;
    const Point right = center + Vector((bmax.x()-bmin.x())/4.0, 0, 0);
    const Point down = center + Vector(0, (bmax.y()-bmin.y())/4.0, 0);
    const Point in = center + Vector(0, 0, (bmax.z()-bmin.z())/4.0);

    const double l2norm = (bmax - bmin).length();

    box_->SetScale(l2norm * 0.015);
    box_->SetPosition(center, right, down, in);

    GeomGroup *widget_group = scinew GeomGroup;
    widget_group->add(box_->GetWidget());

    GeometryOPort *ogport=0;
    ogport = (GeometryOPort*)get_oport("Selection Widget");
    widgetid_ = ogport->addObj(widget_group, "ClipFieldToFieldOrWidget Selection Widget",
			       &widget_lock_);
    ogport->flushViews();

    last_bounds_ = bbox;
    // Force clipper to sync with new widget.
    if (clipper_.get_rep() && !clipper_->mesh_p()) { clipper_ = 0; }
    first_pass_ = false;
  }

  if (!clipper_.get_rep())
  {
    clipper_ = box_->get_clipper();
    do_clip_p = true;
  }
  else if (exec_mode_.get() == "execute" && !clipper_->mesh_p())
  {
    undo_stack_.push(clipper_);
    ClipperHandle ctmp = box_->get_clipper();
    if (clip_mode_.get() == "intersect")
    {
      clipper_ = scinew IntersectionClipper(ctmp, clipper_);
    }
    else if (clip_mode_.get() == "union")
    {
      clipper_ = scinew UnionClipper(ctmp, clipper_);
    }
    else if (clip_mode_.get() == "remove")
    {
      ctmp = scinew InvertClipper(ctmp);
      clipper_ = scinew IntersectionClipper(ctmp, clipper_);
    }
    else
    {
      clipper_ = ctmp;
    }
    do_clip_p = true;
  }
  else if (exec_mode_.get() == "invert")
  {
    undo_stack_.push(clipper_);
    clipper_ = scinew InvertClipper(clipper_);
    do_clip_p = true;
  }
  else if (exec_mode_.get() == "undo")
  {
    if (!undo_stack_.empty())
    {
      clipper_ = undo_stack_.top();
      undo_stack_.pop();
      do_clip_p = true;
    }
  }
  else if (exec_mode_.get() == "location")
  {
    do_clip_p = true;
  }

  if (do_clip_p || ifieldhandle->generation != last_input_generation_ ||
      !ofield_.get_rep())
  {
    last_input_generation_ = ifieldhandle->generation;
    exec_mode_.set("");

    const TypeDescription *ftd = ifieldhandle->get_type_description();
    CompileInfoHandle ci = ClipFieldToFieldOrWidgetAlgo::get_compile_info(ftd);
    Handle<ClipFieldToFieldOrWidgetAlgo> algo;
    if (!DynamicCompilation::compile(ci, algo, this)) return;

    // Maybe invert the clipper again.
    ClipperHandle clipper(clipper_);
    if (autoinvert_.get())
    {
      clipper = scinew InvertClipper(clipper_);
    }

    // Do the clip, dispatch based on which clip location test we are using.
    ofield_ = 0;
    if (clip_location_.get() == "nodeone")
    {
      ofield_ = algo->execute_node(this, ifieldhandle, clipper, true);
    }
    else if (clip_location_.get() == "nodeall")
    {
      ofield_ = algo->execute_node(this, ifieldhandle, clipper, false);
    }
    else // 'cell' and default
    {
      ofield_ = algo->execute_cell(this, ifieldhandle, clipper);
    }
  }

  send_output_handle("Output Field", ofield_, true);
}


void
ClipFieldToFieldOrWidget::widget_moved(bool last, BaseWidget*)
{
  if (last)
  {
    if (!first_pass_) {
      Point center, right, down, in;
      box_->GetPosition(center, right, down, in);
      center_x_.set(center.x());
      center_y_.set(center.y());
      center_z_.set(center.z());
      right_x_.set(right.x());
      right_y_.set(right.y());
      right_z_.set(right.z());
      down_x_.set(down.x());
      down_y_.set(down.y());
      down_z_.set(down.z());
      in_x_.set(in.x());
      in_y_.set(in.y());
      in_z_.set(in.z());
      scale_.set(box_->GetScale());
    }
    autoexec_.reset();
    if (autoexec_.get())
    {
      exec_mode_.set("execute");
      want_to_execute();
    }
  } 
}



CompileInfoHandle
ClipFieldToFieldOrWidgetAlgo::get_compile_info(const TypeDescription *fsrc)
{
  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string template_class_name("ClipFieldToFieldOrWidgetAlgoT");
  static const string base_class_name("ClipFieldToFieldOrWidgetAlgo");

  CompileInfo *rval = 
    scinew CompileInfo(template_class_name + "." +
		       fsrc->get_filename() + ".",
                       base_class_name, 
                       template_class_name, 
                       fsrc->get_name());

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  fsrc->fill_compile_info(rval);
  return rval;
}


CompileInfoHandle
ClipFieldToFieldOrWidgetMeshAlgo::get_compile_info(const TypeDescription *fsrc)
{
  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string template_class_name("ClipFieldToFieldOrWidgetMeshAlgoT");
  static const string base_class_name("ClipFieldToFieldOrWidgetMeshAlgo");

  CompileInfo *rval = 
    scinew CompileInfo(template_class_name + "." +
		       fsrc->get_filename() + ".",
                       base_class_name, 
                       template_class_name, 
                       fsrc->get_name());

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  fsrc->fill_compile_info(rval);
  return rval;
}



} // End namespace SCIRun

