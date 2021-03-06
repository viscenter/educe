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

#ifndef CORE_ALGORITHMS_FIELDS_MAPPING_H
#define CORE_ALGORITHMS_FIELDS_MAPPING_H 1

#include <Core/Basis/Locate.h>
#include <Core/Algorithms/Util/DynamicAlgo.h>

#include <float.h>
#include <vector>
#include <string>
#include <algorithm>

//! for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

// MappingMethod (how do we select data?)
//
//  ClosestNodalData = Find the closest data containing element or node
//
//  ClosestInterpolatedData = Find the closest interpolated data
//  point. Inside the volume it picks the value the interpolation
//  model predicts and outside it makes a shortest projection
//
//  InterpolatedData = Uses interpolated data using the interpolation
//  model whereever possible and assumes no value outside the source
//  field

// Mapping Method to nodes (nodal values)

class SCISHARE MapFieldDataOntoFieldNodesAlgo : public DynamicAlgoBase
{
public:

  virtual bool MapFieldDataOntoFieldNodes(ProgressReporter *pr,
                       int numproc, FieldHandle src,
                       FieldHandle dst, FieldHandle& output, 
                       std::string mappingmethod, double def_value);
};


// IntegrationMethod:
//  Gaussian1 = Use 1st order Gaussian weights and nodes for integration
//  Gaussian2 = Use 2nd order Gaussian weights and nodes for integration
//  Gaussian3 = Use 3rd order Gaussian weights and nodes for integration

// Integration Filter:
//  Average =  take average value over integration nodes but disregard weights
//  Integrate = sum values over integration nodes using gaussian weights
//  Minimum = find minimum value using integration nodes
//  Maximum = find maximum value using integration nodes
//  Median  = find median value using integration nodes
//  MostCommon = find most common value among integration nodes


// Mapping Method to elements (Modal values)

class SCISHARE ModalMappingAlgo : public DynamicAlgoBase
{
public:

  virtual bool ModalMapping(ProgressReporter *pr,
                       int numproc, FieldHandle src,
                       FieldHandle dst, FieldHandle& output, 
                       std::string mappingmethod,
                       std::string integrationmethod,
                       std::string integrationfilter,
                       double def_value );
};


class SCISHARE MapFieldDataGradientOntoFieldAlgo : public DynamicAlgoBase
{
public:

  virtual bool MapFieldDataGradientOntoField(ProgressReporter *pr,
                       int numproc, FieldHandle src,
                       FieldHandle dst, FieldHandle& output, 
                       std::string mappingmethod,
                       std::string integrationmethod,
                       std::string integrationfilter,
                       bool calcnorm);
};

// Templated class for Nodal Mapping

template <class MAPPING, class FSRC, class FDST, class FOUT>
class MapFieldDataOntoFieldNodesAlgoT : public MapFieldDataOntoFieldNodesAlgo
{
public:
  virtual bool MapFieldDataOntoFieldNodes(ProgressReporter *pr,
                       int numproc, FieldHandle src,
                       FieldHandle dst, FieldHandle& output, 
                       std::string mappingmethod,
                       double def_value);
  
  // Input data for parallel algorithm
  class IData
  {
    public:
      ProgressReporter*         pr;
      FSRC*                     ifield;
      FOUT*                     ofield;
      typename FSRC::mesh_type* imesh;
      typename FOUT::mesh_type* omesh;
      int                       numproc;  // number of processes
      bool                      retval;   // return value
      double                    def_value;
  };
  
  // Parallel implementation
  void parallel(int procnum,IData* inputdata); 
};


// Modal version
template <class MAPPING, class INTEGRATOR, class FSRC, class FDST, class FOUT>
class ModalMappingAlgoT : public ModalMappingAlgo
{
public:
  virtual bool ModalMapping(ProgressReporter *pr,
                       int numproc, FieldHandle src,
                       FieldHandle dst, FieldHandle& output, 
                       std::string mappingmethod,
                       std::string integrationmethod,
                       std::string integrationfilter,
                       double def_value);
                       
  class IData
  {
    public:
      ProgressReporter*         pr;
      FSRC*                     ifield;
      FOUT*                     ofield;
      typename FSRC::mesh_type* imesh;
      typename FOUT::mesh_type* omesh;
      int                       numproc;
      std::string               integrationfilter;
      bool                      retval;
      double                    def_value;
  };
  
  void parallel(int procnum,IData* inputdata); 
};


// Modal version
template <class MAPPING, class INTEGRATOR, class FSRC, class FDST, class FOUT>
class MapFieldDataGradientOntoFieldAlgoT : public MapFieldDataGradientOntoFieldAlgo
{
public:
  virtual bool MapFieldDataGradientOntoField(ProgressReporter *pr,
                       int numproc, FieldHandle src,
                       FieldHandle dst, FieldHandle& output, 
                       std::string mappingmethod,
                       std::string integrationmethod,
                       std::string integrationfilter,
                       bool calnorm);
                       
  class IData
  {
    public:
      ProgressReporter*         pr;
      FSRC*                     ifield;
      FOUT*                     ofield;
      typename FSRC::mesh_type* imesh;
      typename FOUT::mesh_type* omesh;
      int                       numproc;
      std::string               integrationfilter;
      bool                      retval;
  };
  
  void parallel(int procnum,IData* inputdata); 
};

template <class MAPPING, class INTEGRATOR, class FSRC, class FDST, class FOUT>
class MapFieldDataGradientOntoFieldNormAlgoT : public MapFieldDataGradientOntoFieldAlgo
{
public:
  virtual bool MapFieldDataGradientOntoField(ProgressReporter *pr,
                       int numproc, FieldHandle src,
                       FieldHandle dst, FieldHandle& output, 
                       std::string mappingmethod,
                       std::string integrationmethod,
                       std::string integrationfilter,
                       bool calnorm);
                       
  class IData
  {
    public:
      ProgressReporter*         pr;
      FSRC*                     ifield;
      FOUT*                     ofield;
      typename FSRC::mesh_type* imesh;
      typename FOUT::mesh_type* omesh;
      int                       numproc;
      std::string               integrationfilter;
      bool                      retval;
  };
  
  void parallel(int procnum,IData* inputdata); 
};




template <class MAPPING, class FSRC, class FDST, class FOUT>
bool MapFieldDataOntoFieldNodesAlgoT<MAPPING,FSRC,FDST,FOUT>::MapFieldDataOntoFieldNodes(ProgressReporter *pr,
                       int numproc, FieldHandle src,FieldHandle dst, FieldHandle& output, 
                       std::string mappingmethod, double def_value) 
{
  // Some sanity checks, in order not to crash SCIRun when some is generating nonsense input
  FSRC* ifield = dynamic_cast<FSRC*>(src.get_rep());
  if (ifield == 0)
  {
    pr->error("MapFieldDataOntoFieldNodes: No input source field was given");
    return (false);
  }

  typename FSRC::mesh_type* imesh = dynamic_cast<typename FSRC::mesh_type*>(src->mesh().get_rep());
  if (imesh == 0)
  {
    pr->error("MapFieldDataOntoFieldNodes: No mesh is associated with input source field");
    return (false);  
  }

  FDST* dfield = dynamic_cast<FDST*>(dst.get_rep());
  if (dfield == 0)
  {
    pr->error("MapFieldDataOntoFieldNodes: No input destination field was given");
    return (false);
  }

  typename FDST::mesh_type* dmesh = dynamic_cast<typename FDST::mesh_type*>(dst->mesh().get_rep());
  if (dmesh == 0)
  {
    pr->error("MapFieldDataOntoFieldNodes: No mesh is associated with input destination field");
    return (false);  
  }

  // Creating output mesh and field
  output = dynamic_cast<Field *>(scinew FOUT(dmesh));
  if (output.get_rep() == 0)
  {
    pr->error("MapFieldDataOntoFieldNodes: Could no allocate output field");
    return (false);
  }
  
  FOUT* ofield = dynamic_cast<FOUT*>(output.get_rep());
  ofield->resize_fdata();
  
  // Make sure it inherits all the properties
  output->copy_properties(dst.get_rep());

  // Now do parallel algorithm

  IData IData;
  IData.ifield = ifield;
  IData.ofield = ofield;
  IData.imesh  = imesh;
  IData.omesh  = dmesh;
  IData.pr     = pr;
  IData.retval = true;
  IData.def_value = def_value;
  
  // Determine the number of processors to use:
  int np = Thread::numProcessors(); 
	// if (np > 5) np = 5;  
  if (numproc > 0) { np = numproc; }
  IData.numproc = np;
   
  Thread::parallel(this,&MapFieldDataOntoFieldNodesAlgoT<MAPPING,FSRC,FDST,FOUT>::parallel,np,&IData);
    
  return (IData.retval);
}


template <class MAPPING, class FSRC, class FDST, class FOUT>
void MapFieldDataOntoFieldNodesAlgoT<MAPPING,FSRC,FDST,FOUT>::parallel(int procnum,IData* idata)
{
  ProgressReporter *pr = idata->pr;
	
  try
  {
    typename FOUT::mesh_type::Node::iterator it, eit;
    typename FOUT::mesh_type::Node::size_type s;
    typename FOUT::mesh_type* omesh = idata->omesh;
    FOUT* ofield = idata->ofield;
    typename FOUT::value_type val;
		
    int numproc = idata->numproc;
		
    // loop over all the output nodes
    omesh->begin(it);
    omesh->end(eit);
    omesh->size(s);
		
    int cnt = 1;
		
    // Define class that defines how we find data from the source mesh
    MAPPING mapping(idata->ifield);
		
    // Make sure we start with the proper node
    for (int p =0; p < procnum; p++) if (it != eit) ++it;
    Point point;
		
    while (it != eit)
    {
      // Find the destination location
      omesh->get_center(point,*it);
      // Find the value associated with that location
      // This is the operation that should take most time
      // Hence we use it as a template so it can be compiled fully inline
      // without having to generate a lot of code
      if(!(mapping.get_data(point,val))) val = static_cast<typename FOUT::value_type>(idata->def_value);
      // Set the value
      ofield->set_value(val,*it);
		
      // Skip to the next one, but disregard the nodes the other 
      // processes are working on.
      for (int p =0; p < numproc; p++) if (it != eit) ++it;  
      if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }
    }
  }
  catch (const Exception &e)
  {
    pr->error(string("ModalMapping crashed with the following exception:\n")+
              e.message());
    idata->retval = false;
  }
  catch (const string a)
  {
    pr->error(a);
    idata->retval = false;
  }
  catch (const char *a)
  {
    pr->error(string(a));
    idata->retval = false;
  }	
  catch (...)
  {
    pr->error(string("ModalMapping crashed for unknown reason"));
    idata->retval = false;
  }	
}



// Modal version

template <class MAPPING, class INTEGRATOR, class FSRC, class FDST, class FOUT>
bool ModalMappingAlgoT<MAPPING,INTEGRATOR,FSRC,FDST,FOUT>::ModalMapping(ProgressReporter *pr,
                       int numproc, FieldHandle src,FieldHandle dst, FieldHandle& output, 
                       std::string mappingmethod,
                       std::string integrationmethod,
                       std::string integrationfilter,
                       double def_value) 
{
  FSRC* ifield = dynamic_cast<FSRC*>(src.get_rep());
  if (ifield == 0)
  {
    pr->error("ModalMapping: No input source field was given");
    return (false);
  }

  typename FSRC::mesh_type* imesh = dynamic_cast<typename FSRC::mesh_type*>(src->mesh().get_rep());
  if (imesh == 0)
  {
    pr->error("ModalMapping: No mesh is associated with input source field");
    return (false);  
  }

  FDST* dfield = dynamic_cast<FDST*>(dst.get_rep());
  if (dfield == 0)
  {
    pr->error("ModalMapping: No input destination field was given");
    return (false);
  }

  typename FDST::mesh_type* dmesh = dynamic_cast<typename FDST::mesh_type*>(dst->mesh().get_rep());
  if (dmesh == 0)
  {
    pr->error("ModalMapping: No mesh is associated with input destination field");
    return (false);  
  }

  output = dynamic_cast<Field *>(scinew FOUT(dmesh));
  if (output.get_rep() == 0)
  {
    pr->error("ModalMapping: Could no allocate output field");
    return (false);
  }
  
  FOUT* ofield = dynamic_cast<FOUT*>(output.get_rep());
  ofield->resize_fdata();
  
  output->copy_properties(dst.get_rep());

  // Now do parallel algorithm

  IData IData;
  IData.ifield = ifield;
  IData.ofield = ofield;
  IData.imesh  = imesh;
  IData.omesh  = dmesh;
  IData.pr     = pr;
  IData.retval = true;
  IData.integrationfilter = integrationfilter;
  IData.def_value = def_value;
  
  // Determine the number of processors to use:
  int np = Thread::numProcessors(); 
	// if (np > 5) np = 5;  
  if (numproc > 0) { np = numproc; }
  IData.numproc = np;
   
  Thread::parallel(this,&ModalMappingAlgoT<MAPPING,INTEGRATOR,FSRC,FDST,FOUT>::parallel,np,&IData);
          
  return (IData.retval);
}



template <class MAPPING, class INTEGRATOR, class FSRC, class FDST, class FOUT>
void ModalMappingAlgoT<MAPPING,INTEGRATOR,FSRC,FDST,FOUT>::parallel(int procnum,IData* idata)
{
  ProgressReporter *pr = idata->pr;

  try
  {
    typename FOUT::mesh_type::Elem::iterator it, eit;
    typename FOUT::mesh_type::Elem::size_type s;
    typename FOUT::mesh_type* omesh = idata->omesh;
    typename FOUT::value_type val, val2;
    FOUT* ofield = idata->ofield;
		
    int numproc = idata->numproc;
		
    omesh->size(s);
    omesh->begin(it);
    omesh->end(eit);
		
    int cnt = 1;
		
    MAPPING mapping(idata->ifield);
    INTEGRATOR integrator(idata->ofield);
		
    for (int p =0; p < procnum; p++) if (it != eit) ++it;
    std::vector<Point> points;
    std::vector<double> weights;
    std::string filter = idata->integrationfilter;
		
    // Determine the filter and loop over nodes
    if ((filter == "median")||(filter == "Median"))
    {
      // median filter over integration nodes
      std::vector<typename FOUT::value_type> valarray;

      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
        valarray.resize(points.size());
        for (size_t p = 0; p < points.size(); p++)
        {
          if(!(mapping.get_data(points[p],valarray[p]))) valarray[p] = static_cast<typename FOUT::value_type>(idata->def_value);
        }
        sort(valarray.begin(),valarray.end());
        int idx = static_cast<int>((valarray.size()/2));
        ofield->set_value(valarray[idx],*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }
      }
    }
    else if ((filter == "minimum")||(filter == "Minimum"))
    {
      // minimum filter over integration nodes
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
        typename FOUT::value_type val = 0;
        typename FOUT::value_type tval = 0;

        if (points.size() > 0)
        {
          if(!(mapping.get_data(points[0],val))) val = static_cast<typename FOUT::value_type>(idata->def_value);
          for (size_t p = 1; p < points.size(); p++)
          {
            if(!(mapping.get_data(points[p],tval))) tval = static_cast<typename FOUT::value_type>(idata->def_value);
            if (tval < val) val = tval;
          }
        }
        ofield->set_value(val,*it);
				
        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }
      }  
    }
    else if ((filter == "maximum")||(filter == "Maximum"))
    {
      // maximum filter over integration nodes
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
        typename FOUT::value_type val = 0;
        typename FOUT::value_type tval = 0;

        if (points.size() > 0)
        {
          if (!(mapping.get_data(points[0],val))) val = static_cast<typename FOUT::value_type>(idata->def_value);
          for (size_t p = 1; p < points.size(); p++)
          {
            if (!(mapping.get_data(points[p],tval))) val = static_cast<typename FOUT::value_type>(idata->def_value);
            if (tval > val) val = tval;
          }
        }
        ofield->set_value(val,*it);
				
        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }
      }
    }
    else if ((filter == "mostcommon")||(filter == "Mostcommon")||(filter == "MostCommon"))
    {
      // Filter designed for segmentations where one wants the most common element to be the
      // sampled element
			
      std::vector<typename FOUT::value_type> valarray;

      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
        valarray.resize(points.size());

        for (size_t p = 0; p < points.size(); p++)
        {
          if(!(mapping.get_data(points[p],valarray[p]))) valarray[p] = static_cast<typename FOUT::value_type>(idata->def_value);
        }
        sort(valarray.begin(),valarray.end());
				 
        typename FOUT::value_type rval = 0;
        typename FOUT::value_type val = 0;
        int rnum = 0;
				
        int p = 0;
        int n = 0;
				
        while (p < valarray.size())
        {
          n = 1;
          val = valarray[p];

          p++;
          while ( p < valarray.size() && valarray[p] == val) { n++; p++; }
					
          if (n >= rnum) { rnum = n; rval = val;}
        }
							
        ofield->set_value(rval,*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }
      }  
    }
    else if ((filter == "integrate")||(filter == "Integrate"))
    {
      // Real integration of underlying value
      while (it != eit)
      {
        integrator.get_nodes_and_iweights(*it,points,weights);

        val = 0;
        for (int p=0; p<points.size(); p++)
        {
          if(!(mapping.get_data(points[p],val2))) val2 = static_cast<typename FOUT::value_type>(idata->def_value);
          val += val2*weights[p];
        }
				
        ofield->set_value(val,*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }
      }
    }
    else if ((filter == "weightedaverage")||(filter == "WeightedAverage"))
    {
      // Real integration of underlying value
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);

        val = 0;
        for (int p=0; p<points.size(); p++)
        {
          if(!(mapping.get_data(points[p],val2))) val2 = static_cast<typename FOUT::value_type>(idata->def_value);
          val += static_cast<typename FOUT::value_type>(val2*weights[p]);
        }
				
        ofield->set_value(val,*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }
      }
    }
    else if ((filter == "average")||(filter == "Average"))
    {
      // Average, like integrate but ignore weights
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
				
        val = 0;
        for (int p=0; p<points.size(); p++)
        {
          if(!(mapping.get_data(points[p],val2))) val2 = static_cast<typename FOUT::value_type>(idata->def_value);
          val += static_cast<typename FOUT::value_type>(val2 * (1.0/points.size()));
        }
        ofield->set_value(val,*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }
      }
    }
    else if ((filter == "sum")||(filter == "Sum"))
    {
      // Average, like integrate but ignore weights
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
				
        val = 0;
        for (int p=0; p<points.size(); p++)
        {
          if (!(mapping.get_data(points[p],val2))) val2 = static_cast<typename FOUT::value_type>(idata->def_value);
          val += val2;;
        }
        ofield->set_value(val,*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }
    }
    else
    {
      if (procnum == 0)
      {
        idata->pr->error("ModalMapping: Filter method is unknown");
        idata->retval = false;
      }
    }
  }
  catch (const Exception &e)
  {
    pr->error(string("ModalMapping crashed with the following exception:\n")+
              e.message());
    idata->retval = false;
  }
  catch (const string a)
  {
    pr->error(a);
    idata->retval = false;
  }
  catch (const char *a)
  {
    pr->error(string(a));
    idata->retval = false;
  }	
  catch (...)
  {
    pr->error(string("ModalMapping crashed for unknown reason"));
    idata->retval = false;
  }
}



// GradientModal version

template <class MAPPING, class INTEGRATOR, class FSRC, class FDST, class FOUT>
bool MapFieldDataGradientOntoFieldAlgoT<MAPPING,INTEGRATOR,FSRC,FDST,FOUT>::MapFieldDataGradientOntoField(ProgressReporter *pr,
                       int numproc, FieldHandle src,FieldHandle dst, FieldHandle& output, 
                       std::string mappingmethod,
                       std::string integrationmethod,
                       std::string integrationfilter, bool calcnorm) 
{
  FSRC* ifield = dynamic_cast<FSRC*>(src.get_rep());
  if (ifield == 0)
  {
    pr->error("MapFieldDataGradientOntoField: No input source field was given");
    return (false);
  }

  typename FSRC::mesh_type* imesh = dynamic_cast<typename FSRC::mesh_type*>(src->mesh().get_rep());
  if (imesh == 0)
  {
    pr->error("MapFieldDataGradientOntoField: No mesh is associated with input source field");
    return (false);  
  }

  FDST* dfield = dynamic_cast<FDST*>(dst.get_rep());
  if (dfield == 0)
  {
    pr->error("MapFieldDataGradientOntoField: No input destination field was given");
    return (false);
  }

  typename FDST::mesh_type* dmesh = dynamic_cast<typename FDST::mesh_type*>(dst->mesh().get_rep());
  if (dmesh == 0)
  {
    pr->error("MapFieldDataGradientOntoField: No mesh is associated with input destination field");
    return (false);  
  }

  output = dynamic_cast<Field *>(scinew FOUT(dmesh));
  if (output.get_rep() == 0)
  {
    pr->error("MapFieldDataGradientOntoField: Could no allocate output field");
    return (false);
  }
  
  FOUT* ofield = dynamic_cast<FOUT*>(output.get_rep());
  ofield->resize_fdata();
  
  output->copy_properties(dst.get_rep());

  // Now do parallel algorithm

  IData IData;
  IData.ifield = ifield;
  IData.ofield = ofield;
  IData.imesh  = imesh;
  IData.omesh  = dmesh;
  IData.pr     = pr;
  IData.retval = true;
  IData.integrationfilter = integrationfilter;
  
  // Determine the number of processors to use:
  int np = Thread::numProcessors(); 
	// if (np > 5) np = 5;  
  if (numproc > 0) { np = numproc; }
  IData.numproc = np;
   
  Thread::parallel(this,&MapFieldDataGradientOntoFieldAlgoT<MAPPING,INTEGRATOR,FSRC,FDST,FOUT>::parallel,np,&IData);
        
  return (IData.retval);
}



template <class MAPPING, class INTEGRATOR, class FSRC, class FDST, class FOUT>
void MapFieldDataGradientOntoFieldAlgoT<MAPPING,INTEGRATOR,FSRC,FDST,FOUT>::parallel(int procnum,IData* idata)
{
  ProgressReporter *pr = idata->pr;
	
  try
  {
    typename FOUT::mesh_type::Elem::iterator it, eit;
    typename FOUT::mesh_type::Elem::size_type s;
    typename FOUT::mesh_type* omesh = idata->omesh;
    typename FOUT::value_type val, val2;
    FOUT* ofield = idata->ofield;
		
    int numproc = idata->numproc;
		
    omesh->size(s);
    omesh->begin(it);
    omesh->end(eit);
		
    int cnt = 1;
		
    MAPPING mapping(idata->ifield);
    INTEGRATOR integrator(idata->ofield);
		
    for (int p =0; p < procnum; p++) if (it != eit) ++it;
    std::vector<Point> points;
    std::vector<double> weights;
    std::string filter = idata->integrationfilter;
		
    // Determine the filter and loop over nodes
    if ((filter == "median")||(filter == "Median"))
    {
      // median filter over integration nodes
      std::vector<typename FOUT::value_type> valarray;

      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
        valarray.resize(points.size());
        for (size_t p = 0; p < points.size(); p++)
        {
          if(!(mapping.get_gradient(points[p],valarray[p]))) valarray[p] = 0;
        }
        sort(valarray.begin(),valarray.end());
        int idx = static_cast<int>((valarray.size()/2));
        ofield->set_value(valarray[idx],*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }
    }
    else if ((filter == "minimum")||(filter == "Minimum"))
    {
      // minimum filter over integration nodes
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
        typename FOUT::value_type val; val = 0;
        typename FOUT::value_type tval; tval = 0;

        if (points.size() > 0)
        {
          if(!(mapping.get_gradient(points[0],val))) val = 0;
          for (size_t p = 1; p < points.size(); p++)
          {
            if(!(mapping.get_gradient(points[p],tval))) tval = 0;
            if (tval < val) val = tval;
          }
        }
        ofield->set_value(val,*it);
				
        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }  
    }
    else if ((filter == "maximum")||(filter == "Maximum"))
    {
      // maximum filter over integration nodes
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
        typename FOUT::value_type val; val = 0;
        typename FOUT::value_type tval; tval = 0;

        if (points.size() > 0)
        {
          if (!(mapping.get_gradient(points[0],val))) val = 0;
          for (size_t p = 1; p < points.size(); p++)
          {
            if (!(mapping.get_gradient(points[p],tval))) tval = 0;
            if (tval > val) val = tval;
          }
        }
        ofield->set_value(val,*it);
				
        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }
    }
    else if ((filter == "mostcommon")||(filter == "Mostcommon")||(filter == "MostCommon"))
    {
      // Filter designed for segmentations where one wants the most common element to be the
      // sampled element

      std::vector<typename FOUT::value_type> valarray;
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
        valarray.resize(points.size());
        for (size_t p = 0; p < points.size(); p++)
        {
          if(!(mapping.get_gradient(points[p],valarray[p]))) valarray[p] = 0;
        }
        sort(valarray.begin(),valarray.end());
				 
        typename FOUT::value_type rval; rval = 0;
        typename FOUT::value_type val; val = 0;
        int rnum = 0;
				
        int p = 0;
        int n = 0;
				
        while (p < valarray.size())
        {
          n = 1;
          val = valarray[p];

          p++;
          while ( p < valarray.size() && valarray[p] == val) { n++; p++; }
					
          if (n >= rnum) { rnum = n; rval = val;}
        }
							
        ofield->set_value(rval,*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }  
    }
    else if ((filter == "integrate")||(filter == "Integrate"))
    {
      // Real integration of underlying value
      while (it != eit)
      {
        integrator.get_nodes_and_iweights(*it,points,weights);

        val = 0;
        for (int p=0; p<points.size(); p++)
        {
          if(!(mapping.get_gradient(points[p],val2))) val2 = 0;
          val += val2*weights[p];
        }
				
        ofield->set_value(val,*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }
    }
    else if ((filter == "weightedaverage")||(filter == "WeightedAverage"))
    {
      // Real integration of underlying value
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);

        val = 0;
        for (int p=0; p<points.size(); p++)
        {
          if(!(mapping.get_gradient(points[p],val2))) val2 = 0;
          val += val2*weights[p];
        }
				
        ofield->set_value(val,*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }
    }
    else if ((filter == "average")||(filter == "Average"))
    {
      // Average, like integrate but ignore weights
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);      
				
        val = 0;
        for (int p=0; p<points.size(); p++)
        {
          if(!(mapping.get_gradient(points[p],val2))) val2 = 0;
          val += val2 * (1.0/points.size());
        }
        ofield->set_value(val,*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }
    }
    else if ((filter == "sum")||(filter == "Sum"))
    {
      // Average, like integrate but ignore weights
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
				
        val = 0;
        for (int p=0; p<points.size(); p++)
        {
          if (!(mapping.get_gradient(points[p],val2))) val2 = 0;
          val += val2;
        }
        ofield->set_value(val,*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }
    }
    else
    {
      if (procnum == 0)
      {
        idata->pr->error("MapFieldDataGradientOntoField: Filter method is unknown");
        idata->retval = false;
      }
      return;
    }
  }
  catch (const Exception &e)
  {
    pr->error(string("ModalMapping crashed with the following exception:\n")+
              e.message());
    idata->retval = false;
  }
  catch (const string a)
  {
    pr->error(a);
    idata->retval = false;
  }
  catch (const char *a)
  {
    pr->error(string(a));
    idata->retval = false;
  }	
  catch (...)
  {
    pr->error(string("ModalMapping crashed for unknown reason"));
    idata->retval = false;
  }		
}



template <class MAPPING, class INTEGRATOR, class FSRC, class FDST, class FOUT>
bool MapFieldDataGradientOntoFieldNormAlgoT<MAPPING,INTEGRATOR,FSRC,FDST,FOUT>::MapFieldDataGradientOntoField(ProgressReporter *pr,
                       int numproc, FieldHandle src,FieldHandle dst, FieldHandle& output, 
                       std::string mappingmethod,
                       std::string integrationmethod,
                       std::string integrationfilter, bool calcnorm) 
{
  FSRC* ifield = dynamic_cast<FSRC*>(src.get_rep());
  if (ifield == 0)
  {
    pr->error("MapFieldDataGradientOntoField: No input source field was given");
    return (false);
  }

  typename FSRC::mesh_type* imesh = dynamic_cast<typename FSRC::mesh_type*>(src->mesh().get_rep());
  if (imesh == 0)
  {
    pr->error("MapFieldDataGradientOntoField: No mesh is associated with input source field");
    return (false);  
  }

  FDST* dfield = dynamic_cast<FDST*>(dst.get_rep());
  if (dfield == 0)
  {
    pr->error("MapFieldDataGradientOntoField: No input destination field was given");
    return (false);
  }

  typename FDST::mesh_type* dmesh = dynamic_cast<typename FDST::mesh_type*>(dst->mesh().get_rep());
  if (dmesh == 0)
  {
    pr->error("MapFieldDataGradientOntoField: No mesh is associated with input destination field");
    return (false);  
  }

  output = dynamic_cast<Field *>(scinew FOUT(dmesh));
  if (output.get_rep() == 0)
  {
    pr->error("MapFieldDataGradientOntoField: Could no allocate output field");
    return (false);
  }
  
  FOUT* ofield = dynamic_cast<FOUT*>(output.get_rep());
  ofield->resize_fdata();
  
  output->copy_properties(dst.get_rep());

  // Now do parallel algorithm

  IData IData;
  IData.ifield = ifield;
  IData.ofield = ofield;
  IData.imesh  = imesh;
  IData.omesh  = dmesh;
  IData.pr     = pr;
  IData.retval = true;
  IData.integrationfilter = integrationfilter;
  
  // Determine the number of processors to use:
  int np = Thread::numProcessors(); 
	// if (np > 5) np = 5;  
  if (numproc > 0) { np = numproc; }
  IData.numproc = np;
   
  Thread::parallel(this,&MapFieldDataGradientOntoFieldNormAlgoT<MAPPING,INTEGRATOR,FSRC,FDST,FOUT>::parallel,np,&IData);
        
  return (IData.retval);
}



template <class MAPPING, class INTEGRATOR, class FSRC, class FDST, class FOUT>
void MapFieldDataGradientOntoFieldNormAlgoT<MAPPING,INTEGRATOR,FSRC,FDST,FOUT>::parallel(int procnum,IData* idata)
{
  ProgressReporter *pr = idata->pr;
	
  try
  {
    typename FOUT::mesh_type::Elem::iterator it, eit;
    typename FOUT::mesh_type::Elem::size_type s;
    typename FOUT::mesh_type* omesh = idata->omesh;
    typename FOUT::value_type val; 
    Vector val2;
    FOUT* ofield = idata->ofield;
    ProgressReporter *pr = idata->pr;
		
    int numproc = idata->numproc;
		
    omesh->size(s);
    omesh->begin(it);
    omesh->end(eit);
		
    int cnt;
		
    MAPPING mapping(idata->ifield);
    INTEGRATOR integrator(idata->ofield);
		
    for (int p =0; p < procnum; p++) if (it != eit) ++it;
    std::vector<Point> points;
    std::vector<double> weights;
    std::string filter = idata->integrationfilter;
		
    // Determine the filter and loop over nodes
    if ((filter == "median")||(filter == "Median"))
    {
      // median filter over integration nodes
      std::vector<typename FOUT::value_type> valarray;

      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
        valarray.resize(points.size());
        for (size_t p = 0; p < points.size(); p++)
        {
          if(!(mapping.get_gradient(points[p],val2))) val2 = 0;
          valarray[p] = static_cast<typename FOUT::value_type>(val2.length());
        }
        sort(valarray.begin(),valarray.end());
        int idx = static_cast<int>((valarray.size()/2));
        ofield->set_value(valarray[idx],*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }
    }
    else if ((filter == "minimum")||(filter == "Minimum"))
    {
      // minimum filter over integration nodes
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
        typename FOUT::value_type val; val = 0;
        typename FOUT::value_type tval; tval = 0;

        if (points.size() > 0)
        {
          if(!(mapping.get_gradient(points[0],val2))) val2 = 0;
          val = static_cast<typename FOUT::value_type>(val2.length());
          for (size_t p = 1; p < points.size(); p++)
          {
            if(!(mapping.get_gradient(points[p],val2))) val2 = 0;
            tval = static_cast<typename FOUT::value_type>(val2.length());
            if (tval < val) val = tval;
          }
        }
        ofield->set_value(val,*it);
				
        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }  
    }
    else if ((filter == "maximum")||(filter == "Maximum"))
    {
      // maximum filter over integration nodes
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
        typename FOUT::value_type val; val = 0;
        typename FOUT::value_type tval; tval = 0;

        if (points.size() > 0)
        {
          if (!(mapping.get_gradient(points[0],val2))) val2 = 0;
          val = static_cast<typename FOUT::value_type>(val2.length());
          for (size_t p = 1; p < points.size(); p++)
          {
            if (!(mapping.get_gradient(points[p],val2))) val2 = 0;
            val = static_cast<typename FOUT::value_type>(val2.length());
            if (tval > val) val = tval;
          }
        }
        ofield->set_value(val,*it);
				
        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }
    }
    else if ((filter == "mostcommon")||(filter == "Mostcommon")||(filter == "MostCommon"))
    {
      // Filter designed for segmentations where one wants the most common element to be the
      // sampled element
      std::vector<typename FOUT::value_type> valarray;
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
        valarray.resize(points.size());
        for (size_t p = 0; p < points.size(); p++)
        {
          if(!(mapping.get_gradient(points[p],val2))) val2 = 0;
          valarray[p] = static_cast<typename FOUT::value_type>(val2.length());
        }
        sort(valarray.begin(),valarray.end());
				 
        typename FOUT::value_type rval; rval = 0;
        typename FOUT::value_type val; val = 0;
        int rnum = 0;
				
        int p = 0;
        int n = 0;
				
        while (p < valarray.size())
        {
          n = 1;
          val = valarray[p];

          p++;
          while ( p < valarray.size() && valarray[p] == val) { n++; p++; }
					
          if (n >= rnum) { rnum = n; rval = val;}
        }
							
        ofield->set_value(rval,*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }            
      }  
    }
    else if ((filter == "integrate")||(filter == "Integrate"))
    {
      // Real integration of underlying value
      while (it != eit)
      {
        integrator.get_nodes_and_iweights(*it,points,weights);

        val = 0.0;
        for (int p=0; p<points.size(); p++)
        {
          if(!(mapping.get_gradient(points[p],val2))) val2 = 0;
          val += static_cast<typename FOUT::value_type>(val2.length())*weights[p];
        }
				
        ofield->set_value(val,*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }
    }
    else if ((filter == "weightedaverage")||(filter == "WeightedAverage"))
    {
      // Real integration of underlying value
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);

        val = 0.0;
        for (int p=0; p<points.size(); p++)
        {
          if(!(mapping.get_gradient(points[p],val2))) val2 = 0;
          val += static_cast<typename FOUT::value_type>(val2.length())*weights[p];
        }
				
        ofield->set_value(val,*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }
    }
    else if ((filter == "average")||(filter == "Average"))
    {
      // Average, like integrate but ignore weights
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);      
				
        val = 0.0;
        for (int p=0; p<points.size(); p++)
        {
          if(!(mapping.get_gradient(points[p],val2))) val2 = 0;
          val += static_cast<typename FOUT::value_type>(val2.length()) * (1.0/points.size());
        }
        ofield->set_value(val,*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }
    }
    else if ((filter == "sum")||(filter == "Sum"))
    {
      // Average, like integrate but ignore weights
      while (it != eit)
      {
        integrator.get_nodes_and_weights(*it,points,weights);
				
        val = 0.0;
        for (int p=0; p<points.size(); p++)
        {
          if (!(mapping.get_gradient(points[p],val2))) val2 = 0;
          val += static_cast<typename FOUT::value_type>(val2.length());
        }
        ofield->set_value(val,*it);

        for (int p =0; p < numproc; p++) if (it != eit) ++it;  
        if (procnum == 0) { cnt++; if (cnt == 100) { pr->update_progress(static_cast<int>(*it),static_cast<int>(s)); cnt = 1; } }      
      }
    }
    else
    {
      if (procnum == 0)
      {
        idata->pr->error("MapFieldDataGradientOntoField: Filter method is unknown");
        idata->retval = false;
      }
    }
  }
  catch (const Exception &e)
  {
    pr->error(string("ModalMapping crashed with the following exception:\n")+
              e.message());
    idata->retval = false;
  }
  catch (const string a)
  {
    pr->error(a);
    idata->retval = false;
  }
  catch (const char *a)
  {
    pr->error(string(a));
    idata->retval = false;
  }	
  catch (...)
  {
    pr->error(string("ModalMapping crashed for unknown reason"));
    idata->retval = false;
  }	
}




// Classes for finding values in a mesh

template <class FIELD>
class ClosestNodalData {
  public:
    ClosestNodalData(FIELD* field);
    ~ClosestNodalData();
    
    bool get_data(Point& p, typename FIELD::value_type& val);

  private:
    // Store these so we do need to reserve memory each time
    FIELD*  field_;
    typename FIELD::mesh_type mesh_;

    typename FIELD::mesh_type::Elem::index_type idx_;
    typename FIELD::mesh_type::Node::array_type nodes_;    
    
    typename FIELD::mesh_type::Node::iterator it_;
    typename FIELD::mesh_type::Node::iterator eit_;
    typename FIELD::mesh_type::Node::index_type minidx_;
};


template <class FIELD>
ClosestNodalData<FIELD>::ClosestNodalData(FIELD* field)
{
  field_ = field;
  mesh_ = field->get_typed_mesh().get_rep();
  mesh_->synchronize(Mesh::LOCATE_E);
}

template <class FIELD>
ClosestNodalData<FIELD>::~ClosestNodalData()
{
}

template <class FIELD>
bool ClosestNodalData<FIELD>::get_data(Point& p, typename FIELD::value_type& val)
{
  Point p2;
  double dist = DBL_MAX;
  double distt;
  int minidx = 0;
  
  if (mesh_->locate(idx_,p))
  {
    mesh_->get_nodes(nodes_,idx_);
    
    for (int r =0; r < nodes_.size(); r++)
    {
      mesh_->get_center(p2,nodes_[r]);  
      distt = Vector(p2-p).length2(); 
      if (distt < dist)
      {
        dist = distt;
        minidx = r;
      }
    }
    
    field_->value(val,nodes_[minidx]);
    return  (true);
  }

  mesh_->begin(it_);
  mesh_->end(eit_);
  while (it_ != eit_)
  {
    Point c;
    mesh_->get_center(c, *it_);
    const double distt = (p - c).length2();
    if (distt < dist)
    {
      dist = distt;
      minidx = *it_;
    }
    ++it_;
  }
 
  field_->value(val,minidx);
  return (true);
}




template <class FIELD>
class ClosestModalData {
  public:
    ClosestModalData(FIELD* field);
    ~ClosestModalData();
    
    bool get_data(Point& p, typename FIELD::value_type& val);

  private:
    // Store these so we do need to reserve memory each time
    FIELD*  field_;
    typename FIELD::mesh_type* mesh_;

    typename FIELD::mesh_type::Elem::index_type idx_;
    
    typename FIELD::mesh_type::Elem::iterator it_;
    typename FIELD::mesh_type::Elem::iterator eit_;
    typename FIELD::mesh_type::Elem::index_type minidx_;

};


template <class FIELD>
ClosestModalData<FIELD>::ClosestModalData(FIELD* field)
{
  field_ = field;
  mesh_ = field->get_typed_mesh().get_rep();
  mesh_->synchronize(Mesh::LOCATE_E);
}

template <class FIELD>
ClosestModalData<FIELD>::~ClosestModalData()
{
}

template <class FIELD>
bool ClosestModalData<FIELD>::get_data(Point& p, typename FIELD::value_type& val)
{
  Point p2;
  double dist = DBL_MAX;
  double distt;
  
  if (mesh_->locate(idx_,p))
  {    
    field_->value(val,idx_);
    return  (true);
  }

  mesh_->begin(it_);
  mesh_->end(eit_);
  while (it_ != eit_)
  {
    Point c;
    mesh_->get_center(c, *it_);
    const double distt = (p - c).length2();
    if (distt < dist)
    {
      dist = distt;
      minidx_ = *it_;
    }
    ++it_;
  }
 
  field_->value(val,minidx_);
  return (true);
}


// InterpolatedData class which gets the
// interpolated value from a mesh

template <class FIELD>
class InterpolatedData {
  public:
    InterpolatedData(FIELD* field);
    ~InterpolatedData();
    
    bool get_data(Point& p, typename FIELD::value_type& val);
    
  private:
    FIELD*              field_;
    typename FIELD::mesh_type*   mesh_;
             
    std::vector<double> coords_;
    typename FIELD::mesh_type::Elem::index_type idx_;
};


template <class FIELD>
InterpolatedData<FIELD>::InterpolatedData(FIELD* field)
{
  field_ = field;
  mesh_ = field->get_typed_mesh().get_rep();
  mesh_->synchronize(Mesh::LOCATE_E);
}

template <class FIELD>
InterpolatedData<FIELD>::~InterpolatedData<FIELD>()
{
}

template <class FIELD>
bool InterpolatedData<FIELD>::get_data(Point& p, typename FIELD::value_type& data)
{
  
  if (mesh_->locate(idx_,p))
  {
    mesh_->get_coords(coords_,p,idx_);
    field_->interpolate(data,coords_,idx_);
    return (true);
  }
  
  return (false);
}


template <class FIELD>
class InterpolatedGradient {
  public:
    InterpolatedGradient(FIELD* field);
    ~InterpolatedGradient();
    
    bool get_gradient(Point& p, Vector& val);
    
  private:
    FIELD*              field_;
    typename FIELD::mesh_type*   mesh_;
             
    std::vector<double> coords_;
    std::vector<typename FIELD::value_type> grad_;
    typename FIELD::mesh_type::Elem::index_type idx_;
};


template <class FIELD>
InterpolatedGradient<FIELD>::InterpolatedGradient(FIELD* field)
{
  field_ = field;
  mesh_ = field->get_typed_mesh().get_rep();
  mesh_->synchronize(Mesh::LOCATE_E);
  grad_.resize(3);
}

template <class FIELD>
InterpolatedGradient<FIELD>::~InterpolatedGradient<FIELD>()
{
}

template <class FIELD>
bool InterpolatedGradient<FIELD>::get_gradient(Point& p, Vector& data)
{
  
  if (mesh_->locate(idx_,p))
  {
    mesh_->get_coords(coords_,p,idx_);
    field_->gradient(grad_,coords_,idx_);
    data[0] = static_cast<double>(grad_[0]);
    data[1] = static_cast<double>(grad_[1]);
    data[2] = static_cast<double>(grad_[2]);
    return (true);
  }
  
  return (false);
}



/*
template <FIELD,DFIELD>
class ClosestInterpolatedCellData {
  public:
    ClosestInterpolatedCellData(FIELD* field);
    ~ClosestInterpolatedCellData();
    
    bool get_data(Point& p, typename FIELD::value_type& val);
    
  private:
    FIELD*              field_;
    DFIELD*             dfield_;
    FIELD::mesh_type*   mesh_;

    FieldHandle         handle_;
               
    std::vector<double> coords_;
};


template <class FIELD, class DFIELD>
bool ClosestInterpolatedCellData<FIELD,DFIELD>::ClosestInterpolatedCellData<FIELD>(FIELD* field)
{
  field_ = field;
  mesh_ = field->get_typed_mesh().get_rep();
  mesh_->synchronize(Mesh::LOCATE_E);
}



template <class FIELD, class DFIELD>
bool ClosestInterpolatedData<FIELD,DFIELD>::get_data(Point& p, typename FIELD::value_type& data)
{
  typename FSRC::mesh_type::Elem::index_type idx;
  
  if (mesh_->locate(idx,p))
  {
    std::vector<double> coords;
    field_->interpolate(data,coord_,idx);
    return (true);
  }
  
  
}
*/


// Integration classes

template <class GAUSSIAN, class FIELD >
class GaussianIntegration 
{
  public:

    GaussianIntegration(FIELD* field)
    {
      field_ = field;
      if (field_)
      {
        mesh_  = field->get_typed_mesh().get_rep();
        basis_ = mesh_->get_basis();
        
        coords_.resize(gauss_.GaussianNum);
        weights_.resize(gauss_.GaussianNum);
        for (int p=0; p<gauss_.GaussianNum; p++)
        {
          for (int q=0; q<basis_.domain_dimension(); q++)
          {
            coords_[p].push_back(gauss_.GaussianPoints[p][q]);
          }
          weights_[p] = gauss_.GaussianWeights[p];
        }
        
        vol_ = basis_.domain_size();
      }
    }

    void get_nodes_and_weights(typename FIELD::mesh_type::Elem::index_type idx, std::vector<Point>& gpoints, std::vector<double>& gweights)
    {    
      gpoints.resize(gauss_.GaussianNum);
      gweights.resize(gauss_.GaussianNum);
      
      for (int k=0; k < coords_.size(); k++)
      {
        mesh_->interpolate(gpoints[k],coords_[k],idx);
        gweights[k] = weights_[k];
      }
    }
        
    void get_nodes_and_iweights(typename FIELD::mesh_type::Elem::index_type idx, std::vector<Point>& gpoints, std::vector<double>& gweights)
    { 
      gpoints.resize(gauss_.GaussianNum);
      gweights.resize(gauss_.GaussianNum);
      
      for (int k=0; k < coords_.size(); k++)
      {
        mesh_->interpolate(gpoints[k],coords_[k],idx);
        gweights[k] = weights_[k]*(vol_/mesh_->det_jacobian(coords_[k],idx));
      }
    }

  private:
    FIELD*                                 field_;
    typename FIELD::mesh_type*             mesh_;
    typename FIELD::mesh_type::basis_type  basis_;
    GAUSSIAN gauss_;

    // Constants for each field
    std::vector<std::vector<double> > coords_;
    std::vector<double> weights_;
    double vol_;    
};


template <class FIELD, int SIZE>
class RegularIntegration 
{
  public:

    RegularIntegration(FIELD* field)
    {
      field_ = field;
      if (field_)
      {
        mesh_  = field->get_typed_mesh().get_rep();
        basis_ = mesh_->get_basis();
                
        int dim = basis_.domain_dimension();
        if (dim == 1)
        {
          coords_.resize(SIZE);
          weights_.resize(SIZE);
          
          for (int p=0; p<SIZE; p++)
          {
            coords_[p].push_back((0.5+p)/SIZE);
            weights_[p] = 1/SIZE;
          }          
        }
        
        if (dim == 2)
        {
          coords_.resize(SIZE*SIZE);
          weights_.resize(SIZE*SIZE);
          
          for (int p=0; p<SIZE; p++)
          {
            for (int q=0; q<SIZE; q++)
            {
              coords_[p+q*SIZE].push_back((0.5+p)/SIZE);
              coords_[p+q*SIZE].push_back((0.5+q)/SIZE);
              weights_[p+q*SIZE] = 1/(SIZE*SIZE);
            }
          }         
        }

        if (dim == 3)
        {
          coords_.resize(SIZE*SIZE*SIZE);
          weights_.resize(SIZE*SIZE*SIZE);
          
          for (int p=0; p<SIZE; p++)
          {
            for (int q=0; q<SIZE; q++)
            {
              for (int r=0; r<SIZE; r++)
              {
                coords_[p+q*SIZE+r*SIZE*SIZE].push_back((0.5+p)/SIZE);
                coords_[p+q*SIZE+r*SIZE*SIZE].push_back((0.5+q)/SIZE);
                coords_[p+q*SIZE+r*SIZE*SIZE].push_back((0.5+r)/SIZE);
                weights_[p+q*SIZE+r*SIZE*SIZE] = 1/(SIZE*SIZE*SIZE);
              }
            }
          }         
        }
      }  
    }

    void get_nodes_and_weights(typename FIELD::mesh_type::Elem::index_type idx, std::vector<Point>& gpoints, std::vector<double>& gweights)
    {    
      gpoints.resize(weights_.size());
      gweights.resize(weights_.size());
      
      for (int k=0; k < weights_.size(); k++)
      {
        mesh_->interpolate(gpoints[k],coords_[k],idx);
        gweights[k] = weights_[k];
      }
    }

    void get_nodes_and_iweights(typename FIELD::mesh_type::Elem::index_type idx, std::vector<Point>& gpoints, std::vector<double>& gweights)
    {    
      gpoints.resize(weights_.size());
      gweights.resize(weights_.size());
      
      for (int k=0; k < weights_.size(); k++)
      {
        mesh_->interpolate(gpoints[k],coords_[k],idx);
        gweights[k] = weights_[k]*(1.0/mesh_->det_jacobian(coords_[k],idx));
      }
    }


  private:
    FIELD*                                 field_;
    typename FIELD::mesh_type*             mesh_;
    typename FIELD::mesh_type::basis_type  basis_;

    std::vector<std::vector<double> > coords_;
    std::vector<double> weights_;
};


template <class GAUSSIAN, class FIELD >
class NormalGaussianIntegration 
{
  public:

    inline NormalGaussianIntegration(FIELD* field)
    {
      field_ = field;
      if (field_)
      {
        mesh_  = field->get_typed_mesh().get_rep();
        basis_ = mesh_->get_basis();
        
        coords_.resize(gauss_.GaussianNum);
        weights_.resize(gauss_.GaussianNum);
        for (int p=0; p<gauss_.GaussianNum; p++)
        {
          for (int q=0; q<basis_.domain_dimension(); q++)
            coords_[p].push_back(gauss_.GaussianPoints[p][q]);
          weights_[p] = gauss_.GaussianWeights[p];
        }
        
				vol_ = basis_.domain_size();
      }
    }

    inline void get_nodes_normals_and_weights(typename FIELD::mesh_type::Elem::index_type idx, std::vector<Point>& gpoints, std::vector<Vector>& gnormals, std::vector<double>& gweights)
    {    
      gpoints.resize(gauss_.GaussianNum);
      gweights.resize(gauss_.GaussianNum);
      gnormals.resize(gauss_.GaussianNum);
      
      for (int k=0; k < coords_.size(); k++)
      {
        mesh_->interpolate(gpoints[k],coords_[k],idx);
        mesh_->get_normal(gnormals[k],coords_[k],idx,0);
        gweights[k] = weights_[k];
      }
    }
        
    inline void get_nodes_normals_and_iweights(typename FIELD::mesh_type::Elem::index_type idx, std::vector<Point>& gpoints, std::vector<Vector>& gnormals, std::vector<double>& gweights)
    {    
      
      gpoints.resize(gauss_.GaussianNum);
      gweights.resize(gauss_.GaussianNum);
      gnormals.resize(gauss_.GaussianNum);
      
      for (int k=0; k < coords_.size(); k++)
      {

        mesh_->interpolate(gpoints[k],coords_[k],idx);
        mesh_->get_normal(gnormals[k],coords_[k],idx,0);
				gweights[k] = weights_[k]*(vol_/mesh_->det_jacobian(coords_[k],idx));
      }
    }

  private:
    FIELD*                                 field_;
    typename FIELD::mesh_type*             mesh_;
    typename FIELD::mesh_type::basis_type  basis_;
    GAUSSIAN gauss_;

    std::vector<std::vector<double> > coords_;
    std::vector<double> weights_;
    double vol_;
};


template <class FIELD, int SIZE >
class NormalRegularIntegration 
{
  public:

    inline NormalRegularIntegration(FIELD* field)
    {
      field_ = field;
      if (field_)
      {
        mesh_  = field->get_typed_mesh().get_rep();
        basis_ = mesh_->get_basis();
        
        int dim = basis_.domain_dimension();
        if (dim == 1)
        {
          coords_.resize(SIZE);
          weights_.resize(SIZE);
          
          for (int p=0; p<SIZE; p++)
          {
            coords_[p].push_back((0.5+p)/SIZE);
            weights_[p] = 1/SIZE;
          }          
        }
        
        if (dim == 2)
        {
          coords_.resize(SIZE*SIZE);
          weights_.resize(SIZE*SIZE);
          
          for (int p=0; p<SIZE; p++)
          {
            for (int q=0; q<SIZE; q++)
            {
              coords_[p+q*SIZE].push_back((0.5+p)/SIZE);
              coords_[p+q*SIZE].push_back((0.5+q)/SIZE);
              weights_[p+q*SIZE] = 1/(SIZE*SIZE);
            }
          }         
        }

        if (dim == 3)
        {
          coords_.resize(SIZE*SIZE*SIZE);
          weights_.resize(SIZE*SIZE*SIZE);
          
          for (int p=0; p<SIZE; p++)
          {
            for (int q=0; q<SIZE; q++)
            {
              for (int r=0; r<SIZE; r++)
              {
                coords_[p+q*SIZE+r*SIZE*SIZE].push_back((0.5+p)/SIZE);
                coords_[p+q*SIZE+r*SIZE*SIZE].push_back((0.5+q)/SIZE);
                coords_[p+q*SIZE+r*SIZE*SIZE].push_back((0.5+r)/SIZE);
                weights_[p+q*SIZE+r*SIZE*SIZE] = 1/(SIZE*SIZE*SIZE);
              }
            }
          }         
        }
      }  
    }

    inline void get_nodes_normals_and_weights(typename FIELD::mesh_type::Elem::index_type idx, std::vector<Point>& gpoints, std::vector<Vector>& gnormals, std::vector<double>& gweights)
    {    
      gpoints.resize(weights_.size());
      gweights.resize(weights_.size());
      gnormals.resize(weights_.size());
      
      for (int k=0; k < weights_.size(); k++)
      {
        mesh_->interpolate(gpoints[k],coords_[k],idx);
        mesh_->get_normal(gnormals[k],coords_[k],idx,0);
        gweights[k] = weights_[k];
      }
    }

    inline void get_nodes_normals_and_iweights(typename FIELD::mesh_type::Elem::index_type idx, std::vector<Point>& gpoints, std::vector<Vector>& gnormals, std::vector<double>& gweights)
    {    
      gpoints.resize(weights_.size());
      gweights.resize(weights_.size());
      gnormals.resize(weights_.size());
      
      for (int k=0; k < weights_.size(); k++)
      {
        mesh_->interpolate(gpoints[k],coords_[k],idx);
        mesh_->get_normal(gnormals[k],coords_[k],idx,0);
				gweights[k] = weights_[k]*(1.0/mesh_->det_jacobian(coords_[k],idx));
      }
    }


  private:
    FIELD*                                 field_;
    typename FIELD::mesh_type*             mesh_;
    typename FIELD::mesh_type::basis_type  basis_;

    std::vector<std::vector<double> > coords_;
    std::vector<double> weights_;

};

} // end namespace SCIRunAlgo

#endif // CORE_ALGORITHMS_FIELDS_MAPPING_H
