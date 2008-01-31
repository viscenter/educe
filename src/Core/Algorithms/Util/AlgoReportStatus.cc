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

#include <Core/Algorithms/Util/AlgoReportStatus.h>

namespace SCIRunAlgo {


AlgoReportStatus::AlgoReportStatus()
{ 
  pr_ = &(default_pr_); 
}

void 
AlgoReportStatus::set_progress_reporter(SCIRun::ProgressReporter* pr)
{ 
  pr_ = pr;  
}
  
SCIRun::ProgressReporter* 
AlgoReportStatus::get_progress_reporter() const
{ 
  return (pr_); 
}
    

void 
AlgoReportStatus::error(std::string error) const
{ 
  pr_->error(error); 
}

void 
AlgoReportStatus::warning(std::string warning) const
{ 
  pr_->warning(warning); 
}

void 
AlgoReportStatus::remark(std::string remark) const
{ 
  pr_->remark(remark);
}
      
void 
AlgoReportStatus::compile_error(std::string filename) const
{ 
  pr_->compile_error(filename); 
}

void 
AlgoReportStatus::update_progress(double percent) const
{ 
  pr_->update_progress(percent); 
}

void 
AlgoReportStatus::update_progress(SCIRun::index_type current, SCIRun::size_type max) const
{ 
  pr_->update_progress(current,max); 
}

void 
AlgoReportStatus::algo_start(std::string tag, bool report_progress)
{
  // TODO: Need to relay information to user
}

void
AlgoReportStatus::algo_end()
{
  // TODO: Need to relay information to user
}

bool
AlgoReportStatus::check_abort() const
{
  // TODO : Add this hook to progress reporter
  return (false);
}

} //namespace SCIRunAlgo
