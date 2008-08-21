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

//    File   : FastLatBoundaries.cc
//    Author : Michael Callahan
//    Date   : June 2007

#include <sci_defs/teem_defs.h>
#include <Core/Datatypes/NrrdData.h>
#include <Core/Basis/TriLinearLgn.h>
#include <Core/Datatypes/TriSurfMesh.h>

#if defined(_WIN32) && !defined(BUILD_SCIRUN_STATIC)
#undef SCISHARE
#define SCISHARE __declspec(dllexport)
#else
#define SCISHARE
#endif

namespace SCIRun {

typedef TriSurfMesh<TriLinearLgn<Point> > TSMesh;


template <class T>
MeshHandle
fast_lat_sb_real(Nrrd *nrrd, T *data, bool boundary)
{
  TSMesh *mesh = new TSMesh();

  const size_t isize = nrrd->axis[0].size;
  const size_t jsize = nrrd->axis[1].size;
  const size_t ksize = nrrd->axis[2].size;

  const size_t ijsize = isize * jsize;

  for (unsigned int k = 0; k < ksize; k++)
  {
    for (unsigned int j = 0; j < jsize; j++)
    {
      for (unsigned int i = 0; i < isize; i++)
      {
        const T v = data[i + j * isize + k * ijsize];

        if (boundary && i == 0 && v != 0)
        {
          // emit i-1 -> i face
          Point p00(i, j, k);
          Point p10(i, j+1, k);
          Point p11(i, j+1, k+1);
          Point p01(i, j, k+1);
          mesh->add_triangle(p00, p01, p11);
          mesh->add_triangle(p00, p11, p10);
        }
        if (boundary && j == 0 && v != 0)
        {
          // emit j-1 -> j face
          Point p00(i, j, k);
          Point p10(i+1, j, k);
          Point p11(i+1, j, k+1);
          Point p01(i, j, k+1);
          mesh->add_triangle(p00, p01, p11);
          mesh->add_triangle(p00, p11, p10);
        }
        if (boundary && k == 0 && v != 0)
        {
          // emit k-1 -> k face
          Point p00(i, j, k);
          Point p10(i+1, j, k);
          Point p11(i+1, j+1, k);
          Point p01(i, j+1, k);
          mesh->add_triangle(p00, p01, p11);
          mesh->add_triangle(p00, p11, p10);
        }

        if (i < isize-1)
        {
          const T vi = data[(i+1) + j * isize + k * ijsize];
          if (v != vi)
          {
            // emit the i -> i+1 face
            Point p00(i+1, j, k);
            Point p10(i+1, j+1, k);
            Point p11(i+1, j+1, k+1);
            Point p01(i+1, j, k+1);
            mesh->add_triangle(p00, p01, p11);
            mesh->add_triangle(p00, p11, p10);
          }
        }
        else if (boundary && v != 0)
        {
          // emit the i -> i+1 face
          Point p00(i+1, j, k);
          Point p10(i+1, j+1, k);
          Point p11(i+1, j+1, k+1);
          Point p01(i+1, j, k+1);
          mesh->add_triangle(p00, p01, p11);
          mesh->add_triangle(p00, p11, p10);
        }

        if (j < jsize-1)
        {
          const T vj = data[i + (j+1) * isize + k * ijsize];
          if (v != vj)
          {
            // emit the j -> j+1 face
            Point p00(i, j+1, k);
            Point p10(i+1, j+1, k);
            Point p11(i+1, j+1, k+1);
            Point p01(i, j+1, k+1);
            mesh->add_triangle(p00, p01, p11);
            mesh->add_triangle(p00, p11, p10);
          }
        }
        else if (boundary && v != 0)
        {
          // emit the j -> j+1 face
          Point p00(i, j+1, k);
          Point p10(i+1, j+1, k);
          Point p11(i+1, j+1, k+1);
          Point p01(i, j+1, k+1);
          mesh->add_triangle(p00, p01, p11);
          mesh->add_triangle(p00, p11, p10);
        }

        if (k < ksize-1)
        {
          const T vk = data[i + j * isize + (k+1) * ijsize];
          if (v != vk)
          {
            // emit the k -> k+1 face
            Point p00(i, j, k+1);
            Point p10(i+1, j, k+1);
            Point p11(i+1, j+1, k+1);
            Point p01(i, j+1, k+1);
            mesh->add_triangle(p00, p01, p11);
            mesh->add_triangle(p00, p11, p10);
          }
        }
        else if (boundary && v != 0)
        {
          // emit the k -> k+1 face;
          Point p00(i, j, k+1);
          Point p10(i+1, j, k+1);
          Point p11(i+1, j+1, k+1);
          Point p01(i, j+1, k+1);
          mesh->add_triangle(p00, p01, p11);
          mesh->add_triangle(p00, p11, p10);
        }
      }
    }
  }

  return mesh;
}


SCISHARE MeshHandle
fast_lat_boundaries(Nrrd *nrrd, bool close_outside_boundary)
{
  const bool boundary = close_outside_boundary;
  switch (nrrd->type)
  {
  case nrrdTypeChar:
    return fast_lat_sb_real(nrrd, (char *)nrrd->data, boundary);
  case nrrdTypeUChar:
    return fast_lat_sb_real(nrrd, (unsigned char *)nrrd->data, boundary);
  case nrrdTypeShort:
    return fast_lat_sb_real(nrrd, (short *)nrrd->data, boundary);
  case nrrdTypeUShort:
    return fast_lat_sb_real(nrrd, (unsigned short *)nrrd->data, boundary);
  case nrrdTypeInt:
    return fast_lat_sb_real(nrrd, (int *)nrrd->data, boundary);
  case nrrdTypeUInt:
    return fast_lat_sb_real(nrrd, (unsigned int *)nrrd->data, boundary);
  case nrrdTypeLLong:
    return fast_lat_sb_real(nrrd, (long long *)nrrd->data, boundary);
  case nrrdTypeULLong:
    return fast_lat_sb_real(nrrd, (unsigned long long *)nrrd->data, boundary);
  case nrrdTypeFloat:
    return fast_lat_sb_real(nrrd, (float *)nrrd->data, boundary);
  case nrrdTypeDouble:
    return fast_lat_sb_real(nrrd, (double *)nrrd->data, boundary);
  default:
    throw "Unknown nrrd type, cannot isosurface.";
  }
}

} // end namespace SCIRun
