/*
 *  ShowPointPath.cc:
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

#include <Dataflow/Widgets/PointWidget.h>

#include <Core/Containers/StringUtil.h>
#include <vector>


namespace SCIRun {

using namespace SCIRun;

class ShowPointPath : public Module {
public:
  ShowPointPath(GuiContext*);

  GeometryOPort *outport;
  GeomHandle currpoint;

  MatrixIPort* matrix_iport;
  MatrixHandle inmatrix;

  CrowdMonitor control_lock_;
  vector<PointWidget*> traj;
  ostringstream tempname;

  virtual ~ShowPointPath();

  virtual void execute();

  virtual void tcl_command(GuiArgs&, void*);
};


DECLARE_MAKER(ShowPointPath)

ShowPointPath::ShowPointPath(GuiContext* ctx) :
  Module("ShowPointPath", ctx, Source, "Visualization", "SCIRun"), control_lock_("ShowPointPath resolution lock")
{
  outport = (GeometryOPort *)get_oport("GeomOut");
  matrix_iport = (MatrixIPort *)get_iport("PointPath");
}

ShowPointPath::~ShowPointPath(){
}

void
ShowPointPath::execute(){
  outport->delAll();
  matrix_iport->get(inmatrix);
  traj.clear();

  if(inmatrix.get_rep() == 0) { error ("no input matrix"); return; }

  if(inmatrix->nrows() == 3)
  {
    for(int m=0;m<inmatrix->ncols();m++)
    {
      traj.push_back(new PointWidget(this, &control_lock_, 1.0));
      traj[m]->SetPosition(Point(inmatrix->get(0,m), inmatrix->get(1,m), inmatrix->get(2,m)));

      currpoint = traj[m]->GetWidget();
      tempname.str("");
      tempname << "ShowPointPathPoint" << m;
      outport->addObj(currpoint,tempname.str().c_str(),&control_lock_);
    }
  }
  else if(inmatrix->ncols() == 3)
  for(int m=0;m<inmatrix->nrows();m++)
  {
    traj.push_back(new PointWidget(this, &control_lock_, 1.0));
    traj[m]->SetPosition(Point(inmatrix->get(m,0), inmatrix->get(m,1), inmatrix->get(m,2)));

    currpoint = traj[m]->GetWidget();
    tempname.str("");
    tempname << "ShowPointPathPoint" << m;
    outport->addObj(currpoint,tempname.str().c_str(),&control_lock_);
  }

  outport->flushViews();
}

void
ShowPointPath::tcl_command(GuiArgs& args, void* userdata)
{
  Module::tcl_command(args, userdata);
}

} // End namespace SCIRun


