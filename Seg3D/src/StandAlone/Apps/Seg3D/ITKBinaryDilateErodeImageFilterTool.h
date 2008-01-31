


#ifndef SEG3D_ITKBinaryDilateErodeImageFilterTool_h
#define SEG3D_ITKBinaryDilateErodeImageFilterTool_h

#include <sci_defs/insight_defs.h>

#include <string>
#include <Core/Events/BaseTool.h>
#include <Core/Datatypes/ITKDatatype.h>
#include <StandAlone/Apps/Seg3D/VolumeFilter.h>
#include <itkCommand.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkImportImageFilter.h>

namespace SCIRun {

class Painter;
class NrrdVolume;

  
class ITKBinaryDilateErodeImageFilterTool : public virtual BaseTool, public PointerTool
{
public:
  ITKBinaryDilateErodeImageFilterTool(Painter *painter);
  propagation_state_e   process_event(event_handle_t);
private:
  void                  run_dilate_filter();
  void                  run_erode_filter();
  void					run_dilate_erode_filter();
  Painter *             painter_;
  NrrdVolumeHandle      source_volume_;

 
   typedef itk::BinaryBallStructuringElement< float, 3> StructuringElementType;
  typedef itk::BinaryDilateImageFilter
    < ITKImageLabel3D, ITKImageLabel3D, StructuringElementType > FilterTypeDilate;
  typedef itk::BinaryErodeImageFilter
    < ITKImageLabel3D, ITKImageLabel3D, StructuringElementType > FilterTypeErode;

  VolumeFilter<FilterTypeErode> erode_filter;
  VolumeFilter<FilterTypeDilate> dilate_filter;
  
  StructuringElementType structuringElement;

};


}

#endif
