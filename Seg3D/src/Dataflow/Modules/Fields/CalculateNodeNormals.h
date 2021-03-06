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


//    File   : CalculateNodeNormals.h
//    Author : Michael Callahan
//    Date   : June 2001

#if !defined(CalculateNodeNormals_h)
#define CalculateNodeNormals_h

#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <Core/Containers/Handle.h>
#include <Core/Geometry/Vector.h>

//! for Windows support
#include <Dataflow/Modules/Fields/share.h>

namespace SCIRun {


class Attractor : public Datatype
{
public:
  virtual void execute(Vector &v, const Point &p);

  virtual void io(Piostream &s);
};

typedef Handle<Attractor> AttractorHandle;


class SCISHARE CalculateNodeNormalsAlgo : public DynamicAlgoBase
{
public:
  virtual FieldHandle execute(ProgressReporter *reporter,
                              FieldHandle src, AttractorHandle a) = 0;

  //! support the dynamically compiled algorithm concept
  static CompileInfoHandle get_compile_info(const TypeDescription *fsrc,
					    const TypeDescription *floc,
					    const TypeDescription *msrc,
                                            const string &fdst,
					    bool scale_p);
};


template <class MSRC, class FLOC, class FDST>
class CalculateNodeNormalsAlgoT : public CalculateNodeNormalsAlgo
{
public:
  //! virtual interface. 
  virtual FieldHandle execute(ProgressReporter *reporter,
                              FieldHandle src, AttractorHandle a);
};



template <class MSRC, class FLOC, class FDST>
FieldHandle
CalculateNodeNormalsAlgoT<MSRC, FLOC, FDST>::execute(ProgressReporter *reporter,
                                               FieldHandle field_h,
					       AttractorHandle attr)
{
  MSRC *mesh = static_cast<MSRC *>(field_h->mesh().get_rep());
  FDST *fdst = scinew FDST(mesh);

  typename FLOC::iterator bi, ei;
  mesh->begin(bi);
  mesh->end(ei);

  typename FLOC::size_type prsizetmp;
  mesh->size(prsizetmp);
  const unsigned int prsize = (unsigned int)prsizetmp;
  unsigned int prcounter = 0;

  while (bi != ei)
  {
    reporter->update_progress(prcounter++, prsize);

    Point c;

    mesh->get_center(c, *bi);

    Vector vec;
    attr->execute(vec, c);

    fdst->set_value(vec, *bi);

    ++bi;
  }
  
  return fdst;
}


template <class FSRC, class FLOC, class FDST>
class CalculateNodeNormalsScaleAlgoT : public CalculateNodeNormalsAlgo
{
public:
  //! virtual interface. 
  virtual FieldHandle execute(ProgressReporter *reporter,
                              FieldHandle src, AttractorHandle a);
};



template <class FSRC, class FLOC, class FDST>
FieldHandle
CalculateNodeNormalsScaleAlgoT<FSRC, FLOC, FDST>::execute(ProgressReporter *reporter,
                                                    FieldHandle field_h,
						    AttractorHandle attr)
{
  FSRC *fsrc = static_cast<FSRC *>(field_h.get_rep());
  typename FSRC::mesh_handle_type mesh = fsrc->get_typed_mesh();
  FDST *fdst = scinew FDST(mesh);

  typename FLOC::iterator bi, ei;
  mesh->begin(bi);
  mesh->end(ei);

  typename FLOC::size_type prsizetmp;
  mesh->size(prsizetmp);
  const unsigned int prsize = (unsigned int)prsizetmp;
  unsigned int prcounter = 0;

  while (bi != ei)
  {
    reporter->update_progress(prcounter++, prsize);

    Point c;

    mesh->get_center(c, *bi);

    Vector vec;
    attr->execute(vec, c);

    typename FSRC::value_type val;
    fsrc->value(val, *bi);
    if (val == 0) { val = 1.0e-3; }
    vec = vec * val;
    
    fdst->set_value(vec, *bi);

    ++bi;
  }
  
  return fdst;
}


} // end namespace SCIRun

#endif // CalculateNodeNormals_h
