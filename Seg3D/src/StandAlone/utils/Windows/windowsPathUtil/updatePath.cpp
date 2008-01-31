//
// updatePath.cpp
//
// UpdatePath.exe queries the Windows' PATH variable and returns what
// it believes are the current SCIRun and Thirdparty directories (and
// whether it believes they are optimized or debug files.  UpdatePath
// will also attempt to change the path to point to debug or optimized
// SCIRun/Thirdparty depending on the command line parameters.  This
// is useful in the event that you are running/building both a debug
// and an optimized SCIRun at (relatively) the same time.
//
// UpdatePath is designed to be run from the (cygwin) command line.
//
// Author: J. Davison de St. Germain
// Date:   Nov 7, 2007
//
// Copyright (c) 2007 - University of Utah
// SCI Institute
//

#include <windows.h>

//// For STAT:
#include <io.h>
#include <direct.h>
#include <sys/stat.h>
#define S_IRUSR 0x0100
#define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)
#define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
typedef unsigned short mode_t;
////

#include <string>
#include <vector>

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////
// Variables
bool setToDbg = false;
bool setToOpt = false;
bool beVerbose = false;
//// Location of Path in Windows registry:
//// \HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment\Path
char * envRegLoc = "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";

bool foundASCIRunDir = false;
bool foundAThirdpartyDir = false;

////////////////////////////////////////////////////////////////////////////////////////
// Functions

vector<string> split( const string & inputString );
void           verifyPath( vector<string> & pieces, HKEY & theKey );

////////////////////////////////////////////////////////////////////////////////////////

void
usage()
{
  printf( "updatePath [options]\n" );
  printf( "\n" );
  printf( "    Without any options, updatePath will let you know if it thinks the\n" );
  printf( "    current Windows PATH variable points to any debug or optimized (release)\n" );
  printf( "    SCIRun and Thirdparty directories.\n" );
  printf( "\n" );
  printf( "    --dbg  - Updates SCIRun and Thirdparty PATH to point to the debug build.\n" );
  printf( "    --opt  - Updates SCIRun and Thirdparty PATH to point to the opt (release) build.\n" );
  printf( "    --v[erbose] - Be verbose.\n" );
  printf( "\n" );
  exit( 1 );
}

void
parse_args( int argc, char *argv[] )
{
  for( int cnt = 1; cnt < argc; cnt++ ) {
    string arg = argv[cnt];
    if( arg == "--help" ) {
      usage();
    }
    else if( arg == "--verbose" || arg == "--v" ) {
      beVerbose = true;
    }
    else if( arg == "--dbg" ) {
      if( setToOpt ) {
        printf( "\n" );
        printf( "ERROR: You may only specify one of --dbg and --opt.\n" );
        printf( "\n" );
        exit( 1 );
      }
      setToDbg = true;
    }
    else if( arg == "--opt" ) {
      if( setToDbg ) {
        printf( "\n" );
        printf( "ERROR: You may only specify one of --dbg and --opt.\n" );
        printf( "\n" );
        exit( 1 );
      }
      setToOpt = true;
    }
    else {
      usage();
    }
  }
}

int
main( int argc, char *argv[] )
{
  parse_args( argc, argv );

  HKEY theKey;
  long result;

  result = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                         envRegLoc,
                         0, KEY_READ | KEY_SET_VALUE, &theKey );

  if( result != ERROR_SUCCESS ) {  
    printf( "\n" );
    printf( "Error with RegOpenKeyEx of %s.. %d\n", envRegLoc, result );  
    printf( "Goodbye.\n" );
    printf( "\n" );
    return 0;
  }

  DWORD size = 2048;
  char  * data = new char[ size ];
  DWORD type;

  result = RegQueryValueEx( theKey, "Path", 0, &type, (LPBYTE)data, &size );

  if( result == ERROR_SUCCESS ) {

    string path = data;

    if( setToOpt || setToDbg || beVerbose ) {
      printf("Original path:\n%s\n", path.c_str());
    }

    vector<string> pathPieces = split( path );

    verifyPath( pathPieces, theKey );

    RegCloseKey( theKey );
  }
  else {
    printf( "Error in trying to get path from registry...\n" );
  }


  if( setToDbg || setToOpt ) {
    printf( "\n" );
    printf( "Telling other Windows applications about this change... this could take a few seconds...\n" );
    printf( "\n" );
    printf( "IMPORTANT: You now need to restart any programs that refer to the PATH... such as Visual Studio.\n" );
    printf( "\n" );
    DWORD rc = 0;
    SendMessageTimeout( HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "Environment", SMTO_ABORTIFHUNG, 5000, &rc );
  }
  
  return 0;

} // end main()

bool
validDir( const std::string & dirname )
{
  struct stat buf;

  if( stat( dirname.c_str(), &buf) == 0 ) {
    mode_t &m = buf.st_mode;
    return (m & S_IRUSR && !S_ISREG(m) && S_ISDIR(m));
  }
  return false;
}

bool
validFile( const std::string & filename ) 
{
  struct stat buf;

  int result = stat( filename.c_str(), &buf );
  if( result == 0 )
  {
    mode_t &m = buf.st_mode;
    bool valid = m & S_IRUSR && S_ISREG(m) && !S_ISDIR(m);
    return valid;
  }
  return false;
}

void
updatePath( string & piece, const bool isOpt, const bool isDbg, const string & type, const string & result )
{
  if( isOpt ) {
    if( setToOpt ) {
      printf( "\n" );
      printf( "Looks like PATH already points to an optimized (release) %s..\n", type.c_str() );
      printf( "  Not updating... goodbye.\n" );
      printf( "\n" );
      exit( 1 );
    } else if( setToDbg ) {
      size_t pos = piece.rfind( result );
      piece.replace( pos, result.length(), "debug" );

      // Hack... 
      pos = piece.find( "-opt" );
      if( pos != string::npos ) {
        piece.replace( pos, 4, "-dbg" );
      }

      if( !validDir( piece ) ) {
        printf( "\n" );
        printf( "ERROR: The updated path (%s) is not a valid directory... Goodbye.\n", piece.c_str() );
        printf( "\n" );
        exit( 1 );
      }

    } else {
      printf( "\n" );
      printf( "Looks like PATH contains an optimized (release) %s...\n", type.c_str() ); 
      printf( "  (%s)\n", piece.c_str() );
      printf( "\n" );
      return;
    }
  }
  //////////////////////
  if( isDbg ) {
    if( setToDbg ) {
      printf( "\n" );
      printf( "Looks like PATH already points to a debug %s...\n", type.c_str() );
      printf( "  Not updating... goodbye.\n" );
      printf( "\n" );
      exit( 1 );
    } else if( setToOpt ) {
      size_t pos = piece.rfind( result );
      piece.replace( pos, result.length(), "release" );

      // Hack... 
      pos = piece.find( "-dbg" );
      if( pos != string::npos ) {
        piece.replace( pos, 4, "-opt" );
      }

      if( !validDir( piece ) ) {
        printf( "\n" );
        printf( "ERROR: The updated path (%s) is not a valid directory... Goodbye.\n", piece.c_str() );
        printf( "\n" );
        exit( 1 );
      }

    } else {
      printf( "\n" );
      printf( "Looks like PATH contains a debug %s...\n", type.c_str() ); 
      printf( "  (%s)\n", piece.c_str() );
      printf( "\n" );
    }
  }
}


void
commitPathToReg( HKEY & theKey, const vector<string> & pieces )
{
  string newPath;
  for( unsigned int cnt = 0; cnt < pieces.size(); cnt++ ) {
    newPath += pieces[ cnt ];
    if( cnt != pieces.size()-1 ) {
      newPath += ";"; // Add back in the separating ';'s
    }
  }

  printf( "\n" );
  printf( "Updated path to:\n%s\n", newPath.c_str() );

  const BYTE * data = (const BYTE *)newPath.c_str();

  long result = RegSetValueEx( theKey, "Path", 0, REG_EXPAND_SZ, data, (DWORD)newPath.length() );

  if( result != ERROR_SUCCESS ) {
    printf( "\n" );
    printf( "ERROR, RegSetValueEx failed with return code: %d\n", result );
    printf( "\n" );
    exit( 1 );
  }
}

// Looks at the path 'piece' that is passed in... If it looks like it
// is a thirdparty path, determines if it is a debug or release
// version.  Returns the string that determined if it is
// release/debug.

string
checkThirdparty( const string & piece, bool & foundOpt, bool & foundDbg )
{

  foundOpt = false;
  foundDbg = false;

  // Look for the tcl84.dll to verify that this is a valid Thirdparty bin dir...
  string filename = piece + "\\tcl84.dll";

  if( validFile( filename ) ) {

    foundAThirdpartyDir = true;

    bool opt1 = piece.find( "release" ) != string::npos;
    bool opt2 = piece.find( "Release" ) != string::npos;
    bool opt3 = piece.find( "opt" ) != string::npos;
    bool opt4 = piece.find( "Opt" ) != string::npos;
    bool dbg1 = piece.find( "debug" ) != string::npos;
    bool dbg2 = piece.find( "Debug" ) != string::npos;
    bool dbg3 = piece.find( "dbg" ) != string::npos;
    bool dbg4 = piece.find( "Dbg" ) != string::npos;

    if( ( opt1 || opt2 || opt3 || opt4 ) && ( dbg1 || dbg2 || dbg3 || dbg4 ) ) {
      printf( "\n" );
      printf( "WARNING... confusing result... could be either an opt or debug thirdparty...\n" );
      printf( "Goodbye.\n" );
      printf( "\n" );
      exit( 1 );
    }
    if( !opt1 && !opt2 && !opt3 && !opt4 && !dbg1 && !dbg2 && !dbg3 && !dbg4 ) {
      printf( "\n" );
      printf( "WARNING... could not determine if thirdparty was opt or debug...\n" );
      printf( "Goodbye.\n" );
      printf( "\n" );
      exit( 1 );
    }

    string keyWord;
    if( opt1 || opt2 || opt3 || opt4 ) {
      foundOpt = true;
      if( opt1 ) keyWord = "release";
      if( opt2 ) keyWord = "Release";
      if( opt3 ) keyWord = "opt";
      if( opt4 ) keyWord = "Opt";
    }
    if( dbg1 || dbg2 || dbg3 || dbg4 ) {
      foundDbg = true;
      if( dbg1 ) keyWord = "debug";
      if( dbg2 ) keyWord = "Debug";
      if( dbg3 ) keyWord = "dbg";
      if( dbg4 ) keyWord = "Dbg";
    }
    return keyWord;
  }
  else if( ( piece.find( "Thirdparty" ) != string::npos ) || ( piece.find( "thirdparty" ) != string::npos ) ) {
    printf( "\n" );
    printf( "Warning, the path directory (%s) contains the word 'Thirdparty',\n", piece.c_str() );
    printf( "but doesn't appear to be a valid SCIRun thirdparty directory...\n" );
    printf( "\n" );
  }
  return "";
}

// Looks at the path 'piece' that is passed in... If it looks like it
// is a thirdparty path, determines if it is a debug or release
// version.  Returns the string that determined if it is
// release/debug.

string
checkSCIRun( const string & piece, bool & foundOpt, bool & foundDbg )
{
  // Look for the Core_OS.dll to verify that this is a SCIRun bin dir...
  string filename = piece + "\\Core_OS.dll";

  if( validFile( filename ) ) {

    foundASCIRunDir = true;

    bool opt1 = piece.find( "lib\\release" ) != string::npos;
    bool dbg1 = piece.find( "lib\\debug" ) != string::npos;

    if( opt1 && dbg1 ) {
      printf( "\n" );
      printf( "WARNING... confusing result... could be either an opt or debug SCIRun...\n" );
      printf( "Goodbye.\n" );
      printf( "\n" );
      exit( 1 );
    }
    if( !opt1 && !dbg1 ) {
      printf( "\n" );
      printf( "WARNING... could not determine if SCIRun was opt or debug...\n" );
      printf( "Goodbye.\n" );
      printf( "\n" );
      exit( 1 );
    }

    string keyWord;
    if( opt1 ) {
      foundOpt = true;
      keyWord = "release";
    }
    if( dbg1 ) {
      foundDbg = true;
      keyWord = "debug";
    }
    return keyWord;
  }

  return "";
}

void
checkForSameType( const bool tmpFoundOpt, const bool tmpFoundDbg )
{
  static bool foundOpt = false;
  static bool foundDbg = false;

  if( ( foundOpt && tmpFoundDbg ) || ( foundDbg && tmpFoundOpt ) ) {
    printf( "\n" );
    printf( "ERROR: Inconsistency in path detected... Looks like the thirdparty and SCIRun\n" );
    printf( "       (debug/release version) do not match.\n" );
    printf( "Goodbye.\n" );
    printf( "\n" );
    exit( 1 );
  }
  
  foundOpt = tmpFoundOpt;
  foundDbg = tmpFoundDbg;
}

void
verifyPath( vector<string> & pieces, HKEY & theKey )
{
  if( beVerbose ) { 
    printf( "\n" ); 
    printf( "Parsing path...\n" );
    printf( "\n" ); 
  }

  for( unsigned int pos = 0; pos < pieces.size(); pos++ ) {

    if( beVerbose ) { 
      printf( "Checking: %s\n", pieces[pos].c_str() );
    }

    bool tmpFoundOpt, tmpFoundDbg;

    ////////////////////////////////////////////////////////////////
    // Determine if the PATH piece is for the thirdparty:

    string result = checkThirdparty( pieces[ pos ], tmpFoundOpt, tmpFoundDbg );

    if( result != "" ) {
      updatePath( pieces[pos], tmpFoundOpt, tmpFoundDbg, "thirdparty", result );
      checkForSameType( tmpFoundOpt, tmpFoundDbg );
    }
    else {
      string result = checkSCIRun( pieces[ pos ], tmpFoundOpt, tmpFoundDbg );
      if( result != "" ) {
        updatePath( pieces[pos], tmpFoundOpt, tmpFoundDbg, "SCIRun", result );
        checkForSameType( tmpFoundOpt, tmpFoundDbg );
      }
    }
  }

  bool updatePath = foundASCIRunDir || foundAThirdpartyDir;

  if( updatePath && ( setToOpt || setToDbg ) ) {
    commitPathToReg( theKey, pieces );
  }

  if( !foundAThirdpartyDir ) {
    printf( "\n" );
    printf( "Warning, a Thirdparty directory was not found in the path!!!\n" );
    printf( "\n" );
  }
  if( !foundASCIRunDir ) {
    printf( "\n" );
    printf( "Warning, a SCIRun directory was not found in the path!!!\n" );
    printf( "\n" );
  }
}


// Split the input string into a vector of strings by separating substrings using a ";"
vector<string>
split( const string & inputString )
{
  vector<string> result;
  size_t beg = 0;
  size_t end = inputString.find( ";" );

  while( end >= 0 && end < inputString.size() ) {

    string piece = inputString.substr( beg, end-beg );
    result.push_back( piece );
    beg = end + 1;
    end = inputString.find( ";", beg );
  }

  if( beg < inputString.size() ) {
    string piece = inputString.substr( beg );
    result.push_back( piece );
  }

  return result;
}
