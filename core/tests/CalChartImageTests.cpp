#include "CalChartImage.h"
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

using namespace CalChart;

TEST_CASE("CalChartImageTests")
{
    SECTION("JSON round trip with data")
    {
        // Create a simple test image with some data
        ImageInfo original;
        original.left = 100;
        original.top = 200;
        original.scaledWidth = 300;
        original.scaledHeight = 400;
        original.data.width = 150;
        original.data.height = 200;

        // Create some test RGB data (simulating a small 2x2 RGB image = 12 bytes)
        original.data.data = { 255, 0, 0, 0, 255, 0, 0, 0, 255, 128, 128, 128 };

        // Create some test alpha data (2x2 = 4 bytes)
        original.data.alpha = { 255, 128, 64, 0 };

        // Serialize to JSON
        auto registry = ImageRegistry{};
        auto const json = ImageInfoToJSON(original, registry);
        auto const registryJSON = ImageRegistryToJSON(registry);

        // Verify JSON structure
        CHECK(json.contains("left"));
        CHECK(json.contains("top"));
        CHECK(json.contains("scaledWidth"));
        CHECK(json.contains("scaledHeight"));
        CHECK(json.contains("image_id"));

        CHECK(registryJSON.is_array());
        CHECK(registryJSON.size() == 1);
        CHECK(registryJSON.at(0).contains("width"));
        CHECK(registryJSON.at(0).contains("height"));
        CHECK(registryJSON.at(0).contains("data"));
        CHECK(registryJSON.at(0).contains("alpha"));

        // Verify values
        CHECK(json.at("left") == 100);
        CHECK(json.at("top") == 200);
        CHECK(json.at("scaledWidth") == 300);
        CHECK(json.at("scaledHeight") == 400);
        CHECK(json.at("image_id") == 0);
        CHECK(registryJSON.at(0).at("width") == 150);
        CHECK(registryJSON.at(0).at("height") == 200);

        // Verify base64 strings are not empty
        CHECK(registryJSON.at(0).at("data").is_string());
        CHECK(!registryJSON.at(0).at("data").get<std::string>().empty());
        CHECK(registryJSON.at(0).at("alpha").is_string());
        CHECK(!registryJSON.at(0).at("alpha").get<std::string>().empty());

        // Deserialize back from JSON
        auto const restoredRegistry = ImageRegistryFromJSON(registryJSON);
        auto const restored = ImageInfoFromJSON(json, restoredRegistry);

        // Verify all fields match
        CHECK(restored.left == original.left);
        CHECK(restored.top == original.top);
        CHECK(restored.scaledWidth == original.scaledWidth);
        CHECK(restored.scaledHeight == original.scaledHeight);
        CHECK(restored.data.width == original.data.width);
        CHECK(restored.data.height == original.data.height);
        CHECK(restored.data.data == original.data.data);
        CHECK(restored.data.alpha == original.data.alpha);

        // Verify round-trip produces identical JSON
        auto registry2 = ImageRegistry{};
        auto const json2 = ImageInfoToJSON(restored, registry2);
        auto const registryJSON2 = ImageRegistryToJSON(registry2);
        CHECK(json == json2);
        CHECK(registryJSON == registryJSON2);
    }

    SECTION("JSON round trip with empty data")
    {
        // Create an image with no actual pixel data
        ImageInfo original;
        original.left = 50;
        original.top = 75;
        original.scaledWidth = 100;
        original.scaledHeight = 150;
        original.data.width = 100;
        original.data.height = 150;
        original.data.data = {}; // Empty
        original.data.alpha = {}; // Empty

        auto registry = ImageRegistry{};
        auto const json = ImageInfoToJSON(original, registry);
        auto const registryJSON = ImageRegistryToJSON(registry);

        // Empty vectors should produce empty strings
        CHECK(registryJSON.at(0).at("data").get<std::string>().empty());
        CHECK(registryJSON.at(0).at("alpha").get<std::string>().empty());

        auto const restoredRegistry = ImageRegistryFromJSON(registryJSON);
        auto const restored = ImageInfoFromJSON(json, restoredRegistry);

        CHECK(restored.left == original.left);
        CHECK(restored.top == original.top);
        CHECK(restored.data.data.empty());
        CHECK(restored.data.alpha.empty());
    }

    SECTION("JSON round trip with data but no alpha")
    {
        ImageInfo original;
        original.left = 10;
        original.top = 20;
        original.scaledWidth = 30;
        original.scaledHeight = 40;
        original.data.width = 30;
        original.data.height = 40;
        original.data.data = { 1, 2, 3, 4, 5 };
        original.data.alpha = {}; // No alpha channel

        auto registry = ImageRegistry{};
        auto const json = ImageInfoToJSON(original, registry);
        auto const restoredRegistry = ImageRegistryFromJSON(ImageRegistryToJSON(registry));
        auto const restored = ImageInfoFromJSON(json, restoredRegistry);

        CHECK(restored.data.data == original.data.data);
        CHECK(restored.data.alpha.empty());
    }

    SECTION("JSON parsing failures")
    {
        // Not an object
        auto const registry = ImageRegistry{};
        auto const notObject = nlohmann::json::array({ 1, 2, 3 });
        CHECK_THROWS_AS(ImageInfoFromJSON(notObject, registry), std::invalid_argument);

        // Missing required field
        auto const missingLeft = nlohmann::json{ { "top", 100 }, { "scaledWidth", 200 }, { "scaledHeight", 300 },
            { "data", { { "width", 200 }, { "height", 300 }, { "data", "" }, { "alpha", "" } } } };
        CHECK_THROWS_AS(ImageInfoFromJSON(missingLeft, registry), std::invalid_argument);

        // Missing data field
        auto const missingData
            = nlohmann::json{ { "left", 10 }, { "top", 20 }, { "scaledWidth", 30 }, { "scaledHeight", 40 } };
        CHECK_THROWS_AS(ImageInfoFromJSON(missingData, registry), std::invalid_argument);

        // Data field is not an object
        auto const dataNotObject = nlohmann::json{ { "left", 10 }, { "top", 20 }, { "scaledWidth", 30 },
            { "scaledHeight", 40 }, { "data", "not an object" } };
        CHECK_THROWS_AS(ImageInfoFromJSON(dataNotObject, registry), std::invalid_argument);

        // Data object missing required fields
        auto const dataMissingWidth = nlohmann::json{ { "left", 10 }, { "top", 20 }, { "scaledWidth", 30 },
            { "scaledHeight", 40 }, { "data", { { "height", 300 }, { "data", "" }, { "alpha", "" } } } };
        CHECK_THROWS_AS(ImageInfoFromJSON(dataMissingWidth, registry), std::invalid_argument);
    }

    SECTION("Base64 encoding verification")
    {
        // Create a known pattern
        ImageInfo original;
        original.left = 0;
        original.top = 0;
        original.scaledWidth = 1;
        original.scaledHeight = 1;
        original.data.width = 1;
        original.data.height = 1;

        // Simple test pattern: ABC
        original.data.data = { 'A', 'B', 'C' };
        original.data.alpha = {};

        auto registry = ImageRegistry{};
        auto const json = ImageInfoToJSON(original, registry);
        auto const registryJSON = ImageRegistryToJSON(registry);

        // "ABC" in base64 should be "QUJD"
        CHECK(registryJSON.at(0).at("data").get<std::string>() == "QUJD");

        // Verify round-trip
        auto const restoredRegistry = ImageRegistryFromJSON(registryJSON);
        auto const restored = ImageInfoFromJSON(json, restoredRegistry);
        CHECK(restored.data.data == original.data.data);
    }

    SECTION("Render field is always nullptr after deserialization")
    {
        ImageInfo original;
        original.left = 0;
        original.top = 0;
        original.scaledWidth = 10;
        original.scaledHeight = 10;
        original.data.width = 10;
        original.data.height = 10;
        original.data.data = { 1, 2, 3 };
        original.data.alpha = {};

        auto registry = ImageRegistry{};
        auto const json = ImageInfoToJSON(original, registry);
        auto const restored = ImageInfoFromJSON(json, registry);

        // render should always be nullptr after deserialization
        CHECK(restored.data.render == nullptr);
    }
}
