//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2007 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  License for the specific language governing rights and limitations under
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#if !defined(Interface_DataIO_DataIOHelper_h)
#define Interface_DataIO_DataIOHelper_h

#include <Core/Events/DataManager.h>
#include <Core/Util/ProgressReporter.h>

#include <string>
#include <iostream>

namespace SCIRun {

  // This is helper class that contains a collection of functions
  // useful for all the DataIO algorithms.
  class DataIOHelper {
  public:
    // Returns "Binary" or "Text" based on the input.
    static std::string streamTypeFromExportType(const std::string& export_type);
    static std::string streamTypeFromFileType(const std::string& file_type);

    // Replaces "%d" in filename with the number.  If filename doesn't
    // contain "%d", filename is unmodified.
    //
    // Returns whether filename contained "%d" or not.
    static bool numberFilename(std::string& filename, int number);

    // Loads the file into the handle.
    // Returns 0 for success.
    template<class HandleType>
    static int readFile(const std::string& filename,
                        HandleType& handle,
                        ProgressReporter* pr)
    {
      // Do some initial error checking before we do any work.
      if (filename == "") {
        std::cerr << "No filename specified.\n";
        return 1;
      }
  
      // Open the stream
      Piostream* stream = auto_istream(filename, pr);

      if (!stream) {
        std::cerr << "Error reading file '"<<filename<<"'.\n";
        return 2;
      }

      Pio(*stream, handle);

      if (!handle.get_rep() || stream->error()) {
        std::cerr << "Error reading data from file '"<<filename<<"'.\n";
        delete stream;
        return 2;
      }

      delete stream;

      return 0;
    }
    
  }; // class DataIOHelper
  
} // end namespace SCIRun

#endif // #if !defined(Interface_DataIO_DataIOHelper_h)


