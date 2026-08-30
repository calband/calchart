#include "CalChartContinuity.h"
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

using namespace CalChart;

namespace {
// Test Suite stuff
struct Continuity_values {
    std::string text;
    std::string GetText;
};

bool Check_Continuity(Continuity const&, Continuity_values const&) { return true; }
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

    SECTION("JSON round trip")
    {
        auto const source = Continuity{ "MT E REM" };
        auto const json = source.toJSON();

        CHECK(json.contains("procedures"));
        CHECK(json.at("procedures").is_array());
        REQUIRE_FALSE(json.at("procedures").empty());
        CHECK(json.at("procedures").at(0).is_object());
        CHECK(json.at("procedures").at(0).at("type") == "ProcMT");
        CHECK(json.at("procedures").at(0).contains("numbeats"));
        CHECK(json.at("procedures").at(0).contains("dir"));
        CHECK(json.at("procedures").at(0).at("numbeats").contains("type"));
        CHECK(json.at("procedures").at(0).at("dir").contains("type"));
        CHECK(json.at("procedures").at(0).contains("line"));
        CHECK(json.at("procedures").at(0).contains("col"));
        CHECK_FALSE(json.at("procedures").at(0).contains("data"));

        auto const fromJson = Continuity(json);
        CHECK(source == fromJson);
    }

    SECTION("JSON round trip with optional null fields")
    {
        auto const fountainSource = Continuity{ "FOUNTAIN DIR(NP) DIRFROM(SP NP) NP" };
        auto const fountainJson = fountainSource.toJSON();
        auto const& fountainProc = fountainJson.at("procedures").at(0);

        CHECK(fountainProc.at("type") == "ProcFountain");
        CHECK(fountainProc.contains("stepsize1"));
        CHECK(fountainProc.contains("stepsize2"));
        CHECK(fountainProc.at("stepsize1").is_null());
        CHECK(fountainProc.at("stepsize2").is_null());
        CHECK(Continuity(fountainJson) == fountainSource);

        auto const marchSource = Continuity{ "MARCH GV STEP(2 2 R1) OPP(S)" };
        auto const marchJson = marchSource.toJSON();
        auto const& marchProc = marchJson.at("procedures").at(0);

        CHECK(marchProc.at("type") == "ProcMarch");
        CHECK(!marchProc.contains("facedir"));
        CHECK(Continuity(marchJson) == marchSource);
    }

    SECTION("JSON parse supports omitted optional fields")
    {
        auto json = nlohmann::json{
            { "procedures",
                nlohmann::json::array({
                    nlohmann::json{
                        { "type", "ProcFountain" },
                        { "dir1",
                            nlohmann::json{
                                { "type", "FuncDir" }, { "pnt", nlohmann::json{ { "type", "NextPoint" } } } } },
                        { "dir2",
                            nlohmann::json{ { "type", "FuncDirFrom" },
                                { "pnt1", nlohmann::json{ { "type", "StartPoint" } } },
                                { "pnt2", nlohmann::json{ { "type", "NextPoint" } } } } },
                        { "pnt", nlohmann::json{ { "type", "NextPoint" } } },
                    },
                }) },
        };

        auto const fromJson = Continuity(json);
        auto const reserialized = fromJson.toJSON();
        auto const& proc = reserialized.at("procedures").at(0);
        CHECK(proc.at("type") == "ProcFountain");
        CHECK(proc.contains("stepsize1"));
        CHECK(proc.contains("stepsize2"));
        CHECK(proc.at("stepsize1").is_null());
        CHECK(proc.at("stepsize2").is_null());
    }

    SECTION("JSON parse failures")
    {
        auto const missingProcedures = nlohmann::json{};
        CHECK_THROWS(Continuity(missingProcedures));

        auto const badProcedureType = nlohmann::json{
            { "procedures", nlohmann::json::array({ "not-procedure-object" }) },
        };
        CHECK_THROWS(Continuity(badProcedureType));

        auto const missingField = nlohmann::json{
            { "procedures",
                nlohmann::json::array({
                    nlohmann::json{
                        { "type", "ProcMT" },
                        { "dir", nlohmann::json{ { "type", "ValueDefined" }, { "defined", "E" } } },
                    },
                }) },
        };
        CHECK_THROWS(Continuity(missingField));

        auto const invalidDefinedValue = nlohmann::json{
            { "procedures",
                nlohmann::json::array({
                    nlohmann::json{
                        { "type", "ProcMT" },
                        { "numbeats", nlohmann::json{ { "type", "ValueREM" } } },
                        { "dir", nlohmann::json{ { "type", "ValueDefined" }, { "defined", "BAD_DIR" } } },
                    },
                }) },
        };
        CHECK_THROWS(Continuity(invalidDefinedValue));

        auto const invalidRefPoint = nlohmann::json{
            { "procedures",
                nlohmann::json::array({
                    nlohmann::json{
                        { "type", "ProcFMTO" },
                        { "pnt", nlohmann::json{ { "type", "RefPoint" }, { "refnum", -1 } } },
                    },
                }) },
        };
        CHECK_THROWS(Continuity(invalidRefPoint));

        auto const missingLocationColumn = nlohmann::json{
            { "procedures",
                nlohmann::json::array({
                    nlohmann::json{
                        { "type", "ProcMT" },
                        { "line", 10 },
                        { "numbeats", nlohmann::json{ { "type", "ValueREM" } } },
                        { "dir", nlohmann::json{ { "type", "ValueDefined" }, { "defined", "E" } } },
                    },
                }) },
        };
        CHECK_THROWS(Continuity(missingLocationColumn));

        auto const invalidNestedLocationType = nlohmann::json{
            { "procedures",
                nlohmann::json::array({
                    nlohmann::json{
                        { "type", "ProcMT" },
                        { "numbeats", nlohmann::json{ { "type", "ValueREM" }, { "line", -1 }, { "col", 3 } } },
                        { "dir", nlohmann::json{ { "type", "ValueDefined" }, { "defined", "E" } } },
                    },
                }) },
        };
        CHECK_THROWS(Continuity(invalidNestedLocationType));
    }
}
