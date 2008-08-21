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


#include <Core/Algorithms/Fields/TransformMesh/GeneratePolarProjection.h>

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <Dataflow/Widgets/ArrowWidget.h>
#include <Core/Thread/CrowdMonitor.h>

#include <Dataflow/Network/Ports/GeometryPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>


namespace SCIRun {

using namespace SCIRun;

class GeneratePolarProjection : public Module {
  public:
    GeneratePolarProjection(GuiContext*);
    virtual ~GeneratePolarProjection() {}

    virtual void execute();
    
    virtual void widget_moved(bool release, BaseWidget*);
  private:  
    SCIRunAlgo::GeneratePolarProjectionAlgo algo_;
    
    CrowdMonitor             widget_lock_;   
    ArrowWidget              widget_; 
    GeomHandle               widget_switch_;
    int                      widget_id_;
    
    GuiDouble dx_,dy_,dz_,px_,py_,pz_;
    
    Point                    origin_;
    Vector                   direction_;

    Point                    old_origin_;
    Vector                   old_direction_;
};


DECLARE_MAKER(GeneratePolarProjection)

GeneratePolarProjection::GeneratePolarProjection(GuiContext* ctx) :
  Module("GeneratePolarProjection", ctx, Source, "ChangeMesh", "SCIRun"),
  widget_lock_("Arrow Widget Lock"),
  widget_(this,&widget_lock_,1.0),
  dx_(get_ctx()->subVar("dx"),0.0),
  dy_(get_ctx()->subVar("dy"),0.0),
  dz_(get_ctx()->subVar("dz"),10.0),
  px_(get_ctx()->subVar("px"),0.0),
  py_(get_ctx()->subVar("py"),0.0),
  pz_(get_ctx()->subVar("pz"),0.0)
{
  algo_.set_progress_reporter(this);
  get_ctx()->reset();

  widget_.SetScale(5);
  widget_.SetLength(25);
  double dx = dx_.get();
  double dy = dy_.get();
  double dz = dz_.get();
  double px = px_.get();
  double py = py_.get();
  double pz = pz_.get();
  
  widget_.SetPosition(Point(px,py,pz));
  widget_.SetDirection(Vector(dx,dy,dz));
  widget_switch_ = widget_.GetWidget();
  GeometryOPortHandle geom_oport;
  get_oport_handle("ProjectionVector",geom_oport);
  ((GeomSwitch*)widget_switch_.get_rep())->set_state(1);
  widget_id_ = geom_oport->addObj(widget_switch_,"ProjectionVector",&widget_lock_);
  geom_oport->flushViews();
  
  origin_ = Point(px,py,pz);
  direction_ = Vector(dx,dy,dz);

  old_direction_ = direction_;
  old_origin_ = origin_;
}

void
GeneratePolarProjection::execute()
{
  // Troubles in setup, since initialization runs through TCL....
  // Cannot access these in constructor hence need to do them here
  double dx = dx_.get();
  double dy = dy_.get();
  double dz = dz_.get();
  double px = px_.get();
  double py = py_.get();
  double pz = pz_.get();
  widget_.SetPosition(Point(px,py,pz));
  widget_.SetDirection(Vector(dx,dy,dz));
  origin_ = Point(px,py,pz);
  direction_ = Vector(dx,dy,dz);

  FieldHandle input, output;
  get_input_handle("Mesh",input,true);
  
  MatrixHandle origin, direction;
  get_input_handle("Origin",origin,false);
  get_input_handle("Direction",direction,false);
  
  if (origin.get_rep() && origin->get_data_size() > 2)
  {
    double *data = origin->get_data_pointer();
    origin_ = Point(data[0],data[1],data[2]);
    widget_.SetPosition(origin_);
  }
  
  if (direction.get_rep() && direction->get_data_size() > 2)
  {
    double *data = direction->get_data_pointer();
    direction_ = Vector(data[0],data[1],data[2]);
    widget_.SetDirection(direction_);
  }
  
  if (inputs_changed_ || origin_ != old_origin_ ||
      direction_ != old_direction_ || !oport_cached("ProjectedMesh")
      || !oport_cached("Origin") || !oport_cached("Direction"))
  {
    algo_.set_point("origin",origin_);
    algo_.set_vector("direction",direction_);
    algo_.run(input,output);
    
    old_origin_ = origin_;
    old_direction_ = direction_;
    send_output_handle("ProjectedMesh",output,true);
    
    origin = new DenseMatrix(3,1);
    double* data = origin->get_data_pointer();
    data[0] = origin_.x(); data[1] = origin_.y(); data[2] = origin_.z();
    send_output_handle("Origin",origin,true);

    direction = new DenseMatrix(3,1);
    data = direction->get_data_pointer();
    data[0] = direction_.x(); data[1] = direction_.y(); data[2] = direction_.z();
    send_output_handle("Direction",direction,true);    
  }
}

void
GeneratePolarProjection::widget_moved(bool release, BaseWidget*)
{
  if (release) 
  {
    Point  p = widget_.GetPosition();
    Vector d = widget_.GetDirection();
    
    px_.set(p.x()); py_.set(p.y()); pz_.set(p.z());
    dx_.set(d.x()); dy_.set(d.y()); dz_.set(d.z());
    
    want_to_execute();
    origin_ = p;
    direction_ = d;
  }
}

} // End namespace SCIRun


