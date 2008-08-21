
#ifndef CORE_ALGORITHMS_FIELDS_FIELDDATA_SMOOTHVECFIELDMEDIAN_H
#define CORE_ALGORITHMS_FIELDS_FIELDDATA_SMOOTHVECFIELDMEDIAN_H 1

//! Datatypes that the algorithm uses
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>

//! Base class for algorithm
#include <Core/Algorithms/Util/AlgoBase.h>

//! for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SCISHARE SmoothVecFieldMedianAlgo : public AlgoBase
{
  public:
    //! Set defaults
    SmoothVecFieldMedianAlgo()
    {
    }
  
    //! run the algorithm
    bool run(FieldHandle input, FieldHandle& output);
};

} // end namespace SCIRun

#endif
