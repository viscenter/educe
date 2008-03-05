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



#ifndef CORE_DATATYPES_GENERICFIELD_H
#define CORE_DATATYPES_GENERICFIELD_H 1

#include <Core/Basis/Locate.h>
#include <Core/Containers/StackVector.h>
#include <Core/Datatypes/builtin.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/VField.h>
#include <Core/Datatypes/VFData.h>
#include <Core/Datatypes/TypeName.h>
#include <Core/Datatypes/MeshTypes.h>
#include <Core/Containers/LockingHandle.h>
#include <Core/Malloc/Allocator.h>
#include <Core/Persistent/PersistentSTL.h>
#include <Core/Containers/FData.h>
#include <Core/Datatypes/CastFData.h>
#include <Core/Containers/StackVector.h>

#include <Core/Datatypes/share.h>

namespace SCIRun {

template <class Mesh, class Basis, class FData>
class GenericField: public Field 
{
public:
  //! Typedefs to support the Field concept.
  typedef GenericField<Mesh, Basis, FData>                 field_type;
  typedef typename FData::value_type                       value_type;
  typedef Mesh                                             mesh_type;
  typedef LockingHandle<mesh_type>                         mesh_handle_type;
  typedef Basis                                            basis_type;
  typedef FData                                            fdata_type;
  typedef LockingHandle<GenericField<Mesh, Basis, FData> > handle_type;

  //! only Pio should use this constructor
  GenericField();
  //! Use this constructor to actually have a field with a mesh
  GenericField(mesh_handle_type mesh);
  GenericField(const GenericField &copy);

  virtual ~GenericField();

  //! Clone the field data, but not the mesh.
  //! Use mesh_detach() first to clone the complete field
  virtual GenericField<Mesh, Basis, FData> *clone() const;

  //! Obtain a Handle to the Mesh
  virtual MeshHandle mesh() const;
  virtual VMesh* vmesh() const;
  virtual VField* vfield() const;
  
  //! Clone the the mesh
  virtual void mesh_detach();

  //! OBSOLETE
  virtual const TypeDescription *order_type_description() const;

  //! Get the order of the field data
  //! -1 = no data
  //! 0 = constant data per element
  //! 1 = linear data per element
  //! >1 = non linear data per element
  virtual int basis_order() const { return basis_.polynomial_order(); }

  //! DIRECT ACCESS TO CONTAINER
  //! NOTE: We may change container types in the future
  //! DIRECT ACCESS REQUIRES DYNAMIC COMPILATION
  fdata_type& fdata();
  const fdata_type& fdata() const;
  
  virtual void resize_fdata();
  virtual size_type size_fdata() { return fdata_.size(); }

  //! Get the classes on which this function relies:
  //! Get the basis describing interpolation within an element
  Basis& get_basis()  { return basis_; }
  
  //! Get the mesh describing how the elements fit together
  const mesh_handle_type &get_typed_mesh() const;

  //! Persistent I/O.
  virtual void io(Piostream &stream);
  
  //! Tag the constructor of this class and put it in the Pio DataBase
  static  PersistentTypeID type_id;
  
  //! Tag the constructor of this class and put it in the Field DataBase
  static  FieldTypeID field_id;
  
  //! Function to retrieve the name of this field class
  static  const string type_name(int n = -1);
 
  //! A different way of tagging a class. Currently two systems are used next
  //! to each other: type_name and get_type_description. Neither is perfect
  virtual 
  const TypeDescription* get_type_description(td_info_e td = FULL_TD_E) const;

  //! Static functions to instantiate the field from Pio or using CreateField()
  static Persistent *maker();
  static FieldHandle field_maker();  
  static FieldHandle field_maker_mesh(MeshHandle mesh);
  
  //! FIELD FUNCTIONS THAT RELY ON DYNAMIC COMPILATION (NOT VIRTUAL)
 
  //! Required interface to support Field Concept.
  bool value(value_type &val, typename mesh_type::Node::index_type i) const;
  bool value(value_type &val, typename mesh_type::Edge::index_type i) const;
  bool value(value_type &val, typename mesh_type::Face::index_type i) const;
  bool value(value_type &val, typename mesh_type::Cell::index_type i) const;

  //! Required interface to support Field Concept.
  void set_value(const value_type &val, typename mesh_type::Node::index_type i);
  void set_value(const value_type &val, typename mesh_type::Edge::index_type i);
  void set_value(const value_type &val, typename mesh_type::Face::index_type i);
  void set_value(const value_type &val, typename mesh_type::Cell::index_type i);

  //! No safety check for the following calls, be sure you know where data is.
  const value_type &value(typename mesh_type::Node::index_type i) const;
  const value_type &value(typename mesh_type::Edge::index_type i) const;
  const value_type &value(typename mesh_type::Face::index_type i) const;
  const value_type &value(typename mesh_type::Cell::index_type i) const;

  //! Using the field variable basis interpolate a value within the 
  //! element, indicated at the paramentric coordinates coords.
  void interpolate(value_type &val, const vector<double> &coords, 
		   typename mesh_type::Elem::index_type ei) const
  {

    ElemData<field_type> fcd(*this, ei);
    val = basis_.interpolate(coords, fcd);
  }

  //! Using the field variable basis interpolate a gradient within the 
  //! element, indicated at the paramentric coordinates coords.
  void  gradient(vector<value_type>& grad, const vector<double>& coords,
          typename mesh_type::Elem::index_type ci ) const;
  
private:
  friend class ElemData;

  template <class FLD>
  class ElemData
  {
  public:
    typedef typename FData::value_type value_type;

    ElemData(const FLD& fld, 
	     typename FLD::mesh_type::Elem::index_type idx) :
      fld_(fld),
      index_(idx)
    {
      fld_.mesh_->get_nodes(nodes_, idx);
      if (fld_.basis_order_ > 1) {
        fld_.mesh_->get_edges(edges_, idx);
      }
    }
    
    // basis may encode extra values based on cell index.
    unsigned elem_index() const { return index_; }
    
    inline 
    typename FData::value_type elem() const {
      return fld_.fdata_[index_];
    }

    inline 
    unsigned node0_index() const {
      return nodes_[0]; 
    }
    inline 
    unsigned node1_index() const {
      return nodes_[1];
    }
    inline 
    unsigned node2_index() const {
      return nodes_[2];
    }
    inline 
    unsigned node3_index() const {
      return nodes_[3];
    }
    inline 
    unsigned node4_index() const {
      return nodes_[4];
    }
    inline 
    unsigned node5_index() const {
      return nodes_[5];
    }
    inline 
    unsigned node6_index() const {
      return nodes_[6];
    }
    inline 
    unsigned node7_index() const {
      return nodes_[7];
    }

    inline 
    unsigned edge0_index() const {
      return edges_[0]; 
    }
    inline 
    unsigned edge1_index() const {
      return edges_[1];
    }
    inline 
    unsigned edge2_index() const {
      return edges_[2];
    }
    inline 
    unsigned edge3_index() const {
      return edges_[3];
    }
    inline 
    unsigned edge4_index() const {
      return edges_[4];
    }
    inline 
    unsigned edge5_index() const {
      return edges_[5];
    }
    inline 
    unsigned edge6_index() const {
      return edges_[6];
    }
    inline 
    unsigned edge7_index() const {
      return edges_[7];
    }
    inline 
    unsigned edge8_index() const {
      return edges_[8];
    }
    inline 
    unsigned edge9_index() const {
      return edges_[9];
    }
    inline 
    unsigned edge10_index() const {
      return edges_[10];
    }
    inline 
    unsigned edge11_index() const {
      return edges_[11];
    }

    inline 
    typename FData::value_type node0() const {
      return fld_.fdata_[nodes_[0]];
    }
    inline 
    typename FData::value_type node1() const {
      return fld_.fdata_[nodes_[1]];
    }
    inline 
    typename FData::value_type node2() const {
      return fld_.fdata_[nodes_[2]];
    }
    inline 
    typename FData::value_type node3() const {
      return fld_.fdata_[nodes_[3]];
    }
    inline 
    typename FData::value_type node4() const {
      return fld_.fdata_[nodes_[4]];
    }
    inline 
    typename FData::value_type node5() const {
      return fld_.fdata_[nodes_[5]];
    }
    inline 
    typename FData::value_type node6() const {
      return fld_.fdata_[nodes_[6]];
    }
    inline 
    typename FData::value_type node7() const {
      return fld_.fdata_[nodes_[7]];
    }


  private:
    const FLD                                   &fld_; //the field 
    typename FLD::mesh_type::Node::array_type    nodes_;
    typename FLD::mesh_type::Edge::array_type    edges_;
    typename FLD::mesh_type::Elem::index_type    index_;
  };


protected:

  //! A (generic) mesh.
  mesh_handle_type             mesh_;
  //! Data container.
  fdata_type                   fdata_;
  Basis                        basis_;
  
  VField*                      vfield_;

  int basis_order_;
  int mesh_dimensionality_;
}; 


template<class FIELD>
class VGenericField : public VField {
  public:
    VGenericField(FIELD* field, VFData* vfdata)
    {
      field_ = field;
      pm_ = field;
      vfdata_ = vfdata;
      mesh_ = field_->mesh().get_rep();
      vmesh_ = mesh_->vmesh();
      basis_order_ = field->basis_order();
      number_of_nodes_ = field->get_basis().number_of_mesh_vertices();
      number_of_enodes_ = field->get_basis().number_of_vertices() - number_of_nodes_;
      element_dim_ = field->get_basis().domain_dimension();
      element_dofs_ = field->get_basis().dofs();
      data_type_ = find_type_name((typename FIELD::value_type*)0);
      for (size_t j=0; j<data_type_.size(); j++) 
        if(data_type_[j] == '_') data_type_[j] = ' ';
        
      //! Create a fast way of checking scalar/pair/vector/tensor
      is_scalar_ = false;
      is_vector_ = false;
      is_tensor_ = false;
      is_pair_ = false;
      
      if (data_type_.substr(0,6) == "Vector") is_vector_ = true;
      else if (data_type_.substr(0,6) == "Tensor") is_tensor_ = true;
      else if (data_type_.substr(0,4) == "Pair") is_pair_ = true;
      else if (field->basis_order() > -1) is_scalar_ = true;
    }
    
    ~VGenericField()
    {
      if (vfdata_) delete vfdata_;
    }
    
};

template <class Mesh, class Basis, class FData>
void
GenericField<Mesh, Basis, FData>::resize_fdata()
{
  if (basis_order_ == 0 && mesh_dimensionality_ == 3)
  {
    typename mesh_type::Cell::size_type ssize;
    mesh_->synchronize(Mesh::CELLS_E);
    mesh_->size(ssize);
    fdata().resize(ssize);
  } 
  else if (basis_order_ == 0 && mesh_dimensionality_ == 2)
  {
    typename mesh_type::Face::size_type ssize;
    mesh_->synchronize(Mesh::FACES_E);
    mesh_->size(ssize);
    fdata().resize(ssize);
  } 
  else if (basis_order_ == 0 && mesh_dimensionality_ == 1)
  {
    typename mesh_type::Edge::size_type ssize;
    mesh_->synchronize(Mesh::EDGES_E);
    mesh_->size(ssize);
    fdata().resize(ssize);
  } 
  else if (basis_order_ == -1)
  {
    // do nothing (really, we want to resize to zero)
  }
  else if (basis_order_ == 1)
  {
    typename mesh_type::Node::size_type ssize;
    mesh_->synchronize(Mesh::NODES_E);
    mesh_->size(ssize);
    fdata().resize(ssize);
  }
  else
  {
    typename mesh_type::Node::size_type ssize;
    typename mesh_type::Edge::size_type lsize;
    mesh_->synchronize(Mesh::NODES_E|Mesh::ENODES_E);
    mesh_->size(ssize);
    mesh_->size(lsize);
    fdata().resize(ssize);
    basis_.resize_node_values(lsize);  // If basis has lagrangian nodes resize this one as well
  }
}

// PIO
const int GENERICFIELD_VERSION = 3;


template <class Mesh, class Basis, class FData>
Persistent *
GenericField<Mesh, Basis, FData>::maker()
{
  return scinew GenericField<Mesh, Basis, FData>;
}

template <class Mesh, class Basis, class FData>
FieldHandle
GenericField<Mesh, Basis, FData>::field_maker()
{
  return FieldHandle(scinew GenericField<Mesh, Basis, FData>());
}


template <class Mesh, class Basis, class FData>
FieldHandle
GenericField<Mesh, Basis, FData>::field_maker_mesh(MeshHandle mesh)
{
  mesh_handle_type mesh_handle = dynamic_cast<mesh_type *>(mesh.get_rep());
  if (mesh_handle.get_rep()) return FieldHandle(scinew GenericField<Mesh, Basis, FData>(mesh_handle));
  else return FieldHandle(0);
}



template <class Mesh, class Basis, class FData>
PersistentTypeID 
GenericField<Mesh, Basis, FData>::type_id(type_name(-1), "Field", maker);

template <class Mesh, class Basis, class FData>
FieldTypeID
GenericField<Mesh, Basis, FData>::field_id(type_name(-1),field_maker,field_maker_mesh);


template <class Mesh, class Basis, class FData>
void GenericField<Mesh, Basis, FData>::io(Piostream& stream)
{
  // we need to pass -1 to type_name() on SGI to fix a compile bug
  int version = stream.begin_class(type_name(-1), GENERICFIELD_VERSION);
  if (stream.backwards_compat_id()) {
    version = stream.begin_class(type_name(-1), GENERICFIELD_VERSION);
  }
  Field::io(stream);
  if (stream.error()) return;

  if (version < 2)
    mesh_->io(stream);
  else
    Pio(stream, mesh_);
    mesh_->freeze();
  if (version >= 3) { 
    basis_.io(stream);
  }
  Pio(stream, fdata_);
  freeze();

  if (stream.backwards_compat_id()) {
    stream.end_class();
  }
  stream.end_class();
  
  // A new mesh is associated with it
  if (stream.reading())
  {
    vfield_->update_mesh_pointer(mesh_.get_rep());
  }
}


template <class Mesh, class Basis, class FData>
GenericField<Mesh, Basis, FData>::GenericField() : 
  Field(),
  mesh_(mesh_handle_type(scinew mesh_type())),
  fdata_(0),
  vfield_(0),
  basis_order_(0),
  mesh_dimensionality_(-1)
{
  basis_order_ = basis_order();
  if (mesh_.get_rep()) mesh_dimensionality_ = mesh_->dimensionality();
  
  if (basis_order_ != -1 && mesh_.get_rep()) resize_fdata();
  
  VFData* vfdata = CreateVFData(fdata(),get_basis().get_nodes(),get_basis().get_derivs());
  if (vfdata)
  {  
    vfield_ = scinew VGenericField<GenericField<Mesh,Basis,FData> >(this, vfdata);
  }
}

template <class Mesh, class Basis, class FData>
GenericField<Mesh, Basis, FData>::GenericField(const GenericField& copy) : 
  Field(),
  mesh_(copy.mesh_),
  fdata_(copy.fdata_),
  basis_(copy.basis_),
  vfield_(0),
  basis_order_(copy.basis_order_),
  mesh_dimensionality_(copy.mesh_dimensionality_)
{
  VFData* vfdata = CreateVFData(fdata(),get_basis().get_nodes(),get_basis().get_derivs());
  if (vfdata)
  {  
    vfield_ = scinew VGenericField<GenericField<Mesh,Basis,FData> >(this, vfdata);
  }
}


template <class Mesh, class Basis, class FData>
GenericField<Mesh, Basis, FData>::GenericField(mesh_handle_type mesh) : 
  Field(),
  mesh_(mesh),
  fdata_(0),
  vfield_(0),
  basis_order_(0),
  mesh_dimensionality_(-1)
{
  basis_order_ = basis_order();
  if (mesh_.get_rep()) mesh_dimensionality_ = mesh_->dimensionality();
 
  if (basis_order_ != -1 && mesh_.get_rep()) resize_fdata();
  
  VFData* vfdata = CreateVFData(fdata(),get_basis().get_nodes(),get_basis().get_derivs());
  if (vfdata)
  {  
    vfield_ = scinew VGenericField<GenericField<Mesh,Basis,FData> >(this, vfdata);
  }  
}


template <class Mesh, class Basis, class FData>
GenericField<Mesh, Basis, FData>::~GenericField()
{
  if (vfield_) delete vfield_;
}

template <class Mesh, class Basis, class FData>
GenericField<Mesh, Basis, FData> *
GenericField<Mesh, Basis, FData>::clone() const
{
  return new GenericField<Mesh, Basis, FData>(*this);
}

template <class Mesh, class Basis, class FData>
MeshHandle
GenericField<Mesh, Basis, FData>::mesh() const
{
  return MeshHandle(mesh_.get_rep());
}

template <class Mesh, class Basis, class FData>
VMesh*
GenericField<Mesh, Basis, FData>::vmesh() const
{
  if (mesh_.get_rep()) return mesh_->vmesh();
  return (0);
}

template <class Mesh, class Basis, class FData>
VField*
GenericField<Mesh, Basis, FData>::vfield() const
{
  return (vfield_);
}

template <class Mesh, class Basis, class FData>
void
GenericField<Mesh, Basis, FData>::mesh_detach()
{
  thaw();
  mesh_.detach();
  mesh_->thaw();
  vfield_->update_mesh_pointer(mesh_.get_rep());
}


// Turn off warning for CHECKARRAYBOUNDS
#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma set woff 1183 1506
#endif

template <class Mesh, class Basis, class FData>
bool
GenericField<Mesh, Basis, FData>::value(value_type &val, 
				 typename mesh_type::Node::index_type i) const
{
  ASSERTL3(basis_order_ >= 1 || mesh_dimensionality_ == 0);
  CHECKARRAYBOUNDS(static_cast<index_type>(i), 
                   static_cast<index_type>(0), 
                   static_cast<index_type>(fdata_.size()));
  if (!(basis_order_ == 1 || mesh_dimensionality_ == 0)) return false;
  val = fdata_[i]; return true;
}

template <class Mesh, class Basis, class FData>
bool
GenericField<Mesh, Basis, FData>::value(value_type &val, 
				 typename mesh_type::Edge::index_type i) const
{
  ASSERTL3(basis_order_ == 0 && mesh_dimensionality_ == 1);
  CHECKARRAYBOUNDS(static_cast<index_type>(i), 
                   static_cast<index_type>(0), 
                   static_cast<index_type>(fdata_.size()));
  if (basis_order_ != 0) return false;
  val = fdata_[i]; return true;
}

template <class Mesh, class Basis, class FData>
bool
GenericField<Mesh, Basis, FData>::value(value_type &val, 
				 typename mesh_type::Face::index_type i) const
{
  ASSERTL3(basis_order_ == 0 && mesh_dimensionality_ == 2);
  CHECKARRAYBOUNDS(static_cast<index_type>(i), 
                   static_cast<index_type>(0), 
                   static_cast<index_type>(fdata_.size()));
  if (basis_order_ != 0) return false;
  val = fdata_[i]; return true;
}

template <class Mesh, class Basis, class FData>
bool
GenericField<Mesh, Basis, FData>::value(value_type &val, 
				 typename mesh_type::Cell::index_type i) const
{
  ASSERTL3(basis_order_ == 0 && mesh_dimensionality_ == 3);
  CHECKARRAYBOUNDS(static_cast<index_type>(i), 
                   static_cast<index_type>(0), 
                   static_cast<index_type>(fdata_.size()));
  if (basis_order_ != 0) return false;
  val = fdata_[i]; return true;
} 

//! Required interface to support Field Concept.
template <class Mesh, class Basis, class FData>
void
GenericField<Mesh, Basis, FData>::set_value(const value_type &val, 
				      typename mesh_type::Node::index_type i)
{
  ASSERTL3(basis_order_ >= 1 || mesh_dimensionality_ == 0);
  CHECKARRAYBOUNDS(static_cast<index_type>(i), 
                   static_cast<index_type>(0), 
                   static_cast<index_type>(fdata_.size()));
  fdata_[i] = val;
}
template <class Mesh, class Basis, class FData>
void
GenericField<Mesh, Basis, FData>::set_value(const value_type &val, 
			       typename mesh_type::Edge::index_type i)
{
  ASSERTL3(basis_order_ == 0 && mesh_dimensionality_ == 1);
  CHECKARRAYBOUNDS(static_cast<index_type>(i), 
                   static_cast<index_type>(0), 
                   static_cast<index_type>(fdata_.size()));
  fdata_[i] = val;
}
template <class Mesh, class Basis, class FData>
void
GenericField<Mesh, Basis, FData>::set_value(const value_type &val, 
				      typename mesh_type::Face::index_type i)
{
  ASSERTL3(basis_order_ == 0 && mesh_dimensionality_ == 2);
  CHECKARRAYBOUNDS(static_cast<index_type>(i), 
                   static_cast<index_type>(0), 
                   static_cast<index_type>(fdata_.size()));
  fdata_[i] = val;
}
template <class Mesh, class Basis, class FData>
void
GenericField<Mesh, Basis, FData>::set_value(const value_type &val, 
				      typename mesh_type::Cell::index_type i)
{
  ASSERTL3(basis_order_ == 0 && mesh_dimensionality_ == 3);
  CHECKARRAYBOUNDS(static_cast<index_type>(i), 
                   static_cast<index_type>(0), 
                   static_cast<index_type>(fdata_.size()));
  fdata_[i] = val;
}

template <class Mesh, class Basis, class FData>
const typename GenericField<Mesh, Basis, FData>::value_type &
GenericField<Mesh, Basis, FData>::
value(typename mesh_type::Node::index_type i) const
{
  ASSERTL3(basis_order_ >= 1 || mesh_dimensionality_ == 0);
  CHECKARRAYBOUNDS(static_cast<index_type>(i), 
                   static_cast<index_type>(0), 
                   static_cast<index_type>(fdata_.size()));
  return fdata_[i];
}

template <class Mesh, class Basis, class FData>
const typename GenericField<Mesh, Basis, FData>::value_type &
GenericField<Mesh, Basis, FData>::
value(typename mesh_type::Edge::index_type i) const
{
  ASSERTL3(basis_order_ == 0 && mesh_dimensionality_ == 1);
  CHECKARRAYBOUNDS(static_cast<index_type>(i), 
                   static_cast<index_type>(0), 
                   static_cast<index_type>(fdata_.size()));
  return fdata_[i];
}
template <class Mesh, class Basis, class FData>
const typename GenericField<Mesh, Basis, FData>::value_type &
GenericField<Mesh, Basis, FData>::
value(typename mesh_type::Face::index_type i) const 
{
  ASSERTL3(basis_order_ == 0 && mesh_dimensionality_ == 2);
  CHECKARRAYBOUNDS(static_cast<index_type>(i), 
                   static_cast<index_type>(0), 
                   static_cast<index_type>(fdata_.size()));
  return fdata_[i];
}
template <class Mesh, class Basis, class FData>
const typename GenericField<Mesh, Basis, FData>::value_type &
GenericField<Mesh, Basis, FData>::
value(typename mesh_type::Cell::index_type i) const 
{
  ASSERTL3(basis_order_ == 0 && mesh_dimensionality_ == 3);
  CHECKARRAYBOUNDS(static_cast<index_type>(i), 
                   static_cast<index_type>(0), 
                   static_cast<index_type>(fdata_.size()));
  return fdata_[i];
}

// Reenable warning for CHECKARRAYBOUNDS
#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1183 1506
#endif

template <class Mesh, class Basis, class FData>
typename GenericField<Mesh, Basis, FData>::fdata_type &
GenericField<Mesh, Basis, FData>::fdata()
{
  return fdata_;
}

template <class Mesh, class Basis, class FData>
const typename GenericField<Mesh, Basis, FData>::fdata_type&
GenericField<Mesh, Basis, FData>::fdata() const
{
  return fdata_;
}

template <class Mesh, class Basis, class FData>
const typename GenericField<Mesh, Basis, FData>::mesh_handle_type &
GenericField<Mesh, Basis, FData>::get_typed_mesh() const
{
  return mesh_;
}

template <class Mesh, class Basis, class FData>
const string GenericField<Mesh, Basis, FData>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 3);
  if (n == -1)
  {
    static const string name = type_name(0) + FTNS + type_name(1) + FTNM
      + type_name(2) + FTNM + type_name(3) + FTNE;
    return name;
  }
  else if (n == 0)
  {
    static const string nm("GenericField");
    return nm;
  }
  else if (n == 1)
  {
    return find_type_name((Mesh *)0);
  }
  else if (n == 2)
  {
    return find_type_name((Basis *)0);
  }
  else
  {
    return find_type_name((FData *)0);
  }
}

template <class Mesh, class Basis, class FData>
const TypeDescription *
GenericField<Mesh, Basis, FData>::get_type_description(td_info_e td) const
{
  static string name(type_name(0));
  static string namesp("SCIRun");
  static string path(__FILE__);
  const TypeDescription *sub1 = SCIRun::get_type_description((Mesh*)0);
  const TypeDescription *sub2 = SCIRun::get_type_description((Basis*)0);
  const TypeDescription *sub3 = SCIRun::get_type_description((FData*)0);

  switch (td) {
  default:
  case FULL_TD_E:
    {
      static TypeDescription* tdn1 = 0;
      if (tdn1 == 0) {
	TypeDescription::td_vec *subs = scinew TypeDescription::td_vec(3);
	(*subs)[0] = sub1;
	(*subs)[1] = sub2;
	(*subs)[2] = sub3;
	tdn1 = scinew TypeDescription(name, subs, path, namesp, 
				      TypeDescription::FIELD_E);
      } 
      return tdn1;
    }
  case FIELD_NAME_ONLY_E:
    {
      static TypeDescription* tdn0 = 0;
      if (tdn0 == 0) {
	tdn0 = scinew TypeDescription(name, 0, path, namesp, 
				      TypeDescription::FIELD_E);
      }
      return tdn0;
    }
  case MESH_TD_E:
    {
      return sub1;
    }
  case BASIS_TD_E:
    {
      return sub2;
    }
  case FDATA_TD_E:
    {
      return sub3;
    }
  };
}


// Unfortunately even for the linear case gradients are not necessarily 
// constant. Hence provide a means of getting the real gradient at a
// location. This method is similar to interpolate but returns the local
// gradient instead.

template <class Mesh, class Basis, class FData>
void
GenericField<Mesh, Basis, FData>::
gradient(vector<value_type>& grad, const vector<double>& coords, 
                     typename mesh_type::Elem::index_type ci) const
{
  // Resize the output data properly
  grad.resize(3);

  // get the mesh Jacobian for the element
  // for the location where we want to calculate the gradient.
  double Ji[9];
  mesh_->inverse_jacobian(coords,ci,Ji);

  // Calculate the gradient in local coordinates
  int dim = basis_.domain_dimension();
  if (dim == 3)
  {
    StackVector<value_type,3> g;
    ElemData<field_type> fcd(*this, ci);
    basis_.derivate(coords, fcd, g);  

    grad[0] = static_cast<value_type>(g[0]*Ji[0])+static_cast<value_type>(g[1]*Ji[1])+static_cast<value_type>(g[2]*Ji[2]);
    grad[1] = static_cast<value_type>(g[0]*Ji[3])+static_cast<value_type>(g[1]*Ji[4])+static_cast<value_type>(g[2]*Ji[5]);
    grad[2] = static_cast<value_type>(g[0]*Ji[6])+static_cast<value_type>(g[1]*Ji[7])+static_cast<value_type>(g[2]*Ji[8]);  
  }
  else if (dim == 2)
  {
    StackVector<value_type,2> g;
    ElemData<field_type> fcd(*this, ci);
    basis_.derivate(coords, fcd, g);  

    grad[0] = static_cast<value_type>(g[0]*Ji[0])+static_cast<value_type>(g[1]*Ji[1]);
    grad[1] = static_cast<value_type>(g[0]*Ji[3])+static_cast<value_type>(g[1]*Ji[4]);
    grad[2] = static_cast<value_type>(g[0]*Ji[6])+static_cast<value_type>(g[1]*Ji[7]);  
  }
  else if (dim == 1)
  {
    StackVector<value_type,1> g;
    ElemData<field_type> fcd(*this, ci);
    basis_.derivate(coords, fcd, g);  

    grad[0] = static_cast<value_type>(g[0]*Ji[0]);
    grad[1] = static_cast<value_type>(g[0]*Ji[3]);
    grad[2] = static_cast<value_type>(g[0]*Ji[6]);
  }
  else
  {
    ASSERTFAIL("Tried to compute a gradient on a domain with a dimension smaller than 1 or a dimension larger than 3");
  }
}


template <class Mesh, class Basis, class FData>
const TypeDescription *
GenericField<Mesh, Basis, FData>::order_type_description() const
{
  const int order = basis_order_;
  const int dim = mesh_dimensionality_;
  if (order == 0 && dim == 3) 
  {
    return Mesh::cell_type_description();
  } 
  else if (order == 0 && dim == 2) 
  {
    return Mesh::face_type_description();
  } 
  else if (order == 0 && dim == 1) 
  {
    return Mesh::edge_type_description();
  } 
  else 
  {
    return Mesh::node_type_description();
  }
}

//! These ended up here, due to the problem with the include order in get_typedescription.
//! Once we have dismanteled that system this can go back to VFData
template<class T, class MESH>
inline VFData* CreateVFData(FData2d<T,MESH>& fdata, std::vector<T>& lfdata, std::vector<std::vector<T> >& hfdata)
{
  return (CreateVFData(static_cast<Array2<T>& >(fdata),lfdata,hfdata));
}

template<class T, class MESH>
inline VFData* CreateVFData(FData3d<T,MESH>& fdata, std::vector<T>& lfdata, std::vector<std::vector<T> >& hfdata)
{
  return (CreateVFData(static_cast<Array3<T>& >(fdata),lfdata,hfdata));
}

} // end namespace SCIRun






#endif // Datatypes_GenericField_h
