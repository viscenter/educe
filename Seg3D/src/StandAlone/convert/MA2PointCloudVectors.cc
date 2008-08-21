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



// This program will read in a .pts (specifying the x/y/z coords of each 
// point, one per line, entries separated by white space, file can have 
// an optional one line header specifying number of points... and if it
// doesn't, you have to use the -noPtsCount command-line argument).  
// And the SCIRun output file is written in ASCII, unless you specify 
// -binOutput.

#include <Core/Geometry/Vector.h>
#include <Core/Basis/Constant.h>
#include <Core/Datatypes/PointCloudMesh.h>
#include <Core/Datatypes/GenericField.h>
#include <Core/Persistent/Pstreams.h>

#include <StandAlone/convert/FileUtils.h>
#include <Core/Init/init.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>

using std::cerr;
using std::ifstream;
using std::endl;

using namespace SCIRun;

typedef PointCloudMesh<ConstantBasis<Point> > PCMesh;
typedef GenericField<PCMesh, ConstantBasis<Vector>, vector<Vector> > PCField;
int
main(int argc, char **argv) {

  if (argc != 3) 
  {
    cerr << "MA2PointCloudVectors <input file (_ma.ptcl)> <output name>" 
         << endl;
    return 2;
  }
  SCIRunInit();

  PCMesh *pcm = new PCMesh();

  //! first line is number of points.
  ifstream in(argv[1]);
  if (! in) {
    cerr << "could not open " << argv[1] << " exiting." 
         << endl;
    return 2;    
  }

  int npts;
  in >> npts;
  
  vector<Vector> vecs;
  for (int i = 0; i < npts; i++) {
    double x, y, z, vx, vy, vz, s;
    in >> x >> y >> z >> vx >> vy >> vz >> s;
    pcm->add_point(Point(x,y,z));
    vecs.push_back(Vector(vx, vy, vz));
  }

  //! build field after mesh is done so that its data storage 
  //! gets sized correctly.
  PCField *pc = new PCField(pcm);

  for (int i = 0; i < npts; i++) {
    pc->set_value(vecs[i], (PCMesh::Node::index_type)i);
  }

  FieldHandle pcH(pc);
  
  BinaryPiostream out_stream(argv[2], Piostream::Write);
  Pio(out_stream, pcH);

  return 0;  
}    
