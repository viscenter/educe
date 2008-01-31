
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

#include <Core/Bundle/Bundle.h>
#include <Dataflow/Network/Ports/BundlePort.h>
#include <Dataflow/Network/Ports/PathPort.h>
#include <Core/Geom/Path.h>
#include <Dataflow/Network/Module.h>

using namespace SCIRun;

class GetPathsFromBundle : public Module {
public:
  GetPathsFromBundle(GuiContext*);
  virtual void execute();
  
private:
  GuiString             guipath1name_;
  GuiString             guipath2name_;
  GuiString             guipath3name_;
  GuiString             guipath4name_;
  GuiString             guipath5name_;
  GuiString             guipath6name_;
  GuiString             guipaths_;
};


DECLARE_MAKER(GetPathsFromBundle)
  GetPathsFromBundle::GetPathsFromBundle(GuiContext* ctx)
    : Module("GetPathsFromBundle", ctx, Filter, "Bundle", "SCIRun"),
      guipath1name_(get_ctx()->subVar("path1-name"), "path1"),
      guipath2name_(get_ctx()->subVar("path2-name"), "path2"),
      guipath3name_(get_ctx()->subVar("path3-name"), "path3"),
      guipath4name_(get_ctx()->subVar("path4-name"), "path4"),
      guipath5name_(get_ctx()->subVar("path5-name"), "path5"),
      guipath6name_(get_ctx()->subVar("path6-name"), "path6"),
      guipaths_(get_ctx()->subVar("path-selection"), "")
{
}

void GetPathsFromBundle::execute()
{
  // Define input handle:
  BundleHandle handle;
  
  // Get data from input port:
  if (!(get_input_handle("bundle",handle,true))) return;
  
  if (inputs_changed_ || guipath1name_.changed() || 
      guipath2name_.changed() || guipath3name_.changed() ||
      guipath4name_.changed() || guipath5name_.changed() ||
      guipath6name_.changed() || !oport_cached("bundle") || 
      !oport_cached("path1") ||  !oport_cached("path2") || 
      !oport_cached("path3") ||  !oport_cached("path4") ||
      !oport_cached("path5") ||  !oport_cached("path6"))
  {
    PathHandle fhandle;
    std::string path1name = guipath1name_.get();
    std::string path2name = guipath2name_.get();
    std::string path3name = guipath3name_.get();
    std::string path4name = guipath4name_.get();
    std::string path5name = guipath5name_.get();
    std::string path6name = guipath6name_.get();
    std::string pathlist;
      
    int numPaths = handle->numPaths();
    for (int p = 0; p < numPaths; p++)
    {
      pathlist += "{" + handle->getPathName(p) + "} ";
    }

    guipaths_.set(pathlist);
    get_ctx()->reset();

    // Send path1 if we found one that matches the name:
    if (handle->isPath(path1name))
    {
      fhandle = handle->getPath(path1name);
      send_output_handle("path1",fhandle);
    } 

    // Send path2 if we found one that matches the name:
    if (handle->isPath(path2name))
    {
      fhandle = handle->getPath(path2name);
      send_output_handle("path2",fhandle);
    } 

    // Send path3 if we found one that matches the name:
    if (handle->isPath(path3name))
    {
      fhandle = handle->getPath(path3name);
      send_output_handle("path3",fhandle);
    } 

    // Send path1 if we found one that matches the name:
    if (handle->isPath(path4name))
    {
      fhandle = handle->getPath(path4name);
      send_output_handle("path4",fhandle);
    } 

    // Send path2 if we found one that matches the name:
    if (handle->isPath(path5name))
    {
      fhandle = handle->getPath(path5name);
      send_output_handle("path5",fhandle);
    } 

    // Send path3 if we found one that matches the name:
    if (handle->isPath(path6name))
    {
      fhandle = handle->getPath(path6name);
      send_output_handle("path6",fhandle);
    }     
    
    send_output_handle("bundle",handle);
  }
}

