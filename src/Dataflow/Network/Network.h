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
 *  Network.h: Interface to Network class from project
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   March 1994
 *
 *  Distributed Dataflow changes:
 *   Michelle Miller
 *   Nov. 1997
 *
 *  Copyright (C) 1994 SCI Group
 */

#ifndef DATAFLOW_NETWORK_NETWORK_H
#define DATAFLOW_NETWORK_NETWORK_H 1

#include <Core/Thread/ConditionVariable.h>
#include <Core/Thread/Mutex.h>
#include <Core/Thread/RecursiveMutex.h>
#include <Core/Util/LogFile.h>
#include <Core/Containers/LockingHandle.h>

#include <string>
#include <vector>
#include <map>

#include <Dataflow/Network/share.h>

namespace SCIRun {

using std::string;
using std::vector;
using std::map;

class Connection;
class Module;
class GuiInterface;
class Scheduler;
class DeleteModuleThread;

typedef LockingHandle<Module> ModuleHandle;
typedef LockingHandle<Connection> ConnectionHandle;
  
class SCISHARE Network {
  friend class DeleteModuleThread;
public:

  typedef map<string, ModuleHandle>	MapStringModule;
  
  int            ref_cnt;
  RecursiveMutex lock;
        
	ConditionVariable wait_delete_;
private:

  MapStringModule module_ids;
    
  vector<ConnectionHandle> connections;
  vector<ModuleHandle> modules;
    
  int reschedule;
  Scheduler* sched;
  
  int network_error_code_;
  int network_dynamic_compiles_;

  LogFileHandle log_file_;
  
public:
  Network();
  ~Network();

  void read_lock();
  void read_unlock();
  void write_lock();
  void write_unlock();

  int nmodules();
  ModuleHandle module(int i);
  
  string connect(ModuleHandle, int, ModuleHandle, int);
  int disconnect(const string&);
  void disable_connection(const string&);

  ModuleHandle add_module(const string& packageName,
		     const string& categoryName,
		     const string& moduleName);
  // For SCIRun2
  ModuleHandle add_module2(const string& packageName,
		      const string& categoryName,
		      const string& moduleName);

  // This version looks up the module in module-remapping.txt if it
  // does not otherwise exist and uses the new replacement module
  // instead.  Use this for Network IO.
  ModuleHandle add_module_maybe_replace(const string& packageName,
                                        const string& categoryName,
                                        const string& moduleName,
                                        const string& versionName,
                                        string &newPackageName,
                                        string &newCategoryName,
                                        string &newModuleName,
                                        string &newVersionName);

  void add_instantiated_module(ModuleHandle mod);

  int delete_module(const string& name);
  ModuleHandle get_module_by_id(const string& name);

  // Change status of network
  void pre_save_network();
  void reset_scheduler();

  void schedule();
  void schedule_all();
  Scheduler* get_scheduler();
  void attach(Scheduler*);

  int  get_network_error_code() { return(network_error_code_); }  
  void set_network_error_code(int code) { network_error_code_ =  code; }
  void incr_network_error_code() { write_lock(); network_error_code_++; write_unlock(); }

  int  get_network_dynamic_compiles() { return(network_dynamic_compiles_); }  
  void set_network_dynamic_compiles(int code) { network_dynamic_compiles_ =  code; }
  void incr_network_dynamic_compiles() { write_lock(); network_dynamic_compiles_++; write_unlock(); }
  
  void add_log(std::string logtext)
    { if(log_file_.get_rep()) log_file_->putmsg(logtext); }

};

} // End namespace SCIRun


#endif
