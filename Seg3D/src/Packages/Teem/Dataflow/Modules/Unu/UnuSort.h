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
 *  UnuSort.cc:
 *
 *  Create a Field from Nrrd Daata.  * Incoming Nrrds may consist of
 *  Data, Points, Connections and a * Field mesh to associate it with.
 *
 *  Written by:
 *    Allen Sanderson, Darby Van Uitert
 *    School of Computing
 *    University of Utah
 *    March 2004
 *
 *  Copyright (C) 2001 SCI Institute
 */

#if !defined(UnuSort_h)
#define UnuSort_h

#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>

#include <Core/Datatypes/NrrdData.h>

#include <algorithm>

namespace SCITeem {

using namespace SCIRun;

class SCISHARE UnuSortAlgo : public DynamicAlgoBase
{
public:
  bool UnuSort( ProgressReporter *pr,
		NrrdDataHandle nrrd_input_handle,
		unsigned int index ) { return false; }

  //! support the dynamically compiled algorithm concept
  static CompileInfoHandle get_compile_info( const unsigned int ntype,
					     const unsigned int index );

  virtual bool execute(NrrdDataHandle nrrd_input_handle) {return false;}
};


template< class NTYPE, unsigned index >
class UnuSortAlgoT :  public UnuSortAlgo
{
public:
  virtual bool execute(NrrdDataHandle nrrd_input_handle);

  static int nrrd_sort_cmp( const void* p, const void* q );
};


template< class NTYPE, unsigned index>
int UnuSortAlgoT< NTYPE, index >::
nrrd_sort_cmp( const void* p, const void* q )
{
  NTYPE *pNrrdptr = (NTYPE *)(p);
  NTYPE *qNrrdptr = (NTYPE *)(q);

  return (int) (pNrrdptr[index] - qNrrdptr[index]);
}
 

template< class NTYPE, unsigned index>
bool UnuSortAlgoT< NTYPE, index >::
execute(NrrdDataHandle nrrd_input_handle)
{
  void *ptr = (void *)(nrrd_input_handle->nrrd_->data);

  NTYPE ntype;

  unsigned width     = nrrd_input_handle->nrrd_->axis[0].size;
  unsigned nElements = nrrd_input_handle->nrrd_->axis[1].size;

  qsort( ptr, nElements, width*sizeof( ntype ), UnuSortAlgoT::nrrd_sort_cmp );

  return true;
}

} // end namespace SCITeem

#endif // UnuSort_h
