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


/* FileUtils.h 
 * 
 * written by 
 *   Chris Moulding
 *   Sept 2000
 *   Copyright (c) 2000
 *   University of Utah
 */

#ifndef FILEUTILS_H
#define FILEUTILS_H 1

#include <map>
#include <string>
#include <vector>

using std::string;
using std::vector;

#include <Core/Util/share.h>

namespace SCIRun {

////////////////////////////////////
// InsertStringInFile()
// Inserts "insert" in front of all occurrances of 
// "match" within the file named "filename"

SCISHARE void InsertStringInFile(char* filename, const char* match, const char* insert);


////////////////////////////////////
// GetFilenamesEndingWith()
// returns a std::map of strings that contains
// all the files with extension "ext" inside
// the directory named "dir"

SCISHARE std::map<int,char*>* GetFilenamesEndingWith(char* dir, char* ext);

SCISHARE vector<string> GetFilenamesStartingWith(const string & dir,
                                                 const string & prefix);

// Return all of the filenames that appear to be in the same numeric
// sequence as the reference one.  This means all the letters in each
// are the same and only the numbers are different.  No sorting is done.
SCISHARE vector<string> GetFilenamesInSequence(const string &dir,
                                               const string &ref);

SCISHARE std::pair<string, string> split_filename(string fname);

SCISHARE std::string findFileInPath(const std::string &filename, 
                                    const std::string &path);

SCISHARE bool getInfo( const string & filename );  // prints out size, type, timestamp, etc about the file. Returns false if file does not exist.
SCISHARE bool validFile( const std::string & filename );
SCISHARE bool validDir( const std::string & filename );
SCISHARE bool isSymLink( std::string filename );

// Creates a temp file (in directoryPath), writes to it, checks the resulting files size, and then deletes it...
SCISHARE bool testFilesystem( std::string directoryPath );

SCISHARE string autocomplete(const string &);
SCISHARE string canonicalize(string);
SCISHARE string substituteTilde(const string &dirstr);

// Replaces '/' with '\' or vice-versa between unix and windows paths
SCISHARE void convertToWindowsPath( string & unixPath );
SCISHARE void convertToUnixPath( string & winPath );

// System copy, move, and delete commands.  The strings are not
// references since windows has to convert '/' to '\\', and we do that
// in the same string
SCISHARE int copyFile(string src, string dest);
SCISHARE int moveFile(string src, string dest);
SCISHARE int deleteFile(string filename);
SCISHARE int copyDir(string src, string dest);
SCISHARE int deleteDir(string filename);

// Replaces the existing extension of the filename with the value of ext
SCISHARE string changeExtension(string filename, const string &ext);

} // End namespace SCIRun

#ifdef _WIN32
// windows doesn't have dirent... make them here
struct dirent
{
    char *d_name;
};

struct DIR;

SCISHARE DIR *opendir(const char *);
SCISHARE int closedir(DIR *);
SCISHARE dirent *readdir(DIR *);

// not implemented yet...
SCISHARE void rewinddir(DIR *);
#endif

#endif

