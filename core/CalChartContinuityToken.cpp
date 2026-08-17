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
#include "CalChartAngles.h"
#include "CalChartAnimationCommand.h"
#include "CalChartAnimationCompile.h"
#include "CalChartAnimationTypes.h"
#include "CalChartSheet.h"
#include "CalChartUtils.h"
#include "parse.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <optional>

// for serialization we need to pre-register all of the different types that can exist in the continuity AST.  Note:
// because this is serialized, take care when adding new tokens to handle backward compatiblity.  If you need to add a
// new token, add it to the end of the list and make sure to handle missing tokens in the Deserialize function.
namespace {
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
    ProcClose,
    ProcStandAndPlay,
};

constexpr std::array<std::string_view, 9> s_var_names = {
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

constexpr std::array<std::string_view, 15> DefinedValue_strings
    = { "N", "NW", "W", "SW", "S", "SE", "E", "NE", "HS", "MM", "SH", "JS", "GV", "M", "DM" };

template <typename Float> auto float2int(CalChart::Animate::Compile& anim, Float f) -> int
{
    static_assert(std::is_floating_point_v<Float>, "float2int requires float");
    auto v = static_cast<int>(floor(f + 0.5));
    if (std::abs(f - v) >= CalChart::kCoordDecimal) {
        anim.RegisterError(CalChart::Animate::Error::NONINT);
    }
    return v;
}

template <typename Float> auto float2unsigned(CalChart::Animate::Compile& anim, Float f) -> unsigned
{
    static_assert(std::is_floating_point_v<Float>, "float2unsigned requires float");
    auto v = float2int(anim, f);
    if (v < 0) {
        anim.RegisterError(CalChart::Animate::Error::NEGINT);
        return 0;
    }
    return static_cast<unsigned>(v);
}

auto EnsureObject(nlohmann::json const& json, std::string_view context) -> nlohmann::json const&
{
    if (!json.is_object()) {
        throw std::runtime_error("bad Procedure JSON: " + std::string{ context } + " must be an object");
    }
    return json;
}

auto EnsureTypeName(nlohmann::json const& json, std::string_view context) -> std::string
{
    auto const& object = EnsureObject(json, context);
    auto const iter = object.find("type");
    if (iter == object.end() || !iter->is_string()) {
        throw std::runtime_error("bad Procedure JSON: " + std::string{ context } + " must contain string field 'type'");
    }
    return iter->get<std::string>();
}

auto RequireField(nlohmann::json const& json, std::string_view field, std::string_view context) -> nlohmann::json const&
{
    auto const& object = EnsureObject(json, context);
    auto const iter = object.find(field);
    if (iter == object.end()) {
        throw std::runtime_error(
            "bad Procedure JSON: missing field '" + std::string{ field } + "' in " + std::string{ context });
    }
    return *iter;
}

auto OptionalField(nlohmann::json const& json, std::string_view field) -> nlohmann::json const*
{
    auto const iter = json.find(field);
    if (iter == json.end()) {
        return nullptr;
    }
    return &*iter;
}

auto AddTokenLocation(nlohmann::json json, CalChart::Cont::Token const& token) -> nlohmann::json
{
    json["line"] = token.GetLine();
    json["col"] = token.GetCol();
    return json;
}

void ApplyTokenLocation(CalChart::Cont::Token& token, nlohmann::json const& json)
{
    auto const* line = OptionalField(json, "line");
    auto const* col = OptionalField(json, "col");
    if (line == nullptr && col == nullptr) {
        return;
    }
    if (line == nullptr || col == nullptr) {
        throw std::runtime_error("bad Procedure JSON: token location requires both 'line' and 'col'");
    }
    if (!line->is_number_unsigned() || !col->is_number_unsigned()) {
        throw std::runtime_error("bad Procedure JSON: token location fields must be unsigned integers");
    }
    token.SetSourceLocation(line->get<uint32_t>(), col->get<uint32_t>());
}

template <typename TokenPtr> auto ApplyTokenLocationToPtr(TokenPtr token, nlohmann::json const& json) -> TokenPtr
{
    ApplyTokenLocation(*token, json);
    return token;
}

auto ParseDefinedValue(std::string const& value) -> CalChart::Cont::DefinedValue
{
    auto const iter = std::find(DefinedValue_strings.begin(), DefinedValue_strings.end(), value);
    if (iter == DefinedValue_strings.end()) {
        throw std::runtime_error("bad Procedure JSON: invalid defined value '" + value + "'");
    }
    return static_cast<CalChart::Cont::DefinedValue>(std::distance(DefinedValue_strings.begin(), iter));
}

auto ParseVariable(std::string const& value) -> CalChart::Cont::Variable
{
    auto const iter = std::find(s_var_names.begin(), s_var_names.end(), value);
    if (iter == s_var_names.end()) {
        throw std::runtime_error("bad Procedure JSON: invalid variable '" + value + "'");
    }
    return static_cast<CalChart::Cont::Variable>(std::distance(s_var_names.begin(), iter));
}

auto PointFromJSON(nlohmann::json const& json) -> std::unique_ptr<CalChart::Cont::Point>;
auto ValueFromJSON(nlohmann::json const& json) -> std::unique_ptr<CalChart::Cont::Value>;
auto ProcedureFromJSONNode(nlohmann::json const& json) -> std::unique_ptr<CalChart::Cont::Procedure>;

auto PointFromJSON(nlohmann::json const& json) -> std::unique_ptr<CalChart::Cont::Point>
{
    namespace C = CalChart::Cont;
    auto const type = EnsureTypeName(json, "point node");
    if (type == "Point") {
        return C::Point::fromJSON(json);
    }
    if (type == "PointUnset") {
        return C::PointUnset::fromJSON(json);
    }
    if (type == "StartPoint") {
        return C::StartPoint::fromJSON(json);
    }
    if (type == "NextPoint") {
        return C::NextPoint::fromJSON(json);
    }
    if (type == "RefPoint") {
        return C::RefPoint::fromJSON(json);
    }
    throw std::runtime_error("bad Procedure JSON: unknown point type '" + type + "'");
}

auto ValueFromJSON(nlohmann::json const& json) -> std::unique_ptr<CalChart::Cont::Value>
{
    namespace C = CalChart::Cont;
    auto const type = EnsureTypeName(json, "value node");
    if (type == "ValueUnset") {
        return C::ValueUnset::fromJSON(json);
    }
    if (type == "ValueFloat") {
        return C::ValueFloat::fromJSON(json);
    }
    if (type == "ValueDefined") {
        return C::ValueDefined::fromJSON(json);
    }
    if (type == "ValueAdd") {
        return C::ValueAdd::fromJSON(json);
    }
    if (type == "ValueSub") {
        return C::ValueSub::fromJSON(json);
    }
    if (type == "ValueMult") {
        return C::ValueMult::fromJSON(json);
    }
    if (type == "ValueDiv") {
        return C::ValueDiv::fromJSON(json);
    }
    if (type == "ValueNeg") {
        return C::ValueNeg::fromJSON(json);
    }
    if (type == "ValueREM") {
        return C::ValueREM::fromJSON(json);
    }
    if (type == "ValueVar") {
        return C::ValueVar::fromJSON(json);
    }
    if (type == "ValueVarUnset") {
        return C::ValueVarUnset::fromJSON(json);
    }
    if (type == "FuncDir") {
        return C::FuncDir::fromJSON(json);
    }
    if (type == "FuncDirFrom") {
        return C::FuncDirFrom::fromJSON(json);
    }
    if (type == "FuncDist") {
        return C::FuncDist::fromJSON(json);
    }
    if (type == "FuncDistFrom") {
        return C::FuncDistFrom::fromJSON(json);
    }
    if (type == "FuncEither") {
        return C::FuncEither::fromJSON(json);
    }
    if (type == "FuncOpp") {
        return C::FuncOpp::fromJSON(json);
    }
    if (type == "FuncStep") {
        return C::FuncStep::fromJSON(json);
    }
    throw std::runtime_error("bad Procedure JSON: unknown value type '" + type + "'");
}

auto ProcedureFromJSONNode(nlohmann::json const& json) -> std::unique_ptr<CalChart::Cont::Procedure>
{
    namespace C = CalChart::Cont;
    auto const type = EnsureTypeName(json, "procedure node");
    if (type == "ProcUnset") {
        return C::ProcUnset::fromJSON(json);
    }
    if (type == "ProcSet") {
        return C::ProcSet::fromJSON(json);
    }
    if (type == "ProcBlam") {
        return C::ProcBlam::fromJSON(json);
    }
    if (type == "ProcClose") {
        return C::ProcClose::fromJSON(json);
    }
    if (type == "ProcCM") {
        return C::ProcCM::fromJSON(json);
    }
    if (type == "ProcDMCM") {
        return C::ProcDMCM::fromJSON(json);
    }
    if (type == "ProcDMHS") {
        return C::ProcDMHS::fromJSON(json);
    }
    if (type == "ProcEven") {
        return C::ProcEven::fromJSON(json);
    }
    if (type == "ProcEWNS") {
        return C::ProcEWNS::fromJSON(json);
    }
    if (type == "ProcFountain") {
        return C::ProcFountain::fromJSON(json);
    }
    if (type == "ProcFM") {
        return C::ProcFM::fromJSON(json);
    }
    if (type == "ProcFMTO") {
        return C::ProcFMTO::fromJSON(json);
    }
    if (type == "ProcGrid") {
        return C::ProcGrid::fromJSON(json);
    }
    if (type == "ProcHSCM") {
        return C::ProcHSCM::fromJSON(json);
    }
    if (type == "ProcHSDM") {
        return C::ProcHSDM::fromJSON(json);
    }
    if (type == "ProcMagic") {
        return C::ProcMagic::fromJSON(json);
    }
    if (type == "ProcMarch") {
        return C::ProcMarch::fromJSON(json);
    }
    if (type == "ProcMT") {
        return C::ProcMT::fromJSON(json);
    }
    if (type == "ProcMTRM") {
        return C::ProcMTRM::fromJSON(json);
    }
    if (type == "ProcNSEW") {
        return C::ProcNSEW::fromJSON(json);
    }
    if (type == "ProcRotate") {
        return C::ProcRotate::fromJSON(json);
    }
    if (type == "ProcStandAndPlay") {
        return C::ProcStandAndPlay::fromJSON(json);
    }
    throw std::runtime_error("bad Procedure JSON: unknown procedure type '" + type + "'");
}
}

namespace CalChart::Cont {

void DoCounterMarch(Animate::Compile& anim, const Point& pnt1, const Point& pnt2, const Value& stps, const Value& dir1,
    const Value& dir2, const Value& numbeats)
{
    auto d1 = CalChart::Degree{ dir1.Get(anim) };
    auto d2 = CalChart::Degree{ dir2.Get(anim) };
    auto c = sin(d1 - d2);
    if (IS_ZERO(c)) {
        anim.RegisterError(Animate::Error::INVALID_CM);
        return;
    }
    auto ref1 = pnt1.Get(anim);
    auto ref2 = pnt2.Get(anim);
    auto steps1 = stps.Get(anim);
    auto beats = numbeats.Get(anim);

    auto v1 = CalChart::CreateCalChartVector(d1, steps1);

    Coord p[4];
    p[1] = ref1 + v1;
    auto steps2 = (ref2 - p[1]).Magnitude() * sin(CalChart::Degree{ ref2.Direction(p[1]) } - d1) / c;
    if (IsDiagonalDirection(d2)) {
        steps2 /= static_cast<float>(std::numbers::sqrt2);
    }
    auto v2 = CreateCalChartVector(d2, steps2);
    p[2] = p[1] + v2;
    p[3] = ref2 - v1;
    p[0] = p[3] - v2;

    v1 = p[1] - anim.GetPointPosition();
    auto leg = 0;
    if ((v1 != Coord{ 0 }) && CalChart::Degree{ v1.Direction() }.IsEqual(d1)) {
        leg = 1;
    } else {
        v1 = p[2] - anim.GetPointPosition();
        if ((v1 != Coord{ 0 }) && CalChart::Degree{ v1.Direction() }.IsEqual(d2)) {
            leg = 2;
        } else {
            v1 = p[3] - anim.GetPointPosition();
            if ((v1 != Coord{ 0 }) && CalChart::Degree{ v1.Direction() }.IsEqual(d1 + CalChart::Degree::South())) {
                leg = 3;
            } else {
                v1 = p[0] - anim.GetPointPosition();
                if ((v1 != Coord{ 0 }) && CalChart::Degree{ v1.Direction() }.IsEqual(d2 + CalChart::Degree::South())) {
                    leg = 0;
                } else {
                    // Current point is not in path of countermarch
                    anim.RegisterError(Animate::Error::INVALID_CM);
                    return;
                }
            }
        }
    }

    while (beats > 0) {
        v1 = p[leg] - anim.GetPointPosition();
        auto distance = static_cast<float>(v1.DM_Magnitude());
        if (distance <= beats) {
            beats -= distance;
            if (!anim.Append(Animate::CommandMove{ anim.GetPointPosition(), float2unsigned(anim, distance), v1 })) {
                return;
            }
        } else {
            switch (leg) {
            case 0:
                v1 = CreateCalChartVector(d2 + CalChart::Degree::South(), beats);
                break;
            case 1:
                v1 = CreateCalChartVector(d1, beats);
                break;
            case 2:
                v1 = CreateCalChartVector(d2, beats);
                break;
            default:
                v1 = CreateCalChartVector(d1 + CalChart::Degree::South(), beats);
                break;
            }
            anim.Append(Animate::CommandMove{ anim.GetPointPosition(), float2unsigned(anim, beats), v1 });
            return;
        }
        leg++;
        if (leg > 3)
            leg = 0;
    }
}

#define CheckForToken(reader, minSize, serialToken) CheckForTokenImpl(reader, minSize, serialToken, #serialToken)

template <typename T, typename U> auto CheckForTokenImpl(Reader reader, size_t minSize, T serialToken, U tokenName)
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
    case SerializationToken::ProcClose:
        v = std::make_unique<ProcClose>();
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
    case SerializationToken::ProcStandAndPlay:
        v = std::make_unique<ProcStandAndPlay>();
        break;
    default:
        throw std::runtime_error("Error, did not find Point");
    }
    auto b = v->Deserialize(reader);
    return { std::move(v), b };
}

auto Procedure::FromJSON(nlohmann::json const& json) -> std::unique_ptr<Procedure>
{
    try {
        return ProcedureFromJSONNode(json);
    } catch (std::runtime_error const&) {
        throw;
    } catch (nlohmann::json::exception const& e) {
        throw std::runtime_error(std::string{ "bad Procedure JSON: " } + e.what());
    }
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

auto Token::ToString() const -> std::string { return "[CT]"; }

void Token::replace(Token const* /*which*/, std::unique_ptr<Token> /*v*/)
{
    throw std::runtime_error("Error, replace not implemented on this class");
}

auto Token::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
Coord Point::Get(Animate::Compile const& anim) const { return anim.GetPointPosition(); }

auto Point::ToString() const -> std::string { return std::format("{}[CP]Point:", super::ToString()); }

Drawable Point::GetDrawable() const { return { this, parent_ptr, Type::point, "Point", "P", {} }; }

auto Point::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "Point" },
        },
        *this);
}

auto Point::fromJSON(nlohmann::json const& json) -> std::unique_ptr<Point>
{
    return ApplyTokenLocationToPtr(std::make_unique<Point>(), json);
}

auto Point::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
auto PointUnset::ToString() const -> std::string { return std::format("{}[CPU]Unset", super::ToString()); }

Drawable PointUnset::GetDrawable() const { return { this, parent_ptr, Type::unset, "unset point", "unset point", {} }; }

auto PointUnset::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "PointUnset" },
        },
        *this);
}

auto PointUnset::fromJSON(nlohmann::json const& json) -> std::unique_ptr<PointUnset>
{
    return ApplyTokenLocationToPtr(std::make_unique<PointUnset>(), json);
}

auto PointUnset::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
Coord StartPoint::Get(Animate::Compile const& anim) const { return anim.GetStartingPosition(); }

auto StartPoint::ToString() const -> std::string { return std::format("{}[CSP]Start Point", super::ToString()); }

Drawable StartPoint::GetDrawable() const { return { this, parent_ptr, Type::point, "Start Point", "SP", {} }; }

auto StartPoint::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "StartPoint" },
        },
        *this);
}

auto StartPoint::fromJSON(nlohmann::json const& json) -> std::unique_ptr<StartPoint>
{
    return ApplyTokenLocationToPtr(std::make_unique<StartPoint>(), json);
}

auto StartPoint::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
Coord NextPoint::Get(Animate::Compile const& anim) const { return anim.GetEndingPosition(); }

auto NextPoint::ToString() const -> std::string { return std::format("{}[CNP]Next Point", super::ToString()); }

Drawable NextPoint::GetDrawable() const { return { this, parent_ptr, Type::point, "Next Point", "NP", {} }; }

auto NextPoint::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "NextPoint" },
        },
        *this);
}

auto NextPoint::fromJSON(nlohmann::json const& json) -> std::unique_ptr<NextPoint>
{
    return ApplyTokenLocationToPtr(std::make_unique<NextPoint>(), json);
}

auto NextPoint::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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

Coord RefPoint::Get(Animate::Compile const& anim) const { return anim.GetReferencePointPosition(refnum); }

auto RefPoint::ToString() const -> std::string { return std::format("{}[CRP]Ref Point {}", super::ToString(), refnum); }

Drawable RefPoint::GetDrawable() const
{
    return { this, parent_ptr, Type::point, std::string("Ref Point ") + std::to_string(refnum),
        std::string("R") + std::to_string(refnum), {} };
}

auto RefPoint::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "RefPoint" },
            { "refnum", refnum },
        },
        *this);
}

auto RefPoint::fromJSON(nlohmann::json const& json) -> std::unique_ptr<RefPoint>
{
    auto const& refnum = RequireField(json, "refnum", "RefPoint");
    if (!refnum.is_number_unsigned()) {
        throw std::runtime_error("bad Procedure JSON: RefPoint.refnum must be an unsigned integer");
    }
    return ApplyTokenLocationToPtr(std::make_unique<RefPoint>(refnum.get<unsigned>()), json);
}

auto RefPoint::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
auto Value::ToString() const -> std::string { return std::format("{}[CV]Value:", super::ToString()); }

auto Value::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
auto ValueUnset::ToString() const -> std::string { return std::format("{}[CVU]Unset", super::ToString()); }

Drawable ValueUnset::GetDrawable() const { return { this, parent_ptr, Type::unset, "unset value", "unset value", {} }; }

auto ValueUnset::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ValueUnset" },
        },
        *this);
}

auto ValueUnset::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ValueUnset>
{
    return ApplyTokenLocationToPtr(std::make_unique<ValueUnset>(), json);
}

auto ValueUnset::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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

float ValueFloat::Get(Animate::Compile const&) const { return val; }

auto ValueFloat::ToString() const -> std::string { return std::format("{}[CVF]{}", super::ToString(), val); }

Drawable ValueFloat::GetDrawable() const
{
    // to_string gives a lot of decimal points.  256 on the stack should be ok...?
    if (int(val) == val) {
        return { this, parent_ptr, Type::value, std::to_string(int(val)), std::to_string(int(val)), {} };
    }
    return { this, parent_ptr, Type::value, std::to_string(val), std::to_string(val), {} };
}

auto ValueFloat::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ValueFloat" },
            { "val", val },
        },
        *this);
}

auto ValueFloat::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ValueFloat>
{
    auto const& val = RequireField(json, "val", "ValueFloat");
    if (!val.is_number()) {
        throw std::runtime_error("bad Procedure JSON: ValueFloat.val must be numeric");
    }
    return ApplyTokenLocationToPtr(std::make_unique<ValueFloat>(val.get<float>()), json);
}

auto ValueFloat::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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

float ValueDefined::Get(Animate::Compile const&) const
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

auto ValueDefined::ToString() const -> std::string
{
    return std::format("{}[CVC]Defined:{}", super::ToString(), DefinedValue_strings[val]);
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
    return { this, parent_ptr, type, std::string{ DefinedValue_strings[val] }, std::string{ DefinedValue_strings[val] },
        {} };
}

auto ValueDefined::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ValueDefined" },
            { "defined", std::string{ DefinedValue_strings.at(val) } },
        },
        *this);
}

auto ValueDefined::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ValueDefined>
{
    auto const& defined = RequireField(json, "defined", "ValueDefined");
    if (!defined.is_string()) {
        throw std::runtime_error("bad Procedure JSON: ValueDefined.defined must be a string");
    }
    return ApplyTokenLocationToPtr(std::make_unique<ValueDefined>(ParseDefinedValue(defined.get<std::string>())), json);
}

auto ValueDefined::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
float ValueAdd::Get(Animate::Compile const& anim) const { return (val1->Get(anim) + val2->Get(anim)); }

auto ValueAdd::ToString() const -> std::string
{
    return std::format("{}[CVA]{} + {}", super::ToString(), *val1, *val2);
}

Drawable ValueAdd::GetDrawable() const
{
    return { this, parent_ptr, Type::function, "( %@ + %@ )", "(%@+%@)", { val1->GetDrawable(), val2->GetDrawable() } };
}

auto ValueAdd::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ValueAdd" },
            { "val1", val1->toJSON() },
            { "val2", val2->toJSON() },
        },
        *this);
}

auto ValueAdd::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ValueAdd>
{
    return ApplyTokenLocationToPtr(std::make_unique<ValueAdd>(ValueFromJSON(RequireField(json, "val1", "ValueAdd")),
                                       ValueFromJSON(RequireField(json, "val2", "ValueAdd"))),
        json);
}

void ValueAdd::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, val1, val2);
}

auto ValueAdd::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
float ValueSub::Get(Animate::Compile const& anim) const { return (val1->Get(anim) - val2->Get(anim)); }

auto ValueSub::ToString() const -> std::string
{
    return std::format("{}[CVS]{} - {}", super::ToString(), *val1, *val2);
}

Drawable ValueSub::GetDrawable() const
{
    return { this, parent_ptr, Type::function, "( %@ - %@ )", "(%@-%@)", { val1->GetDrawable(), val2->GetDrawable() } };
}

auto ValueSub::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ValueSub" },
            { "val1", val1->toJSON() },
            { "val2", val2->toJSON() },
        },
        *this);
}

auto ValueSub::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ValueSub>
{
    return ApplyTokenLocationToPtr(std::make_unique<ValueSub>(ValueFromJSON(RequireField(json, "val1", "ValueSub")),
                                       ValueFromJSON(RequireField(json, "val2", "ValueSub"))),
        json);
}

void ValueSub::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, val1, val2);
}

auto ValueSub::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
float ValueMult::Get(Animate::Compile const& anim) const { return (val1->Get(anim) * val2->Get(anim)); }

auto ValueMult::ToString() const -> std::string
{
    return std::format("{}[CVM]{} * {}", super::ToString(), *val1, *val2);
}

Drawable ValueMult::GetDrawable() const
{
    return { this, parent_ptr, Type::function, "( %@ * %@ )", "(%@*%@)", { val1->GetDrawable(), val2->GetDrawable() } };
}

auto ValueMult::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ValueMult" },
            { "val1", val1->toJSON() },
            { "val2", val2->toJSON() },
        },
        *this);
}

auto ValueMult::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ValueMult>
{
    return ApplyTokenLocationToPtr(std::make_unique<ValueMult>(ValueFromJSON(RequireField(json, "val1", "ValueMult")),
                                       ValueFromJSON(RequireField(json, "val2", "ValueMult"))),
        json);
}

void ValueMult::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, val1, val2);
}

auto ValueMult::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
float ValueDiv::Get(Animate::Compile const& anim) const
{
    auto f = val2->Get(anim);
    if (IS_ZERO(f)) {
        anim.RegisterError(Animate::Error::DIVISION_ZERO);
        return 0.0;
    } else {
        return (val1->Get(anim) / f);
    }
}

auto ValueDiv::ToString() const -> std::string
{
    return std::format("{}[CVD]{} / {}", super::ToString(), *val1, *val2);
}

Drawable ValueDiv::GetDrawable() const
{
    return { this, parent_ptr, Type::function, "( %@ / %@ )", "(%@/%@)", { val1->GetDrawable(), val2->GetDrawable() } };
}

auto ValueDiv::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ValueDiv" },
            { "val1", val1->toJSON() },
            { "val2", val2->toJSON() },
        },
        *this);
}

auto ValueDiv::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ValueDiv>
{
    return ApplyTokenLocationToPtr(std::make_unique<ValueDiv>(ValueFromJSON(RequireField(json, "val1", "ValueDiv")),
                                       ValueFromJSON(RequireField(json, "val2", "ValueDiv"))),
        json);
}

void ValueDiv::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, val1, val2);
}

auto ValueDiv::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
float ValueNeg::Get(Animate::Compile const& anim) const { return -val->Get(anim); }

auto ValueNeg::ToString() const -> std::string { return std::format("{}[CVN]- {}", super::ToString(), *val); }

Drawable ValueNeg::GetDrawable() const
{
    return { this, parent_ptr, Type::function, "-%@", "-%@", { val->GetDrawable() } };
}

auto ValueNeg::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ValueNeg" },
            { "val", val->toJSON() },
        },
        *this);
}

auto ValueNeg::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ValueNeg>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<ValueNeg>(ValueFromJSON(RequireField(json, "val", "ValueNeg"))), json);
}

void ValueNeg::replace(Token const* which, std::unique_ptr<Token> v) { replace_helper<NumParts>(this, which, v, val); }

auto ValueNeg::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
auto ValueREM::Get(Animate::Compile const& anim) const -> float { return static_cast<float>(anim.GetBeatsRemaining()); }

auto ValueREM::ToString() const -> std::string { return std::format("{}[CVR]REM", super::ToString()); }

Drawable ValueREM::GetDrawable() const { return { this, parent_ptr, Type::value, "Remaining", "REM", {} }; }

auto ValueREM::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ValueREM" },
        },
        *this);
}

auto ValueREM::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ValueREM>
{
    return ApplyTokenLocationToPtr(std::make_unique<ValueREM>(), json);
}

auto ValueREM::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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

float ValueVar::Get(Animate::Compile const& anim) const { return anim.GetVarValue(varnum); }

auto ValueVar::ToString() const -> std::string
{
    return std::format("{}[CVV]Var {}", super::ToString(), toUType(varnum));
}

Drawable ValueVar::GetDrawable() const
{
    return { this, parent_ptr, Type::value, std::string{ s_var_names[toUType(varnum)] },
        std::string{ s_var_names[toUType(varnum)] }, {} };
}

auto ValueVar::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ValueVar" },
            { "variable", std::string{ s_var_names.at(CalChart::toUType(varnum)) } },
        },
        *this);
}

auto ValueVar::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ValueVar>
{
    auto const& variable = RequireField(json, "variable", "ValueVar");
    if (!variable.is_string()) {
        throw std::runtime_error("bad Procedure JSON: ValueVar.variable must be a string");
    }
    return ApplyTokenLocationToPtr(std::make_unique<ValueVar>(ParseVariable(variable.get<std::string>())), json);
}

void ValueVar::Set(Animate::Compile& anim, float v) { anim.SetVarValue(varnum, v); }

auto ValueVar::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
auto ValueVarUnset::ToString() const -> std::string { return std::format("{}[CVVU]Unset", super::ToString()); }

Drawable ValueVarUnset::GetDrawable() const
{
    return { this, parent_ptr, Type::unset, "unset value var", "unset value var", {} };
}

auto ValueVarUnset::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ValueVarUnset" },
        },
        *this);
}

auto ValueVarUnset::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ValueVarUnset>
{
    return ApplyTokenLocationToPtr(std::make_unique<ValueVarUnset>(), json);
}

auto ValueVarUnset::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
auto FuncDir::Get(Animate::Compile const& anim) const -> float
{
    auto c = pnt->Get(anim);
    if (c == anim.GetPointPosition()) {
        anim.RegisterError(Animate::Error::UNDEFINED);
    }
    return static_cast<float>(CalChart::Degree{ anim.GetPointPosition().Direction(c) }.getValue());
}

auto FuncDir::ToString() const -> std::string { return std::format("{}[CFD]Direction to {}", super::ToString(), *pnt); }

Drawable FuncDir::GetDrawable() const
{
    return { this, parent_ptr, Type::function, "Direction to %@", "DIR %@", { pnt->GetDrawable() } };
}

auto FuncDir::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "FuncDir" },
            { "pnt", pnt->toJSON() },
        },
        *this);
}

auto FuncDir::fromJSON(nlohmann::json const& json) -> std::unique_ptr<FuncDir>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<FuncDir>(PointFromJSON(RequireField(json, "pnt", "FuncDir"))), json);
}

void FuncDir::replace(Token const* which, std::unique_ptr<Token> v) { replace_helper<NumParts>(this, which, v, pnt); }

auto FuncDir::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
auto FuncDirFrom::Get(Animate::Compile const& anim) const -> float
{
    auto start = pnt_start->Get(anim);
    auto end = pnt_end->Get(anim);
    if (start == end) {
        anim.RegisterError(Animate::Error::UNDEFINED);
    }
    return static_cast<float>(CalChart::Degree{ start.Direction(end) }.getValue());
}

auto FuncDirFrom::ToString() const -> std::string
{
    return std::format("{}[CFDF]Direction from {} to {}", super::ToString(), *pnt_start, *pnt_end);
}

Drawable FuncDirFrom::GetDrawable() const
{
    return { this, parent_ptr, Type::function, "Direction from %@ to %@", "DIRFROM %@ to %@",
        { pnt_start->GetDrawable(), pnt_end->GetDrawable() } };
}

auto FuncDirFrom::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "FuncDirFrom" },
            { "pnt1", pnt_start->toJSON() },
            { "pnt2", pnt_end->toJSON() },
        },
        *this);
}

auto FuncDirFrom::fromJSON(nlohmann::json const& json) -> std::unique_ptr<FuncDirFrom>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<FuncDirFrom>(PointFromJSON(RequireField(json, "pnt1", "FuncDirFrom")),
            PointFromJSON(RequireField(json, "pnt2", "FuncDirFrom"))),
        json);
}

void FuncDirFrom::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt_start, pnt_end);
}

auto FuncDirFrom::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
float FuncDist::Get(Animate::Compile const& anim) const
{
    auto vector = pnt->Get(anim) - anim.GetPointPosition();
    return vector.DM_Magnitude();
}

auto FuncDist::ToString() const -> std::string { return std::format("{}[CFd]Distance to {}", super::ToString(), *pnt); }

Drawable FuncDist::GetDrawable() const
{
    return { this, parent_ptr, Type::function, "Distance to %@", "DIST %@", { pnt->GetDrawable() } };
}

auto FuncDist::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "FuncDist" },
            { "pnt", pnt->toJSON() },
        },
        *this);
}

auto FuncDist::fromJSON(nlohmann::json const& json) -> std::unique_ptr<FuncDist>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<FuncDist>(PointFromJSON(RequireField(json, "pnt", "FuncDist"))), json);
}

void FuncDist::replace(Token const* which, std::unique_ptr<Token> v) { replace_helper<NumParts>(this, which, v, pnt); }

auto FuncDist::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
float FuncDistFrom::Get(Animate::Compile const& anim) const
{
    auto vector = pnt_end->Get(anim) - pnt_start->Get(anim);
    return vector.Magnitude();
}

auto FuncDistFrom::ToString() const -> std::string
{
    return std::format("{}[CFdF]Distance from {} to {}", super::ToString(), *pnt_start, *pnt_end);
}

Drawable FuncDistFrom::GetDrawable() const
{
    return { this, parent_ptr, Type::function, "Distance from %@ to %@", "DISTFROM %@ to %@",
        { pnt_start->GetDrawable(), pnt_end->GetDrawable() } };
}

auto FuncDistFrom::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "FuncDistFrom" },
            { "pnt1", pnt_start->toJSON() },
            { "pnt2", pnt_end->toJSON() },
        },
        *this);
}

auto FuncDistFrom::fromJSON(nlohmann::json const& json) -> std::unique_ptr<FuncDistFrom>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<FuncDistFrom>(PointFromJSON(RequireField(json, "pnt1", "FuncDistFrom")),
            PointFromJSON(RequireField(json, "pnt2", "FuncDistFrom"))),
        json);
}

void FuncDistFrom::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt_start, pnt_end);
}

auto FuncDistFrom::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
float FuncEither::Get(Animate::Compile const& anim) const
{
    auto c = pnt->Get(anim);
    if (anim.GetPointPosition() == c) {
        anim.RegisterError(Animate::Error::UNDEFINED);
        return dir1->Get(anim);
    }
    auto dir = anim.GetPointPosition().Direction(c);
    auto d1 = CalChart::BoundDirectionSigned(CalChart::Radian{ dir1->Get(anim) } - dir);
    auto d2 = CalChart::BoundDirectionSigned(CalChart::Radian{ dir2->Get(anim) } - dir);
    return (std::abs(d1.getValue()) > std::abs(d2.getValue())) ? dir2->Get(anim) : dir1->Get(anim);
}

auto FuncEither::ToString() const -> std::string
{
    return std::format("{}[CFE]Either direction to {} or {}, depending on whichever is a shorter angle to {}",
        super::ToString(), *dir1, *dir2, *pnt);
}

Drawable FuncEither::GetDrawable() const
{
    return { this, parent_ptr, Type::function,
        "Either direction to %@ or %@, depending on whichever is a shorter angle to %@", "EITHER %@ or %@, by %@",
        { dir1->GetDrawable(), dir2->GetDrawable(), pnt->GetDrawable() } };
}

auto FuncEither::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "FuncEither" },
            { "dir1", dir1->toJSON() },
            { "dir2", dir2->toJSON() },
            { "pnt", pnt->toJSON() },
        },
        *this);
}

auto FuncEither::fromJSON(nlohmann::json const& json) -> std::unique_ptr<FuncEither>
{
    return ApplyTokenLocationToPtr(std::make_unique<FuncEither>(ValueFromJSON(RequireField(json, "dir1", "FuncEither")),
                                       ValueFromJSON(RequireField(json, "dir2", "FuncEither")),
                                       PointFromJSON(RequireField(json, "pnt", "FuncEither"))),
        json);
}

void FuncEither::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, dir1, dir2, pnt);
}

auto FuncEither::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
float FuncOpp::Get(Animate::Compile const& anim) const { return (dir->Get(anim) + 180.0f); }

auto FuncOpp::ToString() const -> std::string
{
    return std::format("{}[CFO]opposite direction of {}", super::ToString(), *dir);
}

Drawable FuncOpp::GetDrawable() const
{
    return { this, parent_ptr, Type::function, "opposite direction of %@", "OPP %@", { dir->GetDrawable() } };
}

auto FuncOpp::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "FuncOpp" },
            { "dir", dir->toJSON() },
        },
        *this);
}

auto FuncOpp::fromJSON(nlohmann::json const& json) -> std::unique_ptr<FuncOpp>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<FuncOpp>(ValueFromJSON(RequireField(json, "dir", "FuncOpp"))), json);
}

void FuncOpp::replace(Token const* which, std::unique_ptr<Token> v) { replace_helper<NumParts>(this, which, v, dir); }

auto FuncOpp::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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

float FuncStep::Get(Animate::Compile const& anim) const
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    return (c.DM_Magnitude() * numbeats->Get(anim) / blksize->Get(anim));
}

auto FuncStep::ToString() const -> std::string
{
    return std::format("{}[CFS]Step drill at {} beats for a block size of {} from point {}", super::ToString(),
        *numbeats, *blksize, *pnt);
}

Drawable FuncStep::GetDrawable() const
{
    return { this, parent_ptr, Type::function, "Step drill at %@ beats for a block size of %@ from point %@",
        "STEP %@ Beats, %@ size, from %@", { numbeats->GetDrawable(), blksize->GetDrawable(), pnt->GetDrawable() } };
}

auto FuncStep::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "FuncStep" },
            { "numbeats", numbeats->toJSON() },
            { "blksize", blksize->toJSON() },
            { "pnt", pnt->toJSON() },
        },
        *this);
}

auto FuncStep::fromJSON(nlohmann::json const& json) -> std::unique_ptr<FuncStep>
{
    return ApplyTokenLocationToPtr(std::make_unique<FuncStep>(ValueFromJSON(RequireField(json, "numbeats", "FuncStep")),
                                       ValueFromJSON(RequireField(json, "blksize", "FuncStep")),
                                       PointFromJSON(RequireField(json, "pnt", "FuncStep"))),
        json);
}

void FuncStep::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, numbeats, blksize, pnt);
}

auto FuncStep::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
auto Procedure::ToString() const -> std::string { return std::format("{}[CPr]Procedure: ", super::ToString()); }

auto Procedure::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
auto ProcUnset::ToString() const -> std::string { return std::format("{}[CPrU]Unset", super::ToString()); }

Drawable ProcUnset::GetDrawable() const
{
    return { this, parent_ptr, Type::unset, "unset continuity", "unset continuity", {} };
}

auto ProcUnset::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcUnset" },
        },
        *this);
}

auto ProcUnset::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcUnset>
{
    return ApplyTokenLocationToPtr(std::make_unique<ProcUnset>(), json);
}

auto ProcUnset::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcSet::Compile(Animate::Compile& anim) { var->Set(anim, val->Get(anim)); }

auto ProcSet::ToString() const -> std::string
{
    return std::format("{}[CPrS]Setting variable {} to {}", super::ToString(), *var, *val);
}

Drawable ProcSet::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure, "variable %@ = %@", "%@ = %@",
        { var->GetDrawable(), val->GetDrawable() } };
}

auto ProcSet::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcSet" },
            { "var", var->toJSON() },
            { "val", val->toJSON() },
        },
        *this);
}

auto ProcSet::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcSet>
{
    auto var = ValueFromJSON(RequireField(json, "var", "ProcSet"));
    auto varAsValueVar = dynamic_unique_ptr_cast<ValueVar>(std::move(var));
    if (!varAsValueVar) {
        throw std::runtime_error("bad Procedure JSON: ProcSet.var must be ValueVar or ValueVarUnset");
    }
    return ApplyTokenLocationToPtr(
        std::make_unique<ProcSet>(std::move(varAsValueVar), ValueFromJSON(RequireField(json, "val", "ProcSet"))), json);
}

auto ProcSet::clone() const -> std::unique_ptr<Procedure>
{
    // we need to make a copy of the var, then dynamically cast to std::unique_ptr<ValueVar>
    auto var_clone = var->clone();
    if (ValueVar* cast = dynamic_cast<ValueVar*>(var_clone.get())) {
        std::unique_ptr<ValueVar> t(cast);
        var_clone.release();
        auto result = std::make_unique<ProcSet>(std::move(t), val->clone());
        result->SetSourceLocation(GetLine(), GetCol());
        return result;
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

auto ProcSet::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcBlam::Compile(Animate::Compile& anim)
{
    NextPoint np;
    auto c = np.Get(anim) - anim.GetPointPosition();
    anim.Append(Animate::CommandMove{ anim.GetPointPosition(), anim.GetBeatsRemaining(), c });
}

auto ProcBlam::ToString() const -> std::string { return std::format("{}[CPrB]BLAM", super::ToString()); }

Drawable ProcBlam::GetDrawable() const { return { this, parent_ptr, Type::procedure, "BLAM", "BLAM", {} }; }

auto ProcBlam::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcBlam" },
        },
        *this);
}

auto ProcBlam::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcBlam>
{
    return ApplyTokenLocationToPtr(std::make_unique<ProcBlam>(), json);
}

auto ProcBlam::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcBlam));
    Parser::Append(result, super::Serialize());
    return result;
}

Reader ProcBlam::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcBlam);
    return super::Deserialize(reader);
}

// ProcClose
void ProcClose::Compile(Animate::Compile& anim)
{
    anim.Append(Animate::CommandStill{ anim.GetPointPosition(), anim.GetBeatsRemaining(),
        Animate::CommandStill::Style::Close, CalChart::Degree{ dir->Get(anim) } });
}

auto ProcClose::ToString() const -> std::string
{
    return std::format("{}[CPrClose]Close facing {}", super::ToString(), *dir);
}

Drawable ProcClose::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure, "Close %@", "Close %@", { dir->GetDrawable() } };
}

auto ProcClose::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcClose" },
            { "dir", dir->toJSON() },
        },
        *this);
}

auto ProcClose::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcClose>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<ProcClose>(ValueFromJSON(RequireField(json, "dir", "ProcClose"))), json);
}

void ProcClose::replace(Token const* which, std::unique_ptr<Token> v) { replace_helper<NumParts>(this, which, v, dir); }

auto ProcClose::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcClose));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, dir->Serialize());
    return result;
}

Reader ProcClose::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcClose);
    reader = super::Deserialize(reader);
    std::tie(dir, reader) = DeserializeValue(reader);
    return reader;
}

// ProcCM
void ProcCM::Compile(Animate::Compile& anim) { DoCounterMarch(anim, *pnt1, *pnt2, *stps, *dir1, *dir2, *numbeats); }

auto ProcCM::ToString() const -> std::string
{
    return std::format("{}[CPrCM]CounterMarch starting at {} passing through {} stepping {} off points, first moving "
                       "{} then {} for number beats {}",
        super::ToString(), *pnt1, *pnt2, *stps, *dir1, *dir2, *numbeats);
}

Drawable ProcCM::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure,
        "CounterMarch starting at %@ passing through %@ stepping %@ off points, first moving %@ then %@ for number "
        "beats %@",
        "COUNTERMARCH %@ %@ %@, first %@ then %@ for beats %@",
        { pnt1->GetDrawable(), pnt2->GetDrawable(), stps->GetDrawable(), dir1->GetDrawable(), dir2->GetDrawable(),
            numbeats->GetDrawable() } };
}

auto ProcCM::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcCM" },
            { "pnt1", pnt1->toJSON() },
            { "pnt2", pnt2->toJSON() },
            { "stps", stps->toJSON() },
            { "dir1", dir1->toJSON() },
            { "dir2", dir2->toJSON() },
            { "numbeats", numbeats->toJSON() },
        },
        *this);
}

auto ProcCM::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcCM>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<ProcCM>(PointFromJSON(RequireField(json, "pnt1", "ProcCM")),
            PointFromJSON(RequireField(json, "pnt2", "ProcCM")), ValueFromJSON(RequireField(json, "stps", "ProcCM")),
            ValueFromJSON(RequireField(json, "dir1", "ProcCM")), ValueFromJSON(RequireField(json, "dir2", "ProcCM")),
            ValueFromJSON(RequireField(json, "numbeats", "ProcCM"))),
        json);
}

void ProcCM::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt1, pnt2, stps, dir1, dir2, numbeats);
}

auto ProcCM::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcDMCM::Compile(Animate::Compile& anim)
{
    ValueFloat steps(1.0);

    auto r1 = pnt1->Get(anim);
    auto r2 = pnt2->Get(anim);
    auto c = r2.x - r1.x;
    if (c == (r2.y - r1.y + Int2CoordUnits(2))) {
        if (c >= 0) {
            ValueDefined dir1(CC_SW);
            ValueDefined dir2(CC_W);
            DoCounterMarch(anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    } else if (c == (r1.y - r2.y - Int2CoordUnits(2))) {
        if (c >= 0) {
            ValueDefined dir1(CC_SE);
            ValueDefined dir2(CC_W);
            DoCounterMarch(anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    } else if (c == (r1.y - r2.y + Int2CoordUnits(2))) {
        if (c <= 0) {
            ValueDefined dir1(CC_NW);
            ValueDefined dir2(CC_E);
            DoCounterMarch(anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    } else if (c == (r2.y - r1.y - Int2CoordUnits(2))) {
        if (c <= 0) {
            ValueDefined dir1(CC_NE);
            ValueDefined dir2(CC_E);
            DoCounterMarch(anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    }
    anim.RegisterError(Animate::Error::INVALID_CM);
}

auto ProcDMCM::ToString() const -> std::string
{
    return std::format("{}[CPrDC]Diagonal march CounterMarch starting at {} passing through {} for number beats{}",
        super::ToString(), *pnt1, *pnt2, *numbeats);
}

Drawable ProcDMCM::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure,
        "Diagonal march CounterMarch starting at %@ passing through %@ for number beats %@", "DMCM %@ %@ for beats %@",
        { pnt1->GetDrawable(), pnt2->GetDrawable(), numbeats->GetDrawable() } };
}

auto ProcDMCM::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcDMCM" },
            { "pnt1", pnt1->toJSON() },
            { "pnt2", pnt2->toJSON() },
            { "numbeats", numbeats->toJSON() },
        },
        *this);
}

auto ProcDMCM::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcDMCM>
{
    return ApplyTokenLocationToPtr(std::make_unique<ProcDMCM>(PointFromJSON(RequireField(json, "pnt1", "ProcDMCM")),
                                       PointFromJSON(RequireField(json, "pnt2", "ProcDMCM")),
                                       ValueFromJSON(RequireField(json, "numbeats", "ProcDMCM"))),
        json);
}

void ProcDMCM::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt1, pnt2, numbeats);
}

auto ProcDMCM::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcDMHS::Compile(Animate::Compile& anim)
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
        if (!anim.Append(Animate::CommandMove(anim.GetPointPosition(), std::abs(b), c_dm))) {
            return;
        }
    }
    if (c_hs != Coord{ 0 }) {
        anim.Append(Animate::CommandMove(anim.GetPointPosition(), std::abs(b_hs), c_hs));
    }
}

auto ProcDMHS::ToString() const -> std::string
{
    return std::format("{}[CPrDH]Diagonal march then HighStep to {}", super::ToString(), *pnt);
}

Drawable ProcDMHS::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure, "Diagonal march then HighStep to %@", "DMHS %@",
        { pnt->GetDrawable() } };
}

auto ProcDMHS::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcDMHS" },
            { "pnt", pnt->toJSON() },
        },
        *this);
}

auto ProcDMHS::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcDMHS>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<ProcDMHS>(PointFromJSON(RequireField(json, "pnt", "ProcDMHS"))), json);
}

void ProcDMHS::replace(Token const* which, std::unique_ptr<Token> v) { replace_helper<NumParts>(this, which, v, pnt); }

auto ProcDMHS::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcEven::Compile(Animate::Compile& anim)
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    auto steps = float2int(anim, stps->Get(anim));
    if (steps < 0) {
        anim.Append(
            Animate::CommandMove(anim.GetPointPosition(), (unsigned)-steps, c, -CalChart::Degree{ c.Direction() }));
    } else {
        anim.Append(Animate::CommandMove(anim.GetPointPosition(), (unsigned)steps, c));
    }
}

auto ProcEven::ToString() const -> std::string
{
    return std::format("{}[CPrE]Even march of step size {} to {}", super::ToString(), *stps, *pnt);
}

Drawable ProcEven::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure, "Even march %@ to %@", "EVEN %@ %@",
        { stps->GetDrawable(), pnt->GetDrawable() } };
}

auto ProcEven::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcEven" },
            { "stps", stps->toJSON() },
            { "pnt", pnt->toJSON() },
        },
        *this);
}

auto ProcEven::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcEven>
{
    return ApplyTokenLocationToPtr(std::make_unique<ProcEven>(ValueFromJSON(RequireField(json, "stps", "ProcEven")),
                                       PointFromJSON(RequireField(json, "pnt", "ProcEven"))),
        json);
}

void ProcEven::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, stps, pnt);
}

auto ProcEven::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcEWNS::Compile(Animate::Compile& anim)
{
    auto c1 = pnt->Get(anim) - anim.GetPointPosition();
    if (c1.y != 0) {
        Coord c2{ 0, c1.y };
        auto b = CoordUnits2Int(c2.y);
        if (!anim.Append(Animate::CommandMove(anim.GetPointPosition(), std::abs(b), c2))) {
            return;
        }
    }
    if (c1.x != 0) {
        Coord c2{ c1.x, 0 };
        auto b = CoordUnits2Int(c2.x);
        if (!anim.Append(Animate::CommandMove(anim.GetPointPosition(), std::abs(b), c2))) {
            return;
        }
    }
}

auto ProcEWNS::ToString() const -> std::string
{
    return std::format("{}[CPrEWNS]March EastWest/NorthSouth to {}", super::ToString(), *pnt);
}

Drawable ProcEWNS::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure, "EastWest/NorthSouth to %@", "EW/NS to %@", { pnt->GetDrawable() } };
}

auto ProcEWNS::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcEWNS" },
            { "pnt", pnt->toJSON() },
        },
        *this);
}

auto ProcEWNS::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcEWNS>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<ProcEWNS>(PointFromJSON(RequireField(json, "pnt", "ProcEWNS"))), json);
}

void ProcEWNS::replace(Token const* which, std::unique_ptr<Token> v) { replace_helper<NumParts>(this, which, v, pnt); }

auto ProcEWNS::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcFountain::Compile(Animate::Compile& anim)
{
    auto [a, c] = [this, &anim] {
        auto f1 = CalChart::Degree{ dir1->Get(anim) };
        if (stepsize1) {
            auto f2 = stepsize1->Get(anim);
            return std::tuple<double, double>{ f2 * cos(f1), f2 * -sin(f1) };
        }
        return CreateCalChartUnitVector(CalChart::Degree{ f1 });
    }();
    auto [b, d] = [this, &anim] {
        auto f1 = CalChart::Degree{ dir2->Get(anim) };
        if (stepsize2) {
            auto f2 = stepsize2->Get(anim);
            return std::tuple<double, double>{ f2 * cos(f1), f2 * -sin(f1) };
        }
        return CreateCalChartUnitVector(CalChart::Degree{ f1 });
    }();
    auto v = pnt->Get(anim) - anim.GetPointPosition();
    auto e = CoordUnits2Float(v.x);
    auto f = CoordUnits2Float(v.y);
    auto f1 = a * d - b * c;
    if (IS_ZERO(f1)) {
        if (IS_ZERO(a - b) && IS_ZERO(c - d) && IS_ZERO(e * c - a * f)) {
            // Special case: directions are same
            if (IS_ZERO(c)) {
                f1 = f / c;
            } else {
                f1 = e / a;
            }
            if (!anim.Append(Animate::CommandMove(anim.GetPointPosition(), float2unsigned(anim, f1), v))) {
                return;
            }
        } else {
            anim.RegisterError(Animate::Error::INVALID_FNTN);
            return;
        }
    } else {
        auto f2 = (d * e - b * f) / f1;
        if (!IS_ZERO(f2)) {
            v.x = Float2CoordUnits(f2 * a);
            v.y = Float2CoordUnits(f2 * c);
            if (!anim.Append(Animate::CommandMove(anim.GetPointPosition(), float2unsigned(anim, f2), v))) {
                return;
            }
        }
        f2 = (a * f - c * e) / f1;
        if (!IS_ZERO(f2)) {
            v.x = Float2CoordUnits(f2 * b);
            v.y = Float2CoordUnits(f2 * d);
            if (!anim.Append(Animate::CommandMove(anim.GetPointPosition(), float2unsigned(anim, f2), v))) {
                return;
            }
        }
    }
}

auto ProcFountain::ToString() const -> std::string
{
    std::string result = std::format("{}[CPrF]Fountain step, first going {} then {}", super::ToString(), *dir1, *dir2);
    if (stepsize1)
        result += std::format(", first at {}", *stepsize1);
    if (stepsize2)
        result += std::format(", then at {}", *stepsize2);
    result += std::format("ending at {}", *pnt);
    return result;
}

Drawable ProcFountain::GetDrawable() const
{
    if (stepsize1 && stepsize2) {
        return { this, parent_ptr, Type::procedure,
            "Fountain step, first going %@ then %@, first at %@, then at %@, ending at %@",
            "FOUNTAIN %@ -> %@, Step %@, then %@, ending %@",
            { dir1->GetDrawable(), dir2->GetDrawable(), stepsize1->GetDrawable(), stepsize2->GetDrawable(),
                pnt->GetDrawable() } };
    }
    if (stepsize1) {
        return { this, parent_ptr, Type::procedure, "Fountain step, first going %@ then %@, first at %@, ending at %@",
            "FOUNTAIN %@ -> %@, Step %@ ending %@",
            { dir1->GetDrawable(), dir2->GetDrawable(), stepsize1->GetDrawable(), pnt->GetDrawable() } };
    }
    if (stepsize2) {
        return { this, parent_ptr, Type::procedure, "Fountain step, first going %@ then %@, then at %@, ending at %@",
            "FOUNTAIN %@ -> %@, Step %@, ending %@",
            { dir1->GetDrawable(), dir2->GetDrawable(), stepsize2->GetDrawable(), pnt->GetDrawable() } };
    }
    return { this, parent_ptr, Type::procedure, "Fountain step, first going %@ then %@, ending at %@",
        "FOUNTAIN %@ -> %@, ending %@", { dir1->GetDrawable(), dir2->GetDrawable(), pnt->GetDrawable() } };
}

auto ProcFountain::toJSON() const -> nlohmann::json
{
    auto json = nlohmann::json{
        { "type", "ProcFountain" },
        { "dir1", dir1->toJSON() },
        { "dir2", dir2->toJSON() },
        { "pnt", pnt->toJSON() },
    };
    json["stepsize1"] = stepsize1 ? stepsize1->toJSON() : nlohmann::json(nullptr);
    json["stepsize2"] = stepsize2 ? stepsize2->toJSON() : nlohmann::json(nullptr);
    return AddTokenLocation(std::move(json), *this);
}

auto ProcFountain::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcFountain>
{
    auto const* stepsize1 = OptionalField(json, "stepsize1");
    auto const* stepsize2 = OptionalField(json, "stepsize2");
    auto parsedStep1 = std::unique_ptr<Value>{};
    auto parsedStep2 = std::unique_ptr<Value>{};
    if (stepsize1 && !stepsize1->is_null()) {
        parsedStep1 = ValueFromJSON(*stepsize1);
    }
    if (stepsize2 && !stepsize2->is_null()) {
        parsedStep2 = ValueFromJSON(*stepsize2);
    }
    return ApplyTokenLocationToPtr(
        std::make_unique<ProcFountain>(ValueFromJSON(RequireField(json, "dir1", "ProcFountain")),
            ValueFromJSON(RequireField(json, "dir2", "ProcFountain")), std::move(parsedStep1), std::move(parsedStep2),
            PointFromJSON(RequireField(json, "pnt", "ProcFountain"))),
        json);
}

void ProcFountain::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, dir1, dir2, stepsize1, stepsize2, pnt);
}

auto ProcFountain::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcFM::Compile(Animate::Compile& anim)
{
    auto b = float2int(anim, stps->Get(anim));
    if (b != 0) {
        auto c = CreateCalChartVector(CalChart::Degree{ dir->Get(anim) }, stps->Get(anim));
        if (c != Coord{ 0 }) {
            if (b < 0) {
                anim.Append(
                    Animate::CommandMove(anim.GetPointPosition(), (unsigned)-b, c, -CalChart::Degree{ c.Direction() }));
            } else {
                anim.Append(Animate::CommandMove(anim.GetPointPosition(), (unsigned)b, c));
            }
        }
    }
}

auto ProcFM::ToString() const -> std::string
{
    return std::format("{}[CPrFM]Forward march for steps {} in direction {}", super::ToString(), *stps, *dir);
}

Drawable ProcFM::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure, "Forward march %@ %@", "FM %@ %@",
        { stps->GetDrawable(), dir->GetDrawable() } };
}

auto ProcFM::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcFM" },
            { "stps", stps->toJSON() },
            { "dir", dir->toJSON() },
        },
        *this);
}

auto ProcFM::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcFM>
{
    return ApplyTokenLocationToPtr(std::make_unique<ProcFM>(ValueFromJSON(RequireField(json, "stps", "ProcFM")),
                                       ValueFromJSON(RequireField(json, "dir", "ProcFM"))),
        json);
}

void ProcFM::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, stps, dir);
}

auto ProcFM::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcFMTO::Compile(Animate::Compile& anim)
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    if (c != Coord{ 0 }) {
        anim.Append(Animate::CommandMove(anim.GetPointPosition(), (unsigned)c.DM_Magnitude(), c));
    }
}

auto ProcFMTO::ToString() const -> std::string
{
    return std::format("{}[CPrFMT]Forward march to {}", super::ToString(), *pnt);
}

Drawable ProcFMTO::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure, "Forward march to %@", "FMTO %@", { pnt->GetDrawable() } };
}

auto ProcFMTO::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcFMTO" },
            { "pnt", pnt->toJSON() },
        },
        *this);
}

auto ProcFMTO::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcFMTO>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<ProcFMTO>(PointFromJSON(RequireField(json, "pnt", "ProcFMTO"))), json);
}

void ProcFMTO::replace(Token const* which, std::unique_ptr<Token> v) { replace_helper<NumParts>(this, which, v, pnt); }

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

auto ProcFMTO::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcGrid::Compile(Animate::Compile& anim)
{
    auto gridc = Float2CoordUnits(grid->Get(anim));

    Coord c;
    c.x = roundcoord(anim.GetPointPosition().x, gridc);
    // Adjust so 4 step grid will be on visible grid
    c.y = roundcoord(anim.GetPointPosition().y - Int2CoordUnits(2), gridc) + Int2CoordUnits(2);

    c -= anim.GetPointPosition();
    if (c != Coord{ 0 }) {
        anim.Append(Animate::CommandMove(anim.GetPointPosition(), 0, c));
    }
}

auto ProcGrid::ToString() const -> std::string
{
    return std::format("{}[CPrG]Move on Grid of {} spacing", super::ToString(), *grid);
}

Drawable ProcGrid::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure, "Move on Grid of %@ spacing", "GRID %@", { grid->GetDrawable() } };
}

auto ProcGrid::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcGrid" },
            { "grid", grid->toJSON() },
        },
        *this);
}

auto ProcGrid::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcGrid>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<ProcGrid>(ValueFromJSON(RequireField(json, "grid", "ProcGrid"))), json);
}

void ProcGrid::replace(Token const* which, std::unique_ptr<Token> v) { replace_helper<NumParts>(this, which, v, grid); }

auto ProcGrid::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcHSCM::Compile(Animate::Compile& anim)
{
    ValueFloat steps(1.0);

    auto r1 = pnt1->Get(anim);
    auto r2 = pnt2->Get(anim);
    if ((r1.y - r2.y) == Int2CoordUnits(2)) {
        if (r2.x >= r1.x) {
            ValueDefined dirs(CC_S);
            ValueDefined dirw(CC_W);
            DoCounterMarch(anim, *pnt1, *pnt2, steps, dirs, dirw, *numbeats);
            return;
        }
    } else if ((r1.y - r2.y) == -Int2CoordUnits(2)) {
        if (r1.x >= r2.x) {
            ValueDefined dirn(CC_N);
            ValueDefined dire(CC_E);
            DoCounterMarch(anim, *pnt1, *pnt2, steps, dirn, dire, *numbeats);
            return;
        }
    }
    anim.RegisterError(Animate::Error::INVALID_CM);
}

auto ProcHSCM::ToString() const -> std::string
{
    return std::format("{}[CPrHCM]High Step CounterMarch starting at {} passing through {} for number beats{}",
        super::ToString(), *pnt1, *pnt2, *numbeats);
}

Drawable ProcHSCM::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure,
        "High Step CounterMarch starting at %@ passing through %@ for number beats %@", "HSCM %@ -> %@ for beats %@",
        { pnt1->GetDrawable(), pnt2->GetDrawable(), numbeats->GetDrawable() } };
}

auto ProcHSCM::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcHSCM" },
            { "pnt1", pnt1->toJSON() },
            { "pnt2", pnt2->toJSON() },
            { "numbeats", numbeats->toJSON() },
        },
        *this);
}

auto ProcHSCM::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcHSCM>
{
    return ApplyTokenLocationToPtr(std::make_unique<ProcHSCM>(PointFromJSON(RequireField(json, "pnt1", "ProcHSCM")),
                                       PointFromJSON(RequireField(json, "pnt2", "ProcHSCM")),
                                       ValueFromJSON(RequireField(json, "numbeats", "ProcHSCM"))),
        json);
}

void ProcHSCM::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, pnt1, pnt2, numbeats);
}

auto ProcHSCM::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcHSDM::Compile(Animate::Compile& anim)
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
        if (!anim.Append(Animate::CommandMove(anim.GetPointPosition(), std::abs(b), c_hs))) {
            return;
        }
    }
    if (c_dm != Coord{ 0 }) {
        b = CoordUnits2Int(c_dm.x);
        anim.Append(Animate::CommandMove(anim.GetPointPosition(), std::abs(b), c_dm));
    }
}

auto ProcHSDM::ToString() const -> std::string
{
    return std::format("{}[CPrHD]HighStep then Diagonal march to {}", super::ToString(), *pnt);
}

Drawable ProcHSDM::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure, "HighStep then Diagonal march to %@", "HSDM %@",
        { pnt->GetDrawable() } };
}

auto ProcHSDM::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcHSDM" },
            { "pnt", pnt->toJSON() },
        },
        *this);
}

auto ProcHSDM::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcHSDM>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<ProcHSDM>(PointFromJSON(RequireField(json, "pnt", "ProcHSDM"))), json);
}

void ProcHSDM::replace(Token const* which, std::unique_ptr<Token> v) { replace_helper<NumParts>(this, which, v, pnt); }

auto ProcHSDM::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcMagic::Compile(Animate::Compile& anim)
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    anim.Append(Animate::CommandMove(anim.GetPointPosition(), 0, c));
}

auto ProcMagic::ToString() const -> std::string
{
    return std::format("{}[CPrM]Magic step to {}", super::ToString(), *pnt);
}

Drawable ProcMagic::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure, "Magic step to %@", "MAGIC %@", { pnt->GetDrawable() } };
}

auto ProcMagic::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcMagic" },
            { "pnt", pnt->toJSON() },
        },
        *this);
}

auto ProcMagic::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcMagic>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<ProcMagic>(PointFromJSON(RequireField(json, "pnt", "ProcMagic"))), json);
}

void ProcMagic::replace(Token const* which, std::unique_ptr<Token> v) { replace_helper<NumParts>(this, which, v, pnt); }

auto ProcMagic::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcMarch::Compile(Animate::Compile& anim)
{
    auto b = float2int(anim, stps->Get(anim));
    if (b != 0) {
        auto angle = CalChart::Degree{ dir->Get(anim) };
        auto mag = stpsize->Get(anim) * stps->Get(anim);
        Coord c{ Float2CoordUnits(cos(angle) * mag), static_cast<Coord::units>(-Float2CoordUnits(sin(angle) * mag)) };
        if (c != Coord{ 0 }) {
            if (facedir)
                anim.Append(Animate::CommandMove(
                    anim.GetPointPosition(), (unsigned)std::abs(b), c, CalChart::Degree{ facedir->Get(anim) }));
            else if (b < 0) {
                anim.Append(
                    Animate::CommandMove(anim.GetPointPosition(), (unsigned)-b, c, -CalChart::Degree{ c.Direction() }));
            } else {
                anim.Append(Animate::CommandMove(anim.GetPointPosition(), (unsigned)b, c));
            }
        }
    }
}

auto ProcMarch::ToString() const -> std::string
{
    return std::format("{}[CPrm]March step size {} for steps {} in direction {}{}", super::ToString(), *stpsize, *stps,
        *dir, facedir ? std::format(" facing {}", *facedir) : "");
}

Drawable ProcMarch::GetDrawable() const
{
    if (facedir) {
        return { this, parent_ptr, Type::procedure, "March step size %@ for %@ in direction %@ facing %@",
            "MARCH %@ for %@ DIR %@ FACING %@",
            { stpsize->GetDrawable(), stps->GetDrawable(), dir->GetDrawable(), facedir->GetDrawable() } };
    }
    return { this, parent_ptr, Type::procedure, "March step size %@ for steps %@ in direction %@",
        "MARCH %@ for %@ DIR %@", { stpsize->GetDrawable(), stps->GetDrawable(), dir->GetDrawable() } };
}

auto ProcMarch::toJSON() const -> nlohmann::json
{
    auto json = nlohmann::json{
        { "type", "ProcMarch" },
        { "stpsize", stpsize->toJSON() },
        { "stps", stps->toJSON() },
        { "dir", dir->toJSON() },
    };
    if (facedir) {
        json["facedir"] = facedir->toJSON();
    }
    return AddTokenLocation(std::move(json), *this);
}

auto ProcMarch::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcMarch>
{
    auto const* facedir = OptionalField(json, "facedir");
    auto parsedFace = std::unique_ptr<Value>{};
    if (facedir && !facedir->is_null()) {
        parsedFace = ValueFromJSON(*facedir);
    }
    return ApplyTokenLocationToPtr(
        std::make_unique<ProcMarch>(ValueFromJSON(RequireField(json, "stpsize", "ProcMarch")),
            ValueFromJSON(RequireField(json, "stps", "ProcMarch")),
            ValueFromJSON(RequireField(json, "dir", "ProcMarch")), std::move(parsedFace)),
        json);
}

void ProcMarch::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, stpsize, stps, dir, facedir);
}

auto ProcMarch::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcMT::Compile(Animate::Compile& anim)
{
    auto b = float2int(anim, numbeats->Get(anim));
    if (b != 0) {
        anim.Append(Animate::CommandStill(anim.GetPointPosition(), (unsigned)std::abs(b),
            Animate::CommandStill::Style::MarkTime, CalChart::Degree{ dir->Get(anim) }));
    }
}

auto ProcMT::ToString() const -> std::string
{
    return std::format("{}[CPrMT]MarkTime for {} facing {}", super::ToString(), *numbeats, *dir);
}

Drawable ProcMT::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure, "MarkTime %@ %@", "MT %@ %@",
        { numbeats->GetDrawable(), dir->GetDrawable() } };
}

auto ProcMT::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcMT" },
            { "numbeats", numbeats->toJSON() },
            { "dir", dir->toJSON() },
        },
        *this);
}

auto ProcMT::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcMT>
{
    return ApplyTokenLocationToPtr(std::make_unique<ProcMT>(ValueFromJSON(RequireField(json, "numbeats", "ProcMT")),
                                       ValueFromJSON(RequireField(json, "dir", "ProcMT"))),
        json);
}

void ProcMT::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, numbeats, dir);
}

auto ProcMT::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcMTRM::Compile(Animate::Compile& anim)
{
    anim.Append(Animate::CommandStill(anim.GetPointPosition(), anim.GetBeatsRemaining(),
        Animate::CommandStill::Style::MarkTime, CalChart::Degree{ dir->Get(anim) }));
}

auto ProcMTRM::ToString() const -> std::string
{
    return std::format("{}[CPrMTR]MarkTime for Remaining Beats facing {}", super::ToString(), *dir);
}
Drawable ProcMTRM::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure, "MarkTime for Remaining %@", "MTRM %@", { dir->GetDrawable() } };
}

auto ProcMTRM::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcMTRM" },
            { "dir", dir->toJSON() },
        },
        *this);
}

auto ProcMTRM::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcMTRM>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<ProcMTRM>(ValueFromJSON(RequireField(json, "dir", "ProcMTRM"))), json);
}

void ProcMTRM::replace(Token const* which, std::unique_ptr<Token> v) { replace_helper<NumParts>(this, which, v, dir); }

auto ProcMTRM::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcNSEW::Compile(Animate::Compile& anim)
{
    auto c1 = pnt->Get(anim) - anim.GetPointPosition();
    if (c1.x != 0) {
        Coord c2{ c1.x, 0 };
        auto b = CoordUnits2Int(c2.x);
        if (!anim.Append(Animate::CommandMove(anim.GetPointPosition(), std::abs(b), c2))) {
            return;
        }
    }
    if (c1.y != 0) {
        Coord c2{ 0, c1.y };
        auto b = CoordUnits2Int(c2.y);
        if (!anim.Append(Animate::CommandMove(anim.GetPointPosition(), std::abs(b), c2))) {
            return;
        }
    }
}

auto ProcNSEW::ToString() const -> std::string
{
    return std::format("{}[CPrNSEW]March NorthSouth/EastWest to {}", super::ToString(), *pnt);
}

Drawable ProcNSEW::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure, "NorthSouth/EastWest to %@", "NSEW %@", { pnt->GetDrawable() } };
}

auto ProcNSEW::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcNSEW" },
            { "pnt", pnt->toJSON() },
        },
        *this);
}

auto ProcNSEW::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcNSEW>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<ProcNSEW>(PointFromJSON(RequireField(json, "pnt", "ProcNSEW"))), json);
}

void ProcNSEW::replace(Token const* which, std::unique_ptr<Token> v) { replace_helper<NumParts>(this, which, v, pnt); }

auto ProcNSEW::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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
void ProcRotate::Compile(Animate::Compile& anim)
{
    // Most of the work is converting to polar coordinates
    auto c = pnt->Get(anim);
    auto rad = anim.GetPointPosition() - c;
    auto start_ang = [c, &anim] {
        if (c == anim.GetPointPosition()) {
            return CalChart::Degree{ anim.GetVarValue(Cont::Variable::DOH) };
        }
        return CalChart::Degree{ c.Direction(anim.GetPointPosition()) };
    }();
    int b = float2int(anim, stps->Get(anim));
    auto angle = CalChart::Degree{ ang->Get(anim) };
    bool backwards = false;
    if (b < 0) {
        backwards = true;
    }
    anim.Append(Animate::CommandRotate((unsigned)std::abs(b), c,
        // Don't use Magnitude() because
        // we want Coord numbers
        sqrt(rad.x * rad.x + rad.y * rad.y), start_ang, start_ang + angle, backwards));
}

auto ProcRotate::ToString() const -> std::string
{
    return std::format("{}[CPrR]Rotate at angle {} for {} around pivot point {}", super::ToString(), *ang, *stps, *pnt);
}

Drawable ProcRotate::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure, "Rotate at angle %@ for steps %@ around pivot point %@",
        "ROTATE %@ for %@ around %@", { ang->GetDrawable(), stps->GetDrawable(), pnt->GetDrawable() } };
}

auto ProcRotate::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcRotate" },
            { "ang", ang->toJSON() },
            { "stps", stps->toJSON() },
            { "pnt", pnt->toJSON() },
        },
        *this);
}

auto ProcRotate::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcRotate>
{
    return ApplyTokenLocationToPtr(std::make_unique<ProcRotate>(ValueFromJSON(RequireField(json, "ang", "ProcRotate")),
                                       ValueFromJSON(RequireField(json, "stps", "ProcRotate")),
                                       PointFromJSON(RequireField(json, "pnt", "ProcRotate"))),
        json);
}

void ProcRotate::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, ang, stps, pnt);
}

auto ProcRotate::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
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

// ProcStandAndPlay
void ProcStandAndPlay::Compile(Animate::Compile& anim)
{
    auto b = float2int(anim, numbeats->Get(anim));
    if (b != 0) {
        anim.Append(Animate::CommandStill(anim.GetPointPosition(), (unsigned)std::abs(b),
            Animate::CommandStill::Style::StandAndPlay, CalChart::Degree{ dir->Get(anim) }));
    }
}

auto ProcStandAndPlay::ToString() const -> std::string
{
    return std::format("{}[CPrStandAndPlay]Stand & Play for {} facing {}", super::ToString(), *numbeats, *dir);
}

Drawable ProcStandAndPlay::GetDrawable() const
{
    return { this, parent_ptr, Type::procedure, "Stand & Play %@ %@", "Stand %@ %@",
        { numbeats->GetDrawable(), dir->GetDrawable() } };
}

auto ProcStandAndPlay::toJSON() const -> nlohmann::json
{
    return AddTokenLocation(
        nlohmann::json{
            { "type", "ProcStandAndPlay" },
            { "numbeats", numbeats->toJSON() },
            { "dir", dir->toJSON() },
        },
        *this);
}

auto ProcStandAndPlay::fromJSON(nlohmann::json const& json) -> std::unique_ptr<ProcStandAndPlay>
{
    return ApplyTokenLocationToPtr(
        std::make_unique<ProcStandAndPlay>(ValueFromJSON(RequireField(json, "numbeats", "ProcStandAndPlay")),
            ValueFromJSON(RequireField(json, "dir", "ProcStandAndPlay"))),
        json);
}

void ProcStandAndPlay::replace(Token const* which, std::unique_ptr<Token> v)
{
    replace_helper<NumParts>(this, which, v, numbeats, dir);
}

auto ProcStandAndPlay::Serialize() const -> std::vector<std::byte>
{
    auto result = std::vector<std::byte>{};
    Parser::Append(result, static_cast<uint8_t>(SerializationToken::ProcStandAndPlay));
    Parser::Append(result, super::Serialize());
    Parser::Append(result, numbeats->Serialize());
    Parser::Append(result, dir->Serialize());
    return result;
}

Reader ProcStandAndPlay::Deserialize(Reader reader)
{
    reader = CheckForToken(reader, 1, SerializationToken::ProcStandAndPlay);
    reader = super::Deserialize(reader);
    std::tie(numbeats, reader) = DeserializeValue(reader);
    std::tie(dir, reader) = DeserializeValue(reader);
    return reader;
}
}
