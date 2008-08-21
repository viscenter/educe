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

#include <Core/Algorithms/Fields/DistanceField/CalculateDistanceField.h>
#include <Core/Algorithms/Fields/MeshDerivatives/GetFieldBoundary.h>

#include <Core/Datatypes/FieldInformation.h>
#include <Core/Thread/Thread.h>

//! for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;


class CalculateDistanceFieldP {
  public:
    CalculateDistanceFieldP(VMesh* imesh, VMesh* objmesh, VField*  ofield) :
      imesh(imesh), objmesh(objmesh), ofield(ofield) {}

    CalculateDistanceFieldP(VMesh* imesh, VMesh* objmesh, VMesh* dobjmesh, VField*  ofield) :
      imesh(imesh), objmesh(objmesh), dobjmesh(dobjmesh), ofield(ofield) {}  
      
    void parallel(int proc, int nproc)
    {
      VMesh::size_type num_values = ofield->num_values();
      VMesh::size_type num_evalues = ofield->num_evalues();
    
      double val = 0.0;

      if (ofield->basis_order() == 0)
      {
        VMesh::Elem::index_type fidx;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_values);
        
        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          Point p, p2;
          imesh->get_center(p,idx);
          objmesh->find_closest_elem(val,p2,fidx,p);
          ofield->set_value(val,idx);
        }
      }
      else if (ofield->basis_order() > 0)
      {
        VMesh::Elem::index_type fidx;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_values);

        for (VMesh::Node::index_type idx=start; idx<end; idx++)
        {
          Point p, p2;
          imesh->get_center(p,idx);
          objmesh->find_closest_elem(val,p2,fidx,p);
          ofield->set_value(val,idx);
        }  
      }
      else if (ofield->basis_order() > 1)
      {
        VMesh::Elem::index_type fidx;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_evalues);

        for (VMesh::ENode::index_type idx=start; idx<end; idx++)
        {
          Point p, p2;
          imesh->get_center(p,idx);
          objmesh->find_closest_elem(val,p2,fidx,p);
          ofield->set_value(val,idx);
        }  
      }    
    }
  
    void parallel2(int proc, int nproc)
    {
      VMesh::size_type num_values = ofield->num_values();
      VMesh::size_type num_evalues = ofield->num_evalues();

      double val = 0.0;
      
      if (ofield->basis_order() == 0)
      {
        VMesh::Elem::index_type cidx, fidx;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_values);

        for (VMesh::Elem::index_type idx=start; idx<end; idx++)
        {
          Point p, p2;
          imesh->get_center(p,idx);
          
          if (objmesh->locate(cidx,p))
          {
            val = 0.0; // it is inside
          }
          else
          {
            dobjmesh->find_closest_elem(val,p2,fidx,p);
          }
          ofield->set_value(val,idx);
        }
      }
      else if (ofield->basis_order() > 0)
      {
        VMesh::Elem::index_type cidx, fidx;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_values);

        for (VMesh::Node::index_type idx=start; idx<end; idx++)
        {
          Point p, p2;
          imesh->get_center(p,idx);
          if (objmesh->locate(cidx,p))
          {
            val = 0.0; // it is inside
          }
          else
          {
            dobjmesh->find_closest_elem(val,p2,fidx,p);
         }
          ofield->set_value(val,idx);
        }
      }
      else if (ofield->basis_order() > 1)
      {
        VMesh::Elem::index_type cidx, fidx;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_evalues);
        
        for (VMesh::ENode::index_type idx=start; idx<end; idx++)
        {
          Point p, p2;
          imesh->get_center(p,idx);
          if (objmesh->locate(cidx,p))
          {
            val = 0.0; // it is inside
          }
          else
          {
            dobjmesh->find_closest_elem(val,p2,fidx,p);
         }
          ofield->set_evalue(val,idx);
        }    
      }
    }
  
    void range(int proc, int nproc,
               VMesh::index_type& start, VMesh::index_type& end,
               VMesh::size_type size)
    {
      VMesh::size_type m = size/nproc;
      start = proc*m;
      end = (proc+1)*m;
      if (proc == nproc-1) end = size;
    }
    
  private:
    VMesh*   imesh;
    VMesh*   objmesh;
    VMesh*   dobjmesh;
    VField*  ofield;
};


bool
CalculateDistanceFieldAlgo::
run(FieldHandle input, FieldHandle object, FieldHandle& output)
{
  algo_start("CalculateDistanceField",false);
  
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }

  if (object.get_rep() == 0)
  {
    error("No object field");
    algo_end(); return (false);
  }

  if (!(input->has_virtual_interface()))
  {
    error("The input field has no virtual interface");
    algo_end(); return (false);
  }

  if (!(object->has_virtual_interface()))
  {
    error("The object field has no virtual interface");
    algo_end(); return (false);
  }

  // Determine output type
  FieldInformation fo(input);
  if (fo.is_nodata()) fo.make_lineardata();
  fo.make_double();
  output = CreateField(fo,input->mesh());
  
  if (output.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return (false);
  }
  
  VMesh* imesh = input->vmesh();
  VMesh* objmesh = object->vmesh();
  VField* ofield = output->vfield();
  ofield->resize_values();
  
  //VMesh::size_type num_values = ofield->num_values();
  //VMesh::size_type num_evalues = ofield->num_evalues();


  if (objmesh->is_volume())
  {
    FieldHandle dobject;

    GetFieldBoundaryAlgo algo;
    algo.set_progress_reporter(get_progress_reporter());
    if(!(algo.run(object,dobject)))
    {
      error("Could not derive field boundary of the object field");
      algo_end(); return (false);
    }

    VMesh *dobjmesh = dobject->vmesh();
    
    objmesh->synchronize(Mesh::ELEM_LOCATE_E);
    dobjmesh->synchronize(Mesh::ELEM_LOCATE_E);

    if (ofield->basis_order() > 2)
    {
      error("Cannot add distance data to field");
      algo_end(); return (false);
    }

    CalculateDistanceFieldP palgo(imesh,objmesh,dobjmesh,ofield);
    Thread::parallel(&palgo,&CalculateDistanceFieldP::parallel2,Thread::numProcessors(),Thread::numProcessors());
  
    algo_end(); return (true);  
  }
  
  
  if (objmesh->is_surface() || objmesh->is_line() || objmesh->is_point())
  {
    objmesh->synchronize(Mesh::ELEM_LOCATE_E);

    if (ofield->basis_order() > 2)
    {
      error("Cannot add distance data to field");
      algo_end(); return (false);
    }

    CalculateDistanceFieldP palgo(imesh,objmesh,ofield);
    Thread::parallel(&palgo,&CalculateDistanceFieldP::parallel,Thread::numProcessors(),Thread::numProcessors());
  
    algo_end(); return (true);
  }
  
  algo_end(); return (false);
}


} // end namespace SCIRunAlgo
