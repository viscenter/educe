/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2007 Scientific Computing and Imaging Institute,
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

// A helper function to allow explicit references to symbols in a
// static build that would ordinarily be loaded through a shared
// library.

#ifndef SCIRun_StaticHelper_Loader_H__
#define SCIRun_StaticHelper_Loader_H__ 1

#include <Dataflow/Network/Module.h>
#include <StaticHelper/share.h>

#include <string>

extern "C" SCISHARE SCIRun::ModuleMaker SCIRun_getModule(const std::string& name);
extern "C" SCISHARE void* SCIRun_getPort(const std::string& name);

#endif // #ifndef SCIRun_StaticHelper_Loader_H__
