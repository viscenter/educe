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
//    File   : VolumeRenderer.h
//    Author : Milan Ikits
//    Date   : Sat Jul 10 11:26:26 2004

#ifndef VolumeRenderer_h
#define VolumeRenderer_h

#include <slivr/Texture.h>
#include <slivr/VolumeRenderer.h>
#include <Core/Geom/GeomObj.h>
#include <Core/Thread/Mutex.h>
#include <Core/Volume/share.h>

namespace SCIRun {
using std::vector;

class VolShaderFactory;

class SCISHARE VolumeRenderer : public SLIVR::VolumeRenderer, public GeomObj
{
public:
  VolumeRenderer(SLIVR::Texture *tex, 
		 SLIVR::ColorMap           *cmap1, 
		 vector<SLIVR::ColorMap2*>  &cmap2,
		 vector<SLIVR::Plane*> &planes,
                 int             tex_mem);

  VolumeRenderer(const VolumeRenderer&);
  virtual ~VolumeRenderer();

  void lock() { mutex_.lock(); }
  void unlock() { mutex_.unlock(); }

  virtual void draw(DrawInfoOpenGL*, Material*, double time);
  virtual void get_bounds(BBox& bb);

  virtual GeomObj* clone();

  virtual void io(Piostream&);
  static PersistentTypeID type_id;

private:
  Mutex mutex_;
};

} // End namespace SCIRun

#endif // VolumeRenderer_h
