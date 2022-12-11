/*
 * CalChartContinuityToken.cpp
 * Classes for ContinuityTokens
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

#include "CalChartContinuityToken.h"
#include "CalChartAnimationCommand.h"
#include "CalChartAnimationCompile.h"
#include "CalChartAnimationTypes.h"
#include "CalChartSheet.h"
#include "CalChartUtils.h"
#include "parse.h"

#include <cmath>

// for serialization we need to pre-register all of the different types that can exist in the inuity AST.

enum class SerializationToken {
    Token,
    PointUnset,
    Point,
    StartPoint,
    NextPoint,
    RefPoint,
    Value,
    ValueUnset,
    ValueFloat,
    ValueDefined,
    ValueAdd,
    ValueSub,
    ValueMult,
    ValueDiv,
    ValueNeg,
    ValueREM,
    ValueVar,
    ValueVarUnset,
    FuncDir,
    FuncDirFrom,
    FuncDist,
    FuncDistFrom,
    FuncEither,
    FuncOpp,
    FuncStep,
    Procedure,
    ProcUnset,
    ProcSet,
    ProcBlam,
    ProcCM,
    ProcDMCM,
    ProcDMHS,
    ProcEven,
    ProcEWNS,
    ProcFountain,
    ProcFM,
    ProcFMTO,
    ProcGrid,
    ProcHSCM,
    ProcHSDM,
    ProcMagic,
    ProcMarch,
    ProcMT,
    ProcMTRM,
    ProcNSEW,
    ProcRotate,
};

const std::string s_var_names[] = {
    "A",
    "B",
    "C",
    "D",
    "X",
    "Y",
    "Z",
    "DOF",
    "DOH",
};

namespace CalChart::Cont {

static const std::string DefinedValue_strings[] = {
    "N", "NW", "W", "SW", "S", "SE", "E", "NE",
    "HS", "MM", "SH", "JS", "GV", "M", "DM"
};

int float2int(const Procedure* proc, AnimationCompile& anim, float f)
{
    auto v = (int)floor(f + 0.5);
    if (std::abs(f - (float)v) >= kCoordDecimal) {
        anim.RegisterError(AnimateError::NONINT, proc);
    }
    return v;
}

unsigned float2unsigned(const Procedure* proc, AnimationCompile& anim,
    float f)
{
    auto v = float2int(proc, anim, f);
    if (v < 0) {
        anim.RegisterError(AnimateError::NEGINT, proc);
        return 0;
    } else {
        return (unsigned)v;
    }
}

void DoCounterMarch(const Procedure& proc, AnimationCompile& anim,
    const Point& pnt1, const Point& pnt2,
    const Value& stps, const Value& dir1,
    const Value& dir2, const Value& numbeats)
{
    auto d1 = dir1.Get(anim);
    auto d2 = dir2.Get(anim);
    auto c = sin(Deg2Rad(d1 - d2));
    if (IS_ZERO(c)) {
        anim.RegisterError(AnimateError::INVALID_CM, &proc);
        return;
    }
    auto ref1 = pnt1.Get(anim);
    auto ref2 = pnt2.Get(anim);
    auto steps1 = stps.Get(anim);
    auto beats = numbeats.Get(anim);

    auto v1 = CreateCalChartVectorDeg(d1, steps1);

    Coord p[4];
    p[1] = ref1 + v1;
    auto steps2 = (ref2 - p[1]).Magnitude() * sin(Deg2Rad(ref2.DirectionDeg(p[1]) - d1)) / c;
    if (IsDiagonalDirectionDeg(d2)) {
        steps2 /= static_cast<float>(std::numbers::sqrt2);
    }
    auto v2 = CreateCalChartVectorDeg(d2, steps2);
    p[2] = p[1] + v2;
    p[3] = ref2 - v1;
    p[0] = p[3] - v2;

    v1 = p[1] - anim.GetPointPosition();
    c = BoundDirectionSignedDeg(v1.DirectionDeg() - d1);
    auto leg = 0;
    if ((v1 != Coord{ 0 }) && (IS_ZERO(c))) {
        leg = 1;
    } else {
        v1 = p[2] - anim.GetPointPosition();
        c = BoundDirectionSignedDeg(v1.DirectionDeg() - d2);
        if ((v1 != Coord{ 0 }) && (IS_ZERO(c))) {
            leg = 2;
        } else {
            v1 = p[3] - anim.GetPointPosition();
            c = BoundDirectionSignedDeg(v1.DirectionDeg() - d1 - 180.0f);
            if ((v1 != Coord{ 0 }) && (IS_ZERO(c))) {
                leg = 3;
            } else {
                v1 = p[0] - anim.GetPointPosition();
                c = BoundDirectionSignedDeg(v1.DirectionDeg() - d2 - 180.0f);
                if ((v1 != Coord{ 0 }) && (IS_ZERO(c))) {
                    leg = 0;
                } else {
                    // Current point is not in path of countermarch
                    anim.RegisterError(AnimateError::INVALID_CM, &proc);
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
            if (!anim.Append(std::make_unique<AnimationCommandMove>(
                                 float2unsigned(&proc, anim, c), v1),
                    &proc)) {
                return;
            }
        } else {
            switch (leg) {
            case 0:
                v1 = CreateCalChartVectorDeg(d2 + 180.0f, beats);
                break;
            case 1:
                v1 = CreateCalChartVectorDeg(d1, beats);
                break;
            case 2:
                v1 = CreateCalChartVectorDeg(d2, beats);
                break;
            default:
                v1 = CreateCalChartVectorDeg(d1 + 180.0f, beats);
                break;
            }
            anim.Append(std::make_unique<AnimationCommandMove>(
                            float2unsigned(&proc, anim, beats), v1),
                &proc);
            return;
        }
        leg++;
        if (leg > 3)
            leg = 0;
    }
}

#define CheckForToken(reader, minSize, serialToken) CheckForTokenImpl(reader, minSize, serialToken, #serialToken)

template <typename T, typename U>
auto CheckForTokenImpl(Reader reader, size_t minSize, T serialToken, U tokenName)
{
    using namespace std::string_literals;
    if (reader.size() < minSize) {
        throw std::runtime_error("Error, size of "s + tokenName + " is not correct");
    }
    auto token = static_cast<SerializationToken>(reader.Get<uint8_t>());
    if (token != serialToken) {
        throw std::runtime_error("Error, token is not "s + tokenName);
    }
    return reader;
}

std::tuple<std::unique_ptr<Procedure>, Reader> DeserializeProcedure(Reader reader)
{
    if (reader.size() < 1) {
        throw std::runtime_error("Error, size of Point is not correct");
    }
    auto v = std::unique_ptr<Procedure>();
    auto token = static_cast<SerializationToken>(reader.Peek<uint8_t>());
    switch (token) {
    case SerializationToken::ProcUnset:
        v = std::make_unique<ProcUnset>();
        break;
    case SerializationToken::ProcSet:
        v = std::make_unique<ProcSet>();
        break;
    case SerializationToken::ProcBlam:
        v = std::make_unique<ProcBlam>();
        break;
    case SerializationToken::ProcCM:
        v = std::make_unique<ProcCM>();
        break;
    case SerializationToken::ProcDMCM:
        v = std::make_unique<ProcDMCM>();
        break;
    case SerializationToken::ProcDMHS:
        v = std::make_unique<ProcDMHS>();
        break;
    case SerializationToken::ProcEven:
        v = std::make_unique<ProcEven>();
        break;
    case SerializationToken::ProcEWNS:
        v = std::make_unique<ProcEWNS>();
        break;
    case SerializationToken::ProcFountain:
        v = std::make_unique<ProcFountain>();
        break;
    case SerializationToken::ProcFM:
        v = std::make_unique<ProcFM>();
        break;
    case SerializationToken::ProcFMTO:
        v = std::make_unique<ProcFMTO>();
        break;
    case SerializationToken::ProcGrid:
        v = std::make_unique<ProcGrid>();
        break;
    case SerializationToken::ProcHSCM:
        v = std::make_unique<ProcHSCM>();
        break;
    case SerializationToken::ProcHSDM:
        v = std::make_unique<ProcHSDM>();
        break;
    case SerializationToken::ProcMagic:
        v = std::make_unique<ProcMagic>();
        break;
    case SerializationToken::ProcMarch:
        v = std::make_unique<ProcMarch>();
        break;
    case SerializationToken::ProcMT:
        v = std::make_unique<ProcMT>();
        break;
    case SerializationToken::ProcMTRM:
        v = std::make_unique<ProcMTRM>();
        break;
    case SerializationToken::ProcNSEW:
        v = std::make_unique<ProcNSEW>();
        break;
    case SerializationToken::ProcRotate:
        v = std::make_unique<ProcRotate>();
        break;
    default:
        throw std::runtime_error("Error, did not find Point");
    }
    auto b = v->Deserialize(reader);
    return { std::move(v), b };
}

static std::tuple<std::unique_ptr<Point>, Reader> DeserializePoint(Reader reader)
{
    if (reader.size() < 1) {
        throw std::runtime_error("Error, size of Point is not correct");
    }
    auto v = std::unique_ptr<Point>();
    auto token = static_cast<SerializationToken>(reader.Peek<uint8_t>());
    switch (token) {
    case SerializationToken::Point:
        v = std::make_unique<Point>();
        break;
    case SerializationToken::PointUnset:
        v = std::make_unique<PointUnset>();
        break;
    case SerializationToken::StartPoint:
        v = std::make_unique<StartPoint>();
        break;
    case SerializationToken::NextPoint:
        v = std::make_unique<NextPoint>();
        break;
    case SerializationToken::RefPoint:
        v = std::make_unique<RefPoint>();
        break;
    default:
        throw std::runtime_error("Error, did not find Point");
    }
    auto new_reader = v->Deserialize(reader);
    return { std::move(v), new_reader };
}

static std::tuple<std::unique_ptr<Value>, Reader> DeserializeValue(Reader reader)
{
    if (reader.size() < 1) {
        throw std::runtime_error("Error, size of Point is not correct");
    }
    auto v = std::unique_ptr<Value>();
    auto token = static_cast<SerializationToken>(reader.Peek<uint8_t>());
    switch (token) {
    case SerializationToken::ValueUnset:
        v = std::make_unique<ValueUnset>();
        break;
    case SerializationToken::ValueFloat:
        v = std::make_unique<ValueFloat>();
        break;
    case SerializationToken::ValueDefined:
        v = std::make_unique<ValueDefined>();
        break;
    case SerializationToken::ValueAdd:
        v = std::make_unique<ValueAdd>();
        break;
    case SerializationToken::ValueSub:
        v = std::make_unique<ValueSub>();
        break;
    case SerializationToken::ValueMult:
        v = std::make_unique<ValueMult>();
        break;
    case SerializationToken::ValueDiv:
        v = std::make_unique<ValueDiv>();
        break;
    case SerializationToken::ValueNeg:
        v = std::make_unique<ValueNeg>();
        break;
    case SerializationToken::ValueREM:
        v = std::make_unique<ValueREM>();
        break;
    case SerializationToken::ValueVar:
        v = std::make_unique<ValueVar>();
        break;
    case SerializationToken::FuncDir:
        v = std::make_unique<FuncDir>();
        break;
    case SerializationToken::FuncDirFrom:
        v = std::make_unique<FuncDirFrom>();
        break;
    case SerializationToken::FuncDist:
        v = std::make_unique<FuncDist>();
        break;
    case SerializationToken::FuncDistFrom:
        v = std::make_unique<FuncDistFrom>();
        break;
    case SerializationToken::FuncEither:
        v = std::make_unique<FuncEither>();
        break;
    case SerializationToken::FuncOpp:
        v = std::make_unique<FuncOpp>();
        break;
    case SerializationToken::FuncStep:
        v = std::make_unique<FuncStep>();
        break;
    default:
        throw std::runtime_error("Error, did not find Value");
    }
    auto b = v->Deserialize(reader);
    return { std::move(v), b };
}

static std::tuple<std::unique_ptr<ValueVar>, Reader> DeserializeValueVar(Reader reader)
{
    if (reader.size() < 1) {
        throw std::runtime_error("Error, size of Point is not correct");
    }
    auto v = std::unique_ptr<ValueVar>();
    auto token = static_cast<SerializationToken>(reader.Peek<uint8_t>());
    switch (token) {
    case SerializationToken::ValueVar:
        v = std::make_unique<ValueVar>();
        break;
    case SerializationToken::ValueVarUnset:
        v = std::make_unique<ValueVarUnset>();
        break;
    default:
        throw std::runtime_error("Error, did not find ValueVar");
    }
    auto b = v->Deserialize(reader);
    return { std::move(v), b };
}

// Token
Token::Token()
    : line(yylloc.first_line)
    , col(yylloc.first_column)
{
}

std::ostream& Token::Print(std::ostream& os) const
{
    os << "[CT]";
    return os;
}

void Token::replace(Token const* /*which*/, std::unique_ptr<Token> /*v*/)
{
    throw std::runtime_error("Error, replace not implemented on this class");
}

std::vector<uint8_t> Token::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::Token));
    Parser::Append(result, static_cast<uint32_t>(line));
    Parser::Append(result, static_cast<uint32_t>(col));
    return result;
}

Reader Token::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 9, SerializationToken::Token);
    line = reader.Get<uint32_t>();
    col = reader.Get<uint32_t>();
    return reader;
}

// Point
Coord Point::Get(AnimationCompile const& anim) const
{
    return anim.GetPointPosition();
}

std::ostream& Point::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CP]";
    return os << "Point:";
}

Drawable Point::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::point,
        "Point",
        "P",
        {}
    };
}

std::vector<uint8_t> Point::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::Point));
    Parser::Append(result, super::Serialize());
    return result;
}

Reader Point::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::Point);
    return super::Deserialize(reader);
}

// PointUnset
std::ostream& PointUnset::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPU]";
    return os << "Unset";
}

Drawable PointUnset::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::unset,
        "unset point",
        "unset point",
        {}
    };
}

std::vector<uint8_t> PointUnset::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::PointUnset));
    Parser::Append(result, super::Serialize());
    return result;
}

Reader PointUnset::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::PointUnset);
    return super::Deserialize(reader);
}

// StartPoint
Coord StartPoint::Get(AnimationCompile const& anim) const
{
    return anim.GetStartingPosition();
}

std::ostream& StartPoint::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CSP]";
    return os << "Start Point";
}

Drawable StartPoint::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::point,
        "Start Point",
        "SP",
        {}
    };
}

std::vector<uint8_t> StartPoint::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::StartPoint));
    Parser::Append(result, super::Serialize());
    return result;
}

Reader StartPoint::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::StartPoint);
    return super::Deserialize(reader);
}

// NextPoint
Coord NextPoint::Get(AnimationCompile const& anim) const
{
    return anim.GetEndingPosition(this);
}

std::ostream& NextPoint::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CNP]";
    return os << "Next Point";
}

Drawable NextPoint::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::point,
        "Next Point",
        "NP",
        {}
    };
}

std::vector<uint8_t> NextPoint::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::NextPoint));
    Parser::Append(result, super::Serialize());
    return result;
}

Reader NextPoint::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::NextPoint);
    return super::Deserialize(reader);
}

// RefPoint
RefPoint::RefPoint(unsigned n)
    : refnum(n)
{
}

Coord RefPoint::Get(AnimationCompile const& anim) const
{
    return anim.GetReferencePointPosition(refnum);
}

std::ostream& RefPoint::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CRP]";
    return os << "Ref Point " << refnum;
}

Drawable RefPoint::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::point,
        std::string("Ref Point ") + std::to_string(refnum),
        std::string("R") + std::to_string(refnum),
        {}
    };
}

std::vector<uint8_t> RefPoint::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::RefPoint));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, refnum);
    return result;
}

Reader RefPoint::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 5, SerializationToken::RefPoint);
    reader = super::Deserialize(reader);
    refnum = reader.Get<uint32_t>();
    return reader;
}

// Value
std::ostream& Value::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CV]";
    return os << "Value:";
}

std::vector<uint8_t> Value::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::Value));
    Parser::Append(result, super::Serialize());
    return result;
}

Reader Value::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::Value);
    return super::Deserialize(reader);
}

// ValueUnset
std::ostream& ValueUnset::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CVU]";
    return os << "Unset";
}

Drawable ValueUnset::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::unset,
        "unset value",
        "unset value",
        {}
    };
}

std::vector<uint8_t> ValueUnset::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ValueUnset));
    Parser::Append(result, super::Serialize());
    return result;
}

Reader ValueUnset::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ValueUnset);
    return super::Deserialize(reader);
}

// ValueFloat
ValueFloat::ValueFloat(float v)
    : val(v)
{
}

float ValueFloat::Get(AnimationCompile const&) const { return val; }

std::ostream& ValueFloat::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CVF]";
    return os << val;
}

Drawable ValueFloat::GetDrawable() const
{
    // to_string gives a lot of decimal points.  256 on the stack should be ok...?
    if (int(val) == val) {
        return { this, parent_ptr, Type::value, std::to_string(int(val)), std::to_string(int(val)), {} };
    }
    return { this, parent_ptr, Type::value, std::to_string(val), std::to_string(val), {} };
}

std::vector<uint8_t> ValueFloat::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ValueFloat));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, val);
    return result;
}

Reader ValueFloat::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 5, SerializationToken::ValueFloat);
    reader = super::Deserialize(reader);
    val = reader.Get<float>();
    return reader;
}

// ValueDefined
ValueDefined::ValueDefined(DefinedValue v)
    : val(v)
{
}

float ValueDefined::Get(AnimationCompile const&) const
{
    static const std::map<DefinedValue, float> mapping = {
        { CC_NW, 45.0 },
        { CC_W, 90.0 },
        { CC_SW, 135.0 },
        { CC_S, 180.0 },
        { CC_SE, 225.0 },
        { CC_E, 270.0 },
        { CC_NE, 315.0 },
        { CC_HS, 1.0 },
        { CC_MM, 1.0 },
        { CC_SH, 0.5 },
        { CC_JS, 0.5 },
        { CC_GV, 1.0 },
        { CC_M, 4.0f / 3 },
        { CC_DM, static_cast<float>(std::numbers::sqrt2) },
    };
    auto i = mapping.find(val);
    if (i != mapping.end()) {
        return i->second;
    }
    return 0.0;
}

std::ostream& ValueDefined::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CVC]";
    return os << "Defined:" << DefinedValue_strings[val];
}

Drawable ValueDefined::GetDrawable() const
{
    // to_string gives a lot of decimal points.  256 on the stack should be ok...?
    auto type = Type::value;
    switch (val) {
    case CC_NW:
    case CC_W:
    case CC_SW:
    case CC_S:
    case CC_SE:
    case CC_E:
    case CC_NE:
    case CC_N:
        type = Type::direction;
        break;
    default:
        type = Type::steptype;
    }
    return { this, parent_ptr, type, DefinedValue_strings[val], DefinedValue_strings[val], {} };
}

std::vector<uint8_t> ValueDefined::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ValueDefined));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, static_cast<uint8_t>(val));
    return result;
}

Reader ValueDefined::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 2, SerializationToken::ValueDefined);
    reader = super::Deserialize(reader);
    val = static_cast<DefinedValue>(reader.Get<uint8_t>());
    return reader;
}

// ValueAdd
float ValueAdd::Get(AnimationCompile const& anim) const
{
    return (val1->Get(anim) + val2->Get(anim));
}

std::ostream& ValueAdd::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CVA]";
    return os << *val1 << " + " << *val2;
}

Drawable ValueAdd::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::function,
        "( %@ + %@ )",
        "(%@+%@)",
        { val1->GetDrawable(), val2->GetDrawable() }
    };
}

void ValueAdd::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, val1, val2);
}

std::vector<uint8_t> ValueAdd::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ValueAdd));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, val1->Serialize());
    Parser::Append(result, val2->Serialize());
    return result;
}

Reader ValueAdd::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ValueAdd);
    reader = super::Deserialize(reader);
    std::tie(val1, reader) = DeserializeValue(reader);
    std::tie(val2, reader) = DeserializeValue(reader);
    return reader;
}

// ValueSub
float ValueSub::Get(AnimationCompile const& anim) const
{
    return (val1->Get(anim) - val2->Get(anim));
}

std::ostream& ValueSub::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CVS]";
    return os << *val1 << " - " << *val2;
}

Drawable ValueSub::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::function,
        "( %@ - %@ )",
        "(%@-%@)",
        { val1->GetDrawable(), val2->GetDrawable() }
    };
}

void ValueSub::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, val1, val2);
}

std::vector<uint8_t> ValueSub::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ValueSub));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, val1->Serialize());
    Parser::Append(result, val2->Serialize());
    return result;
}

Reader ValueSub::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ValueSub);
    reader = super::Deserialize(reader);
    std::tie(val1, reader) = DeserializeValue(reader);
    std::tie(val2, reader) = DeserializeValue(reader);
    return reader;
}

// ValueMult
float ValueMult::Get(AnimationCompile const& anim) const
{
    return (val1->Get(anim) * val2->Get(anim));
}

std::ostream& ValueMult::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CVM]";
    return os << *val1 << " * " << *val2;
}

Drawable ValueMult::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::function,
        "( %@ * %@ )",
        "(%@*%@)",
        { val1->GetDrawable(), val2->GetDrawable() }
    };
}

void ValueMult::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, val1, val2);
}

std::vector<uint8_t> ValueMult::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ValueMult));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, val1->Serialize());
    Parser::Append(result, val2->Serialize());
    return result;
}

Reader ValueMult::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ValueMult);
    reader = super::Deserialize(reader);
    std::tie(val1, reader) = DeserializeValue(reader);
    std::tie(val2, reader) = DeserializeValue(reader);
    return reader;
}

// ValueDiv
float ValueDiv::Get(AnimationCompile const& anim) const
{
    auto f = val2->Get(anim);
    if (IS_ZERO(f)) {
        anim.RegisterError(AnimateError::DIVISION_ZERO, this);
        return 0.0;
    } else {
        return (val1->Get(anim) / f);
    }
}

std::ostream& ValueDiv::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CVD]";
    return os << *val1 << " / " << *val2;
}

Drawable ValueDiv::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::function,
        "( %@ / %@ )",
        "(%@/%@)",
        { val1->GetDrawable(), val2->GetDrawable() }
    };
}

void ValueDiv::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, val1, val2);
}

std::vector<uint8_t> ValueDiv::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ValueDiv));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, val1->Serialize());
    Parser::Append(result, val2->Serialize());
    return result;
}

Reader ValueDiv::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ValueDiv);
    reader = super::Deserialize(reader);
    std::tie(val1, reader) = DeserializeValue(reader);
    std::tie(val2, reader) = DeserializeValue(reader);
    return reader;
}

// ValueNeg
float ValueNeg::Get(AnimationCompile const& anim) const { return -val->Get(anim); }

std::ostream& ValueNeg::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CVN]";
    return os << "- " << *val;
}

Drawable ValueNeg::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::function,
        "-%@",
        "-%@",
        { val->GetDrawable() }
    };
}

void ValueNeg::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, val);
}

std::vector<uint8_t> ValueNeg::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ValueNeg));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, val->Serialize());
    return result;
}

Reader ValueNeg::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ValueNeg);
    reader = super::Deserialize(reader);
    std::tie(val, reader) = DeserializeValue(reader);
    return reader;
}

// ValueREM
float ValueREM::Get(AnimationCompile const& anim) const
{
    return anim.GetBeatsRemaining();
}

std::ostream& ValueREM::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CVR]";
    return os << "REM";
}

Drawable ValueREM::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::value,
        "Remaining",
        "REM",
        {}
    };
}

std::vector<uint8_t> ValueREM::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ValueREM));
    Parser::Append(result, super::Serialize());
    return result;
}

Reader ValueREM::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ValueREM);
    return super::Deserialize(reader);
}

// ValueVar
ValueVar::ValueVar(Cont::Variable num)
    : varnum(num)
{
}

float ValueVar::Get(AnimationCompile const& anim) const
{
    return anim.GetVarValue(varnum, this);
}

std::ostream& ValueVar::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CVV]";
    return os << "Var " << toUType(varnum);
}

Drawable ValueVar::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::value,
        s_var_names[toUType(varnum)],
        s_var_names[toUType(varnum)],
        {}
    };
}

void ValueVar::Set(AnimationCompile& anim, float v)
{
    anim.SetVarValue(varnum, v);
}

std::vector<uint8_t> ValueVar::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ValueVar));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, static_cast<uint8_t>(varnum));
    return result;
}

Reader ValueVar::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ValueVar);
    reader = super::Deserialize(reader);
    varnum = static_cast<Cont::Variable>(reader.Get<uint8_t>());
    return reader;
}

// ValueVarUnset
std::ostream& ValueVarUnset::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CVVU]";
    return os << "Unset";
}

Drawable ValueVarUnset::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::unset,
        "unset value var",
        "unset value var",
        {}
    };
}

std::vector<uint8_t> ValueVarUnset::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ValueVarUnset));
    Parser::Append(result, super::Serialize());
    return result;
}

Reader ValueVarUnset::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ValueVarUnset);
    return super::Deserialize(reader);
}

// FuncDir
float FuncDir::Get(AnimationCompile const& anim) const
{
    auto c = pnt->Get(anim);
    if (c == anim.GetPointPosition()) {
        anim.RegisterError(AnimateError::UNDEFINED, this);
    }
    return anim.GetPointPosition().DirectionDeg(c);
}

std::ostream& FuncDir::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CFD]";
    return os << "Direction to " << *pnt;
}

Drawable FuncDir::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::function,
        "Direction to %@",
        "DIR %@",
        { pnt->GetDrawable() }
    };
}

void FuncDir::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt);
}

std::vector<uint8_t> FuncDir::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::FuncDir));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, pnt->Serialize());
    return result;
}

Reader FuncDir::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::FuncDir);
    reader = super::Deserialize(reader);
    std::tie(pnt, reader) = DeserializePoint(reader);
    return reader;
}

// FuncDirFrom
float FuncDirFrom::Get(AnimationCompile const& anim) const
{
    auto start = pnt_start->Get(anim);
    auto end = pnt_end->Get(anim);
    if (start == end) {
        anim.RegisterError(AnimateError::UNDEFINED, this);
    }
    return start.DirectionDeg(end);
}

std::ostream& FuncDirFrom::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CFDF]";
    return os << "Direction from " << *pnt_start << " to " << *pnt_end;
}

Drawable FuncDirFrom::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::function,
        "Direction from %@ to %@",
        "DIRFROM %@ to %@",
        { pnt_start->GetDrawable(), pnt_end->GetDrawable() }
    };
}

void FuncDirFrom::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt_start, pnt_end);
}

std::vector<uint8_t> FuncDirFrom::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::FuncDirFrom));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, pnt_start->Serialize());
    Parser::Append(result, pnt_end->Serialize());
    return result;
}

Reader FuncDirFrom::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::FuncDirFrom);
    reader = super::Deserialize(reader);
    std::tie(pnt_start, reader) = DeserializePoint(reader);
    std::tie(pnt_end, reader) = DeserializePoint(reader);
    return reader;
}

// FuncDist
float FuncDist::Get(AnimationCompile const& anim) const
{
    auto vector = pnt->Get(anim) - anim.GetPointPosition();
    return vector.DM_Magnitude();
}

std::ostream& FuncDist::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CFd]";
    return os << "Distance to " << *pnt;
}

Drawable FuncDist::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::function,
        "Distance to %@",
        "DIST %@",
        { pnt->GetDrawable() }
    };
}

void FuncDist::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt);
}

std::vector<uint8_t> FuncDist::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::FuncDist));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, pnt->Serialize());
    return result;
}

Reader FuncDist::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::FuncDist);
    reader = super::Deserialize(reader);
    std::tie(pnt, reader) = DeserializePoint(reader);
    return reader;
}

// FuncDistFrom
float FuncDistFrom::Get(AnimationCompile const& anim) const
{
    auto vector = pnt_end->Get(anim) - pnt_start->Get(anim);
    return vector.Magnitude();
}

std::ostream& FuncDistFrom::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CFdF]";
    return os << "Distance from " << *pnt_start << " to " << *pnt_end;
}

Drawable FuncDistFrom::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::function,
        "Distance from %@ to %@",
        "DISTFROM %@ to %@",
        { pnt_start->GetDrawable(), pnt_end->GetDrawable() }
    };
}

void FuncDistFrom::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt_start, pnt_end);
}

std::vector<uint8_t> FuncDistFrom::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::FuncDistFrom));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, pnt_start->Serialize());
    Parser::Append(result, pnt_end->Serialize());
    return result;
}

Reader FuncDistFrom::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::FuncDistFrom);
    reader = super::Deserialize(reader);
    std::tie(pnt_start, reader) = DeserializePoint(reader);
    std::tie(pnt_end, reader) = DeserializePoint(reader);
    return reader;
}

// FuncEither
float FuncEither::Get(AnimationCompile const& anim) const
{
    auto c = pnt->Get(anim);
    if (anim.GetPointPosition() == c) {
        anim.RegisterError(AnimateError::UNDEFINED, this);
        return dir1->Get(anim);
    }
    auto dir = anim.GetPointPosition().DirectionDeg(c);
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

std::ostream& FuncEither::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CFE]";
    return os << "Either direction to " << *dir1 << " or " << *dir2
              << ", depending on whichever is a shorter angle to " << *pnt;
}

Drawable FuncEither::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::function,
        "Either direction to %@ or %@, depending on whichever is a shorter angle to %@",
        "EITHER %@ or %@, by %@",
        { dir1->GetDrawable(), dir2->GetDrawable(), pnt->GetDrawable() }
    };
}

void FuncEither::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, dir1, dir2, pnt);
}

std::vector<uint8_t> FuncEither::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::FuncEither));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, dir1->Serialize());
    Parser::Append(result, dir2->Serialize());
    Parser::Append(result, pnt->Serialize());
    return result;
}

Reader FuncEither::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::FuncEither);
    reader = super::Deserialize(reader);
    std::tie(dir1, reader) = DeserializeValue(reader);
    std::tie(dir2, reader) = DeserializeValue(reader);
    std::tie(pnt, reader) = DeserializePoint(reader);
    return reader;
}

// FuncOpp
float FuncOpp::Get(AnimationCompile const& anim) const
{
    return (dir->Get(anim) + 180.0f);
}

std::ostream& FuncOpp::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CFO]";
    return os << "opposite direction of " << *dir;
}

Drawable FuncOpp::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::function,
        "opposite direction of %@",
        "OPP %@",
        { dir->GetDrawable() }
    };
}

void FuncOpp::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, dir);
}

std::vector<uint8_t> FuncOpp::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::FuncOpp));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, dir->Serialize());
    return result;
}

Reader FuncOpp::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::FuncOpp);
    reader = super::Deserialize(reader);
    std::tie(dir, reader) = DeserializeValue(reader);
    return reader;
}

// FuncStep

float FuncStep::Get(AnimationCompile const& anim) const
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    return (c.DM_Magnitude() * numbeats->Get(anim) / blksize->Get(anim));
}

std::ostream& FuncStep::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CFS]";
    return os << "Step drill at " << *numbeats << " beats for a block size of "
              << *blksize << " from point " << *pnt;
}

Drawable FuncStep::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::function,
        "Step drill at %@ beats for a block size of %@ from point %@",
        "STEP %@ Beats, %@ size, from %@",
        { numbeats->GetDrawable(), blksize->GetDrawable(), pnt->GetDrawable() }
    };
}

void FuncStep::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, numbeats, blksize, pnt);
}

std::vector<uint8_t> FuncStep::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::FuncStep));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, numbeats->Serialize());
    Parser::Append(result, blksize->Serialize());
    Parser::Append(result, pnt->Serialize());
    return result;
}

Reader FuncStep::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::FuncStep);
    reader = super::Deserialize(reader);
    std::tie(numbeats, reader) = DeserializeValue(reader);
    std::tie(blksize, reader) = DeserializeValue(reader);
    std::tie(pnt, reader) = DeserializePoint(reader);
    return reader;
}

// Procedure
std::ostream& Procedure::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPr]";
    return os << "Procedure: ";
}

std::vector<uint8_t> Procedure::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::Procedure));
    Parser::Append(result, super::Serialize());
    return result;
}

Reader Procedure::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::Procedure);
    return super::Deserialize(reader);
}

// ProcUnset
std::ostream& ProcUnset::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrU]";
    return os << "Unset";
}

Drawable ProcUnset::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::unset,
        "unset continuity",
        "unset continuity",
        {}
    };
}

std::vector<uint8_t> ProcUnset::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcUnset));
    Parser::Append(result, super::Serialize());
    return result;
}

Reader ProcUnset::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcUnset);
    return super::Deserialize(reader);
}

// ProcSet
void ProcSet::Compile(AnimationCompile& anim)
{
    var->Set(anim, val->Get(anim));
}

std::ostream& ProcSet::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrS]";
    return os << "Setting variable " << *var << " to " << *val;
}

Drawable ProcSet::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "variable %@ = %@",
        "%@ = %@",
        { var->GetDrawable(), val->GetDrawable() }
    };
}

std::unique_ptr<Procedure> ProcSet::clone() const
{
    // we need to make a copy of the var, then dynamically cast to std::unique_ptr<ValueVar>
    auto var_clone = var->clone();
    if (ValueVar* cast = dynamic_cast<ValueVar*>(var_clone.get())) {
        std::unique_ptr<ValueVar> t(cast);
        var_clone.release();
        return std::make_unique<ProcSet>(std::move(t), val->clone());
    }
    throw std::runtime_error("ProcSet var was not of type ValueVar");
}

void ProcSet::replace(Token const* which, std::unique_ptr<Token> v)
{
    if (var.get() == which) {
        // ProcSet is different because we Must have a ValueVar as when replacing
        auto result = dynamic_cast<ValueVar*>(v.get());
        if (!result) {
            throw ReplaceError_NotAVar{};
        }
        var = dynamic_unique_ptr_cast<ValueVar>(std::move(v));
    }
    if (val.get() == which) {
        auto result = dynamic_cast<Value*>(v.get());
        if (!result) {
            throw std::runtime_error("Invalid value in replace");
        }
        val = dynamic_unique_ptr_cast<Value>(std::move(v));
    }
    SetParentPtr_helper(this, var, val);
}

std::vector<uint8_t> ProcSet::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcSet));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, var->Serialize());
    Parser::Append(result, val->Serialize());
    return result;
}

Reader ProcSet::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcSet);
    reader = super::Deserialize(reader);
    std::tie(var, reader) = DeserializeValueVar(reader);
    std::tie(val, reader) = DeserializeValue(reader);
    return reader;
}

// ProcBlam
void ProcBlam::Compile(AnimationCompile& anim)
{
    NextPoint np;
    auto c = np.Get(anim) - anim.GetPointPosition();
    anim.Append(std::make_unique<AnimationCommandMove>(anim.GetBeatsRemaining(), c),
        this);
}

std::ostream& ProcBlam::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrB]";
    return os << "BLAM";
}

Drawable ProcBlam::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "BLAM",
        "BLAM",
        {}
    };
}

std::vector<uint8_t> ProcBlam::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcBlam));
    Parser::Append(result, super::Serialize());
    return result;
}

Reader ProcBlam::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcBlam);
    return super::Deserialize(reader);
}

// ProcCM
void ProcCM::Compile(AnimationCompile& anim)
{
    DoCounterMarch(*this, anim, *pnt1, *pnt2, *stps, *dir1, *dir2, *numbeats);
}

std::ostream& ProcCM::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrCM]";
    return os << "CounterMarch starting at " << *pnt1 << " passing through "
              << *pnt2 << " stepping " << *stps << " off points, first moving "
              << *dir1 << " then " << *dir2 << " for number beats " << *numbeats;
}

Drawable ProcCM::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "CounterMarch starting at %@ passing through %@ stepping %@ off points, first moving %@ then %@ for number beats %@",
        "COUNTERMARCH %@ %@ %@, first %@ then %@ for beats %@",
        { pnt1->GetDrawable(), pnt2->GetDrawable(), stps->GetDrawable(), dir1->GetDrawable(), dir2->GetDrawable(), numbeats->GetDrawable() }
    };
}

void ProcCM::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt1, pnt2, stps, dir1, dir2, numbeats);
}

std::vector<uint8_t> ProcCM::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcCM));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, pnt1->Serialize());
    Parser::Append(result, pnt2->Serialize());
    Parser::Append(result, stps->Serialize());
    Parser::Append(result, dir1->Serialize());
    Parser::Append(result, dir2->Serialize());
    Parser::Append(result, numbeats->Serialize());
    return result;
}

Reader ProcCM::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcCM);
    reader = super::Deserialize(reader);
    std::tie(pnt1, reader) = DeserializePoint(reader);
    std::tie(pnt2, reader) = DeserializePoint(reader);
    std::tie(stps, reader) = DeserializeValue(reader);
    std::tie(dir1, reader) = DeserializeValue(reader);
    std::tie(dir2, reader) = DeserializeValue(reader);
    std::tie(numbeats, reader) = DeserializeValue(reader);
    return reader;
}

// ProcDMCM
void ProcDMCM::Compile(AnimationCompile& anim)
{
    ValueFloat steps(1.0);

    auto r1 = pnt1->Get(anim);
    auto r2 = pnt2->Get(anim);
    auto c = r2.x - r1.x;
    if (c == (r2.y - r1.y + Int2CoordUnits(2))) {
        if (c >= 0) {
            ValueDefined dir1(CC_SW);
            ValueDefined dir2(CC_W);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    } else if (c == (r1.y - r2.y - Int2CoordUnits(2))) {
        if (c >= 0) {
            ValueDefined dir1(CC_SE);
            ValueDefined dir2(CC_W);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    } else if (c == (r1.y - r2.y + Int2CoordUnits(2))) {
        if (c <= 0) {
            ValueDefined dir1(CC_NW);
            ValueDefined dir2(CC_E);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    } else if (c == (r2.y - r1.y - Int2CoordUnits(2))) {
        if (c <= 0) {
            ValueDefined dir1(CC_NE);
            ValueDefined dir2(CC_E);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    }
    anim.RegisterError(AnimateError::INVALID_CM, this);
}

std::ostream& ProcDMCM::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrDC]";
    return os << "Diagonal march CounterMarch starting at " << *pnt1
              << " passing through " << *pnt2 << " for number beats" << *numbeats;
}

Drawable ProcDMCM::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "Diagonal march CounterMarch starting at %@ passing through %@ for number beats %@",
        "DMCM %@ %@ for beats %@",
        { pnt1->GetDrawable(), pnt2->GetDrawable(), numbeats->GetDrawable() }
    };
}

void ProcDMCM::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt1, pnt2, numbeats);
}

std::vector<uint8_t> ProcDMCM::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcDMCM));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, pnt1->Serialize());
    Parser::Append(result, pnt2->Serialize());
    Parser::Append(result, numbeats->Serialize());
    return result;
}

Reader ProcDMCM::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcDMCM);
    reader = super::Deserialize(reader);
    std::tie(pnt1, reader) = DeserializePoint(reader);
    std::tie(pnt2, reader) = DeserializePoint(reader);
    std::tie(numbeats, reader) = DeserializeValue(reader);
    return reader;
}

// ProcDMHS
void ProcDMHS::Compile(AnimationCompile& anim)
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
    } else {
        c_hs.x = 0;
        // adjust sign
        c_hs.y = ((c.x < 0) != (c.y < 0)) ? c.y + c.x : c.y - c.x;
        c_dm.x = c.x;
        // adjust sign
        c_dm.y = ((c.x < 0) != (c.y < 0)) ? -c.x : c.x;
        b_hs = CoordUnits2Int(c_hs.y);
    }
    if (c_dm != Coord{ 0 }) {
        auto b = CoordUnits2Int(c_dm.x);
        if (!anim.Append(std::make_unique<AnimationCommandMove>(std::abs(b), c_dm),
                this)) {
            return;
        }
    }
    if (c_hs != Coord{ 0 }) {
        anim.Append(std::make_unique<AnimationCommandMove>(std::abs(b_hs), c_hs),
            this);
    }
}

std::ostream& ProcDMHS::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrDH]";
    return os << "Diagonal march then HighStep to " << *pnt;
}

Drawable ProcDMHS::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "Diagonal march then HighStep to %@",
        "DMHS %@",
        { pnt->GetDrawable() }
    };
}

void ProcDMHS::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt);
}

std::vector<uint8_t> ProcDMHS::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcDMHS));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, pnt->Serialize());
    return result;
}

Reader ProcDMHS::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcDMHS);
    reader = super::Deserialize(reader);
    std::tie(pnt, reader) = DeserializePoint(reader);
    return reader;
}

// ProcEven
void ProcEven::Compile(AnimationCompile& anim)
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    auto steps = float2int(this, anim, stps->Get(anim));
    if (steps < 0) {
        anim.Append(std::make_unique<AnimationCommandMove>((unsigned)-steps, c,
                        -c.DirectionDeg()),
            this);
    } else {
        anim.Append(std::make_unique<AnimationCommandMove>((unsigned)steps, c), this);
    }
}

std::ostream& ProcEven::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrE]";
    return os << "Even march of step size " << *stps << " to " << *pnt;
}

Drawable ProcEven::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "Even march %@ to %@",
        "EVEN %@ %@",
        { stps->GetDrawable(), pnt->GetDrawable() }
    };
}

void ProcEven::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, stps, pnt);
}

std::vector<uint8_t> ProcEven::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcEven));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, stps->Serialize());
    Parser::Append(result, pnt->Serialize());
    return result;
}

Reader ProcEven::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcEven);
    reader = super::Deserialize(reader);
    std::tie(stps, reader) = DeserializeValue(reader);
    std::tie(pnt, reader) = DeserializePoint(reader);
    return reader;
}

// ProcEWNS
void ProcEWNS::Compile(AnimationCompile& anim)
{
    auto c1 = pnt->Get(anim) - anim.GetPointPosition();
    if (c1.y != 0) {
        Coord c2{ 0, c1.y };
        auto b = CoordUnits2Int(c2.y);
        if (!anim.Append(std::make_unique<AnimationCommandMove>(std::abs(b), c2),
                this)) {
            return;
        }
    }
    if (c1.x != 0) {
        Coord c2{ c1.x, 0 };
        auto b = CoordUnits2Int(c2.x);
        if (!anim.Append(std::make_unique<AnimationCommandMove>(std::abs(b), c2),
                this)) {
            return;
        }
    }
}

std::ostream& ProcEWNS::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrEWNS]";
    return os << "March EastWest/NorthSouth to " << *pnt;
}

Drawable ProcEWNS::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "EastWest/NorthSouth to %@",
        "EW/NS to %@",
        { pnt->GetDrawable() }
    };
}

void ProcEWNS::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt);
}

std::vector<uint8_t> ProcEWNS::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcEWNS));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, pnt->Serialize());
    return result;
}

Reader ProcEWNS::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcEWNS);
    reader = super::Deserialize(reader);
    std::tie(pnt, reader) = DeserializePoint(reader);
    return reader;
}

// ProcFountain
void ProcFountain::Compile(AnimationCompile& anim)
{
    float a, b, c, d;

    auto f1 = dir1->Get(anim);
    if (stepsize1) {
        auto f2 = stepsize1->Get(anim);
        a = f2 * cos(Deg2Rad(f1));
        c = f2 * -sin(Deg2Rad(f1));
    } else {
        std::tie(a, c) = CreateCalChartUnitVectorDeg(f1);
    }
    f1 = dir2->Get(anim);
    if (stepsize2) {
        auto f2 = stepsize2->Get(anim);
        b = f2 * cos(Deg2Rad(f1));
        d = f2 * -sin(Deg2Rad(f1));
    } else {
        std::tie(b, d) = CreateCalChartUnitVectorDeg(f1);
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
            } else {
                f1 = e / a;
            }
            if (!anim.Append(std::make_unique<AnimationCommandMove>(
                                 float2unsigned(this, anim, f1), v),
                    this)) {
                return;
            }
        } else {
            anim.RegisterError(AnimateError::INVALID_FNTN, this);
            return;
        }
    } else {
        auto f2 = (d * e - b * f) / f1;
        if (!IS_ZERO(f2)) {
            v.x = Float2CoordUnits(f2 * a);
            v.y = Float2CoordUnits(f2 * c);
            if (!anim.Append(std::make_unique<AnimationCommandMove>(
                                 float2unsigned(this, anim, f2), v),
                    this)) {
                return;
            }
        }
        f2 = (a * f - c * e) / f1;
        if (!IS_ZERO(f2)) {
            v.x = Float2CoordUnits(f2 * b);
            v.y = Float2CoordUnits(f2 * d);
            if (!anim.Append(std::make_unique<AnimationCommandMove>(
                                 float2unsigned(this, anim, f2), v),
                    this)) {
                return;
            }
        }
    }
}

std::ostream& ProcFountain::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrF]";
    os << "Fountain step, first going " << *dir1 << " then " << *dir2;
    if (stepsize1)
        os << ", first at " << *stepsize1;
    if (stepsize2)
        os << ", then at " << *stepsize2;
    return os << "ending at " << *pnt;
}

Drawable ProcFountain::GetDrawable() const
{
    if (stepsize1 && stepsize2) {
        return {
            this, parent_ptr,
            Type::procedure,
            "Fountain step, first going %@ then %@, first at %@, then at %@, ending at %@",
            "FOUNTAIN %@ -> %@, Step %@, then %@, ending %@",
            { dir1->GetDrawable(), dir2->GetDrawable(), stepsize1->GetDrawable(), stepsize2->GetDrawable(), pnt->GetDrawable() }
        };
    }
    if (stepsize1) {
        return {
            this, parent_ptr,
            Type::procedure,
            "Fountain step, first going %@ then %@, first at %@, ending at %@",
            "FOUNTAIN %@ -> %@, Step %@ ending %@",
            { dir1->GetDrawable(), dir2->GetDrawable(), stepsize1->GetDrawable(), pnt->GetDrawable() }
        };
    }
    if (stepsize2) {
        return {
            this, parent_ptr,
            Type::procedure,
            "Fountain step, first going %@ then %@, then at %@, ending at %@",
            "FOUNTAIN %@ -> %@, Step %@, ending %@",
            { dir1->GetDrawable(), dir2->GetDrawable(), stepsize2->GetDrawable(), pnt->GetDrawable() }
        };
    }
    return {
        this, parent_ptr,
        Type::procedure,
        "Fountain step, first going %@ then %@, ending at %@",
        "FOUNTAIN %@ -> %@, ending %@",
        { dir1->GetDrawable(), dir2->GetDrawable(), pnt->GetDrawable() }
    };
}

void ProcFountain::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, dir1, dir2, stepsize1, stepsize2, pnt);
}

std::vector<uint8_t> ProcFountain::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcFountain));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, dir1->Serialize());
    Parser::Append(result, dir2->Serialize());
    Parser::Append(result, uint8_t(stepsize1 != nullptr));
    if (stepsize1) {
        Parser::Append(result, stepsize1->Serialize());
    }
    Parser::Append(result, uint8_t(stepsize2 != nullptr));
    if (stepsize2) {
        Parser::Append(result, stepsize2->Serialize());
    }
    Parser::Append(result, pnt->Serialize());
    return result;
}

Reader ProcFountain::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcFountain);
    reader = super::Deserialize(reader);
    std::tie(dir1, reader) = DeserializeValue(reader);
    std::tie(dir2, reader) = DeserializeValue(reader);
    bool parsestepsize1 = reader.Get<uint8_t>();
    if (parsestepsize1) {
        std::tie(stepsize1, reader) = DeserializeValue(reader);
    }
    bool parsestepsize2 = reader.Get<uint8_t>();
    if (parsestepsize2) {
        std::tie(stepsize2, reader) = DeserializeValue(reader);
    }
    std::tie(pnt, reader) = DeserializePoint(reader);
    return reader;
}

// ProcFM
void ProcFM::Compile(AnimationCompile& anim)
{
    auto b = float2int(this, anim, stps->Get(anim));
    if (b != 0) {
        auto c = CreateCalChartVectorDeg(dir->Get(anim), stps->Get(anim));
        if (c != Coord{ 0 }) {
            if (b < 0) {
                anim.Append(std::make_unique<AnimationCommandMove>((unsigned)-b, c,
                                -c.DirectionDeg()),
                    this);
            } else {
                anim.Append(std::make_unique<AnimationCommandMove>((unsigned)b, c), this);
            }
        }
    }
}

std::ostream& ProcFM::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrFM]";
    return os << "Forward march for steps " << *stps << " in direction " << *dir;
}

Drawable ProcFM::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "Forward march %@ %@",
        "FM %@ %@",
        { stps->GetDrawable(), dir->GetDrawable() }
    };
}

void ProcFM::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, stps, dir);
}

std::vector<uint8_t> ProcFM::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcFM));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, stps->Serialize());
    Parser::Append(result, dir->Serialize());
    return result;
}

Reader ProcFM::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcFM);
    reader = super::Deserialize(reader);
    std::tie(stps, reader) = DeserializeValue(reader);
    std::tie(dir, reader) = DeserializeValue(reader);
    return reader;
}

// ProcFMTO
void ProcFMTO::Compile(AnimationCompile& anim)
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    if (c != Coord{ 0 }) {
        anim.Append(
            std::make_unique<AnimationCommandMove>((unsigned)c.DM_Magnitude(), c),
            this);
    }
}

std::ostream& ProcFMTO::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrFMT]";
    return os << "Forward march to " << *pnt;
}

Drawable ProcFMTO::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "Forward march to %@",
        "FMTO %@",
        { pnt->GetDrawable() }
    };
}

void ProcFMTO::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt);
}

static inline Coord::units roundcoord(Coord::units a, Coord::units mod)
{
    mod = std::abs(mod);
    if (mod > 0) {
        if (a < 0) {
            a = ((a - (mod / 2)) / mod) * mod;
        } else {
            a = ((a + (mod / 2)) / mod) * mod;
        }
    }
    return a;
}

std::vector<uint8_t> ProcFMTO::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcFMTO));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, pnt->Serialize());
    return result;
}

Reader ProcFMTO::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcFMTO);
    reader = super::Deserialize(reader);
    std::tie(pnt, reader) = DeserializePoint(reader);
    return reader;
}

// ProcGrid
void ProcGrid::Compile(AnimationCompile& anim)
{
    auto gridc = Float2CoordUnits(grid->Get(anim));

    Coord c;
    c.x = roundcoord(anim.GetPointPosition().x, gridc);
    // Adjust so 4 step grid will be on visible grid
    c.y = roundcoord(anim.GetPointPosition().y - Int2CoordUnits(2), gridc) + Int2CoordUnits(2);

    c -= anim.GetPointPosition();
    if (c != Coord{ 0 }) {
        anim.Append(std::make_unique<AnimationCommandMove>(0, c), this);
    }
}

std::ostream& ProcGrid::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrG]";
    return os << "Move on Grid of " << *grid << " spacing";
}

Drawable ProcGrid::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "Move on Grid of %@ spacing",
        "GRID %@",
        { grid->GetDrawable() }
    };
}

void ProcGrid::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, grid);
}

std::vector<uint8_t> ProcGrid::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcGrid));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, grid->Serialize());
    return result;
}

Reader ProcGrid::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcGrid);
    reader = super::Deserialize(reader);
    std::tie(grid, reader) = DeserializeValue(reader);
    return reader;
}

// ProcHSCM
void ProcHSCM::Compile(AnimationCompile& anim)
{
    ValueFloat steps(1.0);

    auto r1 = pnt1->Get(anim);
    auto r2 = pnt2->Get(anim);
    if ((r1.y - r2.y) == Int2CoordUnits(2)) {
        if (r2.x >= r1.x) {
            ValueDefined dirs(CC_S);
            ValueDefined dirw(CC_W);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dirs, dirw, *numbeats);
            return;
        }
    } else if ((r1.y - r2.y) == -Int2CoordUnits(2)) {
        if (r1.x >= r2.x) {
            ValueDefined dirn(CC_N);
            ValueDefined dire(CC_E);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dirn, dire, *numbeats);
            return;
        }
    }
    anim.RegisterError(AnimateError::INVALID_CM, this);
}

std::ostream& ProcHSCM::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrHCM]";
    return os << "High Step CounterMarch starting at " << *pnt1
              << " passing through " << *pnt2 << " for number beats" << *numbeats;
}

Drawable ProcHSCM::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "High Step CounterMarch starting at %@ passing through %@ for number beats %@",
        "HSCM %@ -> %@ for beats %@",
        { pnt1->GetDrawable(), pnt2->GetDrawable(), numbeats->GetDrawable() }
    };
}

void ProcHSCM::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt1, pnt2, numbeats);
}

std::vector<uint8_t> ProcHSCM::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcHSCM));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, pnt1->Serialize());
    Parser::Append(result, pnt2->Serialize());
    Parser::Append(result, numbeats->Serialize());
    return result;
}

Reader ProcHSCM::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcHSCM);
    reader = super::Deserialize(reader);
    std::tie(pnt1, reader) = DeserializePoint(reader);
    std::tie(pnt2, reader) = DeserializePoint(reader);
    std::tie(numbeats, reader) = DeserializeValue(reader);
    return reader;
}

// ProcHSDM
void ProcHSDM::Compile(AnimationCompile& anim)
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
    } else {
        c_hs.x = 0;
        // adjust sign
        c_hs.y = ((c.x < 0) != (c.y < 0)) ? c.y + c.x : c.y - c.x;
        c_dm.x = c.x;
        // adjust sign
        c_dm.y = ((c.x < 0) != (c.y < 0)) ? -c.x : c.x;
        b = CoordUnits2Int(c_hs.y);
    }
    if (c_hs != Coord{ 0 }) {
        if (!anim.Append(std::make_unique<AnimationCommandMove>(std::abs(b), c_hs),
                this)) {
            return;
        }
    }
    if (c_dm != Coord{ 0 }) {
        b = CoordUnits2Int(c_dm.x);
        anim.Append(std::make_unique<AnimationCommandMove>(std::abs(b), c_dm), this);
    }
}

std::ostream& ProcHSDM::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrHD]";
    return os << "HighStep then Diagonal march to " << *pnt;
}

Drawable ProcHSDM::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "HighStep then Diagonal march to %@",
        "HSDM %@",
        { pnt->GetDrawable() }
    };
}

void ProcHSDM::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt);
}

std::vector<uint8_t> ProcHSDM::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcHSDM));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, pnt->Serialize());
    return result;
}

Reader ProcHSDM::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcHSDM);
    reader = super::Deserialize(reader);
    std::tie(pnt, reader) = DeserializePoint(reader);
    return reader;
}

// ProcMagic
void ProcMagic::Compile(AnimationCompile& anim)
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    anim.Append(std::make_unique<AnimationCommandMove>(0, c), this);
}

std::ostream& ProcMagic::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrM]";
    return os << "Magic step to " << *pnt;
}

Drawable ProcMagic::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "Magic step to %@",
        "MAGIC %@",
        { pnt->GetDrawable() }
    };
}

void ProcMagic::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt);
}

std::vector<uint8_t> ProcMagic::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcMagic));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, pnt->Serialize());
    return result;
}

Reader ProcMagic::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcMagic);
    reader = super::Deserialize(reader);
    std::tie(pnt, reader) = DeserializePoint(reader);
    return reader;
}

// ProcMarch
void ProcMarch::Compile(AnimationCompile& anim)
{
    auto b = float2int(this, anim, stps->Get(anim));
    if (b != 0) {
        auto rads = Deg2Rad(dir->Get(anim));
        auto mag = stpsize->Get(anim) * stps->Get(anim);
        Coord c{ Float2CoordUnits(cos(rads) * mag),
            static_cast<Coord::units>(-Float2CoordUnits(sin(rads) * mag)) };
        if (c != Coord{ 0 }) {
            if (facedir)
                anim.Append(std::make_unique<AnimationCommandMove>((unsigned)std::abs(b),
                                c, facedir->Get(anim)),
                    this);
            else if (b < 0) {
                anim.Append(std::make_unique<AnimationCommandMove>((unsigned)-b, c,
                                -c.DirectionDeg()),
                    this);
            } else {
                anim.Append(std::make_unique<AnimationCommandMove>((unsigned)b, c), this);
            }
        }
    }
}

std::ostream& ProcMarch::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrm]";
    os << "March step size" << *stpsize << " for steps " << *stps
       << " in direction " << *dir;
    if (facedir)
        os << " facing " << *facedir;
    return os;
}

Drawable ProcMarch::GetDrawable() const
{
    if (facedir) {
        return {
            this, parent_ptr,
            Type::procedure,
            "March step size %@ for %@ in direction %@ facing %@",
            "MARCH %@ for %@ DIR %@ FACING %@",
            { stpsize->GetDrawable(), stps->GetDrawable(), dir->GetDrawable(), facedir->GetDrawable() }
        };
    }
    return {
        this, parent_ptr,
        Type::procedure,
        "March step size %@ for steps %@ in direction %@",
        "MARCH %@ for %@ DIR %@",
        { stpsize->GetDrawable(), stps->GetDrawable(), dir->GetDrawable() }
    };
}

void ProcMarch::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, stpsize, stps, dir, facedir);
}

std::vector<uint8_t> ProcMarch::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcMarch));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, stpsize->Serialize());
    Parser::Append(result, stps->Serialize());
    Parser::Append(result, dir->Serialize());
    Parser::Append(result, uint8_t(facedir != nullptr));
    if (facedir) {
        Parser::Append(result, facedir->Serialize());
    }
    return result;
}

Reader ProcMarch::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcMarch);
    reader = super::Deserialize(reader);
    std::tie(stpsize, reader) = DeserializeValue(reader);
    std::tie(stps, reader) = DeserializeValue(reader);
    std::tie(dir, reader) = DeserializeValue(reader);
    bool parsefacedir = reader.Get<uint8_t>();
    if (parsefacedir) {
        std::tie(facedir, reader) = DeserializeValue(reader);
    }
    return reader;
}

// ProcMT
void ProcMT::Compile(AnimationCompile& anim)
{
    auto b = float2int(this, anim, numbeats->Get(anim));
    if (b != 0) {
        anim.Append(std::make_unique<AnimationCommandMT>((unsigned)std::abs(b),
                        dir->Get(anim)),
            this);
    }
}

std::ostream& ProcMT::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrMT]";
    return os << "MarkTime for " << *numbeats << " facing " << *dir;
}

Drawable ProcMT::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "MarkTime %@ %@",
        "MT %@ %@",
        { numbeats->GetDrawable(), dir->GetDrawable() }
    };
}

void ProcMT::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, numbeats, dir);
}

std::vector<uint8_t> ProcMT::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcMT));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, numbeats->Serialize());
    Parser::Append(result, dir->Serialize());
    return result;
}

Reader ProcMT::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcMT);
    reader = super::Deserialize(reader);
    std::tie(numbeats, reader) = DeserializeValue(reader);
    std::tie(dir, reader) = DeserializeValue(reader);
    return reader;
}

// ProcMTRM
void ProcMTRM::Compile(AnimationCompile& anim)
{
    anim.Append(std::make_unique<AnimationCommandMT>(anim.GetBeatsRemaining(),
                    dir->Get(anim)),
        this);
}

std::ostream& ProcMTRM::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrMTR]";
    return os << "MarkTime for Remaining Beats facing " << *dir;
}

Drawable ProcMTRM::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "MarkTime for Remaining %@",
        "MTRM %@",
        { dir->GetDrawable() }
    };
}

void ProcMTRM::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, dir);
}

std::vector<uint8_t> ProcMTRM::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcMTRM));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, dir->Serialize());
    return result;
}

Reader ProcMTRM::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcMTRM);
    reader = super::Deserialize(reader);
    std::tie(dir, reader) = DeserializeValue(reader);
    return reader;
}

// ProcNSEW
void ProcNSEW::Compile(AnimationCompile& anim)
{
    auto c1 = pnt->Get(anim) - anim.GetPointPosition();
    if (c1.x != 0) {
        Coord c2{ c1.x, 0 };
        auto b = CoordUnits2Int(c2.x);
        if (!anim.Append(std::make_unique<AnimationCommandMove>(std::abs(b), c2),
                this)) {
            return;
        }
    }
    if (c1.y != 0) {
        Coord c2{ 0, c1.y };
        auto b = CoordUnits2Int(c2.y);
        if (!anim.Append(std::make_unique<AnimationCommandMove>(std::abs(b), c2),
                this)) {
            return;
        }
    }
}

std::ostream& ProcNSEW::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrNSEW]";
    return os << "March NorthSouth/EastWest to " << *pnt;
}

Drawable ProcNSEW::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "NorthSouth/EastWest to %@",
        "NSEW %@",
        { pnt->GetDrawable() }
    };
}

void ProcNSEW::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt);
}

std::vector<uint8_t> ProcNSEW::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcNSEW));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, pnt->Serialize());
    return result;
}

Reader ProcNSEW::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcNSEW);
    reader = super::Deserialize(reader);
    std::tie(pnt, reader) = DeserializePoint(reader);
    return reader;
}

// ProcRotate
void ProcRotate::Compile(AnimationCompile& anim)
{
    // Most of the work is converting to polar coordinates
    auto c = pnt->Get(anim);
    auto rad = anim.GetPointPosition() - c;
    float start_ang;
    if (c == anim.GetPointPosition())
        start_ang = anim.GetVarValue(Cont::Variable::DOH, this);
    else
        start_ang = c.DirectionDeg(anim.GetPointPosition());
    int b = float2int(this, anim, stps->Get(anim));
    float angle = ang->Get(anim);
    bool backwards = false;
    if (b < 0) {
        backwards = true;
    }
    anim.Append(std::make_unique<AnimationCommandRotate>(
                    (unsigned)std::abs(b), c,
                    // Don't use Magnitude() because
                    // we want Coord numbers
                    sqrt(static_cast<float>(rad.x * rad.x + rad.y * rad.y)),
                    start_ang, start_ang + angle, backwards),
        this);
}

std::ostream& ProcRotate::Print(std::ostream& os) const
{
    super::Print(os);
    os << "[CPrR]";
    return os << "Rotate at angle " << *ang << " for " << *stps
              << " around pivot point " << *pnt;
}

Drawable ProcRotate::GetDrawable() const
{
    return {
        this, parent_ptr,
        Type::procedure,
        "Rotate at angle %@ for steps %@ around pivot point %@",
        "ROTATE %@ for %@ around %@",
        { ang->GetDrawable(), stps->GetDrawable(), pnt->GetDrawable() }
    };
}

void ProcRotate::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, ang, stps, pnt);
}

std::vector<uint8_t> ProcRotate::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcRotate));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, ang->Serialize());
    Parser::Append(result, stps->Serialize());
    Parser::Append(result, pnt->Serialize());
    return result;
}

Reader ProcRotate::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcRotate);
    reader = super::Deserialize(reader);
    std::tie(ang, reader) = DeserializeValue(reader);
    std::tie(stps, reader) = DeserializeValue(reader);
    std::tie(pnt, reader) = DeserializePoint(reader);
    return reader;
}

}
