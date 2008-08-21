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
 *  TCLInterface.h:
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   April 2002
 *
 *  Copyright (C) 2002 SCI Group
 */

#include <Dataflow/GuiInterface/TCLInterface.h>
#include <Dataflow/GuiInterface/TCLTask.h>
#include <Dataflow/GuiInterface/GuiContext.h>
#include <Core/Containers/StringUtil.h>

#include <Core/Util/Assert.h>
#include <Core/Util/Environment.h>
#include <Core/Thread/Thread.h>
#include <Core/Thread/Mailbox.h>
#include <Core/Thread/Semaphore.h>
#include <Core/Thread/Time.h>
#include <Core/Exceptions/GuiException.h>
#include <tcl.h>
#include <tk.h>
#include <iostream>
#include <string>

// find a more 'consolidated' place to put this...
#if (TCL_MINOR_VERSION >= 4)
#define TCLCONST const
#else
#define TCLCONST
#endif

#ifdef _WIN32
#  include <windows.h>
#  undef SCISHARE
#  define SCISHARE __declspec(dllexport)
#else
#  define SCISHARE
#endif

using namespace SCIRun;
using std::string;


namespace SCIRun {
  struct TCLCommandData {
    GuiCallback* object;
    void* userdata;
  };
}


#ifdef EXPERIMENTAL_TCL_THREAD
// in windows, we will use a different interface (since it hangs in many
// places the non-windows ones do not).  Instead of each thread potentially
// calling TCL code, and have the GUI lock, we will send the TCL commands
// to the tcl thread via a mailbox, and TCL will get the data via an event 
// callback

//! Mail that the TCL threads checks when idle to see if any of the other
//! threads posted instructions that need to be executed
static Mailbox<EventMessage*> tclQueue("TCL command mailbox", 50);

//! Default time out of no delay. Used when we need to tell TCL when to check
//! again for events. Normally if during setup we see that there are messages
//! use this time out for the initial check
static Tcl_Time tcl_time = {0,0};

//! Function call that TCL eventually calls back up on
int eventCallback(Tcl_Event* event, int flags);

//! Functions passed to TCL
SCISHARE void eventSetup(ClientData cd, int flags);
SCISHARE void eventCheck(ClientData cd, int flags);

int eventCallback(Tcl_Event* event, int flags)
{
  //! Only try to receive a message if TCL is not in use
  if (!TCLTask::tcl_in_use())
  {
    EventMessage* em;
    while (tclQueue.tryReceive(em)) 
    {
      em->execute();
      em->mark_message_delivered();
    }
  }

  return 1;
}


void eventSetup(ClientData cd, int flags)
{
  if (tclQueue.numItems() > 0) 
  {
    Tcl_SetMaxBlockTime(&tcl_time);
  }
}

void eventCheck(ClientData cd, int flags)
{
  if (tclQueue.numItems() > 0) 
  {
    Tcl_Event* event = new Tcl_Event;
    event->proc = eventCallback;
    Tcl_QueueEvent(event, TCL_QUEUE_HEAD);
  }
}

#endif

//! We need this one to access TCL
extern "C" {
  SCISHARE Tcl_Interp* the_interp;
}


namespace SCIRun {

TCLInterface::TCLInterface() :
  pause_semaphore_(new Semaphore("TCL Task Pause Semaphore",0)),
  paused_(0)
                   
{  
  const char *srcdir = sci_getenv("SCIRUN_SRCDIR");
  const char *itcl_dir = sci_getenv("SCIRUN_ITCL_WIDGETS");
  ASSERT(srcdir);
  ASSERT(itcl_dir);
  const std::string src(srcdir);
  const std::string itcl(itcl_dir);

  eval("set scirun2 0");
  eval("lappend auto_path \""+src+"/Dataflow/GUI\" \""+itcl+"\"");
}


TCLInterface::~TCLInterface()
{
}


void TCLInterface::execute(const string& str, GuiContext* ctx)
{
  string result("");
  eval(str, result,ctx);
}


string TCLInterface::eval(const string& str, GuiContext* ctx)
{
  string result("");
  eval(str, result,ctx);
  return result;
}

void
TCLInterface::unpause()
{
}

void
TCLInterface::real_pause()
{
}

void
TCLInterface::pause()
{
}


void TCLInterface::source_once(const string& filename)
{
  string result;
  if(!eval("source \"" + filename+"\"", result)) {
    char* msg = ccast_unsafe("Couldn't source file '" + filename + "'");
    Tcl_AddErrorInfo(the_interp,msg);
    Tk_BackgroundError(the_interp);
  }
}


static int do_command(ClientData cd, Tcl_Interp*, int argc, TCLCONST char* argv[])
{
  TCLCommandData* td=(TCLCommandData*)cd;
  GuiArgs args(argc, argv);
  try {
    td->object->tcl_command(args, td->userdata);
  } catch (const GuiException &exception) {
    args.string_ = exception.message();
    args.have_error_ = true;
    args.have_result_ = true;
  } catch (const string &message) {
    args.string_ = message;
    args.have_error_ = true;
    args.have_result_ = true;
  } catch (const char *message) {
    args.string_ = message;
    args.have_error_ = true;
    args.have_result_ = true;
  }

  if(args.have_result_) {
    Tcl_SetResult(the_interp,
		  strdup(args.string_.c_str()),
		  (Tcl_FreeProc*)free);
  }
  return args.have_error_?TCL_ERROR:TCL_OK;
}


void TCLInterface::lock()
{
  TCLTask::lock();
}


void TCLInterface::unlock()
{
  TCLTask::unlock();
}


GuiContext* 
TCLInterface::createContext(const string& name)
{
  return new GuiContext(this, name);
}


void 
TCLInterface::post_message(const string& errmsg, bool err)
{
  // "Status" could also be "warning", but this function only takes a
  // bool.  In the future, perhas we should update the functions that
  // call post_message to be more expressive.
  // displayErrorWarningOrInfo() is a function in NetworkEditor.tcl.

  string status = "info";
  if( err ) status = "error";

  // Replace any double quotes (") with single quote (') as they break the
  // tcl parser.
  string fixed_errmsg = errmsg;
  for( unsigned int cnt = 0; cnt < fixed_errmsg.size(); cnt++ )
    {
      char ch = errmsg[cnt];
      if( ch == '"' )
	ch = '\'';
      fixed_errmsg[cnt] = ch;
    }
  string command = "displayErrorWarningOrInfo \"" + fixed_errmsg + "\" " + status;
  execute( command );
}


bool
TCLInterface::get(const std::string& name, std::string& value,GuiContext* ctx)
{
#ifndef EXPERIMENTAL_TCL_THREAD
  TCLTask::lock();
  
  if (ctx && !ctx->is_active())
  {
    TCLTask::unlock();
    return (false);
  }
  
  TCLCONST char* l=Tcl_GetVar(the_interp, ccast_unsafe(name),
		     TCL_GLOBAL_ONLY);
  value = l?l:"";
  TCLTask::unlock();
  return l;
#else
  if (TCLTask::is_tcl_thread()) 
  {
    if (ctx && !ctx->is_active()) return (false);
    TCLCONST char* l=Tcl_GetVar(the_interp, ccast_unsafe(name),TCL_GLOBAL_ONLY);
    value = l?l:"";
    return l;
  }
  else 
  {
    TCLTask::lock();
    if (ctx && !ctx->is_active())
    {
      TCLTask::unlock();
      return (false);
    }   
    
    GetEventMessage em(name, "",ctx);
    tclQueue.send(&em);
    Tcl_ThreadAlert(TCLTask::get_tcl_thread_id());
    em.wait_for_message_delivery();
    value = em.result();
    TCLTask::unlock();
    return (value != "");      
  }
#endif
}




void
TCLInterface::set(const std::string& name, const std::string& value,GuiContext* ctx)
{
#ifndef EXPERIMENTAL_TCL_THREAD
  TCLTask::lock();

  if (ctx && !ctx->is_active())
  {
    TCLTask::unlock();
    return;
  }
  
  Tcl_SetVar(the_interp, ccast_unsafe(name),
	     ccast_unsafe(value), TCL_GLOBAL_ONLY);
  TCLTask::unlock();
#else
  if (TCLTask::is_tcl_thread()) 
  {
    if (ctx && !ctx->is_active()) return;
    Tcl_SetVar(the_interp, ccast_unsafe(name), ccast_unsafe(value), TCL_GLOBAL_ONLY);
  }
  else 
  {
    TCLTask::lock();
    if (ctx && !ctx->is_active())
    {
      TCLTask::unlock();
      return;
    }   
    
    SetEventMessage em(name, value, "",ctx);
    tclQueue.send(&em);
    Tcl_ThreadAlert(TCLTask::get_tcl_thread_id());
    em.wait_for_message_delivery();
    TCLTask::unlock();
  }
#endif
}


bool
TCLInterface::get_map(const std::string& name, 
		      const std::string& key, 
		      std::string& value,
          GuiContext* ctx)
{
#ifndef EXPERIMENTAL_TCL_THREAD
  TCLTask::lock();

  if (ctx && !ctx->is_active())
  {
    TCLTask::unlock();
    return (false);
  }
  
  const char* l=Tcl_GetVar2(the_interp, 
			    ccast_unsafe(name),
			    ccast_unsafe(key),
			    TCL_GLOBAL_ONLY);
  value = l?l:"";
  TCLTask::unlock();
  return l;
#else
  if (TCLTask::is_tcl_thread()) 
  {
    if (ctx && !ctx->is_active()) return (false);
    const char* l=Tcl_GetVar2(the_interp, 
			      ccast_unsafe(name),
			      ccast_unsafe(key),
			      TCL_GLOBAL_ONLY);
    value = l?l:"";
  }
  else 
  {
    TCLTask::lock();
    if (ctx && !ctx->is_active())
    {
      TCLTask::unlock();
      return (false);
    }   
    
    GetEventMessage em(name, key,ctx);
    tclQueue.send(&em);
    Tcl_ThreadAlert(TCLTask::get_tcl_thread_id());
    em.wait_for_message_delivery();
    value = em.result();
    TCLTask::unlock();
  }
  return value != "";
#endif
}

		      
bool
TCLInterface::set_map(const std::string& name, 
		      const std::string& key, 
		      const std::string& value,
          GuiContext* ctx)
{
#ifndef EXPERIMENTAL_TCL_THREAD
  TCLTask::lock();

  if (ctx && !ctx->is_active())
  {
    TCLTask::unlock();
    return (false);
  }
  
  const char *l = Tcl_SetVar2(the_interp, 
			      ccast_unsafe(name),
			      ccast_unsafe(key),
			      ccast_unsafe(value), 
			      TCL_GLOBAL_ONLY);
  TCLTask::unlock();
  return l;
#else
  if (TCLTask::is_tcl_thread()) 
  {
    if (ctx && !ctx->is_active()) return (false);
    const char *l = Tcl_SetVar2(the_interp, 
			        ccast_unsafe(name),
			        ccast_unsafe(key),
			        ccast_unsafe(value), 
			        TCL_GLOBAL_ONLY);
    return l;
  }
  else 
  {
    TCLTask::lock();
    if (ctx && !ctx->is_active())
    {
      TCLTask::unlock();
      return (false);
    }    
    
    SetEventMessage em(name, value, key,ctx);
    tclQueue.send(&em);
    Tcl_ThreadAlert(TCLTask::get_tcl_thread_id());
    em.wait_for_message_delivery();
    TCLTask::unlock();
    return em.code() == TCL_OK;
  }
#endif
}


void 
TCLInterface::delete_command( const string& command )
{
#ifndef EXPERIMENTAL_TCL_THREAD
  TCLTask::lock();
  Tcl_DeleteCommand(the_interp, ccast_unsafe(command));
  TCLTask::unlock();
#else
  if (TCLTask::is_tcl_thread())
  {
    Tcl_DeleteCommand(the_interp, ccast_unsafe(command));
  }
  else
  {
    TCLTask::lock();
    DeleteCommandEventMessage em(command);
    tclQueue.send(&em);
    Tcl_ThreadAlert(TCLTask::get_tcl_thread_id());
    em.wait_for_message_delivery();
    TCLTask::unlock();
  }
#endif  
}


void 
TCLInterface::add_command(const string&command , GuiCallback* callback,
			       void* userdata)
{
#ifndef EXPERIMENTAL_TCL_THREAD
  TCLTask::lock();
  TCLCommandData* command_data=new TCLCommandData;
  command_data->object=callback;
  command_data->userdata=userdata;
  Tcl_CreateCommand(the_interp, ccast_unsafe(command),
		    &do_command, command_data, 0);
  TCLTask::unlock();
#else
  if (TCLTask::is_tcl_thread())
  {
    TCLCommandData* command_data=new TCLCommandData;
    command_data->object=callback;
    command_data->userdata=userdata;
    Tcl_CreateCommand(the_interp, ccast_unsafe(command),
          &do_command, command_data, 0);
  }
  else
  {
    TCLTask::lock();
    AddCommandEventMessage em(command,callback,userdata);
    tclQueue.send(&em);
    Tcl_ThreadAlert(TCLTask::get_tcl_thread_id());
    em.wait_for_message_delivery();
    TCLTask::unlock();  
  }
#endif
}



int 
TCLInterface::eval(const string& str, string& result, GuiContext* ctx)
{

#ifndef EXPERIMENTAL_TCL_THREAD

  TCLTask::lock();

  if (ctx && !ctx->is_active())
  {
    TCLTask::unlock();
    return (!TCL_OK);
  }
  
  int code = Tcl_Eval(the_interp, ccast_unsafe(str));
  if(code != TCL_OK){
    Tk_BackgroundError(the_interp);
    result="";
  } 
  else 
  {
    result=string(the_interp->result);
  }
  TCLTask::unlock();
  return code == TCL_OK;
  
#else
    
  //! if we are the TCL Thread, go ahead and execute the command, otherwise
  //! add it to the queue so the tcl thread can get it later
  if (TCLTask::is_tcl_thread())
  {
    //! Lock the TCL task so we are not interupted by a callback from another
    //! thread, this will block receiving events from other threads, the event 
    //! loop will ignore any calls until done.

    if (ctx && !ctx->is_active()) return (!TCL_OK);
        
    int code = Tcl_Eval(the_interp, ccast_unsafe(str));
    if(code != TCL_OK)
    {
      Tk_BackgroundError(the_interp);
    } 
    else 
    {
      result = string(the_interp->result);
    }
    
    return (code == TCL_OK);
  }
  else 
  {

    //! Send message to TCL
    TCLTask::lock();

    if (ctx && !ctx->is_active())
    {
      TCLTask::unlock();
      return (!TCL_OK);
    }
    
    CommandEventMessage em(str,ctx);

    tclQueue.send(&em);
    Tcl_ThreadAlert(TCLTask::get_tcl_thread_id());
    em.wait_for_message_delivery();
    TCLTask::unlock();

    result = em.result();
    return (em.code() == TCL_OK);
  }
#endif
}


bool
TCLInterface::extract_element_from_list(const std::string& contents, 
					const vector <int>& indexes,
					std::string& value,
          GuiContext* ctx)
{
  string command = "lsubindex {"+contents+"}";
  const unsigned int last = indexes.size()-1;
  for (unsigned int i = 0; i <= last; ++i)
    command += " "+to_string(indexes[last-i]);
  return eval(command, value,ctx);
}


bool
TCLInterface::set_element_in_list(std::string& contents, 
				  const vector <int>& indexes,
				  const std::string& value,
          GuiContext* ctx)
{
  string command = "lreplacesubindex {"+contents+"} "+value;
  const unsigned int last = indexes.size()-1;
  for (unsigned int i = 0; i <= last; ++i)
    command += " "+to_string(indexes[last - i]);
  return eval(command, contents,ctx);
}


bool
TCLInterface::complete_command(const string &command)
{
  const int len = command.length()+1;
  char *src = new char[len];
  strcpy (src, command.c_str());
  TCLTask::lock();
  Tcl_Parse parse;
  const int ret_val = Tcl_ParseCommand(0, src, len, 1, &parse);
  TCLTask::unlock();
  delete[] src;
  return (ret_val == TCL_OK);
}


void
EventMessage::execute()
{
  std::cerr << "ERROR IN EVENT CALL\n";
}


void 
GetEventMessage::execute()
{
  if (ctx_ && !ctx_->is_active())
  {
    code() = TCL_OK;
    return;
  }

  const char* res;
  if (key_ == "") {
    res = Tcl_GetVar(the_interp, ccast_unsafe(var_), TCL_GLOBAL_ONLY);
  }
  else {
    res = Tcl_GetVar2(the_interp, ccast_unsafe(var_), ccast_unsafe(key_),
			              TCL_GLOBAL_ONLY);
  }
  
  result() = res?res:"";
  code() = TCL_OK;
}

void 
SetEventMessage::execute()
{  
  if (ctx_ && !ctx_->is_active())
  {
    code() = TCL_OK;
    return;
  }
  
  if (key_ == "") 
  {
    Tcl_SetVar(the_interp, ccast_unsafe(var_), ccast_unsafe(val_), TCL_GLOBAL_ONLY);
    code() = TCL_OK;
  }
  else {
    if (Tcl_SetVar2(the_interp, ccast_unsafe(var_), ccast_unsafe(key_),
			              ccast_unsafe(val_), TCL_GLOBAL_ONLY))
      code() = TCL_OK;
    else
      code() = !TCL_OK;
  }
}

void 
AddCommandEventMessage::execute()
{
  TCLCommandData* command_data=new TCLCommandData;

  command_data->object=callback_;
  command_data->userdata=userdata_;

  if (Tcl_CreateCommand(the_interp, ccast_unsafe(command_), 
                      &do_command, command_data, 0))
    code() = TCL_OK;
  else
    code() = !TCL_OK;
}

void 
DeleteCommandEventMessage::execute()
{
  if (Tcl_DeleteCommand(the_interp, ccast_unsafe(command_)))
    code() = TCL_OK;
  else
    code() = !TCL_OK;
}


void 
CommandEventMessage::execute()
{
  if (ctx_ && !ctx_->is_active())
  {
    result() = "";
    code() = TCL_OK;
    return;
  }
 
  int rcode = Tcl_Eval(the_interp, ccast_unsafe(command_));
  if(rcode != TCL_OK)
  {
    Tk_BackgroundError(the_interp);
  } 
  else 
  {
    result() = the_interp->result;
  }
  
  code() = rcode;
}

}
