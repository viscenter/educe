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


//    File   : ReportMeshQualityMeasures.h
//    Author : Jason Shepherd
//    Date   : January 2006

#if !defined(ReportMeshQualityMeasures_h)
#define ReportMeshQualityMeasures_h

#include <vtkMeshQuality.h>
#include <vtkPoints.h>
#include <vtkTetra.h>
#include <vtkQuad.h>
#include <vtkHexahedron.h>
#include <vtkTriangle.h>
#include <Core/Datatypes/NrrdData.h>
#include <Core/Datatypes/GenericField.h>
#include <Core/Basis/Constant.h>
#include <Dataflow/Network/Module.h>
#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <Core/Util/ProgressReporter.h>
#include <Core/Datatypes/QuadSurfMesh.h>
#include <Core/Datatypes/HexVolMesh.h>
#include <Core/Basis/QuadBilinearLgn.h>
#include <Core/Basis/HexTrilinearLgn.h>
#include <algorithm>
#include <set>

namespace SCIRun {

using std::copy;

class GuiInterface;

class ReportMeshQualityMeasuresAlgo : public DynamicAlgoBase
{
public:

  virtual NrrdDataHandle execute(ProgressReporter *reporter, 
				 FieldHandle fieldh) = 0;

  //! support the dynamically compiled algorithm concept
  static CompileInfoHandle get_compile_info(const TypeDescription *fsrc,
					    string ext);
};


//For Tets...
template <class FIELD>
class ReportMeshQualityMeasuresAlgoTet : public ReportMeshQualityMeasuresAlgo
{
public:
  //! virtual interface. 
  virtual NrrdDataHandle execute(ProgressReporter *reporter, 
				 FieldHandle fieldh);
};

template <class FIELD>
NrrdDataHandle 
ReportMeshQualityMeasuresAlgoTet<FIELD>::execute(ProgressReporter *mod, 
						 FieldHandle fieldh)
{
  const int nmetrics = 10;
  FIELD *field = dynamic_cast<FIELD*>(fieldh.get_rep());
  typename FIELD::mesh_type *mesh = 
    dynamic_cast<typename FIELD::mesh_type *>(fieldh->mesh().get_rep());

  double node_pos[4][3];
  vector<typename FIELD::mesh_type::Elem::index_type> elemmap;
  typename FIELD::mesh_type::Elem::iterator bi, ei;
  mesh->begin(bi); mesh->end(ei);

  //Output a nrrd with all the metrics per element.
  typename FIELD::mesh_type::Elem::size_type sz;
  mesh->size(sz);
  const unsigned nelems = sz;
  Nrrd* onnrd = nrrdNew();
  nrrdAlloc_va(onnrd, nrrdTypeDouble, 2, nmetrics, nelems); 
  nrrdAxisInfoSet_va(onnrd, nrrdAxisInfoLabel, "element", "metrics");
  NrrdDataHandle output = new NrrdData(onnrd);
  double *data = (double*)onnrd->data;

  // record the indices for the metrics.
  nrrdKeyValueAdd(onnrd, "aspect", "0");
  nrrdKeyValueAdd(onnrd, "aspect_gamma", "1");
  nrrdKeyValueAdd(onnrd, "volume", "2");
  nrrdKeyValueAdd(onnrd, "condition", "3");
  nrrdKeyValueAdd(onnrd, "jacobian", "4");
  nrrdKeyValueAdd(onnrd, "scaled_jacobian", "5");
  nrrdKeyValueAdd(onnrd, "shape", "6");
  nrrdKeyValueAdd(onnrd, "shape_size", "7");
  nrrdKeyValueAdd(onnrd, "distortion", "8");
  nrrdKeyValueAdd(onnrd, "radius_ratio", "9");

  mesh->synchronize( Mesh::ALL_ELEMENTS_E );
  int total_elements = 0;
  // each metric will have per elem value, low and high and ave values...
  double metrics[nmetrics][4];
  int inversions = 0;
  bool first_time_thru = true;

  while (bi != ei)
  {
    typename FIELD::mesh_type::Node::array_type onodes;
    mesh->get_nodes(onodes, *bi);

    vtkTetra *tetra = vtkTetra::New(); 

    int i;
    for( i = 0; i < 4; i++ )
    {
      Point p;
      mesh->get_center( p, onodes[i] );
      tetra->GetPoints()->SetPoint(i, p.x(), p.y(), p.z());
    }

    metrics[0][0] = vtkMeshQuality::TetAspectBeta(tetra);     
    metrics[1][0] = vtkMeshQuality::TetAspectGamma(tetra);    
    metrics[2][0] = vtkMeshQuality::TetVolume(tetra);         
    metrics[3][0] = vtkMeshQuality::TetCondition(tetra);      
    metrics[4][0] = vtkMeshQuality::TetJacobian(tetra);       
    metrics[5][0] = vtkMeshQuality::TetScaledJacobian(tetra); 
    metrics[6][0] = vtkMeshQuality::TetShape(tetra);          
    metrics[7][0] = vtkMeshQuality::TetShapeandSize(tetra);   
    metrics[8][0] = vtkMeshQuality::TetDistortion(tetra);     
    metrics[9][0] = 1.0L / metrics[0][0]; // inv radius ratio...
    
    tetra->Delete();
    for(int i = 0; i < nmetrics; ++i)
    {    
      data[i] = metrics[i][0];
    }
    data += nmetrics;

    if( first_time_thru )
    {
      for (int j = 0; j < nmetrics; ++j) {
        metrics[j][1] = metrics[j][0];
        metrics[j][2] = metrics[j][0];
        metrics[j][3] = 0.0L;
      }
      first_time_thru = false;
    }

    // check high, low, and total for the average;
    for (int j = 0; j < nmetrics; ++j)
    {
      if( metrics[j][0] > metrics[j][2] )
        metrics[j][2] = metrics[j][0];
      else if(metrics[j][0] < metrics[j][1])
        metrics[j][1] = metrics[j][0];
      metrics[j][3] += metrics[j][0];
    }

    typename FIELD::mesh_type::Elem::index_type elem_id = *bi;
    // 5 is the scaled jacobian...
    if (metrics[5][0] <= 0.0)
    {
      inversions++;
      cout << "WARNING: Tet " << elem_id << " has negative volume!" << endl;
    }
    total_elements++;
    ++bi;
  }
  

  for (int j = 0; j < nmetrics; ++j) {
    metrics[j][3] /= (double)total_elements;
  }
  

  typename FIELD::mesh_type::Node::size_type nodes;
  typename FIELD::mesh_type::Edge::size_type edges;
  typename FIELD::mesh_type::Face::size_type faces;
  typename FIELD::mesh_type::Cell::size_type tets;
  mesh->size(nodes);
  mesh->size(edges);
  mesh->size(faces);
  mesh->size(tets);
  int holes = (tets - faces + edges - nodes + 2) / 2;

  cout << endl << "Number of Tet elements checked = " << total_elements;
  if( inversions != 0 )
      cout << " (" << inversions << " Tets have negative jacobians!)";
  cout << endl << "Euler characteristics for this mesh indicate " << holes 
       << " holes in this block of elements." << endl 
       << "    (Assumes a single contiguous block of elements.)" << endl;
  cout << "Element counts: Tets: " << tets << " Faces: " << faces 
       << " Edges: " << edges << " Nodes: " << nodes << endl;
  cout << "Aspect Ratio: Low = " << metrics[0][1] << ", Average = " 
       << metrics[0][3] << ", High = " << metrics[0][2] << endl;
  cout << "Aspect Ratio (gamma): Low = " << metrics[1][1] << ", Average = " 
       << metrics[1][3] << ", High = " << metrics[1][2] << endl;
  cout << "Volume: Low = " << metrics[2][1] << ", Average = " << metrics[2][3] 
       << ", High = " << metrics[2][2] << endl;
  cout << "Condition: Low = " << metrics[3][1] << ", Average = " 
       << metrics[3][3] << ", High = " << metrics[3][2] << endl;
  cout << "Jacobian: Low = " << metrics[4][1] << ", Average = " 
       << metrics[4][3] << ", High = " << metrics[4][2] << endl;
  cout << "Scaled_Jacobian: Low = " << metrics[5][1] << ", Average = " 
       << metrics[5][3] << ", High = " << metrics[5][2] << endl;
  cout << "Shape: Low = " << metrics[6][1] << ", Average = " << metrics[6][3] 
       << ", High = " << metrics[6][2] << endl;
  cout << "Shape_Size: Low = " << metrics[7][1] << ", Average = " 
       << metrics[7][3] << ", High = " << metrics[7][2] << endl;
  cout << "Distortion: Low = " << metrics[8][1] << ", Average = " 
       << metrics[8][3] << ", High = " << metrics[8][2] << endl;
  cout << "Radius Ratio: Low = " << metrics[9][1] << ", Average = " 
       << metrics[9][3] << ", High = " << metrics[9][2] << endl;

  return output;
}


//For Hexes...
template <class FIELD>
class ReportMeshQualityMeasuresAlgoHex : public ReportMeshQualityMeasuresAlgo
{
public:
  //! virtual interface. 
  virtual NrrdDataHandle execute(ProgressReporter *reporter, 
				 FieldHandle fieldh);
};

template <class FIELD>
NrrdDataHandle 
ReportMeshQualityMeasuresAlgoHex<FIELD>::execute(ProgressReporter *mod, 
						 FieldHandle fieldh)
{
  const int nmetrics = 15;
  
  FIELD *field = dynamic_cast<FIELD*>(fieldh.get_rep());
  typename FIELD::mesh_type *mesh = 
    dynamic_cast<typename FIELD::mesh_type *>(fieldh->mesh().get_rep());

  double node_pos[8][3];
  vector<typename FIELD::mesh_type::Elem::index_type> elemmap;
  typename FIELD::mesh_type::Elem::iterator bi, ei;
  mesh->begin(bi); mesh->end(ei);

  //Output a nrrd with all the metrics per element.
  typename FIELD::mesh_type::Elem::size_type sz;
  mesh->size(sz);
  const unsigned nelems = sz;
  Nrrd* onnrd = nrrdNew();
  nrrdAlloc_va(onnrd, nrrdTypeDouble, 2, nmetrics, nelems); 
  nrrdAxisInfoSet_va(onnrd, nrrdAxisInfoLabel, "element", "metrics");
  NrrdDataHandle output = new NrrdData(onnrd);
  double *data = (double*)onnrd->data;
  
  nrrdKeyValueAdd(onnrd, "aspect", "0");
  nrrdKeyValueAdd(onnrd, "skew", "1");
  nrrdKeyValueAdd(onnrd, "taper", "2");
  nrrdKeyValueAdd(onnrd, "volume", "3");
  nrrdKeyValueAdd(onnrd, "stretch", "4");
  nrrdKeyValueAdd(onnrd, "diagonal", "5");
  nrrdKeyValueAdd(onnrd, "dimension", "6");
  nrrdKeyValueAdd(onnrd, "condition", "7");
  nrrdKeyValueAdd(onnrd, "jacobian", "8");
  nrrdKeyValueAdd(onnrd, "scaled_jacobian", "9");
  nrrdKeyValueAdd(onnrd, "shear", "10");
  nrrdKeyValueAdd(onnrd, "shape", "11");
  nrrdKeyValueAdd(onnrd, "shear_size", "12");
  nrrdKeyValueAdd(onnrd, "shape_size", "13");
  nrrdKeyValueAdd(onnrd, "distortion", "14");

  //perform Euler checks for topology errors...
  // 2-2g = -#hexes+#faces-#edges+#nodes
  mesh->synchronize( Mesh::ALL_ELEMENTS_E );

  int total_elements = 0;
  // each metric will have per elem value, low and high and ave values...
  double metrics[nmetrics][4];
  
  int inversions = 0;
  bool first_time_thru = true;
  bool inverted_element_detected = false;
  
  while (bi != ei)
  {
    typename FIELD::mesh_type::Node::array_type onodes;
    mesh->get_nodes(onodes, *bi);

    vtkHexahedron *hex = vtkHexahedron::New(); 


    for(int i = 0; i < 8; ++i)
    {
      Point p;
      mesh->get_center( p, onodes[i] );
      hex->GetPoints()->SetPoint(i, p.x(), p.y(), p.z());
    }

    metrics[0][0] = vtkMeshQuality::HexMaxAspectFrobenius(hex);
    metrics[1][0] = vtkMeshQuality::HexSkew(hex);
    metrics[2][0] = vtkMeshQuality::HexTaper(hex);
    metrics[3][0] = vtkMeshQuality::HexVolume(hex);
    metrics[4][0] = vtkMeshQuality::HexStretch(hex);
    metrics[5][0] = vtkMeshQuality::HexDiagonal(hex);
    metrics[6][0] = vtkMeshQuality::HexDimension(hex);
    metrics[7][0] = vtkMeshQuality::HexCondition(hex);
    metrics[8][0] = vtkMeshQuality::HexJacobian(hex);
    metrics[9][0] = vtkMeshQuality::HexScaledJacobian(hex);
    metrics[10][0] = vtkMeshQuality::HexShear(hex);
    metrics[11][0] = vtkMeshQuality::HexShape(hex);
    metrics[12][0] = vtkMeshQuality::HexShearAndSize(hex);
    metrics[13][0] = vtkMeshQuality::HexShapeAndSize(hex);
    metrics[14][0] = vtkMeshQuality::HexDistortion(hex);

    hex->Delete();
    for(int i = 0; i < nmetrics; ++i)
    {    
      data[i] = metrics[i][0];
    }
    data += nmetrics;

    if(first_time_thru)
    {
      for (int j = 0; j < nmetrics; ++j) {
        metrics[j][1] = metrics[j][0];
        metrics[j][2] = metrics[j][0];
        metrics[j][3] = 0.0L;
      }
      first_time_thru = false;
    }

    // check high, low, and total for the average;
    for (int j = 0; j < nmetrics; ++j)
    {
      if( metrics[j][0] > metrics[j][2] )
        metrics[j][2] = metrics[j][0];
      else if(metrics[j][0] < metrics[j][1])
        metrics[j][1] = metrics[j][0];
      metrics[j][3] += metrics[j][0];
    }

    typename FIELD::mesh_type::Elem::index_type elem_id = *bi;
    // 9 is the scaled jacobian...
    if (metrics[9][0] <= 0.0)
    {
      if (inverted_element_detected == false)
      {
        cout << "WARNING: The following hexes have negative volume: ";
        inverted_element_detected = true;
      }
      inversions++;
      cout << elem_id << " ";
    }
    
    total_elements++;
    ++bi;
  }


  for (int j = 0; j < nmetrics; ++j) {
    metrics[j][3] /= (double)total_elements;
  }

  typename FIELD::mesh_type::Node::size_type nodes;
  typename FIELD::mesh_type::Edge::size_type edges;
  typename FIELD::mesh_type::Face::size_type faces;
  typename FIELD::mesh_type::Cell::size_type hexes;
  mesh->size( nodes );
  mesh->size( edges );
  mesh->size( faces );
  mesh->size( hexes );
  signed int holes = (hexes - faces + edges - nodes + 2) / 2;
  
  cout << endl << "Number of Hex elements checked = " << total_elements;
  if( inversions != 0 )
    cout << " (" << inversions << " Hexes have negative jacobians!)";
  cout << endl << "Euler characteristics for this mesh indicate " << holes 
       << " holes in this block of elements." << endl 
       << "    (Assumes a single contiguous block of elements.)" << endl;
  cout << "Number of Elements = Hexes: " << hexes << " Faces: " << faces 
       << " Edges: " << edges << " Nodes: " << nodes << endl;
  cout << "Aspect Ratio: Low = " << metrics[0][1] << ", Average = " 
       << metrics[0][3] << ", High = " << metrics[0][2] << endl;
  cout << "Skew: Low = " << metrics[1][1] << ", Average = " 
       << metrics[1][3] << ", High = " << metrics[1][2] << endl;
  cout << "Taper: Low = " << metrics[2][1] << ", Average = " 
       << metrics[2][3] << ", High = " << metrics[2][2] << endl;
  cout << "Volume: Low = " << metrics[3][1] << ", Average = " 
       << metrics[3][3] << ", High = " << metrics[3][2] << endl;
  cout << "Stretch: Low = " << metrics[4][1] << ", Average = " 
       << metrics[4][3] << ", High = " << metrics[4][2] << endl;
  cout << "Diagonal: Low = " << metrics[5][1] << ", Average = " 
       << metrics[5][3] << ", High = " << metrics[5][2] << endl;
  cout << "Dimension: Low = " << metrics[6][1] << ", Average = " 
       << metrics[6][3] << ", High = " << metrics[6][2] << endl;
  cout << "Condition: Low = " << metrics[7][1] << ", Average = " 
       << metrics[7][3] << ", High = " << metrics[7][2] << endl;
  cout << "Jacobian: Low = " << metrics[8][1] << ", Average = " 
       << metrics[8][3] << ", High = " << metrics[8][2] << endl;
  cout << "Scaled_Jacobian: Low = " << metrics[9][1] << ", Average = " 
       << metrics[9][3] << ", High = " << metrics[9][2] << endl;
  cout << "Shear: Low = " << metrics[10][1] << ", Average = " 
       << metrics[10][3] << ", High = " << metrics[10][2] << endl;
  cout << "Shape: Low = " << metrics[11][1] << ", Average = " 
       << metrics[11][3] << ", High = " << metrics[11][2] << endl;
  cout << "Shear_Size: Low = " << metrics[12][1] << ", Average = " 
       << metrics[12][3] << ", High = " << metrics[12][2] << endl;
  cout << "Shape_Size: Low = " << metrics[13][1] << ", Average = " 
       << metrics[13][3] << ", High = " << metrics[13][2] << endl;
  cout << "Distortion: Low = " << metrics[14][1] << ", Average = " 
       << metrics[14][3] << ", High = " << metrics[14][2] << endl;
 
  return output;
}

//For Tris...
template <class FIELD>
class ReportMeshQualityMeasuresAlgoTri : public ReportMeshQualityMeasuresAlgo
{
public:
  //! virtual interface. 
  virtual NrrdDataHandle execute(ProgressReporter *reporter, 
				 FieldHandle fieldh);
};

double
radius_ratio_tri(const Point &p1, const Point &p2, Point &p3) 
{
  // side lengths
  double a = (p1 - p2).length();
  double b = (p2 - p3).length();
  double c = (p3 - p1).length();

  double k = 0.5L * (a + b + c); 

  // Area squared
  double A2 = k * (k - a) * (k - b) * (k - c);

  return (8.0L * A2) / (k * a * b * c);
}



template <class FIELD>
NrrdDataHandle 
ReportMeshQualityMeasuresAlgoTri<FIELD>::execute(ProgressReporter *mod, 
						 FieldHandle fieldh)
{
  const int nmetrics = 9;
  FIELD *field = dynamic_cast<FIELD*>(fieldh.get_rep());
  typename FIELD::mesh_type *mesh = 
    dynamic_cast<typename FIELD::mesh_type *>(fieldh->mesh().get_rep());

  double node_pos[3][3];
  vector<typename FIELD::mesh_type::Elem::index_type> elemmap;
  typename FIELD::mesh_type::Elem::iterator bi, ei;
  mesh->begin(bi); mesh->end(ei);
  //Output a nrrd with all the metrics per element.
  typename FIELD::mesh_type::Elem::size_type sz;
  mesh->size(sz);
  const unsigned nelems = sz;
  Nrrd* onnrd = nrrdNew();
  nrrdAlloc_va(onnrd, nrrdTypeDouble, 2, nmetrics, nelems);
  nrrdAxisInfoSet_va(onnrd, nrrdAxisInfoLabel, "element", "metrics");
  NrrdDataHandle output = new NrrdData(onnrd);
  double *data = (double*)onnrd->data;
  
  // record the indices for the metrics.
  nrrdKeyValueAdd(onnrd, "area", "0");
  nrrdKeyValueAdd(onnrd, "minimum_angle", "1");
  nrrdKeyValueAdd(onnrd, "maximum_angle", "2");
  nrrdKeyValueAdd(onnrd, "condition", "3");
  nrrdKeyValueAdd(onnrd, "scaled_jacobian", "4");
  nrrdKeyValueAdd(onnrd, "shape", "5");
  nrrdKeyValueAdd(onnrd, "shape_size", "6");
  nrrdKeyValueAdd(onnrd, "distortion", "7");
  nrrdKeyValueAdd(onnrd, "radius_ratio", "8");

  mesh->synchronize( Mesh::EDGES_E );

  int total_elements = 0;
  // per elem value, low, high, ave...
  double metrics[nmetrics][4];
  
  int inversions = 0;
  int first_time_thru = 1;
  
  while (bi != ei)
  {
    typename FIELD::mesh_type::Node::array_type onodes;
    mesh->get_nodes(onodes, *bi);

    vtkTriangle *tri = vtkTriangle::New(); 


    Point p1;
    mesh->get_center(p1, onodes[0] );
    tri->GetPoints()->SetPoint(0, p1.x(), p1.y(), p1.z());
    
    Point p2;
    mesh->get_center(p2, onodes[1] );
    tri->GetPoints()->SetPoint(1, p2.x(), p2.y(), p2.z());
    
    Point p3;
    mesh->get_center(p3, onodes[2] );
    tri->GetPoints()->SetPoint(2, p3.x(), p3.y(), p3.z());
      

    data[0] = vtkMeshQuality::TriangleArea(tri);
    data[1] = vtkMeshQuality::TriangleMinAngle(tri);
    data[2] = vtkMeshQuality::TriangleMaxAngle(tri);
    data[3] = vtkMeshQuality::TriangleCondition(tri);
    data[4] = vtkMeshQuality::TriangleScaledJacobian(tri);
    data[5] = vtkMeshQuality::TriangleShape(tri);
    data[6] = vtkMeshQuality::TriangleShapeAndSize(tri);
    data[7] = vtkMeshQuality::TriangleDistortion(tri);
    data[8] = radius_ratio_tri(p1, p2, p3);

    tri->Delete();

    for (int j = 0; j < nmetrics; ++j) {
      metrics[j][0] = data[j];
    }

    data += nmetrics;

    if( first_time_thru )
    {
      for (int j = 0; j < nmetrics; ++j) {
        metrics[j][1] = metrics[j][0];
        metrics[j][2] = metrics[j][0];
        metrics[j][3] = 0.0L;
      }
      first_time_thru = false;
    }

    // check high, low, and total for the average;
    for (int j = 0; j < nmetrics; ++j)
    {
      if( metrics[j][0] > metrics[j][2] )
        metrics[j][2] = metrics[j][0];
      else if(metrics[j][0] < metrics[j][1])
        metrics[j][1] = metrics[j][0];
      metrics[j][3] += metrics[j][0];
    }

    typename FIELD::mesh_type::Elem::index_type elem_id = *bi;

    if(metrics[4][0] <= 0.0 )
    {
      inversions++;
      cout << "WARNING: Tri " << elem_id << " has negative area!" << endl;
    }

    total_elements++;
    ++bi;
  }

  for (int j = 0; j < nmetrics; ++j) {
      metrics[j][3] /= (double)total_elements;
  }

  typename FIELD::mesh_type::Node::size_type nodes;
  typename FIELD::mesh_type::Edge::size_type edges;
  typename FIELD::mesh_type::Face::size_type faces;
  mesh->size( nodes );
  mesh->size( edges );
  mesh->size( faces );
  int holes = (faces - edges + nodes - 2) / 2;
//  

  cout << endl << "Number of Tri elements checked = " << total_elements;
  if( inversions != 0 )
      cout << " (" << inversions << " Tris have negative jacobians!)";
  cout << endl << "Euler characteristics for this mesh indicate " << holes 
       << " holes in this block of elements." << endl 
       << "    (Assumes a single contiguous block of elements.)" << endl;
  cout << " Tris: " << faces << " Edges: " << edges << " Nodes: " 
       << nodes << endl;

  cout << "Area: Low = " << metrics[0][1] << ", Average = " << metrics[0][3] 
       << ", High = " << metrics[0][2] << endl;
  cout << "Minimum_Angle: Low = " << metrics[1][1] << ", Average = " 
       << metrics[1][3] << ", High = " << metrics[1][2] << endl;
  cout << "Maximum_Angle: Low = " << metrics[2][1] << ", Average = " 
       << metrics[2][3] << ", High = " << metrics[2][2] << endl;
  cout << "Condition: Low = " << metrics[3][1] << ", Average = " 
       << metrics[3][3] << ", High = " << metrics[3][2] << endl;
  cout << "Scaled_Jacobian: Low = " << metrics[4][1] << ", Average = " 
       << metrics[4][3] << ", High = " << metrics[4][2] << endl;
  cout << "Shape: Low = " << metrics[5][1] << ", Average = " << metrics[5][3] 
       << ", High = " << metrics[5][2] << endl;
  cout << "Shape_Size: Low = " << metrics[6][1] << ", Average = " 
       << metrics[6][3] << ", High = " << metrics[6][2] << endl;
  cout << "Distortion: Low = " << metrics[7][1] << ", Average = " 
       << metrics[7][3] << ", High = " << metrics[7][2] << endl;
  cout << "Inv Radius Ratio: Low = " << metrics[8][1] << ", Average = " 
       << metrics[8][3] << ", High = " << metrics[8][2] << endl;
   return output;
}

//For Quads...
template <class FIELD>
class ReportMeshQualityMeasuresAlgoQuad : public ReportMeshQualityMeasuresAlgo
{
public:
  //! virtual interface. 
  virtual NrrdDataHandle execute(ProgressReporter *reporter, 
				 FieldHandle fieldh);
};

template <class FIELD>
NrrdDataHandle 
ReportMeshQualityMeasuresAlgoQuad<FIELD>::execute(ProgressReporter *mod, 
						  FieldHandle fieldh)
{
  const int nmetrics = 16;
  FIELD *field = dynamic_cast<FIELD*>(fieldh.get_rep());
  typename FIELD::mesh_type *mesh = 
    dynamic_cast<typename FIELD::mesh_type *>(fieldh->mesh().get_rep());

  double node_pos[4][3];
  vector<typename FIELD::mesh_type::Elem::index_type> elemmap;
  typename FIELD::mesh_type::Elem::iterator bi, ei;
  mesh->begin(bi); mesh->end(ei);

  //Output a nrrd with all the metrics per element.
  typename FIELD::mesh_type::Elem::size_type sz;
  mesh->size(sz);
  const unsigned nelems = sz;
  Nrrd* onnrd = nrrdNew();
  nrrdAlloc_va(onnrd, nrrdTypeDouble, 2, nmetrics, nelems);
  nrrdAxisInfoSet_va(onnrd, nrrdAxisInfoLabel, "element", "metrics");
  NrrdDataHandle output = new NrrdData(onnrd);
  double *data = (double*)onnrd->data;
  
  // record the indices for the metrics.
  nrrdKeyValueAdd(onnrd, "aspect", "0");
  nrrdKeyValueAdd(onnrd, "skew", "1");
  nrrdKeyValueAdd(onnrd, "taper", "2");
  nrrdKeyValueAdd(onnrd, "warpage", "3");
  nrrdKeyValueAdd(onnrd, "area", "4");
  nrrdKeyValueAdd(onnrd, "stretch", "5");
  nrrdKeyValueAdd(onnrd, "minimum_angle", "6");
  nrrdKeyValueAdd(onnrd, "maximum_angle", "7");
  nrrdKeyValueAdd(onnrd, "condition", "8");
  nrrdKeyValueAdd(onnrd, "jacobian", "9");
  nrrdKeyValueAdd(onnrd, "scaled_jacobian", "10");
  nrrdKeyValueAdd(onnrd, "shear", "11");
  nrrdKeyValueAdd(onnrd, "shape", "12");
  nrrdKeyValueAdd(onnrd, "shear_size", "13");
  nrrdKeyValueAdd(onnrd, "shape_size", "14");
  nrrdKeyValueAdd(onnrd, "distortion", "15");


  mesh->synchronize( Mesh::EDGES_E );
  
  int total_elements = 0;
  // each metric will have per elem value, low and high and ave values...
  double metrics[nmetrics][4];
  
  int inversions = 0;
  bool first_time_thru = true;
  
  while (bi != ei)
  {
    typename FIELD::mesh_type::Node::array_type onodes;
    mesh->get_nodes(onodes, *bi);

    vtkQuad *quad = vtkQuad::New(); 
    
    for(int i = 0; i < 4; ++i)
    {
      Point p;
      mesh->get_center( p, onodes[i] );
      quad->GetPoints()->SetPoint(i, p.x(), p.y(), p.z());
    }
    metrics[0][0] = vtkMeshQuality::QuadMaxAspectFrobenius(quad);     
    metrics[1][0] = vtkMeshQuality::QuadSkew(quad);    
    metrics[2][0] = vtkMeshQuality::QuadTaper(quad);         
    metrics[3][0] = vtkMeshQuality::QuadWarpage(quad);      
    metrics[4][0] = vtkMeshQuality::QuadArea(quad);       
    metrics[5][0] = vtkMeshQuality::QuadStretch(quad); 
    metrics[6][0] = vtkMeshQuality::QuadMinAngle(quad);          
    metrics[7][0] = vtkMeshQuality::QuadMaxAngle(quad);   
    metrics[8][0] = vtkMeshQuality::QuadCondition(quad);
    metrics[9][0] = vtkMeshQuality::QuadJacobian(quad);
    metrics[10][0] = vtkMeshQuality::QuadScaledJacobian(quad);
    metrics[11][0] = vtkMeshQuality::QuadShear(quad);
    metrics[12][0] = vtkMeshQuality::QuadShape(quad);
    metrics[13][0] = vtkMeshQuality::QuadShearAndSize(quad);
    metrics[14][0] = vtkMeshQuality::QuadShapeAndSize(quad);
    metrics[15][0] = vtkMeshQuality::QuadDistortion(quad);
     
    quad->Delete();
    for(int i = 0; i < nmetrics; ++i)
    {    
      data[i] = metrics[i][0];
    }
    data += nmetrics;

    if (first_time_thru)
    {
      for (int j = 0; j < nmetrics; ++j) {
        metrics[j][1] = metrics[j][0];
        metrics[j][2] = metrics[j][0];
        metrics[j][3] = 0.0L;
      }
      first_time_thru = false;
    }

    // check high, low, and total for the average;
    for (int j = 0; j < nmetrics; ++j)
    {
      if( metrics[j][0] > metrics[j][2] )
        metrics[j][2] = metrics[j][0];
      else if(metrics[j][0] < metrics[j][1])
        metrics[j][1] = metrics[j][0];
      metrics[j][3] += metrics[j][0];
    }

    typename FIELD::mesh_type::Elem::index_type elem_id = *bi;
    // 10 is the scaled jacobian...
    if (metrics[10][0] <= 0.0 )
    {
      inversions++;
      cout << "WARNING: Quad " << elem_id << " has negative area!" << endl;
    }

    total_elements++;
    ++bi;
  }
  
  for (int j = 0; j < nmetrics; ++j) {
    metrics[j][3] /= (double)total_elements;
  }

  typename FIELD::mesh_type::Node::size_type nodes;
  typename FIELD::mesh_type::Edge::size_type edges;
  typename FIELD::mesh_type::Face::size_type faces;
  mesh->size( nodes );
  mesh->size( edges );
  mesh->size( faces );
  int holes = (faces - edges + nodes - 2) / 2;

  cout << endl << "Number of Quad elements checked = " << total_elements;
  if( inversions != 0 )
    cout << " (" << inversions << " Quads have negative jacobians!)";
  cout << endl << "Euler characteristics for this mesh indicate " << holes 
       << " holes in this block of elements." << endl 
       << "    (Assumes a single contiguous block of elements.)" << endl;
  cout << "Quads: " << faces << " Edges: " << edges << " Nodes: " << nodes 
       << endl;
  cout << "Aspect Ratio: Low = " << metrics[0][1] << ", Average = " 
       << metrics[0][3] << ", High = " << metrics[0][2] << endl;
  cout << "Skew: Low = " << metrics[1][1] << ", Average = " 
       << metrics[1][3] << ", High = " << metrics[1][2] << endl;
  cout << "Taper: Low = " << metrics[2][1] << ", Average = " 
       << metrics[2][3] << ", High = " << metrics[2][2] << endl;
  cout << "Warpage: Low = " << metrics[3][1] << ", Average = " 
       << metrics[3][3] << ", High = " << metrics[3][2] << endl;
  cout << "Area: Low = " << metrics[4][1] << ", Average = " 
       << metrics[4][3] << ", High = " << metrics[4][2] << endl;
  cout << "Stretch: Low = " << metrics[5][1] << ", Average = " 
       << metrics[5][3] << ", High = " << metrics[5][2] << endl;
  cout << "Minimum_Angle: Low = " << metrics[6][1] << ", Average = " 
       << metrics[6][3] << ", High = " << metrics[6][2] << endl;
  cout << "Maximum_Angle: Low = " << metrics[7][1] << ", Average = " 
       << metrics[7][3] << ", High = " << metrics[7][2] << endl;
  cout << "Condition: Low = " << metrics[8][1] << ", Average = " 
       << metrics[8][3] << ", High = " << metrics[8][2] << endl;
  cout << "Jacobian: Low = " << metrics[9][1] << ", Average = " 
       << metrics[9][3] << ", High = " << metrics[9][2] << endl;
  cout << "Scaled_Jacobian: Low = " << metrics[10][1] << ", Average = " 
       << metrics[10][3] << ", High = " << metrics[10][2] << endl;
  cout << "Shear: Low = " << metrics[11][1] << ", Average = " 
       << metrics[11][3] << ", High = " << metrics[11][2] << endl;
  cout << "Shape: Low = " << metrics[12][1] << ", Average = " 
       << metrics[12][3] << ", High = " << metrics[12][2] << endl;
  cout << "Shear_Size: Low = " << metrics[13][1] << ", Average = " 
       << metrics[13][3] << ", High = " << metrics[13][2] << endl;
  cout << "Shape_Size: Low = " << metrics[14][1] << ", Average = " 
       << metrics[14][3] << ", High = " << metrics[14][2] << endl;
  cout << "Distortion: Low = " << metrics[15][1] << ", Average = " 
       << metrics[15][3] << ", High = " << metrics[15][2] << endl;

   return output;
}

} // end namespace SCIRun

#endif // ReportMeshQualityMeasures_h
