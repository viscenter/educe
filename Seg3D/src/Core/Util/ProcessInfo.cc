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
 *  ProcessInfo.cc:
 *
 *  Written by:
 *   Author: Randy Jones
 *   Department of Computer Science
 *   University of Utah
 *   Date: 2004/02/05
 *
 *  Copyright (C) 2004 SCI Group
 */

#include <Core/Util/ProcessInfo.h>

#ifndef _WIN32
#include <sys/param.h>
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#if defined( __APPLE__ )
#  include <mach/mach_init.h>
#  include <mach/task.h>
#endif


namespace SCIRun {

  bool ProcessInfo::IsSupported ( int info_type )
  {

#if defined( __linux ) || defined( __APPLE__ )

    switch ( info_type ) {
    case MEM_SIZE: return true;
    case MEM_RSS : return true;
    default      : return false;
    }

#else
    return false;
#endif

  }


  unsigned long ProcessInfo::GetInfo ( int info_type )
  {

#if defined( __linux )

    char statusFileName[MAXPATHLEN];
    sprintf( statusFileName, "/proc/%d/status", getpid() );

    FILE* file = fopen( statusFileName, "r" );

    if ( file != NULL ) {
      unsigned long tempLong = 0;
      char tempString[1024];
      const char* compareString;

      switch ( info_type ) {
      case MEM_SIZE: compareString = "VmSize:"; break;
      case MEM_RSS : compareString = "VmRSS:" ; break;
      default:
	fclose( file );
	return 0;
      }

      while ( !feof( file ) ) {
	fscanf( file, "%s", tempString );
	if ( !strcmp( tempString, compareString ) ) {
	  fscanf( file, "%ld", &tempLong );
	  fclose( file );
	  return tempLong * 1024;
	}
      }
    }

    return 0;

#elif defined( __APPLE__ )

    task_basic_info_data_t processInfo;
    mach_msg_type_number_t count;
    kern_return_t          error;

    count = TASK_BASIC_INFO_COUNT;
    error = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&processInfo, &count);

    if (error != KERN_SUCCESS) {
      return 0;
    }

    switch ( info_type ) {
    case MEM_SIZE: return processInfo.virtual_size;
    case MEM_RSS : return processInfo.resident_size;
    default:       return 0;
    }
    
    return 0;

#else
    return 0;
#endif

  } // unsigned long ProcessInfo::GetInfo ( int info_type )


} // namespace SCIRun
