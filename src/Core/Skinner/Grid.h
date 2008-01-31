//
//  For more information, please see: http://software.sci.utah.edu
//
//  The MIT License
//
//  Copyright (c) 2006 Scientific Computing and Imaging Institute,
//  University of Utah.
//
//
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
//    File   : Grid.h
//    Author : McKay Davis
//    Date   : Tue Jun 27 13:04:42 2006

#ifndef SKINNER_GRID_H
#define SKINNER_GRID_H

#include <Core/Skinner/Parent.h>
#include <Core/Skinner/Variables.h>

namespace SCIRun {
namespace Skinner {


class Grid : public Parent {
public:
  Grid (Variables *);
  virtual ~Grid();

  CatcherFunction_t                 process_event;
  virtual void                      set_children(const Drawables_t &);
  virtual int                       get_signal_id(const string &) const;

private:
  CatcherFunction_t                 ReLayoutCells;
  CatcherFunction_t                 do_PointerEvent;

  struct CellInfo_t{
    Var<int>        row_;
    Var<int>        col_;
    Var<double>     width_;
    Var<double>     height_;
    Var<int>        row_begin_;
    Var<int>        row_end_;
    Var<int>        col_begin_;
    Var<int>        col_end_;
    Var<bool>       resize_horizontal_;
    Var<bool>       resize_vertical_;
  };

  Var<int>                          rows_;
  Var<int>                          cols_;
  vector<CellInfo_t>                cell_info_;
  vector<double>                    cell_frac_width_;
  vector<double>                    cell_frac_height_;
  double                            resize_mx_;
  double                            resize_my_;
  int                               resize_row_;
  int                               resize_col_;
  pair<double,double>               resize_vertical_bounds_;
  pair<double,double>               resize_horizontal_bounds_;
};


}
}

#endif
