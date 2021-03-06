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
#include <Core/Algorithms/ArrayMath/ArrayEngineHelp.h>
#include <Core/Algorithms/ArrayMath/ArrayEngineMath.h>

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/String.h>
#include <Dataflow/Network/Ports/StringPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

class CalculateDataArray : public Module {
public:
  CalculateDataArray(GuiContext*);
  virtual void execute();
  virtual void tcl_command(GuiArgs&, void*);
  
private:
    GuiString guifunction_;
    GuiString guiformat_;
};


DECLARE_MAKER(CalculateDataArray)
CalculateDataArray::CalculateDataArray(GuiContext* ctx)
  : Module("CalculateDataArray", ctx, Source, "DataArrayMath", "SCIRun"),
  guifunction_(get_ctx()->subVar("function")),
  guiformat_(get_ctx()->subVar("format"))
{
}

void CalculateDataArray::execute()
{
  // Define handles for input
  MatrixHandle data;
  StringHandle func;
  std::vector<MatrixHandle> matrices;
  
  // Get latest handles from ports
  get_input_handle("Function",func,false);
  get_input_handle("DataArray",data,false);
  get_dynamic_input_handles("Array",matrices,false);

  get_gui()->lock();
  get_gui()->eval(get_id()+" update_text");
  get_gui()->unlock();

  // Do something if data changed
  if (inputs_changed_ || guifunction_.changed() || 
      guiformat_.changed() || !oport_cached("DataArray"))
  {

    size_t mstart = 2;
    if (data.get_rep() == 0) mstart--;

    // Check the size of the inputs
    size_t numinputs = matrices.size();
    if (numinputs > 26)
    {
      error("This module cannot handle more than 26 input matrices");
      return;
    }
    
    // Define input aray
    SCIRunAlgo::ArrayObjectList inputlist(numinputs+mstart,SCIRunAlgo::ArrayObject(this));
    SCIRunAlgo::ArrayObjectList outputlist(1,SCIRunAlgo::ArrayObject(this));
  

    if (func.get_rep())
    {
      guifunction_.set(func->get());
      get_ctx()->reset();
    }

    
    char mname = 'A';
    std::string matrixname("A");
    
    int n = 1;

    if (data.get_rep())
    {
      if ((data->ncols()==1)||(data->ncols()==3)||(data->ncols()==6)||(data->ncols()==9))
      {
        inputlist[0].create_input_data(data,"DATA");
      }
      
      if (n > 1) 
      {
        if (n != data->nrows()&&(data->nrows() != 1))
        {
          std::ostringstream oss;
          oss << "The number of elements in each ScalarArray, VectorArray, or TensorArray is not equal";
          error(oss.str());
          return;          
        }
      }
      else
      {
        n = data->nrows();
      }    
    }
    
    // Add an object for getting the index and size of the array.
    if(!(inputlist[mstart-1].create_input_index("INDEX","SIZE")))
    {
      error("Internal error in module");
      return;
    } 
   
    for (size_t p = 0; p < numinputs; p++)
    {      
      if (matrices[p].get_rep())
      {
        if ((matrices[p]->ncols()==1)||(matrices[p]->ncols()==3)||(matrices[p]->ncols()==6)||(matrices[p]->ncols()==9))
        {
          matrixname[0] = mname++;
          inputlist[p+mstart].create_input_data(matrices[p],matrixname);
        }
        else
        {
          std::ostringstream oss;
          oss << "Input matrix " << p+1 << "is not a valid ScalarArray, VectorArray, or TensorArray";
          error(oss.str());
          return;
        }
        
        if (n > 1) 
        {
          if (n != matrices[p]->nrows()&&(matrices[p]->nrows() != 1))
          {
            std::ostringstream oss;
            oss << "The number of elements in each ScalarArray, VectorArray, or TensorArray is not equal";
            error(oss.str());
            return;          
          }
        }
        else
        {
          n = matrices[p]->nrows();
        }
      }
    }
  
    std::string format = guiformat_.get();
      
    MatrixHandle omatrix;  
    outputlist[0].create_output_data(n,format,"RESULT",omatrix);
  
    std::string function = guifunction_.get();
    
    SCIRunAlgo::ArrayEngine engine(this);
    if (!engine.engine(inputlist,outputlist,function))
    {
      error("An error occured while executing function");
      return;
    }

    send_output_handle("DataArray", omatrix);
  }
}


void
CalculateDataArray::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2)
  {
    args.error("ComputeScalarArray needs a minor command");
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


} // End namespace SCIRun


