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

	struct LocationInfo {
		unsigned line, column, length;
	};

	class FunctionDir;
	class FunctionDirFrom;
	class FunctionDist;
	class FunctionDistFrom;
	class FunctionEither;
	class FunctionOpposite;
	class FunctionStep;

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
	
	class ValueAdd;
	class ValueSub;
	class ValueMult;
	class ValueDiv;
	class ValueNeg;
	class ValueREM;
	
	class Variable;
	
	typedef
	boost::variant<
	double
	, boost::recursive_wrapper< ValueAdd >
	, boost::recursive_wrapper< ValueSub >
	, boost::recursive_wrapper< ValueMult >
	, boost::recursive_wrapper< ValueDiv >
	, boost::recursive_wrapper< ValueNeg >
	, boost::recursive_wrapper< ValueREM >
	, boost::recursive_wrapper< Variable >
	, boost::recursive_wrapper< Function >
	>
	Value;
	
	

// points conform to
//CC_coord Get(AnimateCompile* anim) const;
//std::ostream& Print(std::ostream&) const;
	struct CurrentPoint : public LocationInfo
{
public:
	CC_coord Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
};
	static inline std::ostream& operator<<(std::ostream& os, CurrentPoint const& v) { return v.Print(os); }

	class StartPoint : public LocationInfo
{
public:
	CC_coord Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
};
	static inline std::ostream& operator<<(std::ostream& os, StartPoint const& v) { return v.Print(os); }

	class NextPoint : public LocationInfo
{
public:
	CC_coord Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
};
	static inline std::ostream& operator<<(std::ostream& os, NextPoint const& v) { return v.Print(os); }

	class RefPoint : public LocationInfo
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
	void Annotate(Point& p, LocationInfo);

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
	static inline std::ostream& operator<<(std::ostream& os, ValueNeg const& v) { return v.Print(os); }

class ValueREM
{
public:
	double Get(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
};
	static inline std::ostream& operator<<(std::ostream& os, ValueREM const& v) { return v.Print(os); }

std::ostream& operator<<(std::ostream& os, Value const& v);
double Get(AnimateCompile& anim, Value const& v);
	
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

	class Variable
	{
	public:
		Variable(AnimateVar num): varnum(num) {}
		Variable() {}
		double Get(AnimateCompile* anim) const;
		void Set(AnimateCompile* anim, double v) const;
		std::ostream& Print(std::ostream&) const;
		AnimateVar varnum;
	};

	static inline std::ostream& operator<<(std::ostream& os, Variable const& v) { return v.Print(os); }
//	double Get(AnimateCompile& anim, Variable const& v);
//	void Set(AnimateCompile& anim, Variable& which, Value const& v);
	
}}

BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::Variable,
						  (AnimateVar, varnum)
						  )

namespace calchart { namespace continuity {

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

std::ostream& operator<<(std::ostream& os, Function const& p);

double Get(AnimateCompile& anim, Function const& p);


struct ProcedureSet
{
	ProcedureSet(Variable const& vr, Value const& v) : var(vr), val(v) {}
	ProcedureSet() {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Variable var;
	Value val;
};

struct ProcedureBlam
{
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
};

struct ProcedureCM
{
	ProcedureCM(Point const& p1, Point const& p2, Value const& steps, Value const& d1, Value const& d2, Value const& beats) : pnt1(p1), pnt2(p2), stps(steps), dir1(d1), dir2(d2), numbeats(beats) {}
	ProcedureCM() {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Point pnt1, pnt2;
	Value stps, dir1, dir2, numbeats;
};

struct ProcedureDMCM
{
	ProcedureDMCM(Point const& p1, Point const& p2, Value const& beats) : pnt1(p1), pnt2(p2), numbeats(beats) {}
	ProcedureDMCM() {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Point pnt1, pnt2;
	Value numbeats;
};

struct ProcedureDMHS
{
	ProcedureDMHS(Point const& p) : pnt(p) {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Point pnt;
};

struct ProcedureEven
{
	ProcedureEven(Value const& steps, Point const& p) : stps(steps), pnt(p) {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value stps;
	Point pnt;
};

struct ProcedureEWNS
{
	ProcedureEWNS(Point const& p) : pnt(p) {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Point pnt;
};

struct ProcedureFountain
{
	ProcedureFountain(Value const& d1, Value const& d2, Value const& s1, Value const& s2, Point const& p) : dir1(d1), dir2(d2), stepsize1(s1), stepsize2(s2), pnt(p) {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value dir1, dir2;
	Value stepsize1, stepsize2;
	Point pnt;
};

struct ProcedureFM
{
	ProcedureFM(Value const& steps, Value const& d) : stps(steps), dir(d) {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value stps, dir;
};

struct ProcedureFMTO
{
	ProcedureFMTO(Point const& p) : pnt(p) {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Point pnt;
};

struct ProcedureGrid
{
	ProcedureGrid(Value const& g) : grid(g) {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value grid;
};

struct ProcedureHSCM
{
	ProcedureHSCM(Point const& p1, Point const& p2, Value const& beats) : pnt1(p1), pnt2(p2), numbeats(beats) {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Point pnt1, pnt2;
	Value numbeats;
};

struct ProcedureHSDM
{
	ProcedureHSDM(Point const& p) : pnt(p) {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Point pnt;
};

struct ProcedureMagic
{
	ProcedureMagic(Point const& p) : pnt(p) {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Point pnt;
};

struct ProcedureMarch
{
	ProcedureMarch(Value const& stepsize, Value const& steps, Value const& d, Value const& face) : stpsize(stepsize), stps(steps), dir(d), facedir(face) {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value stpsize, stps, dir, facedir;
};

struct ProcedureMT
{
	ProcedureMT(Value const& beats, Value const& d) : numbeats(beats), dir(d) {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value numbeats, dir;
};

struct ProcedureMTRM
{
	ProcedureMTRM(Value const& d) : dir(d) {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value dir;
};

struct ProcedureNSEW
{
	ProcedureNSEW(Point const& p) : pnt(p) {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Point pnt;
};

struct ProcedureRotate
{
	ProcedureRotate(Value const& angle, Value const& steps, Point const& p) : ang(angle), stps(steps), pnt(p) {}
	void Compile(AnimateCompile* anim) const;
	std::ostream& Print(std::ostream&) const;
	Value ang, stps;
	Point pnt;
};

	typedef
	boost::variant<
	calchart::continuity::ProcedureSet
	, calchart::continuity::ProcedureBlam
	, calchart::continuity::ProcedureCM
	, calchart::continuity::ProcedureDMCM
	/*
	, calchart::continuity::ProcedureDMHS
	, calchart::continuity::ProcedureEven
	, calchart::continuity::ProcedureEWNS
	, calchart::continuity::ProcedureFountain
	, calchart::continuity::ProcedureFM
	, calchart::continuity::ProcedureFMTO
	, calchart::continuity::ProcedureGrid
	, calchart::continuity::ProcedureHSCM
	, calchart::continuity::ProcedureHSDM
	, calchart::continuity::ProcedureMagic
	, calchart::continuity::ProcedureMarch
	, calchart::continuity::ProcedureMT
	, calchart::continuity::ProcedureMTRM
	, calchart::continuity::ProcedureNSEW
	, calchart::continuity::ProcedureRotate
	 */
	>
	Procedure;
	std::ostream& operator<<(std::ostream& os, Procedure const& p);
	
//	void Compile(AnimateCompile& anim, Procedure const& p);
}}

BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureCM,
						  (calchart::continuity::Point, pnt1)
						  (calchart::continuity::Point, pnt2)
						  (calchart::continuity::Value, stps)
						  (calchart::continuity::Value, dir1)
						  (calchart::continuity::Value, dir2)
						  (calchart::continuity::Value, numbeats)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureDMCM,
						  (calchart::continuity::Point, pnt1)
						  (calchart::continuity::Point, pnt2)
						  (calchart::continuity::Value, numbeats)
						  )

#endif
