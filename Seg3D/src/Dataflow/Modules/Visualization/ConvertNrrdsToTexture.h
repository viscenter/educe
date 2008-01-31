#ifndef CONVERTNRRDSTOTEXUTRE_H
#define CONVERTNRRDSTOTEXUTRE_H

#include <Dataflow/GuiInterface/GuiVar.h>
#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/NrrdPort.h>
#include <Dataflow/Network/Ports/TexturePort.h>

#include <Dataflow/Modules/Visualization/share.h>

namespace SCIRun {

class SCISHARE ConvertNrrdsToTexture : public Module
{
public:
  ConvertNrrdsToTexture(GuiContext* ctx, const std::string& name = "ConvertNrrdsToTexture",
                        SchedClass sc = Source, const string& cat = "Visualization", 
                        const string& pack = "SCIRun");
  virtual void execute();

protected:
  TextureHandle tHandle_;

  GuiDouble gui_vminval_;
  GuiDouble gui_vmaxval_;
  GuiDouble gui_gminval_;
  GuiDouble gui_gmaxval_;
  GuiDouble gui_mminval_;
  GuiDouble gui_mmaxval_;

  GuiInt gui_fixed_;
  GuiInt gui_card_mem_;
  GuiInt gui_card_mem_auto_;
  GuiInt gui_uchar_;

  GuiInt gui_histogram_;    
  GuiDouble gui_gamma_;

  int card_mem_;
  int is_uchar_;
  int vnrrd_last_generation_;
  int gnrrd_last_generation_;
  int mnrrd_last_generation_;
  double vminval_, vmaxval_;
  double gminval_, gmaxval_;
  double mminval_, mmaxval_;
};

}

#endif
