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

#ifndef _CONT_H_
#define _CONT_H_

#include "animatecompile.h"
#include <iosfwd>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>

namespace calchart {
namespace continuity {
		
#if 0

enum ContDefinedValue
{
	CC_N, CC_NW, CC_W, CC_SW, CC_S, CC_SE, CC_E, CC_NE,
	CC_HS, CC_MM, CC_SH, CC_JS, CC_GV, CC_M, CC_DM
};

class ContToken
{
public:
	ContToken();
	virtual ~ContToken();
	int line, col;
	virtual std::ostream& Print(std::ostream&) const;
};

static inline std::ostream& operator<<(std::ostream& os, const ContToken& c) { return c.Print(os); }

#endif

// points are anything that conforms to
//CC_coord Get(AnimateCompile* anim) const;
//std::ostream& Print(std::ostream&) const override;
class CurrentPoint
{
public:
	CC_coord Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
};

class StartPoint
{
public:
	CC_coord Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
};

class NextPoint
{
public:
	CC_coord Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
};

class RefPoint
{
public:
	RefPoint(unsigned n=0): refnum(n) {}

	CC_coord Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	unsigned refnum;
};

}}

BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::RefPoint,
						  (unsigned, refnum)
						  )

namespace calchart { namespace continuity {

typedef
boost::variant<
calchart::continuity::CurrentPoint
, calchart::continuity::StartPoint
, calchart::continuity::NextPoint
, calchart::continuity::RefPoint
>
Point;

struct Point_Printer : boost::static_visitor<>
{
	Point_Printer(std::ostream& os) : os(os) {}
	
	void operator()(calchart::continuity::CurrentPoint const& p) const
	{
		p.Print(os);
	}
	void operator()(calchart::continuity::NextPoint const& p) const
	{
		p.Print(os);
	}
	void operator()(calchart::continuity::StartPoint const& p) const
	{
		p.Print(os);
	}
	void operator()(calchart::continuity::RefPoint const& p) const
	{
		p.Print(os);
	}
	std::ostream& os;
};

static inline std::ostream& operator<<(std::ostream& os, Point const& p)
{
	boost::apply_visitor(Point_Printer(os), p);
	return os;
}

struct Point_Getter : boost::static_visitor<CC_coord>
{
	Point_Getter(AnimateCompile& anim) : anim(anim) {}
	
	CC_coord operator()(calchart::continuity::CurrentPoint const& p) const
	{
		return p.Get(&anim);
	}
	CC_coord operator()(calchart::continuity::NextPoint const& p) const
	{
		return p.Get(&anim);
	}
	CC_coord operator()(calchart::continuity::StartPoint const& p) const
	{
		return p.Get(&anim);
	}
	CC_coord operator()(calchart::continuity::RefPoint const& p) const
	{
		return p.Get(&anim);
	}
	AnimateCompile& anim;
};

static inline CC_coord Get(AnimateCompile& anim, Point const& p)
{
	return boost::apply_visitor(Point_Getter(anim), p);
}

#if 0

class ContValue: public ContToken
{
	using super = ContToken;
public:
	ContValue() {}
	virtual ~ContValue();

	virtual float Get(AnimateCompile* anim) const = 0;
	virtual std::ostream& Print(std::ostream&) const override;
};

class ContValueFloat : public ContValue
{
	using super = ContValue;
public:
	ContValueFloat(float v): val(v) {}

	virtual float Get(AnimateCompile* anim) const;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	float val;
};

class ContValueDefined : public ContValue
{
	using super = ContValue;
public:
	ContValueDefined(ContDefinedValue v): val(v) {}

	virtual float Get(AnimateCompile* anim) const;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	ContDefinedValue val;
};

class ContValueAdd : public ContValue
{
	using super = ContValue;
public:
	ContValueAdd(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
	virtual ~ContValueAdd();

	virtual float Get(AnimateCompile* anim) const;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	ContValue *val1, *val2;
};

class ContValueSub : public ContValue
{
	using super = ContValue;
public:
	ContValueSub(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
	virtual ~ContValueSub();

	virtual float Get(AnimateCompile* anim) const;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	ContValue *val1, *val2;
};

class ContValueMult : public ContValue
{
	using super = ContValue;
public:
	ContValueMult(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
	virtual ~ContValueMult();

	virtual float Get(AnimateCompile* anim) const;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	ContValue *val1, *val2;
};

class ContValueDiv : public ContValue
{
	using super = ContValue;
public:
	ContValueDiv(ContValue *v1, ContValue *v2) : val1(v1), val2(v2) {}
	virtual ~ContValueDiv();

	virtual float Get(AnimateCompile* anim) const;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	ContValue *val1, *val2;
};

class ContValueNeg : public ContValue
{
	using super = ContValue;
public:
	ContValueNeg(ContValue *v) : val(v) {}
	virtual ~ContValueNeg();

	virtual float Get(AnimateCompile* anim) const;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	ContValue *val;
};

class ContValueREM : public ContValue
{
	using super = ContValue;
public:
	virtual float Get(AnimateCompile* anim) const;
	virtual std::ostream& Print(std::ostream&) const override;
};

class ContValueVar : public ContValue
{
	using super = ContValue;
public:
	ContValueVar(unsigned num): varnum(num) {}

	virtual float Get(AnimateCompile* anim) const;
	void Set(AnimateCompile* anim, float v);
	virtual std::ostream& Print(std::ostream&) const override;
private:
	unsigned varnum;
};

#endif

class FunctionDir
{
public:
	FunctionDir(const Point& p): point(p) {}
	FunctionDir() {}
	float Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Point point;
};

}}

BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::FunctionDir,
						  (calchart::continuity::Point, point)
						  )

namespace calchart { namespace continuity {

typedef
boost::variant<
FunctionDir
>
Function;

struct Function_Printer : boost::static_visitor<>
{
	Function_Printer(std::ostream& os) : os(os) {}
	
	void operator()(FunctionDir const& f) const
	{
		f.Print(os);
	}
	std::ostream& os;
};

static inline std::ostream& operator<<(std::ostream& os, Function const& p)
{
	boost::apply_visitor(Function_Printer(os), p);
	return os;
}

struct Function_Getter : boost::static_visitor<float>
{
	Function_Getter(AnimateCompile& anim) : anim(anim) {}
	
	float operator()(calchart::continuity::FunctionDir const& f) const
	{
		return f.Get(&anim);
	}
	AnimateCompile& anim;
};

static inline float Get(AnimateCompile& anim, Function const& p)
{
	return boost::apply_visitor(Function_Getter(anim), p);
}

#if 0
class ContFuncDirFrom : public ContValue
{
	using super = ContValue;
public:
	ContFuncDirFrom(ContPoint *start, ContPoint *end)
		: pnt_start(start), pnt_end(end) {}
	virtual ~ContFuncDirFrom();

	virtual float Get(AnimateCompile* anim) const;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	ContPoint *pnt_start, *pnt_end;
};

class ContFuncDist : public ContValue
{
	using super = ContValue;
public:
	ContFuncDist(ContPoint *p): pnt(p) {}
	virtual ~ContFuncDist();

	virtual float Get(AnimateCompile* anim) const;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	ContPoint *pnt;
};

class ContFuncDistFrom : public ContValue
{
	using super = ContValue;
public:
	ContFuncDistFrom(ContPoint *start, ContPoint *end)
		: pnt_start(start), pnt_end(end) {}
	virtual ~ContFuncDistFrom();

	virtual float Get(AnimateCompile* anim) const;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	ContPoint *pnt_start, *pnt_end;
};

class ContFuncEither : public ContValue
{
	using super = ContValue;
public:
	ContFuncEither(ContValue *d1, ContValue *d2, ContPoint *p)
		: dir1(d1), dir2(d2), pnt(p) {}
	virtual ~ContFuncEither();

	virtual float Get(AnimateCompile* anim) const;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	ContValue *dir1, *dir2;
	ContPoint *pnt;
};

class ContFuncOpp : public ContValue
{
	using super = ContValue;
public:
	ContFuncOpp(ContValue *d): dir(d) {}
	virtual ~ContFuncOpp();

	virtual float Get(AnimateCompile* anim) const;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	ContValue *dir;
};

class ContFuncStep : public ContValue
{
	using super = ContValue;
public:
	ContFuncStep(ContValue *beats, ContValue *blocksize, ContPoint *p)
		: numbeats(beats), blksize(blocksize), pnt(p) {}
	virtual ~ContFuncStep();

	virtual float Get(AnimateCompile* anim) const;
	virtual std::ostream& Print(std::ostream&) const override;
private:
	ContValue *numbeats, *blksize;
	ContPoint *pnt;
};

class ContProcedure: public ContToken
{
	using super = ContToken;
public:
	ContProcedure(): next(NULL) {}
	virtual ~ContProcedure();

	virtual void Compile(AnimateCompile* anim) = 0;
	virtual std::ostream& Print(std::ostream&) const override;

	ContProcedure *next;
};

class ContProcSet : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcSet(ContValueVar *vr, ContValue *v)
		: var(vr), val(v) {}
	virtual ~ContProcSet();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContValueVar *var;
	ContValue *val;
};

class ContProcBlam : public ContProcedure
{
	using super = ContProcedure;
public:
	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;
};

class ContProcCM : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcCM(ContPoint *p1, ContPoint *p2, ContValue *steps, ContValue *d1,
		ContValue *d2, ContValue *beats)
		: pnt1(p1), pnt2(p2), stps(steps), dir1(d1), dir2(d2), numbeats(beats) {}
	virtual ~ContProcCM();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContPoint *pnt1, *pnt2;
	ContValue *stps, *dir1, *dir2, *numbeats;
};

class ContProcDMCM : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcDMCM(ContPoint *p1, ContPoint *p2, ContValue *beats)
		: pnt1(p1), pnt2(p2), numbeats(beats) {}
	virtual ~ContProcDMCM();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContPoint *pnt1, *pnt2;
	ContValue *numbeats;
};

class ContProcDMHS : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcDMHS(ContPoint *p)
		: pnt(p) {}
	virtual ~ContProcDMHS();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContPoint *pnt;
};

class ContProcEven : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcEven(ContValue *steps, ContPoint *p)
		: stps(steps), pnt(p) {}
	virtual ~ContProcEven();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContValue *stps;
	ContPoint *pnt;
};

class ContProcEWNS : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcEWNS(ContPoint *p)
		: pnt(p) {}
	virtual ~ContProcEWNS();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContPoint *pnt;
};

class ContProcFountain : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcFountain(ContValue *d1, ContValue *d2, ContValue *s1, ContValue *s2,
		ContPoint *p)
		: dir1(d1), dir2(d2), stepsize1(s1), stepsize2(s2), pnt(p) {}
	virtual ~ContProcFountain();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContValue *dir1, *dir2;
	ContValue *stepsize1, *stepsize2;
	ContPoint *pnt;
};

class ContProcFM : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcFM(ContValue *steps, ContValue *d)
		: stps(steps), dir(d) {}
	virtual ~ContProcFM();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContValue *stps, *dir;
};

class ContProcFMTO : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcFMTO(ContPoint *p)
		: pnt(p) {}
	virtual ~ContProcFMTO();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContPoint *pnt;
};

class ContProcGrid : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcGrid(ContValue *g)
		: grid(g) {}
	virtual ~ContProcGrid();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContValue *grid;
};

class ContProcHSCM : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcHSCM(ContPoint *p1, ContPoint *p2, ContValue *beats)
		: pnt1(p1), pnt2(p2), numbeats(beats) {}
	virtual ~ContProcHSCM();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContPoint *pnt1, *pnt2;
	ContValue *numbeats;
};

class ContProcHSDM : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcHSDM(ContPoint *p)
		: pnt(p) {}
	virtual ~ContProcHSDM();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContPoint *pnt;
};

class ContProcMagic : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcMagic(ContPoint *p)
		: pnt(p) {}
	virtual ~ContProcMagic();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContPoint *pnt;
};

class ContProcMarch : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcMarch(ContValue *stepsize, ContValue *steps, ContValue *d, ContValue *face)
		: stpsize(stepsize), stps(steps), dir(d), facedir(face) {}
	virtual ~ContProcMarch();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContValue *stpsize, *stps, *dir, *facedir;
};

class ContProcMT : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcMT(ContValue *beats, ContValue *d)
		: numbeats(beats), dir(d) {}
	virtual ~ContProcMT();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContValue *numbeats, *dir;
};

class ContProcMTRM : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcMTRM(ContValue *d)
		: dir(d) {}
	virtual ~ContProcMTRM();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContValue *dir;
};

class ContProcNSEW : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcNSEW(ContPoint *p)
		: pnt(p) {}
	virtual ~ContProcNSEW();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContPoint *pnt;
};

class ContProcRotate : public ContProcedure
{
	using super = ContProcedure;
public:
	ContProcRotate(ContValue *angle, ContValue *steps, ContPoint *p)
		: ang(angle), stps(steps), pnt(p) {}
	virtual ~ContProcRotate();

	virtual void Compile(AnimateCompile* anim);
	virtual std::ostream& Print(std::ostream&) const override;

private:
	ContValue *ang, *stps;
	ContPoint *pnt;
};
#endif
} // namespace continuity
} // namespace calchart

#endif
