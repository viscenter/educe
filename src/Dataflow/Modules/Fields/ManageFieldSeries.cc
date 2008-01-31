/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2007 Scientific Computing and Imaging Institute,
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
 *  ManageFieldSeries: Set a property for a Field
 *
 *  Written by:
 *   Allen R. Sanderson
 *   SCI Institute
 *   University of Utah
 *   Febuary 2007
 *
 *  Copyright (C) 2007 SCI Institute
 */

#include <Dataflow/Network/ManageSeriesModule.h>
#include <Dataflow/Network/Ports/FieldPort.h>

namespace SCIRun {

class ManageFieldSeries : public ManageSeriesModule< FieldHandle > {
public:
  ManageFieldSeries(GuiContext* ctx);
};

DECLARE_MAKER(ManageFieldSeries)
ManageFieldSeries::ManageFieldSeries(GuiContext* context)
: ManageSeriesModule< FieldHandle> ("ManageFieldSeries",
				    context, Filter,
				    "MiscField", "SCIRun", "Field")
{
}

} // End namespace SCIRun
