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



/*
 *  BuildPointCloudToLatVolMappingMatrix.cc:  
 *   Builds mapping matrix that projects data from a PointCloud to a LatVol
 *
 *  Written by:
 *   McKay Davis
 *   Scientific Computing and Imaging INstitute
 *   University of Utah
 *   May 2005
 *
 *  Copyright (C) 2005 SCI Group
 */

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Basis/Constant.h>
#include <Core/Datatypes/PointCloudMesh.h>
#include <Core/Basis/HexTrilinearLgn.h>
#include <Core/Datatypes/LatVolMesh.h>
#include <Core/Datatypes/GenericField.h>
#include <Core/Math/MiscMath.h>
#include <Core/Geometry/BBox.h>
#include <Dataflow/GuiInterface/UIvar.h>
#include <vector>
#include <map>

namespace SCIRun {

class BuildPointCloudToLatVolMappingMatrix : public Module
{

typedef LatVolMesh<HexTrilinearLgn<Point> > LVMesh;
typedef PointCloudMesh<ConstantBasis<Point> > PCMesh;

private:
  UIdouble		epsilon_;
  int			pcf_generation_;
  int			lvf_generation_;
  double		epsilon_cache_;
public:
  BuildPointCloudToLatVolMappingMatrix(GuiContext* ctx);
  virtual ~BuildPointCloudToLatVolMappingMatrix();

  virtual void execute();
};


DECLARE_MAKER(BuildPointCloudToLatVolMappingMatrix)

BuildPointCloudToLatVolMappingMatrix::BuildPointCloudToLatVolMappingMatrix(GuiContext* ctx)
  : Module("BuildPointCloudToLatVolMappingMatrix", ctx, Filter, "MiscField", "SCIRun"),
    epsilon_(get_ctx()->subVar("epsilon"), 0.0),
    pcf_generation_(-1),
    lvf_generation_(-1)
{
}


BuildPointCloudToLatVolMappingMatrix::~BuildPointCloudToLatVolMappingMatrix()
{
}


void
BuildPointCloudToLatVolMappingMatrix::execute()
{
  // Get the PointCloudField from the first port
  FieldHandle pcf;
  if (!get_input_handle("PointCloudField", pcf)) return;

  // Get the LatVolField from the first port
  FieldHandle lvf;
  if (!get_input_handle("LatVolField", lvf)) return;

  // Make sure the first input field is of type PointCloudField
  if (pcf->get_type_description()->get_name().find("PointCloudField")) {
    error("Field connected to port 1 must be PointCloudField.");
    return;
  }

  // Make sure the second input field is of type LatVolField
  if (lvf->get_type_description()->get_name().find("LatVolField")) {
    error("Field connected to port 2 must be LatVolField.");
    return;
  }
  
  // Return if nothing in gui or fields has changed
  if (pcf->generation == pcf_generation_ &&
      lvf->generation == lvf_generation_ &&
      epsilon_() == epsilon_cache_ &&
      oport_cached("MappingMatrix"))
  {
    return;
  }

  pcf_generation_ = pcf->generation;
  lvf_generation_ = lvf->generation;
  epsilon_ = Clamp(epsilon_(), 0.0, 1.0);
  epsilon_cache_ = epsilon_;

  // Get the meshes from the fields
  PCMesh::handle_type pcm = (PCMesh *)(pcf->mesh().get_rep());
  LVMesh::handle_type lvm = (LVMesh *)(lvf->mesh().get_rep());

  // LVMesh Node Count
  LVMesh::Node::size_type lvmns;
  lvm->size(lvmns);

  // PointClouldMesh Node Count
  PCMesh::Node::size_type pcmns;
  pcm->size(pcmns);
  
  // LVMesh Node Iterators
  LVMesh::Node::iterator lvmn, lvmne;
  lvm->begin(lvmn);
  lvm->end(lvmne);

  // PCMesh Node Iterators
  PCMesh::Node::iterator pcmn, pcmne;
  pcm->end(pcmne);

  // Map point to distance, sorts by point index
  typedef map<unsigned int, double> point2dist_t;

  double epsilon = lvm->get_bounding_box().diagonal().length() * epsilon_;

  // To automatically handle memory allocation to hold data for the
  // sparse matrix, we use STL vectors here
  vector<Matrix::index_type> rows(lvmns+1);
  vector<Matrix::index_type> cols;
  vector<double> data;
  Matrix::index_type i = 0, row = 0;

  // Iterate through each point of the LVMesh
  while (lvmn != lvmne) 
  {
    // Get the location of this node of the LVMesh
    Point lvp;
    lvm->get_point(lvp, *lvmn);
    // Map from PCMesh node index to distance
    point2dist_t point2dist;
    // Total holds the total distance calculation to all points for normalization
    double total = 0.0;
    // Foreach node of LVMesh, iterate through each node of PCMesh
    pcm->begin(pcmn);
    while (pcmn != pcmne) {
      // Get the location of this node of the PCMesh
      Point pcp;
      pcm->get_point(pcp, *pcmn);
      // Do the distance function calculation: 1/d - epsilon
      double d = 1.0/(pcp-lvp).length() - epsilon;
      // If the function is positive, the PCMesh node contributes
      // to this node of the LVMesh
      if (d > 0.0) 
      {
        // Insert it and increase the normalization total
        point2dist.insert(make_pair((*pcmn).index_, d));
        total += d;
      } 
      // Next PCMeshNode please
      ++pcmn;
    }
    
    // Hack to avoid divide by zero
    if (total == 0.0) 
      total = 1.0;

    // Now fill up the current row of the sparse matrix
    rows[row++] = cols.size();
    // Iterate through all point that contributed in PointCloudNode index order
    point2dist_t::iterator pb = point2dist.begin(), pe = point2dist.end();
    while (pb != pe) {
      // Normalize, and dont add if contribution is nil
      double d = pb->second/total;
      if (d > 0.0000001) 
      { // TODO: Better epsilon checking
        // Add the data to the sparse row matrix
        cols.push_back(pb->first);
        data.push_back(d);
      }
      // Next PCMeshNode/Distance pair please
      ++pb;
    }
    // Next LVMeshNode please
    ++lvmn;
  }
  rows[row] = cols.size();

  // Convert the STL vectors into C arrays for SparseRowMatrix Constructor
  Matrix::size_type rsz = static_cast<Matrix::size_type>(rows.size());
  Matrix::index_type *rowsarr = scinew Matrix::index_type[rsz];
  for (i = 0; i < rsz; ++i) rowsarr[i] = rows[i];

  Matrix::size_type csz = static_cast<Matrix::size_type>(cols.size());
  Matrix::index_type *colsarr = scinew Matrix::index_type[csz];
  for (i = 0; i < csz; ++i) colsarr[i] = cols[i];

  Matrix::size_type dsz = static_cast<Matrix::size_type>(data.size());
  double *dataarr = scinew double[dsz];
  for (i = 0; i < dsz; ++i) dataarr[i] = data[i];

  // Create the SparseRowMatrix to send off
  SparseRowMatrix *matrix = scinew 
    SparseRowMatrix(lvmns, pcmns, rowsarr, colsarr, data.size(), dataarr);

  // DEBUG Validate/Print
  //  matrix->validate();
  //  matrix->dense()->print();

  // Send the mapping matrix downstream
  MatrixHandle mtmp(matrix);
  send_output_handle("MappingMatrix", mtmp);

  //Done!
}


}
