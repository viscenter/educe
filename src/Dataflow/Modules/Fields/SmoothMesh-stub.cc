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
 *  SmoothMesh-stub.cc: Place holder for when an optional third party
 *  package isn't installed.
 *
 *  Written by:
 *   James Bigler
 *   Department of Computer Science
 *   University of Utah
 *   October 2007
 *
 *  Copyright (C) 2007 SCI Group
 */

#include <Dataflow/Network/Module.h>

namespace SCIRun {

  class SmoothMesh : public Module
  {
  public:
    SmoothMesh(GuiContext* ctx);
    virtual ~SmoothMesh();

    virtual void execute();
  };


  DECLARE_MAKER(SmoothMesh)


  SmoothMesh::SmoothMesh(GuiContext* ctx)
  : Module("SmoothMesh", ctx, Filter, "ChangeMesh", "SCIRun")
  {
  }


  SmoothMesh::~SmoothMesh()
  {
  }


  void
  SmoothMesh::execute()
  {
    error("This module is disabled, because the necessary supporting libraries were not found at build time.");
  }

} // End namespace SCIRun

