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
#include <Core/Algorithms/ArrayMath/ArrayObject.h>
#include <Core/Algorithms/ArrayMath/ArrayEngine.h>

// DataArrayMath is my namespace in which all Scalar, Vector, and Tensor math is defined.
// The classes in this namespace have a definition which is more in line with
// how functions are written in Algebra or Matlab than the native SCIRun Tensor
// and Vector classes. Hence all calculations are performed in this specially
// constructed namespace, to enhance the usability of SCIRun. 
// The TVMHelp system contains an almost complete list of functions that are
// defined in the DataArrayMath, so when new functions are added, one does 
// not need to update the GUI, but the module dynamically looks up the available
// functions when it is created.

#include <Core/Algorithms/ArrayMath/ArrayEngineHelp.h>
#include <Core/Algorithms/ArrayMath/ArrayEngineMath.h>

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/String.h>
#include <Core/Datatypes/Field.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/StringPort.h>

namespace ModelCreation {

using namespace SCIRun;

class SelectAndSetFieldData : public Module {
  public:
    SelectAndSetFieldData(GuiContext*);

    virtual ~SelectAndSetFieldData();

    virtual void execute();

    virtual void tcl_command(GuiArgs&, void*);

  private:
    GuiString guiselection1_;
    GuiString guifunction1_;     // function code
    GuiString guiselection2_;
    GuiString guifunction2_;     // function code
    GuiString guiselection3_;
    GuiString guifunction3_;     // function code
    GuiString guiselection4_;
    GuiString guifunction4_;     // function code
    GuiString guifunctiondef_;     // function code
    GuiString guiformat_;       // scalar, vector, or tensor ?
};


DECLARE_MAKER(SelectAndSetFieldData)
SelectAndSetFieldData::SelectAndSetFieldData(GuiContext* ctx)
  : Module("SelectAndSetFieldData", ctx, Source, "ChangeFieldData", "ModelCreation"),
  guiselection1_(get_ctx()->subVar("selection1")),
  guifunction1_(get_ctx()->subVar("function1")),
  guiselection2_(get_ctx()->subVar("selection2")),
  guifunction2_(get_ctx()->subVar("function2")),
  guiselection3_(get_ctx()->subVar("selection3")),
  guifunction3_(get_ctx()->subVar("function3")),
  guiselection4_(get_ctx()->subVar("selection4")),
  guifunction4_(get_ctx()->subVar("function4")),  
  guifunctiondef_(get_ctx()->subVar("functiondef")),
  guiformat_(get_ctx()->subVar("format"))  
{
}


SelectAndSetFieldData::~SelectAndSetFieldData()
{
}


void
SelectAndSetFieldData::execute()
{
  FieldHandle field;
  std::vector<MatrixHandle> matrices;
  
  if (!(get_input_handle("Field",field,true))) return;
  get_dynamic_input_handles("Array",matrices,false);

  get_gui()->lock();
  get_gui()->eval(get_id()+" update_text");
  get_gui()->unlock();

  if (inputs_changed_ || guiselection1_.changed() || guifunction1_.changed() ||
      guiselection2_.changed() || guifunction2_.changed() ||
      guiselection3_.changed() || guifunction3_.changed() ||
      guiselection4_.changed() || guifunction4_.changed() ||
      guiformat_.changed() || guifunctiondef_.changed() ||
      !oport_cached("Field"))
  {
    // Get number of matrix ports with data (the last one is always empty)
    size_t numinputs = matrices.size(); 

    if (numinputs > 23)
    {
      error("This module cannot handle more than 23 input matrices");
      return;
    }
    
    SCIRunAlgo::ArrayObjectList inputlist(numinputs+4,SCIRunAlgo::ArrayObject(this));
    SCIRunAlgo::ArrayObjectList outputlist(1,SCIRunAlgo::ArrayObject(this));
    
    // Create the DATA object for the function
    // DATA is the data on the field
    if(!(inputlist[0].create_inputdata(field,"DATA")))
    {
      error("Failed to read field data");
      return;
    }

    // Create the POS, X,Y,Z, data location objects.  
    if(!(inputlist[1].create_inputlocation(field,"POS","X","Y","Z")))
    {
      error("Failed to read node/element location data");
      return;
    }

    // Create the ELEMENT object describing element properties
    if(!(inputlist[2].create_inputelement(field,"ELEMENT")))
    {
      error("Failed to read element data");
      return;
    }

    // Add an object for getting the index and size of the array.
    if(!(inputlist[3].create_inputindex("INDEX","SIZE")))
    {
      error("Internal error in module");
      return;
    }

    // Loop through all matrices and add them to the engine as well
    char mname = 'A';
    std::string matrixname("A");
    
    for (size_t p = 0; p < numinputs; p++)
    {
      if (matrices[p].get_rep() == 0)
      {
        error("No matrix was found on input port");
        return;      
      }

      matrixname[0] = mname++;    
      if (!(inputlist[p+4].create_inputdata(matrices[p],matrixname)))
      {
        std::ostringstream oss;
        oss << "Input matrix " << p+1 << "is not a valid ScalarArray, VectorArray, or TensorArray";
        error(oss.str());
        return;
      }
    }

    // Check the validity of the input
    int n = 1;
    for (size_t r=0; r<inputlist.size();r++)
    {
      if (n == 1) n = inputlist[r].size();
      if ((inputlist[r].size() != n)&&(inputlist[r].size() != 1))
      {
        if (r < 4)
        {
          error("Number of data entries does not seem to match number of elements/nodes");
          return;
        }
        else
        {
          std::ostringstream oss;
          oss << "The number of rows in matrix " << r-2 << "does not seem to match the number of datapoints in the field";
          error(oss.str());
        }
      }
    }

    // Create the engine to compute new data
    SCIRunAlgo::ArrayEngine engine(this);
    
    std::string format = guiformat_.get();
    if (format == "") format = "double";
    
    // Add as well the output object
    FieldHandle ofield;
    if(!(outputlist[0].create_outputdata(field,format,"RESULT",ofield)))
    {
      return;
    }
        
    std::string function1 = guifunction1_.get();
    std::string selection1 = guiselection1_.get();
    std::string function2 = guifunction2_.get();
    std::string selection2 = guiselection2_.get();
    std::string function3 = guifunction3_.get();
    std::string selection3 = guiselection3_.get();
    std::string function4 = guifunction4_.get();
    std::string selection4 = guiselection4_.get();
    std::string functiondef = guifunctiondef_.get();

    if (selection1.size() > 0) while ((selection1[selection1.size()-1] == '\n')||(selection1[selection1.size()-1] == ' ')||(selection1[selection1.size()-1] == '\r')) { selection1 = selection1.substr(0,selection1.size()-1); if (selection1.size() == 0) break; }
    if (selection2.size() > 0) while ((selection2[selection2.size()-1] == '\n')||(selection2[selection2.size()-1] == ' ')||(selection2[selection2.size()-1] == '\r')) { selection2 = selection2.substr(0,selection2.size()-1); if (selection2.size() == 0) break; }
    if (selection3.size() > 0) while ((selection3[selection3.size()-1] == '\n')||(selection3[selection3.size()-1] == ' ')||(selection3[selection3.size()-1] == '\r')) { selection3 = selection3.substr(0,selection3.size()-1); if (selection3.size() == 0) break; }
    if (selection4.size() > 0) while ((selection4[selection4.size()-1] == '\n')||(selection4[selection4.size()-1] == ' ')||(selection4[selection4.size()-1] == '\r')) { selection4 = selection4.substr(0,selection4.size()-1); if (selection4.size() == 0) break; }

    if (function1.size() > 0) while ((function1[function1.size()-1] == '\n')||(function1[function1.size()-1] == ' ')||(function1[function1.size()-1] == '\r')) { function1 = function1.substr(0,function1.size()-1); if (function1.size() == 0) break; }
    if (function2.size() > 0) while ((function2[function2.size()-1] == '\n')||(function2[function2.size()-1] == ' ')||(function2[function2.size()-1] == '\r')) { function2 = function2.substr(0,function2.size()-1); if (function2.size() == 0) break; }
    if (function3.size() > 0) while ((function3[function3.size()-1] == '\n')||(function3[function3.size()-1] == ' ')||(function3[function3.size()-1] == '\r')) { function3 = function3.substr(0,function3.size()-1); if (function3.size() == 0) break; }
    if (function4.size() > 0) while ((function4[function4.size()-1] == '\n')||(function4[function4.size()-1] == ' ')||(function4[function4.size()-1] == '\r')) { function4 = function4.substr(0,function4.size()-1); if (function4.size() == 0) break; }
    if (functiondef.size() > 0) while ((functiondef[functiondef.size()-1] == '\n')||(functiondef[functiondef.size()-1] == ' ')||(functiondef[functiondef.size()-1] == '\r')) { functiondef = functiondef.substr(0,functiondef.size()-1); if (functiondef.size() == 0) break; }
    
    std::string f = "\n";
    if ((selection1.size()) && (function1.size())) f += "if ("+selection1+")\n{\n RESULT = " + function1 + ";}\n else ";
    if ((selection2.size()) && (function2.size())) f += "if ("+selection2+")\n{\n RESULT = " + function2 + ";}\n else ";
    if ((selection3.size()) && (function3.size())) f += "if ("+selection3+")\n{\n RESULT = " + function3 + ";}\n else ";
    if ((selection4.size()) && (function4.size())) f += "if ("+selection4+")\n{\n RESULT = " + function4 + ";}\n else ";
    if (functiondef.size()) f += "\n{\n RESULT = " + functiondef + ";\n}\n"; else f += "\n {\n }\n";
     
    // Actual engine call, which does the dynamic compilation, the creation of the
    // code for all the objects, as well as inserting the function and looping 
    // over every data point
    if (!engine.engine(inputlist,outputlist,f))
    {
      error("An error occured while executing function");
      return;
    }
    
    send_output_handle("Field",ofield,false);
  }
}

extern std::string tvm_help_field;


void
SelectAndSetFieldData::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2)
  {
    args.error("ComputeDataField needs a minor command");
    return;
  }

  if( args[1] == "gethelp" )
  {
    DataArrayMath::ArrayEngineHelp Help;
    get_gui()->lock();
    get_gui()->eval("global " + get_id() +"-help");
    get_gui()->eval("set " + get_id() + "-help {" + Help.gethelp(true) +"}");
    get_gui()->unlock();

    return;
  }
  else
  {
    Module::tcl_command(args, userdata);
  }
}

} // End namespace ModelCreation


