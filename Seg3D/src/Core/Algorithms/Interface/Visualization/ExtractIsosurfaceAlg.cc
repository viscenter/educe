//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2007 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  License for the specific language governing rights and limitations under
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#include <iostream>
#include <Core/Events/DataManager.h>
#include <Core/Algorithms/Interface/Visualization/ExtractIsosurfaceAlg.h>
#include <Core/Algorithms/Visualization/ExtractIsosurface.h>
#include <Core/Algorithms/Visualization/MarchingCubes.h>
#include <Core/Algorithms/Visualization/Noise.h>
#include <Core/Algorithms/Visualization/Sage.h>
#include <Core/Algorithms/Visualization/TetMC.h>
#include <Core/Algorithms/Visualization/HexMC.h>
#include <Core/Geom/GeomGroup.h>
#include <Core/Geom/GeomMaterial.h>

namespace SCIRun {

ExtractIsosurfaceAlg* get_extract_isosurface_alg()
{
  return new ExtractIsosurfaceAlg();
}

static MatrixHandle
append_sparse_matrices(vector<MatrixHandle> &matrices)
{
  unsigned int i;
  int j;

  int ncols = matrices[0]->ncols();
  int nrows = 0;
  int nnz = 0;
  for (i = 0; i < matrices.size(); i++) {
    SparseRowMatrix *sparse = matrices[i]->sparse();
    nrows += sparse->nrows();
    nnz += sparse->nnz;
  }

  int *rr = new int[nrows+1];
  int *cc = new int[nnz];
  double *dd = new double[nnz];

  int offset = 0;
  int nnzcounter = 0;
  int rowcounter = 0;
  for (i = 0; i < matrices.size(); i++) {
    SparseRowMatrix *sparse = matrices[i]->sparse();
    for (j = 0; j < sparse->nnz; j++) {
      cc[nnzcounter] = sparse->columns[j];
      dd[nnzcounter] = sparse->a[j];
      nnzcounter++;
    }
    const int snrows = sparse->nrows();
    for (j = 0; j <= snrows; j++) {
      rr[rowcounter] = sparse->rows[j] + offset;
      rowcounter++;
    }
    rowcounter--;
    offset += sparse->rows[snrows];
  }

  return new SparseRowMatrix(nrows, ncols, rr, cc, nnz, dd);
}

//! Algorithm Interface.
vector<size_t>
ExtractIsosurfaceAlg::execute(size_t field_id, size_t cmap_id,
			   size_t Optional_Isovalues)
{
  vector<size_t> rval(3,0);
  DataManager *dm = DataManager::get_dm();

  FieldHandle fout;
  MatrixHandle mout;
  GeomHandle gout;

  vector<GeomHandle > geom_hndls;
  vector<FieldHandle> field_handles;
  vector<MatrixHandle> interpolant_handles;

  //! \todo{re-enable multiple isovals}
  vector<double> isovals(0);
  isovals.push_back(get_p_isoval());

  ProgressReporter *prog = get_progress_reporter();
  FieldHandle field_input_handle = dm->get_field(field_id);
  ColorMapHandle colormap_input_handle = dm->get_colormap(cmap_id);

  const TypeDescription *td = field_input_handle->get_type_description();

  switch (get_p_algorithm()) {
  case 0:  // Marching Cubes
    {
      LockingHandle<MarchingCubesAlg> mc_alg;
      if (! mc_alg.get_rep()) {
	CompileInfoHandle ci = MarchingCubesAlg::get_compile_info(td);
	DynamicAlgoHandle  dalgo;    //! the algorithm instance (return data)
	if (!DynamicCompilation::compile(ci, dalgo)) {
	  if (prog)
            prog->error( "Marching Cubes can not work with this field.");
	  return rval;
	}
	
	mc_alg = (MarchingCubesAlg*)dalgo.get_rep();

	int np = get_p_np();
	if (np <  1 ) { np =  1; set_p_np(np); }
	if (np > 32 ) { np = 32; set_p_np(np); }
	mc_alg->set_np(np);
	mc_alg->set_field( field_input_handle.get_rep() );

	for (unsigned int iv=0; iv<isovals.size(); iv++)  {
	  mc_alg->search(isovals[iv],
			 get_p_build_trisurf(),
			 get_p_build_geom(),
			 get_p_transparency());
	  geom_hndls.push_back(mc_alg->get_geom());
	  for (int i = 0 ; i < np; i++) {
	    field_handles.push_back(mc_alg->get_field(i));
	    if (get_p_build_trisurf()) {
	      interpolant_handles.push_back(mc_alg->get_interpolant(i));
	    }
	  }
	}
	mc_alg->release();
      }
    }
    break;
  case 1:  // Noise
    {
      LockingHandle<NoiseAlg> noise_alg;
      if (! noise_alg.get_rep()) {
	CompileInfoHandle ci =
	  NoiseAlg::get_compile_info(td,
				     field_input_handle->basis_order() == 0,
				     false);
	DynamicAlgoHandle  dalgo;    //! the algorithm instance (return data)
	if (!DynamicCompilation::compile(ci, dalgo)) {
	  if (prog)
	  prog->error( "NOISE can not work with this field.");
	  return rval;
	}
	noise_alg = (NoiseAlg*)dalgo.get_rep();
	noise_alg->set_field(field_input_handle.get_rep());

	for (unsigned int iv=0; iv<isovals.size(); iv++) {
	  geom_hndls.push_back(noise_alg->search(isovals[iv],
						 get_p_build_trisurf(),
						 get_p_build_geom(),
						 get_p_transparency()));
	  field_handles.push_back(noise_alg->get_field());
	  if (get_p_build_trisurf()) {
	    interpolant_handles.push_back(noise_alg->get_interpolant());
	  }
	}
	noise_alg->release();
      }
    }
    break;

  case 2:  // View Dependent
    {
      LockingHandle<SageAlg> sage_alg;
      if (! sage_alg.get_rep()){
	CompileInfoHandle ci = SageAlg::get_compile_info(td);
	DynamicAlgoHandle  dalgo;    //! the algorithm instance (return data)
	if (!DynamicCompilation::compile(ci, dalgo)) {
	  if (prog)
	  prog->error( "SAGE can not work with this field.");
	  return rval;
	}
	sage_alg = (SageAlg*)dalgo.get_rep();
	sage_alg->set_field(field_input_handle.get_rep());

	for (unsigned int iv=0; iv<isovals.size(); iv++) {
	  GeomGroup *group = new GeomGroup;
	  GeomPoints *points = new GeomPoints();
	  sage_alg->search(isovals[iv], group, points);
	  geom_hndls.push_back( group );
	}
	sage_alg->release();
      }
    }
    break;
  default: // Error
    if (prog)
    prog->error("Unknown Algorithm requested.");
    return rval;
  }

  // Get the output field handle.
  if (get_p_build_trisurf() && field_handles.size() && 
      field_handles[0].get_rep()) 
  {
    // Copy the name of field to the downstream field.
    string fldname;
    if (field_input_handle->get_property("name",fldname)) {
      for (unsigned int i=0; i < field_handles.size(); i++)
	field_handles[i]->set_property("name",fldname, false);
    } else {
      for (unsigned int i=0; i < field_handles.size(); i++)
	field_handles[i]->set_property("name", string("ExtractIsosurface"), false);
    }


    // Single field.
    if (field_handles.size() == 1) {
      fout = field_handles[0];
      
    // Multiple field_handles.
    } else {
      const TypeDescription *ftd = field_handles[0]->get_type_description();
      CompileInfoHandle ci = ExtractIsosurfaceAlgo::get_compile_info(ftd);
	
      Handle<ExtractIsosurfaceAlgo> algo;
      DynamicAlgoHandle  dalgo;       //! the algorithm instance (return data)
      if (!DynamicCompilation::compile(ci, dalgo)) {
	return rval;
      }
      algo = (ExtractIsosurfaceAlgo*)dalgo.get_rep();
      fout = algo->execute(field_handles);
    }

    // Get the output interpolant handle.
    if (get_p_build_trisurf()) {
      if (interpolant_handles[0].get_rep()) {
	if (interpolant_handles.size() == 1) {
	  mout = interpolant_handles[0];
	} else {
	  mout = append_sparse_matrices(interpolant_handles);
	}
      } else {
	if (prog) {
	  prog->warning("Interpolant not computed for this input field type and data location.");
	}
      }
    } else {
      mout = 0;
    }
  } else {    
    fout = 0;
    mout = 0;
  }

  // Merged the geometry results.
  if(get_p_build_geom() && isovals.size()) 
  {
      
    GeomGroup *geomGroup = new GeomGroup;

    for (unsigned int iv=0; iv<isovals.size(); iv++) {
      MaterialHandle material_handle;

      if (colormap_input_handle.get_rep()) {
	material_handle = colormap_input_handle->lookup(isovals[iv]);
      } else {
	material_handle = new Material(Color(get_p_color_r(),
						get_p_color_g(),
						get_p_color_b()));
      }
      if (geom_hndls[iv].get_rep()) {
	geomGroup->add(new GeomMaterial(geom_hndls[iv],
					   material_handle));
      }
    }

    if( geomGroup->size() ) {
      gout = GeomHandle( geomGroup );
    } else {
      delete geomGroup;
      gout  = 0;
    }


    string fldname;
    if (!field_input_handle->get_property("name", fldname)) {
      fldname = string("ExtractIsosurface");
    }
    rval[1] = dm->add_geom(gout, fldname);
  }
    
  // Send the isosurface field downstream
  rval[0] = dm->add_field(fout);
  rval[2] = dm->add_matrix(mout);
  return rval;
}

} //end namespace SCIRun

