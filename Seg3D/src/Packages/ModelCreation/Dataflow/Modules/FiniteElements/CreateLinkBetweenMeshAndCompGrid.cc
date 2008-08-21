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

#include <Core/Algorithms/FiniteElements/Mapping/BuildFEGridMapping.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>

#include <Dataflow/Network/Module.h>

namespace ModelCreation {

using namespace SCIRun;

class CreateLinkBetweenMeshAndCompGrid : public Module {
  public:
    CreateLinkBetweenMeshAndCompGrid(GuiContext*);
    virtual void execute();

  private: 
    SCIRunAlgo::BuildFEGridMappingAlgo algo_;
};


DECLARE_MAKER(CreateLinkBetweenMeshAndCompGrid)
CreateLinkBetweenMeshAndCompGrid::CreateLinkBetweenMeshAndCompGrid(GuiContext* ctx)
  : Module("CreateLinkBetweenMeshAndCompGrid", ctx, Source, "FiniteElements", "ModelCreation")
{
  algo_.set_progress_reporter(this);
  algo_.set_bool("build_potential_geomtogrid",false);
  algo_.set_bool("build_current_gridtogeom",false);
}


void CreateLinkBetweenMeshAndCompGrid::execute()
{
  MatrixHandle NodeLink;
  MatrixHandle GeomToComp, CompToGeom;
  
  if (!(get_input_handle("NodeLink",NodeLink,true))) return;
  
  if (inputs_changed_ || !oport_cached("GeomToComp") || !oport_cached("CompToGeom"))
  {
    MatrixHandle Dummy1, Dummy2;
    algo_.run(NodeLink,Dummy1,CompToGeom,GeomToComp,Dummy2);
    send_output_handle("GeomToComp",GeomToComp,false);
    send_output_handle("CompToGeom",CompToGeom,false);
  }
}

} // End namespace ModelCreation
