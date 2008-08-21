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

#include <Core/Containers/LockingHandle.h>
#include <Core/Datatypes/Material.h>
#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Geometry/Vector.h>
#include <Core/Geometry/Point.h>

#include <vector>
#include <string>
#include <map>

//! For Windows
#include <Core/Algorithms/Util/share.h>

namespace SCIRunAlgo {

class Algo {
  public:
    enum {
	  IN_E = 1, OUT_E = 2, INOUT_E = 3
    };
};

class SCISHARE AlgoParam : public Algo {
  public:
    AlgoParam(int type) : type_(type) {}
  
    //! Virtual destructor so we use a dynamic cast to check the data type in
    //! the list.
    virtual ~AlgoParam() {}
    inline int& type() { return (type_); }  
    
  private:
    int type_;
};


class SCISHARE BoolParam : public AlgoParam {
  public:
    //! Constructor
    inline BoolParam(bool value, int type) 
      : AlgoParam(type), value_(value) {}
    
    //! Access functions
    inline bool& value() { return (value_); }
        
  private:
    bool value_;
};


class SCISHARE IntegerParam : public AlgoParam {
  public:
    //! Constructor
    inline IntegerParam(int value, int min, int max, int type) 
      : AlgoParam(type), value_(value), min_(min), max_(max), limit_(true) {}

    inline IntegerParam(int value, int type) 
      : AlgoParam(type), value_(value), min_(0), max_(0), limit_(false) {}
    
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
    inline ScalarParam(double value, double min, double max, int type) 
      :  AlgoParam(type), value_(value), min_(min), max_(max), limit_(true) {}

    inline ScalarParam(double value, int type) 
      : AlgoParam(type), value_(value), min_(0.0), max_(0.0), limit_(false) {}
    
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
    inline ColorParam(SCIRun::Color value, int type) 
      :  AlgoParam(type), value_(value) {}
    
    //! Access functions
    inline SCIRun::Color& value() { return(value_); }
     
  private:
    SCIRun::Color value_;  
};

class SCISHARE ColorMapParam : public AlgoParam {
  public:
    //! Constructor
    inline ColorMapParam(SCIRun::ColorMapHandle value, int type) 
      : AlgoParam(type), value_(value) {}
    
    //! Access functions
    inline SCIRun::ColorMapHandle value() { return(value_); }
     
  private:
    SCIRun::ColorMapHandle value_;  
};


class SCISHARE VectorParam : public AlgoParam {
  public:
    //! Constructor
    inline VectorParam(SCIRun::Vector value, int type) 
      : AlgoParam(type), value_(value) {}
    
    //! Access functions
    inline SCIRun::Vector& value() { return(value_); }
     
  private:
    SCIRun::Vector value_;  
};


class SCISHARE PointParam : public AlgoParam {
  public:
    //! Constructor
    inline PointParam(SCIRun::Point value, int type) 
      : AlgoParam(type), value_(value) {}
  
    //! Access functions
    inline SCIRun::Point& value() { return(value_); }
     
  private:
    SCIRun::Point value_;  
};


class SCISHARE OptionParam : public AlgoParam {
  public:
    //! Constructor
    inline OptionParam(std::string option,std::vector<std::string> options,int type) 
      : AlgoParam(type), option_(option), options_(options) {}
    
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
    inline StringParam(std::string value, int type) 
      : AlgoParam(type), value_(value) {}
    
    //! Access functions
    inline std::string& value() { return(value_); }
     
  private:
    std::string value_;  
};

class SCISHARE FilenameParam : public AlgoParam {
  public:
    //! Constructor
    inline FilenameParam(std::string value, int type) 
      : AlgoParam(type), value_(value) {}
    
    //! Access functions
    inline std::string& value() { return(value_); }
     
  private:
    std::string value_;  
};

class SCISHARE HandleParam : public AlgoParam {
  public:
    //! Constructor
    inline HandleParam(int type) 
      : AlgoParam(type), value_(0) {}
    
    //! Access functions
    inline SCIRun::LockingHandle<SCIRun::Datatype>& value() { return(value_); }
     
  private:
    SCIRun::LockingHandle<SCIRun::Datatype> value_;  
};


typedef std::map<std::string,AlgoParam*> parameters_type;

class SCISHARE AlgoParamList : public Algo {
  public:
    
    //! Boolean parameter
    bool set_bool(std::string key, bool value);
    bool get_bool(std::string key, bool& value);
    bool get_bool(std::string key);

    //! Integer parameter
    bool set_int(std::string key, int value);
    bool get_int(std::string key, int& value);
    bool get_int_limits(std::string key, int& min, int& max);
    int get_int(std::string key);

    //! Scalar parameter
    bool set_scalar(std::string key, double value);
    bool get_scalar(std::string key, double& value);
    bool get_scalar_limits(std::string key, double& min, double& max);
    double get_scalar(std::string key);    
        
    //! Color parameter
    bool set_color(std::string key, SCIRun::Color value);
    bool get_color(std::string key, SCIRun::Color& value);
    SCIRun::Color get_color(std::string key);

    //! Colormap parameter
    bool set_colormap(std::string key, SCIRun::ColorMap* value);
    bool get_colormap(std::string key, SCIRun::ColorMap*& value);
    SCIRun::ColorMap* get_colormap(std::string key);
    
    //! Vector parameter
    bool set_vector(std::string key, SCIRun::Vector value);
    bool get_vector(std::string key, SCIRun::Vector& value);    
    SCIRun::Vector get_vector(std::string key);
  
    //! Point parameter
    bool set_point(std::string key, SCIRun::Point value);
    bool get_point(std::string key, SCIRun::Point& value);    
    SCIRun::Point get_point(std::string key);    
        
    //! Option parameter
    bool set_option(std::string key, std::string value);
    bool get_option(std::string key, std::string& value);
    bool get_options(std::string key, std::vector<std::string>& options);
    std::string get_option(std::string key);

    //! String parameter
    bool set_string(std::string key, std::string value);
    bool get_string(std::string key, std::string& value);
    std::string get_string(std::string key);

    //! Filename parameter
    bool set_filename(std::string key, std::string value);
    bool get_filename(std::string key, std::string& value);
    std::string get_filename(std::string key);

    //! Handle parameter
    template <class T>
    bool set_handle(std::string key, SCIRun::LockingHandle<T>& handle);
    template <class T>
    bool get_handle(std::string key, SCIRun::LockingHandle<T>& handle);
      
  public:
    ~AlgoParamList();

  protected:
    void add_bool(std::string key, bool defval, int type = Algo::IN_E);

    void add_int(std::string key, int defval, int type = Algo::IN_E);
    void add_int(std::string key, int defval, int min, int max, int type = Algo::IN_E);

    void add_scalar(std::string key, double defval, int type = Algo::IN_E);
    void add_scalar(std::string key, double defval, double min, double max, int type = Algo::IN_E);

    void add_color(std::string key, SCIRun::Color defval, int type = Algo::IN_E);
    void add_colormap(std::string key, SCIRun::ColorMap* defval, int type = Algo::IN_E);
    void add_vector(std::string key, SCIRun::Vector defval, int type = Algo::IN_E);
    void add_point(std::string key, SCIRun::Point defval, int type = Algo::IN_E);
    
    void add_option(std::string key, std::string defval, std::string options, int type = Algo::IN_E);
    void add_string(std::string key, std::string defval, int type = Algo::IN_E);
    void add_filename(std::string key, std::string defval, int type = Algo::IN_E);
    
    void add_handle(std::string key, int type = Algo::IN_E);

  private:    
    parameters_type parameters_;

};


template<class T>
bool 
AlgoParamList::set_handle(std::string key, SCIRun::LockingHandle<T>& handle )
{
  HandleParam* param = dynamic_cast<HandleParam*>(parameters_[key]);
  if (param) 
  { 
    param->value() = dynamic_cast<T*>(handle.get_rep()); 
    return (true); 
  }
  
  throw std::string("key \""+key+"\" was not defined in algorithm");
  return (false); 
}


template<class T>
bool
AlgoParamList::get_handle(std::string key, SCIRun::LockingHandle<T>& handle )
{
  HandleParam* param = dynamic_cast<HandleParam*>(parameters_[key]);
  if (param) 
  { 
    handle = dynamic_cast<T*>(param->value().get_rep()); 
    return (true); 
  }
  else 
  { 
    throw std::string("key \""+key+"\" was not defined in algorithm");
    return (false); 
  }
}

}

#endif
