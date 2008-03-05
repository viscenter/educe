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

#ifndef CORE_ALGORITHMS_UTIL_BOOLPARAMLIST_H
#define CORE_ALGORITHMS_UTIL_BOOLPARAMLIST_H 1

#include <Core/Datatypes/Color.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Point.h>

#include <vector>
#include <string>
#include <map>

//! For Windows
#include <Core/Algorithms/Util/share.h>

namespace SCIRunAlgo {

class SCISHARE AlgoParam {
  public:
    //! Virtual destructor so we use a dynamic cast to check the data type in
    //! the list.
    virtual ~AlgoParam() {}
};


class SCISHARE BoolParam : public AlgoParam {
  public:
    //! Constructor
    inline BoolParam(bool value) 
      : value_(value) {}
    
    //! Access functions
    inline bool& value() { return (value_); }
        
  private:
    bool value_;
};


class SCISHARE IntegerParam : public AlgoParam {
  public:
    //! Constructor
    inline IntegerParam(int value, int min, int max) 
      : value_(value), min_(min), max_(max), limit_(true) {}

    inline IntegerParam(int value) 
      : value_(value), min_(0), max_(0), limit_(false) {}
    
    //! Access functions
    inline int&  value() { return(value_); }
    inline int&  min()   { return(min_); }
    inline int&  max()   { return(max_); }
    inline bool& limit() { return(limit_); }
     
  private:
    int  value_;  
    int  min_;
    int  max_;
    bool limit_;
};


class SCISHARE ScalarParam : public AlgoParam {
  public:
    //! Constructor
    inline ScalarParam(double value, double min, double max) 
      :  value_(value), min_(min), max_(max), limit_(true) {}

    inline ScalarParam(double value) 
      : value_(value), min_(0.0), max_(0.0), limit_(false) {}
    
    //! Access functions
    inline double& value() { return(value_); }
    inline double& min()   { return(min_); }
    inline double& max()   { return(max_); }
    inline bool&   limit() { return(limit_); }
     
  private:
    double value_;  
    double min_;
    double max_;
    bool   limit_;
};


class SCISHARE ColorParam : public AlgoParam {
  public:
    //! Constructor
    inline ColorParam(SCIRun::Color value) 
      :  value_(value) {}
    
    //! Access functions
    inline SCIRun::Color& value() { return(value_); }
     
  private:
    SCIRun::Color value_;  
};


class SCISHARE VectorParam : public AlgoParam {
  public:
    //! Constructor
    inline VectorParam(SCIRun::Vector value) 
      :  value_(value) {}
    
    //! Access functions
    inline SCIRun::Vector& value() { return(value_); }
     
  private:
    SCIRun::Vector value_;  
};


class SCISHARE PointParam : public AlgoParam {
  public:
    //! Constructor
    inline PointParam(SCIRun::Point value) 
      :  value_(value) {}
    
    //! Access functions
    inline SCIRun::Point& value() { return(value_); }
     
  private:
    SCIRun::Point value_;  
};


class SCISHARE OptionParam : public AlgoParam {
  public:
    //! Constructor
    inline OptionParam(std::string option,std::vector<std::string> options) 
      : option_(option), options_(options) {}
    
    //! Access functions
    inline std::string&  value() { return(option_); }
    inline std::vector<std::string>& options() { return(options_); } 
    
  public:
    std::string              option_;
    std::vector<std::string> options_;
};


class SCISHARE StringParam : public AlgoParam {
  public:
    //! Constructor
    inline StringParam(std::string value) 
      : value_(value) {}
    
    //! Access functions
    inline std::string& value() { return(value_); }
     
  private:
    std::string value_;  
};

class SCISHARE FilenameParam : public AlgoParam {
  public:
    //! Constructor
    inline FilenameParam(std::string value) 
      : value_(value) {}
    
    //! Access functions
    inline std::string& value() { return(value_); }
     
  private:
    std::string value_;  
};


typedef std::map<std::string,AlgoParam*> parameters_type;

class SCISHARE AlgoParamList {
  public:
    
    //! Boolean parameter
    bool set_bool(std::string key, bool value);
    bool get_bool(std::string key, bool& value);

    //! Integer parameter
    bool set_int(std::string key, int value);
    bool get_int(std::string key, int& value);
    bool get_int_limits(std::string key, int& min, int& max);

    //! Scalar parameter
    bool set_scalar(std::string key, double value);
    bool get_scalar(std::string key, double& value);
    bool get_scalar_limits(std::string key, double& min, double& max);
        
    //! Color parameter
    bool set_color(std::string key, SCIRun::Color value);
    bool get_color(std::string key, SCIRun::Color& value);
    
    //! Vector parameter
    bool set_vector(std::string key, SCIRun::Vector value);
    bool get_vector(std::string key, SCIRun::Vector& value);    

    //! Point parameter
    bool set_point(std::string key, SCIRun::Point value);
    bool get_point(std::string key, SCIRun::Point& value);    
        
    //! Option parameter
    bool set_option(std::string key, std::string value);
    bool get_option(std::string key, std::string& value);
    bool get_options(std::string key, std::vector<std::string>& options);

    //! String parameter
    bool set_string(std::string key, std::string value);
    bool get_string(std::string key, std::string& value);

    //! Filename parameter
    bool set_filename(std::string key, std::string value);
    bool get_filename(std::string key, std::string& value);

  public:
    ~AlgoParamList();

  protected:
    void add_bool(std::string key, bool defval);

    void add_int(std::string key, int defval);
    void add_int(std::string key, int defval, int min, int max);

    void add_scalar(std::string key, double defval);
    void add_scalar(std::string key, double defval, double min, double max);

    void add_color(std::string key, SCIRun::Color defval);
    void add_vector(std::string key, SCIRun::Vector defval);
    void add_point(std::string key, SCIRun::Point defval);
    
    void add_option(std::string key, std::string defval, std::string options);
    void add_string(std::string key, std::string defval);
    void add_filename(std::string key, std::string defval);

  private:    
    parameters_type parameters_;

};

}

#endif
