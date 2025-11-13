#include "CalChartContinuity.h"
#include <catch2/catch_test_macros.hpp>

using namespace CalChart;

namespace {
// Test Suite stuff
struct Continuity_values {
    std::string text;
    std::string GetText;
};

bool Check_Continuity(Continuity const&, Continuity_values const&)
{
    return true;
}
}

TEST_CASE("ContinuitySerializedTests", "CalChartShowTests")
{
    // Set some text
    for (auto i : {
             "mt E REM",
             "BLAM",
             "close 1 0",
             "Countermarch R1 R2 1 N E 16",
             "DMCM SP NP 6 / 3",
             "DMHS NP",
             "EVEN 10 + 3 NP",
             "EWNS NP",
             "FM 10 - 3 N",
             "FMTO R3",
             "FOUNTAIN DIR(NP) DIRFROM(SP NP) DIST(NP) 3 NP",
             "FOUNTAIN DIR(NP) DIRFROM(SP NP) NP",
             "GRID DISTFROM(R1 R2)",
             "HSCM NP R1 EITHER(N S R1)",
             "HSDM NP",
             "MAGIC NP",
             "MARCH GV STEP(2 2 R1) OPP(S)",
             "MARCH GV STEP(2 2 R1) OPP(S) S",
             "MT 1 1",
             "MTRM 10.5",
             "NSEW SP",
             "ROTATE 90 SH R2",
             "ROTATE -90 SH R2",
             "A = 10 * 9",
             "  ",

         }) {
        auto uut1 = Continuity{ i };
        auto serialize_result = uut1.Serialize();
        auto reader = Reader({ serialize_result.data(), serialize_result.size() });
        auto uut2 = Continuity{ reader };
        CHECK(uut1 == uut2);
    }
}

TEST_CASE("CalChartContinuityTests", "CalChartShowTests")
{
    // test some defaults:
    Continuity_values values;
    values.text = "";
    values.GetText = values.text;

    // test defaults
    Continuity underTest;
    CHECK(Check_Continuity(underTest, values));

    // test defaults with different init
    Continuity underTest2;
    values.GetText = values.text;
    CHECK(Check_Continuity(underTest2, values));

    // Set some text
    values.text = "mt E REM";
    values.GetText = values.text;
    CHECK(Check_Continuity(underTest2, values));

    underTest2 = Continuity{ "mt E REM" };
    values.text = "mt E REM";
    values.GetText = values.text;
    CHECK(Check_Continuity(underTest2, values));

    // Set some text
    underTest2 = Continuity{ "ewns np" };
    values.text = "ewns np";
    values.GetText = values.text;
    CHECK(Check_Continuity(underTest2, values));

    // Reset text
    underTest2 = Continuity{ "" };
    values.text = "";
    values.GetText = values.text;
    CHECK(Check_Continuity(underTest2, values));
}
