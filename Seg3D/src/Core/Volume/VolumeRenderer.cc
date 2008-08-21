//
//  For more information, please see: http://software.sci.utah.edu
//
//  The MIT License
//
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
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
//    File   : VolumeRenderer.cc
//    Author : Milan Ikits
//    Date   : Thu Jul  8 00:04:15 2004

#include <Core/Geom/DrawInfoOpenGL.h>
#include <Core/Geometry/BBox.h>
#include <Core/Volume/VolumeRenderer.h>
#include <Core/Math/MiscMath.h>
#include <Core/Geom/GeomResourceManager.h>


#include   <string>
#include   <iostream>


using std::cerr;
using std::endl;
using std::string;

namespace SCIRun {
using namespace SLIVR;


#ifdef _WIN32
#  undef min
#  undef max
#endif


VolumeRenderer::VolumeRenderer(Texture                    *tex,
                               SLIVR::ColorMap            *cmap1,
                               vector<SLIVR::ColorMap2*>  &cmap2,
                               vector<SLIVR::Plane*>      &planes,
                               int                         tex_mem) :
  SLIVR::VolumeRenderer(tex, cmap1, cmap2, planes,  tex_mem),
  mutex_("SCIRun::VolumeRenderer mutex")
{
  set_pending_delete_texture_callback(GeomResourceManager::delete_texture_id);
}

VolumeRenderer::VolumeRenderer(const VolumeRenderer& copy) :
  SLIVR::VolumeRenderer(copy),
  mutex_("SCIRun::VolumeRenderer mutex")
{
}


VolumeRenderer::~VolumeRenderer()
{
}


GeomObj*
VolumeRenderer::clone()
{
  return new VolumeRenderer(*this);
}

void 
VolumeRenderer::get_bounds(BBox& bb) 
{ 
  if (!tex_) return;
  double xmin;
  double ymin;
  double zmin;  
  double xmax;
  double ymax;
  double zmax;
  tex_->get_bounds(xmin, ymin, zmin, xmax, ymax, zmax); 

  bb.extend(Point(xmin, ymin, zmin));
  bb.extend(Point(xmax, ymax, zmax));
}

void
VolumeRenderer::draw(DrawInfoOpenGL* di, Material* mat, double)
{
  if (!tex_) return;
  if(!pre_draw(di, mat, shading_)) return;
  mutex_.lock();
  if(di->get_drawtype() == DrawInfoOpenGL::WireFrame )
  {
    draw_wireframe(di->view_.is_ortho());
  }
  else
  {
    draw_volume(di->mouse_action_, di->view_.is_ortho());
  }
  mutex_.unlock();
}

void 
VolumeRenderer::io(Piostream&)
{
}

} // namespace SCIRun
