/*
 *  ShowLinePath.cc:
 *
 *  Written by:
 *   burak
 *   TODAY'S DATE HERE
 *
 */

#include <Dataflow/Network/Module.h>
//#include <Core/Malloc/Allocator.h>

#include <Dataflow/Network/Ports/GeometryPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/ColorMapPort.h>

#include <Core/Geom/GeomLine.h>
#include <Core/Datatypes/ColorMap.h>

#include <Core/Containers/StringUtil.h>
#include <vector>

namespace SCIRun {

using namespace SCIRun;

class ShowLinePath : public Module {
public:
  ShowLinePath(GuiContext*);

  GeometryOPort *outport;
  CrowdMonitor control_lock_;
  GeomLines *lines;
  GeomID lineID;

  MatrixIPort* matrix_iport;
  MatrixHandle inmatrix;

  ColorMapIPort *colormap_iport;
  ColorMapHandle incolors;

  ostringstream tempname;

  virtual ~ShowLinePath();

  virtual void execute();

  virtual void tcl_command(GuiArgs&, void*);
};


DECLARE_MAKER(ShowLinePath)

ShowLinePath::ShowLinePath(GuiContext* ctx) :
  Module("ShowLinePath", ctx, Source, "Visualization", "SCIRun"),
  control_lock_("ShowLinePath resolution lock")
{
// dataflow/modules/fields/getfielddata for better port use
  outport = (GeometryOPort *)get_oport("GeomOut");
  matrix_iport = (MatrixIPort *)get_iport("PointPath");
  colormap_iport = (ColorMapIPort *)get_iport("TransitionColor");

  lines=0;
  lineID=0;
}

ShowLinePath::~ShowLinePath(){
}

void
ShowLinePath::execute(){
  outport->delAll();
  matrix_iport->get(inmatrix);
  colormap_iport->get(incolors);

  if(inmatrix.get_rep() == 0) { error ("no input matrix"); return; }
  if(incolors.get_rep() == 0) { error ("no input colormap"); return; }

  //if(lines != 0)
  //  delete lines;

  lines = new GeomLines();

// lines->add(Point(0,0,0),incolors->lookup(0.0),Point(1,0,0),incolors->lookup(0.25));
// lines->add(Point(1,0,0),incolors->lookup(0.25),Point(1,1,0),incolors->lookup(0.5));
// lines->add(Point(1,1,0),incolors->lookup(0.5),Point(0,1,0),incolors->lookup(0.75));
// lines->add(Point(0,1,0),incolors->lookup(0.75),Point(0,0,0),incolors->lookup(1.0));

  if(inmatrix->nrows() == 3 && inmatrix->ncols() >= 2)
  {
    for(int m=0;m<inmatrix->ncols()-1;m++)
    {
      lines->add(Point(inmatrix->get(0,m), inmatrix->get(1,m), inmatrix->get(2,m)),
                   incolors->lookup(float(m)/float(inmatrix->ncols())),
                   Point(inmatrix->get(0,m+1), inmatrix->get(1,m+1), inmatrix->get(2,m+1)),
                   incolors->lookup(float(m+1)/float(inmatrix->ncols()))
                   );
    }
  }
  else if(inmatrix->ncols() == 3 && inmatrix->nrows() >= 2)
  for(int m=0;m<inmatrix->nrows()-1;m++)
  {
    lines->add(Point(inmatrix->get(m,0), inmatrix->get(m,1), inmatrix->get(m,2)),
                 incolors->lookup(float(m)/float(inmatrix->nrows())),
                 Point(inmatrix->get(m+1,0), inmatrix->get(m+1,1), inmatrix->get(m+1,2)),
                 incolors->lookup(float(m+1)/float(inmatrix->nrows()))
                 );

  }

  const char *name = "ShowLinePathLines";
  //if(lineID) outport->delObj(lineID);
  lineID = outport->addObj(lines,name,&control_lock_);
  outport->flushViews();
}

void
ShowLinePath::tcl_command(GuiArgs& args, void* userdata)
{
  Module::tcl_command(args, userdata);
}

} // End namespace SCIRun


