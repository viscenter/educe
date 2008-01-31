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
 *  Persistent.h: Base class for persistent objects...
 *
 *  Written by:
 *   Steven G. Parker
 *   Department of Computer Science
 *   University of Utah
 *   April 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

#ifndef CORE_PERSISTENT_H
#define CORE_PERSISTENT_H 1

// for storing strings and data
#include <map>
#include <string>

// for index and size types
#include <Core/Datatypes/Types.h>

#include <Core/Util/Assert.h>
#include <Core/Util/ProgressReporter.h>

#include <Core/Persistent/share.h>

namespace SCIRun {

using std::string;
using std::map;
using std::pair;

class Persistent;
class Mutex;
class Piostream;

//----------------------------------------------------------------------
struct SCISHARE PersistentTypeID {
  string type;
  string parent;
  Persistent* (*maker)();
  PersistentTypeID(const string& type, 
		   const string& parent,
		   Persistent* (*maker)(), 
		   Persistent* (*bc_maker1)() = 0, 
		   Persistent* (*bc_maker2)() = 0);
  ~PersistentTypeID();
  Persistent* (*bc_maker1)();
  Persistent* (*bc_maker2)();

  static Mutex* persistentTypeIDMutex;
};



SCISHARE Piostream* auto_istream(const string& filename,
                                 ProgressReporter *pr = 0);
SCISHARE Piostream* auto_ostream(const string& filename, const string& type,
                                 ProgressReporter *pr = 0);


//----------------------------------------------------------------------
class SCISHARE Piostream {
  
public:

  typedef map<Persistent*, int>			MapPersistentInt;
  typedef map<int, Persistent*>			MapIntPersistent;
  typedef map<string, PersistentTypeID*>	MapStringPersistentTypeID;

  enum Direction {
    Read,
    Write
  };

  enum Endian {
    Big,
    Little
  };

  static const int PERSISTENT_VERSION;
  void flag_error() { err = 1; }
  
protected:
  Piostream(Direction, int, const string &, ProgressReporter *pr);

  Direction dir;
  int version_;
  bool err;
  int file_endian;
  
  MapPersistentInt* outpointers;
  MapIntPersistent* inpointers;
  
  int current_pointer_id;

  bool have_peekname_;
  string peekname_;

  ProgressReporter *reporter_;
  bool own_reporter_;
  bool backwards_compat_id_;
  bool disable_pointer_hashing_;
  virtual void emit_pointer(int& have_data, int& pointer_id);
public:
  static bool readHeader(ProgressReporter *pr,
                         const string& filename, char* hdr,
			 const char* type, int& version, int& endian);
private:
  virtual void reset_post_header() = 0;
public:
  string file_name;

  virtual ~Piostream();

  virtual string peek_class();
  virtual int begin_class(const string& name, int current_version);
  virtual void end_class();
  virtual void begin_cheap_delim();
  virtual void end_cheap_delim();

  virtual void io(bool&);
  virtual void io(char&) = 0;
  virtual void io(signed char&) = 0;
  virtual void io(unsigned char&) = 0;
  virtual void io(short&) = 0;
  virtual void io(unsigned short&) = 0;
  virtual void io(int&) = 0;
  virtual void io(unsigned int&) = 0;
  virtual void io(long&) = 0;
  virtual void io(unsigned long&) = 0;
  virtual void io(long long&) = 0;
  virtual void io(unsigned long long&) = 0;
  virtual void io(double&) = 0;
  virtual void io(float&) = 0;
  virtual void io(string& str) = 0;
  
  
  virtual bool eof() { return false; }

  void io(Persistent*&, const PersistentTypeID&);

  bool reading() const { return dir == Read; }
  bool writing() const { return dir == Write; }
  bool error() const { return err; }

  int version() const { return version_; }
  bool backwards_compat_id() const { return backwards_compat_id_; }
  void set_backwards_compat_id(bool p) { backwards_compat_id_ = p; }
  virtual bool supports_block_io() { return false; } // deprecated, redundant.
  // Returns true if bkock_io was supported (even on error).
  virtual bool block_io(void*, size_t, size_t) { return false; }
  
  void disable_pointer_hashing() { disable_pointer_hashing_ = true; }

  SCISHARE friend Piostream* auto_istream(const string& filename,
                                 ProgressReporter *pr);
  SCISHARE friend Piostream* auto_ostream(const string& filename, const string& type,
                                 ProgressReporter *pr);
};


//----------------------------------------------------------------------
class SCISHARE Persistent {

public:
  typedef SCIRun::index_type index_type;
  typedef SCIRun::size_type  size_type;

  virtual ~Persistent();
  virtual void io(Piostream&) = 0;
};

//----------------------------------------------------------------------
inline void Pio(Piostream& stream, bool& data) { stream.io(data); }
inline void Pio(Piostream& stream, char& data) { stream.io(data); }
inline void Pio(Piostream& stream, signed char& data) { stream.io(data); }
inline void Pio(Piostream& stream, unsigned char& data) { stream.io(data); }
inline void Pio(Piostream& stream, short& data) { stream.io(data); }
inline void Pio(Piostream& stream, unsigned short& data) { stream.io(data); }
inline void Pio(Piostream& stream, int& data) { stream.io(data); }
inline void Pio(Piostream& stream, unsigned int& data) { stream.io(data); }
inline void Pio(Piostream& stream, long& data) { stream.io(data); }
inline void Pio(Piostream& stream, unsigned long& data) { stream.io(data); }
inline void Pio(Piostream& stream, long long& data) { stream.io(data); }
inline void Pio(Piostream& stream, unsigned long long& data) { stream.io(data); }
inline void Pio(Piostream& stream, double& data) { stream.io(data); }
inline void Pio(Piostream& stream, float& data) { stream.io(data); }
inline void Pio(Piostream& stream, string& data) { stream.io(data); }
inline void Pio(Piostream& stream, Persistent& data) { data.io(stream); }

SCISHARE void Pio_index(Piostream& stream, Persistent::index_type* data, 
               Persistent::size_type sz);

template<class T>
inline void Pio(Piostream& stream, T* data, Persistent::size_type sz)
{
  if (!stream.block_io(data, sizeof(T), sz))
  {
    for (Persistent::index_type i=0;i<sz;i++)
      stream.io(data[i]);
  }
}

inline void Pio_size(Piostream& stream, Persistent::size_type& size)
{
  long long temp = static_cast<long long>(size);
  stream.io(temp);
  size = static_cast<Persistent::size_type>(temp);
}

inline void Pio_index(Piostream& stream, Persistent::index_type& index)
{
  long long temp = static_cast<long long>(index);
  stream.io(temp);
  index = static_cast<Persistent::size_type>(temp);
}

} // End namespace SCIRun

#endif
