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
 *  ClipFieldWithSeedCompiled.cc:  Clip out parts of a field using a seed(s)
 *
 *  Written by:
 *   Allen R. Sanderson
 *   SCI Institute
 *   University of Utah
 *   July 2007
 *
 *  Copyright (C) 2007 SCI Institute
 */

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/NrrdPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Algorithms/Fields/FieldsAlgo.h>

namespace SCIRun {

class ClipFieldWithSeedCompiled : public Module
{
public:
  ClipFieldWithSeedCompiled(GuiContext* ctx);
  virtual ~ClipFieldWithSeedCompiled() {}

  virtual void execute();

  virtual void presave();
  virtual void post_read();

protected:
  GuiString gui_mode_;
  GuiString gui_function_;
};


DECLARE_MAKER(ClipFieldWithSeedCompiled)

ClipFieldWithSeedCompiled::ClipFieldWithSeedCompiled(GuiContext* ctx)
  : Module("ClipFieldWithSeedCompiled", ctx, Filter, "NewField", "SCIRun"),
    gui_mode_(get_ctx()->subVar("mode"), "allnodes"),
    gui_function_(get_ctx()->subVar("function"), "return (fx < 0);")
{
}

void
ClipFieldWithSeedCompiled::execute()
{
  FieldHandle field_input_handle;
  FieldHandle seed_input_handle;

  // Get the input fields.
  if (!get_input_handle("Input", field_input_handle, true ) )
    return;

  if (!get_input_handle("Seeds", seed_input_handle, true ) )
    return;

  StringHandle sHandle;

  if (get_input_handle( "Function", sHandle, false )) {
    gui_function_.set( sHandle->get() );
    gui_function_.reset();

    get_gui()->execute(get_id() + " set_text");
  }

  // Update gui_function_ before the get.
  get_gui()->execute(get_id() + " update_text");

  // Check to see if the input field has changed.
  if( inputs_changed_ ||
      gui_mode_.changed( true ) ||
      gui_function_.changed( true ) ||
      !oport_cached("Clipped") )
  {
    FieldHandle  field_output_handle;
    MatrixHandle matrix_output_handle;

    string mode = gui_mode_.get();
    string function = gui_function_.get();

    SCIRunAlgo::FieldsAlgo algo(this);

    if (!(algo.ClipFieldWithSeed(field_input_handle,
				 seed_input_handle,
				 field_output_handle,
				 matrix_output_handle,
				 mode,
				 function))) return;

    send_output_handle( "Clipped", field_output_handle );

    if( matrix_output_handle.get_rep() ) 
    {

      SparseRowMatrix *matrix =
        dynamic_cast<SparseRowMatrix *>(matrix_output_handle.get_rep());

      size_t dim[NRRD_DIM_MAX];
      dim[0] = matrix->ncols();    

      NrrdDataHandle nrrd_output_handle = new NrrdData;
      Nrrd *nrrd = nrrd_output_handle->nrrd_;
      nrrdAlloc_nva(nrrd, nrrdTypeUChar, 1, dim);
      unsigned char *mask = (unsigned char *)nrrd->data;
      memset(mask, 0, dim[0]*sizeof(unsigned char));
      Matrix::index_type *rr = matrix->rows;
      Matrix::index_type *cc = matrix->columns;
      double *data = matrix->a;

      for (Matrix::index_type i=0; i<matrix->nrows(); i++) 
      {
        if (rr[i+1] == rr[i]) continue; // No entires on this row
        Matrix::index_type col = cc[rr[i]];
        if (data[rr[i]] > 0.0) {
          mask[col] = 1;
        }
      }

      send_output_handle( "Mapping", matrix_output_handle );
      send_output_handle( "MaskVector", nrrd_output_handle );
    }
  }
}


void
ClipFieldWithSeedCompiled::presave()
{
  // update gui_function_ before saving.
  get_gui()->execute(get_id() + " update_text");
}

void
ClipFieldWithSeedCompiled::post_read()
{
  string function = gui_function_.get();

  if (function.find("return") == string::npos)
  {
    function = string("return (") + function + string( ");" );

    gui_function_.set( function );
    gui_function_.reset();

    get_gui()->execute(get_id() + " set_text");
  }
}

} // End namespace SCIRun

