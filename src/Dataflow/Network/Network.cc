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


 

#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/Scheduler.h>
#include <Dataflow/Network/PackageDB.h>
#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/Module.h>
#include <Core/Malloc/Allocator.h>
#include <Core/Containers/StringUtil.h>
#include <Core/Util/Environment.h>


#ifdef _WIN32
#include <io.h>
#endif
#include <stdio.h>
#include <iostream>

namespace SCIRun {

using namespace std;



Network::Network() :
  ref_cnt(0),
  lock("Network lock"), 
  wait_delete_("Signal module deletion"),
  sched(0),
  network_error_code_(0),
  network_dynamic_compiles_(0)
{
  if (sci_getenv("SCIRUN_LOGFILE"))
  {
    std::string logfile = sci_getenv("SCIRUN_LOGFILE");
    log_file_ = scinew LogFile(logfile);
    log_file_->putmsg("SCIRUN: CREATED NEW NETWORK");
  }
}

 
Network::~Network()
{
}


// For now, we just use a simple mutex for both reading and writing
void
Network::read_lock()
{
  lock.lock();
}


void
Network::read_unlock()
{
  lock.unlock();
}


void
Network::write_lock()
{
  lock.lock();
}


void
Network::write_unlock()
{
  lock.unlock();
}



int
Network::nmodules()
{
  lock.lock();
  int sz = static_cast<int>(modules.size());
  lock.unlock();
  return (sz);
}

ModuleHandle
Network::module(int i)
{

  ModuleHandle handle;
  lock.lock();
  handle = modules[i];
  lock.unlock();
  return (handle);

}

void
Network::pre_save_network()
{
  lock.lock();
  for(unsigned int i=0; i<modules.size(); i++) modules[i]->presave();
  lock.unlock();  
}

void
Network::reset_scheduler()
{
  lock.lock();
  for(unsigned int i=0; i<modules.size(); i++) modules[i]->need_execute_=0;
  lock.unlock();

}


string
Network::connect(ModuleHandle m1, int p1, ModuleHandle m2, int p2)
{
  // assure that the ports are not altered while connecting
  m1->oports_.lock();
  m2->iports_.lock();
  
  // dynamic port safeguard.
  if (m2->lastportdynamic_ && p2 >= m2->iports_.size()) {
    p2 = m2->iports_.size() - 1;
  }
  
  if (p1 >= m1->num_output_ports() || p2 >= m2->num_input_ports())
  {
    return "";
  }


  ostringstream ids;
  ids << m1->id_ << "_p" << p1 << "_to_" << m2->id_ << "_p" << p2;
  

  ConnectionHandle conn = scinew Connection(m1, p1, m2, p2, ids.str());
  
  lock.lock();

  connections.push_back(conn);


  // Reschedule next time we can.
  reschedule=1;

  lock.unlock();


  m1->oports_.unlock();
  m2->iports_.unlock();

  return (conn->id);
}


int
Network::disconnect(const string& connId)
{

  lock.lock();

  unsigned int i;
  for (i = 0; i < connections.size(); i++)
  {
    if (connections[i]->id == connId)
    {
      break;
    }
  }
  if (i == connections.size())
  {
    lock.unlock();
    return 0;
  }
 
  ConnectionHandle conn = connections[i];
  connections.erase(connections.begin() + i);   
  lock.unlock();

  
  conn->disconnect();
  conn = 0;

  return 1;
}


static string
remove_spaces(const string& str)
{
  string result;
  for (string::const_iterator i = str.begin(); i != str.end(); i++)
  {
    if (*i != ' ') { result += *i; }
  }
  return result;
}


// SCIRunComponentModel uses Network::add_module2 to instantiate a
// SCIRun module in the SCIRun2 framework.
ModuleHandle
Network::add_module2(const string& packageName,
                     const string& categoryName,
                     const string& moduleName)
{
  ModuleHandle handle = add_module(packageName, categoryName, moduleName);
  handle->get_gui()->eval("addModule2 "+packageName+" "+categoryName+
                          " "+moduleName+" "+handle->id_);
  return handle;
}


ModuleHandle
Network::add_module_maybe_replace(const string& oldPackageName,
                                  const string& oldCategoryName,
                                  const string& oldModuleName,
                                  const string& oldVersionName,
                                  string &packageName,
                                  string &categoryName,
                                  string &moduleName,
                                  string &versionName)
{
  packageName = oldPackageName;
  categoryName = oldCategoryName;
  moduleName = oldModuleName;
  versionName = oldVersionName;
  if (versionName == "") versionName = "1.0";

  lock.lock();

  while (!packageDB->haveModule(packageName, categoryName, moduleName))
  {
    if (!packageDB->replaceDeprecatedModule(packageName, categoryName,
                                            moduleName,
                                            packageName, categoryName,
                                            moduleName))
    {
      break;
    }
  }
  lock.unlock();

  return add_module(packageName, categoryName, moduleName);
}
  



ModuleHandle
Network::add_module(const string& packageName,
                    const string& categoryName,
                    const string& moduleName)
{ 
  lock.lock();

  const string name = 
    remove_spaces(packageName + "_" + categoryName + "_" + moduleName + "_");
  
  // Find a unique id in the Network for the new instance of this module and
  // form an instance name from it
  string mod_id;

  MapStringModule::iterator mod;
  int i = 0;  
  bool uniquename_found = false;
  
  while (uniquename_found == false)
  {
    mod_id = name+to_string(i);
    mod = module_ids.find(mod_id);
  
    if (mod == module_ids.end())
      uniquename_found = true;
    i++;
  }

  // Instantiate the module and save the structure to a handle
  // Both the NetworkEditor and the Module thread share the structure
  // and whoever deletes it last will destroy the module
  ModuleHandle handle = packageDB->instantiateModule(packageName, categoryName,
                                                     moduleName, mod_id);
  if (!handle.get_rep()) 
  {
    lock.unlock();
    return handle;
  }

  // Save handle in network object
  modules.push_back(handle);
  module_ids[mod_id] = 0;

  lock.unlock();
        
  // Binds NetworkEditor and Network instances to module instance.  
  // Instantiates ModuleHelper and starts event loop.
  handle->set_context(this);

  // add Module id and ptr to Module to hash table of modules in network

  lock.lock();
  module_ids[handle->id_] = handle;
  lock.unlock();

  handle->gui_->add_command(handle->id_+"-c", handle.get_rep(), 0);
  
  return handle;
}


void
Network::add_instantiated_module(ModuleHandle handle)
{
  if(!handle.get_rep()) {
    cerr << "Error: can't add instanated module\n";
    return;
  }
        
  lock.lock();
  modules.push_back(handle);
  lock.unlock();
        
  // Binds NetworkEditor and Network instances to module instance.
  // Instantiates ModuleHelper and starts event loop.
  handle->set_context(this);
  
  // add Module id and ptr to Module to hash table of modules in network
  lock.lock();
  module_ids[handle->id_] = handle;
  lock.unlock();
        
  GuiInterface* gui = handle->gui_;
  // Add a TCL command for this module...
  gui->add_command(handle->id_+"-c", handle.get_rep(), 0);

  gui->eval("addModule2 unknown unknown unknown "+handle->id_);
}


ModuleHandle
Network::get_module_by_id(const string& id)
{
  ModuleHandle handle = 0;
  lock.lock();

  MapStringModule::iterator mod;
  mod = module_ids.find(id);
  if (mod != module_ids.end())
  {
    handle = (*mod).second;
  }
  lock.unlock();

  return (handle);
}


class DeleteModuleThread : public Runnable 
{
private:

  Network *net_;
  ModuleHandle module_;

public:

  DeleteModuleThread(Network *net, ModuleHandle module) :
    net_(net),
    module_(module)
  {
    ASSERT(net);
    ASSERT(module.get_rep());
  }

  void run()
  {
    // Get module ID
    string mod_id = module_->get_id();

    net_->write_lock();
    net_->module_ids[mod_id] = 0;

    unsigned int vpos = 0;
    while (vpos<net_->modules.size() && net_->modules[vpos].get_rep()!=module_.get_rep()) ++vpos;
    ASSERT(vpos<net_->modules.size());
    net_->modules.erase(net_->modules.begin()+vpos);            
    net_->write_unlock();

    module_->delete_thread();
    
    module_ = 0;

    net_->write_lock();
    Network::MapStringModule::iterator const mpos = net_->module_ids.find(mod_id);
    ASSERT(mpos != net_->module_ids.end());
    net_->module_ids.erase(mpos);

    //Signal that we are done with deleting
    net_->wait_delete_.conditionBroadcast();
    net_->write_unlock();


  }
};



// Delete_module will start a seperate thead that waits until a module
// is done executing before deleting it.
int
Network::delete_module(const string& id)
{
  ModuleHandle handle = get_module_by_id(id);
  if (!handle.get_rep()) return 0;
    
  DeleteModuleThread * dm = scinew DeleteModuleThread(this,handle);
  const string tname("Delete module: " + id);
  Thread *mod_deleter = scinew Thread(dm, tname.c_str());
  mod_deleter->detach();

  return 1;
}



void
Network::schedule()
{
  if (log_file_.get_rep()) log_file_->putmsg("SCHEDULER: START SCHEDULING");
  sched->do_scheduling();
}


void
Network::schedule_all()
{
  if (sci_getenv_p("SCI_REGRESSION_TESTING"))
  {
    //    static int excounter = 0;
    //    excounter++;
    //    cout << "STARTING EXECUTION " + to_string(excounter) + "\n";
  }

  lock.lock();
  for(unsigned int i=0; i<modules.size(); i++)
  {
    modules[i]->need_execute_ = true;
  }
  lock.unlock();
  
  schedule();
}


void
Network::attach(Scheduler* _sched)
{
  sched = _sched;
}


void
Network::disable_connection(const string& connId)
{
  lock.lock();
  for (unsigned int i = 0; i < connections.size(); i++)
    if (connections[i]->id == connId)
    {
      connections[i]->disabled_ = true;
      lock.unlock();
      return;
    }
  lock.unlock();
}


Scheduler*
Network::get_scheduler()
{
  return sched;
}

}

