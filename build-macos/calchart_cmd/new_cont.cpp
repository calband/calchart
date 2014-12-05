/*
 * cont.cpp
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

#include "new_cont.h"

#include "math_utils.h"
#include "animatecommand.h"
#include "cc_sheet.h"
//#include "cont.h"
#include "parse.h"

namespace calchart {
namespace continuity {

std::ostream& operator<<(std::ostream& os, LocationInfo const& v)
{
	return os<<"line : "<<v.line<<", column: "<<v.column<<", length "<<v.length;
}

struct Point_Printer : boost::static_visitor<>
{
	Point_Printer(std::ostream& os) : os(os) {}
	void operator()(CurrentPoint const& p) const { p.Print(os); }
	void operator()(NextPoint const& p) const { p.Print(os); }
	void operator()(StartPoint const& p) const { p.Print(os); }
	void operator()(RefPoint const& p) const { p.Print(os); }
	std::ostream& os;
};

std::ostream& operator<<(std::ostream& os, Point const& p)
{
	boost::apply_visitor(Point_Printer(os), p);
	return os;
}

struct Point_Getter : boost::static_visitor<CC_coord>
{
	Point_Getter(AnimateCompile& anim) : anim(anim) {}
	CC_coord operator()(CurrentPoint const& p) const { return p.Get(&anim); }
	CC_coord operator()(NextPoint const& p) const { return p.Get(&anim); }
	CC_coord operator()(StartPoint const& p) const { return p.Get(&anim); }
	CC_coord operator()(RefPoint const& p) const { return p.Get(&anim); }
	AnimateCompile& anim;
};

	CC_coord Get(AnimateCompile& anim, Point const& p)
	{
		return boost::apply_visitor(Point_Getter(anim), p);
	}

	struct Point_Annotate : boost::static_visitor<>
	{
		Point_Annotate(const LocationInfo& l) : l(l) {}
		void operator()(CurrentPoint& p) const { p.line = l.line; p.column = l.column; p.length = l.length; }
		void operator()(NextPoint& p) const { p.line = l.line; p.column = l.column; p.length = l.length; }
		void operator()(StartPoint& p) const { p.line = l.line; p.column = l.column; p.length = l.length; }
		void operator()(RefPoint& p) const { p.line = l.line; p.column = l.column; p.length = l.length; }
		LocationInfo l;
	};


void Annotate(Point& p, LocationInfo l)
{
	boost::apply_visitor(Point_Annotate(l), p);
}

struct Function_Printer : boost::static_visitor<>
{
	Function_Printer(std::ostream& os) : os(os) {}
	
	void operator()(FunctionDir const& f) const { f.Print(os); }
	void operator()(FunctionDirFrom const& f) const { f.Print(os); }
	void operator()(FunctionDist const& f) const { f.Print(os); }
	void operator()(FunctionDistFrom const& f) const { f.Print(os); }
	void operator()(FunctionEither const& f) const { f.Print(os); }
	void operator()(FunctionOpposite const& f) const { f.Print(os); }
	void operator()(FunctionStep const& f) const { f.Print(os); }
	std::ostream& os;
};

std::ostream& operator<<(std::ostream& os, Function const& p)
{
	boost::apply_visitor(Function_Printer(os), p);
	return os;
}

struct Function_Getter : boost::static_visitor<double>
{
	Function_Getter(AnimateCompile& anim) : anim(anim) {}
	
	double operator()(calchart::continuity::FunctionDir const& f) const { return f.Get(&anim); }
	double operator()(calchart::continuity::FunctionDirFrom const& f) const { return f.Get(&anim); }
	double operator()(calchart::continuity::FunctionDist const& f) const { return f.Get(&anim); }
	double operator()(calchart::continuity::FunctionDistFrom const& f) const { return f.Get(&anim); }
	double operator()(calchart::continuity::FunctionEither const& f) const { return f.Get(&anim); }
	double operator()(calchart::continuity::FunctionOpposite const& f) const { return f.Get(&anim); }
	double operator()(calchart::continuity::FunctionStep const& f) const { return f.Get(&anim); }
	AnimateCompile& anim;
};

double Get(AnimateCompile& anim, Function const& p)
{
	return boost::apply_visitor(Function_Getter(anim), p);
}

struct Value_Printer : boost::static_visitor<>
{
	Value_Printer(std::ostream& os) : os(os) {}
	void operator()(double const& v) const { os<<v; }
	void operator()(ValueAdd const& v) const { v.Print(os); }
	void operator()(ValueSub const& v) const { v.Print(os); }
	void operator()(ValueMult const& v) const { v.Print(os); }
	void operator()(ValueDiv const& v) const { v.Print(os); }
	void operator()(ValueNeg const& v) const { v.Print(os); }
	void operator()(ValueREM const& v) const { v.Print(os); }
	void operator()(Variable const& v) const { v.Print(os); }
	void operator()(Function const& v) const { os<<v; }
	std::ostream& os;
};

std::ostream& operator<<(std::ostream& os, Value const& v)
{
	boost::apply_visitor(Value_Printer(os), v);
	return os;
}

struct Value_Getter : boost::static_visitor<double>
{
	Value_Getter(AnimateCompile& anim) : anim(anim) {}
	double operator()(double const& v) const { return v; }
	double operator()(ValueAdd const& v) const { return v.Get(&anim); }
	double operator()(ValueSub const& v) const { return v.Get(&anim); }
	double operator()(ValueMult const& v) const { return v.Get(&anim); }
	double operator()(ValueDiv const& v) const { return v.Get(&anim); }
	double operator()(ValueNeg const& v) const { return v.Get(&anim); }
	double operator()(ValueREM const& v) const { return v.Get(&anim); }
	double operator()(Variable const& v) const { return v.Get(&anim); }
	double operator()(Function const& v) const { using calchart::continuity::Get; return Get(anim, v); }
	AnimateCompile& anim;
};

double Get(AnimateCompile& anim, Value const& v)
{
	return boost::apply_visitor(Value_Getter(anim), v);
}

//Value operator+(Value const& lhs, Value const& rhs)
//{
//	return boost::apply_visitor(Value_Getter(anim), lhs) + boost::apply_visitor(Value_Getter(anim), rhs);
//}
	
	
	struct Procedure_Printer : boost::static_visitor<>
	{
		Procedure_Printer(std::ostream& os) : os(os) {}
		void operator()(ProcedureSet const& p) const { p.Print(os); }
		void operator()(ProcedureBlam const& p) const { p.Print(os); }
		void operator()(ProcedureCM const& p) const { p.Print(os); }
		void operator()(ProcedureDMCM const& p) const { p.Print(os); }
		std::ostream& os;
	};
	
	std::ostream& operator<<(std::ostream& os, Procedure const& p)
	{
		boost::apply_visitor(Procedure_Printer(os), p);
		return os;
	}
	
	struct Procedure_Compiler : boost::static_visitor<>
	{
		Procedure_Compiler(AnimateCompile& anim) : anim(anim) {}
		void operator()(ProcedureSet const& p) const { p.Compile(&anim); }
		void operator()(ProcedureBlam const& p) const { p.Compile(&anim); }
		void operator()(ProcedureCM const& p) const { p.Compile(&anim); }
		void operator()(ProcedureDMCM const& p) const { p.Compile(&anim); }
		AnimateCompile& anim;
	};
	
	void Compile(AnimateCompile& anim, Procedure const& p)
	{
		boost::apply_visitor(Procedure_Compiler(anim), p);
	}
	


std::string ContDefinedValue_strings[] =
{
	"N", "NW", "W", "SW", "S", "SE", "E", "NE",
	"HS", "MM", "SH", "JS", "GV", "M", "DM"
};

#if 0

int float2int(const ContProcedure *proc,
AnimateCompile *anim,
float f)
{
	int v = (int)floor(f+0.5);
	if (std::abs(f - (float)v) >= COORD_DECIMAL)
	{
		anim->RegisterError(ANIMERR_NONINT, proc);
	}
	return v;
}


unsigned float2unsigned(const ContProcedure *proc,
AnimateCompile *anim,
float f)
{
	int v = float2int(proc, anim, f);
	if (v < 0)
	{
		anim->RegisterError(ANIMERR_NEGINT, proc);
		return 0;
	}
	else
	{
		return (unsigned)v;
	}
}


void DoCounterMarch(const ContProcedure *proc,
AnimateCompile* anim, ContPoint *pnt1, ContPoint *pnt2,
ContValue *stps, ContValue *dir1, ContValue *dir2,
ContValue *numbeats)
{
	CC_coord p[4];
	CC_coord v1, v2;
	CC_coord ref1, ref2;
	float steps1, steps2, d1, d2, beats;
	float c;
	unsigned leg;

//use the law of sines to compute components
	ref1 = pnt1->Get(anim);
	ref2 = pnt2->Get(anim);
	steps1 = stps->Get(anim);
	d1 = dir1->Get(anim);
	d2 = dir2->Get(anim);
	beats = numbeats->Get(anim);
	c = sin(Deg2Rad(d1 - d2));
	if (IS_ZERO(c))
	{
		anim->RegisterError(ANIMERR_INVALID_CM, proc);
		return;
	}
	CreateVector(v1, d1, steps1);
	p[1] = ref1 + v1;
	steps2 = (ref2 - p[1]).Magnitude() * sin(Deg2Rad(ref2.Direction(p[1]) - d1))
		/ c;
	if (IsDiagonalDirection(d2))
	{
		steps2 /= static_cast<float>(SQRT2);
	}
	CreateVector(v2, d2, steps2);
	p[2] = p[1] + v2;
	p[3] = ref2 - v1;
	p[0] = p[3] - v2;

	v1 = p[1] - anim->GetPointPosition();
	c = BoundDirectionSigned(v1.Direction() - d1);
	if ((v1 != 0) && (IS_ZERO(c)))
	{
		leg = 1;
	}
	else
	{
		v1 = p[2] - anim->GetPointPosition();
		c = BoundDirectionSigned(v1.Direction() - d2);
		if ((v1 != 0) && (IS_ZERO(c)))
		{
			leg = 2;
		}
		else
		{
			v1 = p[3] - anim->GetPointPosition();
			c = BoundDirectionSigned(v1.Direction() - d1 - 180.0f);
			if ((v1 != 0) && (IS_ZERO(c)))
			{
				leg = 3;
			}
			else
			{
				v1 = p[0] - anim->GetPointPosition();
				c = BoundDirectionSigned(v1.Direction() - d2 - 180.0f);
				if ((v1 != 0) && (IS_ZERO(c)))
				{
					leg = 0;
				}
				else
				{
// Current point is not in path of countermarch
					anim->RegisterError(ANIMERR_INVALID_CM, proc);
					return;
				}
			}
		}
	}

	while (beats > 0)
	{
		v1 = p[leg] - anim->GetPointPosition();
		c = v1.DM_Magnitude();
		if (c <= beats)
		{
			beats -= c;
			if (!anim->Append(std::make_shared<AnimateCommandMove>(float2unsigned(proc, anim, c), v1), proc))
			{
				return;
			}
		}
		else
		{
			switch(leg)
			{
				case 0:
					CreateVector(v1, d2+180.0f, beats);
					break;
				case 1:
					CreateVector(v1, d1, beats);
					break;
				case 2:
					CreateVector(v1, d2, beats);
					break;
				default:
					CreateVector(v1, d1+180.0f, beats);
					break;
			}
			anim->Append(std::make_shared<AnimateCommandMove>(float2unsigned(proc, anim, beats), v1), proc);
			return;
		}
		leg++;
		if (leg > 3) leg = 0;
	}
}
#endif
#if 0


ContToken::ContToken(): line(yylloc.first_line), col(yylloc.first_column) {}
ContToken::~ContToken() {}
std::ostream& ContToken::Print(std::ostream& os) const
{
	return os<<"["<<line<<","<<col<<"]: ";
}
#endif

CC_coord CurrentPoint::Get(AnimateCompile* anim) const
{
	return anim->GetPointPosition();
}

std::ostream& CurrentPoint::Print(std::ostream& os) const
{
	return os<<"Point: "<<static_cast<const LocationInfo&>(*this);
}


CC_coord StartPoint::Get(AnimateCompile* anim) const
{
	return anim->GetStartingPosition();
}

std::ostream& StartPoint::Print(std::ostream& os) const
{
	return os<<"Start Point"<<static_cast<const LocationInfo&>(*this);
}


CC_coord NextPoint::Get(AnimateCompile* anim) const
{
	return anim->GetEndingPosition();
}

std::ostream& NextPoint::Print(std::ostream& os) const
{
	return os<<"Next Point"<<static_cast<const LocationInfo&>(*this);
}


CC_coord RefPoint::Get(AnimateCompile* anim) const
{
	return anim->GetReferencePointPosition(refnum);
}

std::ostream& RefPoint::Print(std::ostream& os) const
{
	return os<<"Ref Point "<<refnum<<static_cast<const LocationInfo&>(*this);
}

//	void function_printer::operator()(Function const& f) const
//	{
//		std::cout << "got: "<<f<<std::endl;
//	}
#if 0
ContValue::~ContValue() {}

std::ostream& ContValue::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Value:";
}


float ContValueFloat::Get(AnimateCompile*) const
{
	return val;
}

std::ostream& ContValueFloat::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<val;
}


float ContValueDefined::Get(AnimateCompile*) const
{
	float f;

	switch (val)
	{
		default:
			f = 0.0;
			break;
		case CC_NW:
			f = 45.0;
			break;
		case CC_W:
			f = 90.0;
			break;
		case CC_SW:
			f = 135.0;
			break;
		case CC_S:
			f = 180.0;
			break;
		case CC_SE:
			f = 225.0;
			break;
		case CC_E:
			f = 270.0;
			break;
		case CC_NE:
			f = 315.0;
			break;
		case CC_HS:
			f = 1.0;
			break;
		case CC_MM:
			f = 1.0;
			break;
		case CC_SH:
			f = 0.5;
			break;
		case CC_JS:
			f = 0.5;
			break;
		case CC_GV:
			f = 1.0;
			break;
		case CC_M:
			f = 4.0f/3;
			break;
		case CC_DM:
			f = static_cast<float>(SQRT2);
			break;
	}
	return f;
}

std::ostream& ContValueDefined::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Defined:"<<ContDefinedValue_strings[val];
}

#endif

double ValueAdd::Get(AnimateCompile* anim) const
{
	using calchart::continuity::Get;
	return Get(*anim, value1) + Get(*anim, value2);
}

std::ostream& ValueAdd::Print(std::ostream& os) const
{
	return os<<value1<<" + "<<value2;
}

double ValueSub::Get(AnimateCompile* anim) const
{
	using calchart::continuity::Get;
	return Get(*anim, value1) - Get(*anim, value2);
}

std::ostream& ValueSub::Print(std::ostream& os) const
{
	return os<<value1<<" - "<<value2;
}


double ValueMult::Get(AnimateCompile* anim) const
{
	using calchart::continuity::Get;
	return Get(*anim, value1) * Get(*anim, value2);
}

std::ostream& ValueMult::Print(std::ostream& os) const
{
	return os<<value1<<" * "<<value2;
}


double ValueDiv::Get(AnimateCompile* anim) const
{
	using calchart::continuity::Get;
	auto f = Get(*anim, value2);
	if (IS_ZERO(f))
	{
#warning need to resolve errors
//		anim->RegisterError(ANIMERR_DIVISION_ZERO, this);
		return 0.0;
	}
	return Get(*anim, value1) / f;
}

std::ostream& ValueDiv::Print(std::ostream& os) const
{
	return os<<value1<<" / "<<value2;
}

double ValueNeg::Get(AnimateCompile* anim) const
{
	using calchart::continuity::Get;
	return -Get(*anim, value);
}

std::ostream& ValueNeg::Print(std::ostream& os) const
{
	return os<<"- "<<value;
}


double ValueREM::Get(AnimateCompile* anim) const
{
	return anim->GetBeatsRemaining();
}

std::ostream& ValueREM::Print(std::ostream& os) const
{
	return os<<"REM";
}


double Variable::Get(AnimateCompile* anim) const
{
	return anim->GetVarValue(varnum);
}

std::ostream& Variable::Print(std::ostream& os) const
{
	return os<<"Var "<<varnum;
}

void Variable::Set(AnimateCompile* anim, double v) const
{
	anim->SetVarValue(varnum, v);
}


double FunctionDir::Get(AnimateCompile* anim) const
{
	using calchart::continuity::Get;
	CC_coord c = Get(*anim, point);
	if (c == anim->GetPointPosition())
	{
#warning need to resolve this:
//		anim->RegisterError(ANIMERR_UNDEFINED, this);
	}
	return anim->GetPointPosition().Direction(c);
}

std::ostream& FunctionDir::Print(std::ostream& os) const
{
	return os<<"Direction to "<<point;
}

	
double FunctionDirFrom::Get(AnimateCompile* anim) const
{
	using calchart::continuity::Get;
	CC_coord start = Get(*anim, point_start);
	CC_coord end = Get(*anim, point_end);
	if (start == end)
	{
#warning need to resolve this:
//		anim->RegisterError(ANIMERR_UNDEFINED, this);
	}
	return start.Direction(end);
}

std::ostream& FunctionDirFrom::Print(std::ostream& os) const
{
	return os<<"Direction from "<<point_start<<" to "<<point_end;
}


double FunctionDist::Get(AnimateCompile* anim) const
{
	using calchart::continuity::Get;
	CC_coord vector = Get(*anim, point) - anim->GetPointPosition();
	return vector.DM_Magnitude();
}

std::ostream& FunctionDist::Print(std::ostream& os) const
{
	return os<<"Distance to "<<point;
}


double FunctionDistFrom::Get(AnimateCompile* anim) const
{
	using calchart::continuity::Get;
	CC_coord vector = Get(*anim, point_end) - Get(*anim, point_start);
	return vector.Magnitude();
}

std::ostream& FunctionDistFrom::Print(std::ostream& os) const
{
	return os<<"Distance from "<<point_start<<" to "<<point_end;
}


double FunctionEither::Get(AnimateCompile* anim) const
{
	using calchart::continuity::Get;
	CC_coord c = Get(*anim, point);
	if (c == anim->GetPointPosition())
	{
#warning need to resolve this:
//		anim->RegisterError(ANIMERR_UNDEFINED, this);
		return Get(*anim, dir1);
	}

	auto dir = anim->GetPointPosition().Direction(c);
	auto d1 = Get(*anim, dir1) - dir;
	auto d2 = Get(*anim, dir2) - dir;
	while (d1 > 180) d1-=360;
	while (d1 < -180) d1+=360;
	while (d2 > 180) d2-=360;
	while (d2 < -180) d2+=360;
	return (std::abs(d1) > std::abs(d2)) ? Get(*anim, dir2) : Get(*anim, dir1);
}

std::ostream& FunctionEither::Print(std::ostream& os) const
{
	return os<<"Either direction to "<<dir1<<" or "<<dir2<<", depending on whichever is a shorter angle to "<<point;
}

double FunctionOpposite::Get(AnimateCompile* anim) const
{
	using calchart::continuity::Get;
	return Get(*anim, dir) + 180.0;
}

std::ostream& FunctionOpposite::Print(std::ostream& os) const
{
	return os<<"opposite direction of "<<dir;
}


double FunctionStep::Get(AnimateCompile* anim) const
{
	using calchart::continuity::Get;
	CC_coord c = Get(*anim, point) - anim->GetPointPosition();
	auto result = c.DM_Magnitude();
	result *= Get(*anim, numbeats);
	result /= Get(*anim, blocksize);
	
	return result;//(c.DM_Magnitude() * Get(*anim, numbeats) / Get(*anim, blocksize));
}

std::ostream& FunctionStep::Print(std::ostream& os) const
{
	return os<<"Step drill at "<<numbeats<<" beats for a block size of "<<blocksize<<" from point "<<point;
}



void ProcedureSet::Compile(AnimateCompile* anim) const
{
	using calchart::continuity::Get;
	var.Set(anim, Get(*anim, val));
}

std::ostream& ProcedureSet::Print(std::ostream& os) const
{
	return os<<"Setting variable "<<var<<" to "<<val;
}


void ProcedureBlam::Compile(AnimateCompile* anim) const
{
	using calchart::continuity::Get;
	NextPoint np;
	CC_coord c = np.Get(anim) - anim->GetPointPosition();
	anim->Append(std::make_shared<AnimateCommandMove>(anim->GetBeatsRemaining(), c));
}

std::ostream& ProcedureBlam::Print(std::ostream& os) const
{
	return os<<"BLAM";
}


void ProcedureCM::Compile(AnimateCompile* anim) const
{
//	DoCounterMarch(this, anim, pnt1, pnt2, stps, dir1, dir2, numbeats);
}

std::ostream& ProcedureCM::Print(std::ostream& os) const
{
	return os<<"CounterMarch starting at "<<pnt1<<" passing through "<<pnt2<<" stepping "<<stps<<" off points, first moving "<<dir1<<" then "<<dir2<<" for number beats "<<numbeats;
}


void ProcedureDMCM::Compile(AnimateCompile* anim) const
{
#if 0
	CC_coord r1, r2;
	Coord c;
	ContValueFloat steps(1.0);

	r1 = pnt1->Get(anim);
	r2 = pnt2->Get(anim);
	c = r2.x - r1.x;
	if (c == (r2.y - r1.y + Int2Coord(2)))
	{
		if (c >= 0)
		{
			ContValueDefined dir1(CC_SW);
			ContValueDefined dir2(CC_W);
			DoCounterMarch(this, anim, pnt1, pnt2, &steps, &dir1, &dir2, numbeats);
			return;
		}
	}
	else if (c == (r1.y - r2.y - Int2Coord(2)))
	{
		if (c >= 0)
		{
			ContValueDefined dir1(CC_SE);
			ContValueDefined dir2(CC_W);
			DoCounterMarch(this, anim, pnt1, pnt2, &steps, &dir1, &dir2, numbeats);
			return;
		}
	}
	else if (c == (r1.y - r2.y + Int2Coord(2)))
	{
		if (c <= 0)
		{
			ContValueDefined dir1(CC_NW);
			ContValueDefined dir2(CC_E);
			DoCounterMarch(this, anim, pnt1, pnt2, &steps, &dir1, &dir2, numbeats);
			return;
		}
	}
	else if (c == (r2.y - r1.y - Int2Coord(2)))
	{
		if (c <= 0)
		{
			ContValueDefined dir1(CC_NE);
			ContValueDefined dir2(CC_E);
			DoCounterMarch(this, anim, pnt1, pnt2, &steps, &dir1, &dir2, numbeats);
			return;
		}
	}
	anim->RegisterError(ANIMERR_INVALID_CM, this);
#endif
}

std::ostream& ProcedureDMCM::Print(std::ostream& os) const
{
	return os<<"Diagonal march CounterMarch starting at "<<pnt1<<" passing through "<<pnt2<<" for number beats"<<numbeats;
}

#if 0
	

ContProcDMHS::~ContProcDMHS()
{
	if (pnt) delete pnt;
}


void ContProcDMHS::Compile(AnimateCompile* anim)
{
	CC_coord c, c_hs, c_dm;
	short b, b_hs;

	c = pnt->Get(anim) - anim->GetPointPosition();
	if (std::abs(c.x) > std::abs(c.y))
	{
												  // adjust sign
		c_hs.x = ((c.x<0) != (c.y<0)) ? c.x+c.y : c.x-c.y;
		c_hs.y = 0;
												  // adjust sign
		c_dm.x = ((c.x < 0) != (c.y < 0)) ? -c.y : c.y;
		c_dm.y = c.y;
		b_hs = Coord2Int(c_hs.x);
	}
	else
	{
		c_hs.x = 0;
												  // adjust sign
		c_hs.y = ((c.x<0) != (c.y<0)) ? c.y+c.x : c.y-c.x;
		c_dm.x = c.x;
												  // adjust sign
		c_dm.y = ((c.x < 0) != (c.y < 0)) ? -c.x : c.x;
		b_hs = Coord2Int(c_hs.y);
	}
	if (c_dm != 0)
	{
		b = Coord2Int(c_dm.x);
		if (!anim->Append(std::make_shared<AnimateCommandMove>(std::abs(b), c_dm), this))
		{
			return;
		}
	}
	if (c_hs != 0)
	{
		anim->Append(std::make_shared<AnimateCommandMove>(std::abs(b_hs), c_hs), this);
	}
}

std::ostream& ContProcDMHS::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Diagonal march then HighStep to "<<*pnt;
}


ContProcEven::~ContProcEven()
{
	if (stps) delete stps;
	if (pnt) delete pnt;
}


void ContProcEven::Compile(AnimateCompile* anim)
{
	CC_coord c;

	c = pnt->Get(anim) - anim->GetPointPosition();
	int steps = float2int(this, anim, stps->Get(anim));
	if (steps < 0)
	{
		anim->Append(std::make_shared<AnimateCommandMove>((unsigned)-steps, c, -c.Direction()), this);
	}
	else
	{
		anim->Append(std::make_shared<AnimateCommandMove>((unsigned)steps, c), this);
	}
}

std::ostream& ContProcEven::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Even march of step size "<<*stps<<" to "<<*pnt;
}


ContProcEWNS::~ContProcEWNS()
{
	if (pnt) delete pnt;
}


void ContProcEWNS::Compile(AnimateCompile* anim)
{
	CC_coord c1, c2;
	short b;

	c1 = pnt->Get(anim) - anim->GetPointPosition();
	if (c1.y != 0)
	{
		c2.x = 0;
		c2.y = c1.y;
		b = Coord2Int(c2.y);
		if (!anim->Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2), this))
		{
			return;
		}
	}
	if (c1.x != 0)
	{
		c2.x = c1.x;
		c2.y = 0;
		b = Coord2Int(c2.x);
		if (!anim->Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2), this))
		{
			return;
		}
	}
}

std::ostream& ContProcEWNS::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"March EastWest/NorthSouth to "<<*pnt;
}


ContProcFountain::~ContProcFountain()
{
	if (dir1) delete dir1;
	if (dir2) delete dir2;
	if (stepsize1) delete stepsize1;
	if (stepsize2) delete stepsize2;
}


void ContProcFountain::Compile(AnimateCompile* anim)
{
	float a, b, c, d, e, f;
	float f1, f2;
	CC_coord v;

	f1 = dir1->Get(anim);
	if (stepsize1)
	{
		f2 = stepsize1->Get(anim);
		a = f2 * cos(Deg2Rad(f1));
		c = f2 * -sin(Deg2Rad(f1));
	}
	else
	{
		CreateUnitVector(a, c, f1);
	}
	f1 = dir2->Get(anim);
	if (stepsize2)
	{
		f2 = stepsize2->Get(anim);
		b = f2 * cos(Deg2Rad(f1));
		d = f2 * -sin(Deg2Rad(f1));
	}
	else
	{
		CreateUnitVector(b, d, f1);
	}
	v = pnt->Get(anim) - anim->GetPointPosition();
	e = Coord2Float(v.x);
	f = Coord2Float(v.y);
	f1 = a*d - b*c;
	if (IS_ZERO(f1))
	{
		if (IS_ZERO(a-b) && IS_ZERO(c-d) && IS_ZERO(e*c-a*f))
		{
// Special case: directions are same
			if (IS_ZERO(c))
			{
				f1 = f/c;
			}
			else
			{
				f1 = e/a;
			}
			if (!anim->Append(std::make_shared<AnimateCommandMove>(float2unsigned(this, anim, f1), v), this))
			{
				return;
			}
		}
		else
		{
			anim->RegisterError(ANIMERR_INVALID_FNTN, this);
			return;
		}
	}
	else
	{
		f2 = (d*e - b*f) / f1;
		if (!IS_ZERO(f2))
		{
			v.x = Float2Coord(f2*a);
			v.y = Float2Coord(f2*c);
			if (!anim->Append(std::make_shared<AnimateCommandMove>(float2unsigned(this, anim, f2), v), this))
			{
				return;
			}
		}
		f2 = (a*f - c*e) / f1;
		if (!IS_ZERO(f2))
		{
			v.x = Float2Coord(f2*b);
			v.y = Float2Coord(f2*d);
			if (!anim->Append(std::make_shared<AnimateCommandMove>(float2unsigned(this, anim, f2), v), this))
			{
				return;
			}
		}
	}
}

std::ostream& ContProcFountain::Print(std::ostream& os) const
{
	super::Print(os);
	os<<"Fountain step, first going "<<*dir1<<" then "<<*dir2;
	if (stepsize1)
		os<<", first at "<<*stepsize1;
	if (stepsize2)
		os<<", then at "<<*stepsize2;
	return os<<"ending at "<<*pnt;
}


ContProcFM::~ContProcFM()
{
	if (stps) delete stps;
	if (dir) delete dir;
}


void ContProcFM::Compile(AnimateCompile* anim)
{
	CC_coord c;
	int b;

	b = float2int(this, anim, stps->Get(anim));
	if (b != 0)
	{
		CreateVector(c, dir->Get(anim), stps->Get(anim));
		if (c != 0)
		{
			if (b < 0)
			{
				anim->Append(std::make_shared<AnimateCommandMove>((unsigned)-b, c, -c.Direction()), this);
			}
			else
			{
				anim->Append(std::make_shared<AnimateCommandMove>((unsigned)b, c), this);
			}
		}
	}
}

std::ostream& ContProcFM::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Forward march for steps "<<*stps<<" in direction "<<*dir;
}


ContProcFMTO::~ContProcFMTO()
{
	if (pnt) delete pnt;
}


void ContProcFMTO::Compile(AnimateCompile* anim)
{
	CC_coord c;

	c = pnt->Get(anim) - anim->GetPointPosition();
	if (c != 0)
	{
		anim->Append(std::make_shared<AnimateCommandMove>((unsigned)c.DM_Magnitude(), c), this);
	}
}

std::ostream& ContProcFMTO::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Forward march to "<<*pnt;
}


ContProcGrid::~ContProcGrid()
{
	if (grid) delete grid;
}


static inline Coord roundcoord(Coord a, Coord mod)
{
	mod = std::abs(mod);
	if (mod > 0)
	{
		if (a < 0)
		{
			a = ((a-(mod/2))/mod)*mod;
		}
		else
		{
			a = ((a+(mod/2))/mod)*mod;
		}
	}
	return a;
}


void ContProcGrid::Compile(AnimateCompile* anim)
{
	Coord gridc;
	CC_coord c;

	gridc = Float2Coord(grid->Get(anim));

	c.x = roundcoord(anim->GetPointPosition().x, gridc);
// Adjust so 4 step grid will be on visible grid
	c.y = roundcoord(anim->GetPointPosition().y-Int2Coord(2), gridc) + Int2Coord(2);

	c -= anim->GetPointPosition();
	if (c != 0)
	{
		anim->Append(std::make_shared<AnimateCommandMove>(0, c), this);
	}
}

std::ostream& ContProcGrid::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Move on Grid of "<<*grid<<" spacing";
}


ContProcHSCM::~ContProcHSCM()
{
	if (pnt1) delete pnt1;
	if (pnt2) delete pnt2;
	if (numbeats) delete numbeats;
}


void ContProcHSCM::Compile(AnimateCompile* anim)
{
	CC_coord r1, r2;
	ContValueFloat steps(1.0);

	r1 = pnt1->Get(anim);
	r2 = pnt2->Get(anim);
	if ((r1.y - r2.y) == Int2Coord(2))
	{
		if (r2.x >= r1.x)
		{
			ContValueDefined dirs(CC_S);
			ContValueDefined dirw(CC_W);
			DoCounterMarch(this, anim, pnt1, pnt2, &steps, &dirs, &dirw, numbeats);
			return;
		}
	}
	else if ((r1.y - r2.y) == -Int2Coord(2))
	{
		if (r1.x >= r2.x)
		{
			ContValueDefined dirn(CC_N);
			ContValueDefined dire(CC_E);
			DoCounterMarch(this, anim, pnt1, pnt2, &steps, &dirn, &dire, numbeats);
			return;
		}
	}
	anim->RegisterError(ANIMERR_INVALID_CM, this);
}

std::ostream& ContProcHSCM::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"High Step CounterMarch starting at "<<*pnt1<<" passing through "<<*pnt2<<" for number beats"<<*numbeats;
}


ContProcHSDM::~ContProcHSDM()
{
	if (pnt) delete pnt;
}


void ContProcHSDM::Compile(AnimateCompile* anim)
{
	CC_coord c, c_hs, c_dm;
	short b;

	c = pnt->Get(anim) - anim->GetPointPosition();
	if (std::abs(c.x) > std::abs(c.y))
	{
												  // adjust sign
		c_hs.x = ((c.x<0) != (c.y<0)) ? c.x+c.y : c.x-c.y;
		c_hs.y = 0;
												  // adjust sign
		c_dm.x = ((c.x < 0) != (c.y < 0)) ? -c.y : c.y;
		c_dm.y = c.y;
		b = Coord2Int(c_hs.x);
	}
	else
	{
		c_hs.x = 0;
												  // adjust sign
		c_hs.y = ((c.x<0) != (c.y<0)) ? c.y+c.x : c.y-c.x;
		c_dm.x = c.x;
												  // adjust sign
		c_dm.y = ((c.x < 0) != (c.y < 0)) ? -c.x : c.x;
		b = Coord2Int(c_hs.y);
	}
	if (c_hs != 0)
	{
		if (!anim->Append(std::make_shared<AnimateCommandMove>(std::abs(b), c_hs), this))
		{
			return;
		}
	}
	if (c_dm != 0)
	{
		b = Coord2Int(c_dm.x);
		anim->Append(std::make_shared<AnimateCommandMove>(std::abs(b), c_dm), this);
	}
}

std::ostream& ContProcHSDM::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"HighStep then Diagonal march to "<<*pnt;
}


ContProcMagic::~ContProcMagic()
{
	if (pnt) delete pnt;
}


void ContProcMagic::Compile(AnimateCompile* anim)
{
	CC_coord c;

	c = pnt->Get(anim) - anim->GetPointPosition();
	anim->Append(std::make_shared<AnimateCommandMove>(0, c), this);
}

std::ostream& ContProcMagic::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Magic step to "<<*pnt;
}


ContProcMarch::~ContProcMarch()
{
	if (stpsize) delete stpsize;
	if (stps) delete stps;
	if (dir) delete dir;
}


void ContProcMarch::Compile(AnimateCompile* anim)
{
	CC_coord c;
	float rads, mag;
	int b;

	b = float2int(this, anim, stps->Get(anim));
	if (b != 0)
	{
		rads = Deg2Rad(dir->Get(anim));
		mag = stpsize->Get(anim) * stps->Get(anim);
		c.x = Float2Coord(cos(rads)*mag);
		c.y = -(Float2Coord(sin(rads)*mag));
		if (c != 0)
		{
			if (facedir)
				anim->Append(std::make_shared<AnimateCommandMove>((unsigned)std::abs(b), c, facedir->Get(anim)), this);
			else
			if (b < 0)
			{
				anim->Append(std::make_shared<AnimateCommandMove>((unsigned)-b, c, -c.Direction()), this);
			}
			else
			{
				anim->Append(std::make_shared<AnimateCommandMove>((unsigned)b, c), this);
			}
		}
	}
}

std::ostream& ContProcMarch::Print(std::ostream& os) const
{
	super::Print(os);
	os<<"March step size"<<*stpsize<<" for steps "<<*stps<<" in direction "<<*dir;
	if (facedir)
		os<<" facing "<<*facedir;
	return os;
}


ContProcMT::~ContProcMT()
{
	if (numbeats) delete numbeats;
	if (dir) delete dir;
}


void ContProcMT::Compile(AnimateCompile* anim)
{
	int b;

	b = float2int(this, anim, numbeats->Get(anim));
	if (b != 0)
	{
		anim->Append(std::make_shared<AnimateCommandMT>((unsigned)std::abs(b), dir->Get(anim)), this);
	}
}

std::ostream& ContProcMT::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"MarkTime for "<<*numbeats<<" facing "<<*dir;
}


ContProcMTRM::~ContProcMTRM()
{
	if (dir) delete dir;
}


void ContProcMTRM::Compile(AnimateCompile* anim)
{
	anim->Append(std::make_shared<AnimateCommandMT>(anim->GetBeatsRemaining(), dir->Get(anim)), this);
}

std::ostream& ContProcMTRM::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"MarkTime for Remaining Beats facing "<<*dir;
}


ContProcNSEW::~ContProcNSEW()
{
	if (pnt) delete pnt;
}


void ContProcNSEW::Compile(AnimateCompile* anim)
{
	CC_coord c1, c2;
	short b;

	c1 = pnt->Get(anim) - anim->GetPointPosition();
	if (c1.x != 0)
	{
		c2.x = c1.x;
		c2.y = 0;
		b = Coord2Int(c2.x);
		if (!anim->Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2), this))
		{
			return;
		}
	}
	if (c1.y != 0)
	{
		c2.x = 0;
		c2.y = c1.y;
		b = Coord2Int(c2.y);
		if (!anim->Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2), this))
		{
			return;
		}
	}
}

std::ostream& ContProcNSEW::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"March NorthSouth/EastWest to "<<*pnt;
}


ContProcRotate::~ContProcRotate()
{
	if (ang) delete ang;
	if (stps) delete stps;
	if (pnt) delete pnt;
}


void ContProcRotate::Compile(AnimateCompile* anim)
{
	float start_ang;
	CC_coord c;
	CC_coord rad;

// Most of the work is converting to polar coordinates
	c = pnt->Get(anim);
	rad = anim->GetPointPosition() - c;
	if (c == anim->GetPointPosition())
		start_ang = anim->GetVarValue(CONTVAR_DOH, this);
	else
		start_ang = c.Direction(anim->GetPointPosition());
	int b = float2int(this, anim, stps->Get(anim));
	float angle = ang->Get(anim);
	bool backwards = false;
	if (b < 0)
	{
		backwards = true;
	}
	anim->Append(std::make_shared<AnimateCommandRotate>((unsigned)std::abs(b), c,
// Don't use Magnitude() because
// we want Coord numbers
		sqrt(static_cast<float>(rad.x*rad.x + rad.y*rad.y)),
		start_ang, start_ang+angle,
		backwards),
		this);
}

std::ostream& ContProcRotate::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Rotate at angle "<<*ang<<" for steps "<<*stps<<" around pivot point "<<*pnt;
}
#endif

} // namespace continuity
} // namespace calchart
