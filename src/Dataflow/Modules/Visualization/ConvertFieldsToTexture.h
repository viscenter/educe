//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  
//    File   : ConvertFieldsToTexture.cc
//    Author : Milan Ikits
//    Date   : Fri Jul 16 03:28:21 2004

#ifndef CONVERT_FIELDS_TO_TEXTURE_H
#define CONVERT_FIELDS_TO_TEXTURE_H

#include <Dataflow/GuiInterface/GuiVar.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/TexturePort.h>

#include <Dataflow/Modules/Visualization/share.h>

namespace SCIRun {

class SCISHARE ConvertFieldsToTexture : public Module
{
public:
  ConvertFieldsToTexture(GuiContext* ctx, const std::string& name = "ConvertFieldsToTexture",
                         SchedClass sc = Source, const string& cat = "Visualization", 
                         const string& pack = "SCIRun");
  virtual ~ConvertFieldsToTexture();

  virtual void execute();

  template<class T>
  void create_scaled_value_nrrd(T* value, unsigned char* nrrd, size_t sz, double fmin, double fmax );

  template<class T>
  void create_scaled_value_normal_nrrd(T* value, Vector* normal, unsigned char* nrrd, size_t sz, double fmin, double fmax );

  void create_scaled_gradient_magnitude_nrrd(Vector* gradient, unsigned char* nrrd, size_t sz, double fmin, double fmax );

protected:

  GuiDouble gui_vminval_;
  GuiDouble gui_vmaxval_;
  GuiDouble gui_gminval_;
  GuiDouble gui_gmaxval_;

  GuiInt gui_fixed_;
  GuiInt gui_card_mem_;
  GuiInt gui_card_mem_auto_;

  GuiInt gui_histogram_;    
  GuiDouble gui_gamma_;
  
  int card_mem_;
};

} // namespace SCIRun

#endif
