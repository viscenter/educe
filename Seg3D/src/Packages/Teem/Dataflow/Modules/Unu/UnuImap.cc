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
 *  UnuImap.cc 
 *
 *  Written by:
 *   Darby Van Uitert
 *   April 2004
 *
 */

#include <Dataflow/Network/Module.h>
#include <Core/Malloc/Allocator.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Dataflow/Network/Ports/NrrdPort.h>

#include <Core/Containers/StringUtil.h>

namespace SCITeem {

using namespace SCIRun;

class UnuImap : public Module {
public:
  UnuImap(GuiContext*);

  virtual ~UnuImap();

  virtual void execute();

private:
  GuiInt       length_;
  GuiInt       rescale_;
  GuiDouble    min_;
  GuiInt       useinputmin_;
  GuiDouble    max_;
  GuiInt       useinputmax_;
  GuiString    type_;
  GuiInt       usetype_;
};


DECLARE_MAKER(UnuImap)
UnuImap::UnuImap(GuiContext* ctx)
  : Module("UnuImap", ctx, Source, "UnuAtoM", "Teem"),
    length_(get_ctx()->subVar("length")),
    rescale_(get_ctx()->subVar("rescale")),
    min_(get_ctx()->subVar("min")),
    useinputmin_(get_ctx()->subVar("useinputmin")),
    max_(get_ctx()->subVar("max")),
    useinputmax_(get_ctx()->subVar("useinputmax")),
    type_(get_ctx()->subVar("type")),
    usetype_(get_ctx()->subVar("usetype"))
{
}


UnuImap::~UnuImap()
{
}


void
UnuImap::execute()
{
  update_state(NeedData);

  NrrdDataHandle nrrd_handle;
  if (!get_input_handle("InputNrrd", nrrd_handle)) return;

  NrrdDataHandle dmap_handle;
  if (!get_input_handle("IrregularMapNrrd", dmap_handle)) return;

  reset_vars();

  Nrrd *nin = nrrd_handle->nrrd_;
  Nrrd *dmap = dmap_handle->nrrd_;
  Nrrd *nout = nrrdNew();
  Nrrd *nacl = 0;
  NrrdRange *range = NULL;

  if (length_.get()) {
    nacl = nrrdNew();
    if (nrrd1DIrregAclGenerate(nacl, dmap, length_.get())) {
      char *err = biffGetDone(NRRD);
      error(string("Error generating accelerator: ") + err);
      free(err);
      return;
    }
  } else {
    nacl = NULL;
  }

  const int rescale = rescale_.get();
  if (rescale)
  {
    double min = AIR_NAN, max = AIR_NAN;
    if (!useinputmin_.get())
      min = min_.get();
    if (!useinputmax_.get())
      max = max_.get();
    range = nrrdRangeNew(min, max);
    nrrdRangeSafeSet(range, nin, nrrdBlind8BitRangeState);
  }

  if (usetype_.get())
  {
    if (nrrdApply1DIrregMap(nout, nin, range, dmap,
                            nacl, dmap->type, rescale))
    {
      char *err = biffGetDone(NRRD);
      error(string("Error Mapping Nrrd to Lookup Table: ") + err);
      free(err);
      return;
    }
  }
  else
  {
    if (nrrdApply1DIrregMap(nout, nin, range, dmap, nacl,
                            string_to_nrrd_type(type_.get()), rescale))
    {
      char *err = biffGetDone(NRRD);
      error(string("Error Mapping Nrrd to Lookup Table: ") + err);
      free(err);
      return;
    }
  }

  NrrdDataHandle out(scinew NrrdData(nout));

  // Copy the properties.
  out->copy_properties(nrrd_handle.get_rep());

  // Copy the axis kinds
  for (unsigned int i=0; i<nin->dim && i<nout->dim; i++)
  {
    nout->axis[i].kind = nin->axis[i].kind;
  }

  send_output_handle("OutputNrrd", out);
}


} // End namespace Teem




