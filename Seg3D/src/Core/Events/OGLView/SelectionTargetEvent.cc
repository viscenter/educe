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
//    File   : SelectionTargetEvent.cc
//    Author : Martin Cole
//    Date   : Fri Jul 28 11:46:33 2006

#include <Core/Events/OGLView/SelectionTargetEvent.h>
#include <string>

namespace SCIRun {

using namespace std;

Persistent *make_SelectionTargetEvent() { return new SelectionTargetEvent(); }
PersistentTypeID SelectionTargetEvent::type_id("SelectionTargetEvent",
					       "BaseEvent",
					       make_SelectionTargetEvent);


SelectionTargetEvent::SelectionTargetEvent(const string &target,
					   long int time) :
  BaseEvent(target, time),
  sel_targ_(0),
  sel_id_(-1)
{
}


SelectionTargetEvent::~SelectionTargetEvent()
{
}


void
SelectionTargetEvent::io(Piostream &stream)
{
  const int SELECTIONTARGETEVENT_VERSION = 1;
  stream.begin_class(type_id.type, SELECTIONTARGETEVENT_VERSION);
  BaseEvent::io(stream);
  //Pio(sel_targ_);
  stream.end_class();
}



} // namespace SCIRun

