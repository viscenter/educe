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
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   July 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

/*
 * Limitations:
 *   Uses .tcl file with "filename"
 *   Output port must be of type SimpleOPort
 */


#include <Dataflow/Network/Ports/StringPort.h>
#include <Core/Datatypes/String.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Core/Malloc/Allocator.h>
#include <Dataflow/Network/Module.h>
#include <sys/stat.h>

namespace SCIRun {

template <class HType> 
class GenericReader : public Module
{
public:
  GenericReader(const string &name, GuiContext* ctx,
		const string &category, const string &package);
  virtual ~GenericReader();

  virtual void execute();

protected:
  GuiFilename gui_filename_;
  GuiString gui_from_env_;

  time_t old_filemodification_;

  bool importing_;

  virtual bool call_importer(const string &filename, HType & handle);
};


template <class HType>
GenericReader<HType>::GenericReader(const string &name, GuiContext* ctx,
				    const string &cat, const string &pack)
  : Module(name, ctx, Source, cat, pack),
    gui_filename_(get_ctx()->subVar("filename"), ""),
    gui_from_env_(get_ctx()->subVar("from-env"),""),
    old_filemodification_(0),
    importing_(false)
{
}

template <class HType>
GenericReader<HType>::~GenericReader()
{
}


template <class HType>
bool
GenericReader<HType>::call_importer(const string &/*filename*/,
				    HType & handle )
{
  return false;
}


template <class HType>
void
GenericReader<HType>::execute()
{

  bool filename_changed = gui_filename_.changed();
  
  if (gui_from_env_.get() != "")
  {
    string filename_from_env = gui_from_env_.get(); 
    if (sci_getenv(filename_from_env))
    {
      string envfilename = sci_getenv(filename_from_env);
      gui_filename_.set(envfilename);
      get_ctx()->reset();
      filename_changed = true;
    }
  }

  // If there is an optional input string set the filename to it in the GUI.
  StringHandle shandle;
  if (get_input_handle("Filename",shandle,false))
  {
    gui_filename_.set(shandle->get());
    get_ctx()->reset();
    filename_changed = true;
  }
  
  const string filename(gui_filename_.get());

  // Read the status of this file so we can compare modification timestamps
  struct stat buf;

  if( filename == "" ) 
  {
    error("No file has been selected.  Please choose a file.");
    return;
  } 
  else if (stat(filename.c_str(), &buf)) 
  {
    if (!importing_)
    {
      error("File '" + filename + "' not found.");
      return;
    }
    else
    {
      warning("File '" + filename + "' not found.  Maybe the plugin can find it?");
    }
  }

  // If we haven't read yet, or if it's a new filename, 
  //  or if the datestamp has changed -- then read...
#ifdef __sgi
  time_t new_filemodification = buf.st_mtim.tv_sec;
#else
  time_t new_filemodification = buf.st_mtime;
#endif

  if( inputs_changed_ || filename_changed ||
      new_filemodification != old_filemodification_ ||
      !oport_cached(0) ||
      !oport_cached("Filename") )
  {
  
    old_filemodification_ = new_filemodification;

    HType handle;

    remark("loading file " +filename);
    
    if (importing_)
    {
      if (!call_importer(filename, handle))
      {
        error("Import failed.");
        return;
      }
    }
    else
    {
      Piostream *stream = auto_istream(filename, this);
      if (!stream)
      {
        error("Error reading file '" + filename + "'.");
        return;
      }
    
      // Read the file
      Pio(*stream, handle);

      if (!handle.get_rep() || stream->error())
      {
        error("Error reading data from file '" + filename +"'.");
        delete stream;
        return;
      }

      delete stream;
    }

    send_output_handle(0, handle);

    shandle = (StringHandle) scinew String(gui_filename_.get());
    send_output_handle("Filename", shandle);
  }

}

} // End namespace SCIRun
