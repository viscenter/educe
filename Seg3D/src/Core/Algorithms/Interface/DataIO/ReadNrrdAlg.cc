//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2007 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  License for the specific language governing rights and limitations under
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

#include <iostream>
#include <Core/Events/DataManager.h>
#include <Core/Algorithms/Interface/DataIO/ReadNrrdAlg.h>

namespace SCIRun {

ReadNrrdAlg* get_read_nrrd_alg()
{
  return new ReadNrrdAlg();
}

//! Algorithm Interface.
vector<size_t>
ReadNrrdAlg::execute(string filename)
{
  vector<size_t> rval(2, 0);
  DataManager *dm = DataManager::get_dm();

  NrrdData *n = new NrrdData;
  if (nrrdLoad(n->nrrd_, airStrdup(filename.c_str()), 0)) {
    char *err = biffGetDone(NRRD);
    ProgressReporter *pr = get_progress_reporter();
    if (pr) {
      pr->error("Read error on '" + filename + "': " + err);
    }
    free(err);
    return rval;
  }
  NrrdDataHandle innrdh = n;
  for (unsigned int i = 0; i < innrdh->nrrd_->dim; i++) {
    if (!(airExists(innrdh->nrrd_->axis[i].min) && 
	  airExists(innrdh->nrrd_->axis[i].max)))
      nrrdAxisInfoMinMaxSet(innrdh->nrrd_, i, nrrdCenterNode);
  }

  rval[0] = dm->add_nrrd(NrrdDataHandle(n));
  return rval;
}

} //end namespace SCIRun

