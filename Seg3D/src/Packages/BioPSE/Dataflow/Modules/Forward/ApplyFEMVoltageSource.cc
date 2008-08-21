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
 *  ApplyFEMVoltageSource.cc:  Builds the RHS of the FE matrix for voltage sources
 *
 *  Written by:
 *   David Weinstein
 *   University of Utah
 *   May 1999
 *  Modified by:
 *   Alexei Samsonov, March 2001
 *   Frank B. Sachse, February 2006
 *  
 */

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/Network/Module.h>

#include <Core/Containers/StringUtil.h>

#include <vector>

namespace BioPSE {

using namespace SCIRun;

class ApplyFEMVoltageSource : public Module
{
  private:
    GuiString bcFlag_; // "none", "GroundZero", or "DirSub"

  public:
    ApplyFEMVoltageSource(GuiContext *context);
    virtual ~ApplyFEMVoltageSource() {}

    //! Public methods
    virtual void execute();
};


DECLARE_MAKER(ApplyFEMVoltageSource)

ApplyFEMVoltageSource::ApplyFEMVoltageSource(GuiContext *context) : 
  Module("ApplyFEMVoltageSource", context, Filter, "Forward", "BioPSE"),
      bcFlag_(context->subVar("bcFlag"))
{
}

void
ApplyFEMVoltageSource::execute()
{
  //! Obtaining handles to computation objects
  FieldHandle hField;
  if (!get_input_handle("Mesh", hField)) return;
  
  std::vector<std::pair<int, double> > dirBC;
  if (bcFlag_.get() == "DirSub") 
    if (!hField->get_property("dirichlet", dirBC))
      warning("The input field doesn't contain Dirichlet boundary conditions.");
  MatrixHandle hMatIn;
  if (!get_input_handle("Stiffness Matrix", hMatIn)) return;

  SparseRowMatrix *matIn;
  if (!(matIn = dynamic_cast<SparseRowMatrix*>(hMatIn.get_rep()))) 
  {
    error("Input stiffness matrix wasn't sparse.");
    return;
  }
  if (matIn->nrows() != matIn->ncols()) 
  {
    error("Input stiffness matrix wasn't square.");
    return;
  }
  
  SparseRowMatrix *mat = matIn->clone();
  
  unsigned int nsize=matIn->ncols();
  ColumnMatrix* rhs = new ColumnMatrix(nsize);
  
  MatrixHandle  hRhsIn;
  ColumnMatrix* rhsIn = NULL;
  
  // -- if the user passed in a vector the right size, copy it into ours 
  if (get_input_handle("RHS", hRhsIn, false) && 
      hRhsIn.get_rep())
  {
    hRhsIn = rhsIn->column();
    rhsIn = hRhsIn->column();
    if (rhsIn &&
      ((unsigned int)(rhsIn->nrows()) == nsize))
    {
      for (unsigned int i=0; i < nsize; i++) 
        (*rhs)[i]=(*rhsIn)[i];
    }
    else
    {
      rhs->zero();    
    }
  }
  else
  {
    rhs->zero();
  }
  
  std::string bcFlag = bcFlag_.get();
    
  if (bcFlag=="GroundZero") dirBC.push_back(std::pair<int, double>(0,0.0));
  else if (bcFlag == "DirSub") hField->vfield()->get_property("dirichlet", dirBC);

  //! adjusting matrix for Dirichlet BC
  Matrix::index_type *idcNz; 
  double *valNz;
  Matrix::index_type idcNzsize;
  Matrix::index_type idcNzstride;
    
  vector<double> dbc;
  Matrix::index_type idx;
  for(idx = 0; idx<dirBC.size(); ++idx)
  {
    Matrix::index_type ni = dirBC[idx].first;
    double val = dirBC[idx].second;
  
    // -- getting column indices of non-zero elements for the current row
    mat->getRowNonzerosNoCopy(ni, idcNzsize, idcNzstride, idcNz, valNz);
  
    // -- updating rhs
    for (Matrix::index_type i=0; i<idcNzsize; ++i)
    {
      Matrix::index_type j = idcNz?idcNz[i*idcNzstride]:i;
      (*rhs)[j] += - val * valNz[i*idcNzstride]; 
    }
  }
 
  //! zeroing matrix row and column corresponding to the dirichlet nodes
  for(idx = 0; idx<dirBC.size(); ++idx)
  {
    Matrix::index_type ni = dirBC[idx].first;
    double val = dirBC[idx].second;
  
    mat->getRowNonzerosNoCopy(ni, idcNzsize, idcNzstride, idcNz, valNz);
    
    for (Matrix::index_type i=0; i<idcNzsize; ++i)
    {
      Matrix::index_type j = idcNz?idcNz[i*idcNzstride]:i;
      mat->put(ni, j, 0.0);
      mat->put(j, ni, 0.0); 
    }
    
    //! updating dirichlet node and corresponding entry in rhs
    mat->put(ni, ni, 1);
    (*rhs)[ni] = val;
  }

  //! Sending result
  MatrixHandle mat_tmp(mat);
  send_output_handle("Forward Matrix", mat_tmp);

  MatrixHandle rhs_tmp(rhs);
  send_output_handle("RHS", rhs_tmp);
}

} // End namespace BioPSE
