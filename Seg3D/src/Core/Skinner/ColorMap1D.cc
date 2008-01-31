//
//  For more information, please see: http://software.sci.utah.edu
//
//  The MIT License
//
//  Copyright (c) 2006 Scientific Computing and Imaging Institute,
//  University of Utah.
//
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//    File   : ColorMap1D.cc
//    Author : McKay Davis
//    Date   : Thu Jul  6 13:48:14 2006


#include <Core/Skinner/Variables.h>
#include <Core/Skinner/ColorMap1D.h>
#include <Core/Events/EventManager.h>
#include <sci_gl.h>


namespace SCIRun {
namespace Skinner {


ColorMap1D::ColorMap1D(Variables *variables) :
  Parent(variables),
  nrrd_handle_(new NrrdData()),
  tex_(0)
{
  SLIVR::ColorMap *scm = ColorMap::create_pseudo_random(101);
  colormap_ = new ColorMap(*scm);
  delete scm;


  Nrrd *nrrd = nrrd_handle_->nrrd_;
  if (nrrdAlloc_va(nrrd, nrrdTypeFloat, 4, 1, 256,1, 1)) {
    char *err = biffGetDone(NRRD);
    string errstr = (err ? err : "");
    free(err);
    throw errstr;
  }

  float *data = (float *)nrrd->data;
  for (int i = 0; i < 255; ++i) {
    data[i] = float(i);
  }

  tex_ = new ColorMappedNrrdTextureObj(nrrd_handle_, colormap_);
  tex_->set_clut_minmax(0,255.0);
  tex_->apply_colormap(0,0,255,1, 0);
  REGISTER_CATCHER_TARGET(ColorMap1D::redraw);
}


ColorMap1D::~ColorMap1D()
{
}


BaseTool::propagation_state_e
ColorMap1D::redraw(event_handle_t &)
{
  const RectRegion &region = get_region();
  Point min(region.x1(), region.y1(), 0);
  Vector xdir(region.width(), 0, 0);
  Vector ydir(0,region.height(), 0);
  tex_->set_coords(min, xdir, ydir);
  tex_->draw_quad();

  return CONTINUE_E;
}


}
}
