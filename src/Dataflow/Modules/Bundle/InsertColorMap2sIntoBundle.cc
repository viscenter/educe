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
#include <Dataflow/Network/Ports/ColorMap2Port.h>
#include <Dataflow/Network/Module.h>

using namespace SCIRun;

class InsertColorMap2sIntoBundle : public Module {
public:
  InsertColorMap2sIntoBundle(GuiContext*);
  virtual void execute();

private:
  GuiString     guicolormap21name_;
  GuiString     guicolormap22name_;
  GuiString     guicolormap23name_;
  GuiString     guicolormap24name_;
  GuiString     guicolormap25name_;
  GuiString     guicolormap26name_;

  GuiInt        guireplace1_;
  GuiInt        guireplace2_;
  GuiInt        guireplace3_;
  GuiInt        guireplace4_;
  GuiInt        guireplace5_;
  GuiInt        guireplace6_;

  GuiString     guibundlename_;
};


DECLARE_MAKER(InsertColorMap2sIntoBundle)

InsertColorMap2sIntoBundle::InsertColorMap2sIntoBundle(GuiContext* ctx)
  : Module("InsertColorMap2sIntoBundle", ctx, Filter, "Bundle", "SCIRun"),
    guicolormap21name_(get_ctx()->subVar("colormap21-name"), "colormap21"),
    guicolormap22name_(get_ctx()->subVar("colormap22-name"), "colormap22"),
    guicolormap23name_(get_ctx()->subVar("colormap23-name"), "colormap23"),
    guicolormap24name_(get_ctx()->subVar("colormap24-name"), "colormap24"),
    guicolormap25name_(get_ctx()->subVar("colormap25-name"), "colormap25"),
    guicolormap26name_(get_ctx()->subVar("colormap26-name"), "colormap26"),
    guireplace1_(get_ctx()->subVar("replace1"),1),
    guireplace2_(get_ctx()->subVar("replace2"),1),
    guireplace3_(get_ctx()->subVar("replace3"),1),
    guireplace4_(get_ctx()->subVar("replace4"),1),
    guireplace5_(get_ctx()->subVar("replace5"),1),
    guireplace6_(get_ctx()->subVar("replace6"),1),
    guibundlename_(get_ctx()->subVar("bundlename"), "")
{
}

void InsertColorMap2sIntoBundle::execute()
{
  BundleHandle  handle;
  ColorMap2Handle colormap21, colormap22, colormap23;
  ColorMap2Handle colormap24, colormap25, colormap26;

  get_input_handle("bundle",handle,false);
  get_input_handle("colormap21",colormap21,false);
  get_input_handle("colormap22",colormap22,false);
  get_input_handle("colormap23",colormap23,false);
  get_input_handle("colormap24",colormap21,false);
  get_input_handle("colormap25",colormap22,false);
  get_input_handle("colormap26",colormap23,false);
  
  if (inputs_changed_ || guicolormap21name_.changed() || 
      guicolormap22name_.changed() || guicolormap23name_.changed() || 
      guicolormap24name_.changed() || guicolormap25name_.changed() || 
      guicolormap26name_.changed() || 
      guireplace1_.changed() || guireplace2_.changed() ||
      guireplace3_.changed() || guireplace4_.changed() ||
      guireplace5_.changed() || guireplace6_.changed() ||
      guibundlename_.changed() || !oport_cached("bundle"))
  {
  
    std::string colormap21Name = guicolormap21name_.get();
    std::string colormap22Name = guicolormap22name_.get();
    std::string colormap23Name = guicolormap23name_.get();
    std::string colormap24Name = guicolormap24name_.get();
    std::string colormap25Name = guicolormap25name_.get();
    std::string colormap26Name = guicolormap26name_.get();
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

    if (colormap21.get_rep()
        &&(guireplace1_.get()||!(handle->isColorMap2(colormap21Name)))) 
      handle->setColorMap2(colormap21Name,colormap21);
      
    if (colormap22.get_rep()
        &&(guireplace2_.get()||!(handle->isColorMap2(colormap22Name)))) 
      handle->setColorMap2(colormap22Name,colormap22);
      
    if (colormap23.get_rep()
        &&(guireplace3_.get()||!(handle->isColorMap2(colormap23Name)))) 
      handle->setColorMap2(colormap23Name,colormap23);
      
    if (colormap24.get_rep()
        &&(guireplace4_.get()||!(handle->isColorMap2(colormap24Name)))) 
      handle->setColorMap2(colormap24Name,colormap24);
      
    if (colormap25.get_rep()
        &&(guireplace5_.get()||!(handle->isColorMap2(colormap25Name)))) 
      handle->setColorMap2(colormap25Name,colormap25);
      
    if (colormap26.get_rep()
        &&(guireplace6_.get()||!(handle->isColorMap2(colormap26Name)))) 
      handle->setColorMap2(colormap26Name,colormap26);
      
    if (bundlename != "")
    {
      handle->set_property("name",bundlename,false);
    }

    send_output_handle("bundle", handle);
  }
}


