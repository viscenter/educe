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
//    File   : SliceRenderer.h
//    Author : Milan Ikits
//    Date   : Wed Jul  7 23:36:05 2004

#ifndef SliceRenderer_h
#define SliceRenderer_h

#include <Core/Geom/GeomObj.h>
#include <slivr/Texture.h>
#include <slivr/SliceRenderer.h>
#include <Core/Volume/share.h>
#include <vector>

namespace SCIRun {
using std::vector;

class SCISHARE SliceRenderer : public SLIVR::SliceRenderer, public GeomObj
{
public:
  SliceRenderer(SLIVR::Texture   *tex, 
		SLIVR::ColorMap             *cmap1, 
		vector<SLIVR::ColorMap2*>    &cmap2,
                int tex_mem);
  SliceRenderer(const SliceRenderer&);
  ~SliceRenderer();

  void lock() { mutex_.lock(); }
  void unlock() { mutex_.unlock(); }

  virtual void draw(DrawInfoOpenGL*, Material*, double time);
  virtual void get_bounds(BBox& bb);
  virtual GeomObj* clone();
private:
  Mutex mutex_;
};

} // end namespace SCIRun

#endif // SliceRenderer_h
