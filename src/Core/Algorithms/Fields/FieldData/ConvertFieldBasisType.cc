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

#include <Core/Algorithms/Fields/FieldData/ConvertFieldBasisType.h>
#include <Core/Datatypes/FieldInformation.h>

#include <Core/Datatypes/SparseRowMatrix.h>

namespace SCIRunAlgo {

bool
ConvertFieldBasisTypeAlgo::
run(FieldHandle input, FieldHandle& output, MatrixHandle& mapping)
{
  algo_start("ConvertFieldBasis",false);
  
  //! Safety check
  if (input.get_rep() == 0)
  {
    error("No input field");
    algo_end(); return (false);
  }
  
  //! Get the information of the input field
  FieldInformation fo(input);
  
  std::string basistype;
  get_option("basistype",basistype);
  
  int basis_order = input->vfield()->basis_order();
  
  if (basistype == "nodata")
  {
    mapping = 0;
    warning("Could not generate a mapping matrix for field with no data");
    if (basis_order == -1)
    {
      // Field is already no data
      output = input;
      algo_end(); return (true);
    }
    
    fo.make_nodata();
    output = CreateField(fo,input->mesh());
    
    if (output.get_rep() == 0)
    {
      error("Could not create output field");
    }
    
    algo_end(); return (true);
  }


  if (basistype == "constant")
  {
    if (basis_order == -1)
    {
      warning("Could not generate a mapping matrix for field with no data");
      mapping = 0;
    }

    VMesh* mesh    = input->vmesh();
    VField* ifield = input->vfield();
    VMesh::size_type num_values = ifield->num_values();

    if (basis_order == 0)
    {
      // Field is already no data
      output = input;
      mapping = SparseRowMatrix::identity(num_values);
      algo_end(); return (true);
    }

    fo.make_constantdata();
    output = CreateField(fo,input->mesh());
    
    VField* ofield = output->vfield();
    
    VMesh::coords_type center;
    mesh->get_element_center(center);
    
    VMesh::size_type num_elems = mesh->num_elems();
    VMesh::size_type num_nodes = mesh->num_nodes();
    VMesh::size_type num_nodes_per_elem = mesh->num_nodes_per_elem();
    
    if (basis_order == 1)
    {
    
      Matrix::index_type* rows = 
        new Matrix::index_type[num_elems+1];
      Matrix::index_type* columns = 
        new Matrix::index_type[num_elems*num_nodes_per_elem];
      double* values = 
        new double[num_elems*num_nodes_per_elem];

      if ((rows==0)||(columns==0)||(values==0))
      {
        error("Could not allocate memory for mapping");
        algo_end(); return (false);
      }

      std::vector<double> weights;
      mesh->get_weights(center,weights,1);
      VMesh::Node::array_type nodes;
      
      Matrix::index_type k=0;  
      for(VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
      {
        mesh->get_nodes(nodes,idx);
        for(size_t j=0; j<num_nodes_per_elem; j++)
        {
          columns[k] = nodes[j];
          values[k]  = weights[j]; 
          k++;
        }
        rows[idx] = idx*num_nodes_per_elem;
      }
      
      rows[num_elems] = num_elems*num_nodes_per_elem;

      mapping = new SparseRowMatrix(num_elems,num_nodes,rows,columns,
                                num_nodes,values,true);
      algo_end(); return (true);
    }

    if (basis_order == 2)
    {
      mesh->synchronize_edges();
      VMesh::size_type num_edges = mesh->num_edges();
      VMesh::size_type num_edges_per_elem = mesh->num_edges_per_elem();

      Matrix::index_type* rows = 
        new Matrix::index_type[num_elems+1];
      Matrix::index_type* columns = 
        new Matrix::index_type[num_elems*(num_nodes_per_elem+num_edges_per_elem)];
      double* values = 
        new double[num_elems*(num_nodes_per_elem+num_edges_per_elem)];
        
      if ((rows==0)||(columns==0)||(values==0))
      {
        error("Could not allocate memory for mapping");
        algo_end(); return (false);
      }
        
      std::vector<double> weights;
      mesh->get_weights(center,weights,2);
      VMesh::Node::array_type nodes;
      VMesh::Edge::array_type edges;
                
      Matrix::index_type k=0;  
      for(VMesh::Elem::index_type idx=0; idx<num_elems; idx++)
      {
        mesh->get_nodes(nodes,idx);
        mesh->get_edges(edges,idx);
        for(size_t j=0; j<num_nodes_per_elem; j++)
        {
          columns[k] = nodes[j];
          values[k]  = weights[j]; 
          k++;
        }

        for(size_t j=0; j<num_edges_per_elem; j++)
        {
          columns[k] = edges[j]+num_nodes;
          values[k]  = weights[j+num_nodes_per_elem]; 
          k++;
        }

        rows[idx] = idx*(num_nodes_per_elem+num_edges_per_elem);
      }
      rows[num_values] = num_elems*(num_nodes_per_elem+num_edges_per_elem);

      mapping = new SparseRowMatrix(num_elems,num_nodes+num_edges,rows,columns,
                                num_nodes+num_edges,values,true);
      algo_end(); return (true);
    }  
  }


  if (basistype == "linear")
  {
    if (basis_order == -1)
    {
      warning("Could not generate a mapping matrix for field with no data");
      mapping = 0;
    }

    VMesh* mesh    = input->vmesh();
    VField* ifield = input->vfield();
    VMesh::size_type num_values = ifield->num_values();

    if (basis_order == 1)
    {
      // Field is already no data
      output = input;
      mapping = SparseRowMatrix::identity(num_values);
      algo_end(); return (true);
    }

    fo.make_lineardata();
    output = CreateField(fo,input->mesh());
    
    VField* ofield = output->vfield();
    
    VMesh::coords_type center;
    mesh->get_element_center(center);
    
    VMesh::size_type num_elems = mesh->num_elems();
    VMesh::size_type num_nodes = mesh->num_nodes();
    
    if (basis_order == 0)
    {
      Matrix::index_type* rows = new Matrix::index_type[num_nodes+1];
      std::vector<Matrix::index_type> cols;

      if (rows==0)
      {
        error("Could not allocate memory for mapping");
        algo_end(); return (false);
      }
    
      VMesh::Elem::array_type elems;
      
      mesh->synchronize_node_neighbors();
      
      Matrix::index_type k=0; 
      rows[0] = 0; 
      for(VMesh::Node::index_type idx=0; idx<num_nodes; idx++)
      {
        mesh->get_elems(elems,idx);
        for(size_t j=0; j<elems.size(); j++) { cols.push_back(elems[j]); k++; }
        rows[idx+1] = rows[idx]+elems.size();  
      }

      Matrix::index_type* columns = new Matrix::index_type[cols.size()];
      double* values = new double[cols.size()];

      for(VMesh::index_type j=0; j<num_nodes; j++)
      {
        for(VMesh::index_type r=rows[j]; r < rows[j+1]; r++)
        {
          double weight = 1.0/static_cast<double>(rows[j+1]-rows[j]);
          columns[r] = cols[r];
          values[r] = weight;
        }
      }
      
      mapping = new SparseRowMatrix(num_nodes,num_elems,rows,columns,
                                num_elems,values,true);
      algo_end(); return (true);
    }

    if (basis_order == 2)
    {
      mesh->synchronize_edges();
      VMesh::size_type num_edges = mesh->num_edges();

      Matrix::index_type* rows = new Matrix::index_type[num_nodes+1];
      Matrix::index_type* columns = new Matrix::index_type[num_nodes];
      double* values = new double[num_nodes];
        
      if ((rows==0)||(columns==0)||(values==0))
      {
        error("Could not allocate memory for mapping");
        algo_end(); return (false);
      }
                        
      for(VMesh::index_type idx=0; idx<num_nodes; idx++)
      {
        columns[idx] = idx;
        values[idx]  = 1.0; 
        rows[idx] = idx;
      }
      rows[num_nodes] = num_nodes;

      mapping = new SparseRowMatrix(num_nodes,num_nodes+num_edges,rows,columns,
                                num_nodes+num_edges,values,true);
      algo_end(); return (true);
    }  
  }

  if (basistype == "quadratic")
  {
    if (basis_order == -1)
    {
      warning("Could not generate a mapping matrix for field with no data");
      mapping = 0;
    }

    VMesh* mesh    = input->vmesh();
    VField* ifield = input->vfield();
    VMesh::size_type num_values = ifield->num_values();

    if (basis_order == 0)
    {
      // Field is already no data
      output = input;
      mapping = SparseRowMatrix::identity(num_values);
      algo_end(); return (true);
    }

    fo.make_quadraticdata();
    output = CreateField(fo,input->mesh());
    
    VField* ofield = output->vfield();
    
    VMesh::coords_type center;
    mesh->get_element_center(center);
    
    mesh->synchronize_edges();

    VMesh::size_type num_elems = mesh->num_elems();
    VMesh::size_type num_nodes = mesh->num_nodes();
    VMesh::size_type num_edges = mesh->num_edges();
    
    if (basis_order == 0)
    {
      Matrix::index_type* rows = new Matrix::index_type[num_nodes+num_edges+1];
      std::vector<Matrix::index_type> cols;

      if (rows==0)
      {
        error("Could not allocate memory for mapping");
        algo_end(); return (false);
      }
    
      VMesh::Elem::array_type elems;
      
      mesh->synchronize_node_neighbors();
      
      Matrix::index_type k=0;
      rows[0] = 0; 
      for(VMesh::Node::index_type idx=0; idx<num_nodes; idx++)
      {
        mesh->get_elems(elems,idx);
        for(size_t j=0; j<elems.size(); j++) { cols.push_back(elems[j]); k++; }
        rows[idx+1] = rows[idx]+elems.size();  
      }

      for(VMesh::Edge::index_type idx=0; idx<num_edges; idx++)
      {
        mesh->get_elems(elems,idx);
        for(size_t j=0; j<elems.size(); j++) { cols.push_back(elems[j]); k++; }
        rows[num_nodes+idx+1] = rows[num_nodes+idx]+elems.size();  
      }

      Matrix::index_type* columns = new Matrix::index_type[cols.size()];
      double* values = new double[cols.size()];

      for(VMesh::index_type j=0; j<num_nodes; j++)
      {
        for(VMesh::index_type r=rows[j]; r < rows[j+1]; r++)
        {
          double weight = 1.0/static_cast<double>(rows[j+1]-rows[j]);
          columns[r] = cols[r];
          values[r] = weight;
        }
      }
      
      mapping = new SparseRowMatrix(num_nodes+num_edges,num_elems,rows,
                                columns,num_elems,values,true);
      algo_end(); return (true);
    }

    if (basis_order == 1)
    {
      Matrix::index_type* rows = new Matrix::index_type[num_nodes+num_edges+1];
      Matrix::index_type* columns = new Matrix::index_type[num_nodes+2*num_edges];
      double* values = new double[num_nodes+2*num_edges];
        
      if ((rows==0)||(columns==0)||(values==0))
      {
        error("Could not allocate memory for mapping");
        algo_end(); return (false);
      }
        
      for(VMesh::index_type idx=0; idx<num_nodes; idx++)
      {
        columns[idx] = idx;
        values[idx]  = 1.0; 
        rows[idx] = idx;
      }
      rows[num_nodes] = num_nodes;
        
     VMesh::Node::array_type nodes;   
        
      for(VMesh::Edge::index_type idx=0; idx<num_edges; idx++)
      {
        mesh->get_nodes(nodes,idx);
        columns[2*idx+num_nodes] = nodes[0];
        columns[2*idx+num_nodes+1] = nodes[1];
        values[2*idx+num_nodes]  = 0.5; 
        values[2*idx+num_nodes+1]  = 0.5; 
        rows[idx+num_nodes] = 2*idx+num_nodes;
      }
      rows[num_nodes+num_edges] = num_nodes+2*num_edges;

      mapping = new SparseRowMatrix(num_nodes+num_edges,num_nodes,rows,columns,
                                num_nodes,values,true);
      algo_end(); return (true);
    }  
  }

}

} // end namespace SCIRunAlgo
