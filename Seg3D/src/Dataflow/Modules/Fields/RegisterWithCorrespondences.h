

/*
 *  SmoothVecFieldsMedians.h:
 *
 *  Written by:
 *   Darrell Swenson
 *   May 2008
 *
 */

#if !defined(SmoothVecFieldMedian_h)
#define SmoothVecFieldMedian_h

#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>

//! for Windows support
#include <Dataflow/Modules/Fields/share.h>

namespace SCIRun {

class SCISHARE SmoothVecFieldMedianAlgo : public DynamicAlgoBase
{
public:
  virtual FieldHandle execute(FieldHandle& src) = 0;

  //! support the dynamically compiled algorithm concept
  static CompileInfoHandle get_compile_info(const TypeDescription *iftd,
					    const string &oftn);
};


template< class IFIELD, class OFIELD >
class SmoothVecFieldMedianAlgoT : public SmoothVecFieldMedianAlgo
{
public:
  //! virtual interface. 
  virtual FieldHandle execute(FieldHandle& src);
};


template< class IFIELD, class OFIELD >
FieldHandle
SmoothVecFieldMedianAlgoT<IFIELD, OFIELD>::execute(FieldHandle& field_h)
{
  IFIELD *ifield = (IFIELD *) field_h.get_rep();

  OFIELD *ofield = scinew OFIELD(ifield->get_typed_mesh());

  typename IFIELD::fdata_type::iterator in  = ifield->fdata().begin();
  typename IFIELD::fdata_type::iterator end = ifield->fdata().end();
  typename OFIELD::fdata_type::iterator out = ofield->fdata().begin();

  while (in != end)
  {
    *out = in->length();
    ++in; ++out;
  }

  return FieldHandle( ofield );
}


} // end namespace SCIRun

#endif // SmoothVecFieldMedian_h















