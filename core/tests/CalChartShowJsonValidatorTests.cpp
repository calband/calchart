/*
 * CalChartShowJsonValidatorTests.cpp
 * Unit tests for CalChartShowJsonValidator
 */

#include "CalChartShowJsonValidator.h"
#include "ccvers.h"
#include <catch2/catch_test_macros.hpp>
#include <fstream>

using namespace CalChart;

namespace {
// Helper to load schema file and create ShowSchemas map
auto LoadTestSchemas() -> ShowSchemas
{
    // Current schema is v3.8, stored as show_schema_v1.json in resources/common
    const std::string schemaPath = "resources/common/show_schema_v1.json";
    std::ifstream schemaFile(schemaPath);
    if (!schemaFile.is_open()) {
        throw std::runtime_error("Could not open schema file: " + schemaPath);
    }

    nlohmann::json schemaJson;
    schemaFile >> schemaJson;

    ShowSchemas schemas;
    schemas[1] = std::move(schemaJson);
    return schemas;
}
} // anonymous namespace

TEST_CASE("ValidateShowJson", "[JSON][Validator]")
{
    auto schemas = LoadTestSchemas();

    SECTION("Valid JSON passes validation")
    {
        std::string validJsonStr = R"({
            "formatVersion": 1,
            "labels_and_instruments": [
                {"label": "S1", "instrument": "Soprano Sax"},
                {"label": "S2"}
            ],
            "sheets": [
                {
                    "name": "Opening",
                    "tempo": 120,
                    "beats": 32,
                    "points": [
                        {
                            "pos": [80, 42],
                            "ref": [[80, 42], [80, 42], [80, 42]],
                            "symbol": "Plain",
                            "flip": false,
                            "label_invisible": false
                        }
                    ]
                }
            ],
            "mode": {
                "size": [160, 84],
                "offset": [0, 0],
                "border1": [32, 32],
                "border2": [128, 52],
                "hash_west": 0,
                "hash_east": 0,
                "yard_lines": ["0", "16", "32", "48", "64", "80", "96", "112", "128", "144", "160"]
            },
            "current_sheet": 0
        })";

        auto json = nlohmann::json::parse(validJsonStr);
        auto result = ValidateShowJson(json, schemas);

        CHECK(result.IsValid());
        CHECK_FALSE(result.HasWarnings());
    }

    SECTION("Invalid JSON with missing required field")
    {
        std::string invalidJsonStr = R"({
            "formatVersion": 1,
            "labels_and_instruments": [
                {"label": "S1", "instrument": "Soprano Sax"}
            ],
            "sheets": [
                {
                    "name": "Opening",
                    "beats": 32
                }
            ]
        })";

        auto json = nlohmann::json::parse(invalidJsonStr);
        auto result = ValidateShowJson(json, schemas);

        CHECK_FALSE(result.IsValid());
        CHECK_FALSE(result.errors.empty());
    }

    SECTION("JSON with unrecognized fields produces warnings")
    {
        std::string jsonWithExtraStr = R"({
            "formatVersion": 1,
            "labels_and_instruments": [
                {"label": "S1", "instrument": "Soprano Sax"}
            ],
            "sheets": [
                {
                    "name": "Opening",
                    "tempo": 120,
                    "beats": 32,
                    "points": [
                        {
                            "pos": [80, 42],
                            "ref": [[80, 42], [80, 42], [80, 42]],
                            "symbol": "Plain",
                            "flip": false,
                            "label_invisible": false,
                            "future_feature": "some value"
                        }
                    ]
                }
            ],
            "mode": {
                "size": [160, 84],
                "offset": [0, 0],
                "border1": [32, 32],
                "border2": [128, 52],
                "hash_west": 0,
                "hash_east": 0,
                "yard_lines": ["0", "16", "32", "48", "64", "80", "96", "112", "128", "144", "160"]
            },
            "current_sheet": 0,
            "future_top_level_field": "should trigger warning"
        })";

        auto json = nlohmann::json::parse(jsonWithExtraStr);
        auto result = ValidateShowJson(json, schemas);

        CHECK(result.IsValid());
        CHECK(result.HasWarnings());
    }

    SECTION("Malformed JSON is rejected")
    {
        std::string malformedJsonStr = R"({
            "labels_and_instruments": [
                {"label": "S1", "instrument": "Soprano Sax"
        )";

        // Malformed JSON should throw during parsing
        CHECK_THROWS_AS(nlohmann::json::parse(malformedJsonStr), nlohmann::json::parse_error);
    }
}
