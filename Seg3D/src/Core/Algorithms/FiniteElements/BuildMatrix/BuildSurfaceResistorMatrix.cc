/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2004 Scientific Coutputputing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without linputitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whoutput the
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

#include <Core/Algorithms/FiniteElements/BuildMatrix/BuildSurfaceResistorMatrix.h>

#include <Core/Containers/SearchGridT.h>

#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Datatypes/VField.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/SparseRowMatrix.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SparseElement {
public:
  index_type     row; 
  index_type     column;
  double         value;
};

inline bool operator<(const SparseElement& s1, const SparseElement& s2)
{
  if (s1.row < s2.row) return(true);
  if ((s1.row == s2.row)&&(s1.column < s2.column)) return(true);
  return (false);
}

inline bool operator==(const SparseElement& s1, const SparseElement& s2)
{
  if ((s1.row == s2.row)&&(s1.column == s2.column)) return(true);
  return (false);
}



bool 
BuildSurfaceResistorMatrixAlgo::
run(FieldHandle& FEMesh, 
    FieldHandle& SurfaceResistor, 
    MatrixHandle& NodeLink, 
    MatrixHandle& DElemLink,
    MatrixHandle& SurfaceResistorMatrix)
{
  algo_start("BuildSurfaceResistorMatrix",false);

  if (FEMesh.get_rep() == 0)
  {
    error("No FEMesh Field was given");
    algo_end(); return (false);
  }

  if (!(FEMesh->has_virtual_interface()))
  {
    error("FEMesh needs to have a virtual interface");
    algo_end(); return (false);
  }

  if (SurfaceResistor.get_rep() == 0)
  {
    error("No SurfaceResistor Field was given");
    algo_end(); return (false);
  }

  if (!(SurfaceResistor->has_virtual_interface()))
  {
    error("SurfaceResistor needs to have a virtual interface");
    algo_end(); return (false);
  }

  // no pregridiled version available, so gridile one

  FieldInformation fi(FEMesh);
  FieldInformation fi2(SurfaceResistor);
  
  if (!((fi.is_volume()&&fi2.is_surface())||(fi.is_surface()&&fi2.is_curve())))
  {
    error("The FEMesh needs to be volume or a surface, and the surface resistor needs to be surface or a curve");
    algo_end(); return (false);
  }  

  VMesh* femesh = FEMesh->vmesh();
  //VField* fefield = FEMesh->vfield();
  
  VMesh* srmesh = SurfaceResistor->vmesh();
  VField* srfield = SurfaceResistor->vfield();
  
  femesh->synchronize(Mesh::NODE_NEIGHBORS_E|Mesh::DELEMS_E|Mesh::EPSILON_E|Mesh::NODE_LOCATE_E);

  VField::size_type num_fenodes  = femesh->num_nodes();
  VField::size_type num_srnodes  = srmesh->num_nodes();
  VField::size_type num_feelems  = femesh->num_elems();
  VField::size_type num_srelems  = srmesh->num_elems();
  VField::size_type num_fedelems = femesh->num_delems();

  if ((num_fenodes == 0)||(num_srnodes == 0))
  {
    error("One of the input meshes has no nodes");
    algo_end(); return (false);  
  }

  if ((num_feelems == 0)||(num_srelems == 0))
  {
    error("One of the input meshes has no elems");
    algo_end(); return (false);  
  }

  // Process ElemLink: this matrix tells which faces are connected by a 
  // membrane to faces on the opposite side of the mesh

  Matrix::index_type* delemlinkrr = 0;
  Matrix::index_type* delemlinkcc = 0;

  if (DElemLink.get_rep())
  {
    // We have a ElemLink Mapping Matrix
    
    // Do a sanity check, if not return a proper error
    if ((num_fedelems != DElemLink->nrows())&&(num_fedelems != DElemLink->ncols()))
    {
      error("The DElemLink mapping matrix is not of the same dimensions as the FEMesh");
      algo_end(); return (false);        
    }
    
    // Get the SparseMatrix, if not we will not do this operation
    SparseRowMatrix *spr = dynamic_cast<SparseRowMatrix *>(DElemLink.get_rep());
    if (spr)
    {
      delemlinkrr = spr->rows;
      delemlinkcc = spr->columns;
    }
    else
    {
      // Inform the user that they tried something ill-fated
      error("The DElemLink matrix is not a sparse matrix");
      algo_end(); return (false);       
    }
    
    for (size_t q=0; q<num_fedelems; q++)
    {
      if (delemlinkrr[q+1]-delemlinkrr[q] > 1)
      {
        // Inform the user that they tried something ill-fated
        error("Each derivative element can only be connected to one other derivative element");
        algo_end(); return (false);             
      }
    }
  }  

  // Process NodeLink: this matrix tells which nodes are connected by a 
  // membrane to faces on the opposite side of the mesh

  // Assume we do not have it
  Matrix::index_type* nodelinkrr = 0;
  Matrix::index_type* nodelinkcc = 0;
  
  if (NodeLink.get_rep())
  {
    // We have a NodeLink Matrix
    
    // Do a sanity check, if not return a proper error
    if ((num_fenodes != NodeLink->nrows())&&(num_fenodes != NodeLink->ncols()))
    {
      error("The NodeLink property is not of the right dimensions");
      algo_end(); return (false);        
    }
    
    // Get the SparseMatrix, if not we will not do this operation
    SparseRowMatrix *spr = dynamic_cast<SparseRowMatrix *>(NodeLink.get_rep());
    if (spr)
    {
      nodelinkrr = spr->rows;
      nodelinkcc = spr->columns;
    }
    else
    {
      // Inform the user that they tried something ill-fated
      error("The NodeLink matrix is not a sparse matrix");
      algo_end(); return (false);       
    }
  }  

  if (NodeLink.get_rep() && !(DElemLink.get_rep()))
  {
    error("This algorithm needs both DElemLink and NodeLink, not only a NodeLink");
    algo_end(); return (false);         
  }

  if (!(NodeLink.get_rep()) && DElemLink.get_rep())
  {
    error("This function needs both DElemLink and NodeLink, not only a DElemLink");
    algo_end(); return (false);         
  }

  bool foundsurf1 = false;
  bool foundsurf2 = false;

  VMesh::Elem::iterator eit, eit_end;
  VMesh::DElem::index_type delem1, delem2;
  VMesh::DElem::array_type delems;	
  VMesh::Node::array_type  nodes;
  VMesh::Node::array_type  fenodes,fenodes1, fenodes2;
  vector<Point> points, points2;
  vector<double> coefs;
  
  double epsilon = 10*femesh->get_epsilon();  
  double epsilon2 = epsilon*epsilon;

  vector<SparseElement> matrix;
        
  vector<Matrix::index_type> index1;      
  vector<Matrix::index_type> index2;      
  vector<double> values;
                    
  // Loop over all elements in the resistor model
  srmesh->begin(eit);
  srmesh->end(eit_end);  
  
  srmesh->get_nodes(nodes,*eit);
  
  matrix.resize(4*nodes.size()*num_srelems);

  while (eit != eit_end)
  {
    srmesh->get_nodes(nodes,*eit); 
    
    double surface = (srmesh->get_size(*eit))/(nodes.size());
    coefs.resize(nodes.size());
    
    // If data is located on elements get it
    if (srfield->basis_order() == 0)
    {
      double val;
      srfield->get_value(val,*eit);
      for (size_t q=0;q<nodes.size();q++) coefs[q] = surface/val;
    }
    else if (srfield->basis_order() == 1)
    {
      double val;
      for (size_t q=0;q<nodes.size();q++)
      {
        srfield->get_value(val,nodes[q]);
        coefs[q] = surface/val;
      }
    }
    
    // Assume we have not found the surfaces yet
    foundsurf1 = false;
    foundsurf2 = false;
    
    // As we need to compare physical locations, start with getting the node locations
    srmesh->get_centers(points,nodes);
    fenodes1.resize(nodes.size());
    fenodes2.resize(nodes.size());
    
    vector<VMesh::Node::index_type> nodelist;
    femesh->find_closest_nodes(nodelist,points[0],epsilon);

    for (size_t q=0; q<nodelist.size() ;q++)
    {
      femesh->get_delems(delems,nodelist[q]);

      for (size_t r=0; r<delems.size(); r++)
      {
        femesh->get_nodes(fenodes,delems[r]);
        femesh->get_centers(points2,fenodes);

        if (foundsurf1 == false)
        {
          foundsurf1 = true;
          for (size_t t=0; t < points.size(); t++)
          {
            size_t u = 0;
            for (;u< points2.size();u++)
            {
              if ((points[t]-points2[u]).length2() < epsilon2)
              {
                fenodes1[t] = fenodes[u];
                break;
              }
            }
            if (u == points2.size()) 
            {
              foundsurf1 = false;
              break;
            }
          }
          delem1 = delems[r];
        }
        else if (foundsurf2 == false)
        {
          if (delems[r] == delem1) continue;
          foundsurf2 = true;
          for (size_t t=0; t < points.size(); t++)
          {
            size_t u = 0;
            for (;u< points2.size();u++)
            {
              if ((points[t]-points2[u]).length2() < epsilon2)
              {
                fenodes2[t] = fenodes[u];
                break;
              }
            }
            if (u == points2.size()) 
            {
              foundsurf2 = false;
              break;
            }
          }
          delem2 = delems[r];
        }
      }
    }
    
    if (foundsurf1 == true && foundsurf2 == false)
    {
      if (delemlinkrr && nodelinkrr)
      {
        if (delemlinkrr[delem1] < delemlinkrr[delem1+1])
        {
          delem2 = delemlinkcc[delemlinkrr[delem1]];
          femesh->get_nodes(fenodes,delem2);

          foundsurf2 = true;
          size_t t;
          for (t=0 ;t<fenodes.size(); t++)
          {
            bool found_node = false;
            for (size_t u=nodelinkrr[fenodes[t]]; u < nodelinkrr[fenodes[t]+1] && found_node == false; u++)
            {
              for (size_t v=0; v<fenodes1.size() && found_node == false; v++)
              { 
                if (fenodes1[v] == nodelinkcc[u])
                {
                  found_node = true;
                  fenodes2[v] = fenodes[t];
                }
              }
            }
            if (found_node == false) break;              
          }
          if (t == fenodes.size()) foundsurf2 = true;
        }  
      }
    }
 
    if (foundsurf1 == false || foundsurf2 == false)
    {
      error("Could not find two surfaces to connect the surface resistor to");
      algo_end(); return (false);
    }
   
    for (size_t q=0; q< fenodes1.size(); q++)
    {
      SparseElement el;
      el.row = fenodes1[q];
      el.column = fenodes1[q];
      el.value = coefs[q];
      matrix.push_back(el);
      
      el.row = fenodes2[q];
      el.column = fenodes2[q];
      el.value = coefs[q];
      matrix.push_back(el);

      el.row = fenodes1[q];
      el.column = fenodes2[q];
      el.value = -coefs[q];
      matrix.push_back(el);

      el.row = fenodes2[q];
      el.column = fenodes1[q];
      el.value = -coefs[q];
      matrix.push_back(el);
    }
    ++eit;
  }
  
  std::sort(matrix.begin(),matrix.end());
  
  Matrix::index_type s = 0;
  size_type nnz = 0;
  size_type nelems = static_cast<size_type>(matrix.size());
  while (s< nelems && matrix[s].value == 0.0) s++;
  
  for (index_type r=1; r< nelems; r++)
  {
    if (matrix[s] == matrix[r])
    {
      matrix[s].value += matrix[r].value;
      matrix[r].value = 0.0;
    }
    else
    {
      if (matrix[r].value)
      {
        nnz++;
        s = r;
      }
    }
  }
  
  if (nelems) if (matrix[s].value) nnz++;
  
  index_type *rr = new index_type[num_fenodes+1];
  index_type *cc = new index_type[nnz];
  double *vv = new double[nnz];
  
  if ((rr == 0) || (cc == 0) || (vv == 0))
  {
    if (rr) delete[] rr;
    if (cc) delete[] cc;
    if (vv) delete[] vv;
    error("Could not allocate output matrix");
    algo_end(); return (false);
  }

  rr[0] = 0;
  index_type q = 0;
  size_type kk = 0;
  for( index_type p=0; p < num_fenodes; p++ )
  {
    while ((kk < nelems) && (matrix[kk].row == p))
    {
      if (matrix[kk].value)
      {
        cc[q] = matrix[kk].column; vv[q] = matrix[kk].value; q++;
      } 
      kk++; 
    }
    rr[p+1] = q;
  }   
    
  SurfaceResistorMatrix = new SparseRowMatrix(num_fenodes,num_fenodes,rr,cc,nnz,vv);
  algo_end(); return (true);
}

} // end namespace SCIRunAlgo
