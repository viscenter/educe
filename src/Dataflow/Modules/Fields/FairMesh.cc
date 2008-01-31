//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  License for the specific language governing rights and limitations under
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
//    File   : FairMesh.cc
//    Author : Martin Cole
//    Date   : Fri Mar 16 09:40:53 2007

#include <Core/Malloc/Allocator.h>
#include <Core/Datatypes/Field.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Modules/Fields/FairMesh.h>
#include <Dataflow/Network/Ports/FieldPort.h>

namespace SCIRun {

class FairMesh : public Module {
public:
  FairMesh(GuiContext*);
  virtual void execute();
  
private:
  GuiInt       iterations_;
  GuiString    method_;

  int last_iterations_;
  string last_method_;
  Handle<SurfaceFairingAlgoBase> fair_;
};


DECLARE_MAKER(FairMesh)
FairMesh::FairMesh(GuiContext* ctx) : 
  Module("FairMesh", ctx, Source, "NewField", "SCIRun"),
  iterations_(get_ctx()->subVar("iterations"), 50),
  method_(get_ctx()->subVar("method"), "fast"),
  last_iterations_(-1),
  last_method_(""),
  fair_(0)
{}


void FairMesh::execute()
{
  SCIRun::FieldHandle input;
  if (!get_input_handle("Input Mesh", input, true)) return;

  //! If it is a new field get appropriate algorithm, 
  //! otherwise the cached algorithm is still good.
  bool changed = false;
  if (inputs_changed_)
  {
    inputs_changed_ = false;
    const TypeDescription *td = input->get_type_description();
    CompileInfoHandle ci = SurfaceFairingAlgoBase::get_compile_info(td);
    if (module_dynamic_compile(ci, fair_)) {
      remark("Compiled surface fairing for new input.");
    } else {
      error("Failed to compile fairing algorithm for input.");
      return;
    }
    changed = true;
  }
  reset_vars();
  int        it = iterations_.get();
  string method = method_.get();

  if (method != last_method_) {
    last_method_ = method;
    fair_->generate_neighborhoods(input, method);
    remark("Cached neighborhood info for new input.");    
  }

  if (changed ||
      it != last_iterations_ ||
      method != last_method_)
  {
    last_iterations_ = it;
    SCIRun::FieldHandle output = fair_->iterate(this, input, it, method);
    send_output_handle("Faired Mesh", output);
  }
}


CompileInfoHandle
SurfaceFairingAlgoBase::get_compile_info(const TypeDescription *td)
{
  // use cc_to_h if this is in the .cc file, otherwise just __FILE__
  static const string include_path(TypeDescription::cc_to_h(__FILE__));
  static const string base_class_name("SurfaceFairingAlgoBase");
  static const string template_class_name("SurfaceFairingAlgo");


  CompileInfo *rval = 
    scinew CompileInfo(template_class_name + "." +
		       td->get_name(".", ".") + ".",
                       base_class_name, 
                       template_class_name, 
                       td->get_name());

  // Add in the include path to compile this obj
  rval->add_include(include_path);
  td->fill_compile_info(rval);
  return rval;
}

} // End namespace SCIRun

