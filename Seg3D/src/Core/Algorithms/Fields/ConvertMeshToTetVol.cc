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

#include <Core/Algorithms/Fields/ConvertMeshToTetVol.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool ConvertMeshToTetVolAlgo::ConvertMeshToTetVol(ProgressReporter *pr, FieldHandle input, FieldHandle& output)
{
  if (input.get_rep() == 0)
  {
    pr->error("ConvertMeshToTetVol: No input field");
    return (false);
  }

  // no precompiled version available, so compile one

  FieldInformation fi(input);
  FieldInformation fo(input);
  
  if (fi.is_nonlinear())
  {
    pr->error("ConvertMeshToTetVol: This function has not yet been defined for non-linear elements");
    return (false);
  }
  
  std::string algotype;
  std::string mesh_type = fi.get_mesh_type();
  if ((mesh_type == "LatVolMesh")||(mesh_type == "StructHexVolMesh"))
  {
    fo.set_mesh_type("TetVolMesh");
    fo.set_container_type("vector");
    algotype = "ConvertLatVolToTetVolAlgoT";
  }
  else if (mesh_type == "HexVolMesh")
  {
    fo.set_mesh_type("TetVolMesh");
    fo.set_container_type("vector");
    algotype = "ConvertHexVolToTetVolAlgoT";
  }
  else
  {
    pr->error("No method available for mesh: " + mesh_type);
    return (false);
  }


  ////////////////////////////////////////////////
  // VIRTUAL VERSION OF THIS ALGORITHM

  if (fi.has_virtual_interface() && fo.has_virtual_interface())
  {
    output = CreateField(fo);
    
    if (algotype == "ConvertLatVolToTetVolAlgoT")
    {
      return (ConvertLatVolToTetVolV(pr,input,output));
    }
    else
    {
      return (ConvertHexVolToTetVolV(pr,input,output));
    }
  }

  ////////////////////////////////////////////////
  // DYNAMIC COMPILATION VERSION OF THIS ALGORITHM


  SCIRun::CompileInfoHandle ci = scinew CompileInfo(
    "ALGOConvertMeshToTetVol."+fi.get_field_filename()+"."+fo.get_field_filename()+".",
    "ConvertMeshToTetVolAlgo",algotype,
    fi.get_field_name() + "," + fo.get_field_name());

  ci->add_include(TypeDescription::cc_to_h(__FILE__));
  ci->add_namespace("SCIRunAlgo");
  ci->add_namespace("SCIRun");
  
  fi.fill_compile_info(ci);
  fo.fill_compile_info(ci);
  
  if (dynamic_cast<RegressionReporter *>(pr)) ci->keep_library_ = false;  
  
  // Handle dynamic compilation
  SCIRun::Handle<ConvertMeshToTetVolAlgo> algo;
  if(!(SCIRun::DynamicCompilation::compile(ci,algo,pr)))
  {
    pr->compile_error(ci->filename_);
    SCIRun::DynamicLoader::scirun_loader().cleanup_failed_compile(ci);  
    return(false);
  }

  return(algo->ConvertMeshToTetVol(pr,input,output));
}


bool ConvertMeshToTetVolAlgo::ConvertHexVolToTetVolV(ProgressReporter *pr, 
                                        FieldHandle input, FieldHandle& output)
{
  VField *ifield = input->vfield();
  if (ifield == 0)
  { 
    pr->error("ConvertMeshToTetVol: Could not obtain input field");
    return (false);
  }

  VMesh *imesh = ifield->vmesh();
  if (imesh == 0)
  {
    pr->error("ConvertMeshToTetVol: Could not obtain input mesh");
    return (false);
  }

  VField *ofield = output->vfield();
  if (ofield == 0)
  { 
    pr->error("ConvertMeshToTetVol: Could not obtain output field");
    return (false);
  }

  VMesh* omesh = ofield->vmesh();
  if (omesh == 0)
  {
    pr->error("ConvertMeshToTetVol: Could not obtain output mesh");
    return (false);
  }
    
  VMesh::Node::size_type numnodes; 
  VMesh::Elem::size_type numelems; 
  imesh->size(numnodes);
  imesh->size(numelems);
  
  omesh->node_reserve(static_cast<unsigned int>(numnodes));

  VMesh::Node::iterator nbi, nei;
  VMesh::Elem::iterator ebi, eei;
  VMesh::Node::array_type a(4);
    
  imesh->begin(nbi); 
  imesh->end(nei);
  while (nbi != nei)
  {
    Point point;
    imesh->get_center(point, *nbi);
    omesh->add_node(point);
    ++nbi;
  }

  imesh->synchronize(Mesh::NODE_NEIGHBORS_E);
  omesh->elem_reserve(static_cast<unsigned int>(numelems*5));

  vector<VMesh::Elem::index_type> elemmap(numelems);
  vector<char> visited(numelems, 0);

  VMesh::Elem::iterator bi, ei;
  imesh->begin(bi); imesh->end(ei);

  size_t surfsize = static_cast<size_t>(pow((double)numelems, 2.0 / 3.0));
  vector<VMesh::Elem::index_type> buffer;
  buffer.reserve(surfsize);
  
  imesh->synchronize(Mesh::FACES_E);

  while (bi != ei)
  {
    // if list of elements to process is empty ad the next one
    if (buffer.size() == 0)
    {
      if(visited[static_cast<unsigned int>(*bi)] == 0) buffer.push_back(*bi);
    }

    
    if (buffer.size() > 0)
    {

      for (unsigned int i=0; i< buffer.size(); i++)
      {
        if (visited[static_cast<unsigned int>(buffer[i])] > 0) { continue; }
        VMesh::Elem::array_type neighbors;

        int newtype = 0;
 
        imesh->get_neighbors(neighbors, buffer[i]);
        for (unsigned int p=0; p<neighbors.size(); p++)
        {
          if(visited[static_cast<unsigned int>(neighbors[p])] > 0)
          {
            if (newtype)
            {
              if (visited[static_cast<unsigned int>(neighbors[p])] != newtype)
              {
                pr->error("ConvertMeshToTetVol: Algorithm cannot deal with topology of input field, field cannot by sorted into checker board type of ordering");
                return (false);
              }
            }
            else
            {
              newtype = visited[static_cast<unsigned int>(neighbors[p])];
            }
          }
          else if(visited[static_cast<unsigned int>(neighbors[p])] == 0)
          {
            visited[static_cast<unsigned int>(neighbors[p])] = -1;
            buffer.push_back(neighbors[p]);
          }
        }
        
        if (newtype == 0) newtype = 1;
        if (newtype == 1) newtype = 2; else newtype = 1;
        
        VMesh::Node::array_type hv;
        imesh->get_nodes(hv, buffer[i]);
        
        // In case mesh is weird an not logically numbered
        if (static_cast<unsigned int>(buffer[i]) >= elemmap.size()) 
                        elemmap.resize(static_cast<unsigned int>(buffer[i]));
        
        if (newtype == 1)
        {
          a[0] = hv[0]; a[1] = hv[1]; a[2] = hv[2]; a[3] = hv[5];
          elemmap[static_cast<unsigned int>(buffer[i])] = omesh->add_elem(a);

          a[0] = hv[0]; a[1] = hv[2]; a[2] = hv[3]; a[3] = hv[7];
          omesh->add_elem(a);
          a[0] = hv[0]; a[1] = hv[5]; a[2] = hv[2]; a[3] = hv[7];
          omesh->add_elem(a);
          a[0] = hv[0]; a[1] = hv[5]; a[2] = hv[7]; a[3] = hv[4];
          omesh->add_elem(a);
          a[0] = hv[5]; a[1] = hv[2]; a[2] = hv[7]; a[3] = hv[6];
          omesh->add_elem(a);

          visited[static_cast<unsigned int>(buffer[i])] = 1;
        }
        else
        {
          a[0] = hv[0]; a[1] = hv[1]; a[2] = hv[3]; a[3] = hv[4];
          elemmap[static_cast<unsigned int>(buffer[i])] = omesh->add_elem(a);

          a[0] = hv[1]; a[1] = hv[2]; a[2] = hv[3]; a[3] = hv[6];
          omesh->add_elem(a);
          a[0] = hv[1]; a[1] = hv[3]; a[2] = hv[4]; a[3] = hv[6];
          omesh->add_elem(a);
          a[0] = hv[1]; a[1] = hv[5]; a[2] = hv[6]; a[3] = hv[4];
          omesh->add_elem(a);
          a[0] = hv[3]; a[1] = hv[4]; a[2] = hv[6]; a[3] = hv[7];
          omesh->add_elem(a);

          visited[static_cast<unsigned int>(buffer[i])] = 2;              
        }
      }
      buffer.clear();
    }
    ++bi;
  }

  ofield->resize_fdata();

  if (ifield->basis_order() == 0)
  {
    VMesh::size_type sz = static_cast<VMesh::size_type>(ifield->num_values());
    VMesh::index_type idx;

    for (VMesh::index_type r=0; r<sz; r++)
    {
      idx = elemmap[static_cast<unsigned int>(r)];
      ofield->copy_value(ifield,r,idx);
      ofield->copy_value(ifield,r,idx+1);
      ofield->copy_value(ifield,r,idx+2);
      ofield->copy_value(ifield,r,idx+3);
      ofield->copy_value(ifield,r,idx+4);
    }
  }
  
  if (ifield->basis_order() == 1)
  {
    ofield->copy_values(ifield);
  }

	output->copy_properties(input.get_rep());
  
  // Success:
  return (true);
}


bool ConvertMeshToTetVolAlgo::ConvertLatVolToTetVolV(ProgressReporter *pr, 
                                        FieldHandle input, FieldHandle& output)
{

  VField *ifield = input->vfield();
  if (ifield == 0)
  { 
    pr->error("ConvertMeshToTetVol: Could not obtain input field");
    return (false);
  }


  VMesh *imesh = ifield->vmesh();
  if (imesh == 0)
  {
    pr->error("ConvertMeshToTetVol: Could not obtain input mesh");
    return (false);
  }

  VField *ofield = output->vfield();
  if (ofield == 0)
  { 
    pr->error("ConvertMeshToTetVol: Could not obtain output field");
    return (false);
  }


  VMesh* omesh = ofield->vmesh();
  if (omesh == 0)
  {
    pr->error("ConvertMeshToTetVol: Could not obtain output mesh");
    return (false);
  }

  VMesh::Node::array_type a(4);
  VMesh::Node::size_type numnodes;
  imesh->size(numnodes);
  omesh->node_reserve(static_cast<unsigned int>(numnodes));

  // Copy points directly, assuming they will have the same order.
  VMesh::Node::iterator nbi, nei;
  VMesh::Node::iterator dbi, dei;
  imesh->begin(nbi); 
  imesh->end(nei);
  
  while (nbi != nei)
  {
    Point point;
    imesh->get_center(point, *nbi);
    omesh->add_node(point);
    ++nbi;
  }

  VMesh::Elem::size_type numelems;
  imesh->size(numelems);
  omesh->elem_reserve(static_cast<unsigned int>(numelems*5));

  VMesh::Elem::iterator bi, ei;
  VMesh::Elem::iterator obi, oei;

  VMesh::dimension_type dims;
  imesh->get_dimensions(dims);
  if (dims.size() != 3)
  {
    pr->error("ConvertMeshToTetVol: Could not obtain LatVol dimensions");
    return (false);    
  }
  const unsigned int d2 = (dims[0]-1)*(dims[1]-1);
  const unsigned int d1 = dims[0]-1;
  
  imesh->begin(bi); 
  imesh->end(ei);
  
  while (bi != ei)
  {
    VMesh::Node::array_type lv;
    imesh->get_nodes(lv, *bi);
    
    const unsigned int idx = *bi;
    const unsigned int k = idx / d2;
    const unsigned int jk = idx % d2;
    const unsigned int j = jk / d1;
    const unsigned int i = jk % d1;
    
    if (!((i^j^k)&1))
    {
      a[0] = lv[0]; a[1] = lv[1]; a[2] = lv[2]; a[3] = lv[5]; omesh->add_elem(a);
      a[0] = lv[0]; a[1] = lv[2]; a[2] = lv[3]; a[3] = lv[7]; omesh->add_elem(a);
      a[0] = lv[0]; a[1] = lv[5]; a[2] = lv[2]; a[3] = lv[7]; omesh->add_elem(a);
      a[0] = lv[0]; a[1] = lv[5]; a[2] = lv[7]; a[3] = lv[4]; omesh->add_elem(a);
      a[0] = lv[5]; a[1] = lv[2]; a[2] = lv[7]; a[3] = lv[6]; omesh->add_elem(a);
    }
    else
    {
      a[0] = lv[0]; a[1] = lv[1]; a[2] = lv[3]; a[3] = lv[4]; omesh->add_elem(a);
      a[0] = lv[1]; a[1] = lv[2]; a[2] = lv[3]; a[3] = lv[6]; omesh->add_elem(a);
      a[0] = lv[1]; a[1] = lv[3]; a[2] = lv[4]; a[3] = lv[6]; omesh->add_elem(a);
      a[0] = lv[1]; a[1] = lv[5]; a[2] = lv[6]; a[3] = lv[4]; omesh->add_elem(a);
      a[0] = lv[3]; a[1] = lv[4]; a[2] = lv[6]; a[3] = lv[7]; omesh->add_elem(a);
    }
    ++bi;
  }
  
  ofield->resize_fdata();

  if (ifield->basis_order() == 0)
  {
    VMesh::size_type sz = ifield->num_values();

    VMesh::index_type q = 0;
    for (VMesh::index_type r=0; r<sz; r++)
    {
      ofield->copy_value(ifield,r,q);
      ofield->copy_value(ifield,r,q+1);
      ofield->copy_value(ifield,r,q+2);
      ofield->copy_value(ifield,r,q+3);
      ofield->copy_value(ifield,r,q+4);
      q += 5;
    }
  }
  
  if (ifield->basis_order() == 1)
  {
    ofield->copy_values(ifield);
  }

	output->copy_properties(input.get_rep());
  
  // Success:
  return (true);
}


} // End namespace SCIRunAlgo
