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
#include <Core/Algorithms/Interface/Visualization/ConvertNrrdsToTextureAlg.h>

namespace SCIRun {

ConvertNrrdsToTextureAlg* get_convert_nrrds_to_texture_alg()
{
  return new ConvertNrrdsToTextureAlg();
}

class SortFloatVector : 
    public std::binary_function<unsigned int, unsigned int, bool>
{
public:
  SortFloatVector(std::vector<float>& hist) {
    data_ = &hist[0];
  }
  
  bool operator()(unsigned int i1, unsigned int i2) {
    return (data_[i1] < data_[i2]);
  }

private:
  float*      data_;
};   


//! Algorithm Interface.
vector<size_t>
ConvertNrrdsToTextureAlg::execute(size_t values, size_t gradients)
{
  vector<size_t> rval(2, 0);
  DataManager *dm = DataManager::get_dm();

  
  TextureHandle texh;

  if (!values) return rval;

  NrrdDataHandle vHandle = dm->get_nrrd(values);
  NrrdDataHandle gHandle;
  
  Nrrd* nv_nrrd = vHandle->nrrd_;
  ProgressReporter *pr = get_progress_reporter();
  
  if (nv_nrrd->dim != 3 && nv_nrrd->dim != 4) {
    if (pr) pr->error("Invalid dimension for input value nrrd.");
    return rval;
  }
  size_t axis_size[4];
  nrrdAxisInfoGet_nva(nv_nrrd, nrrdAxisInfoSize, axis_size);
  if (nv_nrrd->dim == 4 && axis_size[0] != 1 && axis_size[0] != 4) {
    if (pr) pr->error("Invalid axis size for Normal/Value nrrd.");
    return rval;
  }

  if( !get_p_is_fixed() ) {
    // set vmin/vmax
    NrrdRange *range = nrrdRangeNewSet(vHandle->nrrd_, 
                                       nrrdBlind8BitRangeFalse);

    set_p_vmin(range->min);
    set_p_vmax(range->max);
    nrrdRangeNix(range);
  }

  if (texh.get_rep() == 0) { texh = new Texture(); }

  
  // The gradient nrrd input is optional.
  if (gradients)
  {
    gHandle = dm->get_nrrd(gradients);
    Nrrd* gm_nrrd = gHandle->nrrd_;

    if (gm_nrrd->dim != 3 && gm_nrrd->dim != 4) 
    {
      if (pr) 
        pr->error("Invalid dimension for input gradient magnitude nrrd.");
      return rval;
    }
      
    if( gm_nrrd->dim == 4 ) 
    {
      nrrdAxisInfoGet_nva(gm_nrrd, nrrdAxisInfoSize, axis_size);
      if (axis_size[0] != 1) 
      {
        if (pr) pr->error("Invalid axis size for gradient magnitude nrrd.");
        return rval;
      }
    }

    // The input nrrd type must be unsigned char.
    if (get_p_is_uchar() && gHandle->nrrd_->type != nrrdTypeUChar) 
    {
      if (pr) 
        pr->error("Gradient magnitude input must be unsigned char.");
      return rval;
    }

    if (! get_p_is_fixed())
    {
      // set gmin/gmax
      NrrdRange *range = nrrdRangeNewSet(gHandle->nrrd_, 
                                         nrrdBlind8BitRangeFalse);

      set_p_gmin(range->min);
      set_p_gmax(range->max);
    }	
  }

  Nrrd *v = 0;
  Nrrd *g = 0;
  texh = new Texture;
  if (vHandle.get_rep()) v = vHandle->nrrd_;
  if (gHandle.get_rep()) g = gHandle->nrrd_;
  texh->build(v, g, get_p_vmax(), get_p_vmin(), 
              get_p_gmin(), get_p_gmax(), get_p_card_mem());  
  texh->value_nrrd_ = vHandle;
  texh->gradient_magnitude_nrrd_ = gHandle;
  rval[0] = dm->add_texture(texh);

  // Generate a reasonable histogram
  if (gHandle.get_rep() && get_p_histogram())
  {
    NrrdDataHandle HistoGramNrrd = new NrrdData;

    // build joint histogram
    size_t sx = 256;
    size_t sy = 256;

    size_t sdim[2]; sdim[0] = sx; sdim[1] = sy;
    nrrdAlloc_nva(HistoGramNrrd->nrrd_ ,nrrdTypeUChar ,2 ,sdim );
    
    int centerdata[2]; 
    centerdata[0] = nrrdCenterNode; 
    centerdata[1] = nrrdCenterNode;
    nrrdAxisInfoSet_nva(HistoGramNrrd->nrrd_, nrrdAxisInfoCenter, centerdata);
  

    size_t sz = sx*sy;
    std::vector<float> hist(sz);
    std::vector<size_t> index(sz);
    
    for (size_t j=0; j<sz; j++)  {
      hist[j] = 0.0;
    }
    
    unsigned char* value = static_cast<unsigned char*>(vHandle->nrrd_->data);
    unsigned char* gradient = 
      static_cast<unsigned char*>(gHandle->nrrd_->data);
    unsigned char* data = 
      static_cast<unsigned char*>(HistoGramNrrd->nrrd_->data);

    size_t mul, offset;
    if (vHandle->nrrd_->dim == 4) {
      offset = vHandle->nrrd_->axis[0].size-1;
      mul = vHandle->nrrd_->axis[0].size;
      sz = (vHandle->nrrd_->axis[1].size * vHandle->nrrd_->axis[2].size * 
            vHandle->nrrd_->axis[3].size);
    } else {
      offset = 0;
      mul = 1;
      sz = (vHandle->nrrd_->axis[0].size * vHandle->nrrd_->axis[1].size * 
            vHandle->nrrd_->axis[2].size);
    }
    
    for (size_t j=0; j<sz; j++)
    {
      unsigned char p = value[mul*j+offset];
      unsigned char q = gradient[j];
      hist[p+q*sx]++; 
    }

    double gamma = get_p_gamma();
    sz = sx*sy;
    std::vector<size_t> temp(sz);

    for (size_t j=0; j<sz; j++) {
      temp[j] = j;
    }
    
    std::sort(temp.begin(), temp.end(), SortFloatVector(hist));

    float threshold = hist[temp[static_cast<size_t>(0.99 * sz)]] * 0.001;
    
    float oldv = 0.0;
    unsigned char old = 0;
    size_t j=0;
    for (; j<sz; j++) 
    {    
      if (hist[temp[j]] > threshold) break;
      data[temp[j]] = 0;
    }
    
    size_t jc = j;
    
    for (; j<sz; j++) {
      if (hist[temp[j]] == oldv) {
        data[temp[j]] = old;
      } else {
        oldv = hist[temp[j]];
        data[temp[j]] = 
          static_cast<unsigned char>(255.0 * 
                                     pow((double)(j - jc) / 
                                         (double)(sz -jc - 1), gamma)); 
        old = data[temp[j]];
      }
    }

    rval[1] = dm->add_nrrd(HistoGramNrrd);
  }
  return rval;
}

} //end namespace SCIRun

