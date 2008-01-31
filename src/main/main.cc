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
 *  main.cc: 
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   Feb. 1994
 *
 *  Copyright (C) 1999 U of U
 */

#include <main/sci_version.h>

#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/PackageDB.h>
#include <Dataflow/Network/NetworkIO.h>
#include <Dataflow/Network/Scheduler.h>
#include <Dataflow/TCLThread/TCLThread.h>
#include <Core/Containers/StringUtil.h>
#include <Dataflow/GuiInterface/TCLInterface.h>
#include <Core/Init/init.h>
#include <Core/Util/Environment.h>
#include <Core/Util/FileUtils.h>
#include <Core/Util/sci_system.h>
#include <Core/Thread/Thread.h>
#include <Core/Thread/Time.h>
#include <Core/Events/EventManager.h>

#include <Core/Services/ServiceLog.h>
#include <Core/Services/ServiceDB.h>
#include <Core/Services/ServiceManager.h>
#include <Core/SystemCall/SystemCallManager.h>

#include <TauProfilerForSCIRun.h>

#include <sci_defs/ptolemy_defs.h>

#include <string>
#include <iostream>
using std::cout;

#ifdef _WIN32
#  include <windows.h>
#endif

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#  pragma set woff 1209
#  pragma set woff 1424
#endif

using namespace SCIRun;

void
usage()
{
  cout << "Usage: scirun [args] [net_file] [session_file]\n";
  cout << "    [-]-d[atadir]            : scirun data directory\n";
  cout << "    [-]-r[egression]         : regression test a network\n";
  cout << "    [-]-R[egressionimagedir] : output directory for regression tests\n";
  cout << "    [-]-s[erver] [PORT]      : start a TCL server on port number PORT\n";
  cout << "    [-]-e[xecute]            : executes the given network on startup\n";
  cout << "    [-]-E[xecute]            : executes the given network on startup and quits when done\n";
  cout << "    [-]-c[onvert]            : converts a .net to a .srn network and exits\n";
  cout << "    [-]-v[ersion]            : prints out version information\n";
  cout << "    [-]-h[elp]               : prints usage information\n";
  cout << "    [-]-p[ort] [PORT]        : start remote services port on port number PORT\n";
  cout << "    [-]-l[ogfile] file       : add output messages to a logfile\n";
#ifdef HAVE_PTOLEMY_PACKAGE
  cout << "    [-]-SPAs[erver]          : start the S.P.A. server thread\n";
#endif
  cout << "    [--nosplash]             : disable the splash screen\n";
  cout << "    net_file                 : SCIRun Network Input File\n";
  cout << "    session_file             : PowerApp Session File\n";
  exit( 0 );
}

static bool doing_convert_ = false;
// Parse the supported command-line arugments.
// Returns the argument # of the .net file
int
parse_args( int argc, char *argv[] )
{
  int found = 0;
  bool powerapp = false;
  int cnt = 1;
  
  while (cnt < argc)
  {
    string arg( argv[ cnt ] );
    if( ( arg == "--version" ) || ( arg == "-version" ) || ( arg == "-v" ) || ( arg == "--v" ) )
    {
      cout << "Version: " << SCIRUN_VERSION << "\n";
      exit( 0 );
    }
    else if ( ( arg == "--help" ) || ( arg == "-help" ) || ( arg == "-h" ) ||  ( arg == "--h" ) )
    {
      usage();
    }
    else if ( ( arg == "--execute" ) || ( arg == "-execute" ) || ( arg == "-e" ) ||  ( arg == "--e" ) )
    {
      sci_putenv("SCIRUN_EXECUTE_ON_STARTUP","1");
    }
    else if ( ( arg == "--Execute" ) || ( arg == "-Execute" ) || ( arg == "-E" ) ||  ( arg == "--E" ) )
    {
      sci_putenv("SCIRUN_EXECUTE_ON_STARTUP","1");
      sci_putenv("SCIRUN_QUIT_WHEN_DONE","1");
    }
    else if ( ( arg == "--convert" ) || ( arg == "-convert" ) || ( arg == "-c" ) ||  ( arg == "--c" ) )
    {
      sci_putenv("SCIRUN_CONVERT_NET_TO_SRN","1");
      doing_convert_ = true;
    }
    else if ( ( arg == "--eai" ) || ( arg == "-eai" ))
    {
      sci_putenv("SCIRUN_EXTERNAL_APPLICATION_INTERFACE","1");
    }
    else if ( ( arg == "--regression" ) || ( arg == "-regression" ) || ( arg == "-r" ) ||  ( arg == "--r" ) )
    {
      sci_putenv("SCI_REGRESSION_TESTING","1");
    }
    else if ( ( arg == "--Regressionimagedir" ) || ( arg == "-Regressionimagedir" ) || ( arg == "--regressionimagedir" ) || ( arg == "-regressionimagedir" ) || ( arg == "-R" ) ||  ( arg == "--R" ) )
    {
      if (cnt+1 < argc)
      {
        sci_putenv("SCI_REGRESSION_IMAGE_DIR",argv[cnt+1]);
        cnt++;
      }
    }
    else if ( ( arg == "--datadir" ) || ( arg == "-datadir" ) || ( arg == "-d" ) ||  ( arg == "--d" ) )
    {
      if (cnt+1 < argc)
      {
        sci_putenv("SCIRUN_DATA",argv[cnt+1]);
        cnt++;
      }
    }
    else if ( arg == "--nosplash" )
    {
      sci_putenv("SCIRUN_NOSPLASH", "1");
    }
    else if ( (arg == "--logfile") || (arg == "--log") || (arg == "-l") || (arg == "--l"))
    {
      if (cnt+1 < argc)
      {
        sci_putenv("SCIRUN_LOGFILE",argv[cnt+1]);
        cnt++;
      }
    }
    else if (ends_with(string_tolower(arg),".srn") || ends_with(string_tolower(arg),".srn#"))
    {
      if (!validFile(arg))
      {
        std::cerr << "Couldn't find net file " << arg
            << ".\nNo such file or directory.  Exiting." 
            << std::endl;
        exit(0);
      }
      sci_putenv("SCIRUN_NETFILE", arg);
    }
    else if ( ( arg == "--server" ) || ( arg == "-server" ) || ( arg == "-s" ) ||  ( arg == "--s" ) )
    {
      int port;
      if ((cnt+1 < argc) && string_to_int(argv[cnt+1], port)) 
      {
        if (port < 1024 || port > 65535) 
        {
          cerr << "Server port must be in range 1024-65535\n";
          exit(0);
        }
        cnt++;
      } 
      else 
      {
        port = 0;
      }
      sci_putenv("SCIRUN_SERVER_PORT",to_string(port));
    }    
    else if ( ( arg == "--port" ) || ( arg == "-port" ) || ( arg == "-p" ) ||  ( arg == "--p" ) )
    {
      int port;
      if ((cnt+1 < argc) && string_to_int(argv[cnt+1], port)) 
      {
        if (port < 1024 || port > 65535) 
        {
          cerr << "Server port must be in range 1024-65535\n";
          exit(0);
        }
        cnt++;
      } 
      else 
      {
        port = 0;
      }
      sci_putenv("SCIRUN_SERVICE_PORT",to_string(port));
      sci_putenv("SCIRUN_EXTERNAL_APPLICATION_INTERFACE","1");
    }
    else if (arg[0] == '+')
    {
      std::string key = arg.substr(1);
      std::string value;
      std::string::size_type loc = arg.find("=");
      if (loc == std::string::npos)
      { 
        key = arg.substr(1,loc);
        value = arg.substr(loc+1);
      }
      sci_putenv(key,value);
    }
    else
    {
      if (!validFile(arg))
      {
        std::cerr << "Couldn't find net file " << arg
            << ".\nNo such file or directory.  Exiting." 
            << std::endl;
        exit(0);
      }
      string filename(string_tolower(arg));
      if (!ends_with(filename,".net") && !ends_with(filename,".app") &&
        !ends_with(filename,".srn#"))
      {
        std::cerr << "Valid net files end with .srn, .app, " 
                        << "(or .net prior to v1.25.2) exiting." << std::endl;
        exit(0);
      }

      if (found && !powerapp)
      {
        usage();
      }

      // determine if it is a PowerApp
      if (ends_with(arg,".app")) 
      {
        powerapp = true;
        found = cnt;
      } 
      else if(!powerapp) 
      {
        found = cnt;
      }
    }
    cnt++;
  }
  return found;
}


class RegressionKiller : public Runnable
{
public:
  void run()
  {
    int tmp, seconds = 300;
    const char *timeout = sci_getenv("SCIRUN_REGRESSION_TESTING_TIMEOUT");
    if (timeout && string_to_int(timeout, tmp)) {
      seconds = tmp;
    }
    Time::waitFor((double)seconds);
    cout << "\n";
    cout << "main.cc: RegressionKiller: Regression test timed out\n";
    cout << "         after " << seconds << " seconds.  Killing SCIRun.\n\n";
    cout << "ERROR: KILL REQUIRED TO CONTINUE.";
    cout.flush();
    cerr.flush();
    Thread::exitAll(1);
  }
};


// Services start up... 
void
start_eai() {
  // Create a database of all available services. The next piece of code
  // Scans both the SCIRun as well as the Packages directories to find
  // Services that need to be started. Services allow communication with
  // thirdparty software and are Threads that run asychronicly with
  // with the rest of SCIRun. Since the thirdparty software may be running
  // on a different platform it allows for connecting to remote machines
  // and running the service on a different machine 
  ServiceDBHandle servicedb = scinew ServiceDB;     
  // load all services and find all makers
  servicedb->loadpackages();
  // activate all services
  servicedb->activateall();
  
  // Services are started and created by the ServiceManager, 
  // which will be launched here
  // Two competing managers will be started, 
  // one for purely internal usage and one that
  // communicates over a socket. 
  // The latter will only be created if a port is set.
  // If the current instance of SCIRun should not provide any services 
  // to other instances of SCIRun over the internet, 
  // the second manager will not be launched
  

  
  IComAddress internaladdress("internal","servicemanager");

// Only build log file if needed for debugging  
#ifdef DEBUG  
  const char *chome = sci_getenv("HOME");
  string scidir("");
  if (chome)
    scidir = chome+string("/SCIRun/");

  // A log file is not necessary but handy for debugging purposes
  ServiceLogHandle internallogfile = 
    scinew ServiceLog(scidir+"scirun_internal_servicemanager.log");
  
  ServiceManager* internal_service_manager = 
    scinew ServiceManager(servicedb, internaladdress, internallogfile); 
#else
  ServiceManager* internal_service_manager = 
    scinew ServiceManager(servicedb, internaladdress); 
#endif

  Thread* t_int = 
    scinew Thread(internal_service_manager, "internal service manager",
		  0, Thread::NotActivated);
  t_int->setStackSize(1024*20);
  t_int->activate(false);
  t_int->detach();
  
  
  // Use the following environment setting to switch on IPv6 support
  // Most machines should be running a dual-host stack for the internet
  // connections, so it should not hurt to run in IPv6 mode. In most case
  // ipv4 address will work as well.
  // It might be useful
  std::string ipstr(sci_getenv_p("SCIRUN_SERVICE_IPV6")?"ipv6":"");
  
  // Start an external service as well
  const char *serviceport_str = sci_getenv("SCIRUN_SERVICE_PORT");
  // If its not set in the env, we're done
  if (!serviceport_str) return;
  
  // The protocol for conencting has been called "scirun"
  // In the near future this should be replaced with "sciruns" for
  // a secure version which will run over ssl. 
  
  // A log file is not necessary but handy for debugging purposes

  IComAddress externaladdress("scirun","",serviceport_str,ipstr);

#ifdef DEBUG
  ServiceLogHandle externallogfile = 
    scinew ServiceLog(scidir+"scirun_external_servicemanager.log"); 
  
  ServiceManager* external_service_manager = 
    scinew ServiceManager(servicedb,externaladdress,externallogfile); 
#else
  ServiceManager* external_service_manager = 
    scinew ServiceManager(servicedb,externaladdress); 

#endif

  Thread* t_ext = 
    scinew Thread(external_service_manager,"external service manager",
		  0, Thread::NotActivated);
  t_ext->setStackSize(1024*20);
  t_ext->activate(false);
  t_ext->detach();
}  




int
main(int argc, char *argv[], char **environment) 
{
  // TAU_PROFILE("main", "", TAU_DEFAULT);
  // TAU_PROFILE_SET_NODE(0);

  // Setup the SCIRun key/value environment
  create_sci_environment(environment, 0);
  sci_putenv("SCIRUN_VERSION", SCIRUN_VERSION);
  sci_putenv("SCIRUN_RCFILE_SUBVERSION", SCIRUN_RCFILE_SUBVERSION);

  // Parse the command line arguments to find a network to execute
  const int startnetno = parse_args( argc, argv );

  SCIRunInit();
  

#ifndef _WIN32
  // Substitution often fails when the directory does not have a separator at the
  // end. Adding it here if it is not present.
  if (sci_getenv_p("SCIRUN_DATA"))
  {
    std::string datadir =  sci_getenv("SCIRUN_DATA");
    if (datadir[datadir.size()-1] != '/') datadir += "/";
    sci_putenv("SCIRUN_DATA",datadir);
  }
#endif

#ifndef _WIN32
  // Now split off a process for running external processes (not on windows)
  systemcallmanager_ = scinew SystemCallManager();
  systemcallmanager_->create();
  start_eai();
#endif

  //! create the EventManager thread.
  Thread *emt = scinew Thread(new EventManager(), "Event Manager Thread");
  emt->detach();

  Network* net=new Network();

  // Activate the scheduler.  Arguments and return values are meaningless
  Thread* scheduler=new Thread(new Scheduler(net), "Scheduler");
  scheduler->setDaemon(true);
  scheduler->detach();

  // Start the TCL thread, takes care of loading packages and networks
  Thread* tcl=new Thread(new TCLThread(argc, argv, net, startnetno),
                         "TCL main event loop",0,Thread::Activated,1024*1024);
  tcl->detach();
        
  // When doing regressions, make thread to kill ourselves after timeout
  if (sci_getenv_p("SCI_REGRESSION_TESTING")) 
  {
    sci_putenv("SCIRUN_GUI_UseGuiFetch","off");
    sci_putenv("SCIRUN_GUI_MoveGuiToMouse","off");
    RegressionKiller *kill = scinew RegressionKiller();
    Thread *tkill = scinew Thread(kill, "Kill a hung SCIRun");
    tkill->detach();
  }


#ifdef _WIN32
  // windows has a semantic problem with atexit(), so we wait here instead.
  HANDLE forever = CreateSemaphore(0,0,1,"forever");
  WaitForSingleObject(forever,INFINITE);
#endif

#if !defined(__sgi)
  Semaphore wait("main wait", 0);
  wait.down();
#endif
        
  return 0;
}

#if defined(__sgi) && !defined(__GNUC__) && (_MIPS_SIM != _MIPS_SIM_ABI32)
#pragma reset woff 1209
#pragma reset woff 1424
#endif

