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
 *  CreateLatVol.cc:  Make an ImageField that fits the source field.
 *
 *  Written by:
 *   Michael Callahan
 *   Department of Computer Science
 *   University of Utah
 *   March 2001
 *
 *  Copyright (C) 2001 SCI Group
 */

#include <Core/Geometry/BBox.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Tensor.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Core/Basis/NoData.h>
#include <Core/Basis/Constant.h>
#include <Core/Basis/HexTrilinearLgn.h>
#include <Core/Datatypes/LatVolMesh.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Containers/FData.h>
#include <Core/Datatypes/GenericField.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Core/Containers/StringUtil.h>
#include <Core/Algorithms/Util/FieldInformation.h>
#include <iostream>

namespace SCIRun {

class CreateLatVol : public Module
{
public:
  typedef LatVolMesh<HexTrilinearLgn<Point> > LVMesh;
  typedef NoDataBasis<Tensor>             NDTBasis;
  typedef NoDataBasis<Vector>             NDVBasis;
  typedef NoDataBasis<double>             NDDBasis;
  typedef ConstantBasis<Tensor>             CBTBasis;
  typedef ConstantBasis<Vector>             CBVBasis;
  typedef ConstantBasis<double>             CBDBasis;
  typedef HexTrilinearLgn<Tensor>             LBTBasis;
  typedef HexTrilinearLgn<Vector>             LBVBasis;
  typedef HexTrilinearLgn<double>             LBDBasis;
  typedef GenericField<LVMesh, NDTBasis,  
		       FData3d<Tensor, LVMesh> > LVFieldNDT;
  typedef GenericField<LVMesh, NDVBasis,  
		       FData3d<Vector, LVMesh> > LVFieldNDV;
  typedef GenericField<LVMesh, NDDBasis,  
		       FData3d<double, LVMesh> > LVFieldNDD;
  typedef GenericField<LVMesh, CBTBasis,  
		       FData3d<Tensor, LVMesh> > LVFieldCBT;
  typedef GenericField<LVMesh, CBVBasis,  
		       FData3d<Vector, LVMesh> > LVFieldCBV;
  typedef GenericField<LVMesh, CBDBasis,  
		       FData3d<double, LVMesh> > LVFieldCBD;
  typedef GenericField<LVMesh, LBTBasis,  
		       FData3d<Tensor, LVMesh> > LVFieldT;
  typedef GenericField<LVMesh, LBVBasis,  
		       FData3d<Vector, LVMesh> > LVFieldV;
  typedef GenericField<LVMesh, LBDBasis,  
		       FData3d<double, LVMesh> > LVField;

  CreateLatVol(GuiContext* ctx);
  virtual ~CreateLatVol();

  virtual void execute();

private:
  GuiInt size_x_;
  GuiInt size_y_;
  GuiInt size_z_;
  GuiDouble padpercent_;
  GuiString data_at_;
	GuiString element_size_;

  enum DataTypeEnum { SCALAR, VECTOR, TENSOR };
};


DECLARE_MAKER(CreateLatVol)

CreateLatVol::CreateLatVol(GuiContext* ctx)
  : Module("CreateLatVol", ctx, Filter, "NewField", "SCIRun"),
    size_x_(get_ctx()->subVar("sizex"), 16),
    size_y_(get_ctx()->subVar("sizey"), 16),
    size_z_(get_ctx()->subVar("sizez"), 16),
    padpercent_(get_ctx()->subVar("padpercent"), 0.0),
    data_at_(get_ctx()->subVar("data-at"), "Nodes"),
    element_size_(get_ctx()->subVar("element-size"),"Mesh")
{
}



CreateLatVol::~CreateLatVol()
{
}


void
CreateLatVol::execute()
{
  FieldHandle   ifieldhandle;
  MatrixHandle  Size;
	
  get_input_handle("Input Field", ifieldhandle, false);
  get_input_handle("LatVol Size",Size,false);
	
  if (inputs_changed_ || size_x_.changed() || size_y_.changed() ||
      size_z_.changed() || padpercent_.changed() || data_at_.changed() ||
      element_size_.changed() || !oport_cached("Output Sample Field") )
  {
    Point minb, maxb;
    DataTypeEnum datatype;

    if (Size.get_rep())
    {
      if (Size->get_data_size() == 1)
      {
        double* data = Size->get_data_pointer();
        unsigned int size = static_cast<unsigned int>(data[0]);
        size_x_.set(size);
        size_y_.set(size);
        size_z_.set(size);
        get_ctx()->reset();
      }
      else if (Size->get_data_size() == 3)
      {
        double* data = Size->get_data_pointer();
        unsigned int size1 = static_cast<unsigned int>(data[0]);
        unsigned int size2 = static_cast<unsigned int>(data[1]);
        unsigned int size3 = static_cast<unsigned int>(data[2]);
        size_x_.set(size1);
        size_y_.set(size2);
        size_z_.set(size3);		
        get_ctx()->reset();		
      }
      else
      {
        error("LatVol size matrix needs to have or one element or three elements");
      }	
    }	
		
    // Create blank mesh.
    Field::size_type sizex = Max(2, size_x_.get());
    Field::size_type sizey = Max(2, size_y_.get());
    Field::size_type sizez = Max(2, size_z_.get());		
		
    if (ifieldhandle.get_rep() == 0)
    {
      datatype = SCALAR;
      if (element_size_.get() == "Mesh")
      {
        minb = Point(-1.0, -1.0, -1.0);
        maxb = Point(1.0, 1.0, 1.0);
      }
      else
      {
        minb = Point(0.0,0.0,0.0);
        maxb = Point(static_cast<double>(sizex-1),
                     static_cast<double>(sizey-1),
                     static_cast<double>(sizez-1));
      }
    }
    else
    {
      datatype = SCALAR;
      FieldInformation fi(ifieldhandle);
      if (fi.is_vector())
      {
        datatype = VECTOR;
      }
      else if (fi.is_tensor())
      {
        datatype = TENSOR;
      }
      BBox bbox = ifieldhandle->mesh()->get_bounding_box();
      minb = bbox.min();
      maxb = bbox.max();
    }

    Vector diag((maxb.asVector() - minb.asVector()) * (padpercent_.get()/100.0));
    minb -= diag;
    maxb += diag;

    LVMesh::handle_type mesh = scinew LVMesh(sizex, sizey, sizez, minb, maxb);

    int basis_order;
    if (data_at_.get() == "Nodes") basis_order = 1;
    else if (data_at_.get() == "Cells") basis_order = 0;
    else if (data_at_.get() == "None") basis_order = -1;
    else {
      error("Unsupported data_at location " + data_at_.get() + ".");
      return;
    }

    // Create Image Field.
    FieldHandle ofh;
    if (datatype == SCALAR)
    {
      if (basis_order == -1) 
      {
        LVFieldNDD *lvf = scinew LVFieldNDD(mesh);
        ofh = lvf;
      } 
      else if (basis_order == 0) 
      {
        LVFieldCBD *lvf = scinew LVFieldCBD(mesh);
        LVFieldCBD::fdata_type::iterator itr = lvf->fdata().begin();
        while (itr != lvf->fdata().end())
        {
          *itr = 0.0;
          ++itr;
        }   
        ofh = lvf;
      } 
      else 
      {
        LVField *lvf = scinew LVField(mesh);
        LVField::fdata_type::iterator itr = lvf->fdata().begin();
        while (itr != lvf->fdata().end())
        {
          *itr = 0.0;
          ++itr;
        }   
        ofh = lvf;
      }

    } 
    else if (datatype == VECTOR)
    {
      if (basis_order == -1) 
      {
        LVFieldNDV *lvf = scinew LVFieldNDV(mesh);
        ofh = lvf;
      } 
      else if (basis_order == 0) 
      {
        LVFieldCBV *lvf = scinew LVFieldCBV(mesh);
        LVFieldCBV::fdata_type::iterator itr = lvf->fdata().begin();
        while (itr != lvf->fdata().end())
        {
          *itr = Vector(0.0, 0.0, 0.0);
          ++itr;
        }   
        ofh = lvf;
      } 
      else 
      {
        LVFieldV *lvf = scinew LVFieldV(mesh);
        LVFieldV::fdata_type::iterator itr = lvf->fdata().begin();
        while (itr != lvf->fdata().end())
        {
          *itr = Vector(0.0, 0.0, 0.0);
          ++itr;
        }   
        ofh = lvf;
      }
    }				    
    else // if (datatype == TENSOR)	    
    {	
      if (basis_order == -1) 
      {
        LVFieldNDT *lvf = scinew LVFieldNDT(mesh);
        ofh = lvf;
      } 
      else if (basis_order == 0) 
      {
        LVFieldCBT *lvf = scinew LVFieldCBT(mesh);
        LVFieldCBT::fdata_type::iterator itr = lvf->fdata().begin();
        while (itr != lvf->fdata().end())
        {
          *itr = Tensor(0.0);
          ++itr;
        }   
        ofh = lvf;
      } 
      else 
      {
        LVFieldT *lvf = scinew LVFieldT(mesh);
        LVFieldT::fdata_type::iterator itr = lvf->fdata().begin();
        while (itr != lvf->fdata().end())
        {
          *itr = Tensor(0.0);
          ++itr;
        }   
        ofh = lvf;
      }
    }				    

    send_output_handle("Output Sample Field", ofh);
  }
}


} // End namespace SCIRun

