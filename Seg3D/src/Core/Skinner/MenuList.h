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
//    File   : MenuList.h
//    Author : McKay Davis
//    Date   : Sat Aug 12 12:36:19 2006

#ifndef SKINNER_MENULIST_H
#define SKINNER_MENULIST_H

#include <Core/Skinner/Parent.h>
#include <Core/Skinner/MenuManager.h>

namespace SCIRun {
namespace Skinner {


class MenuList : public Parent {
public:
  MenuList (Variables *variables);
  virtual ~MenuList();
  CatcherFunction_t                 process_event;

protected:
  friend class MenuManager;

  Var<double>                       x1_;
  Var<double>                       x2_;
  Var<double>                       y1_;
  Var<double>                       y2_;
  Var<double>                       width_;
  Var<double>                       height_;
};


}
}

#endif
