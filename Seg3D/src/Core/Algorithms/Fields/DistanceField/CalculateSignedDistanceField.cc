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

#include <Core/Algorithms/Fields/DistanceField/CalculateSignedDistanceField.h>

#include <Core/Datatypes/FieldInformation.h>
#include <Core/Thread/Thread.h>
#include <float.h>

//! for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;


class CalculateSignedDistanceFieldP {
  public:
    CalculateSignedDistanceFieldP(VMesh* imesh, VMesh* objmesh, VField*  ofield) :
      imesh(imesh), objmesh(objmesh), ofield(ofield) {}
      
    void parallel(int proc, int nproc)
    {
      VMesh::size_type num_values = ofield->num_values();
      VMesh::size_type num_evalues = ofield->num_evalues();

      double val = 0.0;
      double epsilon = objmesh->get_epsilon();

      if (ofield->basis_order() == 0)
      {
        VMesh::Elem::index_type fidx, fidx_n;
        VMesh::Node::array_type nodes;
        VMesh::DElem::array_type delems;
        Vector n, k;
        Point n0,n1,n2;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_values);
        
        for (VMesh::Elem::index_type idx = start; idx < end; idx++)
        {
          Point p, p1, p2;
          imesh->get_center(p,idx);

          objmesh->find_closest_elem(val,p2,fidx,p);
          objmesh->get_nodes(nodes,fidx);  
          objmesh->get_center(n0,nodes[0]);
          objmesh->get_center(n1,nodes[1]);
          objmesh->get_center(n2,nodes[2]);

          n = Cross(Vector(n1-n0),Vector(n2-n1));
          k = Vector(p-p2); k.normalize();
          
          double angle = Dot(n,k);
          if (angle < epsilon)
          {
            val = -val;
          }
          else if (angle > epsilon)
          {
          }
          else
          {
            // trouble
            if (val != 0.0)
            {
               objmesh->get_delems(delems,fidx);
               double mindist = DBL_MAX;
               double dist;
               int edgeidx = 0;
               for (size_t r=0; r<delems.size();r++)
               {
                 objmesh->get_nodes(nodes,delems[r]);
                 objmesh->get_center(p1,nodes[0]);
                 objmesh->get_center(p2,nodes[1]);

                if (Dot(Vector(p-p2),Vector(p2-p1)) >= 0.0)
                {
                  Vector v = Vector(p-p2);
                  dist  = Dot(v,v);
                }
                else if (Dot(Vector(p-p1),Vector(p1-p2)) >= 0.0) 
                {
                  Vector v = Vector(p-p1);
                  dist = Dot(v,v);
                }
                else
                {
                  Vector v1 = Vector(p1-p2);
                  Vector v = Vector(p-p2)-v1*(Dot(Vector(p-p2),v1)/Dot(v1,v1));
                  dist = Dot(v,v);
                }
                
                if (dist < mindist) { mindist = dist; edgeidx = r;}
              }
              objmesh->get_neighbor(fidx_n,fidx,delems[edgeidx]);
              objmesh->get_nodes(nodes,fidx);  
              objmesh->get_center(n0,nodes[0]);
              objmesh->get_center(n1,nodes[1]);
              objmesh->get_center(n2,nodes[2]);
              n = Cross(Vector(n1-n0),Vector(n2-n1));
              k = Vector(p-p2);
              k.normalize();
              angle = Dot(n,k);
              if (angle < 0) val = -(val);
            }
          }
          ofield->set_value(val,idx);
        }
      }
      else if (ofield->basis_order() > 0)
      {
        VMesh::Elem::index_type fidx, fidx_n;
        VMesh::Node::array_type nodes;
        VMesh::DElem::array_type delems;
        
        Vector n, k;
        Point n0,n1,n2;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_values);
        
        for (VMesh::Node::index_type idx =start; idx <end; idx++)
        {
          Point p, p1, p2;
          imesh->get_center(p,idx);
          objmesh->find_closest_elem(val,p2,fidx,p);

          objmesh->get_nodes(nodes,fidx);  
          objmesh->get_center(n0,nodes[0]);
          objmesh->get_center(n1,nodes[1]);
          objmesh->get_center(n2,nodes[2]);
          n = Cross(Vector(n1-n0),Vector(n2-n1));
          k = Vector(p-p2);
          k.normalize();
          double angle = Dot(n,k);
          if (angle < epsilon)
          {
            val = -val;
          }
          else if (angle > epsilon)
          {
          }
          else
          {
            // trouble
            if (val != 0.0)
            {
               objmesh->get_delems(delems,fidx);
               double mindist = DBL_MAX;
               double dist;
               int edgeidx = 0;
               for (size_t r=0; r<delems.size();r++)
               {
                 objmesh->get_nodes(nodes,delems[r]);
                 objmesh->get_center(p1,nodes[0]);
                 objmesh->get_center(p2,nodes[1]);

                if (Dot(Vector(p-p2),Vector(p2-p1)) >= 0.0)
                {
                  Vector v = Vector(p-p2);
                  dist  = Dot(v,v);
                }
                else if (Dot(Vector(p-p1),Vector(p1-p2)) >= 0.0) 
                {
                  Vector v = Vector(p-p1);
                  dist = Dot(v,v);
                }
                else
                {
                  Vector v1 = Vector(p1-p2);
                  Vector v = Vector(p-p2)-v1*(Dot(Vector(p-p2),v1)/Dot(v1,v1));
                  dist = Dot(v,v);
                }
                
                if (dist < mindist) { mindist = dist; edgeidx = r;}
              }
              objmesh->get_neighbor(fidx_n,fidx,delems[edgeidx]);
              objmesh->get_nodes(nodes,fidx);  
              objmesh->get_center(n0,nodes[0]);
              objmesh->get_center(n1,nodes[1]);
              objmesh->get_center(n2,nodes[2]);
              n = Cross(Vector(n1-n0),Vector(n2-n1));
              k = Vector(p-p2);
              k.normalize();
              angle = Dot(n,k);
              if (angle < 0.0) val = -(val);
            }
          }
          ofield->set_value(val,idx);
        }
      }
      else if (ofield->basis_order() > 1)
      {
        VMesh::Elem::index_type fidx, fidx_n;
        VMesh::Node::array_type nodes;
        VMesh::DElem::array_type delems;
        
        Vector n, k;
        Point n0,n1,n2;
        VMesh::index_type start, end;
        range(proc,nproc,start,end,num_evalues);
        
        for (VMesh::ENode::index_type idx=start; idx < end; idx++)
        {
          Point p, p1, p2;
          imesh->get_center(p,idx);
          objmesh->find_closest_elem(val,p2,fidx,p);

          objmesh->get_nodes(nodes,fidx);  
          objmesh->get_center(n0,nodes[0]);
          objmesh->get_center(n1,nodes[1]);
          objmesh->get_center(n2,nodes[2]);
          n = Cross(Vector(n1-n0),Vector(n2-n1));
          k = Vector(p-p2);
          k.normalize();
          double angle = Dot(n,k);
          if (angle < epsilon)
          {
            val = -val;
          }
          else if (angle > epsilon)
          {
          }
          else
          {
            // trouble
            if (val != 0.0)
            {
               objmesh->get_delems(delems,fidx);
               double mindist = DBL_MAX;
               double dist;
               int edgeidx = 0;
               for (size_t r=0; r<delems.size();r++)
               {
                 objmesh->get_nodes(nodes,delems[r]);
                 objmesh->get_center(p1,nodes[0]);
                 objmesh->get_center(p2,nodes[1]);

                if (Dot(Vector(p-p2),Vector(p2-p1)) >= 0.0)
                {
                  Vector v = Vector(p-p2);
                  dist  = Dot(v,v);
                }
                else if (Dot(Vector(p-p1),Vector(p1-p2)) >= 0.0) 
                {
                  Vector v = Vector(p-p1);
                  dist = Dot(v,v);
                }
                else
                {
                  Vector v1 = Vector(p1-p2);
                  Vector v = Vector(p-p2)-v1*(Dot(Vector(p-p2),v1)/Dot(v1,v1));
                  dist = Dot(v,v);
                }
                
                if (dist < mindist) { mindist = dist; edgeidx = r;}
              }
              objmesh->get_neighbor(fidx_n,fidx,delems[edgeidx]);
              objmesh->get_nodes(nodes,fidx);  
              objmesh->get_center(n0,nodes[0]);
              objmesh->get_center(n1,nodes[1]);
              objmesh->get_center(n2,nodes[2]);
              n = Cross(Vector(n1-n0),Vector(n2-n1));
              k = Vector(p-p2);
              k.normalize();
              angle = Dot(n,k);
              if (angle < 0.0) val = -(val);
            }
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
    VField*  ofield;
};





bool
CalculateSignedDistanceFieldAlgo::
run(FieldHandle input, FieldHandle object, FieldHandle& output)
{
  algo_start("CalculateDistanceField",true);
  
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

  if (!(object->vmesh()->is_surface()))
  {
    error("The object field needs to be surface");
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
 
  if (ofield->basis_order() > 2)
  {
    algo_end(); error("Cannot add distance data to field");
    return (false);
  }
 
  objmesh->synchronize(Mesh::ELEM_LOCATE_E);

  CalculateSignedDistanceFieldP palgo(imesh,objmesh,ofield);
  Thread::parallel(&palgo,&CalculateSignedDistanceFieldP::parallel,Thread::numProcessors(),Thread::numProcessors());

  algo_end(); return (true);
}

} // end namespace SCIRunAlgo
