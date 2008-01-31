// This is an automatically generated file, do not edit!
#include "Core/Geometry/Point.h"
#include "Core/Basis/HexTrilinearLgn.h"
#include "../src/Core/Basis/TriLinearLgn.h"
#include "../src/Core/Basis/QuadBilinearLgn.h"
#include "Core/Datatypes/LatVolMesh.h"
#include "../src/Core/Datatypes/TriSurfMesh.h"
#include "../src/Core/Datatypes/QuadSurfMesh.h"
#include "Core/Containers/FData.h"
#include "Core/Datatypes/GenericField.h"
#include "Core/Algorithms/Visualization/HexMC.h"
#include "Core/Algorithms/Visualization/MarchingCubes.h"
using namespace SCIRun;

extern "C" {
MarchingCubesAlg* maker_MrchngCbs_GFldLtVlMhHxTLLgnPtHxTLLgndblFDt3ddblLtVlMhHxTLLgnPt () {
  return scinew MarchingCubes<HexMC<GenericField<LatVolMesh<HexTrilinearLgn<Point> > ,HexTrilinearLgn<double> ,FData3d<double,LatVolMesh<HexTrilinearLgn<Point> > > > > >;
}
}
