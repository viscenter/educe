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
 * FILE: ImportDatatypesFromMatlab.cc
 * AUTH: Jeroen G Stinstra
 * DATE: 30 MAR 2004
 */
 
/* 
 * This module reads a matlab file and converts it to a SCIRun matrix
 *
 */


/* 
 * This file was adapted from mlMatricesReader.h 
 */

#include <sgi_stl_warnings_off.h>
#include <sstream>
#include <string>
#include <vector>
#include <sgi_stl_warnings_on.h>

#include <Dataflow/Network/Module.h>
#include <Core/Malloc/Allocator.h>
#include <Core/Datatypes/Field.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Ports/NrrdPort.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Core/Matlab/matlabfile.h>
#include <Core/Matlab/matlabarray.h>
#include <Core/Matlab/matlabconverter.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Core/Datatypes/String.h>
#include <Dataflow/Network/Ports/StringPort.h>

namespace MatlabIO {

using namespace SCIRun;

class ImportDatatypesFromMatlab : public Module 
{

  public:

    // Constructor
    ImportDatatypesFromMatlab(GuiContext*);

    // Destructor
    virtual ~ImportDatatypesFromMatlab();

    // Std functions for each module
    // execute():
    //   Execute the module and put data on the output port
    //
    // tcl_command():
    //   Handles call-backs from the TCL code
    
    virtual void execute();
    virtual void tcl_command(GuiArgs&, void*);

  private:
	
    // indexmatlabfile():
    //   This functions is used in the GUI interface, it loads the
    //   currently selected .mat file and returns an information
    //   string to TCL with all the names and formats of the matrices.
    //   NOTE: This Function explicitly depends on the TCL code.
    
    void		indexmatlabfile(bool postmsg);
    
    // readmatlabarray():
    //   This function reads the in the gui selected matlab array. It
    //   retrieves the current filename and the matrix selected in this
    //   file and returns an object containing the full matrix
    
    matlabarray readmatlabarray(int p);

    // displayerror()
    //   Relay an error during the matrixselection process directly to
    //   the user
    
    void displayerror(std::string str);

  private:

    enum { NUMPORTS = 9};
    
    // GUI variables
    GuiFilename 		guifilename_;		// .mat filename (import from GUI)
    GuiString       guifilenameset_;
    GuiString				guimatrixinfotextslist_;   	// A list of matrix-information strings of the contents of a .mat-file
    GuiString				guimatrixnameslist_;	// A list of matrix-names of the contents of a .mat-file 
    GuiString				guimatrixname_;		// the name of the matrix that has been selected

};

DECLARE_MAKER(ImportDatatypesFromMatlab)

// Constructor:
// Initialise all the variables shared between TCL and SCIRun
// Only filename and matrixname are used to reconstruct the 
// settings of a previously created module
// matrixinfotexts and matrixnames serve as outputs to TCL.

ImportDatatypesFromMatlab::ImportDatatypesFromMatlab(GuiContext* ctx)
  : Module("ImportDatatypesFromMatlab", ctx, Source, "DataIO", "MatlabInterface"),
    guifilename_(get_ctx()->subVar("filename")),
    guifilenameset_(get_ctx()->subVar("filename-set")),
    guimatrixinfotextslist_(get_ctx()->subVar("matrixinfotextslist")),     
    guimatrixnameslist_(get_ctx()->subVar("matrixnameslist")),    
    guimatrixname_(get_ctx()->subVar("matrixname"))
{
  indexmatlabfile(false);
}

// Destructor:
// All my objects have descrutors and hence nothing needs
// explicit descruction
ImportDatatypesFromMatlab::~ImportDatatypesFromMatlab()
{
}

// Execute:
// Inner workings of this module
void ImportDatatypesFromMatlab::execute()
{
	StringHandle stringH;
	get_input_handle("Filename",stringH,false);
	if (stringH.get_rep())
	{
		std::string filename = stringH->get();
		guifilename_.set(filename);
		get_ctx()->reset();
    indexmatlabfile(true);
		get_ctx()->reset();
	}

  // Get the filename from TCL.
  std::string filename = guifilename_.get();
  
  // If the filename is empty, launch an error
  if (filename == "")
  {
    error("ImportDatatypesFromMatlab: No file name was specified");
    return;
  }

  try
  {
    for (int p=0;p<3;p++)
    {
      matlabarray ma = readmatlabarray(p);
      if (ma.isempty())
      {
        continue;
      }

      SCIRun::FieldHandle mh;

      matlabconverter translate(dynamic_cast<SCIRun::ProgressReporter*>(this));
      translate.mlArrayTOsciField(ma,mh);
			send_output_handle(p,mh,true);
    }

    for (int p=0;p<3;p++)
    {
      matlabarray ma = readmatlabarray(p+3);
      if (ma.isempty())
      {
        continue;
      }

      SCIRun::MatrixHandle mh;
      matlabconverter translate(dynamic_cast<SCIRun::ProgressReporter*>(this));
      translate.mlArrayTOsciMatrix(ma,mh);
			
			send_output_handle(p+3,mh,true);
    }

    for (int p=0;p<3;p++)
    {
      matlabarray ma = readmatlabarray(p+6);
      if (ma.isempty())
      {
        continue;
      }

      SCIRun::NrrdDataHandle mh;
      matlabconverter translate(dynamic_cast<SCIRun::ProgressReporter*>(this));
      translate.mlArrayTOsciNrrdData(ma,mh);
		
			send_output_handle(p+6,mh,true);
    }
    
    SCIRun::StringHandle filenameH = scinew String(filename);
    send_output_handle("Filename",filenameH,true);    
  }

  // in case something went wrong
          
  catch (matlabfile::could_not_open_file)
  {
    error("ImportDatatypesFromMatlab: Could not open file");
  }
  catch (matlabfile::invalid_file_format)
  {
    error("ImportDatatypesFromMatlab: Invalid file format");
  }
  catch (matlabfile::io_error)
  {
    error("ImportDatatypesFromMatlab: IO error");
  }
  catch (matlabfile::out_of_range)
  {
    error("ImportDatatypesFromMatlab: Out of range");
  }
  catch (matlabfile::invalid_file_access)
  {
    error("ImportDatatypesFromMatlab: Invalid file access");
  }
  catch (matlabfile::empty_matlabarray)
  {
    error("ImportDatatypesFromMatlab: Empty matlab array");
  }
  catch (matlabfile::matfileerror) 
  {
    error("ImportDatatypesFromMatlab: Internal error in reader");
  }
}


void ImportDatatypesFromMatlab::tcl_command(GuiArgs& args, void* userdata)
{
  if(args.count() < 2)
  {
    args.error("ImportDatatypesFromMatlab needs a minor command");
    return;
  }

  if( args[1] == "indexmatlabfile" )
  {
    
    // It turns out that in the current design, SCIRun reads variables once
    // and then assumes they do not change and hence caches the data
    // Why it is done so is unclear to me, but in order to have interactive
    // GUIs I need to reset the context. (this synchronises the data again)
    get_ctx()->reset();
    
    // Find out what the .mat file contains
    indexmatlabfile(true);
    return;
  }
  else 
  {
    // Relay data to the Module class
    Module::tcl_command(args, userdata);
  }
}


matlabarray ImportDatatypesFromMatlab::readmatlabarray(int p)
{
  matlabarray marray;
  std::string filename = guifilename_.get();
  std::string guimatrixname = guimatrixname_.get();
  std::string matrixname = "";
  
  // guimatrixname is a list with the name of the matrices per port
  // use the TCL command lindex to select the proper string from the list
  
  std::ostringstream oss;
  oss << "lindex {" << guimatrixname << "} " << p;
  
  get_gui()->lock();
  get_gui()->eval(oss.str(),matrixname);
  get_gui()->unlock();
  
  if (matrixname == "")
  {
    // return an empty array
    return(marray);
  }
  
  if (matrixname == "<none>")
  {
    // return an empty array
    return(marray);
  }
  
  // this block contains the file IO
  // The change of errors is reasonable
  // hence errors are generated as exceptions
          
  // having a local matfile object here ensures
  // the file will be closed (destructor of the object).
  
  matlabfile  mfile;
  mfile.open(filename,"r");
  marray = mfile.getmatlabarray(matrixname);
  mfile.close();

  return(marray);
}



void ImportDatatypesFromMatlab::indexmatlabfile(bool postmsg)
{
	
  std::string filename = "";
  std::string matrixinfotexts[NUMPORTS];
  std::string matrixnames[NUMPORTS];
  std::string matrixinfotextslist = "";
  std::string matrixnameslist = "";
  std::string newmatrixname = "";
  std::string matrixname = "";
  
  guimatrixinfotextslist_.set(matrixinfotextslist);
  guimatrixnameslist_.set(matrixnameslist);
  
  SCIRun::ProgressReporter* pr = 0;
  if (postmsg) pr = dynamic_cast<SCIRun::ProgressReporter* >(this);
  matlabconverter translate(pr);
  
  filename = guifilename_.get();	

  if (filename == "") 
  {
          // No file has been loaded, so reset the
          // matrix name variable
          guimatrixname_.set(newmatrixname);
          return;
  }
  
  matrixname = guimatrixname_.get();
  
  std::vector<std::string> matrixnamelist(NUMPORTS);
  bool foundmatrixname[NUMPORTS];
  
  for (int p=0;p<NUMPORTS;p++)
  {
    matrixinfotexts[p] = "{ ";
    matrixnames[p] = "{ ";
    // TCL Dependent code
    std::ostringstream oss;
    oss << "lindex { " << matrixname << " } " << p;
    get_gui()->lock();
    get_gui()->eval(oss.str(),matrixnamelist[p]);
    get_gui()->unlock();
    foundmatrixname[p] = false;
  }

  try
  {
    matlabfile mfile;
    // Open the .mat file
    // This function also scans through the file and makes
    // sure it is amat file and counts the number of arrays
    
    mfile.open(filename,"r");
    
    // all matlab data is stored in a matlabarray object
    matlabarray ma;
    int cindex = 0;		// compability index, which matlab array fits the SCIRun matrix best? 
    int maxindex = 0;		// highest index found so far
            
    // Scan the file and see which matrices are compatible
    // Only those will be shown (you cannot select incompatible matrices).
            
    std::string infotext;
    
    for (int p=0;p<mfile.getnummatlabarrays();p++)
    {
      ma = mfile.getmatlabarrayinfo(p); // do not load all the data fields
      for (int q=0;q<NUMPORTS;q++)
      {
        if ((q==0)||(q==1)||(q==2)) cindex = translate.sciFieldCompatible(ma,infotext);
        if ((q==3)||(q==4)||(q==5)) cindex = translate.sciMatrixCompatible(ma,infotext);
        if ((q==6)||(q==7)||(q==8)) cindex = translate.sciNrrdDataCompatible(ma,infotext);
        
        if (cindex)
        {
          // in case we need to propose a matrix to load, select
          // the one that is most compatible with the data
          if (cindex > maxindex) { maxindex = cindex; newmatrixname = ma.getname();}
  
          // create tcl style list to use in the array selection process
  
          matrixinfotexts[q] += std::string("{" + infotext + "} ");
          matrixnames[q] += std::string("{" + ma.getname() + "} "); 
          if (ma.getname() == matrixnamelist[q]) foundmatrixname[q] = true;
        }
      }
    }
    
    
    for (int q=0;q<NUMPORTS;q++)
    {
      matrixinfotexts[q] += "{none} } ";
      matrixnames[q] += "{<none>} } ";
    }
    
    mfile.close();

    // automatically select a matrix if the current matrix name
    // cannot be found or if no matrixname has been specified
    
    matrixname = "";
    for (int p=0;p<NUMPORTS;p++)
    {
      if (foundmatrixname[p] == false) 
      {   
        if (p==0) 
        {
          matrixnamelist[p] = newmatrixname;
        }
        else
        {
          matrixnamelist[p] = "<none>";
        }
      }
      matrixname += "{" + matrixnamelist[p] + "} ";
    }
    
    for (int q=0;q<NUMPORTS;q++)
    {
      matrixinfotextslist += matrixinfotexts[q];
      matrixnameslist += matrixnames[q];
    }
    
    
    // Update TCL on the contents of this matrix
    guimatrixname_.set(matrixname);
    guimatrixinfotextslist_.set(matrixinfotextslist);
    guimatrixnameslist_.set(matrixnameslist);
  }
  
  // in case something went wrong
  // close the file and then dermine the problem

  catch (matlabfile::could_not_open_file)
  {
    displayerror("ImportDatatypesFromMatlab: Could not open file");
  }
  catch (matlabfile::invalid_file_format)
  {
    displayerror("ImportDatatypesFromMatlab: Invalid file format");
  }
  catch (matlabfile::io_error)
  {
    displayerror("ImportDatatypesFromMatlab: IO error");
  }
  catch (matlabfile::out_of_range)
  {
    displayerror("ImportDatatypesFromMatlab: Out of range");
  }
  catch (matlabfile::invalid_file_access)
  {
    displayerror("ImportDatatypesFromMatlab: Invalid file access");
  }
  catch (matlabfile::empty_matlabarray)
  {
    displayerror("ImportDatatypesFromMatlab: Empty matlab array");
  }
  catch (matlabfile::matfileerror) 
  {
    displayerror("ImportDatatypesFromMatlab: Internal error in reader");
  }
  return;
}


void ImportDatatypesFromMatlab::displayerror(std::string str)
{
  get_gui()->lock();
  // Explicit call to TCL
  get_gui()->execute("tk_messageBox -icon error -type ok -title {ERROR} -message {" + str + "}");
  get_gui()->unlock();
}


} // End namespace MatlabIO
