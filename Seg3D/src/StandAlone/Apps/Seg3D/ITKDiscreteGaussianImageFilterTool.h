

#ifndef SEG3D_ITKDiscreteGaussianImageFilterTool_h
#define SEG3D_ITKDiscreteGaussianImageFilterTool_h

#include <sci_defs/insight_defs.h>

#include <string>
#include <StandAlone/Apps/Seg3D/NrrdVolume.h>
#include <StandAlone/Apps/Seg3D/VolumeFilter.h>
#include <Core/Events/BaseTool.h>
#include <Core/Datatypes/ITKDatatype.h>
#include <itkImageToImageFilter.h>
#include <itkCommand.h>
#include <itkDiscreteGaussianImageFilter.h>

namespace SCIRun {

class Painter;

class ITKDiscreteGaussianImageFilterTool : public BaseTool {
public:
  ITKDiscreteGaussianImageFilterTool(Painter *painter);
  propagation_state_e           process_event(event_handle_t);
private:
  void                          run_filter();
  Painter *                     painter_;
  NrrdVolumeHandle              source_volume_;
  Skinner::Var<double>          Variance_;
  Skinner::Var<double>          Max_Error_;
  
  typedef itk::DiscreteGaussianImageFilter
  < ITKImageFloat3D, ITKImageFloat3D > FilterType;

  VolumeFilter<FilterType>      filter_;
};

}

#endif
