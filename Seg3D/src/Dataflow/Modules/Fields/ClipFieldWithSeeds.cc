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

#include <Core/Algorithms/Fields/ClipMesh/ClipMeshBySelection.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/StringPort.h>
#include <Dataflow/Network/Module.h>


namespace SCIRun {

class ClipFieldWithSeeds : public Module {
  public:
    ClipFieldWithSeeds(GuiContext*);

    virtual void execute();
    virtual void tcl_command(GuiArgs&, void*);

    virtual void presave();
    
  private:
    GuiString guifunction_;  
    GuiString guimethod_;
    SCIRunAlgo::ClipMeshBySelectionAlgo clipping_algo_;
};


DECLARE_MAKER(ClipFieldWithSeeds)
ClipFieldWithSeeds::ClipFieldWithSeeds(GuiContext* ctx)
  : Module("ClipFieldWithSeeds", ctx, Source, "NewField", "SCIRun"),
    guifunction_(get_ctx()->subVar("function"),"abs(FDATA-SDATA) < 0.2"),
    guimethod_(get_ctx()->subVar("method"),"onenode")
{
  clipping_algo_.set_progress_reporter(this);
}


void ClipFieldWithSeeds::execute()
{
  // Define local handles of data objects:
  FieldHandle field, seeds;
  StringHandle func;

  // Get the new input data:  
  get_input_handle("Field",field,true);
  get_input_handle("Seeds",seeds,false);

  if (get_input_handle("Function",func,false))
  {
    if (func.get_rep())
    {
      guifunction_.set(func->get());
      get_ctx()->reset();  
    }
  }  

  get_gui()->lock();
  get_gui()->eval(get_id()+" update_text");
  get_gui()->unlock();


  // Only do work if needed:
  if (inputs_changed_ || guifunction_.changed() || guimethod_.changed() ||
      !oport_cached("Field") || !oport_cached("Mapping"))
  {
    // Get number of matrix ports with data (the last one is always empty)
    std::vector<FieldHandle> seed_points;
    
    VMesh* svmesh = seeds->vmesh();
    VField* svfield = seeds->vfield();
    VMesh::size_type num_nodes = svmesh->num_nodes();
    FieldInformation fi(seeds); fi.make_pointcloudmesh(); fi.make_constantdata();
    
    for (VMesh::Node::index_type idx=0; idx<num_nodes;idx++)
    {
      Point p;
      svmesh->get_center(p,idx);
      FieldHandle newfield = CreateField(fi);
      VMesh* omesh = newfield->vmesh();
      VField* ofield = newfield->vfield();
      omesh->add_point(p);
      ofield->resize_values();
      ofield->copy_value(svfield,idx,0);
      seed_points.push_back(newfield);
    }


    std::string method = guimethod_.get();
    int basis_order = 1;
    if (method == "cell") basis_order = 0;

    int field_basis_order = field->vfield()->basis_order();

    FieldInformation sfi(field);
    sfi.make_char();
    sfi.make_lineardata();
    if (basis_order == 0) sfi.make_constantdata();
    FieldHandle sfield = CreateField(sfi,field->mesh());
    sfield->vfield()->clear_all_values(); 

    for (size_t j=0; j<seed_points.size(); j++)
    {
      NewArrayMathEngine engine;
      engine.set_progress_reporter(this);

      engine.add_input_fielddata("__RESULT",sfield);

      // Create the DATA object for the function
      // DATA is the data on the field
      if (basis_order == field_basis_order)
      {
        if(!(engine.add_input_fielddata("FDATA",field))) return;
      }
      
      // Create the POS, X,Y,Z, data location objects.  

      if(!(engine.add_input_fielddata_location("FPOS",field,basis_order))) return;
      if(!(engine.add_input_fielddata_coordinates("FX","FY","FZ",field,basis_order))) return;

      // Create the ELEMENT object describing element properties
      if(!(engine.add_input_fielddata_element("FELEMENT",field,basis_order))) return;

      int field_basis_order2 = seed_points[j]->vfield()->basis_order();
      if (basis_order == field_basis_order2)
      {
        if(!(engine.add_input_fielddata("SDATA",seed_points[j]))) return;
      }
      
      // Create the POS, X,Y,Z, data location objects.  

      if(!(engine.add_input_fielddata_location("SPOS",seed_points[j],basis_order))) return;
      if(!(engine.add_input_fielddata_coordinates("SX","SY","SZ",seed_points[j],basis_order))) return;

      // Create the ELEMENT object describing element properties
      if(!(engine.add_input_fielddata_element("SELEMENT",seed_points[j],basis_order))) return;    
      
      if(!(engine.add_output_fielddata("__RESULT",sfield,basis_order,"char"))) return;

      // Add an object for getting the index and size of the array.

      if(!(engine.add_index("INDEX"))) return;
      if(!(engine.add_size("SIZE"))) return;


      // Define the function we are using for clipping:
      std::string function = guifunction_.get();
      if (function.find("RESULT") == std::string::npos)
        function = std::string("RESULT = ") + function;
      
      function += "; __RESULT = __RESULT + boolean(RESULT);";
      if(!(engine.add_expressions(function))) return;

      // Actual engine call, which does the dynamic compilation, the creation of the
      // code for all the objects, as well as inserting the function and looping 
      // over every data point

      if (!(engine.run())) return;

      // Get the result from the engine
      engine.get_field("__RESULT",sfield);    
    }
    
    FieldHandle  ofield;    
    MatrixHandle mapping;
          
    clipping_algo_.set_option("method",guimethod_.get());
    if(!(clipping_algo_.run(field,sfield,ofield,mapping))) return;

    send_output_handle("Field", ofield);
    send_output_handle("Mapping", mapping);
  }
}


void
ClipFieldWithSeeds::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2)
  {
    args.error("ClipFieldWithSeeds needs a minor command");
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
ClipFieldWithSeeds::presave()
{
  // update gui_function_ before saving.
  get_gui()->execute(get_id() + " update_text");
}


} // End namespace ModelCreation


