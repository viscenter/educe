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


#ifndef CORE_ALGORITHMS_ARRAYMATH_ARRAYOBJECT_H
#define CORE_ALGORITHMS_ARRAYMATH_ARRAYOBJECT_H 1

#include <sgi_stl_warnings_off.h>
#include <string>
#include <vector>
#include <sgi_stl_warnings_on.h>

#include <Core/Util/TypeDescription.h>
#include <Core/Util/DynamicLoader.h>
#include <Core/Containers/StringUtil.h>
#include <Core/Util/DynamicCompilation.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <Core/Algorithms/ArrayMath/ArrayEngineMath.h>
#include <Core/Algorithms/ArrayMath/ArrayEngineMathElement.h>
#include <Core/Algorithms/ArrayMath/share.h>

namespace SCIRunAlgo {

class SCISHARE ArrayObject {

  public:
    ArrayObject(SCIRun::ProgressReporter *pr) :
      vfield_(0), vmesh_(0), pr_(pr), type_(0), size_(0),
      data_(0), idx_(0), is_scalar_(false),
      is_vector_(false), is_tensor_(false),
      is_location_(false), is_index_(false),
      is_element_(false)
    {}
    
                
    bool create_input_data(SCIRun::FieldHandle field, std::string name);
    bool create_input_data(SCIRun::MatrixHandle matrix, std::string name);
    bool create_input_index(std::string name, std::string sizename);
    bool create_input_location(SCIRun::FieldHandle field, std::string locname, std::string xname, std::string yname, std::string zname,std::string location = "");
    bool create_input_element(SCIRun::FieldHandle field, std::string name);
    
    bool create_output_data(SCIRun::FieldHandle& field, std::string datatype, std::string name,SCIRun::FieldHandle& ofield);
    bool create_output_data(SCIRun::FieldHandle& field, std::string datatype, std::string basistype, std::string name,SCIRun::FieldHandle& ofield);
    bool create_output_data(int size, std::string datatype, std::string name,SCIRun::MatrixHandle& omatrix);    
    bool create_output_location(SCIRun::FieldHandle& field, std::string locname,SCIRun::FieldHandle& ofield);

    inline void get_element(DataArrayMath::Element& elem)
    {
      elem = DataArrayMath::Element(vfield_);
    }

    // Get the next value
    inline void get_next_scalar(DataArrayMath::Scalar& scalar)
      {
        switch(type_)
        {
          // from a char field
          case 1:
          {
            scalar= static_cast<DataArrayMath::Scalar>(((char*)data_)[idx_]); 
            idx_++; return;
          }
          // from an unsigned char field
          case 2:
            scalar= static_cast<DataArrayMath::Scalar>(((unsigned char*)data_)[idx_]); 
            idx_++; return;
          // from an short field
          case 3:
            scalar= static_cast<DataArrayMath::Scalar>(((short*)data_)[idx_]); 
            idx_++; return;
          // from an unsigned short field
          case 4:
            scalar= static_cast<DataArrayMath::Scalar>(((unsigned short*)data_)[idx_]); 
            idx_++; return;
          // from an int field
          case 5:
            scalar= static_cast<DataArrayMath::Scalar>(((int*)data_)[idx_]); 
            idx_++; return;
          // from an unsigned int field
          case 6:
            scalar= static_cast<DataArrayMath::Scalar>(((unsigned int*)data_)[idx_]); 
            idx_++; return;
          // from an longlong field
          case 7:
            scalar= static_cast<DataArrayMath::Scalar>(((long long*)data_)[idx_]); 
            idx_++; return;
          // from an unsigned longlong field
          case 8:
            scalar= static_cast<DataArrayMath::Scalar>(((unsigned long long*)data_)[idx_]); 
            idx_++; return;
          // from an float field
          case 9:
            scalar= static_cast<DataArrayMath::Scalar>(((float*)data_)[idx_]); 
            idx_++; return;
          // from an double field
          case 10:
            scalar= ((double*)data_)[idx_]; 
            idx_++; return;
          // constant matrix
          case 11:
            scalar = constant_scalar_; return;
          // from a matrix class              
          case 12:
            scalar = ((double*)data_)[idx_]; 
            idx_++; return;
          default:
            ASSERTFAIL("ArrayObject: encountered an internal error");
        }
      }
    
     inline void get_next_vector(DataArrayMath::Vector& vector)
      {
        switch(type_)
        {
          case 13:
            vector = DataArrayMath::Vector(((SCIRun::Vector*)data_)[idx_].x(),
                                           ((SCIRun::Vector*)data_)[idx_].y(),
                                           ((SCIRun::Vector*)data_)[idx_].z());
            idx_++; return;
          case 14:
            vector = constant_vector_; return;
          case 15:
            vector = DataArrayMath::Vector(((double*)data_)[idx_],
                                           ((double*)data_)[idx_+1],
                                           ((double*)data_)[idx_+2]);
            idx_+=3; return;                                
          default:
            ASSERTFAIL("ArrayObject: encountered an internal error");
        }
      }
    
    inline void get_next_tensor(DataArrayMath::Tensor& tensor)
      {
        switch(type_)
        {
          case 16:  
            tensor = DataArrayMath::Tensor(((SCIRun::Tensor*)data_)[idx_].xx(),
                                           ((SCIRun::Tensor*)data_)[idx_].xy(),
                                           ((SCIRun::Tensor*)data_)[idx_].xz(),
                                           ((SCIRun::Tensor*)data_)[idx_].yy(),
                                           ((SCIRun::Tensor*)data_)[idx_].yz(),
                                           ((SCIRun::Tensor*)data_)[idx_].zz());
            idx_++; return;                                
          case 17:
            tensor = constant_tensor_; return;
          case 18:
            tensor = DataArrayMath::Tensor(((double*)data_)[idx_],
                                           ((double*)data_)[idx_+1],
                                           ((double*)data_)[idx_+2],
                                           ((double*)data_)[idx_+3],
                                           ((double*)data_)[idx_+4],
                                           ((double*)data_)[idx_+5]);
            idx_+=6; return;                                
          case 19:
            tensor = DataArrayMath::Tensor(((double*)data_)[idx_],
                                           ((double*)data_)[idx_+1],
                                           ((double*)data_)[idx_+2],
                                           ((double*)data_)[idx_+4],
                                           ((double*)data_)[idx_+5],
                                           ((double*)data_)[idx_+8]);
            idx_+=9; return;                                
          default:
            ASSERTFAIL("ArrayObject: encountered an internal error");
        }
      }
    

    // Get the next value
    inline void set_next_scalar(DataArrayMath::Scalar& scalar)
      {
        switch(type_)
        {
          // from a char field
          case 1:
            ((char*)data_)[idx_] = static_cast<char>(scalar);
            idx_++; return;
          // from an unsigned char field
          case 2:
            ((unsigned char*)data_)[idx_] = static_cast<unsigned char>(scalar);
            idx_++; return;
          // from an short field
          case 3:
            ((short*)data_)[idx_] = static_cast<short>(scalar);
            idx_++; return;
          // from an unsigned short field
          case 4:
            ((unsigned short*)data_)[idx_] = static_cast<unsigned short>(scalar);
            idx_++; return;
          // from an int field
          case 5:
            ((int*)data_)[idx_] = static_cast<int>(scalar);
            idx_++; return;
          // from an unsigned int field
          case 6:
            ((unsigned int*)data_)[idx_] = static_cast<unsigned int>(scalar);
            idx_++; return;
          // from an longlong field
          case 7:
            ((long long*)data_)[idx_] = static_cast<long long>(scalar);
            idx_++; return;
          // from an unsigned longlong field
          case 8:
            ((unsigned long long*)data_)[idx_] = static_cast<unsigned long long>(scalar);
            idx_++; return;
          // from an float field
          case 9:
            ((float*)data_)[idx_] = static_cast<float>(scalar);
            idx_++; return;
          // from an double field
          case 10:
            ((double*)data_)[idx_] = static_cast<double>(scalar);
            idx_++; return;
          // constant matrix
          case 11:
            constant_scalar_ = scalar; return;
          // from a matrix class              
          case 12:
            ((double*)data_)[idx_] = scalar; 
            idx_++; return;
          default:
            ASSERTFAIL("ArrayObject: encountered an internal error");
        }
      }
    
      inline void set_next_vector(DataArrayMath::Vector& vector)
      {
        switch(type_)
        {
          case 13:  
            ((SCIRun::Vector*)data_)[idx_] = SCIRun::Vector(vector.x(), vector.y(), vector.z());
            idx_++; return;                                
          case 14:
            constant_vector_ = vector; return;
          case 15:
            ((double*)data_)[idx_] = vector.x();
            ((double*)data_)[idx_+1] = vector.y();
            ((double*)data_)[idx_+2] = vector.z();
            idx_+=3; return;                                
          default:
            ASSERTFAIL("ArrayObject: encountered an internal error");
        }
      }   
    
    
    inline void set_next_tensor(DataArrayMath::Tensor& tensor)
      {
        switch(type_)
        {
          case 16:  
            ((SCIRun::Tensor*)data_)[idx_] = SCIRun::Tensor(tensor.xx(),
                                                    tensor.xy(),
                                                    tensor.xz(),
                                                    tensor.yy(),
                                                    tensor.yz(),
                                                    tensor.zz());
            idx_++; return;                                
          case 17:
            tensor = constant_tensor_; return;
          case 18:
            ((double*)data_)[idx_  ] = tensor.xx();
            ((double*)data_)[idx_+1] = tensor.xy();
            ((double*)data_)[idx_+2] = tensor.xz();
            ((double*)data_)[idx_+3] = tensor.yy();
            ((double*)data_)[idx_+4] = tensor.yz();
            ((double*)data_)[idx_+5] = tensor.zz();
            idx_+=6; return;                                
          case 19:
            ((double*)data_)[idx_  ] = tensor.xx();
            ((double*)data_)[idx_+1] = tensor.xy();
            ((double*)data_)[idx_+2] = tensor.xz();
            ((double*)data_)[idx_+3] = tensor.xy();
            ((double*)data_)[idx_+4] = tensor.yy();
            ((double*)data_)[idx_+5] = tensor.yz();
            ((double*)data_)[idx_+6] = tensor.xz();
            ((double*)data_)[idx_+7] = tensor.yz();
            ((double*)data_)[idx_+8] = tensor.zz();
            idx_+=9; return;                                
          default:
            ASSERTFAIL("ArrayObject: encountered an internal error");
        }
      }


    inline void set_next_location(DataArrayMath::Vector& location)
    {
      SCIRun::Point p(location.x(),location.y(),location.z());
      vmesh_->set_point(p,SCIRun::VMesh::Node::index_type(idx_));
      idx_++;
    }

    inline void get_next_location(DataArrayMath::Vector& location)
    {
      SCIRun::Point p;
      if (vfield_->basis_order() == 0)
      {
        vmesh_->get_center(p,SCIRun::VMesh::Elem::index_type(idx_));      
      }
      else
      {
        vmesh_->get_center(p,SCIRun::VMesh::Node::index_type(idx_));
      }
      idx_++;
      location = DataArrayMath::Vector(p.x(),p.y(),p.z());
    }


    inline void getelement(DataArrayMath::Element& elem);

    inline bool is_scalar()   { return (is_scalar_); }
    inline bool is_vector()   { return (is_vector_); }
    inline bool is_tensor()   { return (is_tensor_); }
    inline bool is_location() { return (is_location_); }
    inline bool is_element()  { return (is_element_); }
    inline bool is_index()    { return (is_index_); }
    inline bool is_matrix()    { return (matrix_.get_rep()!=0); }
    inline bool is_valid()     { return (is_scalar_||is_vector_||is_tensor_||
                                  is_location_||is_element_||is_index_); } 
    inline bool is_array()     { return (size_ > 1); }
    inline int  size()         { return (size_); }
    inline void reset()        { idx_ = 0; }

    inline std::string get_name()        { return (name_); }
    inline std::string get_size_name()   { return (sizename_); }
    inline std::string get_x_name()      { return (xname_); }
    inline std::string get_y_name()      { return (yname_); }
    inline std::string get_z_name()      { return (zname_); }
    
  private:

    // Storage for handles
    SCIRun::MatrixHandle  matrix_;
    SCIRun::FieldHandle   field_;

    // Where to access data for field
    SCIRun::VField*       vfield_;
    SCIRun::VMesh*        vmesh_;

    // For dynamic compiler
    SCIRun::ProgressReporter *pr_;

    // Object name in function
    std::string name_;
    std::string xname_;
    std::string yname_;
    std::string zname_;
    std::string sizename_;
    
    // dimensions of this data stream
    SCIRun::Field::size_type  type_;
    SCIRun::Field::size_type  size_;
    
    // Where to get the data
    void*             data_;
    SCIRun::Field::index_type idx_;
    
    bool        is_scalar_;
    bool        is_vector_;
    bool        is_tensor_;
    bool        is_location_;
    bool        is_index_;
    bool        is_element_;
    
    // Constant types
    double                constant_scalar_;
    DataArrayMath::Vector constant_vector_;
    DataArrayMath::Tensor constant_tensor_;
    
    void clear();    
};

typedef std::vector<ArrayObject> ArrayObjectList;

} // end namespace

#endif
