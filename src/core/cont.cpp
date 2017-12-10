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

namespace CalChart {

static const std::string ContDefinedValue_strings[] = {
    "N", "NW", "W", "SW", "S", "SE", "E", "NE",
    "HS", "MM", "SH", "JS", "GV", "M", "DM"
};

int float2int(const ContProcedure* proc, AnimateCompile& anim, float f)
{
    auto v = (int)floor(f + 0.5);
    if (std::abs(f - (float)v) >= COORD_DECIMAL) {
        anim.RegisterError(ANIMERR_NONINT, proc);
    }
    return v;
}

unsigned float2unsigned(const ContProcedure* proc, AnimateCompile& anim,
    float f)
{
    auto v = float2int(proc, anim, f);
    if (v < 0) {
        anim.RegisterError(ANIMERR_NEGINT, proc);
        return 0;
    }
    else {
        return (unsigned)v;
    }
}

void DoCounterMarch(const ContProcedure& proc, AnimateCompile& anim,
    const ContPoint& pnt1, const ContPoint& pnt2,
    const ContValue& stps, const ContValue& dir1,
    const ContValue& dir2, const ContValue& numbeats)
{
    auto d1 = dir1.Get(anim);
    auto d2 = dir2.Get(anim);
    auto c = sin(Deg2Rad(d1 - d2));
    if (IS_ZERO(c)) {
        anim.RegisterError(ANIMERR_INVALID_CM, &proc);
        return;
    }
    auto ref1 = pnt1.Get(anim);
    auto ref2 = pnt2.Get(anim);
    auto steps1 = stps.Get(anim);
    auto beats = numbeats.Get(anim);

    auto v1 = CreateVector(d1, steps1);

    Coord p[4];
    p[1] = ref1 + v1;
    auto steps2 = (ref2 - p[1]).Magnitude() * sin(Deg2Rad(ref2.Direction(p[1]) - d1)) / c;
    if (IsDiagonalDirection(d2)) {
        steps2 /= static_cast<float>(SQRT2);
    }
    auto v2 = CreateVector(d2, steps2);
    p[2] = p[1] + v2;
    p[3] = ref2 - v1;
    p[0] = p[3] - v2;

    v1 = p[1] - anim.GetPointPosition();
    c = BoundDirectionSigned(v1.Direction() - d1);
    auto leg = 0;
    if ((v1 != 0) && (IS_ZERO(c))) {
        leg = 1;
    }
    else {
        v1 = p[2] - anim.GetPointPosition();
        c = BoundDirectionSigned(v1.Direction() - d2);
        if ((v1 != 0) && (IS_ZERO(c))) {
            leg = 2;
        }
        else {
            v1 = p[3] - anim.GetPointPosition();
            c = BoundDirectionSigned(v1.Direction() - d1 - 180.0f);
            if ((v1 != 0) && (IS_ZERO(c))) {
                leg = 3;
            }
            else {
                v1 = p[0] - anim.GetPointPosition();
                c = BoundDirectionSigned(v1.Direction() - d2 - 180.0f);
                if ((v1 != 0) && (IS_ZERO(c))) {
                    leg = 0;
                }
                else {
                    // Current point is not in path of countermarch
                    anim.RegisterError(ANIMERR_INVALID_CM, &proc);
                    return;
                }
            }
        }
    }

    while (beats > 0) {
        v1 = p[leg] - anim.GetPointPosition();
        c = v1.DM_Magnitude();
        if (c <= beats) {
            beats -= c;
            if (!anim.Append(std::make_shared<AnimateCommandMove>(
                                 float2unsigned(&proc, anim, c), v1),
                    &proc)) {
                return;
            }
        }
        else {
            switch (leg) {
            case 0:
                v1 = CreateVector(d2 + 180.0f, beats);
                break;
            case 1:
                v1 = CreateVector(d1, beats);
                break;
            case 2:
                v1 = CreateVector(d2, beats);
                break;
            default:
                v1 = CreateVector(d1 + 180.0f, beats);
                break;
            }
            anim.Append(std::make_shared<AnimateCommandMove>(
                            float2unsigned(&proc, anim, beats), v1),
                &proc);
            return;
        }
        leg++;
        if (leg > 3)
            leg = 0;
    }
}

ContToken::ContToken()
    : line(yylloc.first_line)
    , col(yylloc.first_column)
{
}
std::ostream& ContToken::Print(std::ostream& os) const
{
    return os << "[" << line << "," << col << "]: ";
}

Coord ContPoint::Get(AnimateCompile& anim) const
{
    return anim.GetPointPosition();
}

std::ostream& ContPoint::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Point:";
}

Coord ContStartPoint::Get(AnimateCompile& anim) const
{
    return anim.GetStartingPosition();
}

std::ostream& ContStartPoint::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Start Point";
}

Coord ContNextPoint::Get(AnimateCompile& anim) const
{
    return anim.GetEndingPosition(this);
}

std::ostream& ContNextPoint::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Next Point";
}

Coord ContRefPoint::Get(AnimateCompile& anim) const
{
    return anim.GetReferencePointPosition(refnum);
}

std::ostream& ContRefPoint::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Ref Point " << refnum;
}

std::ostream& ContValue::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Value:";
}

float ContValueFloat::Get(AnimateCompile&) const { return val; }

std::ostream& ContValueFloat::Print(std::ostream& os) const
{
    super::Print(os);
    return os << val;
}

float ContValueDefined::Get(AnimateCompile&) const
{
    static const std::map<ContDefinedValue, float> mapping = {
        { CC_NW, 45.0 }, { CC_W, 90.0 },
        { CC_SW, 135.0 }, { CC_S, 180.0 },
        { CC_SE, 225.0 }, { CC_E, 270.0 },
        { CC_NE, 315.0 }, { CC_HS, 1.0 },
        { CC_MM, 1.0 }, { CC_SH, 0.5 },
        { CC_JS, 0.5 }, { CC_GV, 1.0 },
        { CC_M, 4.0f / 3 }, { CC_DM, static_cast<float>(SQRT2) },
    };
    auto i = mapping.find(val);
    if (i != mapping.end()) {
        return i->second;
    }
    return 0.0;
}

std::ostream& ContValueDefined::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Defined:" << ContDefinedValue_strings[val];
}

float ContValueAdd::Get(AnimateCompile& anim) const
{
    return (val1->Get(anim) + val2->Get(anim));
}

std::ostream& ContValueAdd::Print(std::ostream& os) const
{
    super::Print(os);
    return os << *val1 << " + " << *val2;
}

float ContValueSub::Get(AnimateCompile& anim) const
{
    return (val1->Get(anim) - val2->Get(anim));
}

std::ostream& ContValueSub::Print(std::ostream& os) const
{
    super::Print(os);
    return os << *val1 << " - " << *val2;
}

float ContValueMult::Get(AnimateCompile& anim) const
{
    return (val1->Get(anim) * val2->Get(anim));
}

std::ostream& ContValueMult::Print(std::ostream& os) const
{
    super::Print(os);
    return os << *val1 << " * " << *val2;
}

float ContValueDiv::Get(AnimateCompile& anim) const
{
    auto f = val2->Get(anim);
    if (IS_ZERO(f)) {
        anim.RegisterError(ANIMERR_DIVISION_ZERO, this);
        return 0.0;
    }
    else {
        return (val1->Get(anim) / f);
    }
}

std::ostream& ContValueDiv::Print(std::ostream& os) const
{
    super::Print(os);
    return os << *val1 << " / " << *val2;
}

float ContValueNeg::Get(AnimateCompile& anim) const { return -val->Get(anim); }

std::ostream& ContValueNeg::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "- " << *val;
}

float ContValueREM::Get(AnimateCompile& anim) const
{
    return anim.GetBeatsRemaining();
}

std::ostream& ContValueREM::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "REM";
}

float ContValueVar::Get(AnimateCompile& anim) const
{
    return anim.GetVarValue(varnum, this);
}

std::ostream& ContValueVar::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Var " << varnum;
}

void ContValueVar::Set(AnimateCompile& anim, float v)
{
    anim.SetVarValue(varnum, v);
}

float ContFuncDir::Get(AnimateCompile& anim) const
{
    auto c = pnt->Get(anim);
    if (c == anim.GetPointPosition()) {
        anim.RegisterError(ANIMERR_UNDEFINED, this);
    }
    return anim.GetPointPosition().Direction(c);
}

std::ostream& ContFuncDir::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Direction to " << *pnt;
}

float ContFuncDirFrom::Get(AnimateCompile& anim) const
{
    auto start = pnt_start->Get(anim);
    auto end = pnt_end->Get(anim);
    if (start == end) {
        anim.RegisterError(ANIMERR_UNDEFINED, this);
    }
    return start.Direction(end);
}

std::ostream& ContFuncDirFrom::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Direction from " << *pnt_start << " to " << *pnt_end;
}

float ContFuncDist::Get(AnimateCompile& anim) const
{
    auto vector = pnt->Get(anim) - anim.GetPointPosition();
    return vector.DM_Magnitude();
}

std::ostream& ContFuncDist::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Distance to " << *pnt;
}

float ContFuncDistFrom::Get(AnimateCompile& anim) const
{
    auto vector = pnt_end->Get(anim) - pnt_start->Get(anim);
    return vector.Magnitude();
}

std::ostream& ContFuncDistFrom::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Distance from " << *pnt_start << " to " << *pnt_end;
}

float ContFuncEither::Get(AnimateCompile& anim) const
{
    auto c = pnt->Get(anim);
    if (anim.GetPointPosition() == c) {
        anim.RegisterError(ANIMERR_UNDEFINED, this);
        return dir1->Get(anim);
    }
    auto dir = anim.GetPointPosition().Direction(c);
    auto d1 = dir1->Get(anim) - dir;
    while (d1 > 180)
        d1 -= 360;
    while (d1 < -180)
        d1 += 360;
    auto d2 = dir2->Get(anim) - dir;
    while (d2 > 180)
        d2 -= 360;
    while (d2 < -180)
        d2 += 360;
    return (std::abs(d1) > std::abs(d2)) ? dir2->Get(anim) : dir1->Get(anim);
}

std::ostream& ContFuncEither::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Either direction to " << *dir1 << " or " << *dir2
              << ", depending on whichever is a shorter angle to " << *pnt;
}

float ContFuncOpp::Get(AnimateCompile& anim) const
{
    return (dir->Get(anim) + 180.0f);
}

std::ostream& ContFuncOpp::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "opposite direction of " << *dir;
}

float ContFuncStep::Get(AnimateCompile& anim) const
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    return (c.DM_Magnitude() * numbeats->Get(anim) / blksize->Get(anim));
}

std::ostream& ContFuncStep::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Step drill at " << *numbeats << " beats for a block size of "
              << *blksize << " from point " << *pnt;
}

std::ostream& ContProcedure::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Procedure: ";
}

void ContProcSet::Compile(AnimateCompile& anim)
{
    var->Set(anim, val->Get(anim));
}

std::ostream& ContProcSet::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Setting variable " << *var << " to " << *val;
}

void ContProcBlam::Compile(AnimateCompile& anim)
{
    ContNextPoint np;
    auto c = np.Get(anim) - anim.GetPointPosition();
    anim.Append(std::make_shared<AnimateCommandMove>(anim.GetBeatsRemaining(), c),
        this);
}

std::ostream& ContProcBlam::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "BLAM";
}

void ContProcCM::Compile(AnimateCompile& anim)
{
    DoCounterMarch(*this, anim, *pnt1, *pnt2, *stps, *dir1, *dir2, *numbeats);
}

std::ostream& ContProcCM::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "CounterMarch starting at " << *pnt1 << " passing through "
              << *pnt2 << " stepping " << *stps << " off points, first moving "
              << *dir1 << " then " << *dir2 << " for number beats " << *numbeats;
}

void ContProcDMCM::Compile(AnimateCompile& anim)
{
    ContValueFloat steps(1.0);

    auto r1 = pnt1->Get(anim);
    auto r2 = pnt2->Get(anim);
    auto c = r2.x - r1.x;
    if (c == (r2.y - r1.y + Int2CoordUnits(2))) {
        if (c >= 0) {
            ContValueDefined dir1(CC_SW);
            ContValueDefined dir2(CC_W);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    }
    else if (c == (r1.y - r2.y - Int2CoordUnits(2))) {
        if (c >= 0) {
            ContValueDefined dir1(CC_SE);
            ContValueDefined dir2(CC_W);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    }
    else if (c == (r1.y - r2.y + Int2CoordUnits(2))) {
        if (c <= 0) {
            ContValueDefined dir1(CC_NW);
            ContValueDefined dir2(CC_E);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    }
    else if (c == (r2.y - r1.y - Int2CoordUnits(2))) {
        if (c <= 0) {
            ContValueDefined dir1(CC_NE);
            ContValueDefined dir2(CC_E);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    }
    anim.RegisterError(ANIMERR_INVALID_CM, this);
}

std::ostream& ContProcDMCM::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Diagonal march CounterMarch starting at " << *pnt1
              << " passing through " << *pnt2 << " for number beats" << *numbeats;
}

void ContProcDMHS::Compile(AnimateCompile& anim)
{
    short b_hs;

    Coord c_hs, c_dm;
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    if (std::abs(c.x) > std::abs(c.y)) {
        // adjust sign
        c_hs.x = ((c.x < 0) != (c.y < 0)) ? c.x + c.y : c.x - c.y;
        c_hs.y = 0;
        // adjust sign
        c_dm.x = ((c.x < 0) != (c.y < 0)) ? -c.y : c.y;
        c_dm.y = c.y;
        b_hs = CoordUnits2Int(c_hs.x);
    }
    else {
        c_hs.x = 0;
        // adjust sign
        c_hs.y = ((c.x < 0) != (c.y < 0)) ? c.y + c.x : c.y - c.x;
        c_dm.x = c.x;
        // adjust sign
        c_dm.y = ((c.x < 0) != (c.y < 0)) ? -c.x : c.x;
        b_hs = CoordUnits2Int(c_hs.y);
    }
    if (c_dm != 0) {
        auto b = CoordUnits2Int(c_dm.x);
        if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c_dm),
                this)) {
            return;
        }
    }
    if (c_hs != 0) {
        anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b_hs), c_hs),
            this);
    }
}

std::ostream& ContProcDMHS::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Diagonal march then HighStep to " << *pnt;
}

void ContProcEven::Compile(AnimateCompile& anim)
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    auto steps = float2int(this, anim, stps->Get(anim));
    if (steps < 0) {
        anim.Append(std::make_shared<AnimateCommandMove>((unsigned)-steps, c,
                        -c.Direction()),
            this);
    }
    else {
        anim.Append(std::make_shared<AnimateCommandMove>((unsigned)steps, c), this);
    }
}

std::ostream& ContProcEven::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Even march of step size " << *stps << " to " << *pnt;
}

void ContProcEWNS::Compile(AnimateCompile& anim)
{
    auto c1 = pnt->Get(anim) - anim.GetPointPosition();
    if (c1.y != 0) {
        Coord c2{ 0, c1.y };
        auto b = CoordUnits2Int(c2.y);
        if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2),
                this)) {
            return;
        }
    }
    if (c1.x != 0) {
        Coord c2{ c1.x, 0 };
        auto b = CoordUnits2Int(c2.x);
        if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2),
                this)) {
            return;
        }
    }
}

std::ostream& ContProcEWNS::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "March EastWest/NorthSouth to " << *pnt;
}

void ContProcFountain::Compile(AnimateCompile& anim)
{
    float a, b, c, d;

    auto f1 = dir1->Get(anim);
    if (stepsize1) {
        auto f2 = stepsize1->Get(anim);
        a = f2 * cos(Deg2Rad(f1));
        c = f2 * -sin(Deg2Rad(f1));
    }
    else {
        std::tie(a, c) = CreateUnitVector(f1);
    }
    f1 = dir2->Get(anim);
    if (stepsize2) {
        auto f2 = stepsize2->Get(anim);
        b = f2 * cos(Deg2Rad(f1));
        d = f2 * -sin(Deg2Rad(f1));
    }
    else {
        std::tie(b, d) = CreateUnitVector(f1);
    }
    auto v = pnt->Get(anim) - anim.GetPointPosition();
    auto e = CoordUnits2Float(v.x);
    auto f = CoordUnits2Float(v.y);
    f1 = a * d - b * c;
    if (IS_ZERO(f1)) {
        if (IS_ZERO(a - b) && IS_ZERO(c - d) && IS_ZERO(e * c - a * f)) {
            // Special case: directions are same
            if (IS_ZERO(c)) {
                f1 = f / c;
            }
            else {
                f1 = e / a;
            }
            if (!anim.Append(std::make_shared<AnimateCommandMove>(
                                 float2unsigned(this, anim, f1), v),
                    this)) {
                return;
            }
        }
        else {
            anim.RegisterError(ANIMERR_INVALID_FNTN, this);
            return;
        }
    }
    else {
        auto f2 = (d * e - b * f) / f1;
        if (!IS_ZERO(f2)) {
            v.x = Float2CoordUnits(f2 * a);
            v.y = Float2CoordUnits(f2 * c);
            if (!anim.Append(std::make_shared<AnimateCommandMove>(
                                 float2unsigned(this, anim, f2), v),
                    this)) {
                return;
            }
        }
        f2 = (a * f - c * e) / f1;
        if (!IS_ZERO(f2)) {
            v.x = Float2CoordUnits(f2 * b);
            v.y = Float2CoordUnits(f2 * d);
            if (!anim.Append(std::make_shared<AnimateCommandMove>(
                                 float2unsigned(this, anim, f2), v),
                    this)) {
                return;
            }
        }
    }
}

std::ostream& ContProcFountain::Print(std::ostream& os) const
{
    super::Print(os);
    os << "Fountain step, first going " << *dir1 << " then " << *dir2;
    if (stepsize1)
        os << ", first at " << *stepsize1;
    if (stepsize2)
        os << ", then at " << *stepsize2;
    return os << "ending at " << *pnt;
}

void ContProcFM::Compile(AnimateCompile& anim)
{
    auto b = float2int(this, anim, stps->Get(anim));
    if (b != 0) {
        auto c = CreateVector(dir->Get(anim), stps->Get(anim));
        if (c != 0) {
            if (b < 0) {
                anim.Append(std::make_shared<AnimateCommandMove>((unsigned)-b, c,
                                -c.Direction()),
                    this);
            }
            else {
                anim.Append(std::make_shared<AnimateCommandMove>((unsigned)b, c), this);
            }
        }
    }
}

std::ostream& ContProcFM::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Forward march for steps " << *stps << " in direction " << *dir;
}

void ContProcFMTO::Compile(AnimateCompile& anim)
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    if (c != 0) {
        anim.Append(
            std::make_shared<AnimateCommandMove>((unsigned)c.DM_Magnitude(), c),
            this);
    }
}

std::ostream& ContProcFMTO::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Forward march to " << *pnt;
}

static inline Coord::units roundcoord(Coord::units a, Coord::units mod)
{
    mod = std::abs(mod);
    if (mod > 0) {
        if (a < 0) {
            a = ((a - (mod / 2)) / mod) * mod;
        }
        else {
            a = ((a + (mod / 2)) / mod) * mod;
        }
    }
    return a;
}

void ContProcGrid::Compile(AnimateCompile& anim)
{
    auto gridc = Float2CoordUnits(grid->Get(anim));

    Coord c;
    c.x = roundcoord(anim.GetPointPosition().x, gridc);
    // Adjust so 4 step grid will be on visible grid
    c.y = roundcoord(anim.GetPointPosition().y - Int2CoordUnits(2), gridc) + Int2CoordUnits(2);

    c -= anim.GetPointPosition();
    if (c != 0) {
        anim.Append(std::make_shared<AnimateCommandMove>(0, c), this);
    }
}

std::ostream& ContProcGrid::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Move on Grid of " << *grid << " spacing";
}

void ContProcHSCM::Compile(AnimateCompile& anim)
{
    ContValueFloat steps(1.0);

    auto r1 = pnt1->Get(anim);
    auto r2 = pnt2->Get(anim);
    if ((r1.y - r2.y) == Int2CoordUnits(2)) {
        if (r2.x >= r1.x) {
            ContValueDefined dirs(CC_S);
            ContValueDefined dirw(CC_W);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dirs, dirw, *numbeats);
            return;
        }
    }
    else if ((r1.y - r2.y) == -Int2CoordUnits(2)) {
        if (r1.x >= r2.x) {
            ContValueDefined dirn(CC_N);
            ContValueDefined dire(CC_E);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dirn, dire, *numbeats);
            return;
        }
    }
    anim.RegisterError(ANIMERR_INVALID_CM, this);
}

std::ostream& ContProcHSCM::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "High Step CounterMarch starting at " << *pnt1
              << " passing through " << *pnt2 << " for number beats" << *numbeats;
}

void ContProcHSDM::Compile(AnimateCompile& anim)
{
    Coord c_hs, c_dm;
    short b;

    auto c = pnt->Get(anim) - anim.GetPointPosition();
    if (std::abs(c.x) > std::abs(c.y)) {
        // adjust sign
        c_hs.x = ((c.x < 0) != (c.y < 0)) ? c.x + c.y : c.x - c.y;
        c_hs.y = 0;
        // adjust sign
        c_dm.x = ((c.x < 0) != (c.y < 0)) ? -c.y : c.y;
        c_dm.y = c.y;
        b = CoordUnits2Int(c_hs.x);
    }
    else {
        c_hs.x = 0;
        // adjust sign
        c_hs.y = ((c.x < 0) != (c.y < 0)) ? c.y + c.x : c.y - c.x;
        c_dm.x = c.x;
        // adjust sign
        c_dm.y = ((c.x < 0) != (c.y < 0)) ? -c.x : c.x;
        b = CoordUnits2Int(c_hs.y);
    }
    if (c_hs != 0) {
        if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c_hs),
                this)) {
            return;
        }
    }
    if (c_dm != 0) {
        b = CoordUnits2Int(c_dm.x);
        anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c_dm), this);
    }
}

std::ostream& ContProcHSDM::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "HighStep then Diagonal march to " << *pnt;
}

void ContProcMagic::Compile(AnimateCompile& anim)
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    anim.Append(std::make_shared<AnimateCommandMove>(0, c), this);
}

std::ostream& ContProcMagic::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Magic step to " << *pnt;
}

void ContProcMarch::Compile(AnimateCompile& anim)
{
    auto b = float2int(this, anim, stps->Get(anim));
    if (b != 0) {
        auto rads = Deg2Rad(dir->Get(anim));
        auto mag = stpsize->Get(anim) * stps->Get(anim);
        Coord c{ Float2CoordUnits(cos(rads) * mag),
            static_cast<Coord::units>(-Float2CoordUnits(sin(rads) * mag)) };
        if (c != 0) {
            if (facedir)
                anim.Append(std::make_shared<AnimateCommandMove>((unsigned)std::abs(b),
                                c, facedir->Get(anim)),
                    this);
            else if (b < 0) {
                anim.Append(std::make_shared<AnimateCommandMove>((unsigned)-b, c,
                                -c.Direction()),
                    this);
            }
            else {
                anim.Append(std::make_shared<AnimateCommandMove>((unsigned)b, c), this);
            }
        }
    }
}

std::ostream& ContProcMarch::Print(std::ostream& os) const
{
    super::Print(os);
    os << "March step size" << *stpsize << " for steps " << *stps
       << " in direction " << *dir;
    if (facedir)
        os << " facing " << *facedir;
    return os;
}

void ContProcMT::Compile(AnimateCompile& anim)
{
    auto b = float2int(this, anim, numbeats->Get(anim));
    if (b != 0) {
        anim.Append(std::make_shared<AnimateCommandMT>((unsigned)std::abs(b),
                        dir->Get(anim)),
            this);
    }
}

std::ostream& ContProcMT::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "MarkTime for " << *numbeats << " facing " << *dir;
}

void ContProcMTRM::Compile(AnimateCompile& anim)
{
    anim.Append(std::make_shared<AnimateCommandMT>(anim.GetBeatsRemaining(),
                    dir->Get(anim)),
        this);
}

std::ostream& ContProcMTRM::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "MarkTime for Remaining Beats facing " << *dir;
}

void ContProcNSEW::Compile(AnimateCompile& anim)
{
    auto c1 = pnt->Get(anim) - anim.GetPointPosition();
    if (c1.x != 0) {
        Coord c2{ c1.x, 0 };
        auto b = CoordUnits2Int(c2.x);
        if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2),
                this)) {
            return;
        }
    }
    if (c1.y != 0) {
        Coord c2{ 0, c1.y };
        auto b = CoordUnits2Int(c2.y);
        if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2),
                this)) {
            return;
        }
    }
}

std::ostream& ContProcNSEW::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "March NorthSouth/EastWest to " << *pnt;
}

void ContProcRotate::Compile(AnimateCompile& anim)
{
    // Most of the work is converting to polar coordinates
    auto c = pnt->Get(anim);
    auto rad = anim.GetPointPosition() - c;
    float start_ang;
    if (c == anim.GetPointPosition())
        start_ang = anim.GetVarValue(CONTVAR_DOH, this);
    else
        start_ang = c.Direction(anim.GetPointPosition());
    int b = float2int(this, anim, stps->Get(anim));
    float angle = ang->Get(anim);
    bool backwards = false;
    if (b < 0) {
        backwards = true;
    }
    anim.Append(std::make_shared<AnimateCommandRotate>(
                    (unsigned)std::abs(b), c,
                    // Don't use Magnitude() because
                    // we want Coord numbers
                    sqrt(static_cast<float>(rad.x * rad.x + rad.y * rad.y)),
                    start_ang, start_ang + angle, backwards),
        this);
}

std::ostream& ContProcRotate::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Rotate at angle " << *ang << " for steps " << *stps
              << " around pivot point " << *pnt;
}
}
