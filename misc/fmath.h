/* fmath.h
 * Definitions for the show structures and functions
 *
 * Modification history:
 * 2-1-95     Garrick Meeker              Created
 *
 */

#ifndef _CC_FMATH_H_
#define _CC_FMATH_H_

#ifdef __GNUG__
#pragma interface
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef bool
typedef unsigned short bool;
#endif

class Fixfloat {
public:
  Fixfloat();
  Fixfloat(const Fixfloat& a);
  Fixfloat(unsigned short a, int convert);
  Fixfloat(double a);
  ~Fixfloat();

  unsigned short value() const;

  operator unsigned short ();
  operator double ();

  Fixfloat& operator = (const Fixfloat& y);
  Fixfloat& operator = (unsigned short y);
  Fixfloat& operator = (double y);

  Fixfloat& operator += (const Fixfloat& y);
  Fixfloat& operator += (unsigned short y);
  Fixfloat& operator -= (const Fixfloat& y);
  Fixfloat& operator -= (unsigned short y);
  Fixfloat& operator *= (const Fixfloat& y);
  Fixfloat& operator *= (unsigned short y);
  Fixfloat& operator /= (const Fixfloat& y); 
  Fixfloat& operator /= (unsigned short y); 

private:
  unsigned short val;
};

inline Fixfloat operator + (const Fixfloat& x, const Fixfloat& y);
inline Fixfloat operator + (const Fixfloat& x, unsigned short y);
inline Fixfloat operator + (unsigned short x, const Fixfloat& y);
inline Fixfloat operator - (const Fixfloat& x, const Fixfloat& y);
inline Fixfloat operator - (const Fixfloat& x, unsigned short y);
inline Fixfloat operator - (unsigned short x, const Fixfloat& y);
inline Fixfloat operator * (const Fixfloat& x, const Fixfloat& y);
inline Fixfloat operator * (const Fixfloat& x, unsigned short y);
inline Fixfloat operator * (unsigned short x, const Fixfloat& y);
inline Fixfloat operator / (const Fixfloat& x, const Fixfloat& y);
inline Fixfloat operator / (const Fixfloat& x, unsigned short y);
inline Fixfloat operator / (unsigned short x, const Fixfloat& y);

inline int operator == (const Fixfloat& x, const Fixfloat& y);
inline int operator == (const Fixfloat& x, unsigned short y);
inline int operator != (const Fixfloat& x, const Fixfloat& y);
inline int operator != (const Fixfloat& x, unsigned short y);
inline int operator < (const Fixfloat& x, const Fixfloat& y);
inline int operator < (const Fixfloat& x, unsigned short y);
inline int operator <= (const Fixfloat& x, const Fixfloat& y);
inline int operator <= (const Fixfloat& x, unsigned short y);
inline int operator > (const Fixfloat& x, const Fixfloat& y);
inline int operator > (const Fixfloat& x, unsigned short y);
inline int operator >= (const Fixfloat& x, const Fixfloat& y);
inline int operator >= (const Fixfloat& x, unsigned short y);

// inline members

inline unsigned short Fixfloat::value() const { return val; }

inline Fixfloat::Fixfloat() {}
inline Fixfloat::Fixfloat(const Fixfloat& y) :val(y.value()) {}
inline Fixfloat::Fixfloat(unsigned short n, int convert) {
  if (convert) val = (n << 8);
  else val = n;
}
inline Fixfloat::Fixfloat(double n) :val((unsigned short)(n*256)) {}

inline Fixfloat::~Fixfloat() {}

inline Fixfloat::operator unsigned short () {
  return (val >> 8);
}
inline Fixfloat::operator double () {
  return (val / 256.0);
}

inline Fixfloat& Fixfloat::operator = (const Fixfloat& y) {
  val = y.value(); return *this; 
} 
inline Fixfloat& Fixfloat::operator = (unsigned short y) {
  val = y << 8; return *this; 
} 
inline Fixfloat& Fixfloat::operator = (double y) {
  val = (unsigned short)(y / 256.0); return *this; 
} 

inline Fixfloat& Fixfloat::operator += (const Fixfloat& y) {
  val += y.value(); return *this; 
}
inline Fixfloat& Fixfloat::operator += (unsigned short y) {
  val += (y << 8); return *this;
}
inline Fixfloat& Fixfloat::operator -= (const Fixfloat& y) {
  val -= y.value(); return *this; 
}
inline Fixfloat& Fixfloat::operator -= (unsigned short y) {
  val -= (y << 8); return *this; 
}
inline Fixfloat& Fixfloat::operator *= (const Fixfloat& y) {
  unsigned short n = (val * y.value()) >> 8;
  val = n;
  return *this;
}
inline Fixfloat& Fixfloat::operator *= (unsigned short y) {
  val *= y; return *this;
}
inline Fixfloat& Fixfloat::operator /= (const Fixfloat& y) {
  unsigned short n = (val << 8) / y.value();
  val = n;
  return *this;
}
inline Fixfloat& Fixfloat::operator /= (unsigned short y) {
  val /= y; return *this;
}

inline Fixfloat operator + (const Fixfloat& x, const Fixfloat& y) {
  return Fixfloat(x.value() + y.value(), FALSE);
}
inline Fixfloat operator + (const Fixfloat& x, unsigned short y) {
  return Fixfloat(x.value() + (y << 8), FALSE);
}
inline Fixfloat operator + (unsigned short x, const Fixfloat& y) {
  return Fixfloat((x << 8) + y.value(), FALSE);
}
inline Fixfloat operator - (const Fixfloat& x, const Fixfloat& y) {
  return Fixfloat(x.value() - y.value(), FALSE);
}
inline Fixfloat operator - (const Fixfloat& x, unsigned short y) {
  return Fixfloat(x.value() - (y << 8), FALSE);
}
inline Fixfloat operator - (unsigned short x, const Fixfloat& y) {
  return Fixfloat((x << 8) - y.value(), FALSE);
}
inline Fixfloat operator * (const Fixfloat& x, const Fixfloat& y) {
  return Fixfloat((x.value() * y.value()) >> 8, FALSE);
}
inline Fixfloat operator * (const Fixfloat& x, unsigned short y) {
  return Fixfloat(x.value() * y, FALSE);
}
inline Fixfloat operator * (unsigned short x, const Fixfloat& y) {
  return Fixfloat(x * y.value(), FALSE);
}
inline Fixfloat operator / (const Fixfloat& x, const Fixfloat& y) {
  return Fixfloat((x.value() << 8) / y.value(), FALSE);
}
inline Fixfloat operator / (const Fixfloat& x, unsigned short y) {
  return Fixfloat(x.value() / y, FALSE);
}
inline Fixfloat operator / (unsigned short x, const Fixfloat& y) {
  return Fixfloat((x << 16) / y.value(), FALSE);
}
inline int operator == (const Fixfloat& x, const Fixfloat& y) {
  return x.value() == y.value();
}
inline int operator == (const Fixfloat& x, unsigned short y) {
  return x.value() == (y << 8);
}
inline int operator != (const Fixfloat& x, const Fixfloat& y) {
  return x.value() != y.value();
}
inline int operator != (const Fixfloat& x, unsigned short y) {
  return x.value() != (y << 8);
}
inline int operator < (const Fixfloat& x, const Fixfloat& y) {
  return x.value() < y.value();
}
inline int operator < (const Fixfloat& x, unsigned short y) {
  return x.value() < (y << 8);
}
inline int operator <= (const Fixfloat& x, const Fixfloat& y) {
  return x.value() <= y.value();
}
inline int operator <= (const Fixfloat& x, unsigned short y) {
  return x.value() <= (y << 8);
}
inline int operator > (const Fixfloat& x, const Fixfloat& y) {
  return x.value() > y.value();
}
inline int operator > (const Fixfloat& x, unsigned short y) {
  return x.value() > (y << 8);
}
inline int operator >= (const Fixfloat& x, const Fixfloat& y) {
  return x.value() >= y.value();
}
inline int operator >= (const Fixfloat& x, unsigned short y) {
  return x.value() >= (y << 8);
}

#endif
