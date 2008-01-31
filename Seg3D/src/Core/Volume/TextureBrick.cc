//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
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
//    File   : TextureBrick.cc
//    Author : Milan Ikits
//    Date   : Wed Jul 14 16:03:05 2004

#include <math.h>
#include <Core/Volume/TextureBrick.h>
#include <iostream>

using std::cerr;
using std::endl;

namespace SCIRun {

const SLIVR::BBox&
convert_bbox(const SCIRun::BBox& b) {
  return *((SLIVR::BBox*)(&b));
}

TextureBrick::TextureBrick(NrrdDataHandle n0, NrrdDataHandle n1,
			   int nx, int ny, int nz, int nc, int *nb,
			   int ox, int oy, int oz,
			   int mx, int my, int mz,
			   const BBox& bbox, const BBox& tbox)
  : SLIVR::TextureBrick(n0.get_rep()->nrrd_, n1.get_rep()->nrrd_, 
		       nx, ny, nz, nc, nb, ox, oy, oz, 
		       mx, my, mz, 
		       convert_bbox(bbox), convert_bbox(tbox)),
    ref_cnt(0), 
    lock("LockingHandle lock")
{}

TextureBrick::~TextureBrick()
{}


const string
TextureBrick::type_name(int n)
{
  ASSERT((n >= -1) && n <= 1);
  if (n == -1)
  {
    static const string name = type_name(0);
    return name;

  }
  else if (n == 0)
  {
    return "TextureBrick";
  }
  else
  {
    return "";
  }
}

const TypeDescription*
TextureBrick::get_type_description(tb_td_info_e tdi) const
{
  TypeDescription* td = 0;
  static string name(type_name(0));
  static string namesp("SCIRun");
  static string path(__FILE__);

  switch (tdi) {
  default:
  case TextureBrick::FULL_TD_E:
    {
      static TypeDescription* tdn1 = 0;
      if (tdn1 == 0) {
	tdn1 = scinew TypeDescription(name, 0, path, namesp);
      } 
      td = tdn1;
    }
    break;

  case TextureBrick::TB_NAME_ONLY_E :
    {
      static TypeDescription* tdn0 = 0;
      if (tdn0 == 0) {
	tdn0 = scinew TypeDescription(name, 0, path, namesp);
      }
      td = tdn0;
    }
    break;

  case TextureBrick::DATA_TD_E :
    {
      static TypeDescription* tdnn = 0;
      td = tdnn;
    }
    break;
  };
  return td;
} 

} // end namespace SCIRun
