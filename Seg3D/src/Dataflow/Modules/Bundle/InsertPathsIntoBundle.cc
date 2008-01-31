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
#include <Core/Geom/Path.h>
#include <Dataflow/Network/Ports/PathPort.h>
#include <Dataflow/Network/Module.h>

using namespace SCIRun;

class InsertPathsIntoBundle : public Module {
public:
  InsertPathsIntoBundle(GuiContext*);
  virtual void execute();
  
private:
  GuiString     guipath1name_;
  GuiString     guipath2name_;
  GuiString     guipath3name_;
  GuiString     guipath4name_;
  GuiString     guipath5name_;
  GuiString     guipath6name_;
  
  GuiInt        guireplace1_;
  GuiInt        guireplace2_;
  GuiInt        guireplace3_;
  GuiInt        guireplace4_;
  GuiInt        guireplace5_;
  GuiInt        guireplace6_;
  
  GuiString     guibundlename_;
};


DECLARE_MAKER(InsertPathsIntoBundle)
  InsertPathsIntoBundle::InsertPathsIntoBundle(GuiContext* ctx)
    : Module("InsertPathsIntoBundle", ctx, Filter, "Bundle", "SCIRun"),
      guipath1name_(get_ctx()->subVar("path1-name"), "path1"),
      guipath2name_(get_ctx()->subVar("path2-name"), "path2"),
      guipath3name_(get_ctx()->subVar("path3-name"), "path3"),
      guipath4name_(get_ctx()->subVar("path4-name"), "path4"),
      guipath5name_(get_ctx()->subVar("path5-name"), "path5"),
      guipath6name_(get_ctx()->subVar("path6-name"), "path6"),
      guireplace1_(get_ctx()->subVar("replace1"),1),
      guireplace2_(get_ctx()->subVar("replace2"),1),
      guireplace3_(get_ctx()->subVar("replace3"),1),
      guireplace4_(get_ctx()->subVar("replace4"),1),
      guireplace5_(get_ctx()->subVar("replace5"),1),
      guireplace6_(get_ctx()->subVar("replace6"),1),
      guibundlename_(get_ctx()->subVar("bundlename"), "")
{
}

void InsertPathsIntoBundle::execute()
{
  BundleHandle  handle;
  PathHandle path1, path2, path3;
  PathHandle path4, path5, path6;

  get_input_handle("bundle",handle,false);
  get_input_handle("path1",path1,false);
  get_input_handle("path2",path2,false);
  get_input_handle("path3",path3,false);
  get_input_handle("path4",path4,false);
  get_input_handle("path5",path5,false);
  get_input_handle("path6",path6,false);
  
  if (inputs_changed_ || guipath1name_.changed() || 
      guipath2name_.changed() || guipath3name_.changed() || 
      guipath4name_.changed() || guipath5name_.changed() || 
      guipath6name_.changed() ||
      guireplace1_.changed() || guireplace2_.changed() ||
      guireplace3_.changed() || guireplace4_.changed() ||
      guireplace5_.changed() || guireplace6_.changed() ||
      guibundlename_.changed() || !oport_cached("bundle"))
  {
  
    std::string path1Name = guipath1name_.get();
    std::string path2Name = guipath2name_.get();
    std::string path3Name = guipath3name_.get();
    std::string path4Name = guipath4name_.get();
    std::string path5Name = guipath5name_.get();
    std::string path6Name = guipath6name_.get();
    std::string bundlename = guibundlename_.get();

    if (handle.get_rep())
    {
      handle.detach();
    }
    else
    {
      handle = scinew Bundle();
      if (handle.get_rep() == 0)
      {
        error("Could not allocate new bundle");
        return;
      }
    }

    if (path1.get_rep()
        &&(guireplace1_.get()||!(handle->isPath(path1Name)))) 
      handle->setPath(path1Name,path1);

    if (path2.get_rep()
        &&(guireplace2_.get()||!(handle->isPath(path2Name)))) 
      handle->setPath(path2Name,path2);

    if (path3.get_rep()
        &&(guireplace3_.get()||!(handle->isPath(path3Name)))) 
      handle->setPath(path3Name,path3);

    if (path4.get_rep()
        &&(guireplace4_.get()||!(handle->isPath(path4Name)))) 
      handle->setPath(path4Name,path4);

    if (path5.get_rep()
        &&(guireplace5_.get()||!(handle->isPath(path5Name)))) 
      handle->setPath(path5Name,path5);

    if (path6.get_rep()
        &&(guireplace6_.get()||!(handle->isPath(path6Name)))) 
      handle->setPath(path6Name,path6);
                                      
    if (bundlename != "")
    {
      handle->set_property("name",bundlename,false);
    }

    send_output_handle("bundle",handle);
  }
}


