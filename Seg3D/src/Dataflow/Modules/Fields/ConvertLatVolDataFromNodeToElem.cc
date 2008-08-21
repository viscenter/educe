
#include <Core/Algorithms/Fields/ConvertMeshType/ConvertLatVolDataFromNodeToElem.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

class ConvertLatVolDataFromNodeToElem : public Module
{
  public:
    ConvertLatVolDataFromNodeToElem(GuiContext* ctx);
    virtual ~ConvertLatVolDataFromNodeToElem() {}

    virtual void execute();
    
  private:
    SCIRunAlgo::ConvertLatVolDataFromNodeToElemAlgo algo_;
};


DECLARE_MAKER(ConvertLatVolDataFromNodeToElem)

ConvertLatVolDataFromNodeToElem::ConvertLatVolDataFromNodeToElem(GuiContext* ctx)
  : Module("ConvertLatVolDataFromNodeToElem", ctx, Filter, "ChangeFieldData", "SCIRun")
{
  algo_.set_progress_reporter(this);
}

void
ConvertLatVolDataFromNodeToElem::execute()
{
  // Get input field.
  FieldHandle input, output;
  get_input_handle("Node Field", input, true);
  
  if (inputs_changed_ || !oport_cached("Elem Field"))
  {
    //! Run algorithm
    if(!(algo_.run(input,output))) return;
    
    //! Send output
    send_output_handle("Elem Field", output);
  }
}

} // End namespace SCIRun

