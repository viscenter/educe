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
 *  Connection.cc: A Connection between two modules
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   March 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/Port.h>
#include <Core/Math/MinMax.h>
#include <Core/Math/MiscMath.h>
#include <Dataflow/GuiInterface/GuiContext.h>
#include <Core/Containers/StringUtil.h>

#include <iostream>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

namespace SCIRun {

Connection::Connection(ModuleHandle m1, int p1, ModuleHandle m2, int p2,
		       const string &id)
  : ref_cnt(0),
    lock("Connection lock"),
    omod(m1),
    imod(m2),
    id(id),
    disabled_(false)
{
  m1->get_oport_handle(p1,oport);
  m2->get_iport_handle(p2,iport);

  iport->attach(this);
  oport->attach(this);
  makeID();
}

void
Connection::disconnect()
{
  if (oport.get_rep() && iport.get_rep())
  {
    oport->detach(this, false);
    iport->detach(this, disabled_);  
  }
  id = "";
  oport = 0;
  iport = 0;
  omod = 0;
  imod = 0;
}

Connection::~Connection()
{
  disconnect();
}

void Connection::makeID()
{
  if (!oport.get_rep() || !iport.get_rep() || !omod.get_rep() || !imod.get_rep()) return;
  omod->get_gui()->eval("makeConnID {"+ omod->get_id()+" "+
      to_string(oport->get_which_port())+" "+ imod->get_id()+" "+
      to_string(iport->get_which_port())+"}", id);
}

}

