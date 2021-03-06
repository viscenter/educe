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
 *  ReportScalarFieldStats: Store/retrieve values from an input matrix to/from 
 *            the data of a field
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   February 2001
 *
 *  Copyright (C) 2001 SCI Institute
 */

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Modules/Fields/ReportScalarFieldStats.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Core/Containers/StringUtil.h>
#include <Core/Containers/Handle.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>

namespace SCIRun {


DECLARE_MAKER(ReportScalarFieldStats)
ReportScalarFieldStats::ReportScalarFieldStats(GuiContext* ctx)
  : Module("ReportScalarFieldStats", ctx, Filter, "MiscField", "SCIRun"),
    min_(get_ctx()->subVar("min"), 0.0), 
    max_(get_ctx()->subVar("max"), 0.0),
    mean_(get_ctx()->subVar("mean"), 0.0),
    median_(get_ctx()->subVar("median"), 0.0),
    sigma_(get_ctx()->subVar("sigma"),0.0),
    is_fixed_(get_ctx()->subVar("is_fixed"), 0),
    nbuckets_(get_ctx()->subVar("nbuckets"), 256)
{
}



ReportScalarFieldStats::~ReportScalarFieldStats()
{
}


void
ReportScalarFieldStats::clear_histogram()
{
  get_gui()->execute(get_id() + " clear_data");
}


void
ReportScalarFieldStats::fill_histogram( vector<int>& hits)
{
  ostringstream ostr;
  int nmin, nmax;
  vector<int>::iterator it = hits.begin();
  nmin = 0;  nmax = *it;
  ostr << *it;  ++it;
  for(; it != hits.end(); ++it){
    ostr <<" "<<*it;
    nmin = ((nmin < *it) ? nmin : *it );
    nmax = ((nmax > *it) ? nmax : *it );
  }
  ostr <<std::ends;
  string smin( to_string(nmin) );
  string smax( to_string(nmax) );

  string data = ostr.str();
  get_gui()->execute(get_id() + " graph_data " + smin + " "
	       + smax + " " + data );
}


void
ReportScalarFieldStats::execute()
{
  // Get input field.
  FieldHandle ifieldhandle;
  if (!get_input_handle("Input Field", ifieldhandle)) return;

  if (!ifieldhandle->query_scalar_interface(this).get_rep())
  {
    error("This module only works on scalar fields.");
    return;
  }

  const TypeDescription *ftd = ifieldhandle->get_type_description();
  const TypeDescription *ltd = ifieldhandle->order_type_description();
  CompileInfoHandle ci = ReportScalarFieldStatsAlgo::get_compile_info(ftd, ltd);
  Handle<ReportScalarFieldStatsAlgo> algo;
  if (!module_dynamic_compile(ci, algo)) return;

  algo->execute(ifieldhandle, this);
}


CompileInfoHandle
ReportScalarFieldStatsAlgo::get_compile_info(const TypeDescription *field_td,
				     const TypeDescription *loc_td)
{
  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string template_class_name("ReportScalarFieldStatsAlgoT");
  static const string base_class_name("ReportScalarFieldStatsAlgo");

  CompileInfo *rval = 
    scinew CompileInfo(template_class_name + "." +
		       field_td->get_filename() + "." +
		       loc_td->get_filename() + ".",
                       base_class_name, 
                       template_class_name, 
                       field_td->get_name() + ", " + loc_td->get_name());

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  field_td->fill_compile_info(rval);
  return rval;
}


} // End namespace SCIRun
