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



#ifndef CORE_DATATYPES_FIELD_H
#define CORE_DATATYPES_FIELD_H 1

#include <Core/Datatypes/PropertyManager.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/VField.h>
#include <Core/Containers/LockingHandle.h>
#include <Core/Datatypes/FieldInterface.h>
#include <Core/Util/ProgressReporter.h>
#include <Core/Util/DynamicCompilation.h>

#include <Core/Datatypes/share.h>

namespace SCIRun {
 
typedef LockingHandle<ScalarFieldInterface> ScalarFieldInterfaceHandle;
typedef LockingHandle<VectorFieldInterface> VectorFieldInterfaceHandle;
typedef LockingHandle<TensorFieldInterface> TensorFieldInterfaceHandle;

class SCISHARE Field: public PropertyManager
{
public:
  //! Constructor / Destructor
  Field();
  virtual ~Field();
  
  //! Clone field will generate a pointer to a new copy
  virtual Field *clone() const = 0;

  //! Get pointers to associated structures
  //! mesh -> handle to mesh
  //! vmesh -> handle to virtual mesh interface
  //! field -> handle to this object
  //! vfield -> handle to virtual field interface
  virtual MeshHandle mesh() const = 0;
  inline  FieldHandle field() { return (this); }

  virtual VMesh* vmesh()   const = 0;
  virtual VField* vfield() const = 0;
  
  //! Detach the mesh from the field, if needed make a new copy of it.
  virtual void mesh_detach() = 0;

  //! The order of the field: we could get this one from the type_description
  virtual int basis_order() const = 0;

  //! Type Description to retrieve information on the actual type of the field
  enum  td_info_e {
    FULL_TD_E,
    FIELD_NAME_ONLY_E,
    MESH_TD_E,
    BASIS_TD_E,
    FDATA_TD_E
  };

  virtual const TypeDescription *order_type_description() const = 0; // <- SHOULD REMOVE THIS ONE
  virtual const TypeDescription* get_type_description(td_info_e td = FULL_TD_E) const = 0; 
  
  //! Required interfaces (These will soon be obsolete)
  virtual ScalarFieldInterfaceHandle query_scalar_interface(
						      ProgressReporter* = 0); // <- SHOULD REMOVE THIS ONE
  virtual VectorFieldInterfaceHandle query_vector_interface(
						      ProgressReporter* = 0); // <- SHOULD REMOVE THIS ONE
  virtual TensorFieldInterfaceHandle query_tensor_interface(
						      ProgressReporter* = 0); // <- SHOULD REMOVE THIS ONE

  //! Persistent I/O.
  static  PersistentTypeID type_id;
  virtual void io(Piostream &stream);
  
  virtual void resize_fdata();

  //! A simple test to see whether field has virtual interface
  //! By now most fields shouls have the virtual interface, so this call will soon
  //! be obsolete
  inline bool has_virtual_interface() { return ((vfield() != 0)&&(vmesh() != 0)); }  
};


class SCISHARE FieldTypeID {
  public:
    // Constructor
    FieldTypeID(const string& type, 
                FieldHandle (*field_maker)(),
                FieldHandle (*field_maker_mesh)(MeshHandle));
    
    string type;
    FieldHandle (*field_maker)();
    FieldHandle (*field_maker_mesh)(MeshHandle);
};

SCISHARE FieldHandle CreateField(string type);
SCISHARE FieldHandle CreateField(string type,MeshHandle mesh);

}

#endif

