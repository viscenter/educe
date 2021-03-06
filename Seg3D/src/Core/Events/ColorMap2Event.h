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
//    File   : ColorMap2Event.h
//    Author : McKay Davis
//    Date   : Wed Aug  2 11:06:02 2006

#ifndef COLORMAP2_EVENT_H
#define COLORMAP2_EVENT_H

#include <Core/Events/BaseEvent.h>
#include <Core/Volume/ColorMap2.h>

namespace SCIRun {
  class ColorMap2Event : public BaseEvent {
  public:
    typedef vector<ColorMap2Handle>    ColorMap2Handle_t;
    ColorMap2Event(const ColorMap2Handle_t &data) :
      BaseEvent("", 0),
      data_(data) {}
    virtual ~ColorMap2Event() {}
    ColorMap2Handle_t &                get_data() { return data_; }
    ColorMap2Event *                   clone() { return new ColorMap2Event(*this); }
  private:
    ColorMap2Handle_t                  data_;
  };
}

#endif
