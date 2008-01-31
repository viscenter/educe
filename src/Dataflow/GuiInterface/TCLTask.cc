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
 *  TCLTask.cc
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   March 2006
 *
 *  Copyright (C) 2006 SCI Group
 */

#include <iostream>
#include <Dataflow/GuiInterface/TCLTask.h>

namespace SCIRun {

//! Initiate the lock
RecursiveMutex TCLTask::tcl_lock_("TCL Thread Lock");


#ifndef EXPERIMENTAL_TCL_THREAD

//! Lock the TCL thread
void
TCLTask::lock()
{
  tcl_lock_.lock();
}

//! Unlock the TCL thread
void
TCLTask::unlock()
{
  tcl_lock_.unlock();
}

//! Try to lock the TCL thread
bool
TCLTask::try_lock()
{
  return (tcl_lock_.tryLock());
}

#else

//! Initiate the thread pointer with an invalid pointer
Thread* TCLTask::tcl_thread_ = 0;
int     TCLTask::tcl_use_cnt_ = 0;
int     TCLTask::tcl_thread_use_ = 0;
bool    TCLTask::signal_tcl_when_free_ = false;
Tcl_ThreadId TCLTask::tcl_thread_id_ = 0;

//! Lock the TCL thread
void
TCLTask::lock()
{
  //! If we are inside the TCL thread we should not lock
  if (tcl_thread_ == Thread::self())
  {
    tcl_use_cnt_++;
  }
  else
  {
    tcl_lock_.lock();
  }
}

//! Unlock the TCL thread
void
TCLTask::unlock()
{
  if (tcl_thread_ == Thread::self())
  {
    tcl_use_cnt_--;
    if (tcl_use_cnt_ < 1)
    {
      tcl_use_cnt_ = 0;
      if (signal_tcl_when_free_)
      {
        Tcl_ThreadAlert(tcl_thread_id_);
      }
    }
  }
  else
  {
    tcl_lock_.unlock();
  }
}

//! Try to lock the TCL thread
bool
TCLTask::try_lock()
{
  //! If we are inside the TCL Thread we should not lock
  if (tcl_thread_ == Thread::self())
  {
    //! Inside we can always lock
    tcl_use_cnt_++;
    return (true);
  }
  else
  {
    if (tcl_lock_.tryLock())
    {
      return (true);
    }
    return (false);
  }
}

void
TCLTask::mark_as_tcl_thread()
{
  tcl_thread_ = Thread::self();
}

bool
TCLTask::is_tcl_thread()
{
  return(tcl_thread_ == Thread::self());
}


bool
TCLTask::tcl_in_use()
{
  return(tcl_use_cnt_ > 0);
}

void 
TCLTask::set_tcl_thread_id(Tcl_ThreadId thread_id)
{ 
  tcl_thread_id_ = thread_id;
  signal_tcl_when_free_ = true;
}

Tcl_ThreadId 
TCLTask::get_tcl_thread_id()
{ 
  return(tcl_thread_id_);
}


#endif

}
