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
 *  Connection.h: A Connection between two modules
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   March 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

#ifndef DATAFLOW_NETWORK_CONNECTION_H
#define DATAFLOW_NETWORK_CONNECTION_H 1

#include <Core/Thread/Mutex.h>
#include <Core/Thread/RecursiveMutex.h>
#include <Core/Containers/LockingHandle.h>
#include <string>

#include <Dataflow/Network/share.h>

namespace SCIRun {
  class IPort;
  class Module;
  class OPort;

typedef LockingHandle<IPort>  IPortHandle;
typedef LockingHandle<OPort>  OPortHandle;
typedef LockingHandle<Module> ModuleHandle;

class SCISHARE Connection {
public:

  int ref_cnt;  // for LockingHandle
  RecursiveMutex lock;   // for LockingHandle
  
  Connection(ModuleHandle omod, int oportno, ModuleHandle imod, int imodno,
	     const std::string &id);
  ~Connection();

  OPortHandle oport;
  IPortHandle iport;
  ModuleHandle omod;
  ModuleHandle imod;
  
  std::string id;
  bool disabled_;

  void disconnect();
  
  void makeID();
};

typedef LockingHandle<Connection> ConnectionHandle;

} // End namespace SCIRun


#endif 
