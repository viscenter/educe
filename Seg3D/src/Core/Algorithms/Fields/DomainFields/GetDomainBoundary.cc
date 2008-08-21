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

#include <Core/Algorithms/Fields/DomainFields/GetDomainBoundary.h>

#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/FieldInformation.h>
#include <sci_hash_map.h>

namespace SCIRunAlgo {

using namespace SCIRun;

typedef class {
public:
  VMesh::Node::index_type node;
  int val1;
  int val2;      
  bool hasneighbor;
} pointtype;

bool 
GetDomainBoundaryAlgo::
run(FieldHandle input, FieldHandle& output)
{
  MatrixHandle domainlink;
  return(run(input,domainlink,output));
}


struct IndexHash {
  static const size_t bucket_size = 4;
  static const size_t min_buckets = 8;
  
  size_t operator()(const Field::index_type &idx) const
    { return (static_cast<size_t>(idx)); }
  
  bool operator()(const Field::index_type &i1, const Field::index_type &i2) const
    { return (i1 < i2); }
};

bool 
GetDomainBoundaryAlgo::
run(FieldHandle input,  MatrixHandle domainlink, FieldHandle& output)
{

#ifdef HAVE_HASH_MAP
  typedef hash_multimap<Field::index_type,pointtype,IndexHash> pointhash_map_type;
  typedef hash_map<Field::index_type,VMesh::Node::index_type,IndexHash> hash_map_type;
#else
  typedef multimap<Field::index_type,pointtype> pointhash_map_type;
  typedef map<Field::index_type,VMesh::Node::index_type> hash_map_type;
#endif

  algo_start("getDomainBoundary",true);
  
  int minval = get_int("min_range");
  int maxval = get_int("max_range");
  int domval = get_int("domain");
  
  bool userange = get_bool("use_range");
  
  if (!userange)
  {
    minval = domval; maxval = domval;
  }
  
  bool addouterboundary = get_bool("add_outer_boundary");
  bool innerboundaryonly = get_bool("inner_boundary_only");
  bool noinnerboundary = get_bool("no_inner_boundary");
  bool disconnect = get_bool("disconnect_boundaries");
  
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }

  if (!(input->has_virtual_interface()))
  {
    error("No input field has no virtual interface");
    algo_end(); return (false);
  }

  FieldInformation fi(input);
  FieldInformation fo(input);
  
  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    algo_end(); return (false);
  }
  
  if (!(fi.is_constantdata()))
  {
    error("This function needs a compartment definition on the elements (constant element data)");
    algo_end(); return (false);    
  }
  
  if (!(fi.is_volume()||fi.is_surface()))
  {
    error("This function is only defined for surface and volume data");
    algo_end(); return (false);
  }

  if (fi.is_hex_element())
  {
    fo.make_quadsurfmesh();
  }
  else if (fi.is_quad_element()||fi.is_tri_element())
  {
    fo.make_curvemesh();
  }
  else if (fi.is_tet_element())
  {
    fo.make_trisurfmesh();
  }
  else if (fi.is_crv_element())
  {
    fo.make_pointcloudmesh();
  }
  else
  {
    error("No method available for input mesh");
    algo_end(); return (false);
  }

  fo.make_constantdata();
  fo.make_int();
  output = CreateField(fo);
  
  
  if (output.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return (false);
  }
  

  VField *ifield = input->vfield();
  VField *ofield = output->vfield();
  VMesh  *imesh =  input->vmesh();
  VMesh  *omesh =  output->vmesh();

  imesh->synchronize(Mesh::DELEMS_E|Mesh::ELEM_NEIGHBORS_E|Mesh::NODE_NEIGHBORS_E);
  
  VMesh::Node::size_type numnodes = imesh->num_nodes();
  VMesh::DElem::size_type numdelems = imesh->num_delems();

  bool isdomlink = false;
  Matrix::index_type* domlinkrr = 0;
  Matrix::index_type* domlinkcc = 0;
  
  std::vector<int> newvalues;
  
  if (domainlink.get_rep())
  {
    if ((numdelems != domainlink->nrows())&&(numdelems != domainlink->ncols()))
    {
      error("The Domain Link property is not of the right dimensions");
      algo_end(); return (false);        
    }
    SparseRowMatrix *spr = dynamic_cast<SparseRowMatrix *>(domainlink.get_rep());
    if (spr)
    {
      domlinkrr = spr->rows;
      domlinkcc = spr->columns;
      isdomlink = true;
    }
  }  


  
  if (disconnect)
  {

    pointhash_map_type node_map;
    
    VMesh::Elem::index_type nci, ci;
    VMesh::Elem::array_type elements; 
    VMesh::DElem::array_type delems; 
    VMesh::Node::array_type inodes; 
    VMesh::Node::array_type onodes; 
    VMesh::Node::index_type a;
    
    int val1, val2, newval;

    Point point;

    Field::index_type cnt = 0;

    for(VMesh::DElem::index_type delem = 0; delem < numdelems; delem++)       
    {
      bool neighborexist = false;
      bool includeface = false;

      imesh->get_elems(elements,delem);
      ci = elements[0];
      if (elements.size() > 1) 
      {
        neighborexist = true;
        nci  = elements[1];
      }

      if ((!neighborexist)&&(isdomlink))
      {
        for (Field::index_type rr = domlinkrr[delem]; rr < domlinkrr[delem+1]; rr++)
        {
          VMesh::DElem::index_type idx = domlinkcc[rr];
          VMesh::Node::array_type nodes;
          VMesh::Elem::array_type elems;           
          VMesh::DElem::array_type delems2;           

          imesh->get_nodes(nodes,idx);       
          imesh->get_elems(elems,nodes[0]);

          for (size_t r=0; r<elems.size(); r++)
          {
            imesh->get_delems(delems2,elems[r]);

            for (size_t s=0; s<delems2.size(); s++)
            {
              if (delems2[s]==idx) 
              { 
                nci = elems[r]; 
                neighborexist = true; 
                break; 
              }
            }
            if (neighborexist) break;
          }
          if (neighborexist) break;
        }
      }

      if (neighborexist)
      {
        ifield->value(val1,ci);
        ifield->value(val2,nci);
        if (innerboundaryonly == false)
        {
          if (noinnerboundary)
          {
            if ((((val1 >= minval)&&(val1 <= maxval)&&
                (!((val2 >= minval)&&(val2 <= maxval))))&&
                (userange == true)))
            {
              newval = val1;
              includeface = true;                         
            }
            else if (((val2 >= minval)&&(val2 <= maxval)&&
                (!((val1 >= minval)&&(val1 <= maxval))))&&
                (userange == true))
            {
              newval = val2;
              includeface = true;             
            }              
          }
          else
          {
            if ((((val1 >= minval)&&(val1 <= maxval))||
                ((val2 >= minval)&&(val2 <= maxval)))||
                (userange == false))
            {
              if (!(val1 == val2)) 
              {
                includeface = true;             
                if((val1 >= minval)&&(val1 <= maxval)) newval = val1;
                else newval = val2;
              }
            }
          }
        }
        else
        {
          if ((((val1 >= minval)&&(val2 >= minval))&&
              ((val1 <= maxval)&&(val2 <= maxval)))||
              (userange == false))
          {
            if (!(val1 == val2)) 
            {
              includeface = true;
              newval = val1;
              if (val2 < newval) newval = val2;
            }
          }          
        }
      }
      else if ((addouterboundary)&&(innerboundaryonly == false))
      {
        ifield->value(val1,ci);
        if (((val1 >= minval)&&(val1 <= maxval))||(userange == false)) 
        {
          newval = val1;
          includeface = true;
        }
      }

      if (includeface)
      {
        imesh->get_nodes(inodes,delem);
        onodes.resize(inodes.size());
        for (size_t q=0; q< onodes.size(); q++)
        {
          a = inodes[q];
          
          std::pair<pointhash_map_type::iterator,pointhash_map_type::iterator> lit;
          lit = node_map.equal_range(a);
          
          VMesh::Node::index_type nodci;
          int v1, v2;
          bool hasneighbor;
          
          if (neighborexist)
          {
            if (val1 < val2) { v1 = val1; v2 = val2; } else { v1 = val2; v2 = val1; }
            hasneighbor = true;
          }
          else
          {
            v1 = val1; v2 = 0;
            hasneighbor = false;
          }
          
          while (lit.first != lit.second)
          {
            if (((*(lit.first)).second.val1 == v1)&&
                ((*(lit.first)).second.val2 == v2)&&
                ((*(lit.first)).second.hasneighbor == hasneighbor))
            {
              nodci = (*(lit.first)).second.node;
              break;
            }
            ++(lit.first);
          }
          
          if (lit.first == lit.second)
          {
            pointtype newpoint;
            imesh->get_center(point,a);
            onodes[q] = omesh->add_point(point);
            newpoint.node = onodes[q];
            newpoint.val1 = v1;
            newpoint.val2 = v2;
            newpoint.hasneighbor = hasneighbor;
            node_map.insert(pointhash_map_type::value_type(a,newpoint));
          }
          else
          {
            onodes[q] = nodci;
          }
          
        }
        omesh->add_elem(onodes);
        newvalues.push_back(newval);
      }
      cnt++; if (cnt == 100) update_progress(delem,numdelems);
    }
  }
  else
  {
    hash_map_type node_map;
  
    VMesh::Elem::index_type nci, ci;
    VMesh::Elem::array_type elements; 
    VMesh::DElem::array_type delems; 
    VMesh::Node::array_type inodes; 
    VMesh::Node::array_type onodes; 
    VMesh::Node::index_type a;
    int val1, val2, newval;

    Point point;

    Field::index_type cnt = 0;

    for(VMesh::DElem::index_type delem = 0; delem < numdelems; delem++)       
    {
      bool neighborexist = false;
      bool includeface = false;

      imesh->get_elems(elements,delem);
      ci = elements[0];
      if (elements.size() > 1) 
      {
        neighborexist = true;
        nci  = elements[1];
      }

      if ((!neighborexist)&&(isdomlink))
      {
        for (Field::index_type rr = domlinkrr[delem]; rr < domlinkrr[delem+1]; rr++)
        {
          VMesh::DElem::index_type idx = domlinkcc[rr];
          VMesh::Node::array_type nodes;
          VMesh::Elem::array_type elems;           
          VMesh::DElem::array_type delems2;           

          imesh->get_nodes(nodes,idx);       
          imesh->get_elems(elems,nodes[0]);

          for (size_t r=0; r<elems.size(); r++)
          {
            imesh->get_delems(delems2,elems[r]);

            for (size_t s=0; s<delems2.size(); s++)
            {
              if (delems2[s]==idx) { nci = elems[r]; neighborexist = true; break; }
            }
            if (neighborexist) break;
          }
          if (neighborexist) break;
        }
      }

      if (neighborexist)
      {
        ifield->value(val1,ci);
        ifield->value(val2,nci);
        if (innerboundaryonly == false)
        {
          if (noinnerboundary)
          {
            if (((val1 >= minval)&&(val1 <= maxval)&&
                (!((val2 >= minval)&&(val2 <= maxval))))&&
                (userange == true))
            {
              newval = val1;
              includeface = true;                         
            }    
            else if(((val2 >= minval)&&(val2 <= maxval)&&
                (!((val1 >= minval)&&(val1 <= maxval))))&&
                (userange == true))
            {
              newval = val2;
              includeface = true;             
            }              
          }
          else
          {
            if ((((val1 >= minval)&&(val1 <= maxval))||
                ((val2 >= minval)&&(val2 <= maxval)))||
                (userange == false))
            {
              if (!(val1 == val2)) 
              {
                includeface = true;             
                if((val1 >= minval)&&(val1 <= maxval)) newval = val1;
                else newval = val2;
              }
            }
          }
        }
        else
        {
          if ((((val1 >= minval)&&(val2 >= minval))&&
              ((val1 <= maxval)&&(val2 <= maxval)))||
              (userange == false))
          {
            if (!(val1 == val2)) 
            {
              includeface = true;
              newval = val1;
              if (val2 < newval) newval = val2;
            }
          }          
        }
      }
      else if ((addouterboundary)&&(innerboundaryonly == false))
      {
        ifield->value(val1,ci);
        if (((val1 >= minval)&&(val1 <= maxval))||
            (userange == false)) 
        {
          newval = val1;
          includeface = true;
        }
      }

      if (includeface)
      {
        imesh->get_nodes(inodes,delem);
        onodes.resize(inodes.size());
        for (size_t q=0; q< onodes.size(); q++)
        {
          a = inodes[q];
          
          hash_map_type::iterator lit;
          lit = node_map.find(a);
          
          if (lit == node_map.end())
          {
            imesh->get_center(point,a);
            onodes[q] = omesh->add_point(point);
            node_map[a] = onodes[q];
          }
          else
          {
            onodes[q] = (*lit).second;
          }
          
        }
        omesh->add_elem(onodes);
        newvalues.push_back(newval);
      }
      cnt++; if (cnt == 100) update_progress(delem,numdelems);
    }
  }
  
  ofield->resize_values();
  ofield->set_values(newvalues);
  
  // copy property manager
	output->copy_properties(input.get_rep());
  algo_end(); return (true);
}

} // End namespace SCIRunAlgo
