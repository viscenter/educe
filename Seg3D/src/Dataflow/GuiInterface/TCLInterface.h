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

#ifndef SCIRun_Core_GuiInterface_TCLInterface_h
#define SCIRun_Core_GuiInterface_TCLInterface_h

#include <Core/Thread/Semaphore.h>
#include <Dataflow/GuiInterface/GuiCallback.h>
#include <Dataflow/GuiInterface/GuiInterface.h>

#include <string>

#include <Dataflow/GuiInterface/share.h>

namespace SCIRun {
  using namespace std;
  class TCLInterface;
  class GuiContext;

  class SCISHARE EventMessage {
    public:
      //! The constructor creates the semaphore 
      EventMessage() :
        return_code_(0),
        delivery_semaphore_(new Semaphore("TCL EventMessage Delivery",0))
      {}

      //! delete the semaphore if it was created
      virtual ~EventMessage()
      {
        if (delivery_semaphore_) delete delivery_semaphore_;
      }
            
      void wait_for_message_delivery()
      {
        //! Wait until TCL task has processed this event
        delivery_semaphore_->down();
      }
      
      void mark_message_delivered()
      {
        //! Tell semaphore the processing is done
        delivery_semaphore_->up();
      }
      
      //! Result from TCL
      string& result() { return result_; }

      //! Resulting return code from TCL function call
      int&  code() { return return_code_; } 

      //! Function to call by TCL thread
      virtual void execute();
      
    private:
      string              result_;
      int                 return_code_;
      Semaphore *         delivery_semaphore_;
  };

  //! Event for getting value of a variable
  class SCISHARE GetEventMessage : public EventMessage {
    public:
      GetEventMessage(const string &var, 
                      const string& key,
                      GuiContext* ctx = 0) : 
        var_(var), 
        key_(key),
        ctx_(ctx) 
      {}
    
      //! function to call by event handler
      virtual void execute();
    
    private:
      string              var_;
      string              key_;
      GuiContext*         ctx_;
  };

  //! Event for setting a variable
  class SCISHARE SetEventMessage : public EventMessage {
    public:
      SetEventMessage(const string &var, 
                      const string& val, 
                      const string& key,
                      GuiContext* ctx = 0) : 
        var_(var), 
        val_(val), 
        key_(key),
        ctx_(ctx)
      {}
      
      //! function to call by event handler
      virtual void execute();
      
    private:
      string              var_;
      string              val_;
      string              key_;
      GuiContext*         ctx_;
  };


  class SCISHARE AddCommandEventMessage : public EventMessage {
    public:
      AddCommandEventMessage(const string &command, 
                             GuiCallback* callback, 
                             void* userdata) : 
        command_(command), 
        callback_(callback), 
        userdata_(userdata) 
      {} 

      //! function to call by event handler
      virtual void execute();
      
    private:
      string              command_;
      GuiCallback*        callback_;
      void*               userdata_;
  };


  class SCISHARE DeleteCommandEventMessage : public EventMessage {
    public:
      DeleteCommandEventMessage(const string &command) : 
        command_(command) 
      {} 

      //! function to call by event handler
      virtual void execute();

    private:
      string              command_;
    };


  class SCISHARE CommandEventMessage : public EventMessage {
    public:
      CommandEventMessage(const string &command,
                          GuiContext* ctx = 0) : 
        command_(command),
        ctx_(ctx) 
      {} 

      //! function to call by event handler
      virtual void execute();

    private:
      string              command_;
      GuiContext*         ctx_;
    };





  class SCISHARE TCLInterface : public GuiInterface{
    Semaphore *         pause_semaphore_;
    bool                paused_;
  public:
    TCLInterface();
    virtual ~TCLInterface();
    virtual void execute(const string& str, GuiContext* ctx = 0);
    virtual int eval(const string& str, string& result, GuiContext* ctx = 0);
    virtual string eval(const string &, GuiContext* ctx = 0);
    virtual void pause();
    virtual void real_pause();
    virtual void unpause();
    virtual void source_once(const string&);
    virtual void add_command(const string&, GuiCallback*, void*);
    virtual void delete_command( const string& command );
    virtual void lock();
    virtual void unlock();
    virtual GuiContext* createContext(const string& name);
    virtual void post_message(const string& errmsg, bool err = false);
    // Get TCL array var, which resembles a STL a map<string, string>

    virtual bool get(const std::string& name, std::string& value, GuiContext* ctx = 0);
    virtual void set(const std::string& name, const std::string& value, GuiContext* ctx = 0);

    // Get TCL array var, which resembles a STL a map<string, string>
    virtual bool get_map(const std::string& name, 
			 const std::string &key,
			 std::string& value, 
       GuiContext* ctx = 0);
    virtual bool set_map(const std::string& name, 
			 const std::string &key,
			 const std::string& value, 
       GuiContext* ctx = 0);

    // Get an element of regular tcl list
    virtual bool extract_element_from_list(const std::string& list_contents, 
					   const vector<int> &indexes, 
					   std::string& value, GuiContext* ctx = 0);
    virtual bool set_element_in_list(std::string& list_contents, 
				     const vector<int> &indexes, 
				     const std::string& value, GuiContext* ctx = 0);

    bool complete_command(const string&);


  };
}

#endif

