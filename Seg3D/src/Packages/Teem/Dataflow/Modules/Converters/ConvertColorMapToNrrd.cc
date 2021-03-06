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
 *  ConvertColorMapToNrrd.cc: Converts a SCIRun ColorMap to Nrrd(s).  
 *
 *  Written by:
 *   Darby Van Uitert
 *   April 2004
 *
 */

#include <Dataflow/Network/Module.h>
#include <Core/Malloc/Allocator.h>

#include <Dataflow/Network/Ports/NrrdPort.h>
#include <Dataflow/Network/Ports/ColorMapPort.h>


namespace SCITeem {

using namespace SCIRun;

class ConvertColorMapToNrrd : public Module {
public:

  int          colormap_generation_;

  ConvertColorMapToNrrd(GuiContext*);

  virtual ~ConvertColorMapToNrrd();

  virtual void execute();
};


DECLARE_MAKER(ConvertColorMapToNrrd)
ConvertColorMapToNrrd::ConvertColorMapToNrrd(GuiContext* ctx)
  : Module("ConvertColorMapToNrrd", ctx, Source, "Converters", "Teem"),
    colormap_generation_(-1)
{
}


ConvertColorMapToNrrd::~ConvertColorMapToNrrd()
{
}


void
ConvertColorMapToNrrd::execute()
{
  ColorMapHandle cmapH;
  if (!get_input_handle("ColorMap", cmapH)) return;

  if (colormap_generation_ != cmapH->generation ||
      !oport_cached("Output"))
  {
    colormap_generation_ = cmapH->generation;

    const unsigned int size = cmapH->resolution();
  
    NrrdData *nd = scinew NrrdData();
    size_t s[NRRD_DIM_MAX];
    s[0] = 4; s[1] = size;
    nrrdAlloc_nva(nd->nrrd_, nrrdTypeFloat, 2, s);
    nd->nrrd_->axis[0].kind = nrrdKindUnknown;
    nd->nrrd_->axis[1].kind = nrrdKindDomain;

    float *val = (float *)nd->nrrd_->data;
    const float *data = cmapH->get_rgba();

    memcpy(val, data, sizeof(float) * 4 * size);

    // Send the data nrrd.
    nd->nrrd_->axis[0].label = airStrdup("Colors");
    NrrdDataHandle dataH(nd);

    send_output_handle("Output", dataH);
  }
}


} // End namespace Teem


