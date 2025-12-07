// GitHubIssueSubmitter.cpp
// UI-level GitHub API integration with wxWidgets support
// - StartBackgroundIssueSubmission: fire-and-forget thread-based issue submission
// - SubmitIssueToGitHub: handles libcurl + GitHub API communication
// - Clipboard fallback: when no GitHub token is available

#include "GitHubIssueSubmitter.hpp"
#include <algorithm>
#include <curl/curl.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <thread>
#include <wx/app.h>
#include <wx/clipbrd.h>

namespace {

constexpr auto kGitHubAPI = "https://api.github.com/repos/calband/calchart/issues";

struct IssueSubmissionResult {
    CalChart::IssueSubmissionStatus status;
    std::string issue_url;
    std::string error_message;
};

// libcurl write callback
auto CurlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) -> size_t
{
    auto total = size * nmemb;
    auto* out = static_cast<std::string*>(userp);
    out->append(static_cast<char*>(contents), total);
    return total;
}

// Submit issue to GitHub API using libcurl
// Returns result struct with status and optional issue URL
auto SubmitIssueToGitHub(const CalChart::BugReport& report, const std::string& token) -> IssueSubmissionResult
{
    auto* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[IssueSubmitter] curl_easy_init failed\n";
        return { CalChart::IssueSubmissionStatus::UnknownError, "", "Failed to initialize network library" };
    }

    // Build JSON body for the GitHub API
    try {
        nlohmann::json body;
        body["title"] = report.title;

        // Build description with optional diagnostic info
        auto description = report.description;
        if (!report.steps_to_reproduce.empty()) {
            description += "\n\n### Steps to Reproduce\n" + report.steps_to_reproduce;
        }

        if (report.include_system_info) {
            description += "\n\n<details>\n<summary>System Information</summary>\n\n```\n";
            description += report.diagnostic_info.toString();
            description += "\n```\n</details>";
        }

        body["body"] = description;
        body["labels"] = nlohmann::json::array({ "bug", "user-reported" });

        auto body_str = body.dump();

        // Prepare request
        std::string response;
        auto auth_header = "Authorization: token " + token;

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Accept: application/vnd.github.v3+json");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, auth_header.c_str());

        // Debug logging
        std::cerr << "[IssueSubmitter] ===== Request Details =====\n";
        std::cerr << "[IssueSubmitter] URL: " << kGitHubAPI << "\n";
        std::cerr << "[IssueSubmitter] Token (first 20 chars): " << token.substr(0, 20) << "...\n";
        std::cerr << "[IssueSubmitter] Headers: Accept, Content-Type, Authorization\n";
        std::cerr << "[IssueSubmitter] Body size: " << body_str.length() << " bytes\n";
        std::cerr << "[IssueSubmitter] Body preview: " << body_str.substr(0, 200) << "...\n";

        curl_easy_setopt(curl, CURLOPT_URL, kGitHubAPI);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body_str.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body_str.length());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "CalChart/1.0");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

        // Perform request
        CURLcode res = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        // Debug logging
        std::cerr << "[IssueSubmitter] ===== Response Details =====\n";
        std::cerr << "[IssueSubmitter] HTTP Status Code: " << http_code << "\n";
        std::cerr << "[IssueSubmitter] curl result: " << res << " (" << curl_easy_strerror(res) << ")\n";
        std::cerr << "[IssueSubmitter] Response size: " << response.length() << " bytes\n";
        std::cerr << "[IssueSubmitter] Response body: " << response << "\n";
        std::cerr << "[IssueSubmitter] ===== End Response =====\n";

        IssueSubmissionResult result;

        if (res != CURLE_OK) {
            std::cerr << "[IssueSubmitter] curl_easy_perform failed: " << curl_easy_strerror(res) << "\n";
            result.status = CalChart::IssueSubmissionStatus::NetworkError;
            result.error_message = "Network error: " + std::string(curl_easy_strerror(res));
        } else if (http_code == 401) {
            std::cerr << "[IssueSubmitter] Authentication failed (401)\n";
            result.status = CalChart::IssueSubmissionStatus::ApiError;
            result.error_message = "Authentication failed. Please check your GitHub token.";
        } else if (http_code == 403) {
            std::cerr << "[IssueSubmitter] API rate limited or forbidden (403)\n";
            std::cerr << "[IssueSubmitter] Response: " << response << "\n";
            result.status = CalChart::IssueSubmissionStatus::ApiError;
            result.error_message = "GitHub API error (403): " + response;
        } else if (http_code >= 400 && http_code < 500) {
            std::cerr << "[IssueSubmitter] Client error " << http_code << "\n";
            result.status = CalChart::IssueSubmissionStatus::InvalidInput;
            result.error_message = "Invalid request: " + response;
        } else if (http_code >= 500) {
            std::cerr << "[IssueSubmitter] Server error " << http_code << "\n";
            result.status = CalChart::IssueSubmissionStatus::ApiError;
            result.error_message = "GitHub server error. Please try again later.";
        } else if (http_code >= 200 && http_code < 300) {
            // Success! Parse the response to get the issue URL
            try {
                auto resp_json = nlohmann::json::parse(response);
                if (resp_json.contains("html_url") && resp_json["html_url"].is_string()) {
                    result.status = CalChart::IssueSubmissionStatus::Success;
                    result.issue_url = resp_json["html_url"].get<std::string>();
                    std::cerr << "[IssueSubmitter] Issue created: " << result.issue_url << "\n";
                } else {
                    result.status = CalChart::IssueSubmissionStatus::UnknownError;
                    result.error_message = "Issue created but couldn't parse response.";
                }
            } catch (const std::exception& e) {
                std::cerr << "[IssueSubmitter] JSON parse error: " << e.what() << "\n";
                result.status = CalChart::IssueSubmissionStatus::UnknownError;
                result.error_message = "Failed to parse GitHub response.";
            }
        } else {
            std::cerr << "[IssueSubmitter] Unexpected HTTP status " << http_code << "\n";
            result.status = CalChart::IssueSubmissionStatus::UnknownError;
            result.error_message = "Unexpected response from GitHub API.";
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return result;

    } catch (const std::exception& e) {
        std::cerr << "[IssueSubmitter] Exception during submission: " << e.what() << "\n";
        curl_easy_cleanup(curl);
        return { CalChart::IssueSubmissionStatus::UnknownError, "", std::string("Error: ") + e.what() };
    } catch (...) {
        std::cerr << "[IssueSubmitter] Unknown exception during submission\n";
        curl_easy_cleanup(curl);
        return { CalChart::IssueSubmissionStatus::UnknownError, "", "Unknown error occurred" };
    }
}

} // namespace

namespace CalChart {

void StartBackgroundIssueSubmission(const BugReport& report, IssueSubmissionCallback on_complete, const std::string& token)
{
    std::thread([report, on_complete, token]() mutable {
        // Use provided token, or try to get GitHub token from environment
        std::string token_to_use = token;
        if (token_to_use.empty()) {
            const char* token_env = std::getenv("CALCHART_GITHUB_TOKEN");
            token_to_use = token_env ? token_env : "";
        }

        IssueSubmissionResult result;

        if (token_to_use.empty()) {
            // No token available - copy to clipboard instead
            std::cerr << "[IssueSubmitter] No CALCHART_GITHUB_TOKEN set, will copy to clipboard\n";

            std::string markdown = FormatBugReportAsMarkdown(report);

            // Post to UI thread to copy to clipboard
            if (wxTheApp) {
                wxTheApp->CallAfter([markdown, on_complete]() {
                    if (wxTheApp && wxTheClipboard && wxTheClipboard->Open()) {
                        wxTheClipboard->SetData(new wxTextDataObject(markdown));
                        wxTheClipboard->Close();
                        on_complete(IssueSubmissionStatus::Success, "",
                            "Formatted issue copied to clipboard. Please paste it in the GitHub issue tracker.");
                    } else {
                        on_complete(IssueSubmissionStatus::NetworkError, "",
                            "Could not access clipboard. Please manually file the issue on GitHub.");
                    }
                });
            }
            return;
        }

        // Attempt to submit to GitHub
        result = SubmitIssueToGitHub(report, token_to_use);

        // Post result back to UI thread
        if (wxTheApp) {
            wxTheApp->CallAfter([result, on_complete]() {
                on_complete(result.status, result.issue_url, result.error_message);
            });
        }
    }).detach();
}

} // namespace CalChart
