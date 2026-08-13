#include "CalChartShowJsonValidator.h"
#include "ccvers.h"
#include <fstream>
#include <nlohmann/json-schema.hpp>
#include <set>
#include <sstream>

namespace CalChart {

namespace {
    // Custom error handler for schema validation
    class ValidationErrorHandler : public nlohmann::json_schema::basic_error_handler {
    public:
        void error(const nlohmann::json::json_pointer& pointer, [[maybe_unused]] const nlohmann::json& instance,
            const std::string& message) override
        {
            std::ostringstream oss;
            oss << "Validation error at " << pointer.to_string() << ": " << message;
            errors.push_back(oss.str());
        }

        std::vector<std::string> errors;
    };

    // Get the set of known properties from a schema object
    auto GetKnownProperties(nlohmann::json const& schema) -> std::set<std::string>
    {
        std::set<std::string> known;
        if (schema.contains("properties") && schema["properties"].is_object()) {
            for (auto const& [key, value] : schema["properties"].items()) {
                known.insert(key);
            }
        }
        return known;
    }

    // Resolve a $ref reference in the schema
    auto ResolveRef(nlohmann::json const& rootSchema, std::string const& ref) -> nlohmann::json const*
    {
        // Handle references like "#/$defs/coordPair"
        if (ref.empty() || ref[0] != '#') {
            return nullptr;
        }

        auto path = ref.substr(1); // Remove the '#'
        auto current = &rootSchema;

        // Split by '/' and navigate
        std::string token;
        std::istringstream tokenStream(path);
        while (std::getline(tokenStream, token, '/')) {
            if (token.empty()) {
                continue;
            }
            if (current->contains(token)) {
                current = &(*current)[token];
            } else {
                return nullptr;
            }
        }
        return current;
    }

    // Detect unrecognized fields by comparing JSON object keys to schema properties
    auto DetectUnrecognizedFields(nlohmann::json const& json, nlohmann::json const& schema) -> std::vector<std::string>;

    // Helper: recursively check for unrecognized fields in nested objects
    void CheckObjectForUnrecognizedFields(nlohmann::json const& obj, nlohmann::json const& schema,
        nlohmann::json const& rootSchema, std::string const& path, std::vector<std::string>& warnings);

    auto DetectUnrecognizedFields(nlohmann::json const& json, nlohmann::json const& schema) -> std::vector<std::string>
    {
        std::vector<std::string> warnings;

        if (json.is_object()) {
            CheckObjectForUnrecognizedFields(json, schema, schema, "$", warnings);
        }

        return warnings;
    }

    void CheckObjectForUnrecognizedFields(nlohmann::json const& obj, nlohmann::json const& schema,
        nlohmann::json const& rootSchema, std::string const& path, std::vector<std::string>& warnings)
    {
        if (!obj.is_object() || !schema.is_object()) {
            return;
        }

        // Get the schema's known properties
        auto knownProps = GetKnownProperties(schema);

        // Check each field in the JSON object
        for (auto const& [key, value] : obj.items()) {
            auto currentPath = path + "/" + key;

            if (knownProps.find(key) == knownProps.end()) {
                // Field is not in the schema's declared properties
                warnings.push_back("Unrecognized field at " + currentPath);
            } else {
                // Field is known - recursively check nested objects
                auto const& propSchema = schema["properties"][key];

                // Resolve $ref if present
                nlohmann::json const* actualSchema = &propSchema;
                if (propSchema.contains("$ref") && propSchema["$ref"].is_string()) {
                    auto ref = propSchema["$ref"].get<std::string>();
                    auto resolved = ResolveRef(rootSchema, ref);
                    if (resolved) {
                        actualSchema = resolved;
                    }
                }

                if (value.is_object()) {
                    CheckObjectForUnrecognizedFields(value, *actualSchema, rootSchema, currentPath, warnings);
                } else if (value.is_array()) {
                    // Check array items if the schema defines item schema
                    if (actualSchema->contains("items")) {
                        auto const& itemSchema = (*actualSchema)["items"];
                        nlohmann::json const* actualItemSchema = &itemSchema;

                        // Resolve $ref in items
                        if (itemSchema.contains("$ref") && itemSchema["$ref"].is_string()) {
                            auto ref = itemSchema["$ref"].get<std::string>();
                            auto resolved = ResolveRef(rootSchema, ref);
                            if (resolved) {
                                actualItemSchema = resolved;
                            }
                        }

                        for (size_t i = 0; i < value.size(); ++i) {
                            auto arrayPath = currentPath + "[" + std::to_string(i) + "]";
                            if (value[i].is_object()) {
                                CheckObjectForUnrecognizedFields(
                                    value[i], *actualItemSchema, rootSchema, arrayPath, warnings);
                            }
                        }
                    }
                }
            }
        }
    }

    auto GetFormatVersion(nlohmann::json const& json) -> int { return json.at("formatVersion").get<int>(); }

} // anonymous namespace

auto ValidateShowJson(nlohmann::json const& json, ShowSchemas const& schemas) -> ValidationResult
{
    if (schemas.empty()) {
        return {};
    }
    ValidationResult result;

    auto schema = [&]() {
        try {
            auto version = GetFormatVersion(json);
            auto it = schemas.find(version);
            if (it != schemas.end()) {
                return it;
            } else {
                result.warnings.push_back("Unknown formatVersion, using oldest known version");
                return std::prev(schemas.end());
            }
        } catch (std::exception const& e) {
            result.errors.push_back(std::string("Schema validation exception: ") + e.what());
        }
        return schemas.begin(); // Fallback to the first schema if formatVersion is missing or invalid
    }();

    try {
        // Validate against schema using nlohmann/json-schema-validator
        nlohmann::json_schema::json_validator validator;
        validator.set_root_schema((*schema).second); // Use the determined schema

        ValidationErrorHandler errorHandler;
        validator.validate(json, errorHandler);

        result.errors = std::move(errorHandler.errors);
    } catch (std::exception const& e) {
        result.errors.push_back(std::string("Schema validation exception: ") + e.what());
        return result; // Don't check for unrecognized fields if basic validation failed
    }

    // If validation passed, check for unrecognized fields
    if (result.IsValid()) {
        result.warnings = DetectUnrecognizedFields(json, (*schema).second); // Use the determined schema
    }

    return result;
}

auto ValidationResult::GetMessage() const -> std::string
{
    std::ostringstream oss;

    if (!errors.empty()) {
        oss << "Validation failed with " << errors.size() << " error(s):\n";
        for (auto const& error : errors) {
            oss << "  - " << error << "\n";
        }
    }

    if (!warnings.empty()) {
        if (!errors.empty()) {
            oss << "\n";
        }
        oss << "Validation warnings (" << warnings.size() << "):\n";
        for (auto const& warning : warnings) {
            oss << "  - " << warning << "\n";
        }
    }

    if (errors.empty() && warnings.empty()) {
        oss << "Validation passed with no errors or warnings.";
    }

    return oss.str();
}

} // namespace CalChart
