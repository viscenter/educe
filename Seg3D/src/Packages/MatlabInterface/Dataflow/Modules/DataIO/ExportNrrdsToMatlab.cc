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
 * FILE: ExportNrrdsToMatlab.cc
 * AUTH: Jeroen G Stinstra
 * DATE: 30 MAR 2004
 */ 

#include <sgi_stl_warnings_off.h>
#include <sstream>
#include <string>
#include <vector>
#include <sgi_stl_warnings_on.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/NrrdPort.h>
#include <Dataflow/Network/Ports/StringPort.h>
#include <Core/Malloc/Allocator.h>
#include <Core/Matlab/matlabfile.h>
#include <Core/Matlab/matlabarray.h>
#include <Core/Matlab/matlabconverter.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Core/Datatypes/String.h>
#include <Core/OS/FullFileName.h>

namespace MatlabIO {

using namespace SCIRun;

class ExportNrrdsToMatlab : public Module 
{

  public:

    // Constructor
    ExportNrrdsToMatlab(GuiContext*);

    // Destructor
    virtual ~ExportNrrdsToMatlab();

    // Std functions for each module
    // execute():
    //   Execute the module and put data on the output port
    
    virtual void execute();
    
  private:

    // Support functions for converting between TCL and C++
    // converttcllist:
    // TCL lists are stings in which the elements are separated by {}.
    // In a C++ environment it is easier to have them as a STL vector
    // object. This function converts the TCL list into an STL object
    // convertdataformat:
    // In TCL the dataformat is a string this function is used to convert
    // it into its matlab enum counterpart.
    std::vector<std::string> converttcllist(std::string str);
    matlabarray::mitype convertdataformat(std::string dataformat);

    // Support functions for the GUI
    // displayerror:
    // Directly reporting an error to the user (not in the error log)
    // overwrite:
    // Ask for confirmation to overwrite the file if it already exists
    
    void displayerror(std::string str);
    bool overwrite();

private:

    enum { NUMPORTS = 6};
    
    // GUI variables
    GuiFilename guifilename_; // .mat filename (import from GUI)
    GuiString   guifilenameset_;
    GuiString   guimatrixname_; // A list of the matrix names
    GuiString   guidataformat_; // A list of the dataformat for each matrix (int, double etc.)
    GuiString   guimatrixformat_; // A list of the matlabarray format (numeric array, structured array)
    
};

DECLARE_MAKER(ExportNrrdsToMatlab)

// Constructor:
// Initialise all the variables shared between TCL and SCIRun
// matrixname contains a list of matrix names.
// dataformat contains a list of the format of each matrix (int32,single,double, etc...)
// matrixformat contains a list of the way the object is represented in matlab
// e.g. as a structured object or an object with the dataarray only

ExportNrrdsToMatlab::ExportNrrdsToMatlab(GuiContext* ctx)
  : Module("ExportNrrdsToMatlab", ctx, Sink, "DataIO", "MatlabInterface"),
    guifilename_(get_ctx()->subVar("filename")),
    guifilenameset_(get_ctx()->subVar("filename-set")),
    guimatrixname_(get_ctx()->subVar("matrixname")),     
    guidataformat_(get_ctx()->subVar("dataformat")),    
    guimatrixformat_(get_ctx()->subVar("matrixformat"))
{
}

// Destructor:
// All my objects have descrutors and hence nothing needs
// explicit descruction
ExportNrrdsToMatlab::~ExportNrrdsToMatlab()
{
}

// Execute:
// Inner workings of this module

void ExportNrrdsToMatlab::execute()
{
  matlabconverter translate(dynamic_cast<SCIRun::ProgressReporter*>(this));

	StringHandle stringH;
	get_input_handle("Filename",stringH,false);
	if (stringH.get_rep())
	{
		std::string filename = stringH->get();
		guifilename_.set(filename);
		get_ctx()->reset();
	}

  bool porthasdata[NUMPORTS];
  SCIRun::NrrdDataHandle matrixhandle[NUMPORTS];

  for (int p=0; p<NUMPORTS; p++)
  {
	  porthasdata[p] = get_input_handle(p,matrixhandle[p],false);
  }

  // Reorder the TCL input and put them
  // in orderly STL style vectors.

  // First update the GUI to C++ interface
  get_gui()->execute(get_id()+" Synchronise");
  get_ctx()->reset();

  // Get the contents of the filename entrybox
  std::string filename = guifilename_.get();

  // Make sure we have a .mat extension
  int filenamesize = filename.size();
  if (filenamesize < 4) 
  {   
    filename += ".mat";
  }
  else
  {
    if (filename.substr(filenamesize-4,filenamesize) != ".mat") filename += ".mat";
  }
  
  // If the filename is empty, launch an error
  if (filename == "")
  {
    error("ExportDatatypesToMatlab: No file name was specified");
    return;
  }
	
	// Make sure the path to the new file exists
	// If not make it and as well convert filename
	// to absolute path name
	FullFileName ffn(filename);
	if (!(ffn.create_file_path()))
	{
		error("Could not generate path to file");
		return;
	}		
	filename = ffn.get_abs_filename();
	
	guifilename_.set(filename);
	get_ctx()->reset();					

  if (!overwrite()) return;

  // get all the settings from the GUI

  std::vector<std::string> matrixname;
  std::vector<std::string> dataformat;
  std::vector<std::string> matrixformat;

  matrixname = converttcllist(guimatrixname_.get());
  dataformat = converttcllist(guidataformat_.get());
  matrixformat = converttcllist(guimatrixformat_.get());

  // Check the validity of the matrixnames

  for (int p=0;p<static_cast<int>(matrixname.size());p++)
  {
    if (porthasdata[p] == false) continue; // Do not check not used ports
    if (!translate.isvalidmatrixname(matrixname[p]))
    {
      error("ExportNrrdsToMatlab: The matrix name specified is invalid");
      return;
    }
    for (int q=0;q<p;q++)
    {
      if (porthasdata[q] == false) continue;
      if (matrixname[q] == matrixname[p])
      {
        error("ExportNrrdsToMatlab: A matrix name is used twice");
        return;
      }
    }
  }
  
  
  try
  {
    matlabfile mfile;   // matlab file object contains all function for reading and writing matlab arrayd
    matlabarray ma;		// matlab style formatted array (can be stored in a matlabfile object)
    mfile.open(filename,"w");   // open file for writing

    // Add an information tag to the data, so the origin of the file is known
    // There are 124 bytes of free data storage at the header of the file.
    // Do not start the file with 'SCI ', otherwise the file looks like a
    // native SCIRun file which uses the same extension.

    mfile.setheadertext("Matlab V5 compatible file generated by SCIRun [module ExportNrrdsToMatlab version 1.1]");

    for (int p=0;p<NUMPORTS;p++)
    {
      if (porthasdata[p] == false) continue; // if the port is not connected skip to the next one
      
      // Convert the SCIRun matrixobject to a matlab object
      
      if (matrixformat[p] == "struct array")
      {   
        // translate the matrix into a matlab structured array, which
        // can also store some data from the property manager
        translate.converttostructmatrix();
        translate.setdatatype(convertdataformat(dataformat[p]));
      }
      
      if (matrixformat[p] == "numeric array")
      {
        // only store the numeric parts of the data
        translate.converttonumericmatrix();
        translate.setdatatype(convertdataformat(dataformat[p]));
      }
      translate.sciNrrdDataTOmlArray(matrixhandle[p],ma);
              
      if (ma.isempty())
      {
        warning("One of the matrices is empty");
        continue; // Do not write empty matrices
      }
      // Every thing seems OK, so proceed and store the matrix in the file
      mfile.putmatlabarray(ma,matrixname[p]);
    }
          
    mfile.close();
  }	

  // in case something went wrong
  catch (matlabconverter::matlabconverter_error)
  {
      error("ExportNrrdsToMatlab: Error in the SCIRun to Matlab converter");
  }              
  catch (matlabfile::could_not_open_file)
  {
    error("ExportNrrdsToMatlab: Could not open file");
  }
  catch (matlabfile::invalid_file_format)
  {
    error("ExportNrrdsToMatlab: Invalid file format");
  }
  catch (matlabfile::io_error)
  {   // IO error from ferror
    error("ExportNrrdsToMatlab: IO error");
  }
  catch (matlabfile::unknown_type)
  {
    error("ExportNrrdsToMatlab: Unknow type encountered");
  }
  catch (matlabfile::empty_matlabarray)
  {
    error("ExportNrrdsToMatlab: Empty Matlab array encountered");  
  }
  catch (matlabfile::out_of_range)
  {
    error("ExportNrrdsToMatlab: Out of Range error");
  }
  catch (matlabfile::invalid_file_access)
  {
    error("ExportNrrdsToMatlab: Invalid file access");
  }
  catch (matlabfile::compression_error)
  { 
    error("ExportNrrdsToMatlab: Compression error");
  }  catch (matlabfile::matfileerror) 
  {   // All other errors are classified as internal
    // matfileerrror is the base class on which all
    // other exceptions are based.
    error("ExportNrrdsToMatlab: Internal error in writer");
  }
  // No handling of the SCIRun errors here yet, most SCIRun functions used
  // do not use exceptions yet.
}

// Additional support functions :
// To help coordinate between the GUI in TCL and
// the functions in this module on the C++ site.
// Some of the following functions are TCL specific!

// convertdataformat
// Convert the string TCL returns into a matlabarray::mitype

matlabarray::mitype ExportNrrdsToMatlab::convertdataformat(std::string dataformat)
{
  matlabarray::mitype type = matlabarray::miUNKNOWN;
  if (dataformat == "same as data")  { type = matlabarray::miSAMEASDATA; }
  else if (dataformat == "double")   { type = matlabarray::miDOUBLE; }
  else if (dataformat == "single")   { type = matlabarray::miSINGLE; }
  else if (dataformat == "uint64")   { type = matlabarray::miUINT64; }
  else if (dataformat == "int64")    { type = matlabarray::miINT64; }
  else if (dataformat == "uint32")   { type = matlabarray::miUINT32; }
  else if (dataformat == "int32")    { type = matlabarray::miINT32; }
  else if (dataformat == "uint16")   { type = matlabarray::miUINT16; }
  else if (dataformat == "int16")    { type = matlabarray::miINT16; }
  else if (dataformat == "uint8")    { type = matlabarray::miUINT8; }
  else if (dataformat == "int8")     { type = matlabarray::miINT8; }
  return (type);
}

// converttcllist:
// converts a TCL formatted list into a STL array
// of strings

std::vector<std::string> ExportNrrdsToMatlab::converttcllist(std::string str)
{
  std::string result;
  std::vector<std::string> list(0);
  int lengthlist = 0;

  // Yeah, it is TCL dependent:
  // TCL::llength determines the length of the list
  get_gui()->lock();
  get_gui()->eval("llength { "+str + " }",result);	
  istringstream iss(result);
  iss >> lengthlist;
  get_gui()->unlock();
  if (lengthlist < 0) return(list);

  list.resize(lengthlist);
  get_gui()->lock();
  for (int p = 0;p<lengthlist;p++)
  {
    ostringstream oss;
    // TCL dependency:
    // TCL::lindex retrieves the p th element from the list
    oss << "lindex { " << str <<  " } " << p;
    get_gui()->eval(oss.str(),result);
    list[p] = result;
  }
  get_gui()->unlock();
  return(list);
}

// overwrite:
// Ask the user whether the file should be overwritten

bool ExportNrrdsToMatlab::overwrite()
{
  std::string result;
  get_gui()->lock();
  get_gui()->eval(get_id()+" overwrite",result);
  get_gui()->unlock();
  if (result == std::string("0")) 
  {
    warning("User chose to not save.");
    return(0);
  }
  return(1);
}

// displayerror:
// This function should be replaced with a more
// general function in SCIRun for displaying errors

void ExportNrrdsToMatlab::displayerror(std::string str)
{
  get_gui()->lock();
  // Explicit call to TCL
  get_gui()->execute("tk_messageBox -icon error -type ok -title {ERROR} -message {" + str + "}");
  get_gui()->unlock();
}


} // End namespace MatlabIO
