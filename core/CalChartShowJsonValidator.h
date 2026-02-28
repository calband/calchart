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
/// @param schemaPath Path to the JSON schema file (typically schemas/show/v3.8/schema.json)
/// @return ValidationResult containing errors and warnings
/// @throws std::runtime_error if schema file cannot be loaded or is invalid
[[nodiscard]] auto ValidateShowJson(
    nlohmann::json const& json,
    ShowSchemas const& schemas) -> ValidationResult;

} // namespace CalChart
