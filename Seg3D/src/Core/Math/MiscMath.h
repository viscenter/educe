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



#ifndef CORE_MATH_MISCMATH_H
#define CORE_MATH_MISCMATH_H 1

#include <math.h>
#include <Core/Math/share.h>

namespace SCIRun {

// Faster versions of several math functions

//------------------------------------------
// Power function

inline double Pow(double d, double p)
{
  return pow(d,p);
}


inline double Pow(double x, unsigned int p)
{
  double result=1;
  while(p)
  {
    if(p&1) result*=x;
    x*=x;
    p>>=1;
  }
  return result;
}

inline double Pow(double x, int p)
{
  if(p < 0)
  {
    p=-p;
    double result=1;
    while(p)
    {
      if(p&1)
        result*=x;
      x*=x;
      p>>=1;
    }
    return 1./result;
  } 
  else
  { 
    double result=1;
    while(p){
      if(p&1)
        result*=x;
      x*=x;
      p>>=1;
    }
    return result;
  }
}

//------------------------------------------
// Sqrt and Cbrt function

inline int Sqrt(int i)
{
  return (int)sqrt((double)i);
}

inline double Sqrt(double d)
{
  return sqrt(d);
}

inline float Sqrt(float d)
{
  return sqrtf(d);
}

inline double Cbrt(double d)
{
  return pow(d, 1.0/3.0);
}

inline float Cbrt(float d)
{
  return pow((double)d, 1.0/3.0);
}

inline double Sqr(double x)
{
  return x*x;
}

inline float Sqr(float x)
{
  return x*x;
}
  
//------------------------------------------
// Exponential function

inline double Exp(double x)
{
  return exp(x);
}

inline float Exp(float x)
{
  return expf(x);
}


//------------------------------------------
// Absolute function
inline double Abs(double d)
{
  return d<0?-d:d;
}

inline float Abs(float f)
{
  return f<0?-f:f;
}

inline int Abs(int i)
{
    return i<0?-i:i;
}

//-----------------------------------------
// Sign function
inline int Sign(double d)
{
  return d<0.0?-1:1;
}

inline int Sign(float f)
{
  return f<0.0?-1:1;
}

inline int Sign(int i)
{
  return i<0?-1:1;
}


//-----------------------------------------
// Clamp function
inline double Clamp(double d, double min, double max)
{
  return d<=min?min:d>=max?max:d;
}

inline float Clamp(float f, float min, float max)
{
  return f<=min?min:f>=max?max:f;
}

inline int Clamp(int i, int min, int max)
{
  return i<min?min:i>max?max:i;
}


//-----------------------------------------
// Generate a step between min and max.
// return:   min - if d<=min
//           max - if d>=max
//           hermite curve if d>min && d<max
//
// Make sure fixes end up in both the float and double versions.
inline double SmoothStep(double d, double min, double max)
{
  double ret;
  if(d <= min)
  {
    ret=0.0;
  } 
  else if(d >= max)
  {
    ret=1.0;
  } 
  else 
  {
    double dm=max-min;
    double t=(d-min)/dm;
    ret=t*t*(3.0-2.0*t);
  }
  return ret;
}

// Make sure fixes end up in both the float and double versions.
inline float SmoothStep(float d, float min, float max)
{
  float ret;
  if(d <= min)
  {
    ret=0;
  } 
  else if(d >= max)
  {
    ret=1;
  } 
  else 
  {
    float dm=max-min;
    float t=(d-min)/dm;
    ret=t*t*(3-2*t);
  }
  return (ret);
}


//-----------------------------------------
// Interpolation function
template <class T>
inline T Interpolate(T d1, T d2, double weight)
{
  return T(d2*weight+d1*(1.0-weight));
}

template <class T>
inline T Interpolate(T d1, T d2, float weight)
{
  return T(d2*weight+d1*(1-weight));
}


//-----------------------------------------
// Integer/double conversions
inline double Fraction(double d)
{
  if(d>0)
  {
    return d-static_cast<int>(d);
  } 
  else 
  {
    return d-static_cast<int>(d+1);
  }
}

inline float Fraction(float f)
{
  if(f>0)
  {
    return f-static_cast<int>(f);
  } 
  else 
  {
    return f-static_cast<int>(f+1);
  }
}

//-----------------------------------------
// RoundDown and RoundUp funcions

inline int RoundDown(double d)
{
  if(d>=0.0)
  {
    return static_cast<int>(d);
  } 
  else 
  {
    if(d==static_cast<int>(d))
    {
      return -(static_cast<int>(-d));
    } 
    else 
    {
      return -(static_cast<int>((-d)-1));
    }
  }
}

inline int RoundDown(float f)
{
  if(f>=0.0)
  {
    return static_cast<int>(f);
  } 
  else 
  {
    if(f==static_cast<int>(f))
    {
      return -(static_cast<int>(-f));
    } 
    else 
    {
      return -(static_cast<int>((-f)-1));
    }
  }
}

inline int RoundUp(double d)
{
  if(d>=0.0)
  {
    if((d-static_cast<int>(d)) == 0) 
    {
      return static_cast<int>(d);
    }
    else
    {
      return static_cast<int>(d+1);
    }
  } 
  else 
  {
    return static_cast<int>(d);
  }
}

inline int RoundUp(float f)
{
  if(f>=0.0)
  {
    if((f-static_cast<int>(f)) == 0.0) 
    {
      return static_cast<int>(f);
    }
    else
    {
      return static_cast<int>(f+1);
    }
  } 
  else 
  {
    return static_cast<int>(f);
  }
}

inline int Round(double d)
{
  return (int)(d+0.5);
}

inline int Round(float f)
{
  return (int)(f+0.5);
}



inline int Floor(double d)
{
  if(d<0)
  {
    int i=-(int)-d;
    if((double)i == d)
      return i;
    else
      return i-1;
  } 
  else 
  {
    return (int)d;
  }
}

inline int Floor(float f)
{
  if(f<0)
  {
    int i=-(int)-f;
    if((float)i == f)
      return i;
    else
      return i-1;
  } 
  else 
  {
    return (int)f;
  }
}

// this version is twice as fast for negative numbers
// (on an available Pentium 4 machine, only about 25%
// faster on Powerbook G4)
// than the more robust version above, but will only
//work for fabs(d) <= offset, use with caution
inline int Floor(double d, int offset)
{
  return (int)(d + offset) - offset;
}

inline int Floor(float f, int offset)
{
   return (int)(f + offset) - offset;
}

inline int Ceil(double d)
{
  if(d<0)
  {
    int i=-(int)-d;
    return i;
  } 
  else 
  {
    int i=(int)d;
    if((double)i == d)
      return i;
    else
      return i+1;
  }
}

inline int Ceil(float f)
{
  if(f<0)
  {
    int i=-(int)-f;
    return i;
  } 
  else 
  {
    int i=(int)f;
    if((float)i == f)
      return i;
    else
      return i+1;
  }
}

// using the same trick as above, this
// version of Ceil is a bit faster for
// the architectures it has been tested
// on (Pentium4, Powerbook G4)
inline int Ceil(double d, int offset)
{
  return (int)(d - offset) + offset;
}

inline int Ceil(float f, int offset)
{
   return (int)(f - offset) + offset;
}

inline int Tile(int tile, int tf)
{
  if(tf<0){
    // Tile in negative direction
      if(tile>tf && tile<=0)return 1;
      else return 0;
  } else if(tf>0){
    // Tile in positive direction
      if(tile<tf && tile>=0)return 1;
      else return 0;
  } else {
    return 1;
  }
}

double SCISHARE MakeReal(double value);

inline double getNan()
{
  double nanvalue;
  unsigned int* ntmp = reinterpret_cast<unsigned int*>(&nanvalue);
  ntmp[0] = 0xffff5a5a;
  ntmp[1] = 0xffff5a5a;
  return (nanvalue);
}


template <class T>
inline void SWAP(T& a, T& b) 
{
  T temp;
  temp = a;
  a = b;
  b = temp;
}

// Fast way to check for power of two
inline bool IsPowerOf2(unsigned int n)
{
  return (n & (n-1)) == 0;
}



// Returns a number Greater Than or Equal to dim
// that is an exact power of 2
// Used for determining what size of texture to
// allocate to store an image
inline unsigned int
Pow2(const unsigned int dim) 
{
  if (IsPowerOf2(dim)) return dim;
  unsigned int val = 4;
  while (val < dim) val = val << 1;;
  return val;
}

// Returns a number Less Than or Equal to dim
// that is an exact power of 2
inline unsigned int
LargestPowerOf2(const unsigned int dim) 
{
  if (IsPowerOf2(dim)) return dim;
  return Pow2(dim) >> 1;
}


// Returns the power of 2 of the next higher number thas is a power of 2
// Log2 of Pow2 funciton above
inline unsigned int
Log2(const unsigned int dim) 
{
  unsigned int log = 0;
  unsigned int val = 1;
  while (val < dim) { val = val << 1; log++; };
  return log;
}

// Takes the square root of "value" and tries to find two factors that
// are closest to that square root.
void findFactorsNearRoot(const int value, int& factor1, int& factor2);

} // End namespace SCIRun


#endif
