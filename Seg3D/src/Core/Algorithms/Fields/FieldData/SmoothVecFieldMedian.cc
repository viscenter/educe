

#include <Core/Algorithms/Fields/FieldData/SmoothVecFieldMedian.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Util/ProgressReporter.h>
#include <Core/Geometry/Vector.h>
#include <Core/Math/MiscMath.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>

namespace SCIRunAlgo {

using namespace SCIRun;

bool
SmoothVecFieldMedianAlgo::run(FieldHandle input, FieldHandle& output)
{
  algo_start("SmoothVecFieldMedian",false);
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }
  
  FieldInformation fi(input);

  if (fi.is_nodata())
  {
    error("Input field does not have data associated with it");
    algo_end(); return (false);    
  }

  if (!(fi.is_vector()))
  {
    error("The data needs to be of vector type");
    algo_end(); return (false);    
  }

  output = CreateField(fi,input->mesh());

  if (output.get_rep() == 0)
  {
    error("Could not allocate output field");
    algo_end(); return (false);      
  }

  VField* ifield = input->vfield();
  VField* ofield = output->vfield();
  VMesh* imesh = input->vmesh();
  VMesh* omesh = output->vmesh();
  VMesh::Elem::index_type a,b;
  Vector v0,v1,v2,v3;  
  vector<double> angles, original;
  int middle=0, not_on_list=0, myloc=0, I=0, idx_cnt=0;
  double nv=0, gdot=0, m1=0, m2=0, angle=0;
   
  
  

  if (ifield->is_vector())
  {
    VField::size_type num_values = ifield->num_values();
  
    imesh->synchronize(Mesh::DELEMS_E|Mesh::ELEM_NEIGHBORS_E);

   
    
    //for (VField::index_type idx = 0; idx < 2000; idx++)
    for (VField::index_type idx = 0; idx < num_values; idx++)
    {
      //calculate neighborhoods
      VMesh::Elem::array_type nci, ncitot, Nlist;
      
      imesh->get_neighbors(nci, idx);
      
      ncitot.push_back(idx);
      
      for (size_t t=0; t< nci.size(); t++)
      {
           VMesh::Elem::array_type nci2;

           ncitot.push_back(nci[t]);
           imesh->get_neighbors(nci2, nci[t]);
           for (size_t L=0; L< nci2.size(); L++)
           {            
             ncitot.push_back(nci2[L]);
           } 
           
           for (size_t t2=0; t2< nci2.size(); t2++)
           {
             VMesh::Elem::array_type nci3;

             imesh->get_neighbors(nci3, nci2[t2]);
             for (size_t L=0; L< nci3.size(); L++)
             {            
               ncitot.push_back(nci3[L]);
             }         
           }
        
                                   
      }
      
      
            
      not_on_list=0;
      for (size_t p1=0; p1< ncitot.size(); p1++)
      {
        not_on_list=0;
        for (size_t p2=0; p2< Nlist.size(); p2++)  
        {  
          if(ncitot[p1]==Nlist[p2])
          {
             not_on_list=1;
          }
          
        }
        if(not_on_list==0)
        {
          Nlist.push_back(ncitot[p1]);
        }
      }
            
      angles.clear();
      original.clear();
      ifield->get_value(v0,idx);
      for (size_t q=0; q< Nlist.size(); q++)
      {
        a=Nlist[q];
        ifield->get_value(v1,a);
        if(v0.length()*v1.length()==0)
        {
          angles.push_back(0);
          original.push_back(0);
        }
        else
        {
          gdot=Dot(v0,v1);
          m1=v0.length();
          m2=v1.length();
          angle=(gdot/(m1*m2));
          angles.push_back(angle);
          original.push_back(angle);
        }
      } 
      
      sort(angles.begin(), angles.end());
      middle=(int)((angles.size()+1)/2);
      for(int k=0; k<original.size(); k++)
      {
        if(original[k]==angles[middle])
        {
          myloc=k;
          k=original.size();
        }
      }

      b=Nlist[myloc];
      ifield->get_value(v2,b);
      
      
           
//      double plength=0;
//      if(v2.length()<.1)
//      {
//        for(int k=0; k<angles.size(); k++)
//        {
//            int c=Nlist[k];
//            ifield->get_value(v2,c);
//
//            if(v2.length() > plength)
//            {
//                plength=v2.length();
//                v3=v2;
//            }
//            
//        }
//        v2=v3;
//     
//      }
         
           
      ofield->set_value(v2,idx);
      
      //idx_cnt++;
//      if(idx_cnt>100)
//      {
//         input.update_progress((double(idx) / (double) num_values));
//         idx_cnt=0;
//      }
    }
    
    // send new output if there is any: 
    //send_output_handle(output, ofield);

  }
  algo_end(); return (true);
}

} // end namespace SCIRun
