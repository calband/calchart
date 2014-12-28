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

#include "math_utils.h"
#include "animatecommand.h"
#include "cc_sheet.h"
#include "cont.h"
#include "parse.h"

std::string ContDefinedValue_strings[] =
{
	"N", "NW", "W", "SW", "S", "SE", "E", "NE",
	"HS", "MM", "SH", "JS", "GV", "M", "DM"
};


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


ContToken::ContToken(): line(yylloc.first_line), col(yylloc.first_column) {}
ContToken::~ContToken() {}
std::ostream& ContToken::Print(std::ostream& os) const
{
	return os<<"["<<line<<","<<col<<"]: ";
}

ContPoint::~ContPoint() {}

CC_coord ContPoint::Get(AnimateCompile* anim) const
{
	return anim->GetPointPosition();
}

std::ostream& ContPoint::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Point:";
}


CC_coord ContStartPoint::Get(AnimateCompile* anim) const
{
	return anim->GetStartingPosition();
}

std::ostream& ContStartPoint::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Start Point";
}


CC_coord ContNextPoint::Get(AnimateCompile* anim) const
{
	return anim->GetEndingPosition(this);
}

std::ostream& ContNextPoint::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Next Point";
}


CC_coord ContRefPoint::Get(AnimateCompile* anim) const
{
	return anim->GetReferencePointPosition(refnum);
}

std::ostream& ContRefPoint::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Ref Point "<<refnum;
}


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


ContValueAdd::~ContValueAdd()
{
	if (val1) delete val1;
	if (val2) delete val2;
}


float ContValueAdd::Get(AnimateCompile* anim) const
{
	return (val1->Get(anim) + val2->Get(anim));
}

std::ostream& ContValueAdd::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<*val1<<" + "<<*val2;
}


ContValueSub::~ContValueSub()
{
	if (val1) delete val1;
	if (val2) delete val2;
}


float ContValueSub::Get(AnimateCompile* anim) const
{
	return (val1->Get(anim) - val2->Get(anim));
}

std::ostream& ContValueSub::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<*val1<<" - "<<*val2;
}


ContValueMult::~ContValueMult()
{
	if (val1) delete val1;
	if (val2) delete val2;
}


float ContValueMult::Get(AnimateCompile* anim) const
{
	return (val1->Get(anim) * val2->Get(anim));
}

std::ostream& ContValueMult::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<*val1<<" * "<<*val2;
}


ContValueDiv::~ContValueDiv()
{
	if (val1) delete val1;
	if (val2) delete val2;
}


float ContValueDiv::Get(AnimateCompile* anim) const
{
	float f;

	f = val2->Get(anim);
	if (IS_ZERO(f))
	{
		anim->RegisterError(ANIMERR_DIVISION_ZERO, this);
		return 0.0;
	}
	else
	{
		return (val1->Get(anim) / f);
	}
}

std::ostream& ContValueDiv::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<*val1<<" / "<<*val2;
}


ContValueNeg::~ContValueNeg()
{
	if (val) delete val;
}


float ContValueNeg::Get(AnimateCompile* anim) const
{
	return -val->Get(anim);
}

std::ostream& ContValueNeg::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"- "<<*val;
}


float ContValueREM::Get(AnimateCompile* anim) const
{
	return anim->GetBeatsRemaining();
}

std::ostream& ContValueREM::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"REM";
}


float ContValueVar::Get(AnimateCompile* anim) const
{
	return anim->GetVarValue(varnum, this);
}

std::ostream& ContValueVar::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Var "<<varnum;
}


void ContValueVar::Set(AnimateCompile* anim, float v)
{
	anim->SetVarValue(varnum, v);
}


ContFuncDir::~ContFuncDir()
{
	if (pnt) delete pnt;
}


float ContFuncDir::Get(AnimateCompile* anim) const
{
	CC_coord c = pnt->Get(anim);
	if (c == anim->GetPointPosition())
	{
		anim->RegisterError(ANIMERR_UNDEFINED, this);
	}
	return anim->GetPointPosition().Direction(c);
}

std::ostream& ContFuncDir::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Direction to "<<*pnt;
}


ContFuncDirFrom::~ContFuncDirFrom()
{
	if (pnt_start) delete pnt_start;
	if (pnt_end) delete pnt_end;
}


float ContFuncDirFrom::Get(AnimateCompile* anim) const
{
	CC_coord start = pnt_start->Get(anim);
	CC_coord end = pnt_end->Get(anim);
	if (start == end)
	{
		anim->RegisterError(ANIMERR_UNDEFINED, this);
	}
	return start.Direction(end);
}

std::ostream& ContFuncDirFrom::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Direction from "<<*pnt_start<<" to "<<*pnt_end;
}


ContFuncDist::~ContFuncDist()
{
	if (pnt) delete pnt;
}


float ContFuncDist::Get(AnimateCompile* anim) const
{
	CC_coord vector;

	vector = pnt->Get(anim) - anim->GetPointPosition();
	return vector.DM_Magnitude();
}

std::ostream& ContFuncDist::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Distance to "<<*pnt;
}


ContFuncDistFrom::~ContFuncDistFrom()
{
	if (pnt_start) delete pnt_start;
	if (pnt_end) delete pnt_end;
}


float ContFuncDistFrom::Get(AnimateCompile* anim) const
{
	CC_coord vector;

	vector = pnt_end->Get(anim) - pnt_start->Get(anim);
	return vector.Magnitude();
}

std::ostream& ContFuncDistFrom::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Distance from "<<*pnt_start<<" to "<<*pnt_end;
}


ContFuncEither::~ContFuncEither()
{
	if (dir1) delete dir1;
	if (dir2) delete dir2;
	if (pnt) delete pnt;
}


float ContFuncEither::Get(AnimateCompile* anim) const
{
	float dir;
	float d1, d2;
	CC_coord c = pnt->Get(anim);

	if (anim->GetPointPosition() == c)
	{
		anim->RegisterError(ANIMERR_UNDEFINED, this);
		return dir1->Get(anim);
	}
	dir = anim->GetPointPosition().Direction(c);
	d1 = dir1->Get(anim) - dir;
	d2 = dir2->Get(anim) - dir;
	while (d1 > 180) d1-=360;
	while (d1 < -180) d1+=360;
	while (d2 > 180) d2-=360;
	while (d2 < -180) d2+=360;
	return (std::abs(d1) > std::abs(d2)) ? dir2->Get(anim) : dir1->Get(anim);
}

std::ostream& ContFuncEither::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Either direction to "<<*dir1<<" or "<<*dir2<<", depending on whichever is a shorter angle to "<<*pnt;
}


ContFuncOpp::~ContFuncOpp()
{
	if (dir) delete dir;
}


float ContFuncOpp::Get(AnimateCompile* anim) const
{
	return (dir->Get(anim) + 180.0f);
}

std::ostream& ContFuncOpp::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"opposite direction of "<<*dir;
}


ContFuncStep::~ContFuncStep()
{
	if (numbeats) delete numbeats;
	if (blksize) delete blksize;
	if (pnt) delete pnt;
}


float ContFuncStep::Get(AnimateCompile* anim) const
{
	CC_coord c;

	c = pnt->Get(anim) - anim->GetPointPosition();
	return (c.DM_Magnitude() * numbeats->Get(anim) / blksize->Get(anim));
}

std::ostream& ContFuncStep::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Step drill at "<<*numbeats<<" beats for a block size of "<<*blksize<<" from point "<<*pnt;
}


ContProcedure::~ContProcedure() {}

std::ostream& ContProcedure::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Procedure: ";
}


ContProcSet::~ContProcSet()
{
	if (var) delete var;
	if (val) delete val;
}


void ContProcSet::Compile(AnimateCompile* anim)
{
	var->Set(anim, val->Get(anim));
}

std::ostream& ContProcSet::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Setting variable "<<*var<<" to "<<*val;
}


void ContProcBlam::Compile(AnimateCompile* anim)
{
	ContNextPoint np;
	CC_coord c;

	c = np.Get(anim) - anim->GetPointPosition();
	anim->Append(std::make_shared<AnimateCommandMove>(anim->GetBeatsRemaining(), c), this);
}

std::ostream& ContProcBlam::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"BLAM";
}


ContProcCM::~ContProcCM()
{
	if (pnt1) delete pnt1;
	if (pnt2) delete pnt2;
	if (stps) delete stps;
	if (dir1) delete dir1;
	if (dir2) delete dir2;
	if (numbeats) delete numbeats;
}


void ContProcCM::Compile(AnimateCompile* anim)
{
	DoCounterMarch(this, anim, pnt1, pnt2, stps, dir1, dir2, numbeats);
}

std::ostream& ContProcCM::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"CounterMarch starting at "<<*pnt1<<" passing through "<<*pnt2<<" stepping "<<*stps<<" off points, first moving "<<*dir1<<" then "<<*dir2<<" for number beats "<<*numbeats;
}


ContProcDMCM::~ContProcDMCM()
{
	if (pnt1) delete pnt1;
	if (pnt2) delete pnt2;
	if (numbeats) delete numbeats;
}


void ContProcDMCM::Compile(AnimateCompile* anim)
{
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
}

std::ostream& ContProcDMCM::Print(std::ostream& os) const
{
	super::Print(os);
	return os<<"Diagonal march CounterMarch starting at "<<*pnt1<<" passing through "<<*pnt2<<" for number beats"<<*numbeats;
}


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
	os<<"March step size "<<*stpsize<<" for steps "<<*stps<<" in direction "<<*dir;
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

