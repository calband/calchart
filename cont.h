/* cont.h
 * Classes for continuity
 *
 * Modification history:
 * 12-29-95   Garrick Meeker              Created
 *
 */

#ifndef _CONT_H_
#define _CONT_H_

#ifdef __GNUG__
#pragma interface
#endif

#include "animate.h"

enum ContDefinedValue { CC_N, CC_NW, CC_W, CC_SW, CC_S, CC_SE, CC_E, CC_NE,
			CC_HS, CC_MM, CC_SH, CC_JS, CC_GV, CC_M, CC_DM };

class ContToken {
public:
  ContToken();
  virtual ~ContToken();
  int line, col;
};

class ContPoint: public ContToken {
public:
  ContPoint() {}
  virtual ~ContPoint();

  virtual const CC_coord& Get(AnimateCompile* anim) const;
};

class ContStartPoint : public ContPoint {
public:
  ContStartPoint() {}

  virtual const CC_coord& Get(AnimateCompile* anim) const;
};

class ContNextPoint : public ContPoint {
public:
  ContNextPoint() {}

  virtual const CC_coord& Get(AnimateCompile* anim) const;
};

class ContRefPoint : public ContPoint {
public:
  ContRefPoint(unsigned n): refnum(n) {}

  virtual const CC_coord& Get(AnimateCompile* anim) const;
private:
  unsigned refnum;
};

class ContValue: public ContToken {
public:
  ContValue() {}
  virtual ~ContValue();

  virtual float Get(AnimateCompile* anim) const = 0;
};

class ContValueFloat : public ContValue {
public:
  ContValueFloat(float v): val(v) {}

  virtual float Get(AnimateCompile* anim) const;
private:
  float val;
};

class ContValueDefined : public ContValue {
public:
  ContValueDefined(ContDefinedValue v): val(v) {}

  virtual float Get(AnimateCompile* anim) const;
private:
  ContDefinedValue val;
};

class ContValueAdd : public ContValue {
public:
  ContValueAdd(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
  virtual ~ContValueAdd();

  virtual float Get(AnimateCompile* anim) const;
private:
  ContValue *val1, *val2;
};

class ContValueSub : public ContValue {
public:
  ContValueSub(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
  virtual ~ContValueSub();

  virtual float Get(AnimateCompile* anim) const;
private:
  ContValue *val1, *val2;
};

class ContValueMult : public ContValue {
public:
  ContValueMult(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
  virtual ~ContValueMult();

  virtual float Get(AnimateCompile* anim) const;
private:
  ContValue *val1, *val2;
};

class ContValueDiv : public ContValue {
public:
  ContValueDiv(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
  virtual ~ContValueDiv();

  virtual float Get(AnimateCompile* anim) const;
private:
  ContValue *val1, *val2;
};

class ContValueNeg : public ContValue {
public:
  ContValueNeg(ContValue *v) : val(v) {}
  virtual ~ContValueNeg();

  virtual float Get(AnimateCompile* anim) const;
private:
  ContValue *val;
};

class ContValueREM : public ContValue {
public:
  virtual float Get(AnimateCompile* anim) const;
};

class ContValueVar : public ContValue {
public:
  ContValueVar(unsigned num): varnum(num) {}
  
  virtual float Get(AnimateCompile* anim) const;
  void Set(AnimateCompile* anim, float v);
private:
  unsigned varnum;
};

class ContFuncDir : public ContValue {
public:
  ContFuncDir(ContPoint *p): pnt(p) {}
  virtual ~ContFuncDir();

  virtual float Get(AnimateCompile* anim) const;
private:
  ContPoint *pnt;
};

class ContFuncDirFrom : public ContValue {
public:
  ContFuncDirFrom(ContPoint *start, ContPoint *end)
    : pnt_start(start), pnt_end(end) {}
  virtual ~ContFuncDirFrom();

  virtual float Get(AnimateCompile* anim) const;
private:
  ContPoint *pnt_start, *pnt_end;
};

class ContFuncDist : public ContValue {
public:
  ContFuncDist(ContPoint *p): pnt(p) {}
  virtual ~ContFuncDist();

  virtual float Get(AnimateCompile* anim) const;
private:
  ContPoint *pnt;
};

class ContFuncDistFrom : public ContValue {
public:
  ContFuncDistFrom(ContPoint *start, ContPoint *end)
    : pnt_start(start), pnt_end(end) {}
  virtual ~ContFuncDistFrom();

  virtual float Get(AnimateCompile* anim) const;
private:
  ContPoint *pnt_start, *pnt_end;
};

class ContFuncEither : public ContValue {
public:
  ContFuncEither(ContValue *d1, ContValue *d2, ContPoint *p)
    : dir1(d1), dir2(d2), pnt(p) {}
  virtual ~ContFuncEither();

  virtual float Get(AnimateCompile* anim) const;
private:
  ContValue *dir1, *dir2;
  ContPoint *pnt;
};

class ContFuncOpp : public ContValue {
public:
  ContFuncOpp(ContValue *d): dir(d) {}
  virtual ~ContFuncOpp();

  virtual float Get(AnimateCompile* anim) const;
private:
  ContValue *dir;
};

class ContFuncStep : public ContValue {
public:
  ContFuncStep(ContValue *beats, ContValue *blocksize, ContPoint *p)
    : numbeats(beats), blksize(blocksize), pnt(p) {}
  virtual ~ContFuncStep();

  virtual float Get(AnimateCompile* anim) const;
private:
  ContValue *numbeats, *blksize;
  ContPoint *pnt;
};

class ContProcedure: public ContToken {
public:
  ContProcedure(): next(NULL) {}
  virtual ~ContProcedure();

  virtual void Compile(AnimateCompile* anim) = 0;

  ContProcedure *next;
};

class ContProcSet : public ContProcedure {
public:
  ContProcSet(ContValueVar *vr, ContValue *v)
    : var(vr), val(v) {}
  virtual ~ContProcSet();

  virtual void Compile(AnimateCompile* anim);

private:
  ContValueVar *var;
  ContValue *val;
};

class ContProcBlam : public ContProcedure {
public:
  virtual void Compile(AnimateCompile* anim);
};

class ContProcCM : public ContProcedure {
public:
  ContProcCM(ContPoint *p1, ContPoint *p2, ContValue *steps, ContValue *d1,
	     ContValue *d2, ContValue *beats)
    : pnt1(p1), pnt2(p2), stps(steps), dir1(d1), dir2(d2), numbeats(beats) {}
  virtual ~ContProcCM();

  virtual void Compile(AnimateCompile* anim);

private:
  ContPoint *pnt1, *pnt2;
  ContValue *stps, *dir1, *dir2, *numbeats;
};

class ContProcDMCM : public ContProcedure {
public:
  ContProcDMCM(ContPoint *p1, ContPoint *p2, ContValue *beats)
    : pnt1(p1), pnt2(p2), numbeats(beats) {}
  virtual ~ContProcDMCM();

  virtual void Compile(AnimateCompile* anim);

private:
  ContPoint *pnt1, *pnt2;
  ContValue *numbeats;
};

class ContProcDMHS : public ContProcedure {
public:
  ContProcDMHS(ContPoint *p)
    : pnt(p) {}
  virtual ~ContProcDMHS();

  virtual void Compile(AnimateCompile* anim);

private:
  ContPoint *pnt;
};

class ContProcEven : public ContProcedure {
public:
  ContProcEven(ContValue *steps, ContPoint *p)
    : stps(steps), pnt(p) {}
  virtual ~ContProcEven();

  virtual void Compile(AnimateCompile* anim);

private:
  ContValue *stps;
  ContPoint *pnt;
};

class ContProcEWNS : public ContProcedure {
public:
  ContProcEWNS(ContPoint *p)
    : pnt(p) {}
  virtual ~ContProcEWNS();

  virtual void Compile(AnimateCompile* anim);

private:
  ContPoint *pnt;
};

class ContProcFountain : public ContProcedure {
public:
  ContProcFountain(ContValue *d1, ContValue *d2, ContValue *s1, ContValue *s2,
		   ContPoint *p)
    : dir1(d1), dir2(d2), stepsize1(s1), stepsize2(s2), pnt(p) {}
  virtual ~ContProcFountain();

  virtual void Compile(AnimateCompile* anim);

private:
  ContValue *dir1, *dir2;
  ContValue *stepsize1, *stepsize2;
  ContPoint *pnt;
};

class ContProcFM : public ContProcedure {
public:
  ContProcFM(ContValue *steps, ContValue *d)
    : stps(steps), dir(d) {}
  virtual ~ContProcFM();

  virtual void Compile(AnimateCompile* anim);

private:
  ContValue *stps, *dir;
};

class ContProcFMTO : public ContProcedure {
public:
  ContProcFMTO(ContPoint *p)
    : pnt(p) {}
  virtual ~ContProcFMTO();

  virtual void Compile(AnimateCompile* anim);

private:
  ContPoint *pnt;
};

class ContProcGrid : public ContProcedure {
public:
  ContProcGrid(ContValue *g)
    : grid(g) {}
  virtual ~ContProcGrid();

  virtual void Compile(AnimateCompile* anim);

private:
  ContValue *grid;
};

class ContProcHSCM : public ContProcedure {
public:
  ContProcHSCM(ContPoint *p1, ContPoint *p2, ContValue *beats)
    : pnt1(p1), pnt2(p2), numbeats(beats) {}
  virtual ~ContProcHSCM();

  virtual void Compile(AnimateCompile* anim);

private:
  ContPoint *pnt1, *pnt2;
  ContValue *numbeats;
};

class ContProcHSDM : public ContProcedure {
public:
  ContProcHSDM(ContPoint *p)
    : pnt(p) {}
  virtual ~ContProcHSDM();

  virtual void Compile(AnimateCompile* anim);

private:
  ContPoint *pnt;
};

class ContProcMagic : public ContProcedure {
public:
  ContProcMagic(ContPoint *p)
    : pnt(p) {}
  virtual ~ContProcMagic();

  virtual void Compile(AnimateCompile* anim);

private:
  ContPoint *pnt;
};

class ContProcMarch : public ContProcedure {
public:
  ContProcMarch(ContValue *stepsize, ContValue *steps, ContValue *d, ContValue *face)
    : stpsize(stepsize), stps(steps), dir(d), facedir(face) {}
  virtual ~ContProcMarch();

  virtual void Compile(AnimateCompile* anim);

private:
  ContValue *stpsize, *stps, *dir, *facedir;
};

class ContProcMT : public ContProcedure {
public:
  ContProcMT(ContValue *beats, ContValue *d)
    : numbeats(beats), dir(d) {}
  virtual ~ContProcMT();

  virtual void Compile(AnimateCompile* anim);

private:
  ContValue *numbeats, *dir;
};

class ContProcMTRM : public ContProcedure {
public:
  ContProcMTRM(ContValue *d)
    : dir(d) {}
  virtual ~ContProcMTRM();

  virtual void Compile(AnimateCompile* anim);

private:
  ContValue *dir;
};

class ContProcNSEW : public ContProcedure {
public:
  ContProcNSEW(ContPoint *p)
    : pnt(p) {}
  virtual ~ContProcNSEW();

  virtual void Compile(AnimateCompile* anim);

private:
  ContPoint *pnt;
};

class ContProcRotate : public ContProcedure {
public:
  ContProcRotate(ContValue *angle, ContValue *steps, ContPoint *p)
    : ang(angle), stps(steps), pnt(p) {}
  virtual ~ContProcRotate();

  virtual void Compile(AnimateCompile* anim);

private:
  ContValue *ang, *stps;
  ContPoint *pnt;
};

#endif

