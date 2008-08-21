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
 * Torus.cc: Torus objects
 *
 *  Written by:
 *   Steven G. Parker & David Weinstein
 *   Department of Computer Science
 *   University of Utah
 *   April 1994
 *
 *  Copyright (C) 1994 SCI Group
 */

#include <Core/Geom/GeomTorus.h>
#include <Core/Geom/GeomTri.h>
#include <Core/Geometry/BBox.h>
#include <Core/Geometry/Transform.h>
#include <Core/Util/NotFinished.h>

#include <Core/Math/MiscMath.h>
#include <Core/Math/Trig.h>
#include <Core/Math/TrigTable.h>
#include <iostream>

namespace SCIRun {

using namespace std;


Persistent* make_GeomTorus()
{
    return new GeomTorus;
}

PersistentTypeID GeomTorus::type_id("GeomTorus", "GeomObj", make_GeomTorus);

GeomTorus::GeomTorus(int nu_, int nv_)
: GeomObj(), center_(0,0,0), axis_(0,0,1),
  major_radius_(1), minor_radius_(.1),
  nu_(nu_), nv_(nv_)
{
    adjust();
}

GeomTorus::GeomTorus(const Point& center, const Vector& axis,
		     double major_radius, double minor_radius,
		     int nu, int nv)
: GeomObj(), center_(center), axis_(axis),
  major_radius_(major_radius), minor_radius_(minor_radius),
  nu_(nu), nv_(nv)
{
    adjust();
}

GeomTorus::GeomTorus(const GeomTorus& copy)
: GeomObj(copy), center_(copy.center_), axis_(copy.axis_),
  major_radius_(copy.major_radius_), minor_radius_(copy.minor_radius_),
  nu_(copy.nu_), nv_(copy.nv_)
{
    adjust();
}

GeomTorus::~GeomTorus()
{
}

GeomObj* GeomTorus::clone()
{
    return new GeomTorus(*this);
}

void GeomTorus::move(const Point& center, const Vector& axis,
		     double major_radius, double minor_radius,
		     int nu, int nv)
{
    center_ = center;
    axis_ = axis;
    major_radius_ = major_radius;
    minor_radius_ = minor_radius;
    nu_ = nu;
    nv_ = nv;
    adjust();
}

void GeomTorus::adjust()
{
    axis_.normalize();

    Vector z(0,0,1);
    if(Abs(axis_.y())+Abs(axis_.x()) < 1.e-5) {
      // Only in x-z plane...
      zrotaxis_=Vector(0,-1,0);
    } else {
      zrotaxis_=Cross(axis_, z);
      zrotaxis_.normalize();
    }
    double cangle = Dot(z, axis_);
    zrotangle_ = -Acos(cangle);
}

void GeomTorus::get_bounds(BBox& bb)
{
    bb.extend_disk(center_-axis_*minor_radius_, axis_, major_radius_+minor_radius_);
    bb.extend_disk(center_+axis_*minor_radius_, axis_, major_radius_+minor_radius_);
}

#define GEOMTORUS_VERSION 1

void GeomTorus::io(Piostream& stream)
{
  stream.begin_class("GeomTorus", GEOMTORUS_VERSION);
  GeomObj::io(stream);
  Pio(stream, center_);
  Pio(stream, axis_);
  Pio(stream, major_radius_);
  Pio(stream, minor_radius_);
  Pio(stream, nu_);
  Pio(stream, nv_);
  if(stream.reading())
    adjust();
  stream.end_class();
}


// GeomTorusArc

Persistent* make_GeomTorusArc()
{
    return new GeomTorusArc;
}

PersistentTypeID GeomTorusArc::type_id("GeomTorusArc", "GeomTorus", make_GeomTorusArc);

GeomTorusArc::GeomTorusArc(int nu_, int nv_)
  : GeomTorus(nu_, nv_), zero_(0,1,0), arc_angle_(Pi)
{
}

GeomTorusArc::GeomTorusArc(const Point& center, const Vector& axis,
			   double major_radius, double minor_radius,
			   const Vector& zero,
			   double start_angle, double arc_angle,
			   int nu, int nv)
  : GeomTorus(center, axis, major_radius, minor_radius, nu, nv),
    zero_(zero), start_angle_(start_angle), arc_angle_(arc_angle)
{
}

GeomTorusArc::GeomTorusArc(const GeomTorusArc& copy)
  : GeomTorus(copy),
    zero_(copy.zero_),
    start_angle_(copy.start_angle_),
    arc_angle_(copy.arc_angle_)
{
}

GeomTorusArc::~GeomTorusArc()
{
}

GeomObj* GeomTorusArc::clone()
{
    return new GeomTorusArc(*this);
}

void GeomTorusArc::move(const Point& center, const Vector& axis,
			double major_radius, double minor_radius,
			const Vector& zero,
			double start_angle, double arc_angle,
			int nu, int nv)
{
    center_ = center;
    axis_ = axis;
    major_radius_ = major_radius;
    minor_radius_ = minor_radius;
    nu_ = nu;
    nv_ = nv;
    zero_ = zero;
    start_angle_ = start_angle;
    arc_angle_ = arc_angle;
    adjust();
}


void GeomTorusArc::adjust()
{
    axis_.normalize();
    zero_.normalize();
    yaxis_ = Cross(axis_, zero_);
}

void GeomTorusArc::get_bounds(BBox& bb)
{
    bb.extend_disk(center_-axis_*minor_radius_, axis_, major_radius_+minor_radius_);
    bb.extend_disk(center_+axis_*minor_radius_, axis_, major_radius_+minor_radius_);
}

#define GEOMTORUSARC_VERSION 1

void GeomTorusArc::io(Piostream& stream)
{
    stream.begin_class("GeomTorusArc", GEOMTORUSARC_VERSION);
    GeomTorus::io(stream);
    Pio(stream, zero_);
    Pio(stream, start_angle_);
    Pio(stream, arc_angle_);
    Pio(stream, yaxis_);
    stream.end_class();
}

} // End namespace SCIRun


