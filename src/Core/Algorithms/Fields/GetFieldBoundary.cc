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

#include <Core/Algorithms/Fields/GetFieldBoundary.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool GetFieldBoundaryAlgo::GetFieldBoundary(ProgressReporter *pr, FieldHandle input, FieldHandle& output, MatrixHandle& mapping)
{
  //! Check whether we have an input field
  if (input.get_rep() == 0)
  {
    pr->error("GetFieldBoundary: No input field");
    return (false);
  }

  //! Figure out what the input type and output type have to be
  FieldInformation fi(input);
  FieldInformation fo(input);
  
  //! We do not yet support Quadratic and Cubic Meshes here
  if (fi.is_nonlinear())
  {
    pr->error("GetFieldBoundary: This function has not yet been defined for non-linear elements");
    return (false);
  }
  
  //! Check whether we can derive a boundary
  if (!(fi.is_volume()||fi.is_surface()||fi.is_curve()))
  {
    pr->error("GetFieldBoundary: This function is only defined for curve, surface and volume data");
    return (false);
  }
  
  //! Figure out which type of field the output is:
  bool found_method = false;
  if (fi.is_hex_element())  { fo.make_quadsurfmesh(); found_method = true; }
  if (fi.is_prism_element())  { fo.make_quadsurfmesh(); found_method = true; }
  if (fi.is_tet_element()) { fo.make_trisurfmesh(); found_method = true; }
  if (fi.is_quad_element()) { fo.make_curvemesh(); found_method = true; }
  if (fi.is_tri_element()) { fo.make_curvemesh(); found_method = true; }
  if (fi.is_pnt_element())
  {
    pr->remark("GetFieldBoundary: The field boundary of a point cloud is the same point cloud");
    output = input;
    return (true);
  }
  
  //! Check whether we could make a conversion
  if (!found_method)
  {
    pr->error("GetFieldBoundary: No method available for mesh: " + fi.get_mesh_type());
    return (false);
  }

  //! This algorithm current only works in the virtual interface mode
  if (fi.has_virtual_interface() && fo.has_virtual_interface())
  {
    //! Create the output field
    output = CreateField(fo);
    
    //! Check which data type we should use
    if (UseScalarInterface(fi,fo)) return (GetFieldBoundaryV<double>(pr,input,output,mapping));
    if (UseVectorInterface(fi,fo)) return (GetFieldBoundaryV<Vector>(pr,input,output,mapping));
    if (UseTensorInterface(fi,fo)) return (GetFieldBoundaryV<Tensor>(pr,input,output,mapping));
  }
    
  pr->error("GetFieldBoundary: Datatype is not supported by this algorithm");
  return (false);
}

//! Templated only in data type

template <class DATA>
bool GetFieldBoundaryAlgo::GetFieldBoundaryV(ProgressReporter *pr, FieldHandle input, FieldHandle& output, MatrixHandle& mapping)
{
  
#ifdef HAVE_HASH_MAP
  typedef hash_map<unsigned int,unsigned int> hash_map_type;
#else
  typedef map<unsigned int,unsigned int> hash_map_type;
#endif
  hash_map_type node_map;
  hash_map_type elem_map;
  
  //! Get the virtual interfaces:
  VMesh* imesh = input->vmesh();
  VMesh* omesh = output->vmesh();
  VField* ifield = input->vfield();
  VField* ofield = output->vfield();
    

  imesh->synchronize(Mesh::DELEMS_E|Mesh::ELEM_NEIGHBORS_E);
  
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
          hash_map_type::iterator it = node_map.find(static_cast<unsigned int>(a));
          if (it == node_map.end())
          {
            imesh->get_center(point,a);
            onodes[q] = omesh->add_node(point);
            node_map[static_cast<unsigned int>(a)] = static_cast<unsigned int>(onodes[q]);            
          }
          else
          {
            onodes[q] = static_cast<VMesh::Node::index_type>(node_map[static_cast<unsigned int>(a)]);
          }
        }
        elem_map[static_cast<unsigned int>(omesh->add_elem(onodes))] = static_cast<unsigned int>(ci);
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
    DATA val;
    imesh->size(isize);
    omesh->size(osize);

    Matrix::size_type nrows = static_cast<Matrix::size_type>(osize);
    Matrix::size_type ncols = static_cast<Matrix::size_type>(isize);
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
      ifield->get_value(val,idx1);
      ofield->set_value(val,idx2);

      ++it;
    }
    
    mapping = scinew SparseRowMatrix(nrows, ncols, rr, cc, nrows, d);
  }
  else if (input->basis_order() == 1)
  {
    VMesh::Node::size_type isize;
    VMesh::Node::size_type osize;
    DATA val;
    imesh->size(isize);
    omesh->size(osize);

    Matrix::size_type nrows = static_cast<Matrix::size_type>(osize);
    Matrix::size_type ncols = static_cast<Matrix::size_type>(isize);
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
      ifield->get_value(val,idx1);
      ofield->set_value(val,idx2);
      ++it;
    }
    
    mapping = scinew SparseRowMatrix(nrows, ncols, rr, cc, nrows, d);
  }
  
  // copy property manager
	output->copy_properties(input.get_rep());
  return (true);
}

} // End namespace SCIRunAlgo
