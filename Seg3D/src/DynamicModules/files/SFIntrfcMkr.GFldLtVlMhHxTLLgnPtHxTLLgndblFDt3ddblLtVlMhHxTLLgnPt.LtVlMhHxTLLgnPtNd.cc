// This is an automatically generated file, do not edit!
#include "Core/Geometry/Point.h"
#include "Core/Basis/HexTrilinearLgn.h"
#include "Core/Datatypes/LatVolMesh.h"
#include "Core/Containers/FData.h"
#include "Core/Datatypes/GenericField.h"
#include "Core/Datatypes/FieldInterfaceAux.h"
using namespace SCIRun;

extern "C" {
ScalarFieldInterfaceMaker* maker_SFIntrfcMkr_GFldLtVlMhHxTLLgnPtHxTLLgndblFDt3ddblLtVlMhHxTLLgnPt_LtVlMhHxTLLgnPtNd () {
  return scinew SFInterfaceMaker<GenericField<LatVolMesh<HexTrilinearLgn<Point> > ,HexTrilinearLgn<double> ,FData3d<double,LatVolMesh<HexTrilinearLgn<Point> > > > , LatVolMesh<HexTrilinearLgn<Point> > ::Node>;
}
}
