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


// Include all code for the dynamic engine
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Field.h>
#include <Core/Parser/ArrayMathEngine.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/StringPort.h>

namespace SCIRun {

class CalculateFieldData3 : public Module 
{
  public:
    CalculateFieldData3(GuiContext*);
    virtual ~CalculateFieldData3() {}
    virtual void execute();
    
    virtual void tcl_command(GuiArgs&, void*);
    virtual void presave();
    
  private:
    GuiString guifunction_;     // function code
    GuiString guiformat_;       // scalar, vector, or tensor ?
};


DECLARE_MAKER(CalculateFieldData3)
CalculateFieldData3::CalculateFieldData3(GuiContext* ctx)
  : Module("CalculateFieldData3", ctx, Source, "ChangeFieldData", "SCIRun"),
  guifunction_(get_ctx()->subVar("function")),
  guiformat_(get_ctx()->subVar("format"))  
{
}

void CalculateFieldData3::execute()
{
  // Define input handles:
  FieldHandle field, field2, field3;
  StringHandle func;
  std::vector<MatrixHandle> matrices;
  
  // Get data from input ports:
  get_input_handle("Field1",field,true);
  get_input_handle("Field2",field2,false);
  get_input_handle("Field3",field3,false);
  if (get_input_handle("Function",func,false))
  {
    if (func.get_rep())
    {
      guifunction_.set(func->get());
      get_ctx()->reset();  
    }
  }
  get_dynamic_input_handles("Array",matrices,false);

  // Force TCL to update variables:
  get_gui()->lock();
  get_gui()->eval(get_id()+" update_text");
  get_gui()->unlock();
  
  if (inputs_changed_ || guifunction_.changed() || 
      guiformat_.changed() || !oport_cached("Field"))
  {
    // Get number of matrix ports with data (the last one is always empty)
    size_t numinputs = matrices.size();
    
    if (numinputs > 23)
    {
      error("This module cannot handle more than 23 input matrices.");
      return;
    }

    NewArrayMathEngine engine;
    engine.set_progress_reporter(this);

    // Create the DATA object for the function
    // DATA is the data on the field
    if(!(engine.add_input_fielddata("DATA",field))) return;
    if(!(engine.add_input_fielddata("DATA1",field))) return;

    // Create the POS, X,Y,Z, data location objects.  

    if(!(engine.add_input_fielddata_location("POS",field))) return;
    if(!(engine.add_input_fielddata_location("POS1",field))) return;
    if(!(engine.add_input_fielddata_coordinates("X","Y","Z",field))) return;
    if(!(engine.add_input_fielddata_coordinates("X1","Y1","Z1",field))) return;

    // Create the ELEMENT object describing element properties
    if(!(engine.add_input_fielddata_element("ELEMENT",field))) return;
    if(!(engine.add_input_fielddata_element("ELEMENT1",field))) return;

    if (field2.get_rep())
    {
      if(!(engine.add_input_fielddata("DATA2",field2))) return;

      // Create the POS, X,Y,Z, data location objects.  

      if(!(engine.add_input_fielddata_location("POS2",field2))) return;
      if(!(engine.add_input_fielddata_coordinates("X2","Y2","Z2",field2))) return;

      // Create the ELEMENT object describing element properties
      if(!(engine.add_input_fielddata_element("ELEMENT2",field2))) return;
    }

    if (field3.get_rep())
    {
      if(!(engine.add_input_fielddata("DATA3",field3))) return;

      // Create the POS, X,Y,Z, data location objects.  

      if(!(engine.add_input_fielddata_location("POS3",field3))) return;
      if(!(engine.add_input_fielddata_coordinates("X3","Y3","Z3",field3))) return;

      // Create the ELEMENT object describing element properties
      if(!(engine.add_input_fielddata_element("ELEMENT3",field3))) return;
    }
    
     // Loop through all matrices and add them to the engine as well
    char mname = 'A';
    std::string matrixname("A");
    
    for (size_t p = 0; p < numinputs; p++)
    {
      if (matrices[p].get_rep() == 0)
      {
        error("No matrix was found on input port.");
        return;      
      }

      matrixname[0] = mname++;
      if (!(engine.add_input_matrix(matrixname,matrices[p]))) return;
    }

    int basis_order = field->vfield()->basis_order();
    
    std::string format = guiformat_.get();
    if (format == "") format = "double";
    
    if(!(engine.add_output_fielddata("RESULT",field,basis_order,format))) return;

    // Add an object for getting the index and size of the array.

    if(!(engine.add_index("INDEX"))) return;
    if(!(engine.add_size("SIZE"))) return;

    std::string function = guifunction_.get();
    if(!(engine.add_expressions(function))) return;

    // Actual engine call, which does the dynamic compilation, the creation of the
    // code for all the objects, as well as inserting the function and looping 
    // over every data point

    if (!(engine.run())) return;

    // Get the result from the engine
    FieldHandle ofield;    
    engine.get_field("RESULT",ofield);

    // send new output if there is any: 
    send_output_handle("Field", ofield);
  }
}

void
CalculateFieldData3::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2)
  {
    args.error("CalculateFieldData3 needs a minor command");
    return;
  }

  if( args[1] == "gethelp" )
  {
//    DataArrayMath::ArrayEngineHelp Help;
//    get_gui()->lock();
//    get_gui()->eval("global " + get_id() +"-help");
//    get_gui()->eval("set " + get_id() + "-help {" + Help.gethelp(true) +"}");
//    get_gui()->unlock();
    return;  
  }
  else
  {
    Module::tcl_command(args, userdata);
  }
}

void
CalculateFieldData3::presave()
{
  // update gui_function_ before saving.
  get_gui()->execute(get_id() + " update_text");
}

} // End namespace SCIRun

