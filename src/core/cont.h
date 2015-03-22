/*
 * cont.h
 * Classes for continuity
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "animatecompile.h"
#include <iosfwd>

enum ContDefinedValue
{
	CC_N, CC_NW, CC_W, CC_SW, CC_S, CC_SE, CC_E, CC_NE,
	CC_HS, CC_MM, CC_SH, CC_JS, CC_GV, CC_M, CC_DM
};

class ContToken
{
public:
	ContToken();
	virtual ~ContToken() = default; // base dtor marked virtual, implies all derived classes dtors are virtual
	int line, col;
	virtual std::ostream& Print(std::ostream&) const;
};

static inline std::ostream& operator<<(std::ostream& os, const ContToken& c) { return c.Print(os); }

class ContPoint: public ContToken
{
	using super = ContToken;
public:
	ContPoint() {}
	virtual CC_coord Get(AnimateCompile& anim) const;
	virtual std::ostream& Print(std::ostream&) const override;
};

class ContStartPoint : public ContPoint
{
	using super = ContPoint;
public:
	ContStartPoint() {}
	virtual CC_coord Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
};

class ContNextPoint : public ContPoint
{
	using super = ContPoint;
public:
	ContNextPoint() {}
	virtual CC_coord Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
};

class ContRefPoint : public ContPoint
{
	using super = ContPoint;
public:
	ContRefPoint(unsigned n): refnum(n) {}
	virtual CC_coord Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	unsigned refnum;
};

class ContValue: public ContToken
{
	using super = ContToken;
public:
	ContValue() {}
	virtual float Get(AnimateCompile& anim) const = 0;
	virtual std::ostream& Print(std::ostream&) const override;
};

class ContValueFloat : public ContValue
{
	using super = ContValue;
public:
	ContValueFloat(float v): val(v) {}
	virtual float Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	float val;
};

class ContValueDefined : public ContValue
{
	using super = ContValue;
public:
	ContValueDefined(ContDefinedValue v): val(v) {}
	virtual float Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	ContDefinedValue val;
};

class ContValueAdd : public ContValue
{
	using super = ContValue;
public:
	ContValueAdd(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
	virtual float Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> val1, val2;
};

class ContValueSub : public ContValue
{
	using super = ContValue;
public:
	ContValueSub(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
	virtual float Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> val1, val2;
};

class ContValueMult : public ContValue
{
	using super = ContValue;
public:
	ContValueMult(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
	virtual float Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> val1, val2;
};

class ContValueDiv : public ContValue
{
	using super = ContValue;
public:
	ContValueDiv(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
	virtual float Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> val1, val2;
};

class ContValueNeg : public ContValue
{
	using super = ContValue;
public:
	ContValueNeg(ContValue *v) : val(v) {}
	virtual float Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> val;
};

class ContValueREM : public ContValue
{
	using super = ContValue;
public:
	virtual float Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
};

class ContValueVar : public ContValue
{
	using super = ContValue;
public:
	ContValueVar(unsigned num): varnum(num) {}
	virtual float Get(AnimateCompile& anim) const override;
	void Set(AnimateCompile& anim, float v);
	virtual std::ostream& Print(std::ostream&) const override;
private:
	unsigned varnum;
};

class ContFuncDir : public ContValue
{
	using super = ContValue;
public:
	ContFuncDir(ContPoint *p): pnt(p) {}
	virtual float Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContPoint> pnt;
};

class ContFuncDirFrom : public ContValue
{
	using super = ContValue;
public:
	ContFuncDirFrom(ContPoint *start, ContPoint *end) : pnt_start(start), pnt_end(end) {}
	virtual float Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContPoint> pnt_start, pnt_end;
};

class ContFuncDist : public ContValue
{
	using super = ContValue;
public:
	ContFuncDist(ContPoint *p): pnt(p) {}
	virtual float Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContPoint> pnt;
};

class ContFuncDistFrom : public ContValue
{
	using super = ContValue;
public:
	ContFuncDistFrom(ContPoint *start, ContPoint *end) : pnt_start(start), pnt_end(end) {}
	virtual float Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContPoint> pnt_start, pnt_end;
};

class ContFuncEither : public ContValue
{
	using super = ContValue;
public:
	ContFuncEither(ContValue *d1, ContValue *d2, ContPoint *p) : dir1(d1), dir2(d2), pnt(p) {}
	virtual float Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> dir1, dir2;
	std::unique_ptr<ContPoint> pnt;
};

class ContFuncOpp : public ContValue
{
	using super = ContValue;
public:
	ContFuncOpp(ContValue *d): dir(d) {}
	virtual float Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> dir;
};

class ContFuncStep : public ContValue
{
	using super = ContValue;
public:
	ContFuncStep(ContValue *beats, ContValue *blocksize, ContPoint *p) : numbeats(beats), blksize(blocksize), pnt(p) {}
	virtual float Get(AnimateCompile& anim) const override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> numbeats, blksize;
	std::unique_ptr<ContPoint> pnt;
};

class ContProcedure: public ContToken
{
	using super = ContToken;
public:
	ContProcedure() {}
	virtual void Compile(AnimateCompile& anim) = 0;
	virtual std::ostream& Print(std::ostream&) const override;
};

class ContProcSet : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcSet(ContValueVar *vr, ContValue *v) : var(vr), val(v) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValueVar> var;
	std::unique_ptr<ContValue> val;
};

class ContProcBlam : public ContProcedure
{
	using super = ContProcedure;
public:
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
};

class ContProcCM : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcCM(ContPoint *p1, ContPoint *p2, ContValue *steps, ContValue *d1, ContValue *d2, ContValue *beats) : pnt1(p1), pnt2(p2), stps(steps), dir1(d1), dir2(d2), numbeats(beats) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContPoint> pnt1, pnt2;
	std::unique_ptr<ContValue> stps, dir1, dir2, numbeats;
};

class ContProcDMCM : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcDMCM(ContPoint *p1, ContPoint *p2, ContValue *beats) : pnt1(p1), pnt2(p2), numbeats(beats) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContPoint> pnt1, pnt2;
	std::unique_ptr<ContValue> numbeats;
};

class ContProcDMHS : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcDMHS(ContPoint *p) : pnt(p) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContPoint> pnt;
};

class ContProcEven : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcEven(ContValue *steps, ContPoint *p) : stps(steps), pnt(p) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> stps;
	std::unique_ptr<ContPoint> pnt;
};

class ContProcEWNS : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcEWNS(ContPoint *p) : pnt(p) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContPoint> pnt;
};

class ContProcFountain : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcFountain(ContValue *d1, ContValue *d2, ContValue *s1, ContValue *s2, ContPoint *p) : dir1(d1), dir2(d2), stepsize1(s1), stepsize2(s2), pnt(p) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> dir1, dir2;
	std::unique_ptr<ContValue> stepsize1, stepsize2;
	std::unique_ptr<ContPoint> pnt;
};

class ContProcFM : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcFM(ContValue *steps, ContValue *d) : stps(steps), dir(d) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> stps, dir;
};

class ContProcFMTO : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcFMTO(ContPoint *p) : pnt(p) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContPoint> pnt;
};

class ContProcGrid : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcGrid(ContValue *g) : grid(g) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> grid;
};

class ContProcHSCM : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcHSCM(ContPoint *p1, ContPoint *p2, ContValue *beats) : pnt1(p1), pnt2(p2), numbeats(beats) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContPoint> pnt1, pnt2;
	std::unique_ptr<ContValue> numbeats;
};

class ContProcHSDM : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcHSDM(ContPoint *p) : pnt(p) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContPoint> pnt;
};

class ContProcMagic : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcMagic(ContPoint *p) : pnt(p) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContPoint> pnt;
};

class ContProcMarch : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcMarch(ContValue *stepsize, ContValue *steps, ContValue *d, ContValue *face) : stpsize(stepsize), stps(steps), dir(d), facedir(face) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> stpsize, stps, dir, facedir;
};

class ContProcMT : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcMT(ContValue *beats, ContValue *d) : numbeats(beats), dir(d) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> numbeats, dir;
};

class ContProcMTRM : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcMTRM(ContValue *d) : dir(d) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> dir;
};

class ContProcNSEW : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcNSEW(ContPoint *p) : pnt(p) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContPoint> pnt;
};

class ContProcRotate : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcRotate(ContValue *angle, ContValue *steps, ContPoint *p) : ang(angle), stps(steps), pnt(p) {}
	virtual void Compile(AnimateCompile& anim) override;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	std::unique_ptr<ContValue> ang, stps;
	std::unique_ptr<ContPoint> pnt;
};

