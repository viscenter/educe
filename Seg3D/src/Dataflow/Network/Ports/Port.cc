/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2004 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
 *  Port.cc: Classes for module ports
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   March 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

#include <Dataflow/Network/Ports/Port.h>
#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/Module.h>
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace SCIRun;

Port::Port(Module* module, const string& type_name,
	   const string& port_name, const string& color_name)
: lock("port lock"), module(module), which_port(-1), portstate(Off),
  type_name(type_name), port_name(port_name), color_name(color_name)
{
}

int
Port::nconnections()
{
  int sz;
  lock.lock();
  sz = connections.size();
  lock.unlock();
  return (sz);
}

ModuleHandle
Port::get_module()
{
  return ModuleHandle(module);
}

int
Port::get_which_port()
{
  return which_port;
}

void
Port::attach(Connection* conn)
{
  ConnectionHandle conn_handle(conn);
  lock.lock();
  connections.push_back(conn_handle);
  module->connection(Connected, which_port, this==conn->oport.get_rep());
  lock.unlock();
}

void
Port::detach(Connection* conn, bool blocked)
{
  lock.lock();
  unsigned int i;
  for (i=0; i<connections.size(); i++)
    if (connections[i].get_rep() == conn) break;
  if (i == connections.size()) {
    cerr << "Error: connection not found from "
	 << conn->oport->get_module()->get_modulename() << " ("
	 << conn->oport->get_portname() << ") to "
	 << conn->iport->get_module()->get_modulename() << " ("
	 << conn->iport->get_portname() << ")" << endl;
    return;
  }
  connections.erase(connections.begin() + i);
  module->connection(Disconnected, which_port, this==conn->oport.get_rep() || blocked);
  lock.unlock();
}

void
Port::reset()
{
}

void
Port::finish()
{
}

ConnectionHandle
Port::connection(int i)
{
  return connections[i];
}

void
Port::update_light()
{
}

IPort::IPort(Module* module, const string& type_name,
	     const string& port_name, const string& color_name)
  : Port(module, type_name, port_name, color_name)
{
}

void
IPort::attach(Connection* conn)
{
  // Maybe this should be more user friendly?  You have to edit a .svn
  // by hand to add multiple connections this way, but it's possible
  // to have someone hand you a bad network and have it ASSERT out on
  // the command line.
  lock.lock();
  ASSERTMSG(connections.size() == 0,
            "TOO MANY CONNECTIONS ON IPORT.");
  Port::attach(conn);
  lock.unlock();
}

OPort::OPort(Module* module, const string& type_name,
	     const string& port_name, const string& color_name)
  : Port(module, type_name, port_name, color_name)
{
}

void
Port::set_which_port(int wp)
{
    which_port=wp;
}

void
IPort::update_light()
{
    char* color;
    switch(portstate){
    case Resetting:
	color="blue";
	break;
    case Finishing:
	color="\"dark violet\"";
	break;
    case On:
	color="red";
	break;
    case Off:
    default:
	color="black";
	break;
    }
    char str[1000];
    sprintf(str,"lightPort {%s %d i} %s",module->id_.c_str(),which_port,color);
    module->get_gui()->execute(str);
}

void
OPort::update_light()
{
    char* color;
    switch(portstate){
    case Resetting:
	color="blue";
	break;
    case Finishing:
	color="\"dark violet\"";
	break;
    case On:
	color="red";
	break;
    case Off:
    default:
	if(have_data()){
	    color="DarkOrange";
	} else {
	    color="black";
	}
	break;
    }
    char str[1000];
    sprintf(str,"lightPort {%s %d o} %s",module->id_.c_str(),which_port,color);
    module->get_gui()->execute(str);
}

void
Port::turn_on_light(PortState st)
{
    portstate=st;
    update_light();
}

void
Port::turn_off_light()
{
    portstate=Off;
    update_light();
}

void
Port::synchronize()
{
}

string
Port::get_typename()
{
  return type_name;
}

string
Port::get_portname()
{
  return port_name;
}

Port::~Port()
{
}

IPort::~IPort()
{
}

OPort::~OPort()
{
}

string
Port::get_colorname()
{
  return color_name;
}

void IPort::deactivate()
{
}

void
OPort::issue_no_port_caching_warning()
{
  // Don't really care about thread safety since the worst that happens is
  // multiple warnings are printed.  Not worth creating the lock over.
  static bool warnonce = true;
  if (warnonce)
  {
    warnonce = false;
    std::cerr << "Warning: SCIRUN_NO_PORT_CACHING on!!!\n";
    std::cerr << "Warning: This will break interactive SCIRun usage.\n";
    std::cerr << "Warning: See .scirunrc for details.\n";
  }
}


bool OPort::cache_flag_supported() { return false; }
bool OPort::get_cache() { return true; }
void OPort::set_cache(bool cache) {}

