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


#ifndef CORE_ALGORITHMS_MATH_BUILDFEMATRIX_H
#define CORE_ALGORITHMS_MATH_BUILDFEMATRIX_H 1

#include <Core/Algorithms/Util/DynamicAlgo.h>
#include <Core/Datatypes/MatrixOperations.h>
#include <Core/Thread/Barrier.h>
#include <Core/Thread/Thread.h>

#include <Core/Geometry/Point.h>
#include <Core/Geometry/Tensor.h>
#include <Core/Basis/Locate.h>

#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/Matrix.h>

#include <sgi_stl_warnings_off.h>
#include <string>
#include <vector>
#include <algorithm>
#include <sgi_stl_warnings_on.h>

#include <Core/Algorithms/Math/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SCISHARE BuildFEMatrixAlgo : public DynamicAlgoBase
{
public:
  virtual bool BuildFEMatrix(ProgressReporter *pr, FieldHandle input, MatrixHandle& output, MatrixHandle& ctable, int numproc = 1);


};

template <class FIELD>
class BuildFEMatrixAlgoT : public BuildFEMatrixAlgo
{
public:
  virtual bool BuildFEMatrix(ProgressReporter *pr, FieldHandle input, MatrixHandle& output, MatrixHandle& ctable, int numproc = 1);
  
};

template <class FIELD> class FEMBuilder;


// --------------------------------------------------------------------------
// This piece of code was adapted from BuildFEMatrix.h
// Code has been modernized a little to meet demands.

template <class FIELD>
class FEMBuilder : public DynamicAlgoBase
{
public:

  // Constructor needed as Barrier needs to have name
  FEMBuilder(ProgressReporter* pr) :
    pr_(pr),
    barrier_("FEMBuilder Barrier")
  {
  }

  // Local entry function for none pure function.
  void build_matrix(FieldHandle input, MatrixHandle& output, MatrixHandle& ctable, int numproc);

private:

  // For parallel implementation
  ProgressReporter* pr_;
  Barrier barrier_;
	std::vector<bool> success_;

  typename FIELD::mesh_type::basis_type fieldbasis_;
  typename FIELD::mesh_handle_type meshhandle_;
  typename FIELD::mesh_type* mesh_;

  FieldHandle meshfield_;
  FIELD *fieldptr_;

  MatrixHandle fematrixhandle_;
  SparseRowMatrix* fematrix_;

  int numprocessors_;
  Matrix::index_type* rows_;
  Matrix::index_type* allcols_;
  std::vector<Matrix::index_type> colidx_;

  Matrix::index_type domain_dimension;

  Matrix::index_type local_dimension_nodes;
  Matrix::index_type local_dimension_add_nodes;
  Matrix::index_type local_dimension_derivatives;
  Matrix::index_type local_dimension;

  Matrix::index_type global_dimension_nodes;
  Matrix::index_type global_dimension_add_nodes;
  Matrix::index_type global_dimension_derivatives;
  Matrix::index_type global_dimension; 

  // A copy of the tensors list that was generated by SetConductivities
  std::vector<std::pair<string, Tensor> > tensors_;

  bool regular_;

  // Entry point for the parallel version
  void parallel(int proc);
    
private:
  
  // General case where we can indexed or non indexed data
  template<class T>
  inline Tensor get_tensor(T& val) const
  {
    if (tensors_.size() == 0) return(Tensor(static_cast<double>(val)));
    return (tensors_[static_cast<size_t>(val)].second);
  }

  // Specific case for when we have a tensor as datatype
  inline Tensor get_tensor(const Tensor& val) const
  {
    return (val);
  }

  inline void add_lcl_gbl(Matrix::index_type row, const std::vector<Matrix::index_type> &cols, const std::vector<double> &lcl_a)
  {
    for (size_t i = 0; i < lcl_a.size(); i++) fematrix_->add(row, cols[i], lcl_a[i]);
  }

private:

  void create_numerical_integration(std::vector<std::vector<double> > &p,std::vector<double> &w,std::vector<std::vector<double> > &d);
  void build_local_matrix(typename FIELD::mesh_type::Elem::index_type c_ind,Matrix::index_type row, std::vector<double> &l_stiff,std::vector<std::vector<double> > &p,std::vector<double> &w,std::vector<std::vector<double> >  &d);
  void build_local_matrix_regular(typename FIELD::mesh_type::Elem::index_type c_ind,Matrix::index_type row, std::vector<double> &l_stiff,std::vector<std::vector<double> > &p,std::vector<double> &w,std::vector<std::vector<double> >  &d, std::vector<std::vector<double> > &precompute);
  void setup();
  
};



template <class FIELD>
bool BuildFEMatrixAlgoT<FIELD>::BuildFEMatrix(ProgressReporter *pr, FieldHandle input, MatrixHandle& output, MatrixHandle& ctable, int numproc)
{
  // Some sanity checks
  FIELD* field = dynamic_cast<FIELD *>(input.get_rep());
  if (field == 0)
  {
    pr->error("BuildFEMatrix: Could not obtain input field");
    return (false);
  }

  if (ctable.get_rep())
  {
    if ((ctable->ncols() != 1)&&(ctable->ncols() != 6)&&(ctable->ncols() != 9))
    {
      pr->error("BuildFEMatrix: Conductivity table needs to have 1, 6, or 9 columns");
      return (false);
    } 
    if (ctable->nrows() == 0)
    { 
      pr->error("BuildFEMatrix: ConductivityTable is empty");
      return (false);
    }
  }

  // We build a second class to do the building of the FEMatrix so we can use
  // class variables to use in the multi threaded applications. This way the 
  // original main class is pure and we can use one instance to do multiple
  // computations. This helps when precomputing dynamic algorithms

  Handle<FEMBuilder<FIELD> > builder = scinew FEMBuilder<FIELD>(pr);
  // Call the the none pure version
  builder->build_matrix(input,output,ctable,numproc);

  if (output.get_rep() == 0)
  {    
    pr->error("BuildFEMatrix: Could not build output matrix");
    return (false);
  }
  
  return (true);
}


template <class FIELD>
void FEMBuilder<FIELD>::build_matrix(FieldHandle input, MatrixHandle& output, MatrixHandle& ctable, int numproc)
{
  // Retrieve useful properly typed pointers from the input data
  fieldptr_ = dynamic_cast<FIELD *>(input.get_rep());
  meshhandle_ = fieldptr_->get_typed_mesh();

  // Determine the number of processors to use:
  int np = Thread::numProcessors();
  // if (np > 5) np = 5;  // Never do more than 5 processors automatically
  
  // Get the number of processors from the user input
  // If they specify a number use it if 0 ignore it
  if (numproc > 0) { np = numproc; }
  numprocessors_ = np;
  
  // If we have the Conductivity property use it, if not we assume the values on
  // the data to be the actual tensors.
  fieldptr_->get_property("conductivity_table",tensors_);
  
  // We added a second system of adding a conductivity table, using a matrix
  // Convert that matrix in the conducivity table
  if (ctable.get_rep())
  {
    tensors_.clear();
    DenseMatrix* mat = ctable->dense();
    MatrixHandle temphandle = mat;
    // Only if we can convert it into a denso matrix, otherwise skip it
    if (mat)
    {
      double* data = mat->get_data_pointer();
      Matrix::size_type m = mat->nrows();
      Matrix::size_type n = mat->ncols();
      Tensor T; 

      // Case the table has isotropic conductivities
      if (mat->ncols() == 1)
      {
        for (Matrix::size_type p=0; p<m;p++)
        {
          // Set the diagonals to the proper version.
          T.mat_[0][0] = data[p*n+0];
          T.mat_[1][0] = 0.0;
          T.mat_[2][0] = 0.0;
          T.mat_[0][1] = 0.0;
          T.mat_[1][1] = data[p*n+0];
          T.mat_[2][1] = 0.0;
          T.mat_[0][2] = 0.0;
          T.mat_[1][2] = 0.0;
          T.mat_[2][2] = data[p*n+0];
          tensors_.push_back(std::pair<string, Tensor>("",T));
        }
      }
       
      // Use our compressed way of storing tensors 
      if (mat->ncols() == 6)
      {
        for (Matrix::size_type p=0; p<m;p++)
        {
          T.mat_[0][0] = data[0+p*n];
          T.mat_[1][0] = data[1+p*n];
          T.mat_[2][0] = data[2+p*n];
          T.mat_[0][1] = data[1+p*n];
          T.mat_[1][1] = data[3+p*n];
          T.mat_[2][1] = data[4+p*n];
          T.mat_[0][2] = data[2+p*n];
          T.mat_[1][2] = data[4+p*n];
          T.mat_[2][2] = data[5+p*n];
          tensors_.push_back(std::pair<string, Tensor>("",T));
        }
      }

      // Use the full symmetric tensor. We will make the tensor symmetric here.
      if (mat->ncols() == 9)
      {
        for (Matrix::size_type p=0; p<m;p++)
        {
          T.mat_[0][0] = data[0+p*n];
          T.mat_[1][0] = data[1+p*n];
          T.mat_[2][0] = data[2+p*n];
          T.mat_[0][1] = data[1+p*n];
          T.mat_[1][1] = data[4+p*n];
          T.mat_[2][1] = data[5+p*n];
          T.mat_[0][2] = data[2+p*n];
          T.mat_[1][2] = data[5+p*n];
          T.mat_[2][2] = data[8+p*n];
          tensors_.push_back(std::pair<string, Tensor>("",T));
        }
      }
    }
  }
  
  // Start the multi threaded FE matrix builder.
  Thread::parallel(this, &FEMBuilder<FIELD>::parallel, numprocessors_);

	// Make sure it is symmetric
	MatrixHandle trans = fematrixhandle_->transpose();
	output = 0.5*(trans+fematrixhandle_);
}






template <class FIELD>
void FEMBuilder<FIELD>::create_numerical_integration(std::vector<std::vector<double> > &p,
                                                   std::vector<double> &w,
                                                   std::vector<std::vector<double> > &d)
{
  p.resize(fieldbasis_.GaussianNum);
  w.resize(fieldbasis_.GaussianNum);
  d.resize(fieldbasis_.GaussianNum);

  for(int i=0; i < fieldbasis_.GaussianNum; i++)
  {
    w[i] = fieldbasis_.GaussianWeights[i];

    p[i].resize(domain_dimension);
    for(int j=0; j<domain_dimension; j++)
      p[i][j]=fieldbasis_.GaussianPoints[i][j];

    d[i].resize(local_dimension*3);
    fieldbasis_.get_derivate_weights(p[i], (double *)&d[i][0]);
  }
}


//! build line of the local stiffness matrix
template <class FIELD>
void FEMBuilder<FIELD>::build_local_matrix(typename FIELD::mesh_type::Elem::index_type c_ind,
                                            Matrix::index_type row, std::vector<double> &l_stiff,
                                            std::vector<std::vector<double> > &p,
                                            std::vector<double> &w,
                                            std::vector<std::vector<double> >  &d)
{
  Tensor T = get_tensor(fieldptr_->value(c_ind));

  double Ca = T.mat_[0][0];
  double Cb = T.mat_[0][1];
  double Cc = T.mat_[0][2];
  double Cd = T.mat_[1][1];
  double Ce = T.mat_[1][2];
  double Cf = T.mat_[2][2];

  if ((Ca==0)&&(Cb==0)&&(Cc==0)&&(Cd==0)&&(Ce==0)&&(Cf==0))
  {
    for (int j = 0; j<local_dimension; j++)
    {
      l_stiff[j] = 0.0;
    }
  }
  else
  {
    for(int i=0; i<local_dimension; i++)
      l_stiff[i] = 0.0;

    int local_dimension2=2*local_dimension;

    double vol;
    const int dim = meshhandle_->dimensionality();
    if (dim == 3)
    {
      vol = meshhandle_->get_basis().volume();  
    }
    else if (dim == 2)
    {
      // We need to get the proper factor, a surface has zero volume, so we need to
      // get the area. The Jacobian is made in such a way that its determinant
      // is a area ratio factor 
      vol = meshhandle_->get_basis().area(0);
    }
    else
    {
      // We need to get the proper factor, a curve has zero volume, so we need to
      // get the length. The Jacobian is made in such a way that its determinant
      // is a length ratio factor 
      vol = meshhandle_->get_basis().length(0);  
    }

    ASSERT(dim >=1 && dim <=3);
    for (size_t i = 0; i < d.size(); i++)
    {
      double Ji[9];
      double detJ = meshhandle_->inverse_jacobian(p[i],c_ind,Ji);   
      
      ASSERT(detJ > 0);
      // Volume associated with the local Gaussian Quadrature point:
      // weightfactor * Volume Unit element * Volume ratio (real element/unit element)
      detJ*=w[i]*vol;
    
      // Build local stiffness matrix
      // Get the local derivatives of the basis functions in the basis element
      // They are all the same and are thus precomputed in matrix d
      const double *Nxi = &d[i][0];
      const double *Nyi = &d[i][local_dimension];
      const double *Nzi = &d[i][local_dimension2];
      // Gradients associated with the node we are calculating
      const double &Nxip = Nxi[row];
      const double &Nyip = Nyi[row];
      const double &Nzip = Nzi[row];
      // Calculating gradient shape function * inverse Jacobian * volume scaling factor
      const double uxp = detJ*(Nxip*Ji[0]+Nyip*Ji[1]+Nzip*Ji[2]);
      const double uyp = detJ*(Nxip*Ji[3]+Nyip*Ji[4]+Nzip*Ji[5]);
      const double uzp = detJ*(Nxip*Ji[6]+Nyip*Ji[7]+Nzip*Ji[8]);
      // Matrix multiplication with conductivity tensor :
      const double uxyzpabc = uxp*Ca+uyp*Cb+uzp*Cc;
      const double uxyzpbde = uxp*Cb+uyp*Cd+uzp*Ce;
      const double uxyzpcef = uxp*Cc+uyp*Ce+uzp*Cf;
    
      // The above is constant for this node. Now multiply with the weight function
      // We assume the weight factors are the same as the local gradients 
      // Galerkin approximation:
       
      for (int j = 0; j<local_dimension; j++)
      {
        const double &Nxj = Nxi[j];
        const double &Nyj = Nyi[j];
        const double &Nzj = Nzi[j];
    
        // Matrix multiplication Gradient with inverse Jacobian:
        const double ux = Nxj*Ji[0]+Nyj*Ji[1]+Nzj*Ji[2];
        const double uy = Nxj*Ji[3]+Nyj*Ji[4]+Nzj*Ji[5];
        const double uz = Nxj*Ji[6]+Nyj*Ji[7]+Nzj*Ji[8];
        
        // Add everything together into one coeffiecient of the matrix
        l_stiff[j] += ux*uxyzpabc+uy*uxyzpbde+uz*uxyzpcef;
      }
    }
  }
}




template <class FIELD>
void FEMBuilder<FIELD>::build_local_matrix_regular(typename FIELD::mesh_type::Elem::index_type c_ind,
                                            Matrix::index_type row, std::vector<double> &l_stiff,
                                            std::vector<std::vector<double> > &p,
                                            std::vector<double> &w,
                                            std::vector<std::vector<double> >  &d,
                                            std::vector<std::vector<double> > &precompute)
{
  Tensor T = get_tensor(fieldptr_->value(c_ind));

  double Ca = T.mat_[0][0];
  double Cb = T.mat_[0][1];
  double Cc = T.mat_[0][2];
  double Cd = T.mat_[1][1];
  double Ce = T.mat_[1][2];
  double Cf = T.mat_[2][2];

  if ((Ca==0)&&(Cb==0)&&(Cc==0)&&(Cd==0)&&(Ce==0)&&(Cf==0))
  {
    for (int j = 0; j<local_dimension; j++)
    {
      l_stiff[j] = 0.0;
    }
  }
  else
  {
  
    if (precompute.size() == 0)
    {
      precompute.resize(d.size());
      for (int m=0; m < static_cast<int>(d.size()); m++)
      {
        precompute[m].resize(10);
      }
      
      for(int i=0; i<local_dimension; i++)
        l_stiff[i] = 0.0;

      int local_dimension2=2*local_dimension;

      double vol;
      const int dim = meshhandle_->dimensionality();
      if (dim == 3)
      {
        vol = meshhandle_->get_basis().volume();  
      }
      else if (dim == 2)
      {
        // We need to get the proper factor, a surface has zero volume, so we need to
        // get the area. The Jacobian is made in such a way that its determinant
        // is a area ratio factor 
        vol = meshhandle_->get_basis().area(0);
      }
      else
      {
        // We need to get the proper factor, a curve has zero volume, so we need to
        // get the length. The Jacobian is made in such a way that its determinant
        // is a length ratio factor 
        vol = meshhandle_->get_basis().length(0);  
      }

      for (size_t i = 0; i < d.size(); i++)
      {
        std::vector<double>& pc = precompute[i];

        double Ji[9];
        double detJ = meshhandle_->inverse_jacobian(p[i],c_ind,Ji);           
                   
        // Volume elements can return negative determinants if the order of elements
        // is put in a different order
        // TODO: It seems to be that a negative determinant is not necessarily bad, 
        // we should be more flexible on thiis point
        ASSERT(detJ>0);
        
        // Volume associated with the local Gaussian Quadrature point:
        // weightfactor * Volume Unit element * Volume ratio (real element/unit element)
        detJ*=w[i]*vol;
      
        pc[0] = Ji[0];
        pc[1] = Ji[1];
        pc[2] = Ji[2];
        pc[3] = Ji[3];
        pc[4] = Ji[4];
        pc[5] = Ji[5];
        pc[6] = Ji[6];
        pc[7] = Ji[7];
        pc[8] = Ji[8];
        pc[9] = detJ;
        
        // Build local stiffness matrix
        // Get the local derivatives of the basis functions in the basis element
        // They are all the same and are thus precomputed in matrix d
        const double *Nxi = &d[i][0];
        const double *Nyi = &d[i][local_dimension];
        const double *Nzi = &d[i][local_dimension2];
        // Gradients associated with the node we are calculating
        const double &Nxip = Nxi[row];
        const double &Nyip = Nyi[row];
        const double &Nzip = Nzi[row];
        // Calculating gradient shape function * inverse Jacobian * volume scaling factor
        const double uxp = pc[9]*(Nxip*pc[0]+Nyip*pc[1]+Nzip*pc[2]);
        const double uyp = pc[9]*(Nxip*pc[3]+Nyip*pc[4]+Nzip*pc[5]);
        const double uzp = pc[9]*(Nxip*pc[6]+Nyip*pc[7]+Nzip*pc[8]);
        // Matrix multiplication with conductivity tensor :
        const double uxyzpabc = uxp*Ca+uyp*Cb+uzp*Cc;
        const double uxyzpbde = uxp*Cb+uyp*Cd+uzp*Ce;
        const double uxyzpcef = uxp*Cc+uyp*Ce+uzp*Cf;
      
        // The above is constant for this node. Now multiply with the weight function
        // We assume the weight factors are the same as the local gradients 
        // Galerkin approximation:
         
        for (int j = 0; j<local_dimension; j++)
        {
          const double &Nxj = Nxi[j];
          const double &Nyj = Nyi[j];
          const double &Nzj = Nzi[j];
      
          // Matrix multiplication Gradient with inverse Jacobian:
          const double ux = Nxj*pc[0]+Nyj*pc[1]+Nzj*pc[2];
          const double uy = Nxj*pc[3]+Nyj*pc[4]+Nzj*pc[5];
          const double uz = Nxj*pc[6]+Nyj*pc[7]+Nzj*pc[8];
          
          // Add everything together into one coeffiecient of the matrix
          l_stiff[j] += ux*uxyzpabc+uy*uxyzpbde+uz*uxyzpcef;
        }
      }
    }
    else
    {      
      for(int i=0; i<local_dimension; i++)
        l_stiff[i] = 0.0;
     
       int local_dimension2=2*local_dimension;
      
      for (size_t i = 0; i < d.size(); i++)
      {
        std::vector<double>& pc = precompute[i];
      
        // Build local stiffness matrix
        // Get the local derivatives of the basis functions in the basis element
        // They are all the same and are thus precomputed in matrix d
        const double *Nxi = &d[i][0];
        const double *Nyi = &d[i][local_dimension];
        const double *Nzi = &d[i][local_dimension2];
        // Gradients associated with the node we are calculating
        const double &Nxip = Nxi[row];
        const double &Nyip = Nyi[row];
        const double &Nzip = Nzi[row];
        // Calculating gradient shape function * inverse Jacobian * volume scaling factor
        const double uxp = pc[9]*(Nxip*pc[0]+Nyip*pc[1]+Nzip*pc[2]);
        const double uyp = pc[9]*(Nxip*pc[3]+Nyip*pc[4]+Nzip*pc[5]);
        const double uzp = pc[9]*(Nxip*pc[6]+Nyip*pc[7]+Nzip*pc[8]);
        // Matrix multiplication with conductivity tensor :
        const double uxyzpabc = uxp*Ca+uyp*Cb+uzp*Cc;
        const double uxyzpbde = uxp*Cb+uyp*Cd+uzp*Ce;
        const double uxyzpcef = uxp*Cc+uyp*Ce+uzp*Cf;
      
        // The above is constant for this node. Now multiply with the weight function
        // We assume the weight factors are the same as the local gradients 
        // Galerkin approximation:
         
        for (int j = 0; j<local_dimension; j++)
        {
          const double &Nxj = Nxi[j];
          const double &Nyj = Nyi[j];
          const double &Nzj = Nzi[j];
      
          // Matrix multiplication Gradient with inverse Jacobian:
          const double ux = Nxj*pc[0]+Nyj*pc[1]+Nzj*pc[2];
          const double uy = Nxj*pc[3]+Nyj*pc[4]+Nzj*pc[5];
          const double uz = Nxj*pc[6]+Nyj*pc[7]+Nzj*pc[8];
          
          // Add everything together into one coeffiecient of the matrix
          l_stiff[j] += ux*uxyzpabc+uy*uxyzpbde+uz*uxyzpcef;
        }
      }
    }
  }
}


template <class FIELD>
void FEMBuilder<FIELD>::setup()
{
	mesh_ = dynamic_cast<typename FIELD::mesh_type*>(meshhandle_.get_rep());
	
	success_.resize(numprocessors_,true);
  // The domain dimension
  domain_dimension = fieldbasis_.domain_dimension();
  ASSERT(domain_dimension>0);

  local_dimension_nodes = fieldbasis_.number_of_mesh_vertices();
  local_dimension_add_nodes = fieldbasis_.number_of_vertices()-fieldbasis_.number_of_mesh_vertices();
  local_dimension_derivatives = 0;
  
  // Local degrees of freedom per element
  local_dimension = local_dimension_nodes + local_dimension_add_nodes + local_dimension_derivatives; //!< degrees of freedom (dofs) of system
  ASSERT(fieldbasis_.dofs()==local_dimension);

  typename FIELD::mesh_type::Node::size_type mns;
  meshhandle_->size(mns);
  // Number of mesh points (not necessarily number of nodes)
  global_dimension_nodes = mns;
  global_dimension_add_nodes = fieldptr_->get_basis().size_node_values();
  global_dimension_derivatives = fieldptr_->get_basis().size_derivatives();
  global_dimension = global_dimension_nodes+global_dimension_add_nodes+global_dimension_derivatives;

  if (mns > 0) 
	{
		// We only need edges for the higher order basis in case of quatric lagrangian
		// Hence we should only synchronize it for this case
		if (global_dimension_add_nodes > 0) mesh_->synchronize(Mesh::EDGES_E);
		mesh_->synchronize(Mesh::NODE_NEIGHBORS_E);
  }
	else
	{
	  success_[0] = false;
	}
	rows_ = scinew Matrix::index_type[global_dimension+1];
  
  colidx_.resize(numprocessors_+1);
  
  regular_ = false;
  if (mesh_->topology_geometry() == SCIRun::Mesh::REGULAR) regular_ = true;	
}


// -- callback routine to execute in parallel
template <class FIELD>
void FEMBuilder<FIELD>::parallel(int proc_num)
{
  if (proc_num == 0)
  {
    setup();
  }
	
	barrier_.wait(numprocessors_);
	for (int q=0; q<numprocessors_;q++) 
	if (success_[proc_num] == false) return;

	//! distributing dofs among processors
	const Matrix::index_type start_gd = (global_dimension * proc_num)/numprocessors_;
	const Matrix::index_type end_gd  = (global_dimension * (proc_num+1))/numprocessors_;

	//! creating sparse matrix structure
	std::vector<Matrix::index_type> mycols;
	mycols.reserve((end_gd - start_gd)*local_dimension*8);  //<! rough estimate

	typename FIELD::mesh_type::Elem::array_type ca;
	typename FIELD::mesh_type::Node::array_type na;
	typename FIELD::mesh_type::Edge::array_type ea;
	std::vector<int> neib_dofs;

	//! loop over system dofs for this thread
	int cnt = 0;
	Matrix::size_type size_gd = end_gd-start_gd;
		
  try
	{
		for (Matrix::index_type i = start_gd; i<end_gd; i++)
		{
			rows_[i] = mycols.size();

			neib_dofs.clear();
			//! check for nodes
			if (i<global_dimension_nodes)
			{
				//! get neighboring cells for node
				typename FIELD::mesh_type::Node::index_type idx;
				mesh_->to_index(idx,i);
				mesh_->get_elems(ca, idx);
			}
			else if (i<global_dimension_nodes+global_dimension_add_nodes)
			{
				//! check for additional nodes at edges
				//! get neighboring cells for node
				const int ii = i-global_dimension_nodes;
				typename FIELD::mesh_type::Edge::index_type idx;
				typename FIELD::mesh_type::Node::array_type nodes;
				typename FIELD::mesh_type::Elem::array_type elems;
				typename FIELD::mesh_type::Elem::array_type elems2;

				mesh_->to_index(idx,ii);
				mesh_->get_nodes(nodes,idx);
				mesh_->get_elems(elems,nodes[0]);
				mesh_->get_elems(elems2,nodes[1]);
				ca.clear();
				for (size_t v=0; v < elems.size(); v++)
					 for (size_t w=0; w <elems2.size(); w++)
							if (elems[v] == elems2[w]) ca.push_back(elems[v]);
			}
			else
			{
				//! check for derivatives - to do
			}
		
			for(size_t j = 0; j < ca.size(); j++)
			{
				//! get neighboring nodes
				mesh_->get_nodes(na, ca[j]);

				for(size_t k = 0; k < na.size(); k++) 
				{
					neib_dofs.push_back((int)(na[k])); // Must cast to (int) for SGI compiler. :-(
				}

				//! check for additional nodes at edges
				if (global_dimension_add_nodes)
				{
					//! get neighboring edges
					mesh_->get_edges(ea, ca[j]);

					for(size_t k = 0; k < ea.size(); k++)
						neib_dofs.push_back(global_dimension + ea[k]);
				}
			}
		
			std::sort(neib_dofs.begin(), neib_dofs.end());

			for (size_t j=0; j<neib_dofs.size(); j++)
			{
				if (j == 0 || neib_dofs[j] != mycols.back())
				{
					mycols.push_back(neib_dofs[j]);
				}
			}
			if (proc_num == 0) 
			{
				cnt++;
				if (cnt == 200) { cnt = 0; pr_->update_progress(i,2*size_gd); }
			}    
		}

		colidx_[proc_num] = mycols.size();
	  success_[proc_num] = true;
	}
  catch (const Exception &e)
  {
    pr_->error(string("BuildFEMatrix crashed with the following exception:\n")+
	  e.message());
	  success_[proc_num] = false;
  }
  catch (const string a)
  {
    pr_->error(a);
	  success_[proc_num] = false;
  }
  catch (const char *a)
  {
    pr_->error(string(a));
	  success_[proc_num] = false;
  }	
	catch (...)
	{
		pr_->error(string("BuildFEMatrix crashed for unknown reason"));
	  success_[proc_num] = false;
	}
	
  //! check point
  barrier_.wait(numprocessors_);

	// Bail out if one of the processes failed
	for (int q=0; q<numprocessors_;q++) 
		if (success_[proc_num] == false) return;
	
	std::vector<std::vector<double> > precompute;		
	int st = 0;
	
	try
	{
		if (proc_num == 0)
		{
			for(int i=0; i<numprocessors_; i++)
			{
				const Matrix::index_type ns = colidx_[i];
				colidx_[i] = st;
				st += ns;
			}

			colidx_[numprocessors_] = st;
			allcols_ = scinew int[st];
			if (allcols_ == 0)
			{
				throw "Could not allocate enough memory";
				success_[proc_num] = false;					
			}
		}
		success_[proc_num] = true;
	}
  catch (const Exception &e)
  {
    pr_->error(string("BuildFEMatrix crashed with the following exception:\n")+
	  e.message());
	  success_[proc_num] = false;
  }
  catch (const string a)
  {
    pr_->error(a);
	  success_[proc_num] = false;
  }
  catch (const char *a)
  {
    pr_->error(string(a));
	  success_[proc_num] = false;
  }		
	catch (...)
	{
		pr_->error(string("BuildFEMatrix crashed for unknown reason"));
	  success_[proc_num] = false;
	}	
	
  //! check point
  barrier_.wait(numprocessors_);

	// Bail out if one of the processes failed
	for (int q=0; q<numprocessors_;q++) 
		if (success_[proc_num] == false) return;
	
	try
	{
		//! updating global column by each of the processors
		const Matrix::index_type s = colidx_[proc_num];
		const size_t n = mycols.size();

		for(size_t i=0; i<n; i++)
			allcols_[i+s] = mycols[i];

		for(Matrix::index_type i = start_gd; i<end_gd; i++)
			rows_[i] += s;
		
		success_[proc_num] = true;
	}
  catch (const Exception &e)
  {
    pr_->error(string("BuildFEMatrix crashed with the following exception:\n")+
	  e.message());
	  success_[proc_num] = false;
  }
  catch (const string a)
  {
    pr_->error(a);
	  success_[proc_num] = false;
  }
  catch (const char *a)
  {
    pr_->error(string(a));
	  success_[proc_num] = false;
  }		
	catch (...)
	{
		pr_->error(string("BuildFEMatrix crashed for unknown reason"));
	  success_[proc_num] = false;
	}	
	
  //! check point
  barrier_.wait(numprocessors_);

	// Bail out if one of the processes failed
	for (int q=0; q<numprocessors_;q++) 
		if (success_[proc_num] == false) return;
		
	try
	{	
		//! the main thread makes the matrix
		if (proc_num == 0)
		{
			rows_[global_dimension] = st;
			double* vals_ = scinew double[st];
			if (vals_ == 0)
			{
				throw "Could not allocate enough memory";
				success_[proc_num] = false;			
			}
			// for (int p=0; p <st; p++) vals_[p] = 0.0;
			fematrix_ = scinew SparseRowMatrix(global_dimension, global_dimension, rows_, allcols_, st,vals_);
			fematrixhandle_ = fematrix_;
		}
		success_[proc_num] = true;
	}
  catch (const Exception &e)
  {
    pr_->error(string("BuildFEMatrix crashed with the following exception:\n")+
	  e.message());
	  success_[proc_num] = false;
  }
  catch (const string a)
  {
    pr_->error(a);
	  success_[proc_num] = false;
  }
  catch (const char *a)
  {
    pr_->error(string(a));
	  success_[proc_num] = false;
  }		
	catch (...)
	{
		pr_->error(string("BuildFEMatrix crashed for unknown reason"));
	  success_[proc_num] = false;
	}	

  //! check point
  barrier_.wait(numprocessors_);

	// Bail out if one of the processes failed
	for (int q=0; q<numprocessors_;q++) 
		if (success_[proc_num] == false) return;

	try
	{
		//! zeroing in parallel
		const Matrix::index_type ns = colidx_[proc_num];
		const Matrix::index_type ne = colidx_[proc_num+1];
		double* a = &(fematrix_->a[ns]), *ae=&(fematrix_->a[ne]);
		while (a<ae) *a++=0.0;

		std::vector<std::vector<double> > ni_points;
		std::vector<double> ni_weights;
		std::vector<std::vector<double> > ni_derivatives;
		create_numerical_integration(ni_points, ni_weights, ni_derivatives);

		std::vector<double> lsml; //!< line of local stiffnes matrix
		lsml.resize(local_dimension);
					
		//! loop over system dofs for this thread
		cnt = 0;
		
		size_gd = end_gd-start_gd;
		for (Matrix::index_type i = start_gd; i<end_gd; i++)
		{
			if (i < global_dimension_nodes)
			{
				//! check for nodes
				//! get neighboring cells for node
				typename FIELD::mesh_type::Node::index_type idx;
				meshhandle_->to_index(idx,i);
				meshhandle_->get_elems(ca,idx);
			}
			else if (i < global_dimension_nodes + global_dimension_add_nodes)
			{
				//! check for additional nodes at edges
				//! get neighboring cells for additional nodes
				const Matrix::index_type ii=i-global_dimension_nodes;
				typename FIELD::mesh_type::Edge::index_type idx;
				typename FIELD::mesh_type::Node::array_type nodes;
				typename FIELD::mesh_type::Elem::array_type elems;
				typename FIELD::mesh_type::Elem::array_type elems2;

				meshhandle_->to_index(idx,ii);
				meshhandle_->get_nodes(nodes,idx);
				meshhandle_->get_elems(elems,nodes[0]);
				meshhandle_->get_elems(elems2,nodes[1]);
				ca.clear();
				for (size_t v=0; v < elems.size(); v++)
					 for (size_t w=0; w <elems2.size(); w++)
							if (elems[v] == elems2[w]) ca.push_back(elems[v]);

			}
			else
			{
				//! check for derivatives - to do
			}
		
			//! loop over elements attributed elements
			
			if (regular_)
			{
				for (size_t j = 0; j < ca.size(); j++)
				{
					meshhandle_->get_nodes(na, ca[j]); //!< get neighboring nodes
					neib_dofs.resize(na.size());
					for(size_t k = 0; k < na.size(); k++)
					{
						neib_dofs[k] = na[k]; // Must cast to (int) for SGI compiler :-(
					}
					
					for(size_t k = 0; k < na.size(); k++)
					{
						if (na[k] == i) 
						{
							build_local_matrix_regular(ca[j], k , lsml, ni_points, ni_weights, ni_derivatives,precompute);
							add_lcl_gbl(i, neib_dofs, lsml);
						}
					}
				}
			}
			else
			{
				for (size_t j = 0; j < ca.size(); j++)
				{
					neib_dofs.clear();
					meshhandle_->get_nodes(na, ca[j]); //!< get neighboring nodes
					for(size_t k = 0; k < na.size(); k++)
					{
						neib_dofs.push_back(na[k]); // Must cast to (int) for SGI compiler :-(
					}
					//! check for additional nodes at edges
					if (global_dimension_add_nodes)
					{
						meshhandle_->get_edges(ea, ca[j]); //!< get neighboring edges
						for(size_t k = 0; k < ea.size(); k++)
						{
							neib_dofs.push_back(global_dimension + ea[k]);
						}
					}
					
					ASSERT((int)neib_dofs.size() == local_dimension);
					for(size_t k = 0; k < na.size(); k++)
					{
						if (na[k] == i) 
						{
							build_local_matrix(ca[j], k , lsml, ni_points, ni_weights, ni_derivatives);
							add_lcl_gbl(i, neib_dofs, lsml);
						}
					}

					if (global_dimension_add_nodes)
					{
						for(size_t k = 0; k < ea.size(); k++)
						{
							if (global_dimension + (int)ea[k] == i)
							{
								build_local_matrix(ca[j], k+(int)na.size() , lsml, ni_points, ni_weights, ni_derivatives);
								add_lcl_gbl(i, neib_dofs, lsml);
							}
						}
					}
				}
			}
			
			if (proc_num == 0) 
			{
				cnt++;
				if (cnt == 200) { cnt = 0; pr_->update_progress(i+size_gd,2*size_gd); }
			}
		}
		success_[proc_num] = true;
	}
  catch (const Exception &e)
  {
    pr_->error(string("BuildFEMatrix crashed with the following exception:\n")+
	  e.message());
	  success_[proc_num] = false;
  }
  catch (const string a)
  {
    pr_->error(a);
	  success_[proc_num] = false;
  }
  catch (const char *a)
  {
    pr_->error(string(a));
	  success_[proc_num] = false;
  }	
	
	catch (...)
	{
		pr_->error(string("BuildFEMatrix crashed for unknown reason"));
	  success_[proc_num] = false;
	}	

  barrier_.wait(numprocessors_);
	
	// Bail out if one of the processes failed
	for (int q=0; q<numprocessors_;q++) 
		if (success_[proc_num] == false) return;	
}

} // end namespace SCIRun

#endif 