/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2004 Scientific Computing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/



/*
 *  MessageTypes.h: enum definitions for Message Types...
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   March 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

#ifndef SCI_project_MessageTypes_h
#define SCI_project_MessageTypes_h 1

namespace SCIRun {

class MessageTypes {
    MessageTypes();
public:
    void dummy(); // Keeps g++ happy...
    enum MessageType {
	DoCallback,
	DoDBCallback,
	ExecuteModule,
	TriggerPort,
	ReSchedule,
	MultiSend,
	GoAway,
	GoAwayWarn,

	MUIDispatch,

	GeometryInit,
	GeometryDetach,
	GeometryAddObj,
	GeometryDelObj,
	GeometryAddLight,
	GeometryDelLight,
	GeometryDelAll,
	GeometryFlush,
	GeometryFlushViews,
	GeometryGetData,
	GeometryGetNViewWindows,
	GeometrySetView,

	GeometryPick,
	GeometryRelease,
	GeometryMoved,

	ViewWindowRedraw,
	ViewWindowMouse,
	ViewWindowDumpImage,
	ViewWindowDumpObjects,
	ViewWindowEditLight,
        ViewWindowUpdateClipFrame,
        ViewWindowKill,

	AttachDialbox,
	DialMoved,

	ModuleGeneric1,
	ModuleGeneric2,
	ModuleGeneric3,
	ModuleGeneric4,

        SchedulerInternalExecuteDone,
        GeometrySynchronize,
        SynchronizeModule
    };
};

} // End namespace SCIRun


#endif
