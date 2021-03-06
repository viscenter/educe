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
 *  ViewLeadSignals.cc:
 *
 *  Written by:
 *   mcole
 *   TODAY'S DATE HERE
 *
 */

#include <Dataflow/Network/Module.h>
#include <Core/Malloc/Allocator.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Dataflow/GuiInterface/GuiVar.h>

namespace BioPSE {

using namespace SCIRun;

class ViewLeadSignals : public Module {
public:
  ViewLeadSignals(GuiContext *context);

  virtual ~ViewLeadSignals();

  virtual void execute();

private:
  GuiString units_;
  GuiDouble tmin_;
  GuiDouble tmax_;
  
  int gen_;
};


DECLARE_MAKER(ViewLeadSignals)


ViewLeadSignals::ViewLeadSignals(GuiContext *context) : 
  Module("ViewLeadSignals", context, Source, "Visualization", "BioPSE"),
  units_(context->subVar("time-units")),
  tmin_(context->subVar("time-min")),
  tmax_(context->subVar("time-max")),
  gen_(-1)
{
}


ViewLeadSignals::~ViewLeadSignals()
{
}


void
ViewLeadSignals::execute()
{
  MatrixHandle mh;
  if (!get_input_handle("Potentials", mh)) return;

  if (mh->generation == gen_) { 
    remark("Same input matrix, nothing changed.");
    return; 
  }
  gen_ = mh->generation;

  ostringstream clr;
  clr << get_id() << " clear";
  get_gui()->execute(clr.str().c_str());
  

  int rows = mh->nrows();
  int cols = mh->ncols();
  ostringstream set_mm;
  set_mm << get_id() << " set_min_max_index 0 " << rows - 1; 
  get_gui()->execute(set_mm.str().c_str());

  for(int i = 0; i < rows; i++) {
    ostringstream cmmd;
    ostringstream xstr;
    ostringstream ystr;
    xstr << "{";
    ystr << " {";
    cmmd << get_id() << " add_lead " << i << " ";
    for (int j = 0; j < cols; j++) {
      xstr << j/(float)cols << " "; // the time inc we are at
      ystr << mh->get(i, j) << " "; // the value at i
    }
    xstr << "}";
    ystr << "}";
    cmmd << xstr.str();
    cmmd << ystr.str();
    get_gui()->execute(cmmd.str().c_str());
  }

  // set params from properties before drawing leads
  string units;
  double start;
  double end;
  if (mh.get_rep() && mh->get_property(string("time-units"), units)) {
    units_.set(units.c_str());
  }  
  if (mh.get_rep() && mh->get_property(string("time-start"), start)) {
    tmin_.set(start);
  }  
  if (mh.get_rep() && mh->get_property(string("time-end"), end)) {
    tmax_.set(end);
  }  

  ostringstream cmmd;
  cmmd << get_id() << " draw_leads";
  get_gui()->execute(cmmd.str().c_str());
}

} // End namespace BioPSE


