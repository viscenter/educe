
#ifndef CORE_ALGORITHMS_FIELDS_REGISTERWITHCORRESPONDENCES_H
#define CORE_ALGORITHMS_FIELDS_REGISTERWITHCORRESPONDENCES_H 1

//! Datatypes that the algorithm uses
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/FieldInformation.h>
#include <Core/Util/ProgressReporter.h>
#include <Core/Geometry/Vector.h>
#include <Core/Math/MiscMath.h>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>
//#include <Math.h>

//! Base class for algorithm
#include <Core/Algorithms/Util/AlgoBase.h>

//! for Windows support
#include <Core/Algorithms/Fields/share.h>

namespace SCIRunAlgo {

using namespace SCIRun;

class SCISHARE RegisterWithCorrespondencesAlgo : public AlgoBase
{
  public:
    //! Set defaults
    RegisterWithCorrespondencesAlgo()
    {
    }
  
    //! run the algorithm
    bool runM(FieldHandle input, FieldHandle Cors1, FieldHandle Cors2, FieldHandle& output);
    bool runA(FieldHandle input, FieldHandle Cors1, FieldHandle Cors2, FieldHandle& output);
    bool runN(FieldHandle input, FieldHandle Cors1, FieldHandle Cors2, FieldHandle& output);
    bool radial_basis_func(VMesh* Cors, VMesh* points, MatrixHandle& Smat);
    bool make_new_points(VMesh* points, VMesh* Cors, vector<double>& coefs, VMesh& omesh);
    bool make_new_pointsA(VMesh* points, VMesh* Cors, vector<double>& coefs, VMesh& omesh);


};

} // end namespace SCIRun

#endif
