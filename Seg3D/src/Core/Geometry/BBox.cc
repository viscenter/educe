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
 *  BBox.cc: Bounding box class
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   1994
 *
 *  Copyright (C) 1994,2004 SCI Group
 */

#include <Core/Geometry/BBox.h>
#include <Core/Persistent/Persistent.h>
#include <stdlib.h>

using namespace SCIRun;
using namespace std;

namespace SCIRun {

void Pio(Piostream & stream, BBox & box)
{
  stream.begin_cheap_delim();
    
  // Store the valid flag as an int to be backwards compatible.
  int tmp = box.valid();
  Point min = box.min();
  Point max = box.min();
  Pio(stream, tmp);
  if (tmp) {
    Pio(stream, min);
    Pio(stream, max);
  }

  if(stream.reading()) {
    box.set_valid(tmp);
    if (tmp) {
      box.extend(min);
      box.extend(max);
    }
  }
  stream.end_cheap_delim();
}

} // End namespace SCIRun
