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

#include <Core/Basis/NoData.h>
#include <Core/Basis/TriLinearLgn.h>
#include <Core/Datatypes/TriSurfMesh.h>
#include <Core/Datatypes/GenericField.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/NrrdPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

typedef TriSurfMesh<TriLinearLgn<Point> > TSMesh;

MeshHandle fast_lat_boundaries(Nrrd *nrrd, bool close);

class GetAllSegmentationBoundaries : public Module {
public:
  GetAllSegmentationBoundaries(GuiContext*);
  virtual void execute();
};


DECLARE_MAKER(GetAllSegmentationBoundaries)
GetAllSegmentationBoundaries::GetAllSegmentationBoundaries(GuiContext* ctx)
  : Module("GetAllSegmentationBoundaries", ctx, Source, "NewField", "SCIRun")
{
}


void
GetAllSegmentationBoundaries::execute()
{
  // Declare dataflow object
  NrrdDataHandle nrrdh;
  
  // Get data from ports
  if (!(get_input_handle("Segmentations", nrrdh, true))) return;
  
  // If parameters changed, do algorithm
  if (inputs_changed_ || !oport_cached("Boundaries") && nrrdh->nrrd_)
  {
    // Send Data flow objects downstream
    MeshHandle m = fast_lat_boundaries(nrrdh->nrrd_, true);
    TSMesh *tsm = (TSMesh *)(m.get_rep());
    GenericField<TSMesh, NoDataBasis<double>, vector<double> > *tsfield =
      scinew GenericField<TSMesh, NoDataBasis<double>, vector<double> >(tsm);
    FieldHandle out(tsfield);
    send_output_handle("Boundaries", out);
  }
}

} // End namespace SCIRun