/*
 *  SmoothVecFieldMedian.cc:
 *
 *  Written by:
 *   darrell
 *   TODAY'S DATE HERE
 *
 */
#include <Core/Algorithms/Fields/FieldData/SmoothVecFieldMedian.h>

#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Datatypes/Field.h>
#include <Dataflow/Network/Module.h>
//#include <Core/Malloc/Allocator.h>

namespace SCIRun {

//using namespace SCIRun;

class SmoothVecFieldMedian : public Module 
{
  public:

    SmoothVecFieldMedian(GuiContext* ctx);
    virtual ~SmoothVecFieldMedian();
    virtual void execute();
    virtual void tcl_command(GuiArgs&, void*);
  
  private:
    SCIRunAlgo::SmoothVecFieldMedianAlgo algo_;

};


DECLARE_MAKER(SmoothVecFieldMedian)

SmoothVecFieldMedian::SmoothVecFieldMedian(GuiContext* ctx) :
  Module("SmoothVecFieldMedian", ctx, Filter, "ChangeFieldData", "SCIRun")
{
  algo_.set_progress_reporter(this);
}

SmoothVecFieldMedian::~SmoothVecFieldMedian(){
}

void
SmoothVecFieldMedian::execute()
{
  FieldHandle input, output;

  get_input_handle( "vector_median_filt", input, true );

  // If no data or a changed recalcute.
  if( inputs_changed_ || !oport_cached("Output_vector_field") )
  {
    if (!(algo_.run(input,output))) return;
    send_output_handle( "Output_vector_field", output );
  }

}

void
SmoothVecFieldMedian::tcl_command(GuiArgs& args, void* userdata)
{
  Module::tcl_command(args, userdata);
}

} // End namespace SCIRun


