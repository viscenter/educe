

#ifndef SEG3D_ITKOTSUImageFilterTool_h
#define SEG3D_ITKOTSUImageFilterTool_h

#include <sci_defs/insight_defs.h>

#include <string>
#include <Core/Events/BaseTool.h>
#include <Core/Datatypes/ITKDatatype.h>
#include <StandAlone/Apps/Seg3D/VolumeFilter.h>
#include <itkImageToImageFilter.h>
#include <itkCommand.h>
#include <itkOtsuThresholdImageFilter.h>

namespace SCIRun {

class Painter;
class NrrdVolume;

  
class ITKOtsuImageFilterTool : public virtual BaseTool
{
public:
  ITKOtsuImageFilterTool(Painter *painter);
  propagation_state_e   process_event(event_handle_t);

private:
  void                  run_filter();
  Painter *             painter_;
  NrrdVolumeHandle      source_volume_;
  string                prefix_;

 
  typedef itk::OtsuThresholdImageFilter
    < ITKImageFloat3D, ITKImageLabel3D > FilterType;

  VolumeFilter<FilterType>      filter_;
};
  
  
}

#endif
