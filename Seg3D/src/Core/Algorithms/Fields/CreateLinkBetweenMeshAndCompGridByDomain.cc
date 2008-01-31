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

#include <Core/Algorithms/Fields/CreateLinkBetweenMeshAndCompGridByDomain.h>

namespace SCIRunAlgo {

using namespace SCIRun;

bool CreateLinkBetweenMeshAndCompGridByDomainAlgo::CreateLinkBetweenMeshAndCompGridByDomain(ProgressReporter *pr, FieldHandle input, MatrixHandle NodeLink, MatrixHandle& GeomToComp, MatrixHandle& CompToGeom)
{
  if (input.get_rep() == 0)
  {
    pr->error("CreateLinkBetweenMeshAndCompGridByDomain: No input field");
    return (false);
  }

  // no precompiled version available, so compile one

  FieldInformation fi(input);
  
  if (fi.is_nonlinear())
  {
    pr->error("CreateLinkBetweenMeshAndCompGridByDomain: This function has not yet been defined for non-linear elements");
    return (false);
  }

  if (!(fi.is_constantdata()))
  {
    pr->error("CreateLinkBetweenMeshAndCompGridByDomain: The field needs to have data assigned to the elements");
    return (false);   
  }
   
  if (!(fi.is_volume()||fi.is_surface()||fi.is_curve()))
  {
    pr->error("CreateLinkBetweenMeshAndCompGridByDomain: this function is only defined for curve, surface and volume data");
    return (false);
  }

  if (NodeLink.get_rep() == 0)
  {
    pr->error("LinkToCompGrid: No matrix on input");
    return (false);
  }

  if (!(NodeLink->is_sparse()))
  {
    pr->error("LinkToCompGrid: NodeLink Matrix is not sparse");
    return (false);  
  }

  if (NodeLink->nrows() != NodeLink->ncols())
  {
    pr->error("LinkToCompGrid: NodeLink Matrix needs to be square");
    return (false);      
  }

  VField *ifield = input->vfield();
  if (ifield == 0)
  {
    pr->error("LinkToCompGrid: FieldHandle is empty");
    return (false);     
  }
  
  VMesh *imesh = ifield->vmesh();
  if (imesh == 0)
  {
    pr->error("LinkToCompGrid: Field does not have a mesh");
    return (false);       
  }
        
  VMesh::Node::size_type numnodes;
  imesh->size(numnodes);
  
  if (NodeLink->nrows() != numnodes)
  {
    pr->error("LinkToCompGrid: NodeLink Matrix has improper dimensions");
    return (false);        
  }
  
  SparseRowMatrix* spr = dynamic_cast<SparseRowMatrix*>(NodeLink.get_rep());
  Matrix::size_type m = spr->ncols();
  Matrix::index_type *rows = spr->rows;
  Matrix::index_type *cols = spr->columns;
  
  Matrix::index_type *rr = scinew Matrix::index_type[m+1];
  Matrix::index_type *cc = scinew Matrix::index_type[m];
  double *vv = scinew double[m];  
  if ((rr == 0)||(cc == 0)||(vv == 0))
  {
    if (rr) delete[] rr;
    if (cc) delete[] cc;
    if (vv) delete[] vv;
    
    pr->error("LinkToCompGrid: Could not allocate memory for sparse matrix");
    return (false);        
  }
  
  imesh->synchronize(Mesh::NODE_NEIGHBORS_E);
  
  for (Matrix::index_type r=0; r<m; r++) rr[r] = r;

  for (Matrix::index_type r=0; r<m; r++)
  {
    for (Matrix::index_type c=rows[r]; c<rows[r+1]; c++)
    {
      if (cols[c] > r) 
      {
        VMesh::Elem::array_type elems1;
        VMesh::Elem::array_type elems2;
        imesh->get_elems(elems1,VMesh::Node::index_type(r));
        imesh->get_elems(elems2,VMesh::Node::index_type(cols[c]));
        double val1, val2;
        ifield->get_value(val1,elems1[0]);
        ifield->get_value(val2,elems2[0]);
        if (val1 == val2) rr[cols[c]] = r;
      }
    }
  }

  for (Matrix::index_type r=0; r<m; r++)
  {
    if (rr[r] != r) 
    {
      Matrix::index_type q = r;
      while (rr[q] != q) q = rr[q];
      rr[r] = q;
    }
  }

  Matrix::index_type k=0;
  for (Matrix::index_type r=0; r<m; r++)
  {
    if (rr[r] == r)
    {
      cc[r] = k; k++;
    }
  }
  
  for (Matrix::index_type r=0; r<m; r++)
  {
    rr[r] = cc[rr[r]];
  }

  for (Matrix::index_type r = 0; r < m; r++)
  {
    cc[r] = rr[r];
    rr[r] = r;
    vv[r] = 1.0;
  }
  rr[m] = m; // An extra entry goes on the end of rr.

  spr = scinew SparseRowMatrix(m, k, rr, cc, m, vv);

  if (spr == 0)
  {
    pr->error("LinkToCompGrid: Could build geometry to computational mesh mapping matrix");
    return (false);
  }

  CompToGeom = spr;
  GeomToComp = spr->transpose();

  if ((GeomToComp.get_rep() == 0)||(CompToGeom.get_rep() == 0))
  {
    pr->error("LinkToCompGrid: Could build geometry to computational mesh mapping matrix");
    return (false);
  }
  
  return (true);
}

} // End namespace SCIRunAlgo
