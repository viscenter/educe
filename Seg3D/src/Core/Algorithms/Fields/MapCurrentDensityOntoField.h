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

#ifndef CORE_ALGORITHMS_FIELDS_CURRENTDENSITYMAPPING_H
#define CORE_ALGORITHMS_FIELDS_CURRENTDENSITYMAPPING_H 1

#include <Core/Algorithms/Fields/Mapping.h>

//! for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {


using namespace SCIRun;

class SCISHARE MapCurrentDensityOntoFieldAlgo : public DynamicAlgoBase
{
  public:
  
    virtual bool MapCurrentDensityOntoField(ProgressReporter* pr,
                                    int numproc, FieldHandle pot_src, FieldHandle con_src,
                                    FieldHandle dst, FieldHandle& output,
                                    std::string mappingmethod,
                                    std::string integrationmethod,
                                    std::string integrationfilter,
                                    bool multiply_with_normal,
                                    bool calcnorm);      
};


template <class INTEGRATOR, class FPOT, class FCON, class FDST, class FOUT>
class MapCurrentDensityOntoFieldAlgoT : public MapCurrentDensityOntoFieldAlgo
{
public:
  virtual bool MapCurrentDensityOntoField(ProgressReporter* pr,
                                    int numproc, FieldHandle pot_src, FieldHandle con_src,
                                    FieldHandle dst, FieldHandle& output,
                                    std::string mappingmethod,
                                    std::string integrationmethod,
                                    std::string integrationfilter,
                                    bool multiply_with_normal,
                                    bool calcnorm); 
                       
  class IData
  {
    public:
      ProgressReporter*         pr;
      FPOT*                     pfield;
      FCON*                     cfield;
      FOUT*                     ofield;
      typename FPOT::mesh_type* pmesh;
      typename FCON::mesh_type* cmesh;
      typename FOUT::mesh_type* omesh;
      int                       numproc;
      std::string               integrationfilter;
      bool                      multiply_with_normal;
      bool                      retval;
  };
  
  void parallel_unsafe(int procnum,IData* inputdata); 
  void parallel(int procnum,IData* inputdata); 
};

template <class INTEGRATOR, class FPOT, class FCON, class FDST, class FOUT>
class MapCurrentDensityOntoFieldNormalAlgoT : public MapCurrentDensityOntoFieldAlgo
{
public:
  virtual bool MapCurrentDensityOntoField(ProgressReporter* pr,
                                    int numproc, FieldHandle pot_src, FieldHandle con_src,
                                    FieldHandle dst, FieldHandle& output,
                                    std::string mappingmethod,
                                    std::string integrationmethod,
                                    std::string integrationfilter,
                                    bool multiply_with_normal,
                                    bool calcnorm); 
                       
  class IData
  {
    public:
      ProgressReporter*         pr;
      FPOT*                     pfield;
      FCON*                     cfield;
      FOUT*                     ofield;
      typename FPOT::mesh_type* pmesh;
      typename FCON::mesh_type* cmesh;
      typename FOUT::mesh_type* omesh;
      int                       numproc;
      std::string               integrationfilter;
      bool                      multiply_with_normal;
      bool                      retval;
  };
  
  void parallel_unsafe(int procnum,IData* inputdata); 
  void parallel(int procnum,IData* inputdata); 
};

template <class INTEGRATOR, class FPOT, class FCON, class FDST, class FOUT>
class MapCurrentDensityOntoFieldNormAlgoT : public MapCurrentDensityOntoFieldAlgo
{
public:
  virtual bool MapCurrentDensityOntoField(ProgressReporter* pr,
                                    int numproc, FieldHandle pot_src, FieldHandle con_src,
                                    FieldHandle dst, FieldHandle& output,
                                    std::string mappingmethod,
                                    std::string integrationmethod,
                                    std::string integrationfilter,
                                    bool multiply_with_normal,
                                    bool calcnorm); 
                       
  class IData
  {
    public:
      ProgressReporter*         pr;
      FPOT*                     pfield;
      FCON*                     cfield;
      FOUT*                     ofield;
      typename FPOT::mesh_type* pmesh;
      typename FCON::mesh_type* cmesh;
      typename FOUT::mesh_type* omesh;
      int                       numproc;
      std::string               integrationfilter;
      bool                      multiply_with_normal;
      bool                      retval;
  };
  
  void parallel_unsafe(int procnum,IData* inputdata); 
  void parallel(int procnum,IData* inputdata); 
};



template <class INTEGRATOR, class FPOT, class FCON, class FDST, class FOUT>
bool MapCurrentDensityOntoFieldAlgoT<INTEGRATOR,FPOT,FCON,FDST,FOUT>::MapCurrentDensityOntoField(ProgressReporter *pr,
                       int numproc, FieldHandle pot, FieldHandle con,FieldHandle dst, FieldHandle& output, 
                       std::string mappingmethod,
                       std::string integrationmethod,
                       std::string integrationfilter,
                       bool multiply_with_normal,
                       bool calcnorm) 
{
  FPOT* pfield = dynamic_cast<FPOT*>(pot.get_rep());
  if (pfield == 0)
  {
    pr->error("MapCurrentDensityOntoField: No input potential field was given");
    return (false);
  }

  typename FPOT::mesh_type* pmesh = dynamic_cast<typename FPOT::mesh_type*>(pot->mesh().get_rep());
  if (pmesh == 0)
  {
    pr->error("MapCurrentDensityOntoField: No mesh is associated with input potential field");
    return (false);  
  }

  FCON* cfield = dynamic_cast<FCON*>(con.get_rep());
  if (cfield == 0)
  {
    pr->error("MapCurrentDensityOntoField: No input conductivity field was given");
    return (false);
  }

  typename FCON::mesh_type* cmesh = dynamic_cast<typename FCON::mesh_type*>(con->mesh().get_rep());
  if (cmesh == 0)
  {
    pr->error("MapCurrentDensityOntoField: No mesh is associated with input conductivity field");
    return (false);  
  }

  FDST* dfield = dynamic_cast<FDST*>(dst.get_rep());
  if (dfield == 0)
  {
    pr->error("MapCurrentDensityOntoField: No input destination field was given");
    return (false);
  }

  typename FDST::mesh_type* dmesh = dynamic_cast<typename FDST::mesh_type*>(dst->mesh().get_rep());
  if (dmesh == 0)
  {
    pr->error("MapCurrentDensityOntoField: No mesh is associated with input destination field");
    return (false);  
  }

  output = dynamic_cast<Field *>(new FOUT(dmesh));
  if (output.get_rep() == 0)
  {
    pr->error("MapCurrentDensityOntoField: Could no allocate output field");
    return (false);
  }
  
  FOUT* ofield = dynamic_cast<FOUT*>(output.get_rep());
  ofield->resize_fdata();
  
  output->copy_properties(dst.get_rep());

  // Now do parallel algorithm

  IData IData;
  IData.pfield = pfield;
  IData.cfield = cfield;
  IData.ofield = ofield;
  IData.pmesh  = pmesh;
  IData.cmesh  = cmesh;
  IData.omesh  = dmesh;
  IData.pr     = pr;
  IData.retval = true;
  IData.integrationfilter = integrationfilter;
  
  // Determine the number of processors to use:
  int np = Thread::numProcessors();  
  if (numproc > 0) { np = numproc; }
  IData.numproc = np;

  Thread::parallel(this,&MapCurrentDensityOntoFieldAlgoT<INTEGRATOR,FPOT,FCON,FDST,FOUT>::parallel,np,&IData);
    
  return (IData.retval);
}



template <class INTEGRATOR, class FPOT, class FCON, class FDST, class FOUT>
bool MapCurrentDensityOntoFieldNormAlgoT<INTEGRATOR,FPOT,FCON,FDST,FOUT>::MapCurrentDensityOntoField(ProgressReporter *pr,
                       int numproc, FieldHandle pot, FieldHandle con,FieldHandle dst, FieldHandle& output, 
                       std::string mappingmethod,
                       std::string integrationmethod,
                       std::string integrationfilter,
                       bool multiply_with_normal,
                       bool calcnorm) 
{
  FPOT* pfield = dynamic_cast<FPOT*>(pot.get_rep());
  if (pfield == 0)
  {
    pr->error("MapCurrentDensityOntoField: No input potential field was given");
    return (false);
  }

  typename FPOT::mesh_type* pmesh = dynamic_cast<typename FPOT::mesh_type*>(pot->mesh().get_rep());
  if (pmesh == 0)
  {
    pr->error("MapCurrentDensityOntoField: No mesh is associated with input potential field");
    return (false);  
  }

  FCON* cfield = dynamic_cast<FCON*>(con.get_rep());
  if (cfield == 0)
  {
    pr->error("MapCurrentDensityOntoField: No input conductivity field was given");
    return (false);
  }

  typename FCON::mesh_type* cmesh = dynamic_cast<typename FCON::mesh_type*>(con->mesh().get_rep());
  if (cmesh == 0)
  {
    pr->error("MapCurrentDensityOntoField: No mesh is associated with input conductivity field");
    return (false);  
  }

  FDST* dfield = dynamic_cast<FDST*>(dst.get_rep());
  if (dfield == 0)
  {
    pr->error("MapCurrentDensityOntoField: No input destination field was given");
    return (false);
  }

  typename FDST::mesh_type* dmesh = dynamic_cast<typename FDST::mesh_type*>(dst->mesh().get_rep());
  if (dmesh == 0)
  {
    pr->error("MapCurrentDensityOntoField: No mesh is associated with input destination field");
    return (false);  
  }

  output = dynamic_cast<Field *>(new FOUT(dmesh));
  if (output.get_rep() == 0)
  {
    pr->error("MapCurrentDensityOntoField: Could no allocate output field");
    return (false);
  }
  
  FOUT* ofield = dynamic_cast<FOUT*>(output.get_rep());
  ofield->resize_fdata();
  
  output->copy_properties(dst.get_rep());

  // Now do parallel algorithm

  IData IData;
  IData.pfield = pfield;
  IData.cfield = cfield;
  IData.ofield = ofield;
  IData.pmesh  = pmesh;
  IData.cmesh  = cmesh;
  IData.omesh  = dmesh;
  IData.pr     = pr;
  IData.retval = true;
  IData.integrationfilter = integrationfilter;
  
  // Determine the number of processors to use:
  int np = Thread::numProcessors(); 
  if (numproc > 0) { np = numproc; }
  IData.numproc = np;
   
  Thread::parallel(this,&MapCurrentDensityOntoFieldNormAlgoT<INTEGRATOR,FPOT,FCON,FDST,FOUT>::parallel,np,&IData);
    
  return (IData.retval);
}


template <class INTEGRATOR, class FPOT, class FCON, class FDST, class FOUT>
void MapCurrentDensityOntoFieldAlgoT<INTEGRATOR,FPOT,FCON,FDST,FOUT>::parallel_unsafe(int procnum,IData* idata)
{
  typename FOUT::mesh_type::Elem::iterator it, eit;
  typename FOUT::mesh_type::Elem::size_type s;
  typename FOUT::mesh_type* omesh = idata->omesh;
  typename FOUT::value_type val, val2;
  FOUT* ofield = idata->ofield;
  
  int numproc = idata->numproc;
  ProgressReporter *pr = idata->pr;
  
  omesh->begin(it);
  omesh->end(eit);
  omesh->size(s);
  
  int cnt = 0;
  
  InterpolatedGradient<FPOT> pmapping(idata->pfield);
  InterpolatedData<FCON> cmapping(idata->cfield);

  INTEGRATOR integrator(idata->ofield);
  
  for (int p =0; p < procnum; p++) if (it != eit) ++it;
  std::vector<Point> points;
  std::vector<double> weights;
  std::string filter = idata->integrationfilter;
  typename FCON::value_type con;
  typename FOUT::value_type grad;
  Vector g;
  
  // Determine the filter and loop over nodes
  if ((filter == "median")||(filter == "Median"))
  {
    // median filter over integration nodes
    while (it != eit)
    {
      integrator.get_nodes_and_weights(*it,points,weights);
      std::vector<typename FOUT::value_type> valarray(points.size());

      for (size_t p = 0; p < points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];          
          valarray[p] = -(con*g);
        }
        else
        {
          valarray[p] = typename FOUT::value_type(0);
        }
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
      typename FOUT::value_type val(0);
      typename FOUT::value_type tval(0);

      if (points.size() > 0)
      {
        if (pmapping.get_gradient(points[0],grad)&&cmapping.get_data(points[0],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          val = -(con*g); 
        }
        else
        {
          val = typename FOUT::value_type(0);
        }
        for (size_t p = 1; p < points.size(); p++)
        {
          if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
          {
            g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
            tval = -(con*g); 
          }
          else
          {
            tval = typename FOUT::value_type(0);
          }
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
      typename FOUT::value_type val(0);
      typename FOUT::value_type tval(0);

      if (points.size() > 0)
      {
        if (pmapping.get_gradient(points[0],grad)&&cmapping.get_data(points[0],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          val = -(con*g); 
        }
        else
        {
          val = typename FOUT::value_type(0);
        }
        for (size_t p = 1; p < points.size(); p++)
        {
          if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
          {
            g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
            tval = -(con*g); 
          }
          else
          {
            tval = typename FOUT::value_type(0);
          }
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
    while (it != eit)
    {
      integrator.get_nodes_and_weights(*it,points,weights);
      std::vector<typename FOUT::value_type> valarray(points.size());

      for (size_t p = 0; p < points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          valarray[p] = -(con*g); 
        }
        else
        {
          valarray[p] = typename FOUT::value_type(0);
        }
      }
      sort(valarray.begin(),valarray.end());
       
      typename FOUT::value_type rval(0);
      typename FOUT::value_type val(0);
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

      val = typename FOUT::value_type(0);
      for (int p=0; p<points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          val2 = -(con*g); 
        }
        else
        {
          val2 = typename FOUT::value_type(0);
        }
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

      val = typename FOUT::value_type(0);
      for (int p=0; p<points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];        
          val2 = -(con*g); 
        }
        else
        {
          val2 = typename FOUT::value_type(0);
        }
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
      
      val = typename FOUT::value_type(0);
      for (int p=0; p<points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];        
          val2 = -(con*g); 
        }
        else
        {
          val2 = typename FOUT::value_type(0);
        }
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
      
      val = typename FOUT::value_type(0);
      for (int p=0; p<points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          val2 = -(con*g); 
        }
        else
        {
          val2 = typename FOUT::value_type(0);
        }        
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
      idata->pr->error("CurrentDensintyMapping: Filter method is unknown");
      idata->retval = false;
    }
    return;
  }
}


template <class INTEGRATOR, class FPOT, class FCON, class FDST, class FOUT>
void MapCurrentDensityOntoFieldAlgoT<INTEGRATOR,FPOT,FCON,FDST,FOUT>::parallel(int procnum,IData* idata)
{
  try {
    parallel_unsafe(procnum, idata);
  }
  catch (const Exception &e)
  {
    idata->pr->error(string("Crashed with the following exception:\n")+
              e.message());
    idata->retval = false;
  }
  catch (const string a)
  {
    idata->pr->error(a);
    idata->retval = false;
  }
  catch (const char *a)
  {
    idata->pr->error(string(a));
    idata->retval = false;
  }	
  catch (...)
  {
    idata->pr->error(string("Crashed for unknown reason."));
    idata->retval = false;
  }	
}




template <class INTEGRATOR, class FPOT, class FCON, class FDST, class FOUT>
void MapCurrentDensityOntoFieldNormAlgoT<INTEGRATOR,FPOT,FCON,FDST,FOUT>::parallel_unsafe(int procnum,IData* idata)
{
  typename FOUT::mesh_type::Elem::iterator it, eit;
  typename FOUT::mesh_type::Elem::size_type s;
  typename FOUT::mesh_type* omesh = idata->omesh;
  typename FOUT::value_type val, val2;
  FOUT* ofield = idata->ofield;
  
  int numproc = idata->numproc;
  ProgressReporter *pr = idata->pr;
  
  omesh->begin(it);
  omesh->end(eit);
  omesh->size(s);
  
  int cnt = 0;
  
  InterpolatedGradient<FPOT> pmapping(idata->pfield);
  InterpolatedData<FCON> cmapping(idata->cfield);

  INTEGRATOR integrator(idata->ofield);
  
  for (int p =0; p < procnum; p++) if (it != eit) ++it;
  std::vector<Point> points;
  std::vector<double> weights;
  std::string filter = idata->integrationfilter;
  typename FCON::value_type con;
  Vector grad;
  Vector g;
  
  
  
  // Determine the filter and loop over nodes
  if ((filter == "median")||(filter == "Median"))
  {
    // median filter over integration nodes
    while (it != eit)
    {
      integrator.get_nodes_and_weights(*it,points,weights);
      std::vector<typename FOUT::value_type> valarray(points.size());

      for (size_t p = 0; p < points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          Vector v = con*g;
          valarray[p] = static_cast<typename FOUT::value_type>(v.length());
        }
        else
        {
          valarray[p] = typename FOUT::value_type(0);
        }
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
      typename FOUT::value_type val(0);
      typename FOUT::value_type tval(0);

      if (points.size() > 0)
      {
        if (pmapping.get_gradient(points[0],grad)&&cmapping.get_data(points[0],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          Vector v = con*g;
          val = static_cast<typename FOUT::value_type>(v.length()); 
        }
        else
        {
          val = typename FOUT::value_type(0);
        }
        for (size_t p = 1; p < points.size(); p++)
        {
          if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
          {
            g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
            Vector v = con*g;
            tval = static_cast<typename FOUT::value_type>(v.length()); 
          }
          else
          {
            tval = typename FOUT::value_type(0);
          }
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
      typename FOUT::value_type val(0);
      typename FOUT::value_type tval(0);

      if (points.size() > 0)
      {
        if (pmapping.get_gradient(points[0],grad)&&cmapping.get_data(points[0],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          Vector v = con*g;
          val = static_cast<typename FOUT::value_type>(v.length());
        }
        else
        {
          val = typename FOUT::value_type(0);
        }
        for (size_t p = 1; p < points.size(); p++)
        {
          if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
          {
            g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
            Vector v = con*g;
            tval = static_cast<typename FOUT::value_type>(v.length()); 
          }
          else
          {
            tval = typename FOUT::value_type(0);
          }
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
    while (it != eit)
    {
      integrator.get_nodes_and_weights(*it,points,weights);
      std::vector<typename FOUT::value_type> valarray(points.size());

      for (size_t p = 0; p < points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          Vector v = con*g;
          valarray[p] = static_cast<typename FOUT::value_type>(v.length()); 
        }
        else
        {
          valarray[p] = typename FOUT::value_type(0);
        }
      }
      sort(valarray.begin(),valarray.end());
       
      typename FOUT::value_type rval(0);
      typename FOUT::value_type val(0);
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

      val = typename FOUT::value_type(0);
      for (int p=0; p<points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          Vector v = con*g;
          val2 = static_cast<typename FOUT::value_type>(v.length()); 
        }
        else
        {
          val2 = typename FOUT::value_type(0);
        }
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

      val = typename FOUT::value_type(0);
      for (int p=0; p<points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];        
          Vector v = con*g;
          val2 = static_cast<typename FOUT::value_type>(v.length()); 
        }
        else
        {
          val2 = typename FOUT::value_type(0);
        }
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
      
      val = typename FOUT::value_type(0);
      for (int p=0; p<points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];        
          Vector v = con*g;
          val2 = static_cast<typename FOUT::value_type>(v.length()); 
        }
        else
        {
          val2 = typename FOUT::value_type(0);
        }
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
      
      val = typename FOUT::value_type(0);
      for (int p=0; p<points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          Vector v = con*g;
          val2 = static_cast<typename FOUT::value_type>(v.length()); 
        }
        else
        {
          val2 = typename FOUT::value_type(0);
        }        
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
      idata->pr->error("CurrentDensintyMapping: Filter method is unknown");
      idata->retval = false;
    }
    return;
  }
}


template <class INTEGRATOR, class FPOT, class FCON, class FDST, class FOUT>
void MapCurrentDensityOntoFieldNormAlgoT<INTEGRATOR,FPOT,FCON,FDST,FOUT>::parallel(int procnum,IData* idata)
{
  try {
    parallel_unsafe(procnum, idata);
  }
  catch (const Exception &e)
  {
    idata->pr->error(string("Crashed with the following exception:\n")+
              e.message());
    idata->retval = false;
  }
  catch (const string a)
  {
    idata->pr->error(a);
    idata->retval = false;
  }
  catch (const char *a)
  {
    idata->pr->error(string(a));
    idata->retval = false;
  }	
  catch (...)
  {
    idata->pr->error(string("Crashed for unknown reason."));
    idata->retval = false;
  }	
}



template <class INTEGRATOR, class FPOT, class FCON, class FDST, class FOUT>
bool MapCurrentDensityOntoFieldNormalAlgoT<INTEGRATOR,FPOT,FCON,FDST,FOUT>::MapCurrentDensityOntoField(ProgressReporter *pr,
                       int numproc, FieldHandle pot, FieldHandle con,FieldHandle dst, FieldHandle& output, 
                       std::string mappingmethod,
                       std::string integrationmethod,
                       std::string integrationfilter,
                       bool multiply_with_normal,
                       bool calcnorm) 
{
  FPOT* pfield = dynamic_cast<FPOT*>(pot.get_rep());
  if (pfield == 0)
  {
    pr->error("MapCurrentDensityOntoField: No input potential field was given");
    return (false);
  }

  typename FPOT::mesh_type* pmesh = dynamic_cast<typename FPOT::mesh_type*>(pot->mesh().get_rep());
  if (pmesh == 0)
  {
    pr->error("MapCurrentDensityOntoField: No mesh is associated with input potential field");
    return (false);  
  }

  FCON* cfield = dynamic_cast<FCON*>(con.get_rep());
  if (cfield == 0)
  {
    pr->error("MapCurrentDensityOntoField: No input conductivity field was given");
    return (false);
  }

  typename FCON::mesh_type* cmesh = dynamic_cast<typename FCON::mesh_type*>(con->mesh().get_rep());
  if (cmesh == 0)
  {
    pr->error("MapCurrentDensityOntoField: No mesh is associated with input conductivity field");
    return (false);  
  }

  FDST* dfield = dynamic_cast<FDST*>(dst.get_rep());
  if (dfield == 0)
  {
    pr->error("MapCurrentDensityOntoField: No input destination field was given");
    return (false);
  }

  typename FDST::mesh_type* dmesh = dynamic_cast<typename FDST::mesh_type*>(dst->mesh().get_rep());
  if (dmesh == 0)
  {
    pr->error("MapCurrentDensityOntoField: No mesh is associated with input destination field");
    return (false);  
  }

  output = dynamic_cast<Field *>(new FOUT(dmesh));
  if (output.get_rep() == 0)
  {
    pr->error("MapCurrentDensityOntoField: Could no allocate output field");
    return (false);
  }
  
  FOUT* ofield = dynamic_cast<FOUT*>(output.get_rep());
  ofield->resize_fdata();
  
  output->copy_properties(dst.get_rep());

  // Now do parallel algorithm

  IData IData;
  IData.pfield = pfield;
  IData.cfield = cfield;
  IData.ofield = ofield;
  IData.pmesh  = pmesh;
  IData.cmesh  = cmesh;
  IData.omesh  = dmesh;
  IData.pr     = pr;
  IData.retval = true;
  IData.integrationfilter = integrationfilter;
  
  // Determine the number of processors to use:
  int np = Thread::numProcessors();  
  if (numproc > 0) { np = numproc; }
  IData.numproc = np;
  Thread::parallel(this,&MapCurrentDensityOntoFieldNormalAlgoT<INTEGRATOR,FPOT,FCON,FDST,FOUT>::parallel,np,&IData);
    
  return (IData.retval);
}



template <class INTEGRATOR, class FPOT, class FCON, class FDST, class FOUT>
void MapCurrentDensityOntoFieldNormalAlgoT<INTEGRATOR,FPOT,FCON,FDST,FOUT>::parallel_unsafe(int procnum,IData* idata)
{
  typename FOUT::mesh_type::Elem::iterator it, eit;
  typename FOUT::mesh_type::Elem::size_type s;
  typename FOUT::mesh_type* omesh = idata->omesh;
  typename FOUT::value_type val, val2;
  FOUT* ofield = idata->ofield;
  
  int numproc = idata->numproc;
  ProgressReporter *pr = idata->pr;
  
  omesh->begin(it);
  omesh->end(eit);
  omesh->size(s);
  
  int cnt = 0;
  
  InterpolatedGradient<FPOT> pmapping(idata->pfield);
  InterpolatedData<FCON> cmapping(idata->cfield);

  INTEGRATOR integrator(idata->ofield);
  
  for (int p =0; p < procnum; p++) if (it != eit) ++it;
  std::vector<Point> points;
  std::vector<double> weights;
  std::vector<Vector> normals;
  std::string filter = idata->integrationfilter;
  typename FCON::value_type con;
  Vector grad;
  Vector g;
  
  // Determine the filter and loop over nodes
  if ((filter == "median")||(filter == "Median"))
  {
    // median filter over integration nodes
    std::vector<typename FOUT::value_type> valarray;
    while (it != eit)
    {
      integrator.get_nodes_normals_and_weights(*it,points,normals,weights);
      valarray.resize(points.size());

      for (size_t p = 0; p < points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          valarray[p] = -Dot(normals[p],con*g);         
        }
        else
        {
          valarray[p] = typename FOUT::value_type(0); 
        }
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
      integrator.get_nodes_normals_and_weights(*it,points,normals,weights);
      typename FOUT::value_type val(0);
      typename FOUT::value_type tval(0);

      if (points.size() > 0)
      {
        if (pmapping.get_gradient(points[0],grad)&&cmapping.get_data(points[0],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          val = -Dot(normals[0],con*g);         
        }
        else
        {
          val = typename FOUT::value_type(0);
        }
        for (size_t p = 1; p < points.size(); p++)
        {
          if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
          {
            g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
            tval = -Dot(normals[p],con*g);         
          }
          else
          {
            tval = typename FOUT::value_type(0);
          }
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
      integrator.get_nodes_normals_and_weights(*it,points,normals,weights);
      typename FOUT::value_type val(0);
      typename FOUT::value_type tval(0);

      if (points.size() > 0)
      {
        if (pmapping.get_gradient(points[0],grad)&&cmapping.get_data(points[0],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          val = -Dot(normals[0],con*g);         
        }
        else
        {
          val = typename FOUT::value_type(0);
        }

        for (size_t p = 1; p < points.size(); p++)
        {
          if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
          {
            g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
            tval = -Dot(normals[p],con*g);         
          }
          else
          {
            tval = 0;
          }
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
      integrator.get_nodes_normals_and_weights(*it,points,normals,weights);
      valarray.resize(points.size());
      for (size_t p = 0; p < points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          valarray[p] = -Dot(normals[p],con*g);         
        }
        else
        {
          valarray[p] = typename FOUT::value_type(0);
        }
      }
      sort(valarray.begin(),valarray.end());
       
      typename FOUT::value_type rval(0);
      typename FOUT::value_type val(0);
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
      integrator.get_nodes_normals_and_iweights(*it,points,normals,weights);

      val = typename FOUT::value_type(0);
      for (int p=0; p<points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          val2 = -Dot(normals[p],con*g);         
        }
        else
        {
          val2 = typename FOUT::value_type(0);
        }
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
      integrator.get_nodes_normals_and_weights(*it,points,normals,weights);

      val = typename FOUT::value_type(0);
      for (int p=0; p<points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          val2 = -Dot(normals[p],con*g);         
        }
        else
        {
          val2 = typename FOUT::value_type(0);
        }
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
      integrator.get_nodes_normals_and_weights(*it,points,normals,weights);
      
      val = typename FOUT::value_type(0);
      for (int p=0; p<points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
          g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
          val2 = -Dot(normals[p],con*g);         
        }
        else
        {
          val2 = typename FOUT::value_type(0);
        }
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
      integrator.get_nodes_normals_and_weights(*it,points,normals,weights);
      
      val = typename FOUT::value_type(0);
      for (int p=0; p<points.size(); p++)
      {
        if (pmapping.get_gradient(points[p],grad)&&cmapping.get_data(points[p],con))
        {
           g[0] = grad[0]; g[1] = grad[1]; g[2] = grad[2];
           val2 = -Dot(normals[p],con*g); 
        }
        else
        {
          val2 = typename FOUT::value_type(0);
        }
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
      idata->pr->error("CurrentDensintyMapping: Filter method is unknown");
      idata->retval = false;
    }
    return;
  }
}

template <class INTEGRATOR, class FPOT, class FCON, class FDST, class FOUT>
void MapCurrentDensityOntoFieldNormalAlgoT<INTEGRATOR,FPOT,FCON,FDST,FOUT>::parallel(int procnum,IData* idata)
{
  try {
    parallel_unsafe(procnum, idata);
  }
  catch (const Exception &e)
  {
    idata->pr->error(string("Crashed with the following exception:\n")+
              e.message());
    idata->retval = false;
  }
  catch (const string a)
  {
    idata->pr->error(a);
    idata->retval = false;
  }
  catch (const char *a)
  {
    idata->pr->error(string(a));
    idata->retval = false;
  }	
  catch (...)
  {
    idata->pr->error(string("Crashed for unknown reason."));
    idata->retval = false;
  }	
}


} // end namespace

#endif

