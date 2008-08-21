// This is an automatically generated file, do not edit!
#include "Core/Geometry/Point.h"
#include "Core/Algorithms/ArrayMath/ArrayObjectFieldAlgo.h"
#include "Core/Basis/HexTrilinearLgn.h"
#include "Core/Datatypes/LatVolMesh.h"
#include "Core/Containers/FData.h"
#include "Core/Datatypes/GenericField.h"
using namespace SCIRun;
using namespace SCIRunAlgo;

extern "C" {
ArrayObjectFieldDataAlgo* maker_ALGOArryObjctFldDtSclrAlgT_GFldLtVlMhHxTLLgnPtHxTLLgndblFDt3ddblLtVlMhHxTLLgnPt_LtVlMhHxTLLgnPtNd () {
  return new ArrayObjectFieldDataScalarAlgoT<GenericField<LatVolMesh<HexTrilinearLgn<Point> > ,HexTrilinearLgn<double> ,FData3d<double,LatVolMesh<HexTrilinearLgn<Point> > > > ,LatVolMesh<HexTrilinearLgn<Point> > ::Node>;
}
}
