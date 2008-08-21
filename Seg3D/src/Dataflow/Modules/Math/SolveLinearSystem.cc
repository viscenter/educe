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

#include <Core/Algorithms/Math/LinearSystem/SolveLinearSystem.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Math/MiscMath.h>
#include <Core/Thread/Time.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

#include <iostream>
#include <sstream>


namespace SCIRun {

using namespace SCIRunAlgo;

class SolveLinearSystem : public Module, public AlgoCallBack {

  public:
    SolveLinearSystem(GuiContext* ctx);
    virtual ~SolveLinearSystem() {}
    
    virtual void execute();
    virtual bool callback();

    GuiDouble target_error_;
    GuiDouble orig_error_;
    GuiDouble current_error_;

    GuiString method_;
    GuiString precond_;

    GuiInt iteration_;
    GuiInt maxiter_;
    
    GuiInt use_previous_solution_;
    GuiInt emit_partial_;
    GuiInt emit_iter_;

  private:
      
    int    last_iter_;
    int    next_partial_;
    int    callback_step_;
    
    SCIRunAlgo::SolveLinearSystemAlgo algo_;
    
    MatrixHandle previous_solution_;
};

bool
SolveLinearSystem::callback()
{
  // Get convergence pointer
  MatrixHandle convergence;
  algo_.get_handle("convergence",convergence);
  double* error = convergence->get_data_pointer();

  double target_error = algo_.get_scalar("target_error");
  double current_error = algo_.get_scalar("current_error");
  double orig_error = algo_.get_scalar("original_error");
  int iter = algo_.get_int("iteration");
  
  iteration_.set(iter);
  orig_error_.set(orig_error);
  current_error_.set(current_error);
  
  if (last_iter_ < 0) 
  {
    last_iter_ = 0;
    emit_iter_.reset();
    next_partial_ = emit_iter_.get();
  }  
  
  std::ostringstream str;
  str << get_id() << " append_graph " << iter << " \"";

    double s = 1.0/log(10.0);
  for (int i = last_iter_; i < iter; i++)
  {
    double err = MakeReal(error[i]);
    if (err == 0.0) err = 1e-20;
    if (err < 1000000)
      str << i << " " << s*log(error[i]) << " ";
    else
      str << i << " 1000000 ";
  }
  
  str << "\" \"";

  for (int i = last_iter_; i < iter; i++)
  {
    str << i << " " << s*log(target_error) << " ";
  }
  str << "\" ; update idletasks";
  get_gui()->execute(str.str().c_str());
  
  last_iter_ = iter;

  // Set new target
  target_error_.reset();
  algo_.set_scalar("target_error",target_error_.get());

  int callback_step = callback_step_;
    
  emit_partial_.reset();
  if (emit_partial_.get())
  {
    if (iter >= next_partial_)
    {

      // Send intermediate solutions
      MatrixHandle solution;
      algo_.get_handle("solution",solution);
      solution.detach();
      MatrixHandle num_iter = new DenseMatrix(iter);
      MatrixHandle residue = new DenseMatrix(current_error);

      send_output_handle("Solution", solution, true); 
      send_output_handle("NumIterations",num_iter, true);
      send_output_handle("Residue",residue, true);
      emit_iter_.reset();
      
      next_partial_ = iter + Max(1,emit_iter_.get());  
    }
  }
  else
  {
    next_partial_ = -1;
  }
 
  if (next_partial_ > 0)
  {
    if ((next_partial_-iter) < callback_step) callback_step = next_partial_-iter;
  }
  
  algo_.set_int("callback_step",callback_step);
  return (true);
}

DECLARE_MAKER(SolveLinearSystem)

SolveLinearSystem::SolveLinearSystem(GuiContext* ctx)
  : Module("SolveLinearSystem", ctx, Filter, "Math", "SCIRun"),
    target_error_(get_ctx()->subVar("target_error"), 0.00001),
    orig_error_(get_ctx()->subVar("orig_error"), 0.0),
    current_error_(get_ctx()->subVar("current_error"), 0.0),
    method_(get_ctx()->subVar("method"), "Conjugate Gradient & Precond. (SCI)"),
    precond_(get_ctx()->subVar("precond"), "jacobi"),
    iteration_(get_ctx()->subVar("iteration"), 0),
    maxiter_(get_ctx()->subVar("maxiter"), 500),
    use_previous_solution_(get_ctx()->subVar("use_previous_soln"), 1),
    emit_partial_(get_ctx()->subVar("emit_partial"), 1),
    emit_iter_(get_ctx()->subVar("emit_iter"), 50)
{
  algo_.set_progress_reporter(this);
}


void
SolveLinearSystem::execute()
{
  MatrixHandle matrix, rhs, x0;
 
  get_input_handle("Matrix", matrix,true);
  get_input_handle("RHS", rhs, true);

  if (use_previous_solution_.get() && 
      previous_solution_.get_rep() && rhs.get_rep() &&
      previous_solution_->nrows() == rhs->nrows())
  {
    x0 = previous_solution_;
  }
  
  last_iter_ = 0;

  algo_.set_int("max_iterations",maxiter_.get());
  algo_.set_scalar("current_error",current_error_.get());
  algo_.set_scalar("target_error",target_error_.get());
  callback_step_ = 20;
  algo_.set_int("callback_step",callback_step_);
  
  const string meth = method_.get();
  if (meth == "Conjugate Gradient & Precond. (SCI)") 
  { 
    algo_.set_option("method","cg");
  } 
  else if (meth == "BiConjugate Gradient & Precond. (SCI)") 
  {
    algo_.set_option("method","bicg");
  } 
  else if (meth == "Jacobi & Precond. (SCI)") 
  {
    algo_.set_option("method","jacobi");
  } 
  else if (meth == "MINRES & Precond. (SCI)") 
  {
    algo_.set_option("method","minres");
  }   
  else 
  {
    error("Unknown method: " + meth);
    return;
  }

  const string pre = precond_.get();
  if (pre == "jacobi")
  {
    algo_.set_option("pre_conditioner",pre);
  }
  else
  {
    algo_.set_option("pre_conditioner",pre);
  }
  
  algo_.add_callback(this);

  MatrixHandle solution;
  
  double start_time = Time::currentSeconds();
  
  get_gui()->execute(get_id() + " reset_graph");
  algo_.run(matrix,rhs,x0,solution);
  get_gui()->execute(get_id()+" finish_graph");

  previous_solution_ = solution;

  double end_time = Time::currentSeconds();

  std::ostringstream perf;
  perf << "Linear solver took " <<(end_time-start_time)<< " seconds" <<  std::endl;
  perf << "Linear solver final residue is "<< 
    algo_.get_scalar("current_error") << " after " << algo_.get_int("iteration") <<
    " iterations" << std::endl;
    
  remark(perf.str());
  
  MatrixHandle num_iter = new DenseMatrix(algo_.get_int("iteration"));
  MatrixHandle residue  = new DenseMatrix(algo_.get_scalar("current_error"));

  send_output_handle("Solution", solution, false);
  send_output_handle("NumIterations",num_iter,false);
  send_output_handle("Residue",residue,false);
}

} // End namespace SCIRun
