

#ifndef SEG3D_FloodFillCopyTool_h
#define SEG3D_FloodFillCopyTool_h

#include <sci_defs/insight_defs.h>

#include <string>
#include <Core/Events/BaseTool.h>
#include <Core/Datatypes/ITKDatatype.h>
#include <StandAlone/Apps/Seg3D/VolumeFilter.h>
#include <StandAlone/Apps/Seg3D/BrushFloodFill.h>

namespace SCIRun {

class Painter;
class NrrdVolume;

  
class FloodFillCopyTool : public ITKConnectedImageFilterTool
{
public:
  FloodFillCopyTool(Painter *painter);

private:
  void                  run_filter();
};
  
  
}

#endif
