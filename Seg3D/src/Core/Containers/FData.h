//  The contents of this file are subject to the University of Utah Public
//  License (the "License"); you may not use this file except in compliance
//  with the License.
//  
//  Software distributed under the License is distributed on an "AS IS"
//  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
//  
//  the License.
//  
//  The Original Source Code is SCIRun, released March 12, 2001.
//  
//  The Original Source Code was developed by the University of Utah.
//  Portions created by UNIVERSITY are Copyright (C) 2001, 1994
//  University of Utah. All Rights Reserved.
//  
//    File   : FData.h
//    Author : Martin Cole
//    Date   : Wed Apr 28 09:45:51 2004
//    
//    Taken from old LatVolField.h ... 


#ifndef CORE_CONTAINERS_FDATA_H
#define CORE_CONTAINERS_FDATA_H 1

#include <Core/Util/TypeDescription.h>
#include <Core/Datatypes/Types.h>
#include <Core/Datatypes/TypeName.h>
#include <Core/Datatypes/Mesh.h>
#include <Core/Datatypes/VMesh.h>
#include <Core/Containers/Array3.h>
#include <Core/Containers/Array2.h>
#include <Core/Math/MiscMath.h>
#include <Core/Malloc/Allocator.h>
#include <Core/Util/Assert.h>
#include <sgi_stl_warnings_off.h>
#include <string>
#include <sgi_stl_warnings_on.h>

namespace SCIRun {

using std::string;


template <class Data, class Msh>
class FData3d : public Array3<Data> {
public:
  typedef Data value_type;
  typedef Data * iterator;
  typedef const Data * const_iterator;

  iterator begin() { return &(*this)(0,0,0); } 
  iterator end() { return &((*this)(this->dim1()-1,this->dim2()-1,this->dim3()-1))+1; }
  const_iterator begin() const { return &(*this)(0,0,0); } 
  const_iterator end() const { return &((*this)(this->dim1()-1,this->dim2()-1,this->dim3()-1))+1; }

    
  FData3d() : Array3<Data>() {}
  FData3d(int) : Array3<Data>() {} //default arg sgi bug workaround.
  FData3d(const FData3d& data) {Array3<Data>::copy(data); }
  virtual ~FData3d();
  
  const value_type &operator[](typename Msh::Cell::index_type idx) const
  { return operator()(idx.k_,idx.j_,idx.i_); } 
  const value_type &operator[](typename Msh::Face::index_type idx) const
  { return operator()(0, 0, idx); }
  const value_type &operator[](typename Msh::Edge::index_type idx) const
  { return operator()(0, 0, idx); }    
  const value_type &operator[](typename Msh::Node::index_type idx) const
  { return operator()(idx.k_,idx.j_,idx.i_); }    
  const value_type &operator[](VMesh::index_type idx) const
  { return Array3<Data>::operator()(idx); }

  value_type &operator[](typename Msh::Cell::index_type idx)
  { return operator()(idx.k_,idx.j_,idx.i_); } 
  value_type &operator[](typename Msh::Face::index_type idx)
  { return operator()(0, 0, idx); }
  value_type &operator[](typename Msh::Edge::index_type idx)
  { return operator()(0, 0, idx); }    
  value_type &operator[](typename Msh::Node::index_type idx)
  { return operator()(idx.k_,idx.j_,idx.i_); }    
  value_type &operator[](VMesh::index_type idx)
  { return Array3<Data>::operator()(idx); }

  void resize(const typename Msh::Node::size_type &size) 
  { Array3<Data>::resize(size.k_, size.j_, size.i_); }
  void resize(const typename Msh::Edge::size_type &size)
  { Array3<Data>::resize(1, 1, size); }
  void resize(const typename Msh::Face::size_type &size)
  { Array3<Data>::resize(1, 1, size); }
  void resize(const typename Msh::Cell::size_type &size)
  { Array3<Data>::resize(size.k_, size.j_, size.i_); }
  void resize(const size_t size)
  { Array3<Data>::resize(1,1,size); }

  unsigned int size() const { return this->dim1() * this->dim2() * this->dim3(); }

  static const string type_name(int n = -1);
  const TypeDescription* get_type_description(int n) const;
};


template <class Data, class Msh>
FData3d<Data, Msh>::~FData3d()
{
}

  
template <class Data, class Msh>
const string
FData3d<Data, Msh>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 2);
  if (n == -1)
  {
    static const string name = type_name(0) + FTNS + type_name(1) + FTNM +type_name(2) + FTNE;
    return name;
  }
  else if (n == 0)
  {
    return "FData3d";
  }
  else if (n == 1)
  {
    return find_type_name((Data *)0);
  }
  else
  {
    return find_type_name((Msh *)0);
  }
}

template <class Data, class Msh>
const TypeDescription*
FData3d<Data, Msh>::get_type_description(int n) const 
{
  ASSERT((n >= -1) && n <= 2);

  static string name(type_name(0));
  static string namesp("SCIRun");
  static string path(__FILE__);
  const TypeDescription *sub1 = SCIRun::get_type_description((Data*)0);
  const TypeDescription *sub2 = SCIRun::get_type_description((Msh*)0);

  if (n == -1) {
    static TypeDescription* tdn1 = 0;
    if (tdn1 == 0) {
      TypeDescription::td_vec *subs = scinew TypeDescription::td_vec(2);
      (*subs)[0] = sub1;
      (*subs)[1] = sub2;
      tdn1 = scinew TypeDescription(name, subs, path, namesp,
				    TypeDescription::CONTAINER_E);
    } 
    return tdn1;
  }
  else if(n == 0) {
    static TypeDescription* tdn0 = 0;
    if (tdn0 == 0) {
      tdn0 = scinew TypeDescription(name, 0, path, namesp,
				    TypeDescription::CONTAINER_E);
    }
    return tdn0;
  }
  else if(n == 1) {
    return sub1;
  }
  return sub2;
}

template <class Data, class Msh>
const TypeDescription* 
get_type_description(FData3d<Data, Msh>*)
{
  static string name(FData3d<Data, Msh>::type_name(0));
  static string namesp("SCIRun");
  static string path(__FILE__);
  const TypeDescription *sub1 = SCIRun::get_type_description((Data*)0);
  const TypeDescription *sub2 = SCIRun::get_type_description((Msh*)0);

  static TypeDescription* tdn1 = 0;
  if (tdn1 == 0) {
    TypeDescription::td_vec *subs = scinew TypeDescription::td_vec(2);
    (*subs)[0] = sub1;
    (*subs)[1] = sub2;
    tdn1 = scinew TypeDescription(name, subs, path, namesp,
				  TypeDescription::CONTAINER_E);
  }
  return tdn1;
}



template <class Data, class Msh>
class FData2d : public Array2<Data> {
public:
  typedef Data value_type;
  typedef Data * iterator;
  typedef Data const * const_iterator;

  iterator begin() { return &(*this)(0,0); }
  iterator end() { return &((*this)(this->dim1()-1,this->dim2()-1))+1; }
  const_iterator begin() const { return &(*this)(0,0); }
  const_iterator end() const { return &((*this)(this->dim1()-1,this->dim2()-1))+1; }

  FData2d() : Array2<Data>() {}
  FData2d(int) : Array2<Data>() {} //default var sgi bug workaround.
  FData2d(const FData2d& data) { Array2<Data>::copy(data); }
  virtual ~FData2d();
  
  const value_type &operator[](typename Msh::Cell::index_type idx) const
  { return operator()(0, idx); } 
  const value_type &operator[](typename Msh::Face::index_type idx) const
  { return operator()(idx.j_, idx.i_); }
  const value_type &operator[](typename Msh::Edge::index_type idx) const
  { return operator()(0, idx); }
  const value_type &operator[](typename Msh::Node::index_type idx) const
  { return operator()(idx.j_, idx.i_); }
  const value_type &operator[](VMesh::index_type idx) const
  { return Array2<Data>::operator()(static_cast<unsigned int>(idx)); }

  value_type &operator[](typename Msh::Cell::index_type idx)
  { return operator()(0, idx); } 
  value_type &operator[](typename Msh::Face::index_type idx)
  { return operator()(idx.j_, idx.i_); }
  value_type &operator[](typename Msh::Edge::index_type idx)
  { return operator()(0, idx); }
  value_type &operator[](typename Msh::Node::index_type idx)
  { return operator()(idx.j_, idx.i_); }
  value_type &operator[](VMesh::index_type idx)
  { return Array2<Data>::operator()(static_cast<unsigned int>(idx)); }

  void resize(const typename Msh::Node::size_type &size)
  { Array2<Data>::resize(size.j_, size.i_); }
  void resize(const typename Msh::Edge::size_type &size)
  { Array2<Data>::resize(1, size); }
  void resize(const typename Msh::Face::size_type &size)
  { Array2<Data>::resize(size.j_, size.i_); }
  void resize(const typename Msh::Cell::size_type &size)
  { Array2<Data>::resize(1, size); }
  void resize(const size_t size)
  { Array2<Data>::resize(1,size); }

  unsigned int size() const { return this->dim1() * this->dim2(); }

  static const string type_name(int n = -1);
};


template <class Data, class Msh>
FData2d<Data, Msh>::~FData2d()
{
}


template <class Data, class Msh>
const string
FData2d<Data, Msh>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 2);
  if (n == -1)
  {
    static const string name = type_name(0) + FTNS + type_name(1) + FTNM +type_name(2) + FTNE;
    return name;
  }
  else if (n == 0)
  {
    return "FData2d";
  }
  else if (n == 1)
  {
    return find_type_name((Data *)0);
  }
  else
  {
    return find_type_name((Msh *)0);
  }
}

template <class Data, class Msh>
const TypeDescription* 
get_type_description(FData2d<Data, Msh>*)
{
  static string name(FData2d<Data, Msh>::type_name(0));
  static string namesp("SCIRun");
  static string path(__FILE__);
  const TypeDescription *sub1 = SCIRun::get_type_description((Data*)0);
  const TypeDescription *sub2 = SCIRun::get_type_description((Msh*)0);

  static TypeDescription* tdn1 = 0;
  if (tdn1 == 0) {
    TypeDescription::td_vec *subs = scinew TypeDescription::td_vec(2);
    (*subs)[0] = sub1;
    (*subs)[1] = sub2;
    tdn1 = scinew TypeDescription(name, subs, path, namesp,
				  TypeDescription::CONTAINER_E);
  }
  return tdn1;
}







template <class Data>
class FData1d : public Array1<Data> {
public:
  // import index_type
  typedef typename Array1<Data>::index_type index_type;
  typedef typename Array1<Data>::size_type  size_type;

  typedef Data value_type;
  typedef Data * iterator;
  typedef Data const * const_iterator;

  iterator begin() { return &(*this)(0); }
  iterator end() { return &((*this)(this->dim1()-1))+1; }
  const_iterator begin() const { return &(*this)(0); }
  const_iterator end() const { return &((*this)(this->dim1()-1))+1; }

  FData1d() : Array1<Data>() {}
  FData1d(int) : Array1<Data>() {} //default var sgi bug workaround.
  FData1d(const FData1d& data) { Array1<Data>::copy(data); }
  virtual ~FData1d();
  
  const value_type &operator[](index_type idx) const
  { return operator()(idx); } 
  value_type &operator[](index_type idx)
  { return operator()(idx); } 

  void resize(const size_type size)
  { Array1<Data>::resize(size); }

  size_type size() const { return this->dim1(); }

  static const string type_name(int n = -1);
};


template <class Data>
FData1d<Data>::~FData1d()
{
}


template <class Data>
const string
FData1d<Data>::type_name(int n)
{
  ASSERT((n >= -1) && n <= 2);
  if (n == -1)
  {
    static const string name = type_name(0) + FTNS + type_name(1) + FTNE;
    return name;
  }
  else if (n == 0)
  {
    return "FData1d";
  }
  else 
  {
    return find_type_name((Data *)0);
  }
}

template <class Data>
const TypeDescription* 
get_type_description(FData1d<Data>*)
{
  static string name(FData1d<Data>::type_name(0));
  static string namesp("SCIRun");
  static string path(__FILE__);
  const TypeDescription *sub1 = SCIRun::get_type_description((Data*)0);

  static TypeDescription* tdn1 = 0;
  if (tdn1 == 0) 
  {
    TypeDescription::td_vec *subs = scinew TypeDescription::td_vec(1);
    (*subs)[0] = sub1;
    tdn1 = scinew TypeDescription(name, subs, path, namesp,
				  TypeDescription::CONTAINER_E);
  }
  return tdn1;
}


} // end namespace SCIRun

#endif // Containers_FData_h
