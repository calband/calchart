#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace CalChart {

using ShowSchemas = std::map<uint32_t, nlohmann::json>;

/// Result of JSON schema validation
struct ValidationResult {
    /// Schema validation errors - if non-empty, the JSON is invalid
    std::vector<std::string> errors;

    /// Warnings about unrecognized fields (forward compatibility concern)
    /// These are fields present in the JSON but not declared in the v3.8 schema
    std::vector<std::string> warnings;

    /// Returns true if validation passed (no errors)
    [[nodiscard]] auto IsValid() const -> bool { return errors.empty(); }

    /// Returns true if there are any warnings
    [[nodiscard]] auto HasWarnings() const -> bool { return !warnings.empty(); }

    /// Returns a combined error and warning message for display
    [[nodiscard]] auto GetMessage() const -> std::string;
};

/// Validate a JSON document against a CalChart show schema
/// @param json The JSON document to validate
/// @param schemas JSON schema (typically loaded from resources/common/show_schema_v1.json)
/// @return ValidationResult containing errors and warnings
[[nodiscard]] auto ValidateShowJson(nlohmann::json const& json, nlohmann::json const& schemas) -> ValidationResult;

/// Validate a JSON document against a CalChart show schema
/// @param json The JSON document to validate
/// @param schemas Map of formatVersion to JSON schema, will pick the "right one"
/// @return ValidationResult containing errors and warnings
[[nodiscard]] auto ValidateShowJson(nlohmann::json const& json, ShowSchemas const& schemas) -> ValidationResult;

} // namespace CalChart
