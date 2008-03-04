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

#include <Core/Algorithms/Fields/MeshDerivatives/GetFieldBoundary.h>

#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/FieldInformation.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool 
GetFieldBoundaryAlgo::
run(FieldHandle input, FieldHandle& output, MatrixHandle& mapping)
{
  //! Define types we need for mapping
#ifdef HAVE_HASH_MAP
  typedef hash_map<Field::index_type,Field::index_type> hash_map_type;
#else
  typedef map<Field::index_type,Field::index_type> hash_map_type;
#endif
  hash_map_type node_map;
  hash_map_type elem_map;
  
  algo_start("GetFieldBoundary",true);
  
  //! Check whether we have an input field
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }

  //! Figure out what the input type and output type have to be
  FieldInformation fi(input);
  FieldInformation fo(input);
  
  //! We do not yet support Quadratic and Cubic Meshes here
  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    algo_end(); return (false);
  }
  
  //! Figure out which type of field the output is:
  bool found_method = false;
  if (fi.is_hex_element())    { fo.make_quadsurfmesh(); found_method = true; }
  if (fi.is_prism_element())  { fo.make_quadsurfmesh(); found_method = true; }
  if (fi.is_tet_element())    { fo.make_trisurfmesh(); found_method = true; }
  if (fi.is_quad_element())   { fo.make_curvemesh(); found_method = true; }
  if (fi.is_tri_element())    { fo.make_curvemesh(); found_method = true; }
  if (fi.is_pnt_element())
  {
    remark("The field boundary of a point cloud is the same point cloud");
    output = input;
    algo_end(); return (true);
  }
  
  //! Check whether we could make a conversion
  if (!found_method)
  {
    error("No method available for mesh of type: " + fi.get_mesh_type());
    algo_end(); return (false);
  }

  //! Create the output field
  output = CreateField(fo);
  if (output.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return (false);
  }
  
  //! Get the virtual interfaces:
  VMesh* imesh = input->vmesh();
  VMesh* omesh = output->vmesh();
  VField* ifield = input->vfield();
  VField* ofield = output->vfield();
  
  imesh->synchronize_delems();
  imesh->synchronize_elem_neighbors();
  
  //! These are all virtual iterators, virtual index_types and array_types
  VMesh::Elem::iterator be, ee;
  VMesh::Elem::index_type nci, ci;
  VMesh::DElem::array_type delems; 
  VMesh::Node::array_type inodes; 
  VMesh::Node::array_type onodes; 
  VMesh::Node::index_type a;

  inodes.clear();
  onodes.clear();  
  Point point;

  //! This algorithm was copy from the original dynamic compiled version
  //! and was slightly adapted to work here:
  
  imesh->begin(be); 
  imesh->end(ee);

  while (be != ee) 
  {
    ci = *be;
    imesh->get_delems(delems,ci);
    for (size_t p =0; p < delems.size(); p++)
    {
      bool includeface = false;
      
      if(!(imesh->get_neighbor(nci,ci,delems[p]))) includeface = true;

      if (includeface)
      {
        imesh->get_nodes(inodes,delems[p]);
        if (onodes.size() == 0) onodes.resize(inodes.size());
        for (size_t q=0; q<onodes.size(); q++)
        {
          a = inodes[q];
          hash_map_type::iterator it = node_map.find(a);
          if (it == node_map.end())
          {
            imesh->get_center(point,a);
            onodes[q] = omesh->add_node(point);
            node_map[a] = onodes[q];            
          }
          else
          {
            onodes[q] = node_map[a];
          }
        }
        elem_map[omesh->add_elem(onodes)] = ci;
      }
    }
    ++be;
  }
  
  mapping = 0;
  
  ofield->resize_fdata();
  
  if (ifield->basis_order() == 0)
  {
    VMesh::Elem::size_type isize;
    VMesh::Elem::size_type osize;
    imesh->size(isize);
    omesh->size(osize);

    Matrix::size_type nrows = osize;
    Matrix::size_type ncols = isize;
    Matrix::index_type *rr = scinew Matrix::index_type[nrows+1];
    Matrix::index_type *cc = scinew Matrix::index_type[nrows];
    double *d = scinew double[nrows];

    for (Matrix::index_type p = 0; p < nrows; p++)
    {
      cc[p] = 0;
      rr[p] = p;
      d[p] = 0.0;
    }
    rr[nrows] = nrows; // An extra entry goes on the end of rr.

    hash_map_type::iterator it, it_end;
    it = elem_map.begin();
    it_end = elem_map.end();
    
    while (it != it_end)
    {
      cc[(*it).first] = (*it).second;
      d[(*it).first] += 1.0;
      
      VMesh::Elem::index_type idx1((*it).second);
      VMesh::Elem::index_type idx2((*it).first);  
      
      //! Copying values
      ofield->copy_value(ifield,idx1,idx2);
      ++it;
    }
    
    mapping = scinew SparseRowMatrix(nrows, ncols, rr, cc, nrows, d);
  }
  else if (input->basis_order() == 1)
  {
    VMesh::Node::size_type isize;
    VMesh::Node::size_type osize;
    imesh->size(isize);
    omesh->size(osize);

    Matrix::size_type nrows = osize;
    Matrix::size_type ncols = isize;
    Matrix::index_type *rr = scinew Matrix::index_type[nrows+1];
    Matrix::index_type *cc = scinew Matrix::index_type[nrows];
    double *d = scinew double[nrows];

    for (Matrix::index_type p = 0; p < nrows; p++)
    {
      cc[p] = 0;
      rr[p] = p;
      d[p] = 0.0;
    }
    rr[nrows] = nrows; // An extra entry goes on the end of rr.

    hash_map_type::iterator it, it_end;
    it = node_map.begin();
    it_end = node_map.end();
    
    while (it != it_end)
    {
      cc[(*it).second] = (*it).first;
      d[(*it).second] += 1.0;

      VMesh::Node::index_type idx1((*it).first);
      VMesh::Node::index_type idx2((*it).second);
      ofield->copy_value(ifield,idx1,idx2);
      ++it;
    }
    
    mapping = scinew SparseRowMatrix(nrows, ncols, rr, cc, nrows, d);
  }
  
  // copy property manager
	output->copy_properties(input.get_rep());
  
  algo_end(); return (true);
}


//! A copy of the algorithm without creating the mapping matrix. 
//! Need this for the various algorithms that only use the boundary to
//! project nodes on.

bool 
GetFieldBoundaryAlgo::
run(FieldHandle input, FieldHandle& output)
{
  //! Define types we need for mapping
#ifdef HAVE_HASH_MAP
  typedef hash_map<Field::index_type,Field::index_type> hash_map_type;
#else
  typedef map<Field::index_type,Field::index_type> hash_map_type;
#endif
  hash_map_type node_map;
  hash_map_type elem_map;
  
  algo_start("GetFieldBoundary",true);
  
  //! Check whether we have an input field
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }

  //! Figure out what the input type and output type have to be
  FieldInformation fi(input);
  FieldInformation fo(input);
  
  //! We do not yet support Quadratic and Cubic Meshes here
  if (fi.is_nonlinear())
  {
    error("This function has not yet been defined for non-linear elements");
    algo_end(); return (false);
  }
  
  //! Figure out which type of field the output is:
  bool found_method = false;
  if (fi.is_hex_element())    { fo.make_quadsurfmesh(); found_method = true; }
  if (fi.is_prism_element())  { fo.make_quadsurfmesh(); found_method = true; }
  if (fi.is_tet_element())    { fo.make_trisurfmesh(); found_method = true; }
  if (fi.is_quad_element())   { fo.make_curvemesh(); found_method = true; }
  if (fi.is_tri_element())    { fo.make_curvemesh(); found_method = true; }
  if (fi.is_pnt_element())
  {
    remark("The field boundary of a point cloud is the same point cloud");
    output = input;
    algo_end(); return (true);
  }
  
  //! Check whether we could make a conversion
  if (!found_method)
  {
    error("No method available for mesh of type: " + fi.get_mesh_type());
    algo_end(); return (false);
  }

  //! Create the output field
  output = CreateField(fo);
  if (output.get_rep() == 0)
  {
    error("Could not create output field");
    algo_end(); return (false);
  }
  
  //! Get the virtual interfaces:
  VMesh* imesh = input->vmesh();
  VMesh* omesh = output->vmesh();
  VField* ifield = input->vfield();
  VField* ofield = output->vfield();
  
  imesh->synchronize_delems();
  imesh->synchronize_elem_neighbors();
  
  //! These are all virtual iterators, virtual index_types and array_types
  VMesh::Elem::iterator be, ee;
  VMesh::Elem::index_type nci, ci;
  VMesh::DElem::array_type delems; 
  VMesh::Node::array_type inodes; 
  VMesh::Node::array_type onodes; 
  VMesh::Node::index_type a;

  inodes.clear();
  onodes.clear();  
  Point point;

  //! This algorithm was copy from the original dynamic compiled version
  //! and was slightly adapted to work here:
  
  imesh->begin(be); 
  imesh->end(ee);

  while (be != ee) 
  {
    ci = *be;
    imesh->get_delems(delems,ci);
    for (size_t p =0; p < delems.size(); p++)
    {
      bool includeface = false;
      
      if(!(imesh->get_neighbor(nci,ci,delems[p]))) includeface = true;

      if (includeface)
      {
        imesh->get_nodes(inodes,delems[p]);
        if (onodes.size() == 0) onodes.resize(inodes.size());
        for (size_t q=0; q<onodes.size(); q++)
        {
          a = inodes[q];
          hash_map_type::iterator it = node_map.find(a);
          if (it == node_map.end())
          {
            imesh->get_center(point,a);
            onodes[q] = omesh->add_node(point);
            node_map[a] = onodes[q];            
          }
          else
          {
            onodes[q] = node_map[a];
          }
        }
        elem_map[omesh->add_elem(onodes)] = ci;
      }
    }
    ++be;
  }
  
  ofield->resize_fdata();
  
  if (ifield->basis_order() == 0)
  {
    hash_map_type::iterator it, it_end;
    it = elem_map.begin();
    it_end = elem_map.end();
    
    while (it != it_end)
    {      
      VMesh::Elem::index_type idx1((*it).second);
      VMesh::Elem::index_type idx2((*it).first);  
      
      //! Copying values
      ofield->copy_value(ifield,idx1,idx2);
      ++it;
    }
  }
  else if (input->basis_order() == 1)
  {
    hash_map_type::iterator it, it_end;
    it = node_map.begin();
    it_end = node_map.end();
    
    while (it != it_end)
    {
      VMesh::Node::index_type idx1((*it).first);
      VMesh::Node::index_type idx2((*it).second);
      ofield->copy_value(ifield,idx1,idx2);
      ++it;
    }
  }
  
  // copy property manager
	output->copy_properties(input.get_rep());
  
  algo_end(); return (true);
}


} // End namespace SCIRunAlgo
