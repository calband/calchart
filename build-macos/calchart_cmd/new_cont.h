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

// points conform to
//CC_coord Get(AnimateCompile* anim) const;
//std::ostream& Print(std::ostream&) const;
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

std::ostream& operator<<(std::ostream& os, Point const& p);
CC_coord Get(AnimateCompile& anim, Point const& p);

class ValueAdd;
class ValueSub;
class ValueMult;
class ValueDiv;
	class ValueNeg;

typedef
boost::variant<
	double
	, boost::recursive_wrapper< ValueAdd >
	, boost::recursive_wrapper< ValueSub >
	, boost::recursive_wrapper< ValueMult >
	, boost::recursive_wrapper< ValueDiv >
	, boost::recursive_wrapper< ValueNeg >
>
Value;


class ValueAdd
{
public:
	ValueAdd(Value const& v1, Value const &v2) : value1(v1), value2(v2) {}
	ValueAdd() {}
	double Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value value1, value2;
};

class ValueSub
{
public:
	ValueSub(Value const& v1, Value const &v2) : value1(v1), value2(v2) {}
	ValueSub() {}
	double Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value value1, value2;
};

class ValueMult
{
public:
	ValueMult(Value const& v1, Value const &v2) : value1(v1), value2(v2) {}
	ValueMult() {}
	double Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value value1, value2;
};

class ValueDiv
{
public:
	ValueDiv(Value const& v1, Value const &v2) : value1(v1), value2(v2) {}
	ValueDiv() {}
	double Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value value1, value2;
};

class ValueNeg
{
public:
	ValueNeg(Value const& v) : value(v) {}
	ValueNeg() {}
	double Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value value;
};

}}

BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ValueAdd,
						  (calchart::continuity::Value, value1)
						  (calchart::continuity::Value, value2)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ValueSub,
						  (calchart::continuity::Value, value1)
						  (calchart::continuity::Value, value2)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ValueMult,
						  (calchart::continuity::Value, value1)
						  (calchart::continuity::Value, value2)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ValueDiv,
						  (calchart::continuity::Value, value1)
						  (calchart::continuity::Value, value2)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ValueNeg,
						  (calchart::continuity::Value, value)
						  )

namespace calchart { namespace continuity {

std::ostream& operator<<(std::ostream& os, Value const& v);
double Get(AnimateCompile& anim, Value const& v);
//Value operator+(Value const& lhs, Value const& rhs);

#if 0
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
	double Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Point point;
};

class FunctionDirFrom
{
public:
	FunctionDirFrom(const Point& start, const Point& end) : point_start(start), point_end(end) {}
	FunctionDirFrom() {}
	double Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Point point_start, point_end;
};
	
class FunctionDist
{
public:
	FunctionDist(const Point& p): point(p) {}
	FunctionDist() {}
	double Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Point point;
};

class FunctionDistFrom
{
public:
	FunctionDistFrom(const Point& start, const Point& end) : point_start(start), point_end(end) {}
	FunctionDistFrom() {}
	double Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Point point_start, point_end;
};

class FunctionEither
{
public:
	FunctionEither(Value const& v1, Value const& v2, Point const& p) : dir1(v1), dir2(v2), point(p) {}
	FunctionEither() {}
	double Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value dir1, dir2;
	Point point;
};

class FunctionOpposite
{
public:
	FunctionOpposite(Value const& v) : dir(v) {}
	FunctionOpposite() {}
	double Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value dir;
};

class FunctionStep
{
public:
	FunctionStep(Value const& v1, Value const& v2, Point const& p) : numbeats(v1), blocksize(v2), point(p) {}
	FunctionStep() {}
	double Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value numbeats, blocksize;
	Point point;
};
}}

BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::FunctionDir,
						  (calchart::continuity::Point, point)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::FunctionDirFrom,
						  (calchart::continuity::Point, point_start)
						  (calchart::continuity::Point, point_end)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::FunctionDist,
						  (calchart::continuity::Point, point)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::FunctionDistFrom,
						  (calchart::continuity::Point, point_start)
						  (calchart::continuity::Point, point_end)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::FunctionEither,
						  (calchart::continuity::Value, dir1)
						  (calchart::continuity::Value, dir2)
						  (calchart::continuity::Point, point)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::FunctionOpposite,
						  (calchart::continuity::Value, dir)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::FunctionStep,
						  (calchart::continuity::Value, numbeats)
						  (calchart::continuity::Value, blocksize)
						  (calchart::continuity::Point, point)
						  )


namespace calchart { namespace continuity {

typedef
boost::variant<
FunctionDir
, FunctionDirFrom
, FunctionDist
, FunctionDistFrom
, FunctionEither
, FunctionOpposite
, FunctionStep
>
Function;

std::ostream& operator<<(std::ostream& os, Function const& p);

double Get(AnimateCompile& anim, Function const& p);

#if 0

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
