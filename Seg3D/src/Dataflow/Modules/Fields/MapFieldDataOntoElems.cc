/*
 *  MapFieldDataOntoElems.cc:
 *
 *  Written by:
 *   jeroen
 *   TODAY'S DATE HERE
 *
 */

#include <Dataflow/Network/Module.h>


namespace SCIRun {

using namespace SCIRun;

class MapFieldDataOntoElems : public Module {
public:
  MapFieldDataOntoElems(GuiContext*);
  virtual ~MapFieldDataOntoElems() {}

  virtual void execute();
};


DECLARE_MAKER(MapFieldDataOntoElems)

MapFieldDataOntoElems::MapFieldDataOntoElems(GuiContext* ctx) :
  Module("MapFieldDataOntoElems", ctx, Source, "Fields", "SCIRun")
{
}

void
MapFieldDataOntoElems::execute(){
}

} // End namespace SCIRun


