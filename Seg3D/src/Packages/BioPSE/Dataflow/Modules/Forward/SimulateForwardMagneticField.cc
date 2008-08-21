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

#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/FieldInformation.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>

namespace BioPSE {

using namespace SCIRun;

class CalcFMField
{
  public:

    CalcFMField() :
      np_(-1)
    {}
  
    virtual bool calc_forward_magnetic_field(FieldHandle efield, 
					   FieldHandle ctfield,
					   FieldHandle dipoles, 
					   FieldHandle detectors, 
					   FieldHandle &magnetic_field, 
					   FieldHandle &magnitudes,
					   ProgressReporter *mod);

  private:
    void interpolate(int proc, Point p);
    void set_up_cell_cache();
    void calc_parallel(int proc, ProgressReporter *mod);

    int np_;
    vector<Vector> interp_value_;
    vector<pair<string, Tensor> > tens_;
    bool have_tensors_;

    struct per_cell_cache {
      Vector cur_density_;
      Point  center_;
      double volume_;
    };

    std::vector<per_cell_cache>  cell_cache_;

    VField* efld_; // Electric Field
    VField* ctfld_; // Conductivity Field
    VField* dipfld_; // Dipole Field
    VField* detfld_; // Detector Field

    VMesh* emsh_; // Electric Field
    VMesh* ctmsh_; // Conductivity Field
    VMesh* dipmsh_; // Dipole Field
    VMesh* detmsh_; // Detector Field

    VField* magfld_; // Magnetic Field
    VField* magmagfld_; // Magnetic Field Magnitudes
};


void
CalcFMField::calc_parallel(int proc, ProgressReporter *mod)
{
  VMesh::size_type num_nodes = detmsh_->num_nodes();
  VMesh::size_type nodes_per_thread = num_nodes/np_;
  
  VMesh::Node::index_type start = proc*nodes_per_thread;
  VMesh::Node::index_type end = (proc+1)*nodes_per_thread;
  if (proc == (np_-1)) end = num_nodes;

  Vector mag_field;
  Point  pt;
  Point  pt2;
  Vector P;
  const double one_over_4_pi = 1.0 / (4 * M_PI);

  VMesh::size_type num_dipoles = dipmsh_->num_nodes();

  int cnt = 0;
  for (VMesh::Node::index_type idx = start; idx < end; idx++ )
  {
    // finish loop iteration.
    
    detmsh_->get_center(pt, idx);

    // init the interp val to 0 
    interp_value_[proc] = Vector(0,0,0);
    interpolate(proc, pt);

    mag_field = interp_value_[proc];

    Vector normal;
    detfld_->get_value(normal,idx); 

    // iterate over the dipoles.
    for (VMesh::Node::index_type dip_idx = 0; dip_idx < num_dipoles; dip_idx++)
    {
      dipmsh_->get_center(pt2, dip_idx);
      dipfld_->value(P,dip_idx);
      
      Vector radius = pt - pt2; // detector - source
      Vector valuePXR = Cross(P, radius);
      double length = radius.length();
 
      mag_field += valuePXR / (length * length * length);
    }
    
    mag_field *= one_over_4_pi;
    magmagfld_->set_value(Dot(mag_field, normal),idx);
    magfld_->set_value(mag_field,idx);

    if (proc == 0)
    {
      cnt++; if (cnt == 100) { cnt = 0; mod->update_progress(idx,end); }
    }
  }
}

//! Assume that the value_type for the input fields are Vector.
bool
CalcFMField::calc_forward_magnetic_field(
              FieldHandle efield, 
					    FieldHandle ctfield, 
					    FieldHandle dipoles, 
					    FieldHandle detectors, 
					    FieldHandle &magnetic_field,
					    FieldHandle &magnetic_field_magnitudes,
					    ProgressReporter *mod)
{
  efld_ = efield->vfield();
  ctfld_ = ctfield->vfield();
  dipfld_ = dipoles->vfield();
  detfld_ = detectors->vfield();
  emsh_ = efield->vmesh();
  ctmsh_ = ctfield->vmesh();
  dipmsh_ = dipoles->vmesh();
  detmsh_ = detectors->vmesh();


  // this code should be able to handle Field<Tensor> as well
  have_tensors_ = ctfld_->get_property("conductivity_table", tens_); 

  FieldInformation mfi(detectors);
  mfi.make_lineardata();

  mfi.make_double();
  magnetic_field_magnitudes = CreateField(mfi,detectors->mesh());
  if (magnetic_field_magnitudes.get_rep() == 0) 
  {
    mod->error("Could not allocate field for magnetic field magnitudes");
    return (false);
  }


  magmagfld_ = magnetic_field_magnitudes->vfield();
  magmagfld_->resize_values();

  mfi.make_vector();  
  magnetic_field = CreateField(mfi,detectors->mesh());
  if (magnetic_field.get_rep() == 0) 
  {
    mod->error("Could not allocate field for magnetic field");
    return (false);
  }

  magfld_ = magnetic_field->vfield();
  magfld_->resize_values();

  // Make sure we have more than zero threads
  np_ = Thread::numProcessors();
  interp_value_.resize(np_,Vector(0.0,0.0,0.0));

  // cache per cell calculations that are used over and over again.
  set_up_cell_cache();
  
  // do the parallel work.
  Thread::parallel(this, &CalcFMField::calc_parallel, np_, mod);
  
  return (true);
}

void
CalcFMField::set_up_cell_cache()
{
  VMesh::size_type num_elems = emsh_->num_elems();
  Vector elemField;
  cell_cache_.resize(num_elems);
  
  if (have_tensors_)
  {
    int material = -1;
    for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
    {
      per_cell_cache c;
      emsh_->get_center(c.center_,idx);
      efld_->get_value(elemField,idx);
      ctfld_->get_value(material,idx);
   
      c.cur_density_ = tens_[material].second * -1 * elemField; 
      c.volume_ = emsh_->get_volume(idx);
      cell_cache_[idx] = c;       
    }
  }
  else if (ctfld_->is_tensor())
  {
    Tensor ten;
    for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
    {
      per_cell_cache c;
      emsh_->get_center(c.center_,idx);
      efld_->get_value(elemField,idx);
      ctfld_->get_value(ten,idx);
   
      c.cur_density_ = ten * -1 * elemField; 
      c.volume_ = emsh_->get_volume(idx);
      cell_cache_[idx] = c;       
    }  
  }
  else
  {
    double val;
    for (VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
    {
      per_cell_cache c;
      emsh_->get_center(c.center_,idx);
      efld_->get_value(elemField,idx);
      ctfld_->get_value(val,idx);
   
      c.cur_density_ = val * -1 * elemField; 
      c.volume_ = emsh_->get_volume(idx);
      cell_cache_[idx] = c;       
    }    
  }
}

void
CalcFMField::interpolate(int proc, Point p)  
{
  emsh_->synchronize(Mesh::ELEM_LOCATE_E);

  VMesh::Elem::index_type inside_cell = 0;
  bool outside = !(emsh_->locate(inside_cell, p));

  VMesh::size_type num_elems = emsh_->num_elems();

  for (VMesh::Elem::index_type idx; idx<num_elems; idx++)
  {    
    if (outside || idx != inside_cell) 
    {
      per_cell_cache &c = cell_cache_[idx];
      Vector radius = p - c.center_;
      
      Vector valueJXR = Cross(c.cur_density_, radius);
      double length = radius.length();

      interp_value_[proc] += ((valueJXR / (length * length * length)) * c.volume_); 
    }
  }
}


class SimulateForwardMagneticField : public Module {
public:
  SimulateForwardMagneticField(GuiContext *context);
  virtual ~SimulateForwardMagneticField() {}

  virtual void execute();
};


DECLARE_MAKER(SimulateForwardMagneticField)

SimulateForwardMagneticField::SimulateForwardMagneticField(GuiContext* ctx)
  : Module("SimulateForwardMagneticField", ctx, Source, "Forward", "BioPSE")
{
}

void
SimulateForwardMagneticField::execute()
{
// J = (sigma)*E + J(source)
  FieldHandle efld;
  get_input_handle("Electric Field", efld);

  if (efld->vfield()->is_vector() == false) 
  {
    error("Must have Vector field as Electric Field input");
    return;
  }

  FieldHandle ctfld;
  get_input_handle("Conductivity Tensors", ctfld);

  FieldHandle dipoles;
  get_input_handle("Dipole Sources", dipoles);

  if (dipoles->vfield()->is_vector() == false) 
  {
    error("Must have Vector field as Dipole Sources input");
    return;
  }
  
  FieldHandle detectors;
  get_input_handle("Detector Locations", detectors);
  
  if (detectors->vfield()->is_vector() == false) 
  {
    error("Must have Vector field as Detector Locations input");
    return;
  }

  FieldHandle magnetic_field;
  FieldHandle magnitudes;

  CalcFMField algo;
  
  if (! algo.calc_forward_magnetic_field(efld, ctfld, dipoles, detectors, 
					  magnetic_field, magnitudes, this)) 
  {
    return;
  }
  
  send_output_handle("Magnetic Field", magnetic_field);  
  send_output_handle("Magnitudes", magnitudes);
}

} // End namespace BioPSE

