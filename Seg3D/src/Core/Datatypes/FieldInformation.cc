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

#include <Core/Datatypes/FieldInformation.h>

namespace SCIRun {

void
FieldTypeInformation::insert_field_type_information(Field* field)
{
  std::string temp;
  // Get the name of the GenericField class
  // This should give GenericField
  
  if (field == 0) return;
  field_type = field->get_type_description(Field::FIELD_NAME_ONLY_E)->get_name();
  field_type_h = field->get_type_description(Field::FIELD_NAME_ONLY_E)->get_h_file_path(); 
  // Analyze the mesh type
  // This will result in
  // mesh_type, mesh_basis_type, and point_type
  const TypeDescription* mesh_td = field->get_type_description(Field::MESH_TD_E);
  TypeDescription::td_vec* mesh_sub_td = mesh_td->get_sub_type();
  const TypeDescription* mesh_basis_td = (*mesh_sub_td)[0]; 
  TypeDescription::td_vec* mesh_basis_sub_td = mesh_basis_td->get_sub_type();
  const TypeDescription* point_td = (*mesh_basis_sub_td)[0]; 
  
  temp = mesh_td->get_name(); 
  mesh_type = temp.substr(0,temp.find("<"));
  mesh_type_h = mesh_td->get_h_file_path();
  temp = mesh_basis_td->get_name(); 
  mesh_basis_type = temp.substr(0,temp.find("<"));
  mesh_basis_type_h = mesh_basis_td->get_h_file_path();
  point_type = point_td->get_name();
  point_type_h = point_td->get_h_file_path();
  
  // Analyze the basis type
  
  const TypeDescription* basis_td = field->get_type_description(Field::BASIS_TD_E);
  TypeDescription::td_vec* basis_sub_td = basis_td->get_sub_type();
  const TypeDescription* data_td = (*basis_sub_td)[0]; 
  
  temp = basis_td->get_name(); 
  basis_type = temp.substr(0,temp.find("<"));
  basis_type_h = basis_td->get_h_file_path();
  data_type = data_td->get_name();
  data_type_h = data_td->get_h_file_path();

  const TypeDescription* container_td = field->get_type_description(Field::FDATA_TD_E);
  temp = container_td->get_name(); 
  container_type = temp.substr(0,temp.find("<"));
  container_type_h = container_td->get_h_file_path();

}


FieldInformation::FieldInformation(std::string meshtype,std::string meshbasis,std::string databasis, std::string datatype)
{
  set_field_type("GenericField");
  set_point_type("Point");
  set_container_type("vector");
  set_data_type(datatype);
  set_mesh_type(meshtype);
  set_mesh_basis_type(meshbasis);
  set_data_basis_type(databasis);
}
    

FieldInformation::FieldInformation(std::string meshtype,std::string basis, std::string datatype)
{
  FieldInformation(meshtype,basis,basis,datatype);
}


FieldInformation::FieldInformation(FieldHandle handle)
{
  insert_field_type_information(handle.get_rep());
}


FieldInformation::FieldInformation(Field* field)
{
  insert_field_type_information(field);
}

std::string
FieldInformation::get_field_type()
{
  return(field_type);
}

void
FieldInformation::set_field_type(std::string type)
{
  field_type = type;
  field_type_h = "";
  if (type == "GenericField") field_type_h = "Core/Datatypes/GenericField.h";
  if (type == "MultiLevelField") field_type_h = "Core/Datatypes/MultiLevelField.h";
}

std::string
FieldInformation::get_mesh_type()
{
  return(mesh_type);
}

void
FieldInformation::set_mesh_type(std::string type)
{
  mesh_type = type;
  mesh_type_h = "";
  if (type == "ScanlineMesh")
  { 
    field_type_h = "Core/Datatypes/ScanlineMesh.h";
    if (mesh_basis_type.find("Crv") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("CrvQuadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("CrvCubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("NoDataBasis");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("ConstantBasis");
      else set_mesh_basis_type("CrvLinearLgn");      
    }
    if (basis_type.find("Crv") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("CrvQuadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("CrvCubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("CrvLinearLgn");      
    }
    set_container_type("vector");    
  }
  if (type == "ImageMesh") 
  {
    field_type_h = "Core/Datatypes/ImageMesh.h";
    if (mesh_basis_type.find("Quad") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("QuadBiquadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("QuadBicubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("NoDataBasis");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("ConstantBasis");
      else set_mesh_basis_type("QuadBilinearLgn");      
    }
    if (basis_type.find("Quad") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("QuadBiquadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("QuadBicubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("QuadBilinearLgn");      
    }
    set_container_type("FData2d");    
  }
  if (type == "LatVolMesh") 
  {
    field_type_h = "Core/Datatypes/LatVolMesh.h";
    if (mesh_basis_type.find("Hex") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("HexTriquadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("HexTricubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("NoDataBasis");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("ConstantBasis");
      else set_mesh_basis_type("HexTrilinearLgn");      
    }    
    if (basis_type.find("Hex") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("HexTriquadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("HexTricubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("HexTrilinearLgn");      
    }    
    set_container_type("FData3d");    
  }
  if (type == "MaskedLatVolMesh") 
  {
    field_type_h = "Core/Datatypes/MaskedLatVolMesh.h";
    if (mesh_basis_type.find("Hex") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("HexTriquadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("HexTricubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("NoDataBasis");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("ConstantBasis");
      else set_mesh_basis_type("HexTrilinearLgn");      
    }    
    if (basis_type.find("Hex") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("HexTriquadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("HexTricubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("HexTrilinearLgn");      
    }    
    set_container_type("FData3d");    
  }
  
  if (type == "StructCurveMesh") 
  {
    field_type_h = "Core/Datatypes/StructCurveMesh.h";
    if (mesh_basis_type.find("Crv") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("CrvQuadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("CrvCubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("NoDataBasis");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("ConstantBasis");
      else set_mesh_basis_type("CrvLinearLgn");      
    }
    if (basis_type.find("Crv") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("CrvQuadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("CrvCubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("CrvLinearLgn");      
    }  
    set_container_type("vector");    
  }
  
  if (type == "StructQuadSurfMesh")
  {
    field_type_h = "Core/Datatypes/StructQuadSurfMesh.h";
    if (mesh_basis_type.find("Quad") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("QuadBiquadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("QuadBicubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("NoDataBasis");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("ConstantBasis");
      else set_mesh_basis_type("QuadBilinearLgn");      
    }
    if (basis_type.find("Quad") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("QuadBiquadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("QuadBicubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("QuadBilinearLgn");      
    }  
    set_container_type("FData2d");    
  }
  
  if (type == "StructHexVolMesh") 
  {
    field_type_h = "Core/Datatypes/StructHexVolMesh.h";
    if (mesh_basis_type.find("Hex") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("HexTriquadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("HexTricubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("NoDataBasis");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("ConstantBasis");
      else set_mesh_basis_type("HexTrilinearLgn");      
    }    
    if (basis_type.find("Hex") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("HexTriquadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("HexTricubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("HexTrilinearLgn");      
    }        
    set_container_type("FData3d");    
  }
  
  if (type == "CurveMesh") 
  {
    field_type_h = "Core/Datatypes/CurveMesh.h";
    if (mesh_basis_type.find("Crv") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("CrvQuadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("CrvCubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("NoDataBasis");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("ConstantBasis");
      else set_mesh_basis_type("CrvLinearLgn");      
    }
    if (basis_type.find("Crv") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("CrvQuadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("CrvCubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("CrvLinearLgn");      
    }
    set_container_type("vector");            
  }
  
  if (type == "TriSurfMesh") 
  {
    field_type_h = "Core/Datatypes/TriSurfMesh.h";
    if (mesh_basis_type.find("Tri") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("TriQuadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("TriCubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("NoDataBasis");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("ConstantBasis");
      else set_mesh_basis_type("TriLinearLgn");      
    }
    if (basis_type.find("Tri") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("TriQuadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("TriCubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("TriLinearLgn");      
    }       
    set_container_type("vector");            
  }
  if (type == "QuadSurfMesh")
  {
    field_type_h = "Core/Datatypes/QuadSurfMesh.h";
    if (mesh_basis_type.find("Quad") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("QuadBiquadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("QuadBicubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("NoDataBasis");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("ConstantBasis");
      else set_mesh_basis_type("QuadBilinearLgn");      
    }
    if (basis_type.find("Quad") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("QuadBiquadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("QuadBicubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("QuadBilinearLgn");      
    }
    set_container_type("vector");                
  }
  
  if (type == "TetVolMesh")
  {
    field_type_h = "Core/Datatypes/TetVolMesh.h";
    if (mesh_basis_type.find("Tet") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("TetQuadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("TetCubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("NoDataBasis");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("ConstantBasis");
      else set_mesh_basis_type("TetLinearLgn");      
    }
    if (basis_type.find("Tet") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("TetQuadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("TetCubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("TetLinearLgn");      
    }  
    set_container_type("vector");                
  }
  
  if (type == "PrismVolMesh") 
  {
    field_type_h = "Core/Datatypes/PrismVolMesh.h";
    if (mesh_basis_type.find("Prism") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("PrismQuadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("PrismCubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("NoDataBasis");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("ConstantBasis");
      else set_mesh_basis_type("PrismLinearLgn");      
    }
    if (basis_type.find("Prism") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("PrismQuadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("PrismCubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("PrismLinearLgn");      
    }      
    set_container_type("vector");            
  }
  
  if (type == "HexVolMesh") 
  {
    field_type_h = "Core/Datatypes/HexVolMesh.h";
    if (mesh_basis_type.find("Hex") == std::string::npos)
    {
      if (mesh_basis_type.find("uadraticLgn") != std::string::npos) set_mesh_basis_type("HexTriquadraticLgn");
      else if (mesh_basis_type.find("ubicHmt") != std::string::npos) set_mesh_basis_type("HexTricubicHmt");
      else if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("NoDataBasis");
      else if (mesh_basis_type.find("Constant") != std::string::npos) set_mesh_basis_type("ConstantBasis");
      else set_mesh_basis_type("HexTrilinearLgn");      
    }    
    if (basis_type.find("Hex") == std::string::npos)
    {
      if (basis_type.find("uadraticLgn") != std::string::npos) set_basis_type("HexTriquadraticLgn");
      else if (basis_type.find("ubicHmt") != std::string::npos) set_basis_type("HexTricubicHmt");
      else if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
      else if (basis_type.find("Constant") != std::string::npos) set_basis_type("ConstantBasis");
      else set_basis_type("HexTrilinearLgn");      
    }        
    set_container_type("vector");            
  }
  
  if (type == "PointCloudMesh") 
  {
    field_type_h = "Core/Datatypes/PointCloudMesh.h";
    
    if (mesh_basis_type.find("NoData") != std::string::npos) set_mesh_basis_type("NoDataBasis");
    else set_mesh_basis_type("ConstantBasis");

    if (basis_type.find("NoData") != std::string::npos) set_basis_type("NoDataBasis");
    else set_basis_type("ConstantBasis");
    set_container_type("vector");            
  }
}

std::string
FieldInformation::get_mesh_basis_type()
{
  return(mesh_basis_type);
}

void
FieldInformation::set_mesh_basis_type(std::string type)
{
  if (type == "Linear" || type == "linear" || type == "linearmesh" || type == "LinearMesh" ) 
    { make_linearmesh(); return; }
  if (type == "Quadratic" || type == "quadratic" || type == "quadraticmesh" || type == "QuadraticMesh") 
    { make_quadraticmesh(); return; }
  if (type == "Cubic" || type == "cubic" || type == "cubicmesh" || type == "CubicMesh")
    { make_cubicmesh(); return; } 
  mesh_basis_type = type;
  
  // currently hard coded, due to lack of proper mechanism in the core
  mesh_basis_type_h = "";
  if (type == "NoDataBasis")         mesh_basis_type_h = "Core/Basis/NoData.h";
  if (type == "ConstantBasis")       mesh_basis_type_h = "Core/Basis/Constant.h";
  if (type == "CrvLinearLgn")        mesh_basis_type_h = "Core/Basis/CrvLinearLgn.h";
  if (type == "CrvQuadraticLgn")     mesh_basis_type_h = "Core/Basis/CrvQuadraticLgn.h";
  if (type == "CrvCubicHmt")         mesh_basis_type_h = "Core/Basis/CrvCubicHmt.h";
  if (type == "HexTrilinearLgn")     mesh_basis_type_h = "Core/Basis/HexTrilinearLgn.h";
  if (type == "HexTriquadraticLgn")  mesh_basis_type_h = "Core/Basis/HexTriquadraticLgn.h";
  if (type == "HexTricubicHmt")      mesh_basis_type_h = "Core/Basis/HexTricubicHmt.h";
  if (type == "TetLinearLgn")        mesh_basis_type_h = "Core/Basis/TetLinearLgn.h";
  if (type == "TetQuadraticLgn")     mesh_basis_type_h = "Core/Basis/TetQuadraticLgn.h";
  if (type == "TetCubicHmt")         mesh_basis_type_h = "Core/Basis/TetCubicHmt.h";
  if (type == "TriLinearLgn")        mesh_basis_type_h = "Core/Basis/TriLinearLgn.h";
  if (type == "TriQuadraticLgn")     mesh_basis_type_h = "Core/Basis/TriQuadraticLgn.h";
  if (type == "TriCubicHmt")         mesh_basis_type_h = "Core/Basis/TriCubicHmt.h";
  if (type == "PrismLinearLgn")      mesh_basis_type_h = "Core/Basis/PrismLinearLgn.h";
  if (type == "PrismQuadraticLgn")   mesh_basis_type_h = "Core/Basis/PrismQuadraticLgn.h";
  if (type == "PrismCubicHmt")       mesh_basis_type_h = "Core/Basis/PrismCubicHmt.h";
  if (type == "QuadBilinearLgn")     mesh_basis_type_h = "Core/Basis/QuadBilinearLgn.h";
  if (type == "QuadBiquadraticLgn")  mesh_basis_type_h = "Core/Basis/QuadbiquadraticLgn.h";
  if (type == "QuadBicubicHmt")      mesh_basis_type_h = "Core/Basis/QuadbicubicHmt.h";
}

std::string
FieldInformation::get_point_type()
{
  return(point_type);
}

void
FieldInformation::set_point_type(std::string type)
{
  point_type = type;
  point_type_h = "";
}


std::string
FieldInformation::get_basis_type()
{
  return(basis_type);
}

void
FieldInformation::set_basis_type(std::string type)
{
  if (type == "NoData" || type == "nodata" || type == "No" || type == "no") 
    { make_nodata(); return; }
  if (type == "Constant" || type == "constant"|| type == "ConstantData" || type == "constantdata") 
    { make_constantdata(); return; }
  if (type == "Linear" || type == "linear" || type == "LinearData" || type == "lineardata") 
    { make_lineardata(); return; }
  if (type == "Quadratic" || type == "quadratic" || type == "QuadraticData" || type == "quadraticdata") 
    { make_quadraticdata(); return; }
  if (type == "Cubic" || type == "cubic" || type == "CubicData" || type == "cubicdata")
    { make_cubicdata(); return; } 

  basis_type = type;
  basis_type_h = "";
  if (type == "NoDataBasis")      basis_type_h = "Core/Basis/NoData.h";
  if (type == "ConstantBasis")    basis_type_h = "Core/Basis/Constant.h";
  if (type == "CrvLinearLgn")     basis_type_h = "Core/Basis/CrvLinearLgn.h";
  if (type == "CrvQuadraticLgn")  basis_type_h = "Core/Basis/CrvQuadraticLgn.h";
  if (type == "CrvCubicHmt")      basis_type_h = "Core/Basis/CrvCubicHmt.h";
  if (type == "HexTrilinearLgn")     basis_type_h = "Core/Basis/HexTrilinearLgn.h";
  if (type == "HexTriquadraticLgn")  basis_type_h = "Core/Basis/HexTriquadraticLgn.h";
  if (type == "HexTricubicHmt")      basis_type_h = "Core/Basis/HexTricubicHmt.h";
  if (type == "TetLinearLgn")     basis_type_h = "Core/Basis/TetLinearLgn.h";
  if (type == "TetQuadraticLgn")  basis_type_h = "Core/Basis/TetQuadraticLgn.h";
  if (type == "TetCubicHmt")      basis_type_h = "Core/Basis/TetCubicHmt.h";
  if (type == "TriLinearLgn")     basis_type_h = "Core/Basis/TriLinearLgn.h";
  if (type == "TriQuadraticLgn")  basis_type_h = "Core/Basis/TriQuadraticLgn.h";
  if (type == "TriCubicHmt")      basis_type_h = "Core/Basis/TriCubicHmt.h";
  if (type == "PrismLinearLgn")     basis_type_h = "Core/Basis/PrismLinearLgn.h";
  if (type == "PrismQuadraticLgn")  basis_type_h = "Core/Basis/PrismQuadraticLgn.h";
  if (type == "PrismCubicHmt")      basis_type_h = "Core/Basis/PrismCubicHmt.h";
  if (type == "QuadBilinearLgn")     basis_type_h = "Core/Basis/QuadBilinearLgn.h";
  if (type == "QuadBiquadraticLgn")  basis_type_h = "Core/Basis/QuadbiquadraticLgn.h";
  if (type == "QuadBicubicHmt")      basis_type_h = "Core/Basis/QuadbicubicHmt.h";
}


std::string
FieldInformation::get_data_type()
{
  return(data_type);
}

void
FieldInformation::set_data_type(std::string type)
{
  if (type == "nodata") type = "double";
  data_type = type;
  data_type_h = "";
  if (type == "Vector") data_type_h = "Core/Geometry/Vector.h";
  if (type == "Tensor") data_type_h = "Core/Geometry/Tensor.h";
}

std::string
FieldInformation::get_container_type()
{
  return(container_type);
}

void
FieldInformation::set_container_type(std::string type)
{
  container_type = type;
  container_type_h = "";
  if (type == "vector") container_type_h = "vector";
  else container_type_h = "Core/Containers/FData.h";
}


std::string
FieldInformation::get_field_name()
{
  // Deal with some SCIRun design flaw
  std::string meshptr = "";
  if ((container_type.find("2d") != std::string::npos)||(container_type.find("3d") != std::string::npos)) 
    meshptr = "," + mesh_type + "<" + mesh_basis_type + "<" + point_type + "> " + "> ";
    
  std::string field_template = field_type + "<" + mesh_type + "<" + 
    mesh_basis_type + "<" + point_type + "> " + "> " + "," +
    basis_type + "<" + data_type + "> " + "," + container_type + "<" +
    data_type + meshptr + "> " + "> ";
    
  return(field_template);
}

std::string
FieldInformation::get_field_type_id()
{
  // Deal with some SCIRun design flaw
  std::string meshptr = "";
  if ((container_type.find("2d") != std::string::npos)||(container_type.find("3d") != std::string::npos)) 
    meshptr = "," + mesh_type + "<" + mesh_basis_type + "<" + point_type + ">" + ">";
    
  std::string field_template = field_type + "<" + mesh_type + "<" + 
    mesh_basis_type + "<" + point_type + ">" + ">" + "," +
    basis_type + "<" + data_type + ">" + "," + container_type + "<" +
    data_type + meshptr + ">" + ">";
  
  for (std::string::size_type r=0; r< field_template.size(); r++) if (field_template[r] == ' ') field_template[r] = '_';  
        
  return(field_template);
}


std::string
FieldInformation::get_mesh_type_id()
{
  std::string mesh_template =  mesh_type + "<" + mesh_basis_type + "<" + point_type + ">" + ">";
  
  for (std::string::size_type r=0; r< mesh_template.size(); r++) if (mesh_template[r] == ' ') mesh_template[r] = '_';  
        
  return(mesh_template);
}


std::string
FieldInformation::get_field_filename()
{
  return(DynamicAlgoBase::to_filename(get_field_name()));
}

void
FieldInformation::fill_compile_info(CompileInfoHandle &ci)
{
  if (field_type_h != "") ci->add_field_include(field_type_h);
  if (mesh_type_h != "") ci->add_mesh_include(mesh_type_h);
  if (mesh_basis_type_h != "") ci->add_basis_include(mesh_basis_type_h);
  if (point_type_h != "") ci->add_data_include(point_type_h);
  if (basis_type_h != "") ci->add_basis_include(basis_type_h);
  if (data_type_h != "") ci->add_data_include(data_type_h);
  if (container_type_h != "") ci->add_container_include(container_type_h);
}


bool
FieldTypeInformation::is_isomorphic()
{
  return((mesh_basis_type == basis_type));
}

bool
FieldTypeInformation::is_nonlinear()
{
  return((is_nonlineardata())||(is_nonlinearmesh()));
}

bool
FieldTypeInformation::is_linear()
{
  return((is_lineardata())&&(is_linearmesh()));
}

bool
FieldTypeInformation::is_nodata()
{
  return((basis_type == "NoDataBasis"));
}

bool
FieldTypeInformation::is_constantdata()
{
  return((basis_type == "ConstantBasis"));
}

bool
FieldTypeInformation::is_lineardata()
{
  return((basis_type.find("inear") != std::string::npos));
}

bool
FieldTypeInformation::is_nonlineardata()
{
  return( (basis_type.find("uadratic") != std::string::npos)||
          (basis_type.find("ubicHmt") != std::string::npos));
}

bool
FieldTypeInformation::is_quadraticdata()
{
  return ((basis_type.find("uadratic") != std::string::npos));
}

bool
FieldTypeInformation::is_cubicdata()
{
  return ((basis_type.find("ubicHmt") != std::string::npos));
}

int
FieldTypeInformation::field_basis_order()
{
  if (is_nodata()) return (-1);
  if (is_constantdata()) return (0);
  if (is_lineardata()) return (1);
  if (is_quadraticdata()) return (2);
  if (is_cubicdata()) return (3);
  return (-1);
}


bool
FieldTypeInformation::is_constantmesh()
{
  return((mesh_basis_type == "ConstantBasis"));
}

bool
FieldTypeInformation::is_linearmesh()
{
  return((mesh_basis_type.find("inear") != std::string::npos));
}

bool
FieldTypeInformation::is_nonlinearmesh()
{
  return( (mesh_basis_type.find("uadratic") != std::string::npos)||
          (mesh_basis_type.find("ubicHmt") != std::string::npos));
}

bool
FieldTypeInformation::is_quadraticmesh()
{
  return ((mesh_basis_type.find("uadratic") != std::string::npos));
}

bool
FieldTypeInformation::is_cubicmesh()
{
  return ((mesh_basis_type.find("ubicHmt") != std::string::npos));
}

int
FieldTypeInformation::mesh_basis_order()
{
  if (is_constantmesh()) return (0);
  if (is_linearmesh()) return (1);
  if (is_quadraticmesh()) return (2);
  if (is_cubicmesh()) return (3);
  return (-1);
}

bool
FieldTypeInformation::is_tensor()
{
  return ((data_type == "Tensor"));
}

bool
FieldTypeInformation::is_vector()
{
  return ((data_type == "Vector"));
}

bool
FieldTypeInformation::is_scalar()
{
  return((!is_tensor())&&(!is_vector())&&(data_type!=""));
}

bool
FieldTypeInformation::is_integer()
{
  return(is_char()||is_short()||is_int()||is_long()||is_longlong()
      ||is_unsigned_char()||is_unsigned_short()
      ||is_unsigned_int()||is_unsigned_long()
      ||is_unsigned_longlong());
}

bool
FieldTypeInformation::is_double()
{
  return((data_type == "double"));
}

bool
FieldTypeInformation::is_float()
{
  return((data_type == "float"));
}

bool
FieldTypeInformation::is_longlong()
{
  return((data_type == "long_long")||(data_type == "signed_long_long"));
}

bool
FieldTypeInformation::is_long()
{
  return((data_type == "long")||(data_type == "signed_long"));
}

bool
FieldTypeInformation::is_int()
{
  return((data_type == "int")||(data_type == "signed_int"));
}

bool
FieldTypeInformation::is_short()
{
  return((data_type == "short")||(data_type == "signed_short"));
}

bool
FieldTypeInformation::is_char()
{
  return((data_type == "char")||(data_type == "signed_char"));
}

bool
FieldTypeInformation::is_unsigned_longlong()
{
  return(data_type == "unsigned_long_long");
}

bool
FieldTypeInformation::is_unsigned_long()
{
  return(data_type == "unsigned_long");
}

bool
FieldTypeInformation::is_unsigned_int()
{
  return(data_type == "unsigned_int");
}

bool
FieldTypeInformation::is_unsigned_short()
{
  return(data_type == "unsigned_short");
}

bool
FieldTypeInformation::is_unsigned_char()
{
  return(data_type == "unsigned_char");
}

bool
FieldTypeInformation::is_dvt()
{
  return(is_double()||is_vector()||is_tensor());
}

bool
FieldTypeInformation::is_svt()
{
  return(is_scalar()||is_vector()||is_tensor());
}


bool
FieldTypeInformation::is_structuredmesh()
{
  return((mesh_type.find("Struct")!=std::string::npos) ||
	 (mesh_type=="ScanlineMesh")||
	 (mesh_type=="ImageMesh")||
	 (mesh_type=="LatVolMesh"));
}

bool
FieldTypeInformation::is_regularmesh()
{
  return((mesh_type=="ScanlineMesh")||
	 (mesh_type=="ImageMesh")||
	 (mesh_type=="LatVolMesh"));
}

bool
FieldTypeInformation::is_irregularmesh()
{
  return(!is_regularmesh());
}

bool
FieldTypeInformation::is_unstructuredmesh()
{
  return(!is_structuredmesh());
}

bool
FieldTypeInformation::is_pnt_element()
{
  return((mesh_type=="PointCloud"));
}

bool
FieldTypeInformation::is_crv_element()
{
  return((mesh_type=="CurveMesh")||
	 (mesh_type=="StructCurveMesh")||
	 (mesh_type=="ScanlineMesh"));
}

bool
FieldTypeInformation::is_tri_element()
{
  return((mesh_type=="TriSurfMesh"));
}

bool
FieldTypeInformation::is_quad_element()
{
  return((mesh_type=="QuadSurfMesh")||
	 (mesh_type=="ImageMesh")||
	 (mesh_type=="StructQuadSurfMesh"));
}

bool
FieldTypeInformation::is_tet_element()
{
  return((mesh_type=="TetVolMesh"));
}

bool
FieldTypeInformation::is_prism_element()
{
  return((mesh_type=="PrismVolMesh"));
}

bool
FieldTypeInformation::is_hex_element()
{
  return((mesh_type=="HexVolMesh")||
	 (mesh_type=="StructHexVolMesh")||
	 (mesh_type=="LatVolMesh")||
	 (mesh_type=="MaskedLatVolMesh"));
}

bool
FieldTypeInformation::is_pointcloud()
{
  if (mesh_type == "PointCloudMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_scanline()
{
  if (mesh_type == "ScanlineMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_image()
{
  if (mesh_type == "ImageMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_latvol()
{
  if (mesh_type == "LatVolMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_curve()
{
  if (mesh_type == "CurveMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_trisurf()
{
  if (mesh_type == "TriSurfMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_quadsurf()
{
  if (mesh_type == "QuadSurfMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_tetvol()
{
  if (mesh_type == "TetVolMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_prismvol()
{
  if (mesh_type == "PrismVolMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_hexvol()
{
  if (mesh_type == "HexVolMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_structcurve()
{
  if (mesh_type == "StructCurveMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_structquadsurf()
{
  if (mesh_type == "StructQuadSurfMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_structhexvol()
{
  if (mesh_type == "StructHexVolMesh") return (true);
  return false;
}


bool
FieldTypeInformation::is_point()
{
  if (mesh_type == "PointCloudMesh") return (true);
  return false;
}

bool
FieldTypeInformation::is_line()
{
  if ((mesh_type == "CurveMesh")||
      (mesh_type == "ScanlineMesh")||
      (mesh_type == "StructCurveMesh")) return (true);
  return false;
}

bool
FieldTypeInformation::is_surface()
{
  if ((mesh_type == "TriSurfMesh")||
      (mesh_type == "QuadSurfMesh")||
      (mesh_type == "ImageMesh")||
      (mesh_type == "StructQuadSurfMesh")) return (true);
  return false;
}

bool
FieldTypeInformation::is_volume()
{
  if ((mesh_type == "TetVolMesh")||(mesh_type == "PrismVolMesh")||
      (mesh_type == "HexVolMesh")||(mesh_type == "LatVolMesh")||
      (mesh_type == "StructHexVolMesh")||(mesh_type == "MaskedLatVolMesh")) return (true);
  return false;  
}


bool        
FieldInformation::operator==(const FieldInformation& fi) const
{
  if ( (field_type == fi.field_type) && (mesh_type == fi.mesh_type) && (mesh_basis_type == fi.mesh_basis_type) &&
       (point_type == fi.point_type) && (basis_type == fi.basis_type) && (data_type == fi.data_type) &&
       (container_type == fi.container_type) ) return (true);
  return (false);
}

bool
FieldInformation::make_nodata()
{
  set_basis_type("NoDataBasis");
  set_data_type("double");
  return (true);
}

bool
FieldInformation::make_constantdata()
{
  set_basis_type("ConstantBasis");
  return (true);
}

bool
FieldInformation::make_constantmesh()
{
  set_mesh_basis_type("ConstantBasis");
  return (true);
}

bool
FieldInformation::make_scanlinemesh()
{
  set_mesh_type("ScanlineMesh");
  return (true);
}

bool
FieldInformation::make_curvemesh()
{
  set_mesh_type("CurveMesh");
  return (true);
}

bool
FieldInformation::make_structcurvemesh()
{
  set_mesh_type("StructCurveMesh");
  return (true);
}

bool
FieldInformation::make_imagemesh()
{
  set_mesh_type("ImageMesh");
  return (true);
}

bool
FieldInformation::make_trisurfmesh()
{
  set_mesh_type("TriSurfMesh");
  return (true);
}

bool
FieldInformation::make_quadsurfmesh()
{
  set_mesh_type("QuadSurfMesh");
  return (true);
}

bool
FieldInformation::make_structquadsurfmesh()
{
  set_mesh_type("StructQuadSurfMesh");
  return (true);
}

bool
FieldInformation::make_latvolmesh()
{
  set_mesh_type("LatVolMesh");
  return (true);
}

bool
FieldInformation::make_tetvolmesh()
{
  set_mesh_type("TetVolMesh");
  return (true);
}

bool
FieldInformation::make_prismvolmesh()
{
  set_mesh_type("PrismVolMesh");
  return (true);
}

bool
FieldInformation::make_hexvolmesh()
{
  set_mesh_type("HexVolMesh");
  return (true);
}

bool
FieldInformation::make_structhexvolmesh()
{
  set_mesh_type("StructHexVolMesh");
  return (true);
}

bool
FieldInformation::make_pointcloudmesh()
{
  set_mesh_type("PointCloudMesh");
  return (true);
}


bool
FieldInformation::make_unstructuredmesh()
{
  if (is_pnt_element()) return(make_pointcloudmesh());
  if (is_crv_element()) return(make_curvemesh());
  if (is_tri_element()) return(make_trisurfmesh());
  if (is_quad_element()) return(make_quadsurfmesh());
  if (is_tet_element()) return(make_tetvolmesh());
  if (is_prism_element()) return(make_prismvolmesh());
  if (is_hex_element()) return(make_hexvolmesh());
  return (false);
}


bool
FieldInformation::make_linearmesh()
{
  if (mesh_type == "ScanlineMesh") set_mesh_basis_type("CrvLinearLgn");
  if (mesh_type == "ImageMesh")  set_mesh_basis_type("QuadBilinearLgn");
  if (mesh_type == "LatVolMesh")  set_mesh_basis_type("HexTrilinearLgn");
  if (mesh_type == "MaskedLatVolMesh")  set_mesh_basis_type("HexTrilinearLgn");
  if (mesh_type == "StructCurveMesh") set_mesh_basis_type("CrvLinearLgn");
  if (mesh_type == "StructQuadSurfMesh") set_mesh_basis_type("QuadBilinearLgn");
  if (mesh_type == "StructHexVolMesh") set_mesh_basis_type("HexTrilinearLgn");
  if (mesh_type == "CurveMesh") set_mesh_basis_type("CrvLinearLgn");
  if (mesh_type == "TriSurfMesh") set_mesh_basis_type("TriLinearLgn");
  if (mesh_type == "QuadSurfMesh") set_mesh_basis_type("QuadBilinearLgn");
  if (mesh_type == "TetVolMesh") set_mesh_basis_type("TetLinearLgn");
  if (mesh_type == "PrismVolMesh") set_mesh_basis_type("PrismLinearLgn");
  if (mesh_type == "HexVolMesh") set_mesh_basis_type("HexTrilinearLgn");
  if (mesh_type == "PointCloudMesh") set_mesh_basis_type("ConstantBasis");
  return (true);
}


bool
FieldInformation::make_lineardata()
{
  if (mesh_type == "ScanlineMesh") set_basis_type("CrvLinearLgn");
  if (mesh_type == "ImageMesh")  set_basis_type("QuadBilinearLgn");
  if (mesh_type == "LatVolMesh")  set_basis_type("HexTrilinearLgn");
  if (mesh_type == "MaskedLatVolMesh")  set_basis_type("HexTrilinearLgn");
  if (mesh_type == "StructCurveMesh") set_basis_type("CrvLinearLgn");
  if (mesh_type == "StructQuadSurfMesh") set_basis_type("QuadBilinearLgn");
  if (mesh_type == "StructHexVolMesh") set_basis_type("HexTrilinearLgn");
  if (mesh_type == "CurveMesh") set_basis_type("CrvLinearLgn");
  if (mesh_type == "TriSurfMesh") set_basis_type("TriLinearLgn");
  if (mesh_type == "QuadSurfMesh") set_basis_type("QuadBilinearLgn");
  if (mesh_type == "TetVolMesh") set_basis_type("TetLinearLgn");
  if (mesh_type == "PrismVolMesh") set_basis_type("PrismLinearLgn");
  if (mesh_type == "HexVolMesh") set_basis_type("HexTrilinearLgn");
  if (mesh_type == "PointCloudMesh") set_basis_type("ConstantBasis");
  return (true);
}

bool
FieldInformation::make_quadraticdata()
{
  if (mesh_type == "ScanlineMesh") set_basis_type("CrvQuadraticLgn");
  if (mesh_type == "ImageMesh")  set_basis_type("QuadBiquadraticLgn");
  if (mesh_type == "LatVolMesh")  set_basis_type("HexTriquadraticLgn");
  if (mesh_type == "MaskedLatVolMesh")  set_basis_type("HexTriquadraticLgn");
  if (mesh_type == "StructCurveMesh") set_basis_type("CrvQuadraticLgn");
  if (mesh_type == "StructQuadSurfMesh") set_basis_type("QuadBiquadraticLgn");
  if (mesh_type == "StructHexVolMesh") set_basis_type("HexTriquadraticLgn");
  if (mesh_type == "CurveMesh") set_basis_type("CrvQuadraticLgn");
  if (mesh_type == "TriSurfMesh") set_basis_type("TriQuadraticLgn");
  if (mesh_type == "QuadSurfMesh") set_basis_type("QuadBilinearLgn");
  if (mesh_type == "TetVolMesh") set_basis_type("TetQuadraticLgn");
  if (mesh_type == "PrismVolMesh") set_basis_type("PrismQuadraticLgn");
  if (mesh_type == "HexVolMesh") set_basis_type("HexTriquadraticLgn");
  if (mesh_type == "PointCloudMesh") set_basis_type("ConstantBasis");
  return (true);
}

bool
FieldInformation::make_quadraticmesh()
{
  if (mesh_type == "ScanlineMesh") set_mesh_basis_type("CrvQuadraticLgn");
  if (mesh_type == "ImageMesh")  set_mesh_basis_type("QuadBiquadraticLgn");
  if (mesh_type == "LatVolMesh")  set_mesh_basis_type("HexTriquadraticLgn");
  if (mesh_type == "MaskedLatVolMesh")  set_mesh_basis_type("HexTriquadraticLgn");
  if (mesh_type == "StructCurveMesh") set_mesh_basis_type("CrvQuadraticLgn");
  if (mesh_type == "StructQuadSurfMesh") set_mesh_basis_type("QuadBiquadraticLgn");
  if (mesh_type == "StructHexVolMesh") set_mesh_basis_type("HexTriquadraticLgn");
  if (mesh_type == "CurveMesh") set_mesh_basis_type("CrvQuadraticLgn");
  if (mesh_type == "TriSurfMesh") set_mesh_basis_type("TriQuadraticLgn");
  if (mesh_type == "QuadSurfMesh") set_mesh_basis_type("QuadBilinearLgn");
  if (mesh_type == "TetVolMesh") set_mesh_basis_type("TetQuadraticLgn");
  if (mesh_type == "PrismVolMesh") set_mesh_basis_type("PrismQuadraticLgn");
  if (mesh_type == "HexVolMesh") set_mesh_basis_type("HexTriquadraticLgn");
  if (mesh_type == "PointCloudMesh") set_mesh_basis_type("ConstantBasis");
  return (true);
}

bool
FieldInformation::make_cubicdata()
{    
  if (mesh_type == "ScanlineMesh") set_basis_type("CrvCubicHmt");
  if (mesh_type == "ImageMesh")  set_basis_type("QuadBicubicHmt");
  if (mesh_type == "LatVolMesh")  set_basis_type("HexTricubicHmt");
  if (mesh_type == "MaskedLatVolMesh")  set_basis_type("HexTriquadraticLgn");
  if (mesh_type == "StructCurveMesh") set_basis_type("CrvCubicHmt");
  if (mesh_type == "StructQuadSurfMesh") set_basis_type("QuadBicubicHmt");
  if (mesh_type == "StructHexVolMesh") set_basis_type("HexTricubicHmt");
  if (mesh_type == "CurveMesh") set_basis_type("CrvCubicHmt");
  if (mesh_type == "TriSurfMesh") set_basis_type("TriCubicHmt");
  if (mesh_type == "QuadSurfMesh") set_basis_type("QuadBicubicHmt");
  if (mesh_type == "TetVolMesh") set_basis_type("TetCubicHmt");
  if (mesh_type == "PrismVolMesh") set_basis_type("PrismCubicHmt");
  if (mesh_type == "HexVolMesh") set_basis_type("HexTricubicHmt");
  if (mesh_type == "PointCloudMesh") set_basis_type("ConstantBasis");
  return (true);
}


bool
FieldInformation::make_cubicmesh()
{    
  if (mesh_type == "ScanlineMesh") set_mesh_basis_type("CrvCubicHmt");
  if (mesh_type == "ImageMesh")  set_mesh_basis_type("QuadBicubicHmt");
  if (mesh_type == "LatVolMesh")  set_mesh_basis_type("HexTricubicHmt");
  if (mesh_type == "MaskedLatVolMesh")  set_mesh_basis_type("HexTriquadraticLgn");
  if (mesh_type == "StructCurveMesh") set_mesh_basis_type("CrvCubicHmt");
  if (mesh_type == "StructQuadSurfMesh") set_mesh_basis_type("QuadBicubicHmt");
  if (mesh_type == "StructHexVolMesh") set_mesh_basis_type("HexTricubicHmt");
  if (mesh_type == "CurveMesh") set_mesh_basis_type("CrvCubicHmt");
  if (mesh_type == "TriSurfMesh") set_mesh_basis_type("TriCubicHmt");
  if (mesh_type == "QuadSurfMesh") set_mesh_basis_type("QuadBicubicHmt");
  if (mesh_type == "TetVolMesh") set_mesh_basis_type("TetCubicHmt");
  if (mesh_type == "PrismVolMesh") set_mesh_basis_type("PrismCubicHmt");
  if (mesh_type == "HexVolMesh") set_mesh_basis_type("HexTricubicHmt");
  if (mesh_type == "PointCloudMesh") set_mesh_basis_type("ConstantBasis");
  return (true);
}

bool
FieldInformation::make_scalar()
{
  set_data_type("double");
  return (true);
}

bool
FieldInformation::make_char()
{
  set_data_type("char");
  return (true);
}

bool
FieldInformation::make_unsigned_char()
{
  set_data_type("unsigned_char");
  return (true);
}

bool
FieldInformation::make_short()
{
  set_data_type("short");
  return (true);
}

bool
FieldInformation::make_unsigned_short()
{
  set_data_type("unsigned_short");
  return (true);
}

bool
FieldInformation::make_int()
{
  set_data_type("int");
  return (true);
}

bool
FieldInformation::make_unsigned_int()
{
  set_data_type("unsigned_int");
  return (true);
}

bool
FieldInformation::make_long()
{
  set_data_type("long");
  return (true);
}

bool
FieldInformation::make_unsigned_long()
{
  set_data_type("unsigned_long");
  return (true);
}

bool
FieldInformation::make_long_long()
{
  set_data_type("long_long");
  return (true);
}

bool
FieldInformation::make_unsigned_long_long()
{
  set_data_type("unsigned_long_long");
  return (true);
}

bool
FieldInformation::make_float()
{
  set_data_type("float");
  return (true);
}

bool
FieldInformation::make_double()
{
  set_data_type("double");
  return (true);
}

bool
FieldInformation::make_vector()
{
  set_data_type("Vector");
  return (true);
}

bool
FieldInformation::make_tensor()
{
  set_data_type("Tensor");
  return (true);
}

bool
FieldInformation::has_virtual_interface()
{
  std::string type = get_field_type_id();
  std::string meshtype = get_mesh_type_id();
  
  MeshHandle meshtesthandle = CreateMesh(meshtype);
  if (meshtesthandle.get_rep() == 0) 
  {
    std::cout << "Could not find "<<meshtype<<" in mesh database\n";
    return (false);
  }

  FieldHandle testhandle = CreateField(type,meshtesthandle);
  if (testhandle.get_rep() == 0) 
  {
    std::cout << "Could not find "<<type<<" in field database\n";   
    return (false); 
  }
  if (testhandle->has_virtual_interface() == false) 
  {
    std::cout << "Field ("<<type<<") does not have virtual interface\n";
    return (false); 
  }
  return (true);
}

FieldHandle CreateField(std::string meshtype, 
                         std::string basistype, std::string datatype)
{
  FieldInformation fi(meshtype,basistype,datatype);
  return(CreateField(fi));
}

FieldHandle CreateField(std::string meshtype, std::string meshbasistype, 
                         std::string databasistype, std::string datatype)
{
  FieldInformation fi(meshtype,meshbasistype,databasistype,datatype);
  return(CreateField(fi));
}

FieldHandle
CreateField(FieldInformation &info)
{
  std::string type = info.get_field_type_id();
  std::string meshtype = info.get_mesh_type_id();
  MeshHandle meshhandle = CreateMesh(meshtype);
  
  if (meshhandle.get_rep() == 0) return (0);

  return (CreateField(type,meshhandle));              
}

FieldHandle
CreateField(FieldInformation &info, MeshHandle mesh)
{
  std::string type = info.get_field_type_id();
  return (CreateField(type,mesh));              
}

MeshHandle 
CreateMesh(FieldInformation &info)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type));
}

MeshHandle 
CreateMesh(FieldInformation &info,Mesh::size_type x)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x));
}

MeshHandle 
CreateMesh(FieldInformation &info,Mesh::size_type x,const Point& min,const Point& max)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,min,max));
}

MeshHandle 
CreateMesh(FieldInformation &info,Mesh::size_type x,Mesh::size_type y)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y));
}

MeshHandle 
CreateMesh(FieldInformation &info,Mesh::size_type x,Mesh::size_type y,const Point& min,const Point& max)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y,min,max));
}

MeshHandle 
CreateMesh(FieldInformation &info,Mesh::size_type x,Mesh::size_type y,Mesh::size_type z)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y,z));
}

MeshHandle 
CreateMesh(FieldInformation &info,Mesh::size_type x,Mesh::size_type y,Mesh::size_type z,const Point& min,const Point& max)
{
  std::string type = info.get_mesh_type_id();
  return (CreateMesh(type,x,y,z,min,max));
}

MeshHandle 
CreateMesh(FieldInformation &info,vector<Mesh::size_type> dim)
{
  std::string type = info.get_mesh_type_id();
  if (dim.size() == 1) return (CreateMesh(type,dim[0]));
  if (dim.size() == 2) return (CreateMesh(type,dim[0],dim[1]));
  if (dim.size() == 3) return (CreateMesh(type,dim[0],dim[1],dim[2]));
  else return (MeshHandle(0));
}

MeshHandle 
CreateMesh(FieldInformation &info,vector<Mesh::size_type> dim,const Point& min,const Point& max)
{
  std::string type = info.get_mesh_type_id();
  if (dim.size() == 1) return (CreateMesh(type,dim[0],min,max));
  if (dim.size() == 2) return (CreateMesh(type,dim[0],dim[1],min,max));
  if (dim.size() == 3) return (CreateMesh(type,dim[0],dim[1],dim[2],min,max));
  else return (MeshHandle(0));
}





} // end namespace

