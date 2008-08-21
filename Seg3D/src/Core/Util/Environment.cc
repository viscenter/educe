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

// Core SCIRun Includes

#include <Core/Util/RWS.h>
#include <Core/Util/Assert.h>
#include <Core/Util/FileUtils.h>
#include <Core/Util/sci_system.h>

// STL Includes

#include <Core/Util/Environment.h> // includes <string>
#include <iostream>
#include <fstream>
#include <map>
#include <list>


#define SCI_OK_TO_INCLUDE_SCI_ENVIRONMENT_DEFS_H
#include <sci_defs/environment_defs.h>

#include <stdio.h>
#ifndef _WIN32
#  include <unistd.h>
#  include <sys/param.h>
#else
#  define MAXPATHLEN 256
#  include <direct.h>
#  include <windows.h>
#  include <Core/Util/Dir.h>
#endif


#ifndef LOAD_PACKAGE
#  error You must set a LOAD_PACKAGE or life is pretty dull
#endif

#ifndef ITCL_WIDGETS
#  error You must set ITCL_WIDGETS to the iwidgets/scripts path
#endif

using namespace SCIRun;
using namespace std;

// This set stores all of the environemnt keys that were set when scirun was
// started. Its checked by sci_putenv to ensure we don't overwrite variables
static map<string,string> scirun_env;

// MacroSubstitute takes var_value returns a string with the environment
// variables expanded out.  Performs one level of substitution
//   Note: Must delete the returned string when you are done with it.
char*
MacroSubstitute( const char * var_value )
{
  int    cur = 0;
  int    start = 0;
  int    end = start;
  string newstring("");
  char * macro = 0;
  
  if (var_value==0)
    return 0;

  char* var_val = strdup(var_value);
  int length = strlen(var_val);
  while (cur < length-1) {
    if (var_val[cur] == '$' && var_val[cur+1]=='(') {
      cur+=2;
      start = cur;
      while (cur < length) {
        if (var_val[cur]==')') {
          end = cur-1;
          var_val[cur]='\0';
          macro = new char[end-start+2];
          sprintf(macro,"%s",&var_val[start]);
          const char *env = sci_getenv(macro);
          delete macro;
          if (env) 
            newstring += string(env);
          var_val[cur]=')';
          cur++;
          break;
        } else
          cur++;
      }
    } else {
      newstring += var_val[cur];
      cur++;
    }
  }

  newstring += var_val[cur]; // don't forget the last character!
  free(var_val);

  unsigned long newlength = strlen(newstring.c_str());
  char* retval = new char[newlength+1];
  sprintf(retval,"%s",newstring.c_str());
  
  return retval;
}

// WARNING: According to other software (specifically: tcl) you should
// lock before messing with the environment.

// Have to append 'SCIRun::' to these function names so that the
// compiler believes that they are in the SCIRun namespace (even
// though they are declared in the SCIRun namespace in the .h file...)
const char *
SCIRun::sci_getenv( const string & key )
{
  if (scirun_env.find(key) == scirun_env.end()) return 0;
  return scirun_env[key].c_str();
}

void
SCIRun::sci_putenv( const string &key, const string &val )
{
  scirun_env[key] = val;
}  


std::map<std::string,std::string>& 
SCIRun::get_sci_environment()
{
  return (scirun_env);
}


#ifdef _WIN32
void getWin32RegistryValues(string& obj, string& src, string& appdata, string& thirdparty, string& packages)
{
  // on an installed version of SCIRun, query these values from the registry, overwriting the compiled version
  // if not an installed version, return the compiled values unchanged
  HKEY software, company, scirun, pack, volatileEnvironment;
  const unsigned int PATH_SIZE = 512;
  const unsigned int NAME_SIZE = 128;

  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE", 0, KEY_READ, &software) == ERROR_SUCCESS) {
    if (RegOpenKeyEx(software, "SCI Institute", 0, KEY_READ, &company) == ERROR_SUCCESS) {
      if (RegOpenKeyEx(company, "SCIRun", 0, KEY_READ, &scirun) == ERROR_SUCCESS) {
        char data[512];
        DWORD size = 512;
        DWORD type;
        int code = RegQueryValueEx(scirun, "InstallPath", 0, &type, (LPBYTE) data, &size);
        if (type == REG_SZ && code == ERROR_SUCCESS) {
          string installDir = data;
          convertToUnixPath(installDir);
          if (installDir[installDir.length()-1] == '/')
            installDir = installDir.substr(0, installDir.length()-1);
          obj = installDir + "/bin";
          src = installDir + "/src";
          thirdparty = installDir;
          cout << "Loading SCIRun from: " << installDir << "\n";
        }

        if (RegOpenKeyEx(scirun, "Packages", 0, KEY_READ|KEY_ENUMERATE_SUB_KEYS, &pack) == ERROR_SUCCESS) {
          packages = "";
          int code = ERROR_SUCCESS;
          char name[NAME_SIZE];
          DWORD nameSize = NAME_SIZE;
          FILETIME filetime;
          int index = 0;
          for (; code == ERROR_SUCCESS; index++) {
            nameSize = 128;
            if (index > 0)
              packages = packages + name + ",";
            code = RegEnumKeyEx(pack, index, name, &nameSize, 0, 0, 0, &filetime);
          }
          // lose trailing comma
          if (index > 0 && packages[packages.length()-1] == ',')
            packages[packages.length()-1] = 0;
          cout << "Packages: " << packages << "\n";
          RegCloseKey(pack);
        }
        RegCloseKey(scirun);
      }
      RegCloseKey(company);
    }
    RegCloseKey(software);
  }

  if (RegOpenKeyEx(HKEY_CURRENT_USER, "Volatile Environment", 0, KEY_READ, &volatileEnvironment) == ERROR_SUCCESS) {
    char data[PATH_SIZE];
    DWORD size = PATH_SIZE;
    DWORD type;
    int code = RegQueryValueEx(volatileEnvironment, "APPDATA", 0, &type, (LPBYTE) data, &size);
    if (type == REG_SZ && code == ERROR_SUCCESS) {
      string userAppdata = data;
      convertToUnixPath(userAppdata);
      if (userAppdata[userAppdata.length() - 1] == '/')
        userAppdata = userAppdata.substr(0, userAppdata.length() - 1);
      appdata = userAppdata + "/SCIRun";
    }
    RegCloseKey(volatileEnvironment);
  }
}
#endif
// get_existing_env() will fill up the SCIRun::existing_env string set
// with all the currently set environment variable keys, but not their values
void
SCIRun::create_sci_environment(char **env, char *execname)
{
  if (env) {
    char **environment = env;
    scirun_env.clear();
    while (*environment) {
      const string str(*environment);
      const size_t pos = str.find("=");
      scirun_env[str.substr(0,pos)] = str.substr(pos+1, str.length());
      environment++;
    }
  }

  string executable_name = "scirun";
  string objdir = SCIRUN_OBJDIR;
  string srcdir = SCIRUN_SRCDIR;
  string thirdpartydir = SCIRUN_THIRDPARTY_DIR;
  string packages = LOAD_PACKAGE;
  string iwidgets = ITCL_WIDGETS;
  string appdata;

#if defined(SCIRUN_PACKAGE_SRC_PATH)
  string pkg_src_path = SCIRUN_PACKAGE_SRC_PATH;
#endif
#if defined(SCIRUN_PACKAGE_LIB_PATH)
  string pkg_lib_path = SCIRUN_PACKAGE_LIB_PATH;
#endif

#ifdef _WIN32
  getWin32RegistryValues(objdir, srcdir, appdata, thirdpartydir, packages);
  if (thirdpartydir != SCIRUN_THIRDPARTY_DIR) {
    iwidgets = thirdpartydir + "/lib/iwidgets/scripts";
  }

  if (!validDir(appdata)) {
    Dir d = Dir::create(appdata);
    if (!d.exists()) {
      std::cerr << "The following directory could not be created " << appdata << "\n";
    }
    else {
      if (!validDir(appdata)) {
        std::cerr << "The following directory does not exist " << appdata << "\n";
      }
    }
  }
#endif
  if (!sci_getenv("SCIRUN_OBJDIR")) 
    {
      if (!execname)
        sci_putenv("SCIRUN_OBJDIR", objdir);
      else {
        string objdir(execname);
        const char *path = sci_getenv("PATH");
        if (execname[0] != '/' && execname[1] != ':') { // unix and windows compatible
          if (objdir.find("/") == string::npos && 
              objdir.find("\\") == string::npos && 
              path) {
            objdir = findFileInPath(execname, path);
            ASSERT(objdir.length());
          } else {
            char cwd[MAXPATHLEN];
            getcwd(cwd,MAXPATHLEN);
            objdir = cwd+string("/")+objdir;
          }
        }

        string::size_type pos = objdir.find_last_of('/');
        if (pos == string::npos)
          pos = objdir.find_last_of('\\');

        executable_name = objdir.substr(pos+1, objdir.size()-pos-1);;
        objdir.erase(objdir.begin()+pos+1, objdir.end());

        sci_putenv("SCIRUN_OBJDIR",  objdir);
      }
    }

  if (!sci_getenv("SCIRUN_SRCDIR"))
    sci_putenv("SCIRUN_SRCDIR", srcdir);
  if (!sci_getenv("SCIRUN_THIRDPARTY_DIR"))
    sci_putenv("SCIRUN_THIRDPARTY_DIR", thirdpartydir);
  if (!sci_getenv("SCIRUN_LOAD_PACKAGE"))
    sci_putenv("SCIRUN_LOAD_PACKAGE", packages);
  if (!sci_getenv("SCIRUN_ITCL_WIDGETS"))
    sci_putenv("SCIRUN_ITCL_WIDGETS", iwidgets);
  
#if defined(SCIRUN_PACKAGE_SRC_PATH)
  if(!sci_getenv("SCIRUN_PACKAGE_SRC_PATH") &&  pkg_src_path != "")
    sci_putenv("SCIRUN_PACKAGE_SRC_PATH", pkg_src_path);
#endif
#if defined(SCIRUN_PACKAGE_LIB_PATH)
  if(!sci_getenv("SCIRUN_PACKAGE_LIB_PATH") && pkg_lib_path != "")
    sci_putenv("SCIRUN_PACKAGE_LIB_PATH", pkg_lib_path);
#endif

  if (!sci_getenv("SCIRUN_TMP_DIR"))
    sci_putenv("SCIRUN_TMP_DIR", "/tmp/");

  // native windows doesn't have "HOME"
  if (!sci_getenv("HOME")) {
    // Use appropriate AppData directory (VISTA permissions) instead of OBJTOP (previous versions did this)
    sci_putenv("HOME", appdata);
  }

  sci_putenv("SCIRUN_ITCL_WIDGETS", 
             MacroSubstitute(sci_getenv("SCIRUN_ITCL_WIDGETS")));

  sci_putenv("EXECUTABLE_NAME", executable_name);
  string rcfile = "." + executable_name + "rc";
  find_and_parse_rcfile(rcfile);
}

// emptryOrComment returns true if the 'line' passed in is a comment
// ie: the first non-whitespace character is a '#'
// or if the entire line is empty or white space.
bool
emptyOrComment( const char * line )
{
  const char A_TAB = 9; // 9 is the ascii value of TAB
  int   length = (int)strlen( line );

  for( int pos = 0; pos < length; pos++ ) {
    if( line[pos] == '#' ) {
      return true;
    } else if( ( line[pos] != ' ' ) && ( line[pos] != A_TAB ) ) {
      return false;
    }
  }
  return true;
}

// parse_rcfile reads the file 'rcfile' into SCIRuns enviroment mechanism
// It uses sci_putenv to set variables in the environment. 
// Returns true if the file was opened and parsed.  False otherwise.
bool
SCIRun::parse_rcfile( const string &rcfile )
{
  FILE* filein = fopen(rcfile.c_str(),"r");
  if (!filein) return false;

  char var[0xff];
  char var_val[0xffff];
  bool done = false;
  int linenum = 0;

  while( !done ) {
    linenum++;
    var[0]='\0';
    var_val[0]='\0';

    char line[1024];
    // If we get to the EOF:
    if( !fgets( line, 1023, filein ) ) break;

    int length = (int)strlen(line);
    if( line[length-1] == '\n' || line[length-1] == '\r' ) 
    {
      // Replace CR with EOL.
      line[length-1] = 0;
    }
    if( line[length-2] == '\n' || line[length-2] == '\r' ) 
    {
      // Replace CR with EOL.
      line[length-2] = 0;
    }

      
    // Skip commented out lines or blank lines
    if( emptyOrComment( line ) ) continue;

    // Get the environment variable and its value
    if( sscanf( line, "%[^=]=%s", var, var_val ) == 2 ){
      if (var[0]!='\0' && var_val[0]!='\0') {
        removeLTWhiteSpace(var);
        removeLTWhiteSpace(var_val);
        char* sub = MacroSubstitute(var_val);

        // Only put the var into the environment if it is not already there.
        if(!SCIRun::sci_getenv( var ) || 
           // Except the .scirunrc version, it should always come from the file
           string(var) == string("SCIRUN_RCFILE_VERSION")) {
          sci_putenv(var,sub);
        } 

        delete[] sub;
      }
    } else { // Couldn't find a string of the format var=var_val
      // Print out the offending line
      cerr << "Error parsing " << rcfile << " file on line: " 
           << linenum <<  "\n--> " << line << "\n";
    }
  }
  fclose(filein);
  sci_putenv("SCIRUN_RC_PARSED","1");
  sci_putenv("SCIRUN_RCFILE",rcfile);
  
  return true;
}

// find_and_parse_rcfile will search for the rcfile file in 
// default locations and read it into the environemnt if possible.
void
SCIRun::find_and_parse_rcfile(const string &rcfile)
{
  bool foundrc=false;
  const string slash("/");

  // 1. check the local directory
  string filename(rcfile);
  foundrc = parse_rcfile(filename);
  
  // 2. check the BUILD_DIR
  if (!foundrc) {
    filename = SCIRUN_OBJDIR + slash + string(rcfile);
    foundrc = parse_rcfile(filename);
  }
  
  // 3. check the user's home directory
  const char *HOME = NULL;
  if (!foundrc && (HOME = sci_getenv("HOME"))) {
    filename = HOME + slash + string(rcfile);
    foundrc = parse_rcfile(filename);
  }
  
  // 4. check the source code directory
  if (!foundrc) {
    filename = SCIRUN_SRCDIR + slash + string(rcfile);
    foundrc = parse_rcfile(filename);
  }

  if (foundrc) sci_putenv("SCIRUN_RCFILE",filename);

  // Tell the user that we parsed the the rcfile...
  std::cout << "Parsed " << rcfile << "... " << filename << "\n";
}


bool
SCIRun::update_rcfile(std::string key, std::string value)
{
  std::string filename = sci_getenv("SCIRUN_RCFILE");
  if (filename.size() == 0) return (false);
  
   std::ifstream file;

  try
  {
    file.open(filename.c_str());
  }
  catch(...)
  {
    return (false);
  }

  std::string line;
  std::list<std::string> lines;
  
  bool found = false;
        
  while(!file.eof())
  { 
    std::getline(file,line);
    size_t startloc = 0;
    while (startloc < line.size() && ((line[startloc] == ' ')||(line[startloc] == '\t'))) startloc++;
    size_t equalloc = line.find("=");
    if (equalloc != std::string::npos)
    {
      std::string keyname = line.substr(startloc,equalloc-startloc);
      size_t endloc = equalloc-startloc-1;
      while (endloc >= 0 && ((keyname[endloc] == ' ')||(keyname[endloc] == '\t')) ) endloc--;
      keyname = keyname.substr(0,endloc+1);
      if ((keyname == key) || (keyname == ("# "+key)))
      {
        line = key + " = " + value;
        found = true;
      }
    }
    line += "\n";
    lines.push_back(line);
  }
  file.close();
  
  if (found == false)
  {
    line = key + "=" + value + "\n";
    lines.push_back(line);
  }
  
  std::ofstream nfile;
  
  try
  {
    nfile.open(filename.c_str());
  }
  catch(...)
  {
    return (false);
  }

  std::list<std::string>::iterator it, it_end;
  it = lines.begin();
  it_end = lines.end();
  
  while(it != it_end)
  {
    nfile << (*it); ++it;
  }

  nfile.close();
  return (true);
}


void
SCIRun::copy_and_parse_scirunrc()
{
  string home   = sci_getenv("HOME");
  string srcdir = sci_getenv("SCIRUN_SRCDIR");

  if( home == "" || srcdir == "" ) {
    cout << "Warning: HOME ('" << home << "') or SCIRUN_SRCDIR ('" << srcdir << "') invalid... skipping copy...\n";
    return;
  }

  string srcRc = srcdir + "/scirunrc";

  string homerc = home + "/.scirunrc";
  if (validFile(homerc)) {
    const char* env_rcfile_version = sci_getenv("SCIRUN_RCFILE_VERSION");
    string backup_extension =(env_rcfile_version ? env_rcfile_version:"bak");
    string backuprc = homerc + "." + backup_extension;

    std::cout << "Backing up " << homerc << " to " << backuprc << endl;
    copyFile(homerc, backuprc);
  }

  std::cout << "Copying " << srcRc << " to " << homerc << "\n";
  if (copyFile(srcRc, homerc) == 0) 
  {
    // If the scirunrc file was copied, then parse it.
    parse_rcfile(homerc);
  }
}

// sci_getenv_p will lookup the value of the environment variable 'key' and 
// returns false if the variable is equal to 'false', 'no', 'off', or '0'
// returns true otherwise.  Case insensitive.
bool
SCIRun::sci_getenv_p(const string &key) 
{
  const char *value = sci_getenv(key);

  // If the environment variable does NOT EXIST OR is EMPTY then return FASE
  if (!value || !(*value)) return false;
  string str;
  while (*value) {
    str += toupper(*value);
    value++;
  }

  // Only return false if value is zero (or equivalant)
  if (str == "FALSE" || str == "NO" || str == "OFF" || str == "0")
    return false;
  // Following C convention where any non-zero value is assumed true
  return true;
}

