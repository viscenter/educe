// This is an automatically generated file, do not edit!
#include "Core/Algorithms/ArrayMath/ArrayEngine.h"

namespace DataArrayMath {
template <class DATATYPE>
class ALGOArrayEngine_532229600_LCELID_FS : public ArrayEngineAlgo
{
virtual void function(SCIRunAlgo::ArrayObjectList& input_,SCIRunAlgo::ArrayObjectList& output_, int size_, SCIRun::ProgressReporter* pr_)
  {
    DATATYPE dummy_ = 0.0; dummy_ += 1.0;

    DataArrayMath::Scalar X;
    DataArrayMath::Scalar Y;
    DataArrayMath::Scalar Z;
    DataArrayMath::Vector POS;
    DataArrayMath::Element ELEMENT;
    input_[1].getelement(ELEMENT);
    DataArrayMath::Scalar INDEX;
    DataArrayMath::Scalar SIZE = static_cast<DataArrayMath::Scalar>(size_);
    DataArrayMath::Scalar RESULT;


int cnt_ = 0;    for (int p_ = 0; p_ < size_; p_++)
    {
      input_[0].getnextfieldlocation(POS);
      X=POS.x();
      Y=POS.y();
      Z=POS.z();
      INDEX= static_cast<double>(p_);


      RESULT = X + Y + Z; 


      ELEMENT.next();
     output_[0].setnextfieldscalar(RESULT);
    cnt_++; if (cnt_ == 200) { cnt_ = 0; pr_->update_progress(p_,size_); }
    }
  }

  virtual std::string identify()
  { return std::string("RESULT = X + Y + Z;"); }
};

}


extern "C" {
DataArrayMath::ArrayEngineAlgo* maker_ALGOArrayEngine_532229600_LCELID_FS() {
  return new DataArrayMath::ALGOArrayEngine_532229600_LCELID_FS<double>;
}
}
