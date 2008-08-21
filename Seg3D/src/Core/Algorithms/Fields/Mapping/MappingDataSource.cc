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

#include <Core/Math/MiscMath.h>
#include <Core/Algorithms/Fields/Mapping/MappingDataSource.h>

namespace SCIRunAlgo {

using namespace SCIRun;

// Constructor of base class
MappingDataSource::MappingDataSource() :
  ref_cnt(0), lock("DataSource Mutex"),
  is_double_(false), is_vector_(false), is_tensor_(false) 
{}

// Default implementations

void 
MappingDataSource::get_data(double& data, Point& p)
{ ASSERTFAIL("get_data(double) was not implemented"); }

void 
MappingDataSource::get_data(Vector& data, Point& p)
{ ASSERTFAIL("get_data(Vector) was not implemented"); }

void 
MappingDataSource::get_data(Tensor& data, Point& p)
{ ASSERTFAIL("get_data(Tensor) was not implemented"); }

void 
MappingDataSource::get_data(std::vector<double>& data, std::vector<Point>& p)
{ ASSERTFAIL("get_data(std::vector<double>) was not implemented"); }

void 
MappingDataSource::get_data(std::vector<Vector>& data, std::vector<Point>& p)
{ ASSERTFAIL("get_data(std::vector<Vector>) was not implemented"); }

void 
MappingDataSource::get_data(std::vector<Tensor>& data, std::vector<Point>& p)
{ ASSERTFAIL("get_data(std::vector<Tensor>) was not implemented"); }


bool
MappingDataSource::is_scalar()
{ return (is_double_); }

bool
MappingDataSource::is_double()
{ return (is_double_); }

bool
MappingDataSource::is_vector()
{ return (is_vector_); }

bool
MappingDataSource::is_tensor()
{ return (is_tensor_); }

// DataSource code

// InterpolateData: find the data through interpolation

class InterpolatedDataSource : public MappingDataSource {
  public:
    virtual void get_data(double& data, Point& p)
    {
      sfield_->interpolate(data,p,def_value_);
    }

    virtual void get_data(Vector& data, Point& p)
    {
      sfield_->interpolate(data,p,Vector(0.0,0.0,0.0));
    }

    virtual void get_data(Tensor& data, Point& p)
    {
      sfield_->interpolate(data,p,Tensor(0.0));
    }
    
    virtual void get_datas(std::vector<double>& data, std::vector<Point>& p)
    {
      sfield_->minterpolate(data,p,def_value_);
    }

    virtual void get_datas(std::vector<Vector>& data, std::vector<Point>& p)
    {
      sfield_->minterpolate(data,p,Vector(0.0,0.0,0.0));
    }
   
    virtual void get_datas(std::vector<Tensor>& data, std::vector<Point>& p)
    {
      sfield_->minterpolate(data,p,Tensor(0.0));
    }
        
    InterpolatedDataSource(FieldHandle& sfield,double def_value)
    {
      sfield_ = sfield->vfield();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      def_value_ = def_value;
      
      if (sfield_->is_scalar()) is_double_ = true;
      if (sfield_->is_vector()) is_vector_ = true;
      if (sfield_->is_tensor()) is_tensor_ = true;
    }

  private:
    VField *sfield_;
    double def_value_;
};

class InterpolatedWeightedDataSource : public MappingDataSource {
  public:
    virtual void get_data(double& data, Point& p)
    {
      double weight; wfield_->interpolate(weight,p,0.0);
      sfield_->interpolate(data,p,def_value_);
      data = data*weight;
    }

    virtual void get_data(Vector& data, Point& p)
    {
      double weight; wfield_->interpolate(weight,p,0.0);
      sfield_->interpolate(data,p,Vector(0.0,0.0,0.0));
      data = data*weight;
    }

    virtual void get_data(Tensor& data, Point& p)
    {
      double weight; wfield_->interpolate(weight,p,0.0);
      sfield_->interpolate(data,p,Tensor(def_value_));
      data = data*weight;
    }
    
    virtual void get_datas(std::vector<double>& data, std::vector<Point>& p)
    {
      wfield_->minterpolate(weights_,p,0.0);
      sfield_->minterpolate(data,p,def_value_);
      for (size_t j=0; j<weights_.size(); j++) data[j] = weights_[j]*data[j];
    }

    virtual void get_datas(std::vector<Vector>& data, std::vector<Point>& p)
    {
      wfield_->minterpolate(weights_,p,0.0);
      sfield_->minterpolate(data,p,Vector(0.0,0.0,0.0));
      for (size_t j=0; j<weights_.size(); j++) data[j] = weights_[j]*data[j];
    }
   
    virtual void get_datas(std::vector<Tensor>& data, std::vector<Point>& p)
    {
      wfield_->minterpolate(weights_,p,0.0);
      sfield_->minterpolate(data,p,Tensor(def_value_));
      for (size_t j=0; j<weights_.size(); j++) data[j] = weights_[j]*data[j];
    }
        
    InterpolatedWeightedDataSource(FieldHandle& sfield,FieldHandle& wfield, double def_value)
    {
      sfield_ = sfield->vfield();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      wfield_ = wfield->vfield();
      wfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      def_value_ = def_value;

      if (sfield_->is_scalar()) is_double_ = true;
      if (sfield_->is_vector()) is_vector_ = true;
      if (sfield_->is_tensor()) is_tensor_ = true;
    }

  private:
    VField *sfield_;
    VField *wfield_;
    double def_value_;
    
    // Stored here we do not need to allocate this one each time
    // Its memory is allocated on first use
    std::vector<double> weights_;
};


class InterpolatedWeightedTensorDataSource : public MappingDataSource {
  public:
    virtual void get_data(Vector& data, Point& p)
    {
      Tensor weight; wfield_->interpolate(weight,p,Tensor(0.0));
      sfield_->interpolate(data,p,Vector(0.0,0.0,0.0));
      data = weight*data;
    }

    virtual void get_data(Tensor& data, Point& p)
    {
      wfield_->interpolate(data,p,Tensor(0.0));
      double tdata; sfield_->interpolate(tdata,p,def_value_);
      data = data*tdata;
    }
    
    virtual void get_datas(std::vector<Vector>& data, std::vector<Point>& p)
    {
      wfield_->minterpolate(weights_,p,Tensor(0.0));
      sfield_->minterpolate(data,p,Vector(0.0,0.0,0.0));
      for (size_t j=0; j<weights_.size(); j++) data[j] = weights_[j]*data[j];
    }
   
    virtual void get_datas(std::vector<Tensor>& data, std::vector<Point>& p)
    {
      wfield_->minterpolate(data,p,Tensor(0.0));
      sfield_->minterpolate(tdata_,p,def_value_);
      for (size_t j=0; j<weights_.size(); j++) data[j] = tdata_[j]*data[j];
    }
        
    InterpolatedWeightedTensorDataSource(FieldHandle& sfield,FieldHandle& wfield, double def_value)
    {
      sfield_ = sfield->vfield();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      wfield_ = wfield->vfield();
      wfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      def_value_ = def_value;
      
      if (sfield_->is_scalar()) is_tensor_ = true;
      if (sfield_->is_vector()) is_vector_ = true;
    }

  private:
    VField *sfield_;
    VField *wfield_;
    double def_value_;
    
    // Stored here we do not need to allocate this one each time
    // Its memory is allocated on first use
    std::vector<double> tdata_;
    std::vector<Tensor> weights_;
};

class InterpolatedGradientSource : public MappingDataSource {
  public:
    virtual void get_data(Vector& data, Point& p)
    {
      // use doubles to avoid quantization effects
      StackVector<double,3> grad;
      sfield_->gradient(grad,p);
      data = Vector(grad[0],grad[1],grad[2]);
    }

    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p)
    {
      sfield_->mgradient(grads_,p);
      data.resize(grads_.size());
      for (size_t j=0; j<data.size();j++)
        data[j] = Vector(grads_[j][0],grads_[j][1],grads_[j][2]);
    }

    InterpolatedGradientSource(FieldHandle& sfield)
    {
      sfield_ = sfield->vfield();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      
      is_vector_ = true;
    }

  private:
    VField *sfield_;
    
    // temp variables that need allocation for first tine use
    std::vector<StackVector<double,3> > grads_; 
};


class InterpolatedWeightedGradientSource : public MappingDataSource {
  public:
    virtual void get_data(Vector& data, Point& p)
    {
      // use doubles to avoid quantization effects
      StackVector<double,3> grad;
      double weight;
      wfield_->interpolate(weight,p,0.0);
      sfield_->gradient(grad,p);
      data = weight*Vector(grad[0],grad[1],grad[2]);
    }

    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p)
    {
      wfield_->minterpolate(weights_,p,0.0);
      sfield_->mgradient(grads_,p);
      data.resize(grads_.size());
      for (size_t j=0; j<data.size();j++)
        data[j] = weights_[j]*Vector(grads_[j][0],grads_[j][1],grads_[j][2]);
    }

    InterpolatedWeightedGradientSource(FieldHandle& sfield, FieldHandle& wfield)
    {
      sfield_ = sfield->vfield();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      wfield_ = wfield->vfield();
      wfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      
      is_vector_ = true;
    }

  private:
    VField *sfield_;
    VField *wfield_;
    
    // temp variables that need allocation for first tine use
    std::vector<StackVector<double,3> > grads_; 
    std::vector<double> weights_;
};

class InterpolatedWeightedTensorGradientSource : public MappingDataSource {
  public:
    virtual void get_data(Vector& data, Point& p)
    {
      // use doubles to avoid quantization effects
      StackVector<double,3> grad;
      Tensor weight;
      wfield_->interpolate(weight,p,Tensor(0.0));
      sfield_->gradient(grad,p);
      data = weight*Vector(grad[0],grad[1],grad[2]);
    }

    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p)
    {
      wfield_->minterpolate(weights_,p,0.0);
      sfield_->mgradient(grads_,p);
      data.resize(grads_.size());
      for (size_t j=0; j<data.size();j++)
        data[j] = weights_[j]*Vector(grads_[j][0],grads_[j][1],grads_[j][2]);
    }

    InterpolatedWeightedTensorGradientSource(FieldHandle& sfield, FieldHandle& wfield)
    {
      sfield_ = sfield->vfield();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      wfield_ = wfield->vfield();
      wfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      
      is_vector_ = true;
    }

  private:
    VField *sfield_;
    VField *wfield_;
    
    // temp variables that need allocation for first tine use
    std::vector<StackVector<double,3> > grads_; 
    std::vector<Tensor> weights_;
};

class InterpolatedGradientNormSource : public MappingDataSource {
  public:
    virtual void get_data(double& data, Point& p)
    {
      // use doubles to avoid quantization effects
      StackVector<double,3> grad;
      sfield_->gradient(grad,p);
      data = (Vector(grad[0],grad[1],grad[2])).length();
    }

    virtual void get_data(std::vector<double>& data, std::vector<Point>& p)
    {
      sfield_->mgradient(grads_,p);
      data.resize(grads_.size());
      for (size_t j=0; j<data.size();j++)
        data[j] = (Vector(grads_[j][0],grads_[j][1],grads_[j][2])).length();
    }

    InterpolatedGradientNormSource(FieldHandle& sfield)
    {
      sfield_ = sfield->vfield();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      
      is_double_ = true;
    }

  private:
    VField *sfield_;
    
    // temp variables that need allocation for first tine use
    std::vector<StackVector<double,3> > grads_; 
};


class InterpolatedWeightedGradientNormSource : public MappingDataSource {
  public:
    virtual void get_data(double& data, Point& p)
    {
      // use doubles to avoid quantization effects
      StackVector<double,3> grad;
      double weight;
      wfield_->interpolate(weight,p,0.0);
      sfield_->gradient(grad,p);
      data = (weight*Vector(grad[0],grad[1],grad[2])).length();
    }

    virtual void get_data(std::vector<double>& data, std::vector<Point>& p)
    {
      sfield_->mgradient(grads_,p);
      wfield_->minterpolate(weights_,p,0.0);
      data.resize(grads_.size());
      for (size_t j=0; j<data.size();j++)
        data[j] = (weights_[j]*Vector(grads_[j][0],grads_[j][1],grads_[j][2])).length();
    }

    InterpolatedWeightedGradientNormSource(FieldHandle& sfield, FieldHandle& wfield)
    {
      sfield_ = sfield->vfield();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      wfield_ = wfield->vfield();
      wfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      
      is_double_ = true;
    }

  private:
    VField *sfield_;
    VField *wfield_;
    
    // temp variables that need allocation for first tine use
    std::vector<StackVector<double,3> > grads_; 
    std::vector<double> weights_;
};

class InterpolatedWeightedTensorGradientNormSource : public MappingDataSource {
  public:
    virtual void get_data(double& data, Point& p)
    {
      // use doubles to avoid quantization effects
      StackVector<double,3> grad;
      Tensor weight;
      wfield_->interpolate(weight,p,Tensor(0.0));
      sfield_->gradient(grad,p);
      data = (weight*Vector(grad[0],grad[1],grad[2])).length();
    }

    virtual void get_data(std::vector<double>& data, std::vector<Point>& p)
    {
      sfield_->mgradient(grads_,p);
      wfield_->minterpolate(weights_,p,0.0);
      data.resize(grads_.size());
      for (size_t j=0; j<data.size();j++)
        data[j] = (weights_[j]*Vector(grads_[j][0],grads_[j][1],grads_[j][2])).length();
    }

    InterpolatedWeightedTensorGradientNormSource(FieldHandle& sfield, FieldHandle& wfield)
    {
      sfield_ = sfield->vfield();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      wfield_ = wfield->vfield();
      wfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E);
      
      is_double_ = true;
    }

  private:
    VField *sfield_;
    VField *wfield_;
    
    // temp variables that need allocation for first tine use
    std::vector<StackVector<double,3> > grads_; 
    std::vector<Tensor> weights_;
};


class ClosestInterpolatedDataSource : public MappingDataSource {
  public:
    virtual void get_data(double& data, Point& p)
    {
      if(!(sfield_->interpolate(data,p,def_value_)))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        smesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          sfield_->interpolate(data,coords,elem);
        }
        else
        {
          data = def_value_;
        }
      }
    }

    virtual void get_data(Vector& data, Point& p)
    {
      if(!(sfield_->interpolate(data,p)))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        smesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          sfield_->interpolate(data,coords,elem);
        }
        else
        {
          data = Vector(0.0,0.0,0.0);
        }
      }
    }

    virtual void get_data(Tensor& data, Point& p)
    {
      if(!(sfield_->interpolate(data,p,Tensor(def_value_))))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        smesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          sfield_->interpolate(data,coords,elem);
        }
        else
        {
          data = Tensor(def_value_);
        }
      }
    }

    virtual void get_data(std::vector<double>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
      {
        if(!(sfield_->interpolate(data[j],p[j],def_value_)))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          smesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            sfield_->interpolate(data[j],coords,elem);
          }
          else
          {
            data[j] = def_value_;
          }
        }
      }
    }

    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
      {
        if(!(sfield_->interpolate(data[j],p[j],Vector(0,0,0))))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          smesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            sfield_->interpolate(data[j],coords,elem);
          }
          else
          {
            data[j] = Vector(0.0,0.0,0.0);
          }
        }
      }
    }

    virtual void get_data(std::vector<Tensor>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
      {
        if(!(sfield_->interpolate(data[j],p[j],Tensor(def_value_))))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          smesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            sfield_->interpolate(data[j],coords,elem);
          }
          else
          {
            data[j] = Tensor(def_value_);
          }
        }
      }
    }
    
    ClosestInterpolatedDataSource(FieldHandle& sfield,double def_value,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E);
      def_value_ = def_value;
      maxdist_ = max_dist;

      if (sfield_->is_scalar()) is_double_ = true;
      if (sfield_->is_vector()) is_vector_ = true;
      if (sfield_->is_tensor()) is_tensor_ = true;
    }

  private:
    double  maxdist_;
    VField *sfield_;
    VMesh  *smesh_;
    double def_value_;
};

class ClosestInterpolatedWeightedDataSource : public MappingDataSource {
  public:
    virtual void get_data(double& data, Point& p)
    {
      double weight;
      if(!(wfield_->interpolate(weight,p,0.0)))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        wmesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          wfield_->interpolate(weight,coords,elem);
        }
        else
        {
          weight = 0.0;
        }
      }    
      if(!(sfield_->interpolate(data,p,def_value_)))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        smesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          sfield_->interpolate(data,coords,elem);
        }
        else
        {
          data = def_value_;
        }
      }
      data = weight*data;
    }

    virtual void get_data(Vector& data, Point& p)
    {
      double weight;
      if(!(wfield_->interpolate(weight,p,0.0)))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        wmesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          wfield_->interpolate(weight,coords,elem);
        }
        else
        {
          weight = 0.0;
        }
      } 
      if(!(sfield_->interpolate(data,p,Vector(0.0,0.0,0.0))))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        smesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          sfield_->interpolate(data,coords,elem);
        }
        else
        {
          data = Vector(0.0,0.0,0.0);
        }
      }
      data = weight * data;
    }

    virtual void get_data(Tensor& data, Point& p)
    {
      double weight;
      if(!(wfield_->interpolate(weight,p,0.0)))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        wmesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          wfield_->interpolate(weight,coords,elem);
        }
        else
        {
          weight = 0.0;
        }
      }     
      if(!(sfield_->interpolate(data,p,Tensor(def_value_))))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        smesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          sfield_->interpolate(data,coords,elem);
        }
        else
        {
          data = Tensor(def_value_);
        }
      }
      data = weight* data;
    }

    virtual void get_data(std::vector<double>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
      {
        double weight;
        if(!(wfield_->interpolate(weight,p[j],0.0)))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          wmesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            wfield_->interpolate(weight,coords,elem);
          }
          else
          {
            weight = 0.0;
          }
        } 
        if(!(sfield_->interpolate(data[j],p[j],def_value_)))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          smesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            sfield_->interpolate(data[j],coords,elem);
          }
          else
          {
            data[j] = def_value_;
          }
        }
        data[j] = weight * data[j];
      }
    }

    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
      {
        double weight;
        if(!(wfield_->interpolate(weight,p[j],0.0)))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          wmesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            wfield_->interpolate(weight,coords,elem);
          }
          else
          {
            weight = 0.0;
          }
        } 
        if(!(sfield_->interpolate(data[j],p[j],Vector(0.0,0.0,0.0))))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          smesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            sfield_->interpolate(data[j],coords,elem);
          }
          else
          {
            data[j] = Vector(0.0,0.0,0.0);
          }
        }
        data[j] = weight * data[j];
      }
    }

    virtual void get_data(std::vector<Tensor>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
      {
        double weight;
        if(!(wfield_->interpolate(weight,p[j],0.0)))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          wmesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            wfield_->interpolate(weight,coords,elem);
          }
          else
          {
            weight = 0.0;
          }
        }       
        if(!(sfield_->interpolate(data[j],p[j],Tensor(def_value_))))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          smesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            sfield_->interpolate(data[j],coords,elem);
          }
          else
          {
            data[j] = Tensor(def_value_);
          }
        }
        data[j] = weight * data[j];
      }
    }
    
    ClosestInterpolatedWeightedDataSource(FieldHandle& sfield,FieldHandle& wfield,double& def_value,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E);
      wfield_ = wfield->vfield();
      wmesh_ =  wfield->vmesh();
      wfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E);
      def_value_ = def_value;
      maxdist_ = max_dist;

      if (sfield_->is_scalar()) is_double_ = true;
      if (sfield_->is_vector()) is_vector_ = true;
      if (sfield_->is_tensor()) is_tensor_ = true;
    }

  private:
    double  maxdist_;

    VField *sfield_;
    VMesh  *smesh_;
    VField *wfield_;
    VMesh  *wmesh_;

    double def_value_;
};

class ClosestInterpolatedWeightedTensorDataSource : public MappingDataSource {
  public:

    virtual void get_data(Vector& data, Point& p)
    {
      Tensor weight;
      if(!(wfield_->interpolate(weight,p,Tensor(0.0))))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        wmesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          wfield_->interpolate(weight,coords,elem);
        }
        else
        {
          weight = 0.0;
        }
      } 
      if(!(sfield_->interpolate(data,p,Vector(0.0,0.0,0.0))))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        smesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          sfield_->interpolate(data,coords,elem);
        }
        else
        {
          data = Vector(0.0,0.0,0.0);
        }
      }
      data = weight * data;
    }

    virtual void get_data(Tensor& data, Point& p)
    {
      if(!(wfield_->interpolate(data,p,Tensor(0.0))))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        wmesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          wfield_->interpolate(data,coords,elem);
        }
        else
        {
          data = 0.0;
        }
      }
      double tdata;
      if(!(sfield_->interpolate(tdata,p,def_value_)))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        smesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          sfield_->interpolate(data,coords,elem);
        }
        else
        {
          data = Tensor(def_value_);
        }
      }
      data = tdata* data;
    }

    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
      {
        Tensor weight;
        if(!(wfield_->interpolate(weight,p[j],Tensor(0.0))))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          wmesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            wfield_->interpolate(weight,coords,elem);
          }
          else
          {
            weight = 0.0;
          }
        } 
        if(!(sfield_->interpolate(data[j],p[j],Vector(0.0,0.0,0.0))))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          smesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            sfield_->interpolate(data[j],coords,elem);
          }
          else
          {
            data[j] = Vector(0.0,0.0,0.0);
          }
        }
        data[j] = weight * data[j];
      }
    }

    virtual void get_data(std::vector<Tensor>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
      {
        if(!(wfield_->interpolate(data[j],p[j],Tensor(0.0))))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          wmesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            wfield_->interpolate(data[j],coords,elem);
          }
          else
          {
             data[j] = 0.0;
          }
        }
        double tdata;
        if(!(sfield_->interpolate(tdata,p[j],def_value_)))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          smesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            sfield_->interpolate(tdata,coords,elem);
          }
          else
          {
            data[j] = Tensor(def_value_);
          }
        }
        data[j] = tdata * data[j];
      }
    }
    
    ClosestInterpolatedWeightedTensorDataSource(FieldHandle& sfield,FieldHandle& wfield,double& def_value,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E);
      wfield_ = wfield->vfield();
      wmesh_ =  wfield->vmesh();
      wfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E);
      def_value_ = def_value;
      maxdist_ = max_dist;

      if (sfield_->is_scalar()) is_tensor_ = true;
      if (sfield_->is_vector()) is_vector_ = true;
      if (sfield_->is_tensor()) is_tensor_ = true;
    }

  private:
    double  maxdist_;

    VField *sfield_;
    VMesh  *smesh_;
    VField *wfield_;
    VMesh  *wmesh_;

    double def_value_;
};

 
class ClosestInterpolatedGradientSource : public MappingDataSource {
  public:
    virtual void get_data(Vector& data, Point& p)
    {
      StackVector<double,3> grad;
      if(!(sfield_->gradient(grad,p)))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        smesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          sfield_->gradient(grad,coords,elem);
          data = Vector(grad[0],grad[1],grad[2]);
        }
        else
        {
          data = Vector(0.0,0.0,0.0);
        }
        return;
      }
      data = Vector(grad[0],grad[1],grad[2]);
    }

    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      StackVector<double,3> grad;
      for (size_t j=0; j<p.size(); j++)
      {
        if(!(sfield_->gradient(grad,p[j])))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          smesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            sfield_->gradient(grad,coords,elem);
            data[j] = Vector(grad[0],grad[1],grad[2]);
          }
          else
          {
            data[j] = Vector(0.0,0.0,0.0);
          }
        }
        else
        {
          data[j] = Vector(grad[0],grad[1],grad[2]);
        }
      }
    }


    ClosestInterpolatedGradientSource(FieldHandle& sfield,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E);
      maxdist_ = max_dist;
      
      is_vector_ = true;
    }

  private:
    double  maxdist_;
    VField *sfield_;
    VMesh  *smesh_;
};

class ClosestInterpolatedWeightedGradientSource : public MappingDataSource {
  public:
    virtual void get_data(Vector& data, Point& p)
    {
      double weight;
      if(!(wfield_->interpolate(weight,p,0.0)))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        wmesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          wfield_->interpolate(weight,coords,elem);
        }
        else
        {
          weight = 0.0;
        }
      }   
      StackVector<double,3> grad;
      if(!(sfield_->gradient(grad,p)))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        smesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          sfield_->gradient(grad,coords,elem);
          data = weight*Vector(grad[0],grad[1],grad[2]);
        }
        else
        {
          data = weight*Vector(0.0,0.0,0.0);
        }
      }
      else
      {
        data = weight*Vector(grad[0],grad[1],grad[2]);
      }
    }

    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      StackVector<double,3> grad;
      for (size_t j=0; j<p.size(); j++)
      {
        double weight;
        if(!(wfield_->interpolate(weight,p[j],0.0)))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          wmesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            wfield_->interpolate(weight,coords,elem);
          }
          else
          {
            weight = 0.0;
          }
        }   
        if(!(sfield_->gradient(grad,p[j])))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          smesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            sfield_->gradient(grad,coords,elem);
            data[j] = weight*Vector(grad[0],grad[1],grad[2]);
          }
          else
          {
            data[j] = weight*Vector(0.0,0.0,0.0);
          }
        }
        else
        {
          data[j] = weight*Vector(grad[0],grad[1],grad[2]);
        }
      }
    }


    ClosestInterpolatedWeightedGradientSource(FieldHandle& sfield,FieldHandle& wfield,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E);
      wfield_ = wfield->vfield();
      wmesh_ =  wfield->vmesh();
      wfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E);
      maxdist_ = max_dist;
      
      is_vector_ = true;
    }

  private:
    double  maxdist_;
    VField *sfield_;
    VMesh  *smesh_;
    VField *wfield_;
    VMesh  *wmesh_;
};

class ClosestInterpolatedWeightedTensorGradientSource : public MappingDataSource {
  public:
    virtual void get_data(Vector& data, Point& p)
    {
      Tensor weight;
      if(!(wfield_->interpolate(weight,p,Tensor(0.0))))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        wmesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          wfield_->interpolate(weight,coords,elem,Tensor(0.0));
        }
        else
        {
          weight = 0.0;
        }
      }   
      StackVector<double,3> grad;
      if(!(sfield_->gradient(grad,p)))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        smesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          sfield_->gradient(grad,coords,elem);
          data = weight*Vector(grad[0],grad[1],grad[2]);
        }
        else
        {
          data = Vector(0.0,0.0,0.0);
        }
      }
      else
      {
        data = weight*Vector(grad[0],grad[1],grad[2]);
      }
    }

    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      StackVector<double,3> grad;
      for (size_t j=0; j<p.size(); j++)
      {
        Tensor weight;
        if(!(wfield_->interpolate(weight,p[j],Tensor(0.0))))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          wmesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            wfield_->interpolate(weight,coords,elem,Tensor(0.0));
          }
          else
          {
            weight = 0.0;
          }
        }   
        if(!(sfield_->gradient(grad,p[j])))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          smesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            sfield_->gradient(grad,coords,elem);
            data[j] = weight*Vector(grad[0],grad[1],grad[2]);
          }
          else
          {
            data[j] = Vector(0.0,0.0,0.0);
          }
        }
        else
        {
          data[j] = weight*Vector(grad[0],grad[1],grad[2]);
        }
      }
    }


    ClosestInterpolatedWeightedTensorGradientSource(FieldHandle& sfield,FieldHandle& wfield,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E);
      wfield_ = wfield->vfield();
      wmesh_ =  wfield->vmesh();
      wfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E);
      maxdist_ = max_dist;
      
      is_vector_ = true;
    }

  private:
    double  maxdist_;
    VField *sfield_;
    VMesh  *smesh_;
    VField *wfield_;
    VMesh  *wmesh_;
};

class ClosestInterpolatedGradientNormSource : public MappingDataSource {
  public:
    virtual void get_data(double& data, Point& p)
    {
      StackVector<double,3> grad;
      if(!(sfield_->gradient(grad,p)))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        smesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          sfield_->gradient(grad,coords,elem);
          data = (Vector(grad[0],grad[1],grad[2])).length();
        }
        else
        {
          data = 0.0;
        }
        return;
      }
      data = (Vector(grad[0],grad[1],grad[2])).length();
    }

    virtual void get_data(std::vector<double>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      StackVector<double,3> grad;
      for (size_t j=0; j<p.size(); j++)
      {
        if(!(sfield_->gradient(grad,p[j])))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          smesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            sfield_->gradient(grad,coords,elem);
            data[j] = (Vector(grad[0],grad[1],grad[2])).length();
          }
          else
          {
            data[j] = 0.0;
          }
        }
        else
        {
          data[j] = (Vector(grad[0],grad[1],grad[2])).length();
        }
      }
    }


    ClosestInterpolatedGradientNormSource(FieldHandle& sfield,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E);
      maxdist_ = max_dist;
      
      is_double_ = true;
    }

  private:
    double  maxdist_;
    VField *sfield_;
    VMesh  *smesh_;
};

class ClosestInterpolatedWeightedGradientNormSource : public MappingDataSource {
  public:
    virtual void get_data(double& data, Point& p)
    {
      double weight;
      if(!(wfield_->interpolate(weight,p,0.0)))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        wmesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          wfield_->interpolate(weight,coords,elem,0.0);
        }
        else
        {
          weight = 0.0;
        }
      }   
      StackVector<double,3> grad;
      if(!(sfield_->gradient(grad,p)))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        smesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          sfield_->gradient(grad,coords,elem);
          data = (weight*Vector(grad[0],grad[1],grad[2])).length();
        }
        else
        {
          data = 0.0;
        }
      }
      else
      {
        data = (weight*Vector(grad[0],grad[1],grad[2])).length();
      }
    }

    virtual void get_data(std::vector<double>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      StackVector<double,3> grad;
      for (size_t j=0; j<p.size(); j++)
      {
        double weight;
        if(!(wfield_->interpolate(weight,p[j],0.0)))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          wmesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            wfield_->interpolate(weight,coords,elem,0.0);
          }
          else
          {
            weight = 0.0;
          }
        }   
        if(!(sfield_->gradient(grad,p[j])))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          smesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            sfield_->gradient(grad,coords,elem);
            data[j] = (weight*Vector(grad[0],grad[1],grad[2])).length();
          }
          else
          {
            data[j] = 0.0;
          }
        }
        else
        {
          data[j] = (weight*Vector(grad[0],grad[1],grad[2])).length();
        }
      }
    }


    ClosestInterpolatedWeightedGradientNormSource(FieldHandle& sfield,FieldHandle& wfield,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E);
      wfield_ = wfield->vfield();
      wmesh_ =  wfield->vmesh();
      wfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E);
      maxdist_ = max_dist;
      
      is_double_ = true;
    }

  private:
    double  maxdist_;
    VField *sfield_;
    VMesh  *smesh_;
    VField *wfield_;
    VMesh  *wmesh_;
};

class ClosestInterpolatedWeightedTensorGradientNormSource : public MappingDataSource {
  public:
    virtual void get_data(double& data, Point& p)
    {
      Tensor weight;
      if(!(wfield_->interpolate(weight,p,Tensor(0.0))))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        wmesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          wfield_->interpolate(weight,coords,elem,Tensor(0.0));
        }
        else
        {
          weight = 0.0;
        }
      }   
      StackVector<double,3> grad;
      if(!(sfield_->gradient(grad,p)))
      {
        double dist; Point r;
        VMesh::Elem::index_type elem;
        VMesh::coords_type coords;
        smesh_->find_closest_elem(dist,r,coords,elem,p);
        if (dist < maxdist_)
        {
          sfield_->gradient(grad,coords,elem);
          data = (weight*Vector(grad[0],grad[1],grad[2])).length();
        }
        else
        {
          data = 0.0;
        }
      }
      else
      {
        data = (weight*Vector(grad[0],grad[1],grad[2])).length();
      }
    }

    virtual void get_data(std::vector<double>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      StackVector<double,3> grad;
      for (size_t j=0; j<p.size(); j++)
      {
        Tensor weight;
        if(!(wfield_->interpolate(weight,p[j],Tensor(0.0))))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          wmesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            wfield_->interpolate(weight,coords,elem,Tensor(0.0));
          }
          else
          {
            weight = 0.0;
          }
        }   
        if(!(sfield_->gradient(grad,p[j])))
        {
          double dist; Point r;
          VMesh::Elem::index_type elem;
          VMesh::coords_type coords;
          smesh_->find_closest_elem(dist,r,coords,elem,p[j]);
          if (dist < maxdist_)
          {
            sfield_->gradient(grad,coords,elem);
            data[j] = (weight*Vector(grad[0],grad[1],grad[2])).length();
          }
          else
          {
            data[j] = 0.0;
          }
        }
        else
        {
          data[j] = (weight*Vector(grad[0],grad[1],grad[2])).length();
        }
      }
    }


    ClosestInterpolatedWeightedTensorGradientNormSource(FieldHandle& sfield,FieldHandle& wfield,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E);
      wfield_ = wfield->vfield();
      wmesh_ =  wfield->vmesh();
      wfield_->vmesh()->synchronize(Mesh::ELEM_LOCATE_E|Mesh::FIND_CLOSEST_ELEM_E);
      maxdist_ = max_dist;
      
      is_double_ = true;
    }

  private:
    double  maxdist_;
    VField *sfield_;
    VMesh  *smesh_;
    VField *wfield_;
    VMesh  *wmesh_;
};

class ClosestNodeDataSource : public MappingDataSource {
  public:
    virtual void get_data(double& data, Point& p)
    {
      VMesh::Node::index_type node; Point r; double dist;
      smesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        sfield_->get_value(data,node);
      }
      else
      {
        data = def_value_;
      }
    }

    virtual void get_data(Vector& data, Point& p)
    {
      VMesh::Node::index_type node; Point r; double dist;
      smesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        sfield_->get_value(data,node);
      }
      else
      {
        data = def_value_;
      }
    }

    virtual void get_data(Tensor& data, Point& p)
    {
      VMesh::Node::index_type node; Point r; double dist;
      smesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        sfield_->get_value(data,node);
      }
      else
      {
        data = def_value_;
      }
    }

    virtual void get_data(std::vector<double>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
        {    
        VMesh::Node::index_type node; Point r; double dist;
        smesh_->find_closest_node(dist,r,node,p[j]);
        if (dist < maxdist_)
        {
          sfield_->get_value(data[j],node);
        }
        else
        {
          data[j] = def_value_;
        }
      }
    }

    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
        {    
        VMesh::Node::index_type node; Point r; double dist;
        smesh_->find_closest_node(dist,r,node,p[j]);
        if (dist < maxdist_)
        {
          sfield_->get_value(data[j],node);
        }
        else
        {
          data[j] = def_value_;
        }
      }
    }

    virtual void get_data(std::vector<Tensor>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
        {    
        VMesh::Node::index_type node; Point r; double dist;
        smesh_->find_closest_node(dist,r,node,p[j]);
        if (dist < maxdist_)
        {
          sfield_->get_value(data[j],node);
        }
        else
        {
          data[j] = def_value_;
        }
      }
    }
    
    ClosestNodeDataSource(FieldHandle& sfield,double def_value,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E);
      def_value_ = def_value;
      maxdist_ = max_dist;

      if (sfield_->is_scalar()) is_tensor_ = true;
      if (sfield_->is_vector()) is_vector_ = true;
      if (sfield_->is_tensor()) is_tensor_ = true;
    }

  private:
    double  maxdist_;
    VField *sfield_;
    VMesh  *smesh_;
    double def_value_;
};

class ClosestNodeWeightedDataSource : public MappingDataSource {
  public:
    virtual void get_data(double& data, Point& p)
    {
      VMesh::Node::index_type node; Point r; double dist;
      double weight;
      wmesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        wfield_->get_value(weight,node);
      }
      else
      {
        weight = 0.0;
      }
      smesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        sfield_->get_value(data,node);
      }
      else
      {
        data = def_value_;
      }
      data = weight*data;
    }

    virtual void get_data(Vector& data, Point& p)
    {
      VMesh::Node::index_type node; Point r; double dist;
      double weight;
      wmesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        wfield_->get_value(weight,node);
      }
      else
      {
        weight = 0.0;
      }
      smesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        sfield_->get_value(data,node);
      }
      else
      {
        data = def_value_;
      }
      data = weight*data;
    }

    virtual void get_data(Tensor& data, Point& p)
    {
      VMesh::Node::index_type node; Point r; double dist;
      double weight;
      wmesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        wfield_->get_value(weight,node);
      }
      else
      {
        weight = 0.0;
      }
      smesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        sfield_->get_value(data,node);
      }
      else
      {
        data = def_value_;
      }
      data = weight*data;
    }

    virtual void get_data(std::vector<double>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
      {    
        VMesh::Node::index_type node; Point r; double dist;
        double weight;
        wmesh_->find_closest_node(dist,r,node,p[j]);
        if (dist < maxdist_)
        {
          wfield_->get_value(weight,node);
        }
        else
        {
          weight = 0.0;
        }
        smesh_->find_closest_node(dist,r,node,p[j]);
        if (dist < maxdist_)
        {
          sfield_->get_value(data[j],node);
        }
        else
        {
          data[j] = def_value_;
        }
        data[j] = weight*data[j];
      }
    }

    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
      {    
        VMesh::Node::index_type node; Point r; double dist;
        double weight;
        wmesh_->find_closest_node(dist,r,node,p[j]);
        if (dist < maxdist_)
        {
          wfield_->get_value(weight,node);
        }
        else
        {
          weight = 0.0;
        }
        smesh_->find_closest_node(dist,r,node,p[j]);
        if (dist < maxdist_)
        {
          sfield_->get_value(data[j],node);
        }
        else
        {
          data[j] = def_value_;
        }
        data[j] = weight*data[j];
      }
    }

    virtual void get_data(std::vector<Tensor>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
        {    
        VMesh::Node::index_type node; Point r; double dist;
        double weight;
        wmesh_->find_closest_node(dist,r,node,p[j]);
        if (dist < maxdist_)
        {
          wfield_->get_value(weight,node);
        }
        else
        {
          weight = 0.0;
        }
        smesh_->find_closest_node(dist,r,node,p[j]);
        if (dist < maxdist_)
        {
          sfield_->get_value(data[j],node);
        }
        else
        {
          data[j] = def_value_;
        }
        data[j] = weight*data[j];
      }
    }
    
    ClosestNodeWeightedDataSource(FieldHandle& sfield,FieldHandle& wfield,double def_value,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E);
      wfield_ = wfield->vfield();
      wmesh_ =  wfield->vmesh();
      wfield_->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E);

      def_value_ = def_value;
      maxdist_ = max_dist;

      if (sfield_->is_scalar()) is_double_ = true;
      if (sfield_->is_vector()) is_vector_ = true;
      if (sfield_->is_tensor()) is_tensor_ = true;
    }

  private:
    double  maxdist_;

    VField *sfield_;
    VMesh  *smesh_;
    VField *wfield_;
    VMesh  *wmesh_;

    double def_value_;
};

class ClosestNodeWeightedTensorDataSource : public MappingDataSource {
  public:
    virtual void get_data(Vector& data, Point& p)
    {
      VMesh::Node::index_type node; Point r; double dist;
      Tensor weight;
      wmesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        wfield_->get_value(weight,node);
      }
      else
      {
        weight = 0.0;
      }
      smesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        sfield_->get_value(data,node);
      }
      else
      {
        data = def_value_;
      }
      data = weight*data;
    }

    virtual void get_data(Tensor& data, Point& p)
    {
      VMesh::Node::index_type node; Point r; double dist;
      wmesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        wfield_->get_value(data,node);
      }
      else
      {
        data = 0.0;
      }
      double tdata;
      smesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        sfield_->get_value(tdata,node);
      }
      else
      {
        tdata = def_value_;
      }
      data = tdata*data;
    }
    
    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
      {    
        VMesh::Node::index_type node; Point r; double dist;
        Tensor weight;
        wmesh_->find_closest_node(dist,r,node,p[j]);
        if (dist < maxdist_)
        {
          wfield_->get_value(weight,node);
        }
        else
        {
          weight = 0.0;
        }
        smesh_->find_closest_node(dist,r,node,p[j]);
        if (dist < maxdist_)
        {
          sfield_->get_value(data[j],node);
        }
        else
        {
          data[j] = def_value_;
        }
        data[j] = weight*data[j];
      }
    }

    virtual void get_data(std::vector<Tensor>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t j=0; j<p.size(); j++)
      {    
        VMesh::Node::index_type node; Point r; double dist;
        wmesh_->find_closest_node(dist,r,node,p[j]);
        if (dist < maxdist_)
        {
          wfield_->get_value(data[j],node);
        }
        else
        {
          data[j] = 0.0;
        }
        double tdata;
        smesh_->find_closest_node(dist,r,node,p[j]);
        if (dist < maxdist_)
        {
          sfield_->get_value(tdata,node);
        }
        else
        {
          tdata = def_value_;
        }
        data[j] = tdata*data[j];
      }
    }
    
    ClosestNodeWeightedTensorDataSource(FieldHandle& wfield,FieldHandle& sfield,double def_value,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E);
      wfield_ = wfield->vfield();
      wmesh_ =  wfield->vmesh();
      wfield_->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E);
      def_value_ = def_value;
      maxdist_ = max_dist;

      if (sfield_->is_scalar()) is_tensor_ = true;
      if (sfield_->is_vector()) is_vector_ = true;
      if (sfield_->is_tensor()) is_tensor_ = true;
    }

  private:
    double  maxdist_;

    VField *sfield_;
    VMesh  *smesh_;
    VField *wfield_;
    VMesh  *wmesh_;

    double def_value_;
};


class ClosestNodeGradientSource : public MappingDataSource {
  public:
    virtual void get_data(Vector& data, Point& p)
    {
      VMesh::Node::index_type node; Point r; double dist;
      smesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        smesh_->get_neighbors(nodes_,node);
        smesh_->get_centers(points_,nodes_);
        sfield_->get_values(values_,nodes_);
        double val;
        sfield_->get_value(val,node);
        
        data[0] = 0.0;
        data[1] = 0.0;
        data[2] = 0.0;
        
        double sxw = 0.0; double syw = 0.0; double szw = 0.0;
        for (size_t j=0; j<values_.size(); j++)
        {
          double l = 1.0/(points_[j]-r).length();

          double dx = Abs(points_[j].x()-r.x());
          double xw = l*dx;
          if (xw > 1e-3)
          {
            data[0] += xw*(val-values_[j])/(r.x()-points_[j].x());
            sxw += xw;
          }

          double dy = Abs(points_[j].y()-r.y());
          double yw = l*dy;
          if (yw > 1e-3)
          {
            data[1] += yw*(val-values_[j])/(r.y()-points_[j].y());
            syw += yw;
          }

          double dz = Abs(points_[j].z()-r.z());
          double zw = l*dz;
          if (zw > 1e-3)
          {
            data[2] += zw*(val-values_[j])/(r.z()-points_[j].z());
            szw += zw;
          }
        }
        
        data[0] = data[0]/sxw;
        data[1] = data[1]/syw;
        data[2] = data[2]/szw;
      }
      else
      {
        data = Vector(0.0,0.0,0.0);
      }
    }

    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t k=0; k<p.size(); k++)
      {
        VMesh::Node::index_type node; Point r; double dist;
        smesh_->find_closest_node(dist,r,node,p[k]);
        if (dist < maxdist_)
        {
          smesh_->get_neighbors(nodes_,node);
          smesh_->get_centers(points_,nodes_);
          sfield_->get_values(values_,nodes_);
          double val;
          sfield_->get_value(val,node);
          
          data[k][0] = 0.0;
          data[k][1] = 0.0;
          data[k][2] = 0.0;
          
          double sxw = 0.0; double syw = 0.0; double szw = 0.0;
          for (size_t j=0; j<values_.size(); j++)
          {
            double l = 1.0/(points_[j]-r).length();

            double dx = Abs(points_[j].x()-r.x());
            double xw = l*dx;
            if (xw > 1e-3)
            {
              data[k][0] += xw*(val-values_[j])/(r.x()-points_[j].x());
              sxw += xw;
            }

            double dy = Abs(points_[j].y()-r.y());
            double yw = l*dy;
            if (yw > 1e-3)
            {
              data[k][1] += yw*(val-values_[j])/(r.y()-points_[j].y());
              syw += yw;
            }

            double dz = Abs(points_[j].z()-r.z());
            double zw = l*dz;
            if (zw > 1e-3)
            {
              data[k][2] += zw*(val-values_[j])/(r.z()-points_[j].z());
              szw += zw;
            }
          }
          
          data[k][0] = data[k][0]/sxw;
          data[k][1] = data[k][1]/syw;
          data[k][2] = data[k][2]/szw;
        }
        else
        {
          data[k] = Vector(0.0,0.0,0.0);
        }             
      }
    }
    
    ClosestNodeGradientSource(FieldHandle& sfield,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E|Mesh::NODE_NEIGHBORS_E);
      maxdist_ = max_dist;

      is_vector_ = true;
    }

  private:
    double  maxdist_;
    VField *sfield_;
    VMesh  *smesh_;

    std::vector<double>     values_;
    VMesh::Node::array_type nodes_;
    VMesh::points_type      points_;

};

class ClosestNodeWeightedGradientSource : public MappingDataSource {
  public:
    virtual void get_data(Vector& data, Point& p)
    {
      VMesh::Node::index_type node; Point r; double dist;
      wmesh_->find_closest_node(dist,r,node,p);
      double weight;
      if (dist < maxdist_)
      {
        wfield_->get_value(weight,node);
      }
      else
      {
        weight = 0.0;
      }
      
      smesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        smesh_->get_neighbors(nodes_,node);
        smesh_->get_centers(points_,nodes_);
        sfield_->get_values(values_,nodes_);
        double val;
        sfield_->get_value(val,node);
        
        data[0] = 0.0;
        data[1] = 0.0;
        data[2] = 0.0;
        
        double sxw = 0.0; double syw = 0.0; double szw = 0.0;
        for (size_t j=0; j<values_.size(); j++)
        {
          double l = 1.0/(points_[j]-r).length();

          double dx = Abs(points_[j].x()-r.x());
          double xw = l*dx;
          if (xw > 1e-3)
          {
            data[0] += xw*(val-values_[j])/(r.x()-points_[j].x());
            sxw += xw;
          }

          double dy = Abs(points_[j].y()-r.y());
          double yw = l*dy;
          if (yw > 1e-3)
          {
            data[1] += yw*(val-values_[j])/(r.y()-points_[j].y());
            syw += yw;
          }

          double dz = Abs(points_[j].z()-r.z());
          double zw = l*dz;
          if (zw > 1e-3)
          {
            data[2] += zw*(val-values_[j])/(r.z()-points_[j].z());
            szw += zw;
          }
        }
        
        data[0] = weight*data[0]/sxw;
        data[1] = weight*data[1]/syw;
        data[2] = weight*data[2]/szw;
      }
      else
      {
        data = Vector(0.0,0.0,0.0);
      }
    }

    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t k=0; k<p.size(); k++)
      {
        VMesh::Node::index_type node; Point r; double dist;
        wmesh_->find_closest_node(dist,r,node,p[k]);
        double weight;
        if (dist < maxdist_)
        {
          wfield_->get_value(weight,node);
        }
        else
        {
          weight = 0.0;
        }
        smesh_->find_closest_node(dist,r,node,p[k]);
        if (dist < maxdist_)
        {
          smesh_->get_neighbors(nodes_,node);
          smesh_->get_centers(points_,nodes_);
          sfield_->get_values(values_,nodes_);
          double val;
          sfield_->get_value(val,node);
          
          data[k][0] = 0.0;
          data[k][1] = 0.0;
          data[k][2] = 0.0;
          
          double sxw = 0.0; double syw = 0.0; double szw = 0.0;
          for (size_t j=0; j<values_.size(); j++)
          {
            double l = 1.0/(points_[j]-r).length();

            double dx = Abs(points_[j].x()-r.x());
            double xw = l*dx;
            if (xw > 1e-3)
            {
              data[k][0] += xw*(val-values_[j])/(r.x()-points_[j].x());
              sxw += xw;
            }

            double dy = Abs(points_[j].y()-r.y());
            double yw = l*dy;
            if (yw > 1e-3)
            {
              data[k][1] += yw*(val-values_[j])/(r.y()-points_[j].y());
              syw += yw;
            }

            double dz = Abs(points_[j].z()-r.z());
            double zw = l*dz;
            if (zw > 1e-3)
            {
              data[k][2] += zw*(val-values_[j])/(r.z()-points_[j].z());
              szw += zw;
            }
          }
          
          data[k][0] = (weight*data[k][0])/sxw;
          data[k][1] = (weight*data[k][1])/syw;
          data[k][2] = (weight*data[k][2])/szw;
        }
        else
        {
          data[k] = Vector(0.0,0.0,0.0);
        }             
      }
    }
    
    ClosestNodeWeightedGradientSource(FieldHandle& sfield,FieldHandle& wfield,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E|Mesh::NODE_NEIGHBORS_E);
      wfield_ = wfield->vfield();
      wmesh_ =  wfield->vmesh();
      wfield_->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E);
      maxdist_ = max_dist;

      is_vector_ = true;
    }

  private:
    double  maxdist_;
    VField *sfield_;
    VMesh  *smesh_;
    VField *wfield_;
    VMesh  *wmesh_;

    std::vector<double>     values_;
    VMesh::Node::array_type nodes_;
    VMesh::points_type      points_;

};

class ClosestNodeWeightedTensorGradientSource : public MappingDataSource {
  public:
    virtual void get_data(Vector& data, Point& p)
    {
      VMesh::Node::index_type node; Point r; double dist;
      wmesh_->find_closest_node(dist,r,node,p);
      Tensor weight;
      if (dist < maxdist_)
      {
        wfield_->get_value(weight,node);
      }
      else
      {
        weight = 0.0;
      }
      
      smesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        smesh_->get_neighbors(nodes_,node);
        smesh_->get_centers(points_,nodes_);
        sfield_->get_values(values_,nodes_);
        double val;
        sfield_->get_value(val,node);
        
        data[0] = 0.0;
        data[1] = 0.0;
        data[2] = 0.0;
        
        double sxw = 0.0; double syw = 0.0; double szw = 0.0;
        for (size_t j=0; j<values_.size(); j++)
        {
          double l = 1.0/(points_[j]-r).length();

          double dx = Abs(points_[j].x()-r.x());
          double xw = l*dx;
          if (xw > 1e-3)
          {
            data[0] += xw*(val-values_[j])/(r.x()-points_[j].x());
            sxw += xw;
          }

          double dy = Abs(points_[j].y()-r.y());
          double yw = l*dy;
          if (yw > 1e-3)
          {
            data[1] += yw*(val-values_[j])/(r.y()-points_[j].y());
            syw += yw;
          }

          double dz = Abs(points_[j].z()-r.z());
          double zw = l*dz;
          if (zw > 1e-3)
          {
            data[2] += zw*(val-values_[j])/(r.z()-points_[j].z());
            szw += zw;
          }
        }
        
        data[0] = data[0]/sxw;
        data[1] = data[1]/syw;
        data[2] = data[2]/szw;
        data = weight*data;
      }
      else
      {
        data = Vector(0.0,0.0,0.0);
      }
    }

    virtual void get_data(std::vector<Vector>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t k=0; k<p.size(); k++)
      {
        VMesh::Node::index_type node; Point r; double dist;
        wmesh_->find_closest_node(dist,r,node,p[k]);
        Tensor weight;
        if (dist < maxdist_)
        {
          wfield_->get_value(weight,node);
        }
        else
        {
          weight = 0.0;
        }
        smesh_->find_closest_node(dist,r,node,p[k]);
        if (dist < maxdist_)
        {
          smesh_->get_neighbors(nodes_,node);
          smesh_->get_centers(points_,nodes_);
          sfield_->get_values(values_,nodes_);
          double val;
          sfield_->get_value(val,node);
          
          data[k][0] = 0.0;
          data[k][1] = 0.0;
          data[k][2] = 0.0;
          
          double sxw = 0.0; double syw = 0.0; double szw = 0.0;
          for (size_t j=0; j<values_.size(); j++)
          {
            double l = 1.0/(points_[j]-r).length();

            double dx = Abs(points_[j].x()-r.x());
            double xw = l*dx;
            if (xw > 1e-3)
            {
              data[k][0] += xw*(val-values_[j])/(r.x()-points_[j].x());
              sxw += xw;
            }

            double dy = Abs(points_[j].y()-r.y());
            double yw = l*dy;
            if (yw > 1e-3)
            {
              data[k][1] += yw*(val-values_[j])/(r.y()-points_[j].y());
              syw += yw;
            }

            double dz = Abs(points_[j].z()-r.z());
            double zw = l*dz;
            if (zw > 1e-3)
            {
              data[k][2] += zw*(val-values_[j])/(r.z()-points_[j].z());
              szw += zw;
            }
          }
          
          data[k][0] = (data[k][0])/sxw;
          data[k][1] = (data[k][1])/syw;
          data[k][2] = (data[k][2])/szw;
          data[k] = weight*data[k];
        }
        else
        {
          data[k] = Vector(0.0,0.0,0.0);
        }             
      }
    }
    
    ClosestNodeWeightedTensorGradientSource(FieldHandle& sfield,FieldHandle& wfield,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E|Mesh::NODE_NEIGHBORS_E);
      wfield_ = wfield->vfield();
      wmesh_ =  wfield->vmesh();
      wfield_->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E);
      maxdist_ = max_dist;

      is_vector_ = true;
    }

  private:
    double  maxdist_;
    VField *sfield_;
    VMesh  *smesh_;
    VField *wfield_;
    VMesh  *wmesh_;

    std::vector<double>     values_;
    VMesh::Node::array_type nodes_;
    VMesh::points_type      points_;
};




class ClosestNodeGradientNormSource : public MappingDataSource {
  public:
    virtual void get_data(double& data, Point& p)
    {
      VMesh::Node::index_type node; Point r; double dist;
      smesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        smesh_->get_neighbors(nodes_,node);
        smesh_->get_centers(points_,nodes_);
        sfield_->get_values(values_,nodes_);
        double val; double datax; double datay; double dataz;
        sfield_->get_value(val,node);
        
        datax = 0.0;
        datay = 0.0;
        dataz = 0.0;
        
        double sxw = 0.0; double syw = 0.0; double szw = 0.0;
        for (size_t j=0; j<values_.size(); j++)
        {
          double l = 1.0/(points_[j]-r).length();

          double dx = Abs(points_[j].x()-r.x());
          double xw = l*dx;
          if (xw > 1e-3)
          {
            datax += xw*(val-values_[j])/(r.x()-points_[j].x());
            sxw += xw;
          }

          double dy = Abs(points_[j].y()-r.y());
          double yw = l*dy;
          if (yw > 1e-3)
          {
            datay += yw*(val-values_[j])/(r.y()-points_[j].y());
            syw += yw;
          }

          double dz = Abs(points_[j].z()-r.z());
          double zw = l*dz;
          if (zw > 1e-3)
          {
            dataz += zw*(val-values_[j])/(r.z()-points_[j].z());
            szw += zw;
          }
        }
        
        datax = datax/sxw;
        datay = datay/syw;
        dataz = dataz/szw;
        data = (Vector(datax,datay,dataz)).length();
      }
      else
      {
        data = 0.0;
      }
    }

    virtual void get_data(std::vector<double>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t k=0; k<p.size(); k++)
      {
        VMesh::Node::index_type node; Point r; double dist;
        smesh_->find_closest_node(dist,r,node,p[k]);
        if (dist < maxdist_)
        {
          smesh_->get_neighbors(nodes_,node);
          smesh_->get_centers(points_,nodes_);
          sfield_->get_values(values_,nodes_);
          double val;double datax; double datay; double dataz;
          sfield_->get_value(val,node);
          
          datax = 0.0;
          datay = 0.0;
          dataz = 0.0;
          
          double sxw = 0.0; double syw = 0.0; double szw = 0.0;
          for (size_t j=0; j<values_.size(); j++)
          {
            double l = 1.0/(points_[j]-r).length();

            double dx = Abs(points_[j].x()-r.x());
            double xw = l*dx;
            if (xw > 1e-3)
            {
              datax += xw*(val-values_[j])/(r.x()-points_[j].x());
              sxw += xw;
            }

            double dy = Abs(points_[j].y()-r.y());
            double yw = l*dy;
            if (yw > 1e-3)
            {
              datay += yw*(val-values_[j])/(r.y()-points_[j].y());
              syw += yw;
            }

            double dz = Abs(points_[j].z()-r.z());
            double zw = l*dz;
            if (zw > 1e-3)
            {
              dataz += zw*(val-values_[j])/(r.z()-points_[j].z());
              szw += zw;
            }
          }
          
          datax = datax/sxw;
          datay = datay/syw;
          dataz = dataz/szw;
          data[k] = (Vector(datax,datay,dataz)).length();
        }
        else
        {
          data[k] = 0.0;
        }             
      }
    }
    
    ClosestNodeGradientNormSource(FieldHandle& sfield,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E|Mesh::NODE_NEIGHBORS_E);
      maxdist_ = max_dist;

      is_double_ = true;
    }

  private:
    double  maxdist_;
    VField *sfield_;
    VMesh  *smesh_;

    std::vector<double>     values_;
    VMesh::Node::array_type nodes_;
    VMesh::points_type      points_;

};

class ClosestNodeWeightedGradientNormSource : public MappingDataSource {
  public:
    virtual void get_data(Vector& data, Point& p)
    {
      VMesh::Node::index_type node; Point r; double dist;
      wmesh_->find_closest_node(dist,r,node,p);
      double weight;
      if (dist < maxdist_)
      {
        wfield_->get_value(weight,node);
      }
      else
      {
        weight = 0.0;
      }
      
      smesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        smesh_->get_neighbors(nodes_,node);
        smesh_->get_centers(points_,nodes_);
        sfield_->get_values(values_,nodes_);
        double val; double datax; double datay; double dataz;
        sfield_->get_value(val,node);
        
        datax = 0.0;
        datay = 0.0;
        dataz = 0.0;
        
        double sxw = 0.0; double syw = 0.0; double szw = 0.0;
        for (size_t j=0; j<values_.size(); j++)
        {
          double l = 1.0/(points_[j]-r).length();

          double dx = Abs(points_[j].x()-r.x());
          double xw = l*dx;
          if (xw > 1e-3)
          {
            datax += xw*(val-values_[j])/(r.x()-points_[j].x());
            sxw += xw;
          }

          double dy = Abs(points_[j].y()-r.y());
          double yw = l*dy;
          if (yw > 1e-3)
          {
            datay += yw*(val-values_[j])/(r.y()-points_[j].y());
            syw += yw;
          }

          double dz = Abs(points_[j].z()-r.z());
          double zw = l*dz;
          if (zw > 1e-3)
          {
            dataz += zw*(val-values_[j])/(r.z()-points_[j].z());
            szw += zw;
          }
        }
        
        datax = weight*datax/sxw;
        datay = weight*datay/syw;
        dataz = weight*dataz/szw;
        data = Vector(datax,datay,dataz).length();
      }
      else
      {
        data = 0.0;
      }
    }

    virtual void get_data(std::vector<double>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t k=0; k<p.size(); k++)
      {
        VMesh::Node::index_type node; Point r; double dist;
        wmesh_->find_closest_node(dist,r,node,p[k]);
        double weight;
        if (dist < maxdist_)
        {
          wfield_->get_value(weight,node);
        }
        else
        {
          weight = 0.0;
        }
        smesh_->find_closest_node(dist,r,node,p[k]);
        if (dist < maxdist_)
        {
          smesh_->get_neighbors(nodes_,node);
          smesh_->get_centers(points_,nodes_);
          sfield_->get_values(values_,nodes_);
          double val; double datax; double datay; double dataz;
          sfield_->get_value(val,node);
          
          datax = 0.0;
          datay = 0.0;
          dataz = 0.0;
          
          double sxw = 0.0; double syw = 0.0; double szw = 0.0;
          for (size_t j=0; j<values_.size(); j++)
          {
            double l = 1.0/(points_[j]-r).length();

            double dx = Abs(points_[j].x()-r.x());
            double xw = l*dx;
            if (xw > 1e-3)
            {
              datax += xw*(val-values_[j])/(r.x()-points_[j].x());
              sxw += xw;
            }

            double dy = Abs(points_[j].y()-r.y());
            double yw = l*dy;
            if (yw > 1e-3)
            {
              datay += yw*(val-values_[j])/(r.y()-points_[j].y());
              syw += yw;
            }

            double dz = Abs(points_[j].z()-r.z());
            double zw = l*dz;
            if (zw > 1e-3)
            {
              dataz += zw*(val-values_[j])/(r.z()-points_[j].z());
              szw += zw;
            }
          }
          
          datax = (weight*datax)/sxw;
          datay = (weight*datay)/syw;
          dataz = (weight*dataz)/szw;
          data[k] = (Vector(datax,datay,dataz)).length();
        }
        else
        {
          data[k] = 0.0;
        }             
      }
    }
    
    ClosestNodeWeightedGradientNormSource(FieldHandle& sfield,FieldHandle& wfield,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E|Mesh::NODE_NEIGHBORS_E);
      wfield_ = wfield->vfield();
      wmesh_ =  wfield->vmesh();
      wfield_->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E);

      maxdist_ = max_dist;
      is_double_ = true;
    }

  private:
    double  maxdist_;
    VField *sfield_;
    VMesh  *smesh_;
    VField *wfield_;
    VMesh  *wmesh_;

    std::vector<double>     values_;
    VMesh::Node::array_type nodes_;
    VMesh::points_type      points_;

};

class ClosestNodeWeightedTensorGradientNormSource : public MappingDataSource {
  public:
    virtual void get_data(double& data, Point& p)
    {
      VMesh::Node::index_type node; Point r; double dist;
      wmesh_->find_closest_node(dist,r,node,p);
      Tensor weight;
      if (dist < maxdist_)
      {
        wfield_->get_value(weight,node);
      }
      else
      {
        weight = 0.0;
      }
      
      smesh_->find_closest_node(dist,r,node,p);
      if (dist < maxdist_)
      {
        smesh_->get_neighbors(nodes_,node);
        smesh_->get_centers(points_,nodes_);
        sfield_->get_values(values_,nodes_);
        double val; double datax; double datay; double dataz;
        sfield_->get_value(val,node);
        
        datax = 0.0;
        datay = 0.0;
        dataz = 0.0;
        
        double sxw = 0.0; double syw = 0.0; double szw = 0.0;
        for (size_t j=0; j<values_.size(); j++)
        {
          double l = 1.0/(points_[j]-r).length();

          double dx = Abs(points_[j].x()-r.x());
          double xw = l*dx;
          if (xw > 1e-3)
          {
            datax += xw*(val-values_[j])/(r.x()-points_[j].x());
            sxw += xw;
          }

          double dy = Abs(points_[j].y()-r.y());
          double yw = l*dy;
          if (yw > 1e-3)
          {
            datay += yw*(val-values_[j])/(r.y()-points_[j].y());
            syw += yw;
          }

          double dz = Abs(points_[j].z()-r.z());
          double zw = l*dz;
          if (zw > 1e-3)
          {
            dataz += zw*(val-values_[j])/(r.z()-points_[j].z());
            szw += zw;
          }
        }
        
        datax = datax/sxw;
        datay = datay/syw;
        dataz = dataz/szw;
        data = (weight*Vector(datax,datay,dataz)).length();
      }
      else
      {
        data = 0.0;
      }
    }

    virtual void get_data(std::vector<double>& data, std::vector<Point>& p)
    {
      data.resize(p.size());
      for (size_t k=0; k<p.size(); k++)
      {
        VMesh::Node::index_type node; Point r; double dist;
        wmesh_->find_closest_node(dist,r,node,p[k]);
        Tensor weight;
        if (dist < maxdist_)
        {
          wfield_->get_value(weight,node);
        }
        else
        {
          weight = 0.0;
        }
        smesh_->find_closest_node(dist,r,node,p[k]);
        if (dist < maxdist_)
        {
          smesh_->get_neighbors(nodes_,node);
          smesh_->get_centers(points_,nodes_);
          sfield_->get_values(values_,nodes_);
          double val; double datax; double datay; double dataz;
          sfield_->get_value(val,node);
          
          datax = 0.0;
          datay = 0.0;
          dataz = 0.0;
          
          double sxw = 0.0; double syw = 0.0; double szw = 0.0;
          for (size_t j=0; j<values_.size(); j++)
          {
            double l = 1.0/(points_[j]-r).length();

            double dx = Abs(points_[j].x()-r.x());
            double xw = l*dx;
            if (xw > 1e-3)
            {
              datax += xw*(val-values_[j])/(r.x()-points_[j].x());
              sxw += xw;
            }

            double dy = Abs(points_[j].y()-r.y());
            double yw = l*dy;
            if (yw > 1e-3)
            {
              datay += yw*(val-values_[j])/(r.y()-points_[j].y());
              syw += yw;
            }

            double dz = Abs(points_[j].z()-r.z());
            double zw = l*dz;
            if (zw > 1e-3)
            {
              dataz += zw*(val-values_[j])/(r.z()-points_[j].z());
              szw += zw;
            }
          }
          datax = datax/sxw;
          datay = datay/syw;
          dataz = dataz/szw;
          data[k] = (weight*Vector(datax,datay,dataz)).length();
        }
        else
        {
          data[k] = 0.0;
        }             
      }
    }
    
    ClosestNodeWeightedTensorGradientNormSource(FieldHandle& sfield,FieldHandle& wfield,double max_dist)
    {
      sfield_ = sfield->vfield();
      smesh_ =  sfield->vmesh();
      sfield_->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E|Mesh::NODE_NEIGHBORS_E);
      wfield_ = wfield->vfield();
      wmesh_ =  wfield->vmesh();
      wfield_->vmesh()->synchronize(Mesh::FIND_CLOSEST_NODE_E);

      maxdist_ = max_dist;
      is_double_ = true;
    }

  private:
    double  maxdist_;
    VField *sfield_;
    VMesh  *smesh_;
    VField *wfield_;
    VMesh  *wmesh_;

    std::vector<double>     values_;
    VMesh::Node::array_type nodes_;
    VMesh::points_type      points_;
};



bool
CreateDataSource(MappingDataSourceHandle& handle, 
                      FieldHandle& sfield,
                      FieldHandle& wfield, 
                      AlgoBase* algo)
{
  std::string quantity = algo->get_option("quantity");
  std::string value =  algo->get_option("value");
  
  double def_value = algo->get_scalar("outside_value");
  double max_dist = algo->get_scalar("max_distance");

  // Value of the data
  if (quantity == "value" && value == "interpolateddata")
  {
    if (wfield.get_rep())
    {
      if (wfield->vfield()->is_scalar())
      {
        handle = new InterpolatedWeightedDataSource(sfield,wfield,def_value);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);
      }
      else
      {
        handle = new InterpolatedWeightedTensorDataSource(sfield,wfield,def_value);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);      
      }
    }
    else
    {
      handle = new InterpolatedDataSource(sfield,def_value);
      if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);       
    }
  }
  else if (quantity == "value" && value == "closestinterpolateddata")
  {
    if (wfield.get_rep())
    {
      if (wfield->vfield()->is_scalar())
      {
        handle = new ClosestInterpolatedWeightedDataSource(sfield,wfield,def_value,max_dist);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);
      }
      else
      {
        handle = new ClosestInterpolatedWeightedTensorDataSource(sfield,wfield,def_value,max_dist);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);      
      }
    }
    else
    {
      handle = new ClosestInterpolatedDataSource(sfield,def_value,max_dist);
      if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);          
    }
  }
  else if (quantity == "value" && value == "closestnodedata")
  {
    if (wfield.get_rep())
    {
      if (wfield->vfield()->is_scalar())
      {
        handle = new ClosestNodeWeightedDataSource(sfield,wfield,def_value,max_dist);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);
      }
      else
      {
        handle = new ClosestNodeWeightedTensorDataSource(sfield,wfield,def_value,max_dist);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);      
      }
    }
    else
    {
      handle = new ClosestNodeDataSource(sfield,def_value,max_dist);
      if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);          
    }
  }

  // Gradient of the data
  if (quantity == "gradient" && value == "interpolateddata")
  {
    if (wfield.get_rep())
    {
      if (wfield->vfield()->is_scalar())
      {
        handle = new InterpolatedWeightedGradientSource(sfield,wfield);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);
      }
      else
      {
        handle = new InterpolatedWeightedTensorGradientSource(sfield,wfield);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);      
      }
    }
    else
    {
      handle = new InterpolatedGradientSource(sfield);
      if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);       
    }
  }
  else if (quantity == "gradient" && value == "closestinterpolateddata")
  {
    if (wfield.get_rep())
    {
      if (wfield->vfield()->is_scalar())
      {
        handle = new ClosestInterpolatedWeightedGradientSource(sfield,wfield,max_dist);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);
      }
      else
      {
        handle = new ClosestInterpolatedWeightedTensorGradientSource(sfield,wfield,max_dist);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);      
      }
    }
    else
    {
      handle = new ClosestInterpolatedGradientSource(sfield,max_dist);
      if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);          
    }
  }
  else if (quantity == "gradient" && value == "closestnodedata")
  {
    if (wfield.get_rep())
    {
      if (wfield->vfield()->is_scalar())
      {
        handle = new ClosestNodeWeightedGradientSource(sfield,wfield,max_dist);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);
      }
      else
      {
        handle = new ClosestNodeWeightedTensorGradientSource(sfield,wfield,max_dist);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);      
      }
    }
    else
    {
      handle = new ClosestNodeGradientSource(sfield,max_dist);
      if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);          
    }
  }

  // GradientNorm of the data
  if (quantity == "gradientnorm" && value == "interpolateddata")
  {
    if (wfield.get_rep())
    {
      if (wfield->vfield()->is_scalar())
      {
        handle = new InterpolatedWeightedGradientNormSource(sfield,wfield);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);
      }
      else
      {
        handle = new InterpolatedWeightedTensorGradientNormSource(sfield,wfield);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);      
      }
    }
    else
    {
      handle = new InterpolatedGradientNormSource(sfield);
      if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);       
    }
  }
  else if (quantity == "gradientnorm" && value == "closestinterpolateddata")
  {
    if (wfield.get_rep())
    {
      if (wfield->vfield()->is_scalar())
      {
        handle = new ClosestInterpolatedWeightedGradientNormSource(sfield,wfield,max_dist);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);
      }
      else
      {
        handle = new ClosestInterpolatedWeightedTensorGradientNormSource(sfield,wfield,max_dist);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);      
      }
    }
    else
    {
      handle = new ClosestInterpolatedGradientNormSource(sfield,max_dist);
      if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);          
    }
  }
  else if (quantity == "gradientnorm" && value == "closestnodedata")
  {
    if (wfield.get_rep())
    {
      if (wfield->vfield()->is_scalar())
      {
        handle = new ClosestNodeWeightedGradientNormSource(sfield,wfield,max_dist);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);
      }
      else
      {
        handle = new ClosestNodeWeightedTensorGradientNormSource(sfield,wfield,max_dist);
        if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);      
      }
    }
    else
    {
      handle = new ClosestNodeGradientNormSource(sfield,max_dist);
      if (handle->is_scalar() || handle->is_vector() || handle->is_tensor()) return (true);          
    }
  }

  return (false);
}


} // end namespace SCIRunAlgo
