// UpdateChecker.cpp
// Implements helpers to check GitHub releases/latest and notify the UI when a newer
// release is available. This file provides:
//  - ParseLatestReleaseTagFromJson
//  - CompareSemVer
//  - StartBackgroundCheck (fire-and-forget thread, invokes callback on main thread)

#include "UpdateChecker.h"
#include <algorithm>
#include <curl/curl.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace {

constexpr auto kAPI = "https://api.github.com/repos/calband/calchart/releases/latest";

// Helper: strip optional leading 'v' or 'V'
auto StripPrefixV(std::string s) -> std::string
{
    if (!s.empty() && (s[0] == 'v' || s[0] == 'V')) {
        return s.substr(1);
    }
    return s;
}

// Helper: split dot-separated numeric parts into ints
auto SplitDots(std::string const& s) -> std::vector<int>
{
    std::vector<int> parts;
    std::istringstream iss(s);
    std::string token;
    while (std::getline(iss, token, '.')) {
        try {
            parts.push_back(std::stoi(token));
        } catch (...) {
            parts.push_back(0);
        }
    }
    return parts;
}

// libcurl write callback
auto CurlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) -> size_t
{
    auto total = size * nmemb;
    auto* out = static_cast<std::string*>(userp);
    out->append(static_cast<char*>(contents), total);
    return total;
}

// Fetch a URL using libcurl and return the response body as a std::string.
// Returns empty string on failure. Sets a User-Agent so GitHub accepts the request.
auto FetchUrlToString(const std::string& url) -> std::string
{
    auto* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[UpdateChecker] curl_easy_init failed\n";
        return {};
    }
    std::string response;
    struct curl_slist* headers = nullptr;
    // GitHub API recommends setting an Accept header for v3
    headers = curl_slist_append(headers, "Accept: application/vnd.github.v3+json");
    // Set a reasonable User-Agent
    const char* ua = "CalChartUpdateChecker/1.0";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, ua);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    // follow redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    // small timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (res != CURLE_OK) {
        std::cerr << "[UpdateChecker] curl_easy_perform failed: " << curl_easy_strerror(res) << "\n";
        response.clear();
    } else if (http_code < 200 || http_code >= 300) {
        std::cerr << "[UpdateChecker] HTTP status " << http_code << " for " << url << "\n";
        response.clear();
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

} // namespace

namespace CalChart {

auto ParseLatestReleaseTagFromJson(std::string const& json_payload) -> std::optional<std::string>
{
    try {
        auto j = nlohmann::json::parse(json_payload);
        if (j.contains("tag_name") && j["tag_name"].is_string()) {
            return j["tag_name"].get<std::string>();
        }
    } catch (const std::exception& e) {
        std::cerr << "[UpdateChecker] JSON parse error: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "[UpdateChecker] JSON parse unknown error\n";
    }
    return std::nullopt;
}

auto CompareSemVer(std::string const& a, std::string const& b) -> int
{
    auto sa = StripPrefixV(a);
    auto sb = StripPrefixV(b);
    auto pa = SplitDots(sa);
    auto pb = SplitDots(sb);
    using std::max;
    auto n = max(pa.size(), pb.size());
    pa.resize(n, 0);
    pb.resize(n, 0);
    for (size_t i = 0; i < n; ++i) {
        if (pa[i] < pb[i]) {
            return -1;
        }
        if (pa[i] > pb[i]) {
            return 1;
        }
    }
    return 0;
}

void StartBackgroundCheck(std::string current_version, std::string ignored_version, std::function<void(std::string)> on_new_version)
{
    std::thread([current_version = std::move(current_version), ignored_version = std::move(ignored_version), on_new_version = std::move(on_new_version)]() mutable {
        auto payload = FetchUrlToString(kAPI);
        if (payload.empty()) {
            std::cerr << "[UpdateChecker] Fetch returned empty payload\n";
            return;
        }

        auto tag = ParseLatestReleaseTagFromJson(payload);
        if (!tag) {
            std::cerr << "[UpdateChecker] No tag_name found in payload\n";
            return;
        }

        if (!ignored_version.empty() && *tag == ignored_version) {
            std::cerr << "[UpdateChecker] Tag matches ignored version; skipping\n";
            return;
        }

        if (auto cmp = CompareSemVer(current_version, *tag);
            cmp < 0) {
            on_new_version(*tag);
        }
    }).detach();
}

} // namespace CalChart
