
%module sr_py
%{
#include <sr_py/api.h>
#include <Core/Thread/Mutex.h>
#include <Core/Persistent/Persistent.h>
#include <Core/Util/ProgressReporter.h>
#include <Core/Datatypes/Datatype.h>
#include <Core/Events/BaseEvent.h>
#include <Core/Geom/OpenGLContext.h>
#include <Core/Geom/CallbackOpenGLContext.h>
#include <iostream>
#include <Core/Algorithms/Interface/Bundle/CreateParameterBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/GetBundlesFromBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/GetColorMap2sFromBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/GetColorMapsFromBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/GetFieldsFromBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/GetMatricesFromBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/GetNrrdsFromBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/GetPathsFromBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/GetStringsFromBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/InsertBundlesIntoBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/InsertColorMap2sIntoBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/InsertColorMapsIntoBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/InsertEnvironmentIntoBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/InsertFieldsIntoBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/InsertMatricesIntoBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/InsertNrrdsIntoBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/InsertPathsIntoBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/InsertStringsIntoBundleAlg.h>
#include <Core/Algorithms/Interface/Bundle/JoinBundlesAlg.h>
#include <Core/Algorithms/Interface/Bundle/ReportBundleInfoAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/ApplyFilterToFieldDataAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/ApplyMappingMatrixAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateDistanceToFieldBoundaryAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateDistanceToFieldAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateFieldData2Alg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateFieldData3Alg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateFieldDataCompiledAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateFieldDataAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateGradientsAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateInsideWhichFieldAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateIsInsideFieldAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateLatVolGradientsAtNodesAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateNodeNormalsAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateSignedDistanceToFieldAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/CalculateVectorMagnitudesAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/ConvertFieldBasisAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/ConvertFieldDataTypeAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/ConvertIndicesToFieldDataAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/ConvertLatVolDataFromElemToNodeAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/ConvertLatVolDataFromNodeToElemAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/CreateFieldDataAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/GetFieldDataAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/MapFieldDataFromElemToNodeAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/MapFieldDataFromNodeToElemAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/MapFieldDataFromSourceToDestinationAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/MaskLatVolWithTriSurfAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/ReportMeshQualityMeasuresAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/SelectAndSetFieldData3Alg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/SelectAndSetFieldDataAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/SetFieldDataAlg.h>
#include <Core/Algorithms/Interface/ChangeFieldData/SwapFieldDataWithMatrixEntriesAlg.h>
#include <Core/Algorithms/Interface/ChangeMesh/CalculateMeshNodesAlg.h>
#include <Core/Algorithms/Interface/ChangeMesh/ConvertHexVolToTetVolAlg.h>
#include <Core/Algorithms/Interface/ChangeMesh/ConvertMeshCoordinateSystemAlg.h>
#include <Core/Algorithms/Interface/ChangeMesh/ConvertMeshToPointCloudAlg.h>
#include <Core/Algorithms/Interface/ChangeMesh/ConvertMeshToUnstructuredMeshAlg.h>
#include <Core/Algorithms/Interface/ChangeMesh/ConvertQuadSurfToTriSurfAlg.h>
#include <Core/Algorithms/Interface/ChangeMesh/ConvertMeshToIrregularMeshAlg.h>
#include <Core/Algorithms/Interface/ChangeMesh/EditMeshBoundingBoxAlg.h>
#include <Core/Algorithms/Interface/ChangeMesh/MapFieldDataToNodeCoordinateAlg.h>
#include <Core/Algorithms/Interface/ChangeMesh/ScaleFieldMeshAndDataAlg.h>
#include <Core/Algorithms/Interface/ChangeMesh/SmoothMeshAlg.h>
#include <Core/Algorithms/Interface/ChangeMesh/SwapNodeLocationsWithMatrixEntriesAlg.h>
#include <Core/Algorithms/Interface/ChangeMesh/TransformMeshWithFunctionAlg.h>
#include <Core/Algorithms/Interface/ChangeMesh/TransformMeshWithTransformAlg.h>
#include <Core/Algorithms/Interface/ChangeMesh/TransformPlanarMeshAlg.h>
#include <Core/Algorithms/Interface/Converters/ConvertMatrixToFieldAlg.h>
#include <Core/Algorithms/Interface/Converters/ConvertMatrixToStringAlg.h>
#include <Core/Algorithms/Interface/DataArrayMath/AppendDataArraysAlg.h>
#include <Core/Algorithms/Interface/DataArrayMath/CalculateDataArrayAlg.h>
#include <Core/Algorithms/Interface/DataArrayMath/CreateDataArrayFromIndicesAlg.h>
#include <Core/Algorithms/Interface/DataArrayMath/CreateDataArrayAlg.h>
#include <Core/Algorithms/Interface/DataArrayMath/CreateTensorArrayAlg.h>
#include <Core/Algorithms/Interface/DataArrayMath/CreateVectorArrayAlg.h>
#include <Core/Algorithms/Interface/DataArrayMath/DecomposeTensorArrayIntoEigenVectorsAlg.h>
#include <Core/Algorithms/Interface/DataArrayMath/ReplicateDataArrayAlg.h>
#include <Core/Algorithms/Interface/DataArrayMath/ReportDataArrayInfoAlg.h>
#include <Core/Algorithms/Interface/DataArrayMath/ReportDataArrayMeasureAlg.h>
#include <Core/Algorithms/Interface/DataArrayMath/SplitVectorArrayInXYZAlg.h>
#include <Core/Algorithms/Interface/DataIO/ReadBundleAlg.h>
#include <Core/Algorithms/Interface/DataIO/ReadColorMap2DAlg.h>
#include <Core/Algorithms/Interface/DataIO/ReadColorMapAlg.h>
#include <Core/Algorithms/Interface/DataIO/ReadFieldAlg.h>
#include <Core/Algorithms/Interface/DataIO/ReadNrrdAlg.h>
#include <Core/Algorithms/Interface/DataIO/ReadHDF5FileAlg.h>
#include <Core/Algorithms/Interface/DataIO/ReadMatrixAlg.h>
#include <Core/Algorithms/Interface/DataIO/ReadPathAlg.h>
#include <Core/Algorithms/Interface/DataIO/ReadStringAlg.h>
#include <Core/Algorithms/Interface/DataIO/StreamMatrixFromDiskAlg.h>
#include <Core/Algorithms/Interface/DataIO/WriteBundleAlg.h>
#include <Core/Algorithms/Interface/DataIO/WriteColorMap2DAlg.h>
#include <Core/Algorithms/Interface/DataIO/WriteColorMapAlg.h>
#include <Core/Algorithms/Interface/DataIO/WriteFieldAlg.h>
#include <Core/Algorithms/Interface/DataIO/WriteMatrixAlg.h>
#include <Core/Algorithms/Interface/DataIO/WritePathAlg.h>
#include <Core/Algorithms/Interface/DataIO/WriteStringAlg.h>
#include <Core/Algorithms/Interface/Math/AppendMatrixAlg.h>
#include <Core/Algorithms/Interface/Math/BuildNoiseColumnMatrixAlg.h>
#include <Core/Algorithms/Interface/Math/ChooseMatrixAlg.h>
#include <Core/Algorithms/Interface/Math/CollectMatricesAlg.h>
#include <Core/Algorithms/Interface/Math/ConvertMappingMatrixToMaskVectorAlg.h>
#include <Core/Algorithms/Interface/Math/ConvertMaskVectorToMappingMatrixAlg.h>
#include <Core/Algorithms/Interface/Math/ConvertMatrixTypeAlg.h>
#include <Core/Algorithms/Interface/Math/CreateGeometricTransformAlg.h>
#include <Core/Algorithms/Interface/Math/CreateMatrixAlg.h>
#include <Core/Algorithms/Interface/Math/EvaluateLinAlgBinaryAlg.h>
#include <Core/Algorithms/Interface/Math/EvaluateLinAlgGeneralAlg.h>
#include <Core/Algorithms/Interface/Math/EvaluateLinAlgUnaryAlg.h>
#include <Core/Algorithms/Interface/Math/GetColumnOrRowFromMatrixAlg.h>
#include <Core/Algorithms/Interface/Math/GetSubmatrixAlg.h>
#include <Core/Algorithms/Interface/Math/RemoveZeroRowsAndColumnsAlg.h>
#include <Core/Algorithms/Interface/Math/RemoveZerosFromMatrixAlg.h>
#include <Core/Algorithms/Interface/Math/ReorderMatrixByCuthillMcKeeAlg.h>
#include <Core/Algorithms/Interface/Math/ReorderMatrixByReverseCuthillMcKeeAlg.h>
#include <Core/Algorithms/Interface/Math/ReportColumnMatrixMisfitAlg.h>
#include <Core/Algorithms/Interface/Math/ReportMatrixColumnMeasureAlg.h>
#include <Core/Algorithms/Interface/Math/ReportMatrixInfoAlg.h>
#include <Core/Algorithms/Interface/Math/ReportMatrixRowMeasureAlg.h>
#include <Core/Algorithms/Interface/Math/ResizeMatrixAlg.h>
#include <Core/Algorithms/Interface/Math/SolveLinearSystemAlg.h>
#include <Core/Algorithms/Interface/Math/SolveMinNormLeastSqSystemAlg.h>
#include <Core/Algorithms/Interface/Math/SortMatrixAlg.h>
#include <Core/Algorithms/Interface/MiscField/BuildMappingMatrixAlg.h>
#include <Core/Algorithms/Interface/MiscField/BuildMatrixOfSurfaceNormalsAlg.h>
#include <Core/Algorithms/Interface/MiscField/BuildPointCloudToLatVolMappingMatrixAlg.h>
#include <Core/Algorithms/Interface/MiscField/ChooseFieldAlg.h>
#include <Core/Algorithms/Interface/MiscField/CollectPointCloudsAlg.h>
#include <Core/Algorithms/Interface/MiscField/CoregisterPointCloudsAlg.h>
#include <Core/Algorithms/Interface/MiscField/ManageFieldSeriesAlg.h>
#include <Core/Algorithms/Interface/MiscField/ReportFieldGeometryMeasuresAlg.h>
#include <Core/Algorithms/Interface/MiscField/ReportFieldInfoAlg.h>
#include <Core/Algorithms/Interface/MiscField/ReportScalarFieldStatsAlg.h>
#include <Core/Algorithms/Interface/MiscField/ReportSearchGridInfoAlg.h>
#include <Core/Algorithms/Interface/MiscField/SelectFieldROIWithBoxWidgetAlg.h>
#include <Core/Algorithms/Interface/MiscField/SetFieldOrMeshStringPropertyAlg.h>
#include <Core/Algorithms/Interface/MiscField/SetFieldPropertyAlg.h>
#include <Core/Algorithms/Interface/NewField/ClipFieldByFunctionAlg.h>
#include <Core/Algorithms/Interface/NewField/ClipFieldToFieldOrWidgetAlg.h>
#include <Core/Algorithms/Interface/NewField/ClipFieldWithSeedAlg.h>
#include <Core/Algorithms/Interface/NewField/ClipLatVolByIndicesOrWidgetAlg.h>
#include <Core/Algorithms/Interface/NewField/ClipVolumeByIsovalueAlg.h>
#include <Core/Algorithms/Interface/NewField/CollectFieldsAlg.h>
#include <Core/Algorithms/Interface/NewField/ConvertMatricesToMeshAlg.h>
#include <Core/Algorithms/Interface/NewField/CreateImageAlg.h>
#include <Core/Algorithms/Interface/NewField/CreateLatVolAlg.h>
#include <Core/Algorithms/Interface/NewField/CreateStructHexAlg.h>
#include <Core/Algorithms/Interface/NewField/DecimateTriSurfAlg.h>
#include <Core/Algorithms/Interface/NewField/ExtractIsosurfaceByFunctionAlg.h>
#include <Core/Algorithms/Interface/NewField/FairMeshAlg.h>
#include <Core/Algorithms/Interface/NewField/GeneratePointSamplesFromFieldOrWidgetAlg.h>
#include <Core/Algorithms/Interface/NewField/GeneratePointSamplesFromFieldAlg.h>
#include <Core/Algorithms/Interface/NewField/GenerateSinglePointProbeFromFieldAlg.h>
#include <Core/Algorithms/Interface/NewField/GetAllSegmentationBoundariesAlg.h>
#include <Core/Algorithms/Interface/NewField/GetCentroidsFromMeshAlg.h>
#include <Core/Algorithms/Interface/NewField/GetDomainBoundaryAlg.h>
#include <Core/Algorithms/Interface/NewField/GetFieldBoundaryAlg.h>
#include <Core/Algorithms/Interface/NewField/GetSliceFromStructuredFieldByIndicesAlg.h>
#include <Core/Algorithms/Interface/NewField/InsertHexVolSheetAlongSurfaceAlg.h>
#include <Core/Algorithms/Interface/NewField/InterfaceWithCamalAlg.h>
#include <Core/Algorithms/Interface/NewField/InterfaceWithCubitAlg.h>
#include <Core/Algorithms/Interface/NewField/InterfaceWithTetGenAlg.h>
#include <Core/Algorithms/Interface/NewField/JoinFieldsAlg.h>
#include <Core/Algorithms/Interface/NewField/MergeFieldsAlg.h>
#include <Core/Algorithms/Interface/NewField/MergeTriSurfsAlg.h>
#include <Core/Algorithms/Interface/NewField/RefineMeshByIsovalue2Alg.h>
#include <Core/Algorithms/Interface/NewField/RefineMeshByIsovalueAlg.h>
#include <Core/Algorithms/Interface/NewField/RefineMeshAlg.h>
#include <Core/Algorithms/Interface/NewField/RemoveHexVolSheetAlg.h>
#include <Core/Algorithms/Interface/NewField/SplitNodesByDomainAlg.h>
#include <Core/Algorithms/Interface/NewField/SubsampleStructuredFieldByIndicesAlg.h>
#include <Core/Algorithms/Interface/String/CreateStringAlg.h>
#include <Core/Algorithms/Interface/String/GetFileNameAlg.h>
#include <Core/Algorithms/Interface/String/GetNetworkFileNameAlg.h>
#include <Core/Algorithms/Interface/String/JoinStringsAlg.h>
#include <Core/Algorithms/Interface/String/PrintMatrixIntoStringAlg.h>
#include <Core/Algorithms/Interface/String/PrintStringIntoStringAlg.h>
#include <Core/Algorithms/Interface/String/ReportStringInfoAlg.h>
#include <Core/Algorithms/Interface/String/SplitFileNameAlg.h>
#include <Core/Algorithms/Interface/Time/TimeControlsAlg.h>
#include <Core/Algorithms/Interface/Visualization/ChooseColorMapAlg.h>
#include <Core/Algorithms/Interface/Visualization/ColorMap2DSemanticsAlg.h>
#include <Core/Algorithms/Interface/Visualization/ConvertFieldsToTextureAlg.h>
#include <Core/Algorithms/Interface/Visualization/ConvertNrrdsToTextureAlg.h>
#include <Core/Algorithms/Interface/Visualization/CreateAndEditColorMap2DAlg.h>
#include <Core/Algorithms/Interface/Visualization/CreateAndEditColorMapAlg.h>
#include <Core/Algorithms/Interface/Visualization/CreateLightForViewerAlg.h>
#include <Core/Algorithms/Interface/Visualization/CreateStandardColorMapsAlg.h>
#include <Core/Algorithms/Interface/Visualization/CreateViewerAxesAlg.h>
#include <Core/Algorithms/Interface/Visualization/CreateViewerClockIconAlg.h>
#include <Core/Algorithms/Interface/Visualization/ExtractIsosurfaceAlg.h>
#include <Core/Algorithms/Interface/Visualization/GenerateStreamLinesAlg.h>
#include <Core/Algorithms/Interface/Visualization/GenerateStreamLinesWithPlacementHeuristicAlg.h>
#include <Core/Algorithms/Interface/Visualization/RescaleColorMapAlg.h>
#include <Core/Algorithms/Interface/Visualization/ShowColorMapAlg.h>
#include <Core/Algorithms/Interface/Visualization/ShowFieldGlyphsAlg.h>
#include <Core/Algorithms/Interface/Visualization/ShowFieldAlg.h>
#include <Core/Algorithms/Interface/Visualization/ShowMatrixAlg.h>
#include <Core/Algorithms/Interface/Visualization/ShowMeshBoundingBoxAlg.h>
#include <Core/Algorithms/Interface/Visualization/ShowStringAlg.h>
#include <Core/Algorithms/Interface/Visualization/ShowTextureSlicesAlg.h>
#include <Core/Algorithms/Interface/Visualization/ShowTextureVolumeAlg.h>



static int PythonCallBack(void *clientdata)
{
   PyObject *func, *arglist;
   PyObject *result;
   int    ires = 0;
   
   func = (PyObject *) clientdata;               // Get Python function
   if (! func) {
     std::cerr << "NULL python function pointer" << std::endl;
     return 0;
   }	
   PyGILState_STATE state = PyGILState_Ensure();	


   arglist = Py_BuildValue("()");                // Build argument list
   result = PyEval_CallObject(func, arglist);     // Call Python

   Py_DECREF(arglist);                           // Trash arglist
   if (result) {                                 // If no errors, return int
     ires = PyInt_AsLong(result);
     Py_DECREF(result);
   }

   PyGILState_Release(state);
   return ires;
}


%}
%include std_vector.i
%include std_string.i
%include std_map.i
%include std_deque.i

%include <Core/Geom/OpenGLContext.h>
%include <Core/Geom/CallbackOpenGLContext.h>
%include <Core/Thread/Mutex.h>
%include <Core/Util/ProgressReporter.h>
%include <Core/Persistent/Persistent.h>
%include <Core/Datatypes/Datatype.h>
%include <Core/Events/BaseEvent.h>

class size_t;

namespace std {
   %template(vector_string) vector<string>;
   %template(vector_double) vector<double>;
   %template(vector_int) vector<int>;	
   %template(vector_size_t) vector<size_t>;
};

// This tells SWIG to treat char ** as a special case
%typemap(in) char ** {
  /* Check if is a list */
  if (PyList_Check($input)) {
    int size = PyList_Size($input);
    int i = 0;
    $1 = (char **) malloc((size+1)*sizeof(char *));
    for (i = 0; i < size; i++) {
      PyObject *o = PyList_GetItem($input,i);
      if (PyString_Check(o))
	$1[i] = PyString_AsString(PyList_GetItem($input,i));
      else {
	PyErr_SetString(PyExc_TypeError,"list must contain strings");
	free($1);
	return NULL;
      }
    }
    $1[i] = 0;
  } else {
    PyErr_SetString(PyExc_TypeError,"not a list");
    return NULL;
  }
}

// This cleans up the char ** array we malloc'd before the function call
%typemap(freearg) char ** {
  free((char *) $1);
}

%exception {
  Py_BEGIN_ALLOW_THREADS
  $function
  Py_END_ALLOW_THREADS
}

// Grab a Python function object as a Python object.
%typemap(in) PyObject *pyfunc {
  if (!PyCallable_Check($input)) {
      PyErr_SetString(PyExc_TypeError, "Need a callable object!");
      return NULL;
  }
  $1 = $input;
}


// Attach a new method to our plot widget for adding Python functions
%extend SCIRun::CallbackOpenGLContext {
   // Set a Python function object as a callback function
   // Note : PyObject *pyfunc is remapped with a typempap
   void set_pymake_current_func(PyObject *pyfunc) {
     self->set_make_current_func(PythonCallBack, (void *) pyfunc);
     Py_INCREF(pyfunc);
   }
   void set_pyrelease_func(PyObject *pyfunc) {
     self->set_release_func(PythonCallBack, (void *) pyfunc);
     Py_INCREF(pyfunc);
   }
   void set_pyswap_func(PyObject *pyfunc) {
     self->set_swap_func(PythonCallBack, (void *) pyfunc);
     Py_INCREF(pyfunc);
   }
   void set_pywidth_func(PyObject *pyfunc) {
     self->set_width_func(PythonCallBack, (void *) pyfunc);
     Py_INCREF(pyfunc);
   }
   void set_pyheight_func(PyObject *pyfunc) {
     self->set_height_func(PythonCallBack, (void *) pyfunc);
     Py_INCREF(pyfunc);
   }
}


%include <sr_py/api.h>
%include <Core/Algorithms/Interface/Bundle/CreateParameterBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/GetBundlesFromBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/GetColorMap2sFromBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/GetColorMapsFromBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/GetFieldsFromBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/GetMatricesFromBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/GetNrrdsFromBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/GetPathsFromBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/GetStringsFromBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/InsertBundlesIntoBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/InsertColorMap2sIntoBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/InsertColorMapsIntoBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/InsertEnvironmentIntoBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/InsertFieldsIntoBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/InsertMatricesIntoBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/InsertNrrdsIntoBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/InsertPathsIntoBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/InsertStringsIntoBundleAlg.h>
%include <Core/Algorithms/Interface/Bundle/JoinBundlesAlg.h>
%include <Core/Algorithms/Interface/Bundle/ReportBundleInfoAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/ApplyFilterToFieldDataAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/ApplyMappingMatrixAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/CalculateDistanceToFieldBoundaryAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/CalculateDistanceToFieldAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/CalculateFieldData2Alg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/CalculateFieldData3Alg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/CalculateFieldDataCompiledAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/CalculateFieldDataAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/CalculateGradientsAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/CalculateInsideWhichFieldAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/CalculateIsInsideFieldAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/CalculateLatVolGradientsAtNodesAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/CalculateNodeNormalsAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/CalculateSignedDistanceToFieldAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/CalculateVectorMagnitudesAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/ConvertFieldBasisAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/ConvertFieldDataTypeAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/ConvertIndicesToFieldDataAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/ConvertLatVolDataFromElemToNodeAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/ConvertLatVolDataFromNodeToElemAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/CreateFieldDataAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/GetFieldDataAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/MapFieldDataFromElemToNodeAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/MapFieldDataFromNodeToElemAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/MapFieldDataFromSourceToDestinationAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/MaskLatVolWithTriSurfAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/ReportMeshQualityMeasuresAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/SelectAndSetFieldData3Alg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/SelectAndSetFieldDataAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/SetFieldDataAlg.h>
%include <Core/Algorithms/Interface/ChangeFieldData/SwapFieldDataWithMatrixEntriesAlg.h>
%include <Core/Algorithms/Interface/ChangeMesh/CalculateMeshNodesAlg.h>
%include <Core/Algorithms/Interface/ChangeMesh/ConvertHexVolToTetVolAlg.h>
%include <Core/Algorithms/Interface/ChangeMesh/ConvertMeshCoordinateSystemAlg.h>
%include <Core/Algorithms/Interface/ChangeMesh/ConvertMeshToPointCloudAlg.h>
%include <Core/Algorithms/Interface/ChangeMesh/ConvertMeshToUnstructuredMeshAlg.h>
%include <Core/Algorithms/Interface/ChangeMesh/ConvertQuadSurfToTriSurfAlg.h>
%include <Core/Algorithms/Interface/ChangeMesh/ConvertMeshToIrregularMeshAlg.h>
%include <Core/Algorithms/Interface/ChangeMesh/EditMeshBoundingBoxAlg.h>
%include <Core/Algorithms/Interface/ChangeMesh/MapFieldDataToNodeCoordinateAlg.h>
%include <Core/Algorithms/Interface/ChangeMesh/ScaleFieldMeshAndDataAlg.h>
%include <Core/Algorithms/Interface/ChangeMesh/SmoothMeshAlg.h>
%include <Core/Algorithms/Interface/ChangeMesh/SwapNodeLocationsWithMatrixEntriesAlg.h>
%include <Core/Algorithms/Interface/ChangeMesh/TransformMeshWithFunctionAlg.h>
%include <Core/Algorithms/Interface/ChangeMesh/TransformMeshWithTransformAlg.h>
%include <Core/Algorithms/Interface/ChangeMesh/TransformPlanarMeshAlg.h>
%include <Core/Algorithms/Interface/Converters/ConvertMatrixToFieldAlg.h>
%include <Core/Algorithms/Interface/Converters/ConvertMatrixToStringAlg.h>
%include <Core/Algorithms/Interface/DataArrayMath/AppendDataArraysAlg.h>
%include <Core/Algorithms/Interface/DataArrayMath/CalculateDataArrayAlg.h>
%include <Core/Algorithms/Interface/DataArrayMath/CreateDataArrayFromIndicesAlg.h>
%include <Core/Algorithms/Interface/DataArrayMath/CreateDataArrayAlg.h>
%include <Core/Algorithms/Interface/DataArrayMath/CreateTensorArrayAlg.h>
%include <Core/Algorithms/Interface/DataArrayMath/CreateVectorArrayAlg.h>
%include <Core/Algorithms/Interface/DataArrayMath/DecomposeTensorArrayIntoEigenVectorsAlg.h>
%include <Core/Algorithms/Interface/DataArrayMath/ReplicateDataArrayAlg.h>
%include <Core/Algorithms/Interface/DataArrayMath/ReportDataArrayInfoAlg.h>
%include <Core/Algorithms/Interface/DataArrayMath/ReportDataArrayMeasureAlg.h>
%include <Core/Algorithms/Interface/DataArrayMath/SplitVectorArrayInXYZAlg.h>
%include <Core/Algorithms/Interface/DataIO/ReadBundleAlg.h>
%include <Core/Algorithms/Interface/DataIO/ReadColorMap2DAlg.h>
%include <Core/Algorithms/Interface/DataIO/ReadColorMapAlg.h>
%include <Core/Algorithms/Interface/DataIO/ReadFieldAlg.h>
%include <Core/Algorithms/Interface/DataIO/ReadNrrdAlg.h>
%include <Core/Algorithms/Interface/DataIO/ReadHDF5FileAlg.h>
%include <Core/Algorithms/Interface/DataIO/ReadMatrixAlg.h>
%include <Core/Algorithms/Interface/DataIO/ReadPathAlg.h>
%include <Core/Algorithms/Interface/DataIO/ReadStringAlg.h>
%include <Core/Algorithms/Interface/DataIO/StreamMatrixFromDiskAlg.h>
%include <Core/Algorithms/Interface/DataIO/WriteBundleAlg.h>
%include <Core/Algorithms/Interface/DataIO/WriteColorMap2DAlg.h>
%include <Core/Algorithms/Interface/DataIO/WriteColorMapAlg.h>
%include <Core/Algorithms/Interface/DataIO/WriteFieldAlg.h>
%include <Core/Algorithms/Interface/DataIO/WriteMatrixAlg.h>
%include <Core/Algorithms/Interface/DataIO/WritePathAlg.h>
%include <Core/Algorithms/Interface/DataIO/WriteStringAlg.h>
%include <Core/Algorithms/Interface/Math/AppendMatrixAlg.h>
%include <Core/Algorithms/Interface/Math/BuildNoiseColumnMatrixAlg.h>
%include <Core/Algorithms/Interface/Math/ChooseMatrixAlg.h>
%include <Core/Algorithms/Interface/Math/CollectMatricesAlg.h>
%include <Core/Algorithms/Interface/Math/ConvertMappingMatrixToMaskVectorAlg.h>
%include <Core/Algorithms/Interface/Math/ConvertMaskVectorToMappingMatrixAlg.h>
%include <Core/Algorithms/Interface/Math/ConvertMatrixTypeAlg.h>
%include <Core/Algorithms/Interface/Math/CreateGeometricTransformAlg.h>
%include <Core/Algorithms/Interface/Math/CreateMatrixAlg.h>
%include <Core/Algorithms/Interface/Math/EvaluateLinAlgBinaryAlg.h>
%include <Core/Algorithms/Interface/Math/EvaluateLinAlgGeneralAlg.h>
%include <Core/Algorithms/Interface/Math/EvaluateLinAlgUnaryAlg.h>
%include <Core/Algorithms/Interface/Math/GetColumnOrRowFromMatrixAlg.h>
%include <Core/Algorithms/Interface/Math/GetSubmatrixAlg.h>
%include <Core/Algorithms/Interface/Math/RemoveZeroRowsAndColumnsAlg.h>
%include <Core/Algorithms/Interface/Math/RemoveZerosFromMatrixAlg.h>
%include <Core/Algorithms/Interface/Math/ReorderMatrixByCuthillMcKeeAlg.h>
%include <Core/Algorithms/Interface/Math/ReorderMatrixByReverseCuthillMcKeeAlg.h>
%include <Core/Algorithms/Interface/Math/ReportColumnMatrixMisfitAlg.h>
%include <Core/Algorithms/Interface/Math/ReportMatrixColumnMeasureAlg.h>
%include <Core/Algorithms/Interface/Math/ReportMatrixInfoAlg.h>
%include <Core/Algorithms/Interface/Math/ReportMatrixRowMeasureAlg.h>
%include <Core/Algorithms/Interface/Math/ResizeMatrixAlg.h>
%include <Core/Algorithms/Interface/Math/SolveLinearSystemAlg.h>
%include <Core/Algorithms/Interface/Math/SolveMinNormLeastSqSystemAlg.h>
%include <Core/Algorithms/Interface/Math/SortMatrixAlg.h>
%include <Core/Algorithms/Interface/MiscField/BuildMappingMatrixAlg.h>
%include <Core/Algorithms/Interface/MiscField/BuildMatrixOfSurfaceNormalsAlg.h>
%include <Core/Algorithms/Interface/MiscField/BuildPointCloudToLatVolMappingMatrixAlg.h>
%include <Core/Algorithms/Interface/MiscField/ChooseFieldAlg.h>
%include <Core/Algorithms/Interface/MiscField/CollectPointCloudsAlg.h>
%include <Core/Algorithms/Interface/MiscField/CoregisterPointCloudsAlg.h>
%include <Core/Algorithms/Interface/MiscField/ManageFieldSeriesAlg.h>
%include <Core/Algorithms/Interface/MiscField/ReportFieldGeometryMeasuresAlg.h>
%include <Core/Algorithms/Interface/MiscField/ReportFieldInfoAlg.h>
%include <Core/Algorithms/Interface/MiscField/ReportScalarFieldStatsAlg.h>
%include <Core/Algorithms/Interface/MiscField/ReportSearchGridInfoAlg.h>
%include <Core/Algorithms/Interface/MiscField/SelectFieldROIWithBoxWidgetAlg.h>
%include <Core/Algorithms/Interface/MiscField/SetFieldOrMeshStringPropertyAlg.h>
%include <Core/Algorithms/Interface/MiscField/SetFieldPropertyAlg.h>
%include <Core/Algorithms/Interface/NewField/ClipFieldByFunctionAlg.h>
%include <Core/Algorithms/Interface/NewField/ClipFieldToFieldOrWidgetAlg.h>
%include <Core/Algorithms/Interface/NewField/ClipFieldWithSeedAlg.h>
%include <Core/Algorithms/Interface/NewField/ClipLatVolByIndicesOrWidgetAlg.h>
%include <Core/Algorithms/Interface/NewField/ClipVolumeByIsovalueAlg.h>
%include <Core/Algorithms/Interface/NewField/CollectFieldsAlg.h>
%include <Core/Algorithms/Interface/NewField/ConvertMatricesToMeshAlg.h>
%include <Core/Algorithms/Interface/NewField/CreateImageAlg.h>
%include <Core/Algorithms/Interface/NewField/CreateLatVolAlg.h>
%include <Core/Algorithms/Interface/NewField/CreateStructHexAlg.h>
%include <Core/Algorithms/Interface/NewField/DecimateTriSurfAlg.h>
%include <Core/Algorithms/Interface/NewField/ExtractIsosurfaceByFunctionAlg.h>
%include <Core/Algorithms/Interface/NewField/FairMeshAlg.h>
%include <Core/Algorithms/Interface/NewField/GeneratePointSamplesFromFieldOrWidgetAlg.h>
%include <Core/Algorithms/Interface/NewField/GeneratePointSamplesFromFieldAlg.h>
%include <Core/Algorithms/Interface/NewField/GenerateSinglePointProbeFromFieldAlg.h>
%include <Core/Algorithms/Interface/NewField/GetAllSegmentationBoundariesAlg.h>
%include <Core/Algorithms/Interface/NewField/GetCentroidsFromMeshAlg.h>
%include <Core/Algorithms/Interface/NewField/GetDomainBoundaryAlg.h>
%include <Core/Algorithms/Interface/NewField/GetFieldBoundaryAlg.h>
%include <Core/Algorithms/Interface/NewField/GetSliceFromStructuredFieldByIndicesAlg.h>
%include <Core/Algorithms/Interface/NewField/InsertHexVolSheetAlongSurfaceAlg.h>
%include <Core/Algorithms/Interface/NewField/InterfaceWithCamalAlg.h>
%include <Core/Algorithms/Interface/NewField/InterfaceWithCubitAlg.h>
%include <Core/Algorithms/Interface/NewField/InterfaceWithTetGenAlg.h>
%include <Core/Algorithms/Interface/NewField/JoinFieldsAlg.h>
%include <Core/Algorithms/Interface/NewField/MergeFieldsAlg.h>
%include <Core/Algorithms/Interface/NewField/MergeTriSurfsAlg.h>
%include <Core/Algorithms/Interface/NewField/RefineMeshByIsovalue2Alg.h>
%include <Core/Algorithms/Interface/NewField/RefineMeshByIsovalueAlg.h>
%include <Core/Algorithms/Interface/NewField/RefineMeshAlg.h>
%include <Core/Algorithms/Interface/NewField/RemoveHexVolSheetAlg.h>
%include <Core/Algorithms/Interface/NewField/SplitNodesByDomainAlg.h>
%include <Core/Algorithms/Interface/NewField/SubsampleStructuredFieldByIndicesAlg.h>
%include <Core/Algorithms/Interface/String/CreateStringAlg.h>
%include <Core/Algorithms/Interface/String/GetFileNameAlg.h>
%include <Core/Algorithms/Interface/String/GetNetworkFileNameAlg.h>
%include <Core/Algorithms/Interface/String/JoinStringsAlg.h>
%include <Core/Algorithms/Interface/String/PrintMatrixIntoStringAlg.h>
%include <Core/Algorithms/Interface/String/PrintStringIntoStringAlg.h>
%include <Core/Algorithms/Interface/String/ReportStringInfoAlg.h>
%include <Core/Algorithms/Interface/String/SplitFileNameAlg.h>
%include <Core/Algorithms/Interface/Time/TimeControlsAlg.h>
%include <Core/Algorithms/Interface/Visualization/ChooseColorMapAlg.h>
%include <Core/Algorithms/Interface/Visualization/ColorMap2DSemanticsAlg.h>
%include <Core/Algorithms/Interface/Visualization/ConvertFieldsToTextureAlg.h>
%include <Core/Algorithms/Interface/Visualization/ConvertNrrdsToTextureAlg.h>
%include <Core/Algorithms/Interface/Visualization/CreateAndEditColorMap2DAlg.h>
%include <Core/Algorithms/Interface/Visualization/CreateAndEditColorMapAlg.h>
%include <Core/Algorithms/Interface/Visualization/CreateLightForViewerAlg.h>
%include <Core/Algorithms/Interface/Visualization/CreateStandardColorMapsAlg.h>
%include <Core/Algorithms/Interface/Visualization/CreateViewerAxesAlg.h>
%include <Core/Algorithms/Interface/Visualization/CreateViewerClockIconAlg.h>
%include <Core/Algorithms/Interface/Visualization/ExtractIsosurfaceAlg.h>
%include <Core/Algorithms/Interface/Visualization/GenerateStreamLinesAlg.h>
%include <Core/Algorithms/Interface/Visualization/GenerateStreamLinesWithPlacementHeuristicAlg.h>
%include <Core/Algorithms/Interface/Visualization/RescaleColorMapAlg.h>
%include <Core/Algorithms/Interface/Visualization/ShowColorMapAlg.h>
%include <Core/Algorithms/Interface/Visualization/ShowFieldGlyphsAlg.h>
%include <Core/Algorithms/Interface/Visualization/ShowFieldAlg.h>
%include <Core/Algorithms/Interface/Visualization/ShowMatrixAlg.h>
%include <Core/Algorithms/Interface/Visualization/ShowMeshBoundingBoxAlg.h>
%include <Core/Algorithms/Interface/Visualization/ShowStringAlg.h>
%include <Core/Algorithms/Interface/Visualization/ShowTextureSlicesAlg.h>
%include <Core/Algorithms/Interface/Visualization/ShowTextureVolumeAlg.h>

