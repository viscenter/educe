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



#ifndef CORE_DATATYPES_VFIELD_H
#define CORE_DATATYPES_VFIELD_H 1

#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/VFData.h>
#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/PropertyManager.h>


#include <Core/Datatypes/share.h>

namespace SCIRun {

// Define a handle to the virtual interface
class Field;
typedef LockingHandle<Field> FieldHandle;

class SCISHARE VField {
//! The FieldTypeInformation call introduces functions to check type of the field
//! Like is_pointcloudmesh() is_lineardata() is_vector() etc.
//! Check FieldInformation.h for a full list

public:

  VField() :
    field_(0),
    mesh_(0),
    vmesh_(0),
    vfdata_(0),
    basis_order_(0),
    number_of_nodes_(0),
    number_of_enodes_(0),
    element_dim_(0),
    element_dofs_(0),
    is_scalar_(false),
    is_pair_(false),
    is_vector_(false),
    is_tensor_(false)
  {}
  
  //! Import a couple of useful typedefs from VMesh
  typedef VMesh::index_type           index_type;
  typedef VMesh::size_type            size_type;
  typedef VMesh::coords_type          coords_type;
  typedef VMesh::weight_type          weight_type;

  typedef VMesh::index_array_type     index_array_type;
  typedef VMesh::weight_array_type    weight_array_type;

  //! mesh() and vmesh() get the handle to the mesh and its virtual interface
  inline MeshHandle mesh()  { return (MeshHandle(mesh_)); }
  inline VMesh*     vmesh() { return (vmesh_); }
  
  //! get a handle to the field
  inline FieldHandle field() { return (FieldHandle(field_)); }
  //! for completeness get a pointer to itself
  inline VField* vfield() { return (this); }
    
  //! Get the basis_order of the field data, -1 = nodata, 0 = constant, 1 = linear
  //! 2 = quadratic 3 = cubic  
  inline int basis_order() { return (basis_order_); }
  
  //! get the number of values in the field (data at corner nodes of the elements)
  inline VMesh::size_type num_values() { return (vfdata_->fdata_size()); }
  //! get the number of edge values in the field (for quadratic approximation)
  inline VMesh::size_type num_evalues() { return (vfdata_->efdata_size()); }
  
  //! resize the data fields to match the number of nodes/edges in the mesh
  inline void resize_fdata() 
    { 
      if (basis_order_ == -1)
      {
        VMesh::dimension_type dim;
        dim.resize(1); dim[0] = 0;
        vfdata_->resize_fdata(dim);
        // do nothing
      }
      else if (basis_order_ == 0)
      {
        VMesh::dimension_type dim;
        vmesh_->get_elem_dimensions(dim);
        vfdata_->resize_fdata(dim);
      }
      else if (basis_order_ == 1)
      {
        VMesh::dimension_type dim;
        vmesh_->get_dimensions(dim);
        vfdata_->resize_fdata(dim);      
      }
      else if (basis_order_ == 2)
      {
        VMesh::dimension_type dim;
        vmesh_->get_dimensions(dim);
        vfdata_->resize_fdata(dim);      
        vfdata_->resize_efdata(dim);      
      }
    }
  //! same function but now uses the systematic naming 
  //! (resize_fdata is the old call, SCIRun was using)
  inline void resize_values() { resize_fdata(); }

  //! Get values from field (different varieties for different index types)
  //! Unlike the direct interface, we do not check index_type here
  //! all calls are rerouted to the same function call
  
  //! NOTE THE FOLLOWING TEMPLATES WILL COMPILE
  //! Template T can be of the following types:
  //! char, unsigned char, short, unsigned short, int, unsigned int, long,
  //! unsigned long, long long, unsigned long long, float, double,
  //! Vector, and Tensor
  
  template<class T> inline void get_value(T& val, index_type idx) const
  { vfdata_->get_value(val,idx); }  
  //! evalue stands for edge value used in quadratic approximation, the latter
  //! can be accessed using direct indices or ENode::index_type
  template<class T> inline void get_evalue(T& val, index_type idx) const
  { vfdata_->get_evalue(val,idx); }  
  template<class T>  inline void get_value(T& val, VMesh::Node::index_type idx) const
  { vfdata_->get_value(val,static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void get_value(T& val, VMesh::Edge::index_type idx) const
  { vfdata_->get_value(val,static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void get_value(T& val, VMesh::Face::index_type idx) const
  { vfdata_->get_value(val,static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void get_value(T& val, VMesh::Cell::index_type idx) const
  { vfdata_->get_value(val,static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void get_value(T& val, VMesh::Elem::index_type idx) const
  { vfdata_->get_value(val,static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void get_value(T& val, VMesh::DElem::index_type idx) const
  { vfdata_->get_value(val,static_cast<VMesh::index_type>(idx)); }  
  template<class T>  inline void get_value(T& val, VMesh::ENode::index_type idx) const
  { vfdata_->get_evalue(val,static_cast<VMesh::index_type>(idx)); }

  //! Ensure compatibility with old field class, these are equivalent with get_value()
  //! unlike the direct interface we do not use ASSERTs to check bounds and this function
  //! calls are thus exactly the same. They are provided for compatibility with the old
  //! classes
  template<class T> inline void value(T& val, index_type idx) const
  { vfdata_->get_value(val,idx); }  
  template<class T>  inline void value(T& val, VMesh::Node::index_type idx) const
  { vfdata_->get_value(val,static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void value(T& val, VMesh::Edge::index_type idx) const
  { vfdata_->get_value(val,static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void value(T& val, VMesh::Face::index_type idx) const
  { vfdata_->get_value(val,static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void value(T& val, VMesh::Cell::index_type idx) const
  { vfdata_->get_value(val,static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void value(T& val, VMesh::Elem::index_type idx) const
  { vfdata_->get_value(val,static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void value(T& val, VMesh::DElem::index_type idx) const
  { vfdata_->get_value(val,static_cast<VMesh::index_type>(idx)); }  

  //! Functions for getting a weighted value
  template<class T> inline void get_weighted_value(T& val, index_type* idx, weight_type* w, size_type sz) const
  { vfdata_->get_weighted_value(val,idx,w,sz); }
  template<class T> inline void get_weighted_value(T& val, index_array_type idx, weight_array_type w) const
  { vfdata_->get_weighted_value(val,&(idx[0]),&(w[0]),idx.size()); }
  template<class T> inline void get_weighted_evalue(T& val, index_type* idx, weight_type* w, size_type sz) const
  { vfdata_->get_weighted_evalue(val,idx,w,sz); }
  template<class T> inline void get_weighted_evalue(T& val, index_array_type idx, weight_array_type w) const
  { vfdata_->get_weighted_value(val,&(idx[0]),&(w[0]),idx.size()); }


  //! Insert values into field, for every get_value there is an equivalent set_value
  //! likewise get_evalue is replaced by set set_evalue
  template<class T> inline void set_value(const T& val, index_type idx)
  { vfdata_->set_value(val,idx); }
  template<class T> inline void set_evalue(const T& val, index_type idx)
  { vfdata_->set_evalue(val,idx); }
  template<class T>  inline void set_value(const T& val, VMesh::Node::index_type idx)
  { vfdata_->set_value(val,static_cast<VMesh::index_type>(idx)); }  
  template<class T>  inline void set_value(const T& val, VMesh::Edge::index_type idx)
  { vfdata_->set_value(val,static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void set_value(const T& val, VMesh::Face::index_type idx)
  { vfdata_->set_value(val,static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void set_value(const T& val, VMesh::Cell::index_type idx)
  { vfdata_->set_value(val,static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void set_value(const T& val, VMesh::Elem::index_type idx)
  { vfdata_->set_value(val,static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void set_value(const T& val, VMesh::DElem::index_type idx)
  { vfdata_->set_value(val,static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void set_value(const T& val, VMesh::ENode::index_type idx)
  { vfdata_->set_evalue(val,static_cast<VMesh::index_type>(idx)); }  

  //! Get/Set all values at once
  template<class T> inline void set_values(const vector<T>& values)
  { vfdata_->set_values(&(values[0]),values.size()); }
  template<class T> inline void set_values(const T* data, size_type sz)
  { vfdata_->set_values(data,sz); }
  template<class T> inline void get_values(vector<T>& values) const
  { values.resize(vfdata_->fdata_size()); if (values.size()) vfdata_->get_values(&(values[0]),values.size()); }
  template<class T> inline void get_values(T* data, size_type sz) const
  { vfdata_->get_values(data,sz); }
  
  //! Set all values to a specific value
  template<class T> inline void set_all_values(const T val)
  { vfdata_->set_all_values(val); }


  //! Functions for getting a weighted value
  template<class T> inline void copy_weighted_value(VField* field, index_type* idx, weight_type* w, size_type sz, index_type i) const
  { vfdata_->copy_weighted_value(field->vfdata_,idx,w,sz,i); }
  template<class T> inline void copy_weighted_value(VField* field, index_array_type idx, weight_array_type w, index_type i) const
  { vfdata_->copy_weighted_value(field->vfdata_,&(idx[0]),&(w[0]),idx.size(),i); }
  template<class T> inline void copy_weighted_evalue(VField* field, index_type* idx, weight_type* w, size_type sz, index_type i) const
  { vfdata_->copy_weighted_evalue(field->vfdata_,idx,w,sz,i); }
  template<class T> inline void copy_weighted_evalue(VField* field, index_array_type idx, weight_array_type w, index_type i) const
  { vfdata_->copy_weighted_value(field->vfdata_,&(idx[0]),&(w[0]),idx.size(),i); }

  //! Set all values to zero or its equivalent, all none double data will be casted
  //! to the proper value automatically. This way we do not need an additional
  //! virtual function call 
  inline void clear_all_values()
  { vfdata_->set_all_values((double)0); }
  
  //! The following cases are more specialized cases for copying entiry sets of
  //! data. These functions need to know the size of the inserted data as they
  //! perform a safety check on the length of the fdata array.
  template<class T> inline void set_evalues(const vector<T>& values)
  { vfdata_->set_evalues(&(values[0]),values.size()); }
  template<class T> inline void set_evalues(const T* data, size_type sz)
  { vfdata_->set_evalues(data,sz); }
  template<class T> inline void get_evalues(vector<T>& values) const
  { values.resize(vfdata_->efdata_size()); if (values.size()) vfdata_->get_evalues(&(values[0]),values.size()); }
  template<class T> inline void get_evalues(T* data, size_type sz) const
  { vfdata_->get_evalues(data,sz); }  

  //! Copy a value from one vfdata container to another, without having to know
  //! its type. Often for geometric operations one only has to copy the data.
  //! in tota this function will do to virtual function calls to move data from
  //! one container to the next. It however does direct casting between types. 
  //! call these functions from the destination field to import data from another field
  template<class INDEX1, class INDEX2> inline void copy_value(VField* field, INDEX1 idx1, INDEX2 idx2)
  {
    vfdata_->copy_value(field->vfdata_,index_type(idx1),index_type(idx2));
  }
  
  //! Same for edge values
  template<class INDEX1, class INDEX2> inline void copy_evalue(VField* field, INDEX1 idx1, INDEX2 idx2)
  {
    vfdata_->copy_evalue(field->vfdata_,index_type(idx1),index_type(idx2));
  }

  //! Copy all the values from one container to another container
  //! call these functions from the destination field to import data from another field
  inline void copy_values(VField* field)
  { vfdata_->copy_values(field->vfdata_); }

  inline void copy_evalues(VField* field)
  { vfdata_->copy_evalues(field->vfdata_); }

  //! Maximum and minimum of values (with index to see where maximum is located)
  inline bool min(double& mn,index_type& idx)
  { mn = 0.0; return(vfdata_->min(mn,idx)); }
  
  inline bool min(double& mn)
  { mn = 0.0; index_type idx; return(vfdata_->min(mn,idx)); }

  inline bool max(double& mx,index_type& idx) const
  { mx = 0.0; return(vfdata_->max(mx,idx)); }
  inline bool max(double& mx) const
  { index_type idx; mx =0.0; return(vfdata_->max(mx,idx)); }

  //! Combined min max for efficiency
  inline bool minmax(double& mn, index_type& idxmn, 
                     double& mx, index_type idxmx) const
  { 
    mn = 0.0;
    mx = 0.0;
    return(vfdata_->minmax(mn,idxmn,mx,idxmx)); 
  }  
  
  //! version of minmax without indices
  inline bool minmax(double& mn, double& mx) const
  { 
    index_type idxmn;
    index_type idxmx;
    mn = 0.0;
    mx = 0.0;
    return(vfdata_->minmax(mn,idxmn,mx,idxmx)); 
  }
  
  inline void size(VMesh::Node::size_type& sz) { sz = number_of_nodes_; }
  inline void size(VMesh::ENode::size_type& sz) { sz = number_of_enodes_; }
  
  //! dimensions of the element 0 .. 3
  inline int dimension() { return(element_dim_); }
  //! number of degree of freedoms in each element type in the mesh
  inline int dofs() { return(element_dofs_); }


  //! NOTE FOR INTERPOLATE AND GRADIENT
  //! Interpolate and Gradient functions are only available for the following
  //! datatypes: double, Vector, and Tensor

  //! General interpolation function. This function does at most two virtual
  //! function calls per call. It first will retrieve proper interpolation
  //! weights from the mesh and then will apply it to the data inside the
  //! data container. All objects needed for interpolation are reserved on
  //! the stack for efficiency.
  //! Note that different data orders have different implementations as non linear
  //! interpolation requires far more data, these are handled in separate classes

  template<class T>  
  inline void minterpolate(T& val,const  VMesh::coords_array_type &coords, VMesh::index_type idx) const
  { 
    VMesh::MultiElemInterpolate ei;
    vmesh_->get_minterpolate_weights(coords,idx,ei,basis_order_);
    vfdata_->minterpolate(val,ei);
  }

  template<class T>  
  inline void interpolate(T& val,const  VMesh::coords_type &coords, VMesh::index_type idx) const
  { 
    VMesh::ElemInterpolate ei;
    vmesh_->get_interpolate_weights(coords,idx,ei,basis_order_);
    vfdata_->interpolate(val,ei);
  }

  inline void interpolate(Point& val,const  VMesh::coords_type &coords, VMesh::index_type idx) const
  {
    vmesh_->interpolate(val,coords,idx);
  }
   
  // inline conversion using other index types.
  template<class T>  inline void interpolate(T& val, const VMesh::coords_type &coords, VMesh::Edge::index_type idx) const
  { interpolate(val, coords, static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void interpolate(T& val, const VMesh::coords_type &coords, VMesh::Face::index_type idx) const
  { interpolate(val, coords, static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void interpolate(T& val, const VMesh::coords_type &coords, VMesh::Cell::index_type idx) const
  { interpolate(val, coords, static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void interpolate(T& val, const VMesh::coords_type &coords, VMesh::Elem::index_type idx) const
  { interpolate(val, coords, static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void interpolate(T& val, const VMesh::coords_type &coords, VMesh::DElem::index_type idx) const
  { interpolate(val, coords, static_cast<VMesh::index_type>(idx)); }

  //! Similar to interpolation, but now this one derives the first derivative of
  //! the field (i.e. the gradient). Again the implementation is similar to the
  //! implementation of interpolate
  template<class T>  inline void gradient(StackVector<T,3>& val, const VMesh::coords_type &coords, VMesh::index_type idx) const
  { 
    VMesh::ElemGradient eg;
    vmesh_->get_gradient_weights(coords,idx,eg,basis_order_);
    vfdata_->gradient(val,eg);
  }

  template<class T>  inline void mgradient(StackVector<T,3>& val, const VMesh::coords_array_type &coords, VMesh::index_type idx) const
  { 
    VMesh::MultiElemGradient eg;
    vmesh_->get_mgradient_weights(coords,idx,eg,basis_order_);
    vfdata_->mgradient(val,eg);
  }

  //! Different versions for different index types
  template<class T>  inline void gradient(StackVector<T,3>& val, const VMesh::coords_type &coords, VMesh::Edge::index_type idx) const
  { gradient(val, coords, static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void gradient(StackVector<T,3>& val, const VMesh::coords_type &coords, VMesh::Face::index_type idx) const
  { gradient(val, coords, static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void gradient(StackVector<T,3>& val, const VMesh::coords_type &coords, VMesh::Cell::index_type idx) const
  { gradient(val, coords, static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void gradient(StackVector<T,3>& val, const VMesh::coords_type &coords, VMesh::Elem::index_type idx) const
  { gradient(val, coords, static_cast<VMesh::index_type>(idx)); }
  template<class T>  inline void gradient(StackVector<T,3>& val, const VMesh::coords_type &coords, VMesh::DElem::index_type idx) const
  { gradient(val, coords, static_cast<VMesh::index_type>(idx)); }
    
  //! internal function - this one may change in the future
  void update_mesh_pointer(Mesh* mesh)
  {
    vmesh_ = mesh->vmesh();
    mesh_ = mesh;
  }
        
  inline void* fdata_pointer()   { return (vfdata_->fdata_pointer()); }      
  inline void* efdata_pointer()   { return (vfdata_->efdata_pointer()); }      
        
  inline bool is_nodata()        { return (basis_order_ == -1); }
  inline bool is_constantdata()  { return (basis_order_ == 0); }
  inline bool is_lineardata()    { return (basis_order_ == 1); }
  inline bool is_nonlineardata() { return (basis_order_ > 1); }
  inline bool is_quadraticdata() { return (basis_order_ == 2); }
  inline bool is_cubicdata()     { return (basis_order_ == 3); }

  inline bool is_constantmesh()  { return (vmesh_->basis_order() == 0); }
  inline bool is_linearmesh()    { return (vmesh_->basis_order() == 1); }
  inline bool is_nonlinearmesh() { return (vmesh_->basis_order() > 1); }
  inline bool is_quadraticmesh() { return (vmesh_->basis_order() == 2); }
  inline bool is_cubicmesh()     { return (vmesh_->basis_order() == 3); }

  inline bool is_isomorphic()    { return (basis_order_ == vmesh_->basis_order()); }

  inline bool is_scalar()        { return (is_scalar_); }
  inline bool is_pair()          { return (is_pair_); }
  inline bool is_vector()        { return (is_vector_); }
  inline bool is_tensor()        { return (is_tensor_); }
  
  inline bool is_char()                { return ((data_type_=="char")||(data_type_=="signed_char")); }
  inline bool is_unsigned_char()       { return (data_type_=="unsigned_char"); }        
  inline bool is_short()               { return ((data_type_=="short")||(data_type_=="signed_short")); }
  inline bool is_unsigned_short()      { return (data_type_=="unsigned_short"); }
  inline bool is_int()                 { return ((data_type_=="int")||(data_type_=="signed_int")); }
  inline bool is_unsigned_int()        { return (data_type_=="unsigned_int"); }
  inline bool is_long()                { return ((data_type_=="long")||(data_type_=="signed_long")); }
  inline bool is_unsigned_long()       { return (data_type_=="unsigned_long"); }
  inline bool is_longlong()            { return ((data_type_=="long_long")||(data_type_=="signed_long_long")); }
  inline bool is_unsigned_longlong()   { return (data_type_=="unsigned_long_long"); }
  inline bool is_float()               { return (data_type_=="float"); }
  inline bool is_double()              { return (data_type_=="double"); }

  // check whether it is of integer class
  inline bool is_signed_integer()     { return (is_char()||is_short()||is_int()); }
  inline bool is_unsigned_integer()   { return (is_unsigned_char()||is_unsigned_short()||is_unsigned_int()); }
  inline bool is_integer()            { return (is_signed_integer()||is_unsigned_integer()); }

  // Shortcuts to property manager
  inline void copy_properties(VField* ifield)
    { pm_->copy_properties(ifield->pm_); }
    
  template<class T> 
  inline void set_property(const string &name, const T &val, bool is_transient)
    { pm_->set_property(name,val,is_transient); }
    
  template<class T> 
  inline bool get_property( const string &name, T &val)
    { return(pm_->get_property(name,val)); }
    
  inline bool is_property( const string &name)
    { return(pm_->is_property(name)); }

protected:
  
  // Pointers to structures to access the data virtually
  
  // Interface to Field
  Field*        field_;
  Mesh*         mesh_;
  
  PropertyManager* pm_;
  
  // Interface to the data in the field
  VMesh*        vmesh_;
  VFData*       vfdata_;
  
  // Information from the basis
  int           basis_order_;
  int           number_of_nodes_;
  int           number_of_enodes_;
  int           element_dim_;
  int           element_dofs_;

  bool          is_scalar_;
  bool          is_pair_;
  bool          is_vector_;
  bool          is_tensor_;

  std::string   data_type_;

};  


}

#endif