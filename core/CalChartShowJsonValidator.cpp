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

    // Forward declaration for mutual recursion
    auto ResolveRef(nlohmann::json const& rootSchema, std::string const& ref) -> nlohmann::json const*;

    // Get the set of known properties from a schema object
    auto GetKnownProperties(nlohmann::json const& schema, nlohmann::json const& rootSchema) -> std::set<std::string>
    {
        std::set<std::string> known;
        if (schema.contains("properties") && schema["properties"].is_object()) {
            for (auto const& [key, value] : schema["properties"].items()) {
                known.insert(key);
            }
        }
        // Handle anyOf: gather properties from all alternatives
        if (schema.contains("anyOf") && schema["anyOf"].is_array()) {
            for (auto const& alternative : schema["anyOf"]) {
                // Resolve $ref if present in the alternative
                nlohmann::json const* actualAlternative = &alternative;
                if (alternative.contains("$ref") && alternative["$ref"].is_string()) {
                    auto ref = alternative["$ref"].get<std::string>();
                    auto resolved = ResolveRef(rootSchema, ref);
                    if (resolved) {
                        actualAlternative = resolved;
                    }
                }

                if (actualAlternative->contains("properties") && (*actualAlternative)["properties"].is_object()) {
                    for (auto const& [key, value] : (*actualAlternative)["properties"].items()) {
                        known.insert(key);
                    }
                }
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
        std::istringstream pathStream(path);
        std::string token;
        while (std::getline(pathStream, token, '/')) {
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
        auto knownProps = GetKnownProperties(schema, rootSchema);

        // Check if schema uses additionalProperties (for dynamic property names)
        bool hasAdditionalProperties
            = schema.contains("additionalProperties") && schema["additionalProperties"].is_object();
        nlohmann::json const* additionalPropsSchema
            = hasAdditionalProperties ? &schema["additionalProperties"] : nullptr;

        // Check each field in the JSON object
        for (auto const& [key, value] : obj.items()) {
            auto currentPath = path + "/" + key;
            nlohmann::json const* propSchema = nullptr;

            if (knownProps.find(key) != knownProps.end()) {
                // Field is explicitly declared in properties
                if (schema.contains("properties") && schema["properties"].contains(key)) {
                    propSchema = &schema["properties"][key];
                } else if (schema.contains("anyOf") && schema["anyOf"].is_array()) {
                    // Search for the property in anyOf alternatives (with $ref resolution)
                    for (auto const& alternative : schema["anyOf"]) {
                        nlohmann::json const* actualAlternative = &alternative;
                        // Resolve $ref if present in the alternative
                        if (alternative.contains("$ref") && alternative["$ref"].is_string()) {
                            auto ref = alternative["$ref"].get<std::string>();
                            auto resolved = ResolveRef(rootSchema, ref);
                            if (resolved) {
                                actualAlternative = resolved;
                            }
                        }

                        if (actualAlternative->contains("properties")
                            && (*actualAlternative)["properties"].contains(key)) {
                            propSchema = &(*actualAlternative)["properties"][key];
                            break;
                        }
                    }
                }
            } else if (hasAdditionalProperties) {
                // Field is allowed via additionalProperties (dynamic property names like symbol-keyed continuities)
                propSchema = additionalPropsSchema;
            } else {
                // Field is not in the schema's declared properties and no additionalProperties
                warnings.push_back("Unrecognized field at " + currentPath);
                continue;
            }

            // If we couldn't find a schema for this property, skip recursion
            if (!propSchema) {
                continue;
            }

            // Resolve $ref if present
            nlohmann::json const* actualSchema = propSchema;
            if (propSchema->contains("$ref") && (*propSchema)["$ref"].is_string()) {
                auto ref = (*propSchema)["$ref"].get<std::string>();
                auto resolved = ResolveRef(rootSchema, ref);
                if (resolved) {
                    actualSchema = resolved;
                }
            }

            // If the schema has anyOf at the top level, pick the matching alternative
            if (actualSchema->contains("anyOf") && (*actualSchema)["anyOf"].is_array() && value.is_object()) {
                // Try to find the alternative that matches the object's "type" field
                if (value.contains("type") && value["type"].is_string()) {
                    auto typeValue = value["type"].get<std::string>();
                    for (auto const& alternative : (*actualSchema)["anyOf"]) {
                        if (alternative.contains("properties") && alternative["properties"].contains("type")
                            && alternative["properties"]["type"].contains("const")
                            && alternative["properties"]["type"]["const"].is_string()
                            && alternative["properties"]["type"]["const"].get<std::string>() == typeValue) {
                            // Found matching alternative, resolve any $ref in it
                            actualSchema = &alternative;
                            if (alternative.contains("$ref") && alternative["$ref"].is_string()) {
                                auto ref = alternative["$ref"].get<std::string>();
                                auto resolved = ResolveRef(rootSchema, ref);
                                if (resolved) {
                                    actualSchema = resolved;
                                }
                            }
                            break;
                        }
                    }
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

    auto GetFormatVersion(nlohmann::json const& json) -> int { return json.at("formatVersion").get<int>(); }

} // anonymous namespace

auto ValidateShowJson(nlohmann::json const& json, nlohmann::json const& schema) -> ValidationResult
{
    ValidationResult result;

    try {
        // Validate against schema using nlohmann/json-schema-validator
        nlohmann::json_schema::json_validator validator;
        validator.set_root_schema(schema); // Use the determined schema

        ValidationErrorHandler errorHandler;
        validator.validate(json, errorHandler);

        result.errors = std::move(errorHandler.errors);
    } catch (std::exception const& e) {
        result.errors.push_back(std::string("Schema validation exception: ") + e.what());
        return result; // Don't check for unrecognized fields if basic validation failed
    }

    // If validation passed, check for unrecognized fields
    if (result.IsValid()) {
        result.warnings = DetectUnrecognizedFields(json, schema); // Use the determined schema
    }

    return result;
}

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

    return ValidateShowJson(json, (*schema).second);
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
