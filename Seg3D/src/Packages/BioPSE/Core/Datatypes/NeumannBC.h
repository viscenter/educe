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
  The contents of this file are subject to the University of Utah Public
  License (the "License"); you may not use this file except in compliance
  with the License.
  
  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
  
  the License.
  
  The Original Source Code is SCIRun, released March 12, 2001.
  
  The Original Source Code was developed by the University of Utah.
  Portions created by UNIVERSITY are Copyright (C) 2001, 1994 
  University of Utah. All Rights Reserved.
*/

// Attrib.h - the base attribute class.
//
//  Written by:
//   Eric Kuehne, Alexei Samsonov
//   Department of Computer Science
//   University of Utah
//   April 2000, December 2000
//
//  Copyright (C) 2000 SCI Institute
//
//  General storage class for Fields.
//

#ifndef SCI_project_NeumannBC_h
#define SCI_project_NeumannBC_h 1

#include <vector>
#include <string>
#include <iostream>

#include <Core/Datatypes/Datatype.h>
#include <Core/Containers/LockingHandle.h>
#include <Core/Util/FancyAssert.h>
#include <Core/Persistent/PersistentSTL.h>
#include <Core/Geometry/Vector.h>

#include <Packages/BioPSE/Core/Datatypes/share.h>

namespace BioPSE {

using namespace SCIRun;

/////////
// Structure to hold Neumann BC related values
class SCISHARE NeumannBC {
public:  
  // GROUP: public data
  //////////
  // 
  NeumannBC(){};
  NeumannBC(Vector v, double d): dir(v), val(d){};
  //////////
  // Direction to take derivative in
  Vector dir;
  //////////
  // Value of the derivative
  double val;
};
} // end namespace BioPSE


namespace SCIRun {
using namespace std;
//////////
// PIO for NeumannBC objects
void  Pio(Piostream&, BioPSE::NeumannBC&);
ostream& operator<<(ostream&, BioPSE::NeumannBC&);
} // end namespace SCIRun

#endif
