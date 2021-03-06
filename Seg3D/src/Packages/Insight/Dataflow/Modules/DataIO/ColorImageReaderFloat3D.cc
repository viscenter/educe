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
 *  ColorImageReaderFloat3D.cc:
 *
 *  Written by:
 *   darbyb
 *   TODAY'S DATE HERE
 *
 */

#include <Dataflow/Network/Module.h>
#include <Core/Malloc/Allocator.h>

#include <Dataflow/Network/Ports/ITKDatatypePort.h>
#include "itkImageFileReader.h"
#include "itkVector.h"

namespace Insight {

using namespace SCIRun;

class ColorImageReaderFloat3D : public Module {
public:

  //! GUI variables
  GuiString gui_FileName_;

  ITKDatatypeHandle handle_;

  string prevFile;
  
  ColorImageReaderFloat3D(GuiContext*);

  virtual ~ColorImageReaderFloat3D();

  virtual void execute();
};


DECLARE_MAKER(ColorImageReaderFloat3D)
ColorImageReaderFloat3D::ColorImageReaderFloat3D(GuiContext* ctx)
  : Module("ColorImageReaderFloat3D", ctx, Source, "DataIO", "Insight"),
    gui_FileName_(get_ctx()->subVar("FileName"))
{
  prevFile = "";
}


ColorImageReaderFloat3D::~ColorImageReaderFloat3D()
{
}


void
ColorImageReaderFloat3D::execute()
{
  typedef itk::ImageFileReader<itk::Image<itk::Vector<float>, 3> > FileReaderType;
  
  // create a new reader
  FileReaderType::Pointer reader = FileReaderType::New();
  
  // set reader
  string fn = gui_FileName_.get();
  reader->SetFileName( fn.c_str() );
  
  try {
    reader->Update();  
  } catch  ( itk::ExceptionObject & err ) {
     error("ExceptionObject caught!");
     error(err.GetDescription());
  }
  
  // get reader output
  if(!handle_.get_rep() || (fn != prevFile))
  {
    ITKDatatype *im = scinew ITKDatatype;
    im->data_ = reader->GetOutput();  
    handle_ = im; 
    prevFile = fn;
  }
  
  // Send the data downstream
  send_output_handle("Image", handle_, true);
}


} // End namespace Insight

