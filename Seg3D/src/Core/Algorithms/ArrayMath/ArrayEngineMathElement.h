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


#ifndef CORE_ALGORITHMS_ARRAYMATH_ARRAYENGINEMATHELEMENT_H
#define CORE_ALGORITHMS_ARRAYMATH_ARRAYENGINEMATHELEMENT_H 1

#include <Core/Algorithms/ArrayMath/ArrayEngineMath.h>

namespace DataArrayMath {

class Element {
  public:
    Element(SCIRun::VField* vfield) :
      vfield_(vfield), vmesh_(vfield->vmesh()), idx_(0) {}

    Element() :
      vfield_(0), vmesh_(0), idx_(0) {}
      
    Element& operator=(const Element& elem)
    {
      vfield_ = elem.vfield_;
      vmesh_ = elem.vmesh_;
      idx_    = elem.idx_;
      return (*this);
    }
    
    inline Vector center()
    {
      SCIRun::Point p;
      if (vfield_->basis_order() == 0) vmesh_->get_center(p,SCIRun::VMesh::Node::index_type(idx_));
      else vmesh_->get_center(p,SCIRun::VMesh::Elem::index_type(idx_));
      return (Vector(p.x(),p.y(),p.z()));
    }
    
    inline Scalar size()
    {
      if (vfield_->basis_order() == 0) return (Scalar(vmesh_->get_size(SCIRun::VMesh::Elem::index_type(idx_))));
      else return (Scalar(0.0));
    }
    
    inline Scalar length()
    {
      if (vfield_->basis_order() == 0) return (Scalar(vmesh_->get_size(SCIRun::VMesh::Elem::index_type(idx_))));
      else return (Scalar(0.0));
    }

    inline Scalar area()
    {
      if (vfield_->basis_order() == 0) return (Scalar(vmesh_->get_size(SCIRun::VMesh::Elem::index_type(idx_))));
      else return (Scalar(0.0));
    }

    inline Scalar volume()
    {
      if (vfield_->basis_order() == 0) return (Scalar(vmesh_->get_size(SCIRun::VMesh::Elem::index_type(idx_))));
      else return (Scalar(0.0));
    }
    
    inline Scalar dimension()
    {
      return (static_cast<Scalar>(vmesh_->dimensionality()));
    }
    
    inline Vector normal()
    {
      if (vfield_->basis_order() == 0)
      {
        SCIRun::VMesh::coords_type coords(3,0.0);
        SCIRun::Vector vec;
        // TO DO: fix this function call
        vmesh_->get_normal(vec,coords,SCIRun::VMesh::Elem::index_type(idx_),0);
        return Vector(vec.x(),vec.y(),vec.z());
      }
      else
      {
        return Vector(0.0,0.0,0.0);
      }
    }
  
    inline void next()  { idx_++; }
    inline void reset() { idx_ = 0; }
    
  private:
    SCIRun::VField*            vfield_;
    SCIRun::VMesh*             vmesh_;
    SCIRun::VField::index_type idx_;
};

} // end namespace

#endif

