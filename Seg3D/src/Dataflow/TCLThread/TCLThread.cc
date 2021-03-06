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

#include <Dataflow/TCLThread/TCLThread.h>
#include <Core/Containers/StringUtil.h>
#include <Dataflow/GuiInterface/TCLInterface.h>
#include <Dataflow/GuiInterface/TCLTask.h>
#include <slivr/ShaderProgramARB.h>
#include <Core/Util/Environment.h>
#include <Core/Util/sci_system.h>
#include <Dataflow/Network/NetworkEditor.h>
#include <Dataflow/Network/PackageDB.h>
#include <Dataflow/Network/NetworkIO.h>
#include <main/sci_version.h>


#ifdef _WIN32
#  include <Core/Geom/Win32OpenGLContext.h>
#else
#  include <Core/Geom/X11OpenGLContext.h>
#endif

#include <tcl.h>
#include <tk.h>
#include <itk.h>

// find a more 'consolidated' place to put this...
#if (TCL_MINOR_VERSION >= 4)
#  define TCLCONST const
#else
#  define TCLCONST
#endif

typedef void (Tcl_LockProc)();

using namespace SCIRun;

#ifdef _WIN32
#  ifndef BUILD_SCIRUN_STATIC
#    define SCISHARE __declspec(dllimport)
#  else
#    define SCISHARE
#  endif
#else
#  define SCISHARE
#endif // _WIN32

extern "C" SCISHARE Tcl_Interp* the_interp;

#ifndef EXPERIMENTAL_TCL_THREAD
  extern "C" SCISHARE void Tcl_SetLock(Tcl_LockProc*, Tcl_LockProc*);
#endif

/*
 * The following declarations refer to internal Tk routines.  These
 * interfaces are available for use, but are not supported.
 */

//EXTERN void		TkConsoleCreate(void);
//EXTERN int		TkConsoleInit(Tcl_Interp *interp);
//extern "C" int Table_Init _ANSI_ARGS_((Tcl_Interp* interp));

extern "C" SCISHARE int OpenGLCmd _ANSI_ARGS_((ClientData clientData,
                                      Tcl_Interp *interp, 
                                      int argc, TCLCONST char **argv));
extern "C" SCISHARE int BevelCmd _ANSI_ARGS_((ClientData clientData,
                                              Tcl_Interp *interp, 
                                              int argc, TCLCONST char **argv));
extern "C" SCISHARE int Tk_RangeObjCmd _ANSI_ARGS_((ClientData clientData, 
                                                    Tcl_Interp *interp, 
                                                    int objc, Tcl_Obj *CONST objv[])); 
extern "C" SCISHARE int Tk_CursorCmd _ANSI_ARGS_((ClientData clientData,
                                                  Tcl_Interp *interp, 
                                                  int argc, TCLCONST char **argv));
extern "C" SCISHARE int BLineInit _ANSI_ARGS_((void));
extern "C" int Blt_SafeInit _ANSI_ARGS_((Tcl_Interp *interp));
extern "C" int Blt_Init _ANSI_ARGS_((Tcl_Interp *interp));

using namespace std;

static
int
x_error_handler(Display* dpy, XErrorEvent* error)
{
#ifndef _WIN32
    char msg[200];
    XGetErrorText(dpy, error->error_code, msg, 200);
    cerr << "X Error: " << msg << endl;
    abort();
#endif
    return 0; // Never reached...
}

TCLThread::TCLThread(int argc, char* argv[], Network* net, int startnetno) :
  argc(argc), argv(argv), net(net), startnetno(startnetno)
{
  // Setup the error handler to catch errors...
  // The default one exits, and makes it very hard to 
  // track down errors.  We need core dumps!
  XSetErrorHandler(x_error_handler);
#ifndef EXPERIMENTAL_TCL_THREAD
  Tcl_SetLock(TCLTask::lock, TCLTask::unlock);
#endif
}


SCIRun::TCLThread *SCIRun::TCLThread::init_ptr_ = 0;
int
TCLThread::appInit(Tcl_Interp *interp)
{
  the_interp = interp;
  ASSERT(init_ptr_);
  return init_ptr_->startNetworkEditor();
}


void
TCLThread::run()
{
#ifdef EXPERIMENTAL_TCL_THREAD
  //! Mark that we are inside the thread running TCL
  //! This will disable the GUI locks() called on this thread
  //! Instead the TCLTask::lock will be treated as a counter to check
  //! availability to execute jobs from other threads
  TCLTask::mark_as_tcl_thread();
#endif

  //! Lock TCL, in case of none experimental thread this blocks threads that 
  //! are trying to send jobs to TCL, in case of the experimental thread, it
  //! will block these jobs when it is receiving them from the mailbox
  TCLTask::lock();
  init_ptr_ = this;

  if (sci_getenv_p("SCIRUN_NOGUI"))
    Tcl_Main(1, argv, TCLThread::appInit);
  else
    Tk_Main(1, argv, TCLThread::appInit);
}


bool
TCLThread::check_for_newer_scirunrc() {
  if (sci_getenv("SCI_REGRESSION_TESTING"))
    return false; // don't do check when regression testing

  const char *rcversion = sci_getenv("SCIRUN_RCFILE_VERSION");
  const string curversion = (rcversion ? rcversion : "");
  const string newversion = 
    string(SCIRUN_VERSION)+"."+string(SCIRUN_RCFILE_SUBVERSION);
  
  // If the .scirunrc is an old version
  if (curversion < newversion)
    // Ask them if they want to copy over a new one
    return gui->eval("promptUserToCopySCIRunrc") == "1";
  return false; // current version is equal to newest version
}
  
#ifdef EXPERIMENTAL_TCL_THREAD
SCISHARE void eventCheck(ClientData cd, int flags);
SCISHARE void eventSetup(ClientData cd, int flags);
#endif

static
int
exitproc(ClientData, Tcl_Interp*, int, TCLCONST char* [])
{
  Thread::exitAll(0);
  return TCL_OK; // not reached
}


int
TCLThread::startNetworkEditor()
{
  //! Check whether we have to run without a GUI
  bool scirun_nogui = false;
  if (getenv("SCIRUN_NOGUI"))
  {
    scirun_nogui = true;
  }

  std::cout << "Loading Tcl,"; std::cout.flush();
  
  if (Tcl_Init(the_interp) == TCL_ERROR) 
  {
    std::cerr << "\nTcl_Init() failed with TCL_ERROR="<< TCL_ERROR <<"\n";
    return TCL_ERROR;
  }

  std::cout << "Tk,"; std::cout.flush();
  if (!scirun_nogui)
  {
    printf("tk, ");
    if (Tk_Init(the_interp) == TCL_ERROR) {
      printf("Tk_Init() failed.  Is the DISPLAY environment variable set properly?\n");

      Thread::exitAll(-1);//exit_all_threads(TCL_ERROR);
    }
    Tcl_StaticPackage(the_interp, "Tk", Tk_Init, Tk_SafeInit);
  }

  //! Load the ITCL Package
  std::cout << "Itcl,"; std::cout.flush();
  if (Itcl_Init(the_interp) == TCL_ERROR) 
  {
    std::cerr << "\nItcl_Init() failed with TCL_ERROR="<< TCL_ERROR <<"\n";
    return TCL_ERROR;
  }  
  
  if (!scirun_nogui)
  {
    std::cout << "Itk,"; std::cout.flush();
    if (Itk_Init(the_interp) == TCL_ERROR) 
    {
      std::cerr << "\nItk_Init() failed with TCL_ERROR="<< TCL_ERROR <<"\n";
      return TCL_ERROR;
    }

    std::cout << "Blt,"; std::cout.flush();
    if (Blt_Init(the_interp) == TCL_ERROR) 
    {
      std::cerr << "\nBlt_Init() failed with TCL_ERROR="<< TCL_ERROR <<"\n";
      return TCL_ERROR;
    }
  }
  
  Tcl_StaticPackage(the_interp, "Itcl", Itcl_Init, Itcl_SafeInit);
  if (!scirun_nogui)
  {
    Tcl_StaticPackage(the_interp, "Itk", Itk_Init, (Tcl_PackageInitProc *) NULL);
    Tcl_StaticPackage(the_interp, "BLT", Blt_Init, Blt_SafeInit);
  }

  /*
   *  This is itkwish, so import all [incr Tcl] commands by
   *  default into the global namespace.  Fix up the autoloader
   *  to do the same.
   */
  if (!scirun_nogui)
  {
    if (Tcl_Import(the_interp, Tcl_GetGlobalNamespace(the_interp),
                   "::itk::*", /* allowOverwrite */ 1) != TCL_OK) {
      return TCL_ERROR;
    }
  }

  if (Tcl_Import(the_interp, Tcl_GetGlobalNamespace(the_interp),
		 "::itcl::*", /* allowOverwrite */ 1) != TCL_OK) {
    return TCL_ERROR;
  }

  if (!scirun_nogui)
  {
    if (Tcl_Eval(the_interp, "auto_mkindex_parser::slavehook { _%@namespace import -force ::itcl::* ::itk::* }") != TCL_OK) {
      return TCL_ERROR;
    }
  }
  else
  {
    if (Tcl_Eval(the_interp, "auto_mkindex_parser::slavehook { _%@namespace import -force ::itcl::* }") != TCL_OK) {
      return TCL_ERROR;
    }
  }
  

  /*
   * Call Tcl_CreateCommand for application-specific commands, if
   * they weren't already created by the init procedures called above.
   */

  /*
   * Call Tcl_CreateCommand for application-specific commands, if
   * they weren't already created by the init procedures called above.
   */

#ifdef _WIN32
#  define PARAMETERTYPE int*
#else
#  define PARAMETERTYPE void
#endif

  if (!scirun_nogui)
  {

    std::cout << "Widgets"; std::cout.flush();
    Tcl_CreateCommand(the_interp, "opengl", OpenGLCmd, 
                      (ClientData) Tk_MainWindow(the_interp), 0);

    Tcl_CreateCommand(the_interp, "bevel", BevelCmd,
                      (ClientData) Tk_MainWindow(the_interp), 0);

    Tcl_CreateObjCommand(the_interp, "range", (Tcl_ObjCmdProc *)Tk_RangeObjCmd,
                         (ClientData) NULL, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateCommand(the_interp, "cursor", Tk_CursorCmd,
                      (ClientData) Tk_MainWindow(the_interp), NULL);

    /*
     * Initialize the BLine Canvas item
     */

    BLineInit();
  }

  std::cout << "\n";

  /*
   * Specify a user-specific startup file to invoke if the application
   * is run interactively.  Typically the startup file is "~/.apprc"
   * where "app" is the name of the application.  If this line is deleted
   * then no user-specific startup file will be run under any conditions.
   */

  gui = scinew TCLInterface;
  
  // We parse the scirunrc file here before creating the network
  // editor.  Note that this may fail, but we need the network editor
  // up before we can prompt the user what to do.  This means that the
  // environment variables used by the network editor are assumed to
  // be in the same state as the default ones in the srcdir/scirunrc
  // file.  For now only SCIRUN_NOGUI is affected.
  //  const bool scirunrc_parsed = find_and_parse_scirunrc();

  // Create the network editor here.  For now we just dangle it and
  // let exitAll destroy it with everything else at the end.
  if (scirun_nogui)
  {
    gui->eval(string("rename unknown _old_unknown"));
    gui->eval(string("proc unknown args {\n") +
              string("    catch \"[uplevel 1 _old_unknown $args]\" result\n") +
              string("    return 0\n") +
              //string("    return $result\n") +
              string("}"));
  }
  
  NetworkEditor* editor = new NetworkEditor(net, gui);

#if (TCL_MINOR_VERSION >= 4)
  // tcl 8.4 deprecates some private internal commands, 
  // but allows us to alias them
  // FieldReader
  gui->eval("::tk::unsupported::ExposePrivateCommand tkFDGetFileTypes");
  gui->eval("::tk::unsupported::ExposePrivateCommand tkFocusGroup_Create");
  gui->eval("::tk::unsupported::ExposePrivateCommand tkFocusGroup_BindIn");
  gui->eval("::tk::unsupported::ExposePrivateCommand tkFocusGroup_BindOut");
  gui->eval("::tk::unsupported::ExposePrivateCommand tkButtonInvoke");
  gui->eval("::tk::unsupported::ExposePrivateCommand tkCancelRepeat");
#endif

  // If the user doesnt have a .scirunrc file, or it is out of date,
  // provide them with a default one
  if (!sci_getenv_p("SCIRUN_RC_PARSED") || check_for_newer_scirunrc()) {
    copy_and_parse_scirunrc();
  }

  // Determine if we are loading an app.
  const bool powerapp_p = (startnetno && ends_with(argv[startnetno],".app"));
  if (!powerapp_p)
  {
    gui->eval("set PowerApp 0");
    // Wait for the main window to display before continuing the startup.
    gui->eval("wm deiconify .");
    gui->eval("tkwait visibility $minicanvas");
    gui->eval("showProgress 1 0 1");
  }
  else
  { // If loading an app, don't wait.
    gui->eval("set PowerApp 1");
    if (argv[startnetno+1])
    {
      gui->eval("set PowerAppSession {"+string(argv[startnetno+1])+"}");
    }
    // Determine which standalone and set splash.
    if(strstr(argv[startnetno], "BioTensor"))
    {
      gui->eval("set splashImageFile $bioTensorSplashImageFile");
      gui->eval("showProgress 1 3221 1");
    }
    else if(strstr(argv[startnetno], "BioFEM"))
    {
      gui->eval("set splashImageFile $bioFEMSplashImageFile");
      gui->eval("showProgress 1 426 1");
    }
    else if(strstr(argv[startnetno], "BioImage"))
    {
      // Need to make a BioImage splash screen.
      gui->eval("set splashImageFile $bioImageSplashImageFile");
      gui->eval("showProgress 1 759 1");
    }
    else if(strstr(argv[startnetno], "FusionViewer"))
    {
      // Need to make a FusionViewer splash screen.
      gui->eval("set splashImageFile $fusionViewerSplashImageFile");
      gui->eval("showProgress 1 310 1");
    }
  }

  packageDB = new PackageDB(gui);
  // load the packages
  packageDB->loadPackage(!sci_getenv_p("SCIRUN_LOAD_MODULES_ON_STARTUP"));  

  if (!powerapp_p)
  {
    gui->eval("hideProgress");
  }
  
  // Check the dynamic compilation directory for validity
  sci_putenv("SCIRUN_ON_THE_FLY_LIBS_DIR",gui->eval("getOnTheFlyLibsDir"));

  // Activate "File" menu sub-menus once packages are all loaded.
  gui->eval("activate_file_submenus");

  // Test for shaders.
#if defined(HAVE_X11)
  X11OpenGLContext *context = new X11OpenGLContext(0, 0, 0, 10, 10, 0, false);
  delete context;
#elif defined(_WIN32)
  Win32OpenGLContext *context = new Win32OpenGLContext(0, 0, 0, 10, 10, false, false);
  delete context;
#endif
  Tcl_CreateCommand(the_interp, "exit", exitproc, 0, 0);

#ifdef EXPERIMENTAL_TCL_THREAD

  //! Forward this one to the TCLTask as well so it can signal when internal
  //! evaluations are finished
  TCLTask::set_tcl_thread_id(Tcl_GetCurrentThread());

  // windows doesn't communicate TCL with threads like other OSes do.
  // do instead of direct TCL communication, setup tcl callbacks
  Tcl_CreateEventSource(eventSetup, eventCheck, 0);

  //! Unlock the GUI so we can start processing callbacks from other threads
  TCLTask::unlock();
#endif
  SLIVR::ShaderProgramARB::init_shaders_supported();
  bool loaded_network = false;
  // load network from an xml file...
 
  NetworkIO* netio = editor->get_networkio();
  
  if(sci_getenv("SCIRUN_NETFILE") != 0)
  {
    std::cout<< "loading scirun network file: "<<sci_getenv("SCIRUN_NETFILE")<<std::endl;
    netio->load_net(std::string(sci_getenv("SCIRUN_NETFILE")));
    loaded_network = true;
  }
  // Load the Network file specified from the command line
  string fname; 

  if (startnetno)
  {
    fname = string(argv[startnetno]);
    std::cout<< "loading scirun network file: "<<fname<<std::endl;
    gui->eval("loadnet {"+fname+"}");
    loaded_network = true;
  }

  if (loaded_network && (sci_getenv_p("SCIRUN_CONVERT_NET_TO_SRN"))) 
  {
    size_t pos = fname.find(".net");
    fname.replace(pos, 4, ".srn");
    gui->eval("set netedit_savefile " + fname);
    gui->eval("popupSaveMenu");
    gui->eval("netedit quit");
  }

  if (loaded_network &&
      (sci_getenv_p("SCIRUN_EXECUTE_ON_STARTUP") || 
       sci_getenv_p("SCI_REGRESSION_TESTING"))) {
    gui->eval("netedit scheduleall");
  }

  return TCL_OK;
}


