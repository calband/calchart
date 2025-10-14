#pragma once

#include <functional>
#include <optional>
#include <string>

namespace CalChart {

// Parse a GitHub releases/latest JSON payload and return the tag_name if present.
auto ParseLatestReleaseTagFromJson(std::string const& json_payload) -> std::optional<std::string>;

// Compare semantic version strings (optionally prefixed with 'v').
// Returns: -1 if a<b, 0 if equal, +1 if a>b.
auto CompareSemVer(std::string const& a, std::string const& b) -> int;

// Start a background check for newer releases on GitHub. This function is fire-and-forget
// and will call `on_new_version(found_version)` on the main thread (via wx events) if a newer
// version is found. The function returns immediately and performs network IO on a detached thread.
//
// Parameters:
// - current_version: the current app version string (e.g. "v3.7.2" or "3.7.2").
// - ignored_version: a value read from configuration; if equal to latest version found, no prompt will be made.
// - on_new_version: callback to invoke on the main/UI thread when a newer version is found; signature takes the tag string.
void StartBackgroundCheck(std::string current_version, std::string ignored_version, std::function<void(std::string)> on_new_version);

}
