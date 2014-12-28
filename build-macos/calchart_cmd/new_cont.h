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

#ifndef _NEW_CONT_H_
#define _NEW_CONT_H_

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_LIST_SIZE 30

//#include "animatecompile.h"
#include "cc_coord.h"
#include "animate_types.h"
#include <iosfwd>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/phoenix/object/construct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/foreach.hpp>
#include <boost/phoenix/object/construct.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/home/support/iterators/line_pos_iterator.hpp>


class AnimateCompile;

namespace calchart {
namespace continuity {

struct ParsedLocationInfo {
	unsigned line, column, length;
	void Annotate(ParsedLocationInfo const&);
	ParsedLocationInfo(unsigned l=0, unsigned c=0, unsigned len=0) : line(l), column(c), length(len) {}
	virtual ~ParsedLocationInfo() = default;
	virtual std::ostream& Print(std::ostream&) const;
};
static inline std::ostream& operator<<(std::ostream& os, ParsedLocationInfo const& v) { return v.Print(os); }

// Functions
struct FunctionDir;
struct FunctionDirFrom;
struct FunctionDist;
struct FunctionDistFrom;
struct FunctionEither;
struct FunctionOpposite;
struct FunctionStep;

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
void Annotate(Function& p, const ParsedLocationInfo&);

// Variables
struct Variable;

// Values
struct ValueAdd;
struct ValueSub;
struct ValueMult;
struct ValueDiv;
struct ValueNeg;
struct ValueREM;

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

// Functions on Values
std::ostream& operator<<(std::ostream& os, Value const& v);
double Get(AnimateCompile& anim, Value const& v);
void Annotate(Value& p, const ParsedLocationInfo&);

// Points
struct CurrentPoint;
struct StartPoint;
struct NextPoint;
struct RefPoint;

typedef
boost::variant<
CurrentPoint
, StartPoint
, NextPoint
, RefPoint
>
Point;

std::ostream& operator<<(std::ostream& os, Point const& p);
CC_coord Get(AnimateCompile& anim, Point const& p);
void Annotate(Point& p, const ParsedLocationInfo&);

struct ProcedureSet;
struct ProcedureBlam;
struct ProcedureCM;
struct ProcedureDMCM;
struct ProcedureDMHS;
struct ProcedureEven;
struct ProcedureEWNS;
struct ProcedureFountain1;
struct ProcedureFountain2;
struct ProcedureFM;
struct ProcedureFMTO;
struct ProcedureGrid;
struct ProcedureHSCM;
struct ProcedureHSDM;
struct ProcedureMagic;
struct ProcedureMarch1;
struct ProcedureMarch2;
struct ProcedureMT;
struct ProcedureMTRM;
struct ProcedureNSEW;
struct ProcedureRotate;

typedef
boost::variant<
ProcedureSet
, ProcedureBlam
, ProcedureCM
, ProcedureDMCM
, ProcedureDMHS
, ProcedureEven
, ProcedureEWNS
, ProcedureFountain1
, ProcedureFountain2
, ProcedureFM
, ProcedureFMTO
, ProcedureGrid
, ProcedureHSCM
, ProcedureHSDM
, ProcedureMagic
, ProcedureMarch1
, ProcedureMarch2
, ProcedureMT
, ProcedureMTRM
, ProcedureNSEW
, ProcedureRotate
>
Procedure;

std::ostream& operator<<(std::ostream& os, Procedure const& p);
void Compile(AnimateCompile& anim, Procedure const& p);
void Annotate(Procedure& p, const ParsedLocationInfo&);


// points conform to
//CC_coord Get(AnimateCompile& anim) const;
//std::ostream& Print(std::ostream&) const;
struct CurrentPoint : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	CC_coord Get(AnimateCompile& anim) const;
	virtual ~CurrentPoint() = default;
	virtual std::ostream& Print(std::ostream&) const;
};

struct StartPoint : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	CC_coord Get(AnimateCompile& anim) const;
	virtual ~StartPoint() = default;
	virtual std::ostream& Print(std::ostream&) const;
};

struct NextPoint : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	CC_coord Get(AnimateCompile& anim) const;
	virtual ~NextPoint() = default;
	virtual std::ostream& Print(std::ostream&) const;
};

struct RefPoint : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	RefPoint(unsigned n=0): refnum(n) {}
	CC_coord Get(AnimateCompile& anim) const;
	virtual ~RefPoint() = default;
	virtual std::ostream& Print(std::ostream&) const;
	unsigned refnum;
};


// Value conform to
//double Get(AnimateCompile& anim) const;
struct ValueAdd : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ValueAdd(Value const& v1, Value const &v2) : value1(v1), value2(v2) {}
	ValueAdd() {}
	double Get(AnimateCompile& anim) const;
	virtual ~ValueAdd() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value value1, value2;
};

struct ValueSub : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ValueSub(Value const& v1, Value const &v2) : value1(v1), value2(v2) {}
	ValueSub() {}
	double Get(AnimateCompile& anim) const;
	virtual ~ValueSub() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value value1, value2;
};


struct ValueMult : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ValueMult(Value const& v1, Value const &v2) : value1(v1), value2(v2) {}
	ValueMult() {}
	double Get(AnimateCompile& anim) const;
	virtual ~ValueMult() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value value1, value2;
};

struct ValueDiv : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ValueDiv(Value const& v1, Value const &v2) : value1(v1), value2(v2) {}
	ValueDiv() {}
	double Get(AnimateCompile& anim) const;
	virtual ~ValueDiv() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value value1, value2;
};

struct ValueNeg : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ValueNeg(Value const& v) : value(v) {}
	ValueNeg() {}
	double Get(AnimateCompile& anim) const;
	virtual ~ValueNeg() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value value;
};

struct ValueREM : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	double Get(AnimateCompile& anim) const;
	virtual ~ValueREM() = default;
	virtual std::ostream& Print(std::ostream&) const;
};

// Variable
struct Variable : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	Variable(AnimateVar num): varnum(num) {}
	Variable() {}
	double Get(AnimateCompile& anim) const;
	void Set(AnimateCompile& anim, double v) const;
	virtual ~Variable() = default;
	virtual std::ostream& Print(std::ostream&) const;
	AnimateVar varnum;
};
	

// Function conforms to
//double Get(AnimateCompile& anim) const;
struct FunctionDir : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	FunctionDir(const Point& p): point(p) {}
	FunctionDir() {}
	double Get(AnimateCompile& anim) const;
	virtual ~FunctionDir() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Point point;
};

struct FunctionDirFrom : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	FunctionDirFrom(const Point& start, const Point& end) : point_start(start), point_end(end) {}
	FunctionDirFrom() {}
	double Get(AnimateCompile& anim) const;
	virtual ~FunctionDirFrom() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Point point_start, point_end;
};
	
struct FunctionDist : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	FunctionDist(const Point& p): point(p) {}
	FunctionDist() {}
	double Get(AnimateCompile& anim) const;
	virtual ~FunctionDist() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Point point;
};

struct FunctionDistFrom : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	FunctionDistFrom(const Point& start, const Point& end) : point_start(start), point_end(end) {}
	FunctionDistFrom() {}
	double Get(AnimateCompile& anim) const;
	virtual ~FunctionDistFrom() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Point point_start, point_end;
};

struct FunctionEither : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	FunctionEither(Value const& v1, Value const& v2, Point const& p) : dir1(v1), dir2(v2), point(p) {}
	FunctionEither() {}
	double Get(AnimateCompile& anim) const;
	virtual ~FunctionEither() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value dir1, dir2;
	Point point;
};

struct FunctionOpposite : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	FunctionOpposite(Value const& v) : dir(v) {}
	FunctionOpposite() {}
	double Get(AnimateCompile& anim) const;
	virtual ~FunctionOpposite() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value dir;
};

struct FunctionStep : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	FunctionStep(Value const& v1, Value const& v2, Point const& p) : numbeats(v1), blocksize(v2), point(p) {}
	FunctionStep() {}
	double Get(AnimateCompile& anim) const;
	virtual ~FunctionStep() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value numbeats, blocksize;
	Point point;
};


struct ProcedureSet : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureSet(Variable const& vr, Value const& v) : var(vr), val(v) {}
	ProcedureSet() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureSet() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Variable var;
	Value val;
};

struct ProcedureBlam : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureBlam() = default;
	virtual std::ostream& Print(std::ostream&) const;
};

struct ProcedureCM : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureCM(Point const& p1, Point const& p2, Value const& steps, Value const& d1, Value const& d2, Value const& beats) : pnt1(p1), pnt2(p2), stps(steps), dir1(d1), dir2(d2), numbeats(beats) {}
	ProcedureCM() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureCM() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Point pnt1, pnt2;
	Value stps, dir1, dir2, numbeats;
};

struct ProcedureDMCM : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureDMCM(Point const& p1, Point const& p2, Value const& beats) : pnt1(p1), pnt2(p2), numbeats(beats) {}
	ProcedureDMCM() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureDMCM() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Point pnt1, pnt2;
	Value numbeats;
};

struct ProcedureDMHS : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureDMHS(Point const& p) : pnt(p) {}
	ProcedureDMHS() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureDMHS() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Point pnt;
};

struct ProcedureEven : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureEven(Value const& steps, Point const& p) : stps(steps), pnt(p) {}
	ProcedureEven() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureEven() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value stps;
	Point pnt;
};

struct ProcedureEWNS : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureEWNS(Point const& p) : pnt(p) {}
	ProcedureEWNS() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureEWNS() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Point pnt;
};

struct ProcedureFountain1 : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureFountain1(Value const& d1, Value const& d2, Value const& s1, Value const& s2, Point const& p) : dir1(d1), dir2(d2), stepsize1(s1), stepsize2(s2), pnt(p) {}
	ProcedureFountain1() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureFountain1() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value dir1, dir2;
	Value stepsize1, stepsize2;
	Point pnt;
};

struct ProcedureFountain2 : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureFountain2(Value const& d1, Value const& d2, Point const& p) : dir1(d1), dir2(d2), pnt(p) {}
	ProcedureFountain2() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureFountain2() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value dir1, dir2;
	Point pnt;
};

struct ProcedureFM : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureFM(Value const& steps, Value const& d) : stps(steps), dir(d) {}
	ProcedureFM() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureFM() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value stps, dir;
};

struct ProcedureFMTO : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureFMTO(Point const& p) : pnt(p) {}
	ProcedureFMTO() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureFMTO() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Point pnt;
};

struct ProcedureGrid : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureGrid(Value const& g) : grid(g) {}
	ProcedureGrid() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureGrid() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value grid;
};

struct ProcedureHSCM : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureHSCM(Point const& p1, Point const& p2, Value const& beats) : pnt1(p1), pnt2(p2), numbeats(beats) {}
	ProcedureHSCM() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureHSCM() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Point pnt1, pnt2;
	Value numbeats;
};

struct ProcedureHSDM : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureHSDM(Point const& p) : pnt(p) {}
	ProcedureHSDM() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureHSDM() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Point pnt;
};

struct ProcedureMagic : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureMagic(Point const& p) : pnt(p) {}
	ProcedureMagic() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureMagic() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Point pnt;
};

struct ProcedureMarch1 : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureMarch1(Value const& stepsize, Value const& steps, Value const& d, Value const& face) : stpsize(stepsize), stps(steps), dir(d), facedir(face) {}
	ProcedureMarch1() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureMarch1() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value stpsize, stps, dir, facedir;
};

struct ProcedureMarch2 : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureMarch2(Value const& stepsize, Value const& steps, Value const& d) : stpsize(stepsize), stps(steps), dir(d) {}
	ProcedureMarch2() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureMarch2() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value stpsize, stps, dir;
};

struct ProcedureMT : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureMT(Value const& beats, Value const& d) : numbeats(beats), dir(d) {}
	ProcedureMT() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureMT() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value numbeats, dir;
};

struct ProcedureMTRM : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureMTRM(Value const& d) : dir(d) {}
	ProcedureMTRM() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureMTRM() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value dir;
};

struct ProcedureNSEW : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureNSEW(Point const& p) : pnt(p) {}
	ProcedureNSEW() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureNSEW() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Point pnt;
};

struct ProcedureRotate : public ParsedLocationInfo
{
	using super = ParsedLocationInfo;
	ProcedureRotate(Value const& angle, Value const& steps, Point const& p) : ang(angle), stps(steps), pnt(p) {}
	ProcedureRotate() = default;
	void Compile(AnimateCompile& anim) const;
	virtual ~ProcedureRotate() = default;
	virtual std::ostream& Print(std::ostream&) const;
	Value ang, stps;
	Point pnt;
};

template<typename It>
struct annotation_f {
	typedef void result_type;
	
	annotation_f(It first) : first(first) {}
	It const first;
	
	template<typename Val, typename First, typename Last>
	void operator()(Val& v, First f, Last l) const {
		using std::distance;
		do_annotate(v, { static_cast<unsigned>(get_line(f)), static_cast<unsigned>(get_column(first, f)), static_cast<unsigned>(distance(f, l)) });
	}
private:
	void static do_annotate(Point& v, ParsedLocationInfo const& li) {
		using calchart::continuity::Annotate;
		Annotate(v, li);
	}
	void static do_annotate(Function& v, ParsedLocationInfo const& li) {
		using calchart::continuity::Annotate;
		Annotate(v, li);
	}
	void static do_annotate(Value& v, ParsedLocationInfo const& li) {
		using calchart::continuity::Annotate;
		Annotate(v, li);
	}
	void static do_annotate(Procedure& v, ParsedLocationInfo const& li) {
		using calchart::continuity::Annotate;
		Annotate(v, li);
	}
	void static do_annotate(ParsedLocationInfo& v, ParsedLocationInfo const& li) {
		v.Annotate(li);
	}
};

template<typename Iterator>
auto set_location_info() -> decltype(boost::phoenix::function<annotation_f<Iterator>>::operator())
{
	static const boost::phoenix::function<annotation_f<Iterator>> annotate;
	return annotate(boost::spirit::qi::_val, boost::spirit::qi::labels::_1, boost::spirit::qi::labels::_3);
}


}}


// FUSION adaptors work
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::RefPoint,
						  (unsigned, refnum)
						  )

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

BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::Variable,
						  (AnimateVar, varnum)
						  )

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

BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureSet,
						  (calchart::continuity::Variable, var)
						  (calchart::continuity::Value, val)
						  )
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
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureDMHS,
						  (calchart::continuity::Point, pnt)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureEven,
						  (calchart::continuity::Value, stps)
						  (calchart::continuity::Point, pnt)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureEWNS,
						  (calchart::continuity::Point, pnt)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureFountain1,
						  (calchart::continuity::Value, dir1)
						  (calchart::continuity::Value, dir2)
						  (calchart::continuity::Value, stepsize1)
						  (calchart::continuity::Value, stepsize2)
						  (calchart::continuity::Point, pnt)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureFountain2,
						  (calchart::continuity::Value, dir1)
						  (calchart::continuity::Value, dir2)
						  (calchart::continuity::Point, pnt)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureFM,
						  (calchart::continuity::Value, stps)
						  (calchart::continuity::Value, dir)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureFMTO,
						  (calchart::continuity::Point, pnt)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureGrid,
						  (calchart::continuity::Value, grid)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureHSCM,
						  (calchart::continuity::Point, pnt1)
						  (calchart::continuity::Point, pnt2)
						  (calchart::continuity::Value, numbeats)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureHSDM,
						  (calchart::continuity::Point, pnt)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureMagic,
						  (calchart::continuity::Point, pnt)
						  )

BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureMarch1,
						  (calchart::continuity::Value, stpsize)
						  (calchart::continuity::Value, stps)
						  (calchart::continuity::Value, dir)
						  (calchart::continuity::Value, facedir)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureMarch2,
						  (calchart::continuity::Value, stpsize)
						  (calchart::continuity::Value, stps)
						  (calchart::continuity::Value, dir)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureMT,
						  (calchart::continuity::Value, numbeats)
						  (calchart::continuity::Value, dir)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureMTRM,
						  (calchart::continuity::Value, dir)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureNSEW,
						  (calchart::continuity::Point, pnt)
						  )
BOOST_FUSION_ADAPT_STRUCT(
						  calchart::continuity::ProcedureRotate,
						  (calchart::continuity::Value, ang)
						  (calchart::continuity::Value, stps)
						  (calchart::continuity::Point, pnt)
						  )

#endif
