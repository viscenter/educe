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
//    File   : SliceRenderer.cc
//    Author : Milan Ikits
//    Date   : Wed Jul  7 23:37:16 2004


#include <Core/Volume/SliceRenderer.h>
#include <Core/Geometry/BBox.h>
#include <Core/Geom/DrawInfoOpenGL.h>

#include <sgi_stl_warnings_off.h>
#include   <iostream>
#include   <string>
#include <sgi_stl_warnings_on.h>

using std::cerr;
using std::endl;
using std::string;

namespace SCIRun {

#ifndef GL_TEXTURE_3D
#  define GL_TEXTURE_3D 0x806F
#endif

#ifdef _WIN32
#  undef min
#  undef max
#endif

SliceRenderer::SliceRenderer(SLIVR::Texture             *tex,
                             SLIVR::ColorMap            *cmap1, 
			     vector<SLIVR::ColorMap2*>  &cmap2,
                             int                         tex_mem) : 
  SLIVR::SliceRenderer(tex, cmap1, cmap2, tex_mem),
  mutex_("SCIRun::SliceRenderer mutex")
{
}

SliceRenderer::SliceRenderer(const SliceRenderer& copy) : 
  SLIVR::SliceRenderer(copy),
  mutex_("SCIRun::SliceRenderer mutex")
{
}

SliceRenderer::~SliceRenderer()
{}

GeomObj*
SliceRenderer::clone()
{
  return new SliceRenderer(*this);
}

void 
SliceRenderer::get_bounds(BBox& bb) 
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
SliceRenderer::draw(DrawInfoOpenGL* di, Material* mat, double)
{
  if (!tex_) return;
  if(!pre_draw(di, mat, lighting_)) return;
  mutex_.lock();
  if(di->get_drawtype() == DrawInfoOpenGL::WireFrame) {
    draw_wireframe();
  } else {
    draw_slice();
  }
  mutex_.unlock();
}

} // namespace SCIRun
