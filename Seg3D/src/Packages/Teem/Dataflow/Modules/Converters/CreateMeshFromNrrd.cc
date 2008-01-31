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
 *  CreateMeshFromNrrd.cc:
 *
 *  Create a Field Mesh from Nrrd Daata. Incoming Nrrds may consist of
 *  mesh points and optionally mesh connections.
 *
 *  Written by:
 *   Allen R. Sanderson
 *   SCI Institute
 *   University of Utah
 *   Febuary 2007
 *
 *  Copyright (C) 2007 SCI Institute
 */

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/NrrdPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace SCITeem {

using namespace SCIRun;
using namespace std;

class CreateMeshFromNrrd : public Module {
public:
  CreateMeshFromNrrd(GuiContext* ctx);
  virtual ~CreateMeshFromNrrd();
  virtual void execute();

protected:
  GuiString   gui_QuadOrTet_;
  GuiString   gui_StructOrUnstruct_;
  GuiString   gui_Datasets_;
};


DECLARE_MAKER(CreateMeshFromNrrd)
CreateMeshFromNrrd::CreateMeshFromNrrd(GuiContext* ctx)
  : Module("CreateMeshFromNrrd", ctx, Source, "Converters", "Teem"),
    gui_QuadOrTet_(get_ctx()->subVar("quad-or-tet"), "Auto"),
    gui_StructOrUnstruct_(get_ctx()->subVar("struct-or-unstruct"), "Auto"),
    gui_Datasets_(get_ctx()->subVar("datasets"), "")
{
}


CreateMeshFromNrrd::~CreateMeshFromNrrd()
{
}

void
CreateMeshFromNrrd::execute()
{
  NrrdDataHandle point_nrrd_input_handle;
  NrrdDataHandle connection_nrrd_input_handle;
  
  // Get required input mesh points
  if (!get_input_handle("Mesh Points", point_nrrd_input_handle, true ) )
    return;

  // Get optional input mesh connections
  get_input_handle("Mesh Connections", connection_nrrd_input_handle, false );


  if( inputs_changed_ ) {
    // set the names of the datasets for the UI
    string datasetsStr = "";
    string property;

    if (point_nrrd_input_handle != 0) {
      if (point_nrrd_input_handle->get_property( "Name", property ) && property != "Unknown")
	datasetsStr.append( "{Points : " + property + "} ");
      else
	datasetsStr.append( "{Points : Unknown} " );
    } else {
      datasetsStr.append( "{Points : (none)} " );
    }
    if (connection_nrrd_input_handle != 0) {
      if (connection_nrrd_input_handle->get_property( "Name", property ) && property != "Unknown")
	datasetsStr.append( "{Connections: " + property + "} ");
      else
	datasetsStr.append( "{Connections : Unknown} " );
    } else {
      datasetsStr.append( "{Connections : (none)} " );
    }

    gui_Datasets_.reset();

    if( datasetsStr != gui_Datasets_.get() ) {
      // Update the dataset names and dims in the GUI.
      ostringstream str;
      str << get_id() << " set_names {" << datasetsStr << "}";
      
      get_gui()->execute(str.str().c_str());
    }
  }

  if( inputs_changed_ ||
      gui_QuadOrTet_.changed( true ) ||
      gui_StructOrUnstruct_.changed( true ) ||
      !oport_cached("Output Field") ) {

    FieldHandle field_output_handle;

    string QuadOrTet = gui_QuadOrTet_.get();
    string StructOrUnstruct = gui_StructOrUnstruct_.get();

    SCIRunAlgo::FieldsAlgo algo(this);
    if (!(algo.CreateMeshFromNrrd( point_nrrd_input_handle,
				   connection_nrrd_input_handle,
				   field_output_handle,
				   QuadOrTet, StructOrUnstruct))) return;
    
    // set the name of the field to be that of the points
    string property;
    if (point_nrrd_input_handle->get_property( "Name", property ))
      field_output_handle->set_property( "Name", property, false);

    send_output_handle("Output Field", field_output_handle, true);
  }
}

} // End namespace Teem
