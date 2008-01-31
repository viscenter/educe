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

#ifndef CORE_ALGORITHMS_UTIL_ALGOREPORTSTATUS_H
#define CORE_ALGORITHMS_UTIL_ALGOREPORTSTATUS_H 1

#include <Core/Datatypes/Types.h>
#include <Core/Util/ProgressReporter.h>

//! For Windows
#include <Core/Algorithms/Util/share.h>

namespace SCIRunAlgo {

class SCISHARE AlgoReportStatus {

  public:
  
    AlgoReportStatus();
  
    //! Functions for setting progress reporter
    //! The progress reporter relays information to other parts of the program.
    void set_progress_reporter(SCIRun::ProgressReporter* pr);
    SCIRun::ProgressReporter* get_progress_reporter() const;
    
    //! functions for the algorithm, so it can forward errors if needed
    void error(std::string error) const;
    void warning(std::string warning) const;
    void remark(std::string remark) const;
    void compile_error(std::string filename) const;

    //! Tag algorithms so we can relay this information to the user
    //! These functions tag which algorithm is currently running
    void algo_start(std::string tag, bool report_progress = false);
    void algo_end();
    
    //! Report progres of the algorithm
    void update_progress(double percent) const;
    void update_progress(SCIRun::index_type current, SCIRun::size_type max) const;

    //! Check abort status, by polling this function
    bool check_abort() const;
    
  private:
    // ProgressReporter
    SCIRun::ProgressReporter*   pr_;
    SCIRun::ProgressReporter    default_pr_;
    
};

} //namespace SCIRunAlgo

#endif
