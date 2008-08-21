// This is an automatically generated file, do not edit!
#include "Core/Geometry/Point.h"
#include "Core/Basis/HexTrilinearLgn.h"
#include "Core/Datatypes/LatVolMesh.h"
#include "Core/Containers/FData.h"
#include "Core/Datatypes/GenericField.h"
#include "Core/Algorithms/ArrayMath/ArrayObjectFieldAlgo.h"
using namespace SCIRun;
using namespace SCIRunAlgo;

extern "C" {
ArrayObjectFieldLocationAlgo* maker_ALGOArryObjctFldLctnNdAlgT_GFldLtVlMhHxTLLgnPtHxTLLgndblFDt3ddblLtVlMhHxTLLgnPt_LtVlMhHxTLLgnPtNd () {
  return new ArrayObjectFieldLocationNodeAlgoT<GenericField<LatVolMesh<HexTrilinearLgn<Point> > ,HexTrilinearLgn<double> ,FData3d<double,LatVolMesh<HexTrilinearLgn<Point> > > > >;
}
}
