/*
 *  MapFieldDataOntoNodes.cc:
 *
 *  Written by:
 *   jeroen
 *   TODAY'S DATE HERE
 *
 */

#include <Dataflow/Network/Module.h>


namespace SCIRun {

using namespace SCIRun;

class MapFieldDataOntoNodes : public Module {
public:
  MapFieldDataOntoNodes(GuiContext*);
  virtual ~MapFieldDataOntoNodes() {}

  virtual void execute();
};


DECLARE_MAKER(MapFieldDataOntoNodes)

MapFieldDataOntoNodes::MapFieldDataOntoNodes(GuiContext* ctx) :
  Module("MapFieldDataOntoNodes", ctx, Source, "Fields", "SCIRun")
{
}

void
MapFieldDataOntoNodes::execute(){
}


} // End namespace SCIRun


