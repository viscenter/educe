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



#include <Core/Util/Dir.h>
#include <Core/Util/FileUtils.h>
#include <Core/Exceptions/ErrnoException.h>
#include <Core/Exceptions/InternalError.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <iostream>
#ifndef _WIN32
#include <unistd.h>
#include <dirent.h>
#else
   typedef unsigned short mode_t;
#endif

using namespace std;

using namespace SCIRun;

Dir Dir::create(const string& name)
{
   int code = MKDIR(name.c_str(), 0777);
   if(code != 0)
      throw ErrnoException("Dir::create (mkdir call)", errno, __FILE__, __LINE__);
   return Dir(name);
}

Dir Dir::current_directory()
{
	char *dirname = ::getcwd(0,0);
	std::string name = dirname;
	free(dirname);
	return Dir(name);
}

Dir::Dir()
{
}

Dir::Dir(const string& name)
   : name_(name)
{
}

Dir::Dir(const Dir& dir)
  : name_(dir.name_)
{
}

Dir::~Dir()
{
}

Dir& Dir::operator=(const Dir& copy)
{
   name_ = copy.name_;
   return *this;
}

void Dir::remove(bool throwOnError)
{
  // only removes empty dir
  int code = rmdir(name_.c_str());
  if (code != 0) {
    ErrnoException exception("Dir::remove (rmdir call)", errno, __FILE__, __LINE__);
    if (throwOnError)
      throw exception;
    else
      cerr << "WARNING: " << exception.message() << endl;
  }
  return;
}

bool
Dir::removeDir( const char * dirName )
{
  // Walk through the dir, delete files, find sub dirs, recurse, delete sub dirs, ...

  DIR *    dir = opendir( dirName );
  dirent * file = 0;

  if( dir == NULL ) {
    cout << "Error in Dir::removeDir():\n";
    cout << "  opendir failed for " << dirName << "\n";
    cout << "  - errno is " << errno << "\n";
    return false;
  }

  file = readdir(dir);
  while( file ) {
    if (strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") !=0) {
      string fullpath = string(dirName) + "/" + file->d_name;
      struct stat buf;
      stat(file->d_name, &buf);
      mode_t &m = buf.st_mode;
      if(S_ISDIR(m)) {
        removeDir( fullpath.c_str() );
      } else {
        int rc = ::remove( fullpath.c_str() );
        if (rc != 0) {
          cout << "WARNING: remove() failed for '" << fullpath.c_str()
               << "'.  Return code is: " << rc << ", errno: " << errno << ": " << strerror(errno) << "\n";
          return false;
        }
      }
    }
    file = readdir(dir);
  }

  closedir(dir);
  int code = rmdir( dirName );

  if (code != 0) {
    cerr << "Error, rmdir failed for '" << dirName << "'\n";
    cerr << "  errno is " << errno << "\n";
    return false;
  }
  return true;
}

void Dir::forceRemove(bool throwOnError)
{
   int code = deleteDir(name_);
   if (code != 0) {
     ErrnoException exception(string("Dir::remove failed to remove: ") + name_, errno, __FILE__, __LINE__);
     if (throwOnError)
       throw exception;
     else
       cerr << "WARNING: " << exception.message() << endl;
   }
   return;
}

void Dir::remove(const string& filename, bool throwOnError)
{
   string filepath = name_ + "/" + filename;
   int code = deleteFile(filepath);
   if (code != 0) {
     ErrnoException exception(string("Dir::remove failed to remove: ") + filepath, errno, __FILE__, __LINE__);
     if (throwOnError)
       throw exception;
     else
       cerr << "WARNING: " << exception.message() << endl;
   }
   return;
}

Dir Dir::createSubdir(const string& sub)
{
   return create(name_+"/"+sub);
}

Dir Dir::getSubdir(const string& sub)
{
   // This should probably do more
   return Dir(name_+"/"+sub);
}

void Dir::copy(Dir& destDir)
{
   int code = copyDir(name_, destDir.name_);
   if (code != 0)
      throw InternalError(string("Dir::copy failed to copy: ") + name_, __FILE__, __LINE__);
   return;
}

void Dir::move(Dir& destDir)
{
   int code = moveFile(name_, destDir.name_);
   if (code != 0)
      throw InternalError(string("Dir::move failed to move: ") + name_, __FILE__, __LINE__);
   return;
}

void Dir::copy(const std::string& filename, Dir& destDir)
{
   string filepath = name_ + "/" + filename;
   int code = copyFile(filepath, destDir.name_);
   if (code != 0)
      throw InternalError(string("Dir::copy failed to copy: ") + filepath, __FILE__, __LINE__);
   return;
}

void Dir::move(const std::string& filename, Dir& destDir)
{
   string filepath = name_ + "/" + filename;
   int code =moveFile(filepath, destDir.name_);
   if (code != 0)
      throw InternalError(string("Dir::move failed to move: ") + filepath, __FILE__, __LINE__);
   return;
}

void Dir::getFilenamesBySuffix(const std::string& suffix,
			       vector<string>& filenames)
{
  DIR* dir = opendir(name_.c_str());
  if (!dir) return;
  const char* ext = suffix.c_str();
  for(dirent* file = readdir(dir); file != 0; file = readdir(dir))
  {
    if ((strlen(file->d_name)>=strlen(ext)) && 
      (strcmp(file->d_name+strlen(file->d_name)-strlen(ext),ext)==0)) 
    {
      filenames.push_back(file->d_name);
      cout << "  Found " << file->d_name << endl;
    }
  }
}

bool Dir::exists()
{
  if (name_ == "/") return (true);
  struct stat buf;
  if(::stat(name_.c_str(),&buf) != 0)
  {
    return false;
  }
  
  if (S_ISDIR(buf.st_mode))
    return true;
  return false;
}

bool Dir::isfile()
{
  if (name_ == "/") return (false);
  
  struct stat buf;
  if(::stat(name_.c_str(),&buf) != 0)
  {
    return false;
  }
  if (S_ISDIR(buf.st_mode))
    return false;
  return true;
}
