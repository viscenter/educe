

#ifndef SEG3D_ITKThresholdImageFilterTool_h
#define SEG3D_ITKThresholdImageFilterTool_h

#include <sci_defs/insight_defs.h>

#include <string>
#include <StandAlone/Apps/Seg3D/NrrdVolume.h>
#include <StandAlone/Apps/Seg3D/VolumeFilter.h>
#include <Core/Events/BaseTool.h>
#include <Core/Datatypes/ITKDatatype.h>
#include <itkImageToImageFilter.h>
#include <itkCommand.h>
#include <itkBinaryThresholdImageFilter.h>

namespace SCIRun {

class Painter;

class ITKThresholdImageFilterTool : public BaseTool {
public:
  ITKThresholdImageFilterTool(Painter *painter);
  propagation_state_e           process_event(event_handle_t);

private:
  void                          run_filter();

  Painter *                     painter_;
  Skinner::Var<double>          LowerThreshold_;
  Skinner::Var<double>          UpperThreshold_;
};

}

#endif
