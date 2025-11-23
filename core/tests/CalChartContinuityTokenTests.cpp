#include "CalChartAnimationCommand.h"
#include "CalChartAnimationCompile.h"
#include "CalChartContinuityToken.h"
#include "CalChartFileFormat.h"
#include "CalChartPoint.h"
#include "CalChartSheet.h"
#include <catch2/catch_test_macros.hpp>
#include <iostream>

using namespace CalChart;

// this will just run things through the serializer
template <typename T>
auto serializeDeserialize(T const& cont)
{
    auto result = cont.Serialize();
    auto uut2 = T{};
    uut2.Deserialize(Reader{ { result.data(), result.size() } });
    return uut2;
}

// this will pull out the string:
template <typename T>
auto print(T const& cont)
{
    std::stringstream os;
    cont.Print(os);
    return os.str();
}

template <typename Sheets, typename Conts>
auto GetCompiledResults(Sheets const& sheets, Conts const& proc)
{
    Animate::Variables vars{};
    auto animationData = Animate::AnimationData{
        0,
        sheets.begin()->GetMarcher(0),
        (sheets.begin() + 1)->GetMarcher(0).GetPos(),
        sheets.begin()->GetBeats(),
        true
    };

    return Animate::CreateCompileResult(animationData, &proc, vars);
}

auto CreateSheetsForTest(Coord begin, Coord end, int beats)
{
    auto sheets = std::vector<Sheet>{};
    auto sheet = Sheet(1);
    sheet.SetBeats(beats);
    sheet.SetPosition({ Int2CoordUnits(begin.x), Int2CoordUnits(begin.y) }, 0);
    sheets.push_back(sheet);
    sheet.SetPosition({ Int2CoordUnits(end.x), Int2CoordUnits(end.y) }, 0);
    sheets.push_back(sheet);
    return sheets;
}

auto CreateSheetsForTest(Coord begin, Coord ref1, Coord ref2, Coord end, int beats)
{
    auto sheets = CreateSheetsForTest(begin, end, beats);
    sheets.at(0).SetPosition({ Int2CoordUnits(ref1.x), Int2CoordUnits(ref1.y) }, 0, 1);
    sheets.at(0).SetPosition({ Int2CoordUnits(ref2.x), Int2CoordUnits(ref2.y) }, 0, 2);
    return sheets;
}

template <typename T>
auto Compare(T const& test, T const& gold)
{
    auto whereMiss = std::mismatch(test.begin(), test.end(), gold.begin(), gold.end(), [](auto&& a, auto&& b) {
        return a == b;
    });
    return (whereMiss.first == test.end()) && (whereMiss.second == gold.end());
}

TEST_CASE("Fountain", "CalChartContinuityToken")
{
    auto uut = std::make_unique<Cont::ProcFountain>(std::make_unique<Cont::ValueDefined>(Cont::DefinedValue::CC_NE), std::make_unique<Cont::ValueDefined>(Cont::DefinedValue::CC_E), std::make_unique<Cont::ValueFloat>(2), std::make_unique<Cont::ValueFloat>(1), std::make_unique<Cont::NextPoint>());

    CHECK(*uut == serializeDeserialize(*uut));
    CHECK(*uut == *uut);
    CHECK(print(*uut) == "[CT][CPr]Procedure: [CPrF]Fountain step, first going [CT][CV]Value:[CVC]Defined:NE then [CT][CV]Value:[CVC]Defined:E, first at [CT][CV]Value:[CVF]2, then at [CT][CV]Value:[CVF]1ending at [CT][CP]Point:[CNP]Next Point");

    // now move things into a vector so we can work with it
    auto procs = std::vector<std::unique_ptr<Cont::Procedure>>{};
    procs.push_back(std::move(uut));
    auto sheets = CreateSheetsForTest({ 0, 0 }, { 8, 16 }, 14);
    auto continuity = Continuity(std::move(procs));
    auto [compiledResults, errors] = GetCompiledResults(sheets, continuity);

    auto goldCompile = std::vector<Animate::Command>{
        Animate::CommandMove{ Coord{ 0, 0 }, 6, Coord{ 128, 128 } },
        Animate::CommandMove{ Coord{ 128, 128 }, 8, Coord{ 0, 128 } },
    };
    CHECK(Compare(compiledResults, goldCompile));
}

TEST_CASE("Fountain with nulls", "CalChartContinuityToken")
{
    auto uut = std::make_unique<Cont::ProcFountain>(std::make_unique<Cont::ValueDefined>(Cont::DefinedValue::CC_NE), std::make_unique<Cont::ValueDefined>(Cont::DefinedValue::CC_E), nullptr, nullptr, std::make_unique<Cont::NextPoint>());

    CHECK(*uut == serializeDeserialize(*uut));
    CHECK(*uut == *uut);
    CHECK(print(*uut) == "[CT][CPr]Procedure: [CPrF]Fountain step, first going [CT][CV]Value:[CVC]Defined:NE then [CT][CV]Value:[CVC]Defined:Eending at [CT][CP]Point:[CNP]Next Point");

    // now move things into a vector so we can work with it
    auto procs = std::vector<std::unique_ptr<Cont::Procedure>>{};
    procs.push_back(std::move(uut));
    auto sheets = CreateSheetsForTest({ 0, 0 }, { 8, 16 }, 16);
    auto continuity = Continuity(std::move(procs));
    auto [compiledResults, errors] = GetCompiledResults(sheets, continuity);

    auto goldCompile = std::vector<Animate::Command>{
        Animate::CommandMove{ Coord{ 0, 0 }, 8, Coord{ 128, 128 } },
        Animate::CommandMove{ Coord{ 128, 128 }, 8, Coord{ 0, 128 } },
    };

    CHECK(Compare(compiledResults, goldCompile));
}

TEST_CASE("HSCM", "CalChartContinuityToken")
{
    auto uut = std::make_unique<Cont::ProcHSCM>(std::make_unique<Cont::RefPoint>(1), std::make_unique<Cont::RefPoint>(2), std::make_unique<Cont::ValueFloat>(40));

    CHECK(*uut == serializeDeserialize(*uut));
    CHECK(*uut == *uut);
    CHECK(print(*uut) == "[CT][CPr]Procedure: [CPrHCM]High Step CounterMarch starting at [CT][CP]Point:[CRP]Ref Point 1 passing through [CT][CP]Point:[CRP]Ref Point 2 for number beats[CT][CV]Value:[CVF]40");

    // now move things into a vector so we can work with it
    auto procs = std::vector<std::unique_ptr<Cont::Procedure>>{};
    procs.push_back(std::move(uut));
    auto sheets = CreateSheetsForTest({ 0, 0 }, { -8, 0 }, { 8, -2 }, { 0, 0 }, 40);
    auto continuity = Continuity(std::move(procs));
    auto [compiledResults, errors] = GetCompiledResults(sheets, continuity);

    auto goldCompile = std::vector<Animate::Command>{
        Animate::CommandMove{ Coord{ 0, 0 }, 9, Coord{ -16 * 9, 0 } },
        Animate::CommandMove{ Coord{ -144, 0 }, 2, Coord{ 0, -16 * 2 } },
        Animate::CommandMove{ Coord{ -144, -32 }, 18, Coord{ 16 * 18, 0 } },
        Animate::CommandMove{ Coord{ 144, -32 }, 2, Coord{ 0, 16 * 2 } },
        Animate::CommandMove{ Coord{ 144, 0 }, 9, Coord{ -16 * 9, 0 } },
    };
    CHECK(Compare(compiledResults, goldCompile));
}

TEST_CASE("DMCM", "CalChartContinuityToken")
{
    auto uut = std::make_unique<Cont::ProcDMCM>(std::make_unique<Cont::RefPoint>(1), std::make_unique<Cont::RefPoint>(2), std::make_unique<Cont::ValueFloat>(40));

    CHECK(*uut == serializeDeserialize(*uut));
    CHECK(*uut == *uut);
    CHECK(print(*uut) == "[CT][CPr]Procedure: [CPrDC]Diagonal march CounterMarch starting at [CT][CP]Point:[CRP]Ref Point 1 passing through [CT][CP]Point:[CRP]Ref Point 2 for number beats[CT][CV]Value:[CVF]40");

    // now move things into a vector so we can work with it
    auto procs = std::vector<std::unique_ptr<Cont::Procedure>>{};
    procs.push_back(std::move(uut));
    auto sheets = CreateSheetsForTest({ 0, 0 }, { -8, 8 }, { 8, -10 }, { 0, 0 }, 40);
    auto continuity = Continuity(std::move(procs));
    auto [compiledResults, errors] = GetCompiledResults(sheets, continuity);

    auto goldCompile = std::vector<Animate::Command>{
        Animate::CommandMove{ Coord{ 0, 0 }, 9, Coord{ -16 * 9, 16 * 9 } },
        Animate::CommandMove{ Coord{ -144, 144 }, 2, Coord{ 0, -16 * 2 } },
        Animate::CommandMove{ Coord{ -144, 112 }, 18, Coord{ 16 * 18, -16 * 18 } },
        Animate::CommandMove{ Coord{ 144, -176 }, 2, Coord{ 0, 16 * 2 } },
        Animate::CommandMove{ Coord{ 144, -144 }, 9, Coord{ -16 * 9, 16 * 9 } },
    };
    CHECK(Compare(compiledResults, goldCompile));
}
