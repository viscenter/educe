// This is an automatically generated file, do not edit!
#include <vector>
#include "Core/Geometry/Point.h"
#include "Core/Basis/TriLinearLgn.h"
#include "Core/Basis/HexTrilinearLgn.h"
#include "Core/Datatypes/TriSurfMesh.h"
#include "Core/Datatypes/LatVolMesh.h"
#include "Core/Containers/FData.h"
#include "Core/Datatypes/GenericField.h"
#include "Dataflow/Modules/Fields/MapFieldDataFromSourceToDestination.h"
using namespace SCIRun;
using namespace std;

extern "C" {
MapFieldDataFromSourceToDestinationAlgo* maker_MpFldDtFrmSrcTDstntnAlgT_GFldLtVlMhHxTLLgnPtHxTLLgndblFDt3ddblLtVlMhHxTLLgnPt_LtVlMhHxTLLgnPtNd_GFldTrSrfMhTrLnrLgnPtTrLnrLgndblvctrdbl_TrSrfMhTrLnrLgnPtNd () {
  return scinew MapFieldDataFromSourceToDestinationAlgoT<GenericField<LatVolMesh<HexTrilinearLgn<Point> > ,HexTrilinearLgn<double> ,FData3d<double,LatVolMesh<HexTrilinearLgn<Point> > > > , LatVolMesh<HexTrilinearLgn<Point> > ::Node, GenericField<TriSurfMesh<TriLinearLgn<Point> > , TriLinearLgn<double >, vector<double > >, TriSurfMesh<TriLinearLgn<Point> > ::Node>;
}
}
