/*
 *  RegisterWithCorrespondences.cc:
 *
 *  Written by:
 *   darrell
 *   TODAY'S DATE HERE
 *
 */

#include <Dataflow/Network/Module.h>
#include <Core/Datatypes/Field.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Algorithms/Fields/RegisterWithCorrespondences.h>
//#include <sci_defs/lapack_defs.h>
//#include <Core/Math/MiscMath.h>

//#if defined(HAVE_LAPACK)
//#include <Core/Math/sci_lapack.h>
//#endif


namespace SCIRun {

using namespace SCIRun;
//using namespace SCIRunAlgo;

class RegisterWithCorrespondences : public Module {
public:
  RegisterWithCorrespondences(GuiContext*);

  virtual ~RegisterWithCorrespondences();

  virtual void execute();

  virtual void tcl_command(GuiArgs&, void*);
private:
  SCIRunAlgo::RegisterWithCorrespondencesAlgo algo_;
  GuiString method_;
  GuiString edmethod_;

};


DECLARE_MAKER(RegisterWithCorrespondences)

RegisterWithCorrespondences::RegisterWithCorrespondences(GuiContext* ctx) :
  Module("RegisterWithCorrespondences", ctx, Source, "ChangeFieldData", "SCIRun"),
    method_(ctx->subVar("method")),
    edmethod_(ctx->subVar("ed-method"))
{
  algo_.set_progress_reporter(this);
}

RegisterWithCorrespondences::~RegisterWithCorrespondences(){
}

void
RegisterWithCorrespondences::execute()
{
  FieldHandle input, cors1, cors2, output;
  
  get_input_handle("InputField",input,true);
  get_input_handle("Correspondences1",cors1,true);
  get_input_handle("Correspondences2",cors2,true);
  

  
  if (inputs_changed_ || !oport_cached("Output") || method_.changed() ||
      edmethod_.changed())
  {
    //FieldsAlgo algo(this);
  
    if (method_.get() == "transform")
    {
      //FieldsAlgo::FilterSettings settings;
      
      if (edmethod_.get() == "affine")
      {
        cout<<"affine"<<endl;
        
        if(!(algo_.runA(input,cors1,cors2,output))) return;
      }
      else  if (edmethod_.get() == "morph")
      {
        cout<<"morph"<<endl;
         if(!(algo_.runM(input,cors1,cors2,output))) return;
      }
      else
      {
        cout<<"none"<<endl;
         if(!(algo_.runN(input,cors1,cors2,output))) return;    
      }
    }
  
    send_output_handle("Output",output,true);
  }



}

void
RegisterWithCorrespondences::tcl_command(GuiArgs& args, void* userdata)
{
  Module::tcl_command(args, userdata);
}

} // End namespace SCIRun


