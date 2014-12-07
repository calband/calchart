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

#include <vector>

namespace calchart {
namespace continuity {

enum ContDefinedValue
{
	CC_N, CC_NW, CC_W, CC_SW, CC_S, CC_SE, CC_E, CC_NE,
	CC_HS, CC_MM, CC_SH, CC_JS, CC_GV, CC_M, CC_DM
};

static const std::vector<double> DirectionValues =
{
	0.0 /*CC_N*/, 45.0 /*CC_NW*/, 90.0 /*CC_W*/, 135.0 /*CC_SW*/,
	180.0 /*CC_S*/, 225.0 /*CC_SE*/, 270 /*CC_E*/, 315.0 /*CC_NE*/
};

int float2int(ParsedLocationInfo const& info, AnimateCompile& anim, float f)
{
	int v = (int)floor(f+0.5);
	if (std::abs(f - (float)v) >= COORD_DECIMAL)
	{
		anim.RegisterError(ANIMERR_NONINT, info.line, info.column);
	}
	return v;
}


unsigned float2unsigned(ParsedLocationInfo const& info, AnimateCompile& anim, float f)
{
	int v = float2int(info, anim, f);
	if (v < 0)
	{
		anim.RegisterError(ANIMERR_NEGINT, info.line, info.column);
		return 0;
	}
	else
	{
		return (unsigned)v;
	}
}

void DoCounterMarch(ParsedLocationInfo const& info, AnimateCompile& anim, Point const& pnt1, Point const& pnt2, Value const& stps, Value const& dir1, Value const& dir2, Value const&numbeats)
{
	CC_coord p[4];

//use the law of sines to compute components
	auto d1 = Get(anim, dir1);
	auto d2 = Get(anim, dir2);
	auto beats = Get(anim ,numbeats);
	auto c = sin(Deg2Rad(d1 - d2));
	if (IS_ZERO(c))
	{
		anim.RegisterError(ANIMERR_INVALID_CM, info.line, info.column);
		return;
	}
	CC_coord v1;
	CreateVector(v1, d1, Get(anim, stps));
	p[1] = Get(anim, pnt1) + v1;
	CC_coord ref2 = Get(anim, pnt2);

	auto steps2 = (ref2 - p[1]).Magnitude() * sin(Deg2Rad(ref2.Direction(p[1]) - d1))
		/ c;
	if (IsDiagonalDirection(d2))
	{
		steps2 /= static_cast<float>(SQRT2);
	}
	CC_coord v2;
	CreateVector(v2, d2, steps2);
	p[2] = p[1] + v2;
	p[3] = ref2 - v1;
	p[0] = p[3] - v2;

	v1 = p[1] - anim.GetPointPosition();
	c = BoundDirectionSigned(v1.Direction() - d1);
	unsigned leg = 0;
	if ((v1 != 0) && (IS_ZERO(c)))
	{
		leg = 1;
	}
	else
	{
		v1 = p[2] - anim.GetPointPosition();
		c = BoundDirectionSigned(v1.Direction() - d2);
		if ((v1 != 0) && (IS_ZERO(c)))
		{
			leg = 2;
		}
		else
		{
			v1 = p[3] - anim.GetPointPosition();
			c = BoundDirectionSigned(v1.Direction() - d1 - 180.0f);
			if ((v1 != 0) && (IS_ZERO(c)))
			{
				leg = 3;
			}
			else
			{
				v1 = p[0] - anim.GetPointPosition();
				c = BoundDirectionSigned(v1.Direction() - d2 - 180.0f);
				if ((v1 != 0) && (IS_ZERO(c)))
				{
					leg = 0;
				}
				else
				{
// Current point is not in path of countermarch
					anim.RegisterError(ANIMERR_INVALID_CM, info.line, info.column);
					return;
				}
			}
		}
	}

	while (beats > 0)
	{
		v1 = p[leg] - anim.GetPointPosition();
		c = v1.DM_Magnitude();
		if (c <= beats)
		{
			beats -= c;
			if (!anim.Append(std::make_shared<AnimateCommandMove>(float2unsigned(info, anim, c), v1), info.line, info.column))
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
			anim.Append(std::make_shared<AnimateCommandMove>(float2unsigned(info, anim, beats), v1), info.line, info.column);
			return;
		}
		leg++;
		if (leg > 3) leg = 0;
	}
}

std::ostream& ParsedLocationInfo::Print(std::ostream& os) const
{
	return os<<"["<<line<<","<<column<<","<<length<<"]";
}

void ParsedLocationInfo::Annotate(const ParsedLocationInfo& l)
{
	line = l.line;
	column = l.column;
	length = l.length;
}


CC_coord CurrentPoint::Get(AnimateCompile& anim) const
{
	return anim.GetPointPosition();
}

std::ostream& CurrentPoint::Print(std::ostream& os) const
{
	return super::Print(os)<<"Point";
}


CC_coord StartPoint::Get(AnimateCompile& anim) const
{
	return anim.GetStartingPosition();
}

std::ostream& StartPoint::Print(std::ostream& os) const
{
	return super::Print(os)<<"Start Point";
}


CC_coord NextPoint::Get(AnimateCompile& anim) const
{
	return anim.GetEndingPosition();
}

std::ostream& NextPoint::Print(std::ostream& os) const
{
	return super::Print(os)<<"Next Point";
}


CC_coord RefPoint::Get(AnimateCompile& anim) const
{
	return anim.GetReferencePointPosition(refnum);
}

std::ostream& RefPoint::Print(std::ostream& os) const
{
	return super::Print(os)<<"Ref Point "<<refnum;
}


double ValueAdd::Get(AnimateCompile& anim) const
{
	using calchart::continuity::Get;
	return Get(anim, value1) + Get(anim, value2);
}

std::ostream& ValueAdd::Print(std::ostream& os) const
{
	return super::Print(os)<<value1<<" + "<<value2;
}

double ValueSub::Get(AnimateCompile& anim) const
{
	using calchart::continuity::Get;
	return Get(anim, value1) - Get(anim, value2);
}

std::ostream& ValueSub::Print(std::ostream& os) const
{
	return super::Print(os)<<value1<<" - "<<value2;
}


double ValueMult::Get(AnimateCompile& anim) const
{
	using calchart::continuity::Get;
	return Get(anim, value1) * Get(anim, value2);
}

std::ostream& ValueMult::Print(std::ostream& os) const
{
	return super::Print(os)<<value1<<" * "<<value2;
}


double ValueDiv::Get(AnimateCompile& anim) const
{
	using calchart::continuity::Get;
	auto f = Get(anim, value2);
	if (IS_ZERO(f))
	{
		anim.RegisterError(ANIMERR_DIVISION_ZERO, line, column);
		return 0.0;
	}
	return Get(anim, value1) / f;
}

std::ostream& ValueDiv::Print(std::ostream& os) const
{
	return super::Print(os)<<value1<<" / "<<value2;
}

double ValueNeg::Get(AnimateCompile& anim) const
{
	using calchart::continuity::Get;
	return -Get(anim, value);
}

std::ostream& ValueNeg::Print(std::ostream& os) const
{
	return super::Print(os)<<"- "<<value;
}


double ValueREM::Get(AnimateCompile& anim) const
{
	return anim.GetBeatsRemaining();
}

std::ostream& ValueREM::Print(std::ostream& os) const
{
	return super::Print(os)<<"REM";
}


double Variable::Get(AnimateCompile& anim) const
{
	return anim.GetVarValue(varnum, line, column);
}

std::ostream& Variable::Print(std::ostream& os) const
{
	return super::Print(os)<<"Var "<<varnum;
}

void Variable::Set(AnimateCompile& anim, double v) const
{
	anim.SetVarValue(varnum, v);
}


double FunctionDir::Get(AnimateCompile& anim) const
{
	using calchart::continuity::Get;
	CC_coord c = Get(anim, point);
	if (c == anim.GetPointPosition())
	{
		anim.RegisterError(ANIMERR_UNDEFINED, line, column);
	}
	return anim.GetPointPosition().Direction(c);
}

std::ostream& FunctionDir::Print(std::ostream& os) const
{
	return super::Print(os)<<"Direction to "<<point;
}

	
double FunctionDirFrom::Get(AnimateCompile& anim) const
{
	using calchart::continuity::Get;
	CC_coord start = Get(anim, point_start);
	CC_coord end = Get(anim, point_end);
	if (start == end)
	{
		anim.RegisterError(ANIMERR_UNDEFINED, line, column);
	}
	return start.Direction(end);
}

std::ostream& FunctionDirFrom::Print(std::ostream& os) const
{
	return super::Print(os)<<"Direction from "<<point_start<<" to "<<point_end;
}


double FunctionDist::Get(AnimateCompile& anim) const
{
	using calchart::continuity::Get;
	CC_coord vector = Get(anim, point) - anim.GetPointPosition();
	return vector.DM_Magnitude();
}

std::ostream& FunctionDist::Print(std::ostream& os) const
{
	return super::Print(os)<<"Distance to "<<point;
}


double FunctionDistFrom::Get(AnimateCompile& anim) const
{
	using calchart::continuity::Get;
	CC_coord vector = Get(anim, point_end) - Get(anim, point_start);
	return vector.Magnitude();
}

std::ostream& FunctionDistFrom::Print(std::ostream& os) const
{
	return super::Print(os)<<"Distance from "<<point_start<<" to "<<point_end;
}


double FunctionEither::Get(AnimateCompile& anim) const
{
	using calchart::continuity::Get;
	CC_coord c = Get(anim, point);
	if (c == anim.GetPointPosition())
	{
		anim.RegisterError(ANIMERR_UNDEFINED, line, column);
		return Get(anim, dir1);
	}

	auto dir = anim.GetPointPosition().Direction(c);
	auto d1 = Get(anim, dir1) - dir;
	auto d2 = Get(anim, dir2) - dir;
	while (d1 > 180) d1-=360;
	while (d1 < -180) d1+=360;
	while (d2 > 180) d2-=360;
	while (d2 < -180) d2+=360;
	return (std::abs(d1) > std::abs(d2)) ? Get(anim, dir2) : Get(anim, dir1);
}

std::ostream& FunctionEither::Print(std::ostream& os) const
{
	return super::Print(os)<<"Either direction to "<<dir1<<" or "<<dir2<<", depending on whichever is a shorter angle to "<<point;
}

double FunctionOpposite::Get(AnimateCompile& anim) const
{
	using calchart::continuity::Get;
	return Get(anim, dir) + 180.0;
}

std::ostream& FunctionOpposite::Print(std::ostream& os) const
{
	return super::Print(os)<<"opposite direction of "<<dir;
}


double FunctionStep::Get(AnimateCompile& anim) const
{
	using calchart::continuity::Get;
	CC_coord c = Get(anim, point) - anim.GetPointPosition();
	auto result = c.DM_Magnitude();
	result *= Get(anim, numbeats);
	result /= Get(anim, blocksize);
	
	return result;//(c.DM_Magnitude() * Get(anim, numbeats) / Get(anim, blocksize));
}

std::ostream& FunctionStep::Print(std::ostream& os) const
{
	return super::Print(os)<<"Step drill at "<<numbeats<<" beats for a block size of "<<blocksize<<" from point "<<point;
}



void ProcedureSet::Compile(AnimateCompile& anim) const
{
	using calchart::continuity::Get;
	var.Set(anim, Get(anim, val));
}

std::ostream& ProcedureSet::Print(std::ostream& os) const
{
	return super::Print(os)<<"Setting variable "<<var<<" to "<<val;
}


void ProcedureBlam::Compile(AnimateCompile& anim) const
{
	using calchart::continuity::Get;
	NextPoint np;
	CC_coord c = np.Get(anim) - anim.GetPointPosition();
	anim.Append(std::make_shared<AnimateCommandMove>(anim.GetBeatsRemaining(), c), line, column);
}

std::ostream& ProcedureBlam::Print(std::ostream& os) const
{
	return super::Print(os)<<"BLAM";
}


void ProcedureCM::Compile(AnimateCompile& anim) const
{
	DoCounterMarch(*this, anim, pnt1, pnt2, stps, dir1, dir2, numbeats);
}

std::ostream& ProcedureCM::Print(std::ostream& os) const
{
	return super::Print(os)<<"CounterMarch starting at "<<pnt1<<" passing through "<<pnt2<<" stepping "<<stps<<" off points, first moving "<<dir1<<" then "<<dir2<<" for number beats "<<numbeats;
}


void ProcedureDMCM::Compile(AnimateCompile& anim) const
{
	Value steps = 1.0;

	auto r1 = Get(anim, pnt1);
	auto r2 = Get(anim, pnt2);
	Coord c = r2.x - r1.x;
	if (c == (r2.y - r1.y + Int2Coord(2)))
	{
		if (c >= 0)
		{
			Value dir1 = DirectionValues.at(CC_SW);
			Value dir2 = DirectionValues.at(CC_W);
			DoCounterMarch(*this, anim, pnt1, pnt2, steps, dir1, dir2, numbeats);
			return;
		}
	}
	else if (c == (r1.y - r2.y - Int2Coord(2)))
	{
		if (c >= 0)
		{
			Value dir1 = DirectionValues.at(CC_SE);
			Value dir2 = DirectionValues.at(CC_W);
			DoCounterMarch(*this, anim, pnt1, pnt2, steps, dir1, dir2, numbeats);
			return;
		}
	}
	else if (c == (r1.y - r2.y + Int2Coord(2)))
	{
		if (c <= 0)
		{
			Value dir1 = DirectionValues.at(CC_NW);
			Value dir2 = DirectionValues.at(CC_E);
			DoCounterMarch(*this, anim, pnt1, pnt2, steps, dir1, dir2, numbeats);
			return;
		}
	}
	else if (c == (r2.y - r1.y - Int2Coord(2)))
	{
		if (c <= 0)
		{
			Value dir1 = DirectionValues.at(CC_NE);
			Value dir2 = DirectionValues.at(CC_E);
			DoCounterMarch(*this, anim, pnt1, pnt2, steps, dir1, dir2, numbeats);
			return;
		}
	}
	anim.RegisterError(ANIMERR_INVALID_CM, line, column);
}

std::ostream& ProcedureDMCM::Print(std::ostream& os) const
{
	return super::Print(os)<<"Diagonal march CounterMarch starting at "<<pnt1<<" passing through "<<pnt2<<" for number beats"<<numbeats;
}


void ProcedureDMHS::Compile(AnimateCompile& anim) const
{
	CC_coord c_hs, c_dm;
	short b_hs;

	auto c = Get(anim, pnt) - anim.GetPointPosition();
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
		auto b = Coord2Int(c_dm.x);
		if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c_dm), line, column))
		{
			return;
		}
	}
	if (c_hs != 0)
	{
		anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b_hs), c_hs), line, column);
	}
}

std::ostream& ProcedureDMHS::Print(std::ostream& os) const
{
	return super::Print(os)<<"Diagonal march then HighStep to "<<pnt;
}


void ProcedureEven::Compile(AnimateCompile& anim) const
{
	auto c = Get(anim, pnt) - anim.GetPointPosition();
	int steps = float2int(*this, anim, Get(anim, stps));
	if (steps < 0)
	{
		anim.Append(std::make_shared<AnimateCommandMove>((unsigned)-steps, c, -c.Direction()), line, column);
	}
	else
	{
		anim.Append(std::make_shared<AnimateCommandMove>((unsigned)steps, c), line, column);
	}
}

std::ostream& ProcedureEven::Print(std::ostream& os) const
{
	super::Print(os);
	return super::Print(os)<<"Even march of step size "<<stps<<" to "<<pnt;
}


void ProcedureEWNS::Compile(AnimateCompile& anim) const
{
	auto c1 = Get(anim, pnt) - anim.GetPointPosition();
	if (c1.y != 0)
	{
		CC_coord c2 = { 0, c1.y };
		auto b = Coord2Int(c2.y);
		if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2), line, column))
		{
			return;
		}
	}
	if (c1.x != 0)
	{
		CC_coord c2 = { c1.x, 0 };
		auto b = Coord2Int(c2.x);
		if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2), line, column))
		{
			return;
		}
	}
}

std::ostream& ProcedureEWNS::Print(std::ostream& os) const
{
	return super::Print(os)<<"March EastWest/NorthSouth to "<<pnt;
}


void ProcedureFountain::Compile(AnimateCompile& anim) const
{
	float a, b, c, d, e, f;

	auto f1 = Get(anim, dir1);
	if (use_stepsize)
	{
		auto f2 = Get(anim, stepsize1);
		a = f2 * cos(Deg2Rad(f1));
		c = f2 * -sin(Deg2Rad(f1));
	}
	else
	{
		CreateUnitVector(a, c, f1);
	}
	f1 = Get(anim, dir2);
	if (use_stepsize)
	{
		auto f2 = Get(anim, stepsize2);
		b = f2 * cos(Deg2Rad(f1));
		d = f2 * -sin(Deg2Rad(f1));
	}
	else
	{
		CreateUnitVector(b, d, f1);
	}
	auto v = Get(anim, pnt) - anim.GetPointPosition();
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
			if (!anim.Append(std::make_shared<AnimateCommandMove>(float2unsigned(*this, anim, f1), v), line, column))
			{
				return;
			}
		}
		else
		{
			anim.RegisterError(ANIMERR_INVALID_FNTN, line, column);
			return;
		}
	}
	else
	{
		auto f2 = (d*e - b*f) / f1;
		if (!IS_ZERO(f2))
		{
			v.x = Float2Coord(f2*a);
			v.y = Float2Coord(f2*c);
			if (!anim.Append(std::make_shared<AnimateCommandMove>(float2unsigned(*this, anim, f2), v), line, column))
			{
				return;
			}
		}
		f2 = (a*f - c*e) / f1;
		if (!IS_ZERO(f2))
		{
			v.x = Float2Coord(f2*b);
			v.y = Float2Coord(f2*d);
			if (!anim.Append(std::make_shared<AnimateCommandMove>(float2unsigned(*this, anim, f2), v), line, column))
			{
				return;
			}
		}
	}
}

std::ostream& ProcedureFountain::Print(std::ostream& os) const
{
	if (use_stepsize)
		return super::Print(os)<<"Fountain step, first going "<<dir1<<" then "<<dir2<<", first at "<<stepsize1<<", then at "<<stepsize2<<", ending at "<<pnt;
	return super::Print(os)<<"Fountain step, first going "<<dir1<<" then "<<dir2<<", ending at "<<pnt;
}


void ProcedureFM::Compile(AnimateCompile& anim) const
{
	auto b = float2int(*this, anim, Get(anim, stps));
	if (b != 0)
	{
		CC_coord c;
		CreateVector(c, Get(anim, dir), Get(anim, stps));
		if (c != 0)
		{
			if (b < 0)
			{
				anim.Append(std::make_shared<AnimateCommandMove>((unsigned)-b, c, -c.Direction()), line, column);
			}
			else
			{
				anim.Append(std::make_shared<AnimateCommandMove>((unsigned)b, c), line, column);
			}
		}
	}
}

std::ostream& ProcedureFM::Print(std::ostream& os) const
{
	return super::Print(os)<<"Forward march for steps "<<stps<<" in direction "<<dir;
}


void ProcedureFMTO::Compile(AnimateCompile& anim) const
{
	auto c = Get(anim, pnt) - anim.GetPointPosition();
	if (c != 0)
	{
		anim.Append(std::make_shared<AnimateCommandMove>((unsigned)c.DM_Magnitude(), c), line, column);
	}
}

std::ostream& ProcedureFMTO::Print(std::ostream& os) const
{
	return super::Print(os)<<"Forward march to "<<pnt;
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


void ProcedureGrid::Compile(AnimateCompile& anim) const
{
	auto gridc = Float2Coord(Get(anim, grid));

	CC_coord c;
	c.x = roundcoord(anim.GetPointPosition().x, gridc);
// Adjust so 4 step grid will be on visible grid
	c.y = roundcoord(anim.GetPointPosition().y-Int2Coord(2), gridc) + Int2Coord(2);

	c -= anim.GetPointPosition();
	if (c != 0)
	{
		anim.Append(std::make_shared<AnimateCommandMove>(0, c), line, column);
	}
}

std::ostream& ProcedureGrid::Print(std::ostream& os) const
{
	return super::Print(os)<<"Move on Grid of "<<grid<<" spacing";
}


void ProcedureHSCM::Compile(AnimateCompile& anim) const
{
	Value steps(1.0);

	auto r1 = Get(anim, pnt1);
	auto r2 = Get(anim, pnt2);
	if ((r1.y - r2.y) == Int2Coord(2))
	{
		if (r2.x >= r1.x)
		{
			Value dirs = DirectionValues.at(CC_S);
			Value dirw = DirectionValues.at(CC_W);
			DoCounterMarch(*this, anim, pnt1, pnt2, steps, dirs, dirw, numbeats);
			return;
		}
	}
	else if ((r1.y - r2.y) == -Int2Coord(2))
	{
		if (r1.x >= r2.x)
		{
			Value dirn = DirectionValues.at(CC_N);
			Value dire = DirectionValues.at(CC_E);
			DoCounterMarch(*this, anim, pnt1, pnt2, steps, dirn, dire, numbeats);
			return;
		}
	}
	anim.RegisterError(ANIMERR_INVALID_CM, line, column);
}

std::ostream& ProcedureHSCM::Print(std::ostream& os) const
{
	return super::Print(os)<<"High Step CounterMarch starting at "<<pnt1<<" passing through "<<pnt2<<" for number beats"<<numbeats;
}


void ProcedureHSDM::Compile(AnimateCompile& anim) const
{
	CC_coord c_hs, c_dm;
	short b;

	auto c = Get(anim, pnt) - anim.GetPointPosition();
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
		if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c_hs), line, column))
		{
			return;
		}
	}
	if (c_dm != 0)
	{
		b = Coord2Int(c_dm.x);
		anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c_dm), line, column);
	}
}

std::ostream& ProcedureHSDM::Print(std::ostream& os) const
{
	return super::Print(os)<<"HighStep then Diagonal march to "<<pnt;
}


void ProcedureMagic::Compile(AnimateCompile& anim) const
{
	auto c = Get(anim, pnt) - anim.GetPointPosition();
	anim.Append(std::make_shared<AnimateCommandMove>(0, c), line, column);
}

std::ostream& ProcedureMagic::Print(std::ostream& os) const
{
	return super::Print(os)<<"Magic step to "<<pnt;
}


void ProcedureMarch::Compile(AnimateCompile& anim) const
{
	auto b = float2int(*this, anim, Get(anim, stps));
	if (b != 0)
	{
		auto rads = Deg2Rad(Get(anim, dir));
		auto mag = Get(anim, stpsize) * Get(anim, stps);
		CC_coord c;

		c.x = Float2Coord(cos(rads)*mag);
		c.y = -(Float2Coord(sin(rads)*mag));
		if (c != 0)
		{
			if (use_facedir)
				anim.Append(std::make_shared<AnimateCommandMove>((unsigned)std::abs(b), c, Get(anim, facedir)), line, column);
			else
			if (b < 0)
			{
				anim.Append(std::make_shared<AnimateCommandMove>((unsigned)-b, c, -c.Direction()), line, column);
			}
			else
			{
				anim.Append(std::make_shared<AnimateCommandMove>((unsigned)b, c), line, column);
			}
		}
	}
}

std::ostream& ProcedureMarch::Print(std::ostream& os) const
{
	super::Print(os)<<"March step size"<<stpsize<<" for steps "<<stps<<" in direction "<<dir;
	if (use_facedir)
		os<<" facing "<<facedir;
	return os;
}


void ProcedureMT::Compile(AnimateCompile& anim) const
{
	auto b = float2int(*this, anim, Get(anim, numbeats));
	if (b != 0)
	{
		anim.Append(std::make_shared<AnimateCommandMT>((unsigned)std::abs(b), Get(anim, dir)), line, column);
	}
}

std::ostream& ProcedureMT::Print(std::ostream& os) const
{
	return super::Print(os)<<"MarkTime for "<<numbeats<<" facing "<<dir;
}


void ProcedureMTRM::Compile(AnimateCompile& anim) const
{
	anim.Append(std::make_shared<AnimateCommandMT>(anim.GetBeatsRemaining(), Get(anim, dir)), line, column);
}

std::ostream& ProcedureMTRM::Print(std::ostream& os) const
{
	return super::Print(os)<<"MarkTime for Remaining Beats facing "<<dir;
}


void ProcedureNSEW::Compile(AnimateCompile& anim) const
{
	auto c1 = Get(anim, pnt) - anim.GetPointPosition();
	if (c1.x != 0)
	{
		CC_coord c2 = { c1.x, 0 };
		auto b = Coord2Int(c2.x);
		if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2), line, column))
		{
			return;
		}
	}
	if (c1.y != 0)
	{
		CC_coord c2 = { 0, c1.y };
		auto b = Coord2Int(c2.y);
		if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2), line, column))
		{
			return;
		}
	}
}

std::ostream& ProcedureNSEW::Print(std::ostream& os) const
{
	return super::Print(os)<<"March NorthSouth/EastWest to "<<pnt;
}


void ProcedureRotate::Compile(AnimateCompile& anim) const
{

// Most of the work is converting to polar coordinates
	auto c = Get(anim, pnt);
	auto rad = anim.GetPointPosition() - c;
	float start_ang;
	if (c == anim.GetPointPosition())
		start_ang = anim.GetVarValue(CONTVAR_DOH, line, column);
	else
		start_ang = c.Direction(anim.GetPointPosition());
	auto b = float2int(*this, anim, Get(anim, stps));
	float angle = Get(anim, ang);
	bool backwards = false;
	if (b < 0)
	{
		backwards = true;
	}
	anim.Append(std::make_shared<AnimateCommandRotate>((unsigned)std::abs(b), c,
// Don't use Magnitude() because
// we want Coord numbers
		sqrt(static_cast<float>(rad.x*rad.x + rad.y*rad.y)),
		start_ang, start_ang+angle,
		backwards),
		line, column);
}

std::ostream& ProcedureRotate::Print(std::ostream& os) const
{
	return super::Print(os)<<"Rotate at angle "<<ang<<" for steps "<<stps<<" around pivot point "<<pnt;
}

// Varient support functions
struct Point_Printer : boost::static_visitor<std::ostream&>
{
	Point_Printer(std::ostream& os) : os(os) {}
	std::ostream& operator()(CurrentPoint const& p) const { return p.Print(os); }
	std::ostream& operator()(NextPoint const& p) const { return p.Print(os); }
	std::ostream& operator()(StartPoint const& p) const { return p.Print(os); }
	std::ostream& operator()(RefPoint const& p) const { return p.Print(os); }
	std::ostream& os;
};

std::ostream& operator<<(std::ostream& os, Point const& p)
{
	return boost::apply_visitor(Point_Printer(os), p);
}

struct Point_Getter : boost::static_visitor<CC_coord>
{
	Point_Getter(AnimateCompile& anim) : anim(anim) {}
	CC_coord operator()(CurrentPoint const& p) const { return p.Get(anim); }
	CC_coord operator()(NextPoint const& p) const { return p.Get(anim); }
	CC_coord operator()(StartPoint const& p) const { return p.Get(anim); }
	CC_coord operator()(RefPoint const& p) const { return p.Get(anim); }
	AnimateCompile& anim;
};

CC_coord Get(AnimateCompile& anim, Point const& p)
{
	return boost::apply_visitor(Point_Getter(anim), p);
}

struct Point_Annotate : boost::static_visitor<>
{
	Point_Annotate(const ParsedLocationInfo& l) : l(l) {}
	void operator()(CurrentPoint& p) const { p.Annotate(l); }
	void operator()(NextPoint& p) const { p.Annotate(l); }
	void operator()(StartPoint& p) const { p.Annotate(l); }
	void operator()(RefPoint& p) const { p.Annotate(l); }
	ParsedLocationInfo l;
};

void Annotate(Point& p, const ParsedLocationInfo& l)
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

struct Function_Annotate : boost::static_visitor<>
{
	Function_Annotate(const ParsedLocationInfo& l) : l(l) {}

	void operator()(FunctionDir& f) const { f.Annotate(l); }
	void operator()(FunctionDirFrom& f) const { f.Annotate(l); }
	void operator()(FunctionDist& f) const { f.Annotate(l); }
	void operator()(FunctionDistFrom& f) const { f.Annotate(l); }
	void operator()(FunctionEither& f) const { f.Annotate(l); }
	void operator()(FunctionOpposite& f) const { f.Annotate(l); }
	void operator()(FunctionStep& f) const { f.Annotate(l); }
	ParsedLocationInfo l;
};

void Annotate(Function& f, const ParsedLocationInfo& l)
{
	boost::apply_visitor(Function_Annotate(l), f);
}

struct Function_Getter : boost::static_visitor<double>
{
	Function_Getter(AnimateCompile& anim) : anim(anim) {}
	
	double operator()(calchart::continuity::FunctionDir const& f) const { return f.Get(anim); }
	double operator()(calchart::continuity::FunctionDirFrom const& f) const { return f.Get(anim); }
	double operator()(calchart::continuity::FunctionDist const& f) const { return f.Get(anim); }
	double operator()(calchart::continuity::FunctionDistFrom const& f) const { return f.Get(anim); }
	double operator()(calchart::continuity::FunctionEither const& f) const { return f.Get(anim); }
	double operator()(calchart::continuity::FunctionOpposite const& f) const { return f.Get(anim); }
	double operator()(calchart::continuity::FunctionStep const& f) const { return f.Get(anim); }
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
	double operator()(ValueAdd const& v) const { return v.Get(anim); }
	double operator()(ValueSub const& v) const { return v.Get(anim); }
	double operator()(ValueMult const& v) const { return v.Get(anim); }
	double operator()(ValueDiv const& v) const { return v.Get(anim); }
	double operator()(ValueNeg const& v) const { return v.Get(anim); }
	double operator()(ValueREM const& v) const { return v.Get(anim); }
	double operator()(Variable const& v) const { return v.Get(anim); }
	double operator()(Function const& v) const { using calchart::continuity::Get; return Get(anim, v); }
	AnimateCompile& anim;
};

double Get(AnimateCompile& anim, Value const& v)
{
	return boost::apply_visitor(Value_Getter(anim), v);
}

struct Value_Annotate : boost::static_visitor<>
{
	Value_Annotate(const ParsedLocationInfo& l) : l(l) {}

	void operator()(double const& v) const {}
	void operator()(ValueAdd& v) const { return v.Annotate(l); }
	void operator()(ValueSub& v) const { return v.Annotate(l); }
	void operator()(ValueMult& v) const { return v.Annotate(l); }
	void operator()(ValueDiv& v) const { return v.Annotate(l); }
	void operator()(ValueNeg& v) const { return v.Annotate(l); }
	void operator()(ValueREM& v) const { return v.Annotate(l); }
	void operator()(Variable& v) const { return v.Annotate(l); }
	void operator()(Function& v) const { using calchart::continuity::Annotate; return Annotate(v, l); }

	ParsedLocationInfo l;
};

void Annotate(Value& v, const ParsedLocationInfo& l)
{
	boost::apply_visitor(Value_Annotate(l), v);
}
	
struct Procedure_Printer : boost::static_visitor<>
{
	Procedure_Printer(std::ostream& os) : os(os) {}
	void operator()(ProcedureSet const& p) const { p.Print(os); }
	void operator()(ProcedureBlam const& p) const { p.Print(os); }
	void operator()(ProcedureCM const& p) const { p.Print(os); }
	void operator()(ProcedureDMCM const& p) const { p.Print(os); }
	void operator()(ProcedureDMHS const& p) const { p.Print(os); }
	void operator()(ProcedureEven const& p) const { p.Print(os); }
	void operator()(ProcedureEWNS const& p) const { p.Print(os); }
	void operator()(ProcedureFountain const& p) const { p.Print(os); }
	void operator()(ProcedureFM const& p) const { p.Print(os); }
	void operator()(ProcedureFMTO const& p) const { p.Print(os); }
	void operator()(ProcedureGrid const& p) const { p.Print(os); }
	void operator()(ProcedureHSCM const& p) const { p.Print(os); }
	void operator()(ProcedureHSDM const& p) const { p.Print(os); }
	void operator()(ProcedureMagic const& p) const { p.Print(os); }
	void operator()(ProcedureMarch const& p) const { p.Print(os); }
	void operator()(ProcedureMT const& p) const { p.Print(os); }
	void operator()(ProcedureMTRM const& p) const { p.Print(os); }
	void operator()(ProcedureNSEW const& p) const { p.Print(os); }
	void operator()(ProcedureRotate const& p) const { p.Print(os); }
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
	void operator()(ProcedureSet const& p) const { p.Compile(anim); }
	void operator()(ProcedureBlam const& p) const { p.Compile(anim); }
	void operator()(ProcedureCM const& p) const { p.Compile(anim); }
	void operator()(ProcedureDMCM const& p) const { p.Compile(anim); }
	void operator()(ProcedureDMHS const& p) const { p.Compile(anim); }
	void operator()(ProcedureEven const& p) const { p.Compile(anim); }
	void operator()(ProcedureEWNS const& p) const { p.Compile(anim); }
	void operator()(ProcedureFountain const& p) const { p.Compile(anim); }
	void operator()(ProcedureFM const& p) const { p.Compile(anim); }
	void operator()(ProcedureFMTO const& p) const { p.Compile(anim); }
	void operator()(ProcedureGrid const& p) const { p.Compile(anim); }
	void operator()(ProcedureHSCM const& p) const { p.Compile(anim); }
	void operator()(ProcedureHSDM const& p) const { p.Compile(anim); }
	void operator()(ProcedureMagic const& p) const { p.Compile(anim); }
	void operator()(ProcedureMarch const& p) const { p.Compile(anim); }
	void operator()(ProcedureMT const& p) const { p.Compile(anim); }
	void operator()(ProcedureMTRM const& p) const { p.Compile(anim); }
	void operator()(ProcedureNSEW const& p) const { p.Compile(anim); }
	void operator()(ProcedureRotate const& p) const { p.Compile(anim); }
	AnimateCompile& anim;
};

void Compile(AnimateCompile& anim, Procedure const& p)
{
	boost::apply_visitor(Procedure_Compiler(anim), p);
}

struct Procedure_Annotate : boost::static_visitor<>
{
	Procedure_Annotate(const ParsedLocationInfo& l) : l(l) {}
	void operator()(ProcedureSet& p) const { p.Annotate(l); }
	void operator()(ProcedureBlam& p) const { p.Annotate(l); }
	void operator()(ProcedureCM& p) const { p.Annotate(l); }
	void operator()(ProcedureDMCM& p) const { p.Annotate(l); }
	void operator()(ProcedureDMHS& p) const { p.Annotate(l); }
	void operator()(ProcedureEven& p) const { p.Annotate(l); }
	void operator()(ProcedureEWNS& p) const { p.Annotate(l); }
	void operator()(ProcedureFountain& p) const { p.Annotate(l); }
	void operator()(ProcedureFM& p) const { p.Annotate(l); }
	void operator()(ProcedureFMTO& p) const { p.Annotate(l); }
	void operator()(ProcedureGrid& p) const { p.Annotate(l); }
	void operator()(ProcedureHSCM& p) const { p.Annotate(l); }
	void operator()(ProcedureHSDM& p) const { p.Annotate(l); }
	void operator()(ProcedureMagic& p) const { p.Annotate(l); }
	void operator()(ProcedureMarch& p) const { p.Annotate(l); }
	void operator()(ProcedureMT& p) const { p.Annotate(l); }
	void operator()(ProcedureMTRM& p) const { p.Annotate(l); }
	void operator()(ProcedureNSEW& p) const { p.Annotate(l); }
	void operator()(ProcedureRotate& p) const { p.Annotate(l); }
	ParsedLocationInfo l;
};

void Annotate(Procedure& v, const ParsedLocationInfo& l)
{
	boost::apply_visitor(Procedure_Annotate(l), v);
}


} // namespace continuity
} // namespace calchart
