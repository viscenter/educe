

#ifndef SEG3D_ITGradientMagnitudeImageFilterTool_h
#define SEG3D_ITKGradientMagnitudeImageFilterTool_h

#include <sci_defs/insight_defs.h>

#include <string>
#include <Core/Events/BaseTool.h>
#include <Core/Datatypes/ITKDatatype.h>
#include <StandAlone/Apps/Seg3D/VolumeFilter.h>
#include <itkImageToImageFilter.h>
#include <itkCommand.h>
#include <itkGradientMagnitudeImageFilter.h>

namespace SCIRun {

class Painter;
class NrrdVolume;

  
class ITKGradientMagnitudeImageFilterTool : public virtual BaseTool, public PointerTool
{
public:
  ITKGradientMagnitudeImageFilterTool(Painter *painter);
  propagation_state_e   process_event(event_handle_t);
private:
  void                  run_filter();
  Painter *             painter_;
  NrrdVolumeHandle      source_volume_;


  typedef itk::GradientMagnitudeImageFilter
    < ITKImageFloat3D, ITKImageFloat3D > FilterType;

  VolumeFilter<FilterType>      filter_;
};
  
  
}

#endif
