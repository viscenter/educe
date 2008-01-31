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

#ifndef CORE_DATATYPES_FIELDINFORMATION
#define CORE_DATATYPES_FIELDINFORMATION 1

#include <string>
#include <Core/Datatypes/Field.h>

#include <Core/Datatypes/share.h>

namespace SCIRun {

class SCISHARE FieldTypeInformation;
class SCISHARE FieldInformation;

class SCISHARE FieldTypeInformation {

  public:

    bool        is_isomorphic();
    bool        is_nonlinear();
    bool        is_linear();
    
    bool        is_nodata();
    bool        is_constantdata();
    bool        is_lineardata();
    bool        is_nonlineardata();
    bool        is_quadraticdata();
    bool        is_cubicdata();
    int         field_basis_order();
    
    bool        is_constantmesh();
    bool        is_linearmesh();
    bool        is_nonlinearmesh();
    bool        is_quadraticmesh();
    bool        is_cubicmesh();
    int         mesh_basis_order();
  
    bool        is_tensor();
    bool        is_vector();
    bool        is_scalar();
    bool        is_integer();
    bool        is_double();
    bool        is_float();
    bool        is_longlong();
    bool        is_long();
    bool        is_int();
    bool        is_short();
    bool        is_char();
    bool        is_unsigned_longlong();
    bool        is_unsigned_long();
    bool        is_unsigned_int();
    bool        is_unsigned_short();
    bool        is_unsigned_char();
    bool        is_dvt(); // double vector tensor
    bool        is_svt(); // scalar vector tensor
    
    bool        is_regularmesh();
    bool        is_irregularmesh();
    bool        is_structuredmesh();
    bool        is_unstructuredmesh();
    
    bool        is_pointcloud();
    bool        is_scanline();
    bool        is_image();
    bool        is_latvol();
    bool        is_curve();
    bool        is_trisurf();
    bool        is_quadsurf();
    bool        is_tetvol();
    bool        is_prismvol();
    bool        is_hexvol();
    bool        is_structcurve();    
    bool        is_structquadsurf();    
    bool        is_structhexvol();
    
    bool        is_point();
    bool        is_line();
    bool        is_surface();
    bool        is_volume();
    
    bool        is_pnt_element();
    bool        is_crv_element();
    bool        is_tri_element();
    bool        is_quad_element();
    bool        is_tet_element();
    bool        is_prism_element();
    bool        is_hex_element();

    void insert_field_type_information(Field* field);
  protected:
  
    // type names
    std::string field_type;
    std::string mesh_type;
    std::string mesh_basis_type;
    std::string point_type;
    std::string basis_type;
    std::string data_type;
    std::string container_type;
    
    // include files for dynamic compilation
    std::string field_type_h;
    std::string mesh_type_h;
    std::string mesh_basis_type_h;
    std::string point_type_h;
    std::string basis_type_h;
    std::string data_type_h;
    std::string container_type_h;
};


class SCISHARE FieldInformation : public FieldTypeInformation {
  
  public:
    FieldInformation(std::string meshtype,std::string meshbasis,
                                  std::string databasis, std::string datatype);
    FieldInformation(std::string meshtype,std::string basis, std::string datatype);
    
    FieldInformation(FieldHandle handle);
    FieldInformation(Field* field);
    
  
    std::string get_field_type();
    void        set_field_type(std::string);

    std::string get_mesh_type();
    std::string get_mesh_type_id();
    void        set_mesh_type(std::string);
    
    std::string get_mesh_basis_type();
    void        set_mesh_basis_type(std::string);

    std::string get_point_type();
    void        set_point_type(std::string);

    std::string get_basis_type();
    void        set_basis_type(std::string);

    // alternative way of setting data_basis
    std::string get_data_basis_type() { return (get_basis_type()); }
    void        set_data_basis_type(std::string s) { set_basis_type(s); }

    std::string get_data_type();
    void        set_data_type(std::string);

    std::string get_container_type();
    void        set_container_type(std::string);

    std::string get_field_name();
    std::string get_field_type_id();
    std::string get_field_filename();
    
    void fill_compile_info(CompileInfoHandle &ci);
  
    bool        make_nodata();
    bool        make_constantdata();
    bool        make_lineardata();
    bool        make_quadraticdata();
    bool        make_cubicdata();    

    bool        make_constantmesh(); // for pointcloud only
    bool        make_linearmesh();
    bool        make_quadraticmesh();
    bool        make_cubicmesh();    

    bool        make_char();
    bool        make_unsigned_char();
    bool        make_short();
    bool        make_unsigned_short();
    bool        make_int();
    bool        make_unsigned_int();
    bool        make_long();
    bool        make_unsigned_long();
    bool        make_long_long();
    bool        make_unsigned_long_long();
    bool        make_float();    
    bool        make_scalar();
    bool        make_double();
    bool        make_vector();
    bool        make_tensor();
    
    bool        make_pointcloudmesh();
    bool        make_scanlinemesh();
    bool        make_imagemesh();
    bool        make_latvolmesh();
    bool        make_structcurvemesh();
    bool        make_structquadsurfmesh();
    bool        make_structhexvolmesh();
    bool        make_curvemesh();
    bool        make_trisurfmesh();
    bool        make_quadsurfmesh();
    bool        make_tetvolmesh();
    bool        make_prismvolmesh();
    bool        make_hexvolmesh();

    bool        make_unstructuredmesh();
    
    // This function tests whether the field and the mesh have a virtual interface
    // and whether the mesh can be instantiated by CreateField
    bool        has_virtual_interface();
    
    bool        operator==(const FieldInformation&) const;

};







SCISHARE FieldHandle CreateField(std::string meshtype, std::string basistype, std::string datatype);
SCISHARE FieldHandle CreateField(std::string meshtype, std::string meshbasistype, std::string databasistype, std::string datatype);

SCISHARE FieldHandle CreateField(FieldInformation &info);
SCISHARE FieldHandle CreateField(FieldInformation &info,MeshHandle mesh);

SCISHARE MeshHandle CreateMesh(FieldInformation &info);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,Mesh::size_type x);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,Mesh::size_type x,const Point& min,const Point& max);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,Mesh::size_type x,Mesh::size_type y);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,Mesh::size_type x,Mesh::size_type y,const Point& min,const Point& max);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,Mesh::size_type x,Mesh::size_type y,Mesh::size_type z);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,Mesh::size_type x,Mesh::size_type y,Mesh::size_type z,const Point& min,const Point& max);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,vector<Mesh::size_type> x);
SCISHARE MeshHandle CreateMesh(FieldInformation &info,vector<Mesh::size_type> x,const Point& min,const Point& max);


inline bool UseIntegerInterface(FieldInformation &info) 
{ return(info.is_integer()); }

inline bool UseIntegerInterface(FieldInformation &info,FieldInformation &info2)
{ return(info.is_integer()&info2.is_integer()); }

inline bool UseIntegerInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3)
{ return(info.is_integer()&info2.is_integer()&info3.is_integer()); }

inline bool UseIntegerInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4)
{ return(info.is_integer()&info2.is_integer()&info3.is_integer()&info4.is_integer()); }

inline bool UseIntegerInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4, FieldInformation &info5)
{ return(info.is_integer()&info2.is_integer()&info3.is_integer()&info4.is_integer()&info5.is_integer()); }



inline bool UseScalarInterface(FieldInformation &info) 
{ return(info.is_scalar()); }

inline bool UseScalarInterface(FieldInformation &info,FieldInformation &info2)
{ return(info.is_scalar()&info2.is_scalar()); }

inline bool UseScalarInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3)
{ return(info.is_scalar()&info2.is_scalar()&info3.is_scalar()); }

inline bool UseScalarInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4)
{ return(info.is_scalar()&info2.is_scalar()&info3.is_scalar()&info4.is_scalar()); }

inline bool UseScalarInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4, FieldInformation &info5)
{ return(info.is_scalar()&info2.is_scalar()&info3.is_scalar()&info4.is_scalar()&info5.is_scalar()); }


inline bool UseVectorInterface(FieldInformation &info) 
{ return(info.is_vector()); }

inline bool UseVectorInterface(FieldInformation &info,FieldInformation &info2)
{ return(info.is_vector()&info2.is_vector()); }

inline bool UseVectorInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3)
{ return(info.is_vector()&info2.is_vector()&info3.is_vector()); }

inline bool UseVectorInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4)
{ return(info.is_vector()&info2.is_vector()&info3.is_vector()&info4.is_vector()); }

inline bool UseVectorInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4, FieldInformation &info5)
{ return(info.is_vector()&info2.is_vector()&info3.is_vector()&info4.is_vector()&info5.is_vector()); }


inline bool UseTensorInterface(FieldInformation &info) 
{ return(info.is_tensor()); }

inline bool UseTensorInterface(FieldInformation &info,FieldInformation &info2)
{ return(info.is_tensor()&info2.is_tensor()); }

inline bool UseTensorInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3)
{ return(info.is_tensor()&info2.is_tensor()&info3.is_tensor()); }

inline bool UseTensorInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4)
{ return(info.is_tensor()&info2.is_tensor()&info3.is_tensor()&info4.is_tensor()); }

inline bool UseTensorInterface(FieldInformation &info,FieldInformation &info2,FieldInformation &info3,
                        FieldInformation &info4, FieldInformation &info5)
{ return(info.is_tensor()&info2.is_tensor()&info3.is_tensor()&info4.is_tensor()&info5.is_tensor()); }

} // end namespace

#endif

