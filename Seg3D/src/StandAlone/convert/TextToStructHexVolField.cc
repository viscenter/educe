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
 *  TextToStructHexVolField.cc
 *
 *  Written by:
 *   David Weinstein
 *   Department of Computer Science
 *   University of Utah
 *   April 2003
 *
 *  Copyright (C) 2003 SCI Group
 */

// This program will read in a .pts (specifying the x/y/z coords of each 
// point, one per line, entries separated by white space.  If file doesn't
// have a one line header specifying ni nj nk (white-space separated), the
// user must specify a -noHeader command line argument, followed by ni, nj,
// and nk.
// The SCIRun output file is written in ASCII, unless you specify 
// -binOutput.


#include <Core/Basis/HexTrilinearLgn.h>
#include <Core/Basis/NoData.h>
#include <Core/Datatypes/StructHexVolMesh.h>
#include <Core/Containers/FData.h>
#include <Core/Datatypes/GenericField.h>
#include <Core/Persistent/Pstreams.h>
#include <Core/Containers/HashTable.h>
#include <StandAlone/convert/FileUtils.h>
#include <Core/Init/init.h>

#include <iostream>
#include <fstream>
#include <stdlib.h>

using std::cerr;
using std::ifstream;
using std::endl;

using namespace SCIRun;

bool header;
bool binOutput;
bool debugOn;
int ni;
int nj;
int nk;

void setDefaults() {
  header=true;
  binOutput=false;
  debugOn=false;
  ni=0;
  nj=0;
  nk=0;
}

int parseArgs(int argc, char *argv[]) {
  int currArg = 4;
  while (currArg < argc) {
    if (!strcmp(argv[currArg],"-noHeader")) {
      header=false;
      currArg++;
      ni=atoi(argv[currArg]);
      currArg++;
      nj=atoi(argv[currArg]);
      currArg++;
      nk=atoi(argv[currArg]);
      currArg++;
    } else if (!strcmp(argv[currArg], "-binOutput")) {
      binOutput=true;
      currArg++;
    } else if (!strcmp(argv[currArg], "-debug")) {
      debugOn=true;
      currArg++;
    } else {
      cerr << "Error - unrecognized argument: "<<argv[currArg]<<"\n";
      return 0;
    }
  }
  return 1;
}

void printUsageInfo(char *progName) {
  cerr << "\n Usage: "<<progName<<" pts StructHexVolMesh [-noHeader ni nj nk] [-binOutput] [-debug]\n\n";
  cerr << "\t This program will read in a .pts (specifying the x/y/z \n";
  cerr << "\t coords of each point, one per line, entries separated by \n";
  cerr << "\t white space.  If file doesn't have a one line header \n";
  cerr << "\t specifying ni nj nk (white-space separated), the user must \n";
  cerr << "\t specify a -noHeader command line argument, followed by ni, \n";
  cerr << "\t nj, and nk.  The SCIRun output file is written in ASCII, \n";
  cerr << "\t unless you specify -binOutput.\n\n";
}

int
main(int argc, char **argv) {
  if (argc < 3 || argc > 9) {
    printUsageInfo(argv[0]);
    return 2;
  }
  SCIRunInit();
  setDefaults();

  char *ptsName = argv[1];
  char *fieldName = argv[2];
  if (!parseArgs(argc, argv)) {
    printUsageInfo(argv[0]);
    return 2;
  }

  ifstream ptsstream(ptsName);
  if (ptsstream.fail()) {
    cerr << "Error -- Could not open file " << ptsName << "\n";
    return 2;
  }
  if (header) ptsstream >> ni >> nj >> nk;
  cerr << "number of points = ("<<ni<<" "<<nj<<" "<<nk<<")\n";

  typedef StructHexVolMesh<HexTrilinearLgn<Point> > SHVMesh;
  SHVMesh *hvm = new SHVMesh(ni, nj, nk);
  int i, j, k;
  for (i=0; i<ni; i++)
    for (j=0; j<nj; j++)
      for (k=0; k<nk; k++) {
	double x, y, z;
	ptsstream >> x >> y >> z;
	SHVMesh::Node::index_type idx(hvm, i, j, k);
	hvm->set_point(Point(x, y, z), idx);
	if (debugOn) 
	  cerr << "Added point (idx=["<<i<<" "<<j<<" "<<k<<"]) at ("<<x<<", "<<y<<", "<<z<<")\n";
      }
  cerr << "done adding points.\n";
  typedef NoDataBasis<double>                DatBasis;
  typedef GenericField<SHVMesh, DatBasis, FData3d<double, SHVMesh> > SHVField;

  SHVField *hvf = scinew SHVField(hvm);
  FieldHandle hvH(hvf);
  
  if (binOutput) {
    BinaryPiostream out_stream(fieldName, Piostream::Write);
    Pio(out_stream, hvH);
  } else {
    TextPiostream out_stream(fieldName, Piostream::Write);
    Pio(out_stream, hvH);
  }
  return 0;  
}    
