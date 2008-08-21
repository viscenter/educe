
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
 *  TCLTask.h:  Handle TCL
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   August 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

#ifndef DATAFLOW_GUIINTERFACE_TCLTASK_H
#define DATAFLOW_GUIINTERFACE_TCLTASK_H 1

#include <Core/Thread/Thread.h>
#include <Core/Thread/RecursiveMutex.h>

#include <tcl.h>
#if (TCL_MINOR_VERSION >= 4)
#define TCLCONST const
#else
#define TCLCONST
#endif

#include <Dataflow/GuiInterface/share.h>

#if (TCL_MINOR_VERSION >= 4)
#  define EXPERIMENTAL_TCL_THREAD 1
#endif


//#define EXPERIMENTAL_TCL_THREAD 1

namespace SCIRun {

class SCISHARE TCLTask {

  private:
    //! Lock guarding the access to TCL
    static RecursiveMutex tcl_lock_;


  public:
    //! Lock the gui to have exclusive access to TCL
    static void lock();
    static void unlock();
    static bool try_lock();

#ifdef EXPERIMENTAL_TCL_THREAD
  private:
    //! The thread accessing TCL needs to have itsm own locking mechanism
    //! Hence we need to be able to check whether we are inside the TCL thread
    //! or not. Hence we store the pointer to the task having access to TCL
    static Thread* tcl_thread_;
    
    //! This counter counts the number of internal locks and unlocks
    //! As this one is only accessed from the TCLTask itself it does not need a
    //! mutex.
    static int     tcl_use_cnt_;
    static int     tcl_thread_use_;
    
    //! Set up a signaling loop back to the TCL interface for signalling when 
    //! TCL is free for receiving instructions
    static bool signal_tcl_when_free_;
    static Tcl_ThreadId tcl_thread_id_;
  
  public:
    //! Mark the current thread as the thread having access to TCL
    static void mark_as_tcl_thread();
    
    //! Check whether we are inside the TCL Thread
    static bool is_tcl_thread();
    
    //! Check whether TCL is in use
    static bool tcl_in_use();
    
    //! Store the pointer to the TCL Thread
    static void set_tcl_thread_id(Tcl_ThreadId thread_id);
    
    //! Get the pointer
    static Tcl_ThreadId get_tcl_thread_id();
#endif

};
  
} // End namespace SCIRun


#endif
