#pragma once

#include "CalChartDiagnosticInfo.hpp"
#include <string>

namespace CalChart {

// Structure to hold user bug report data
struct BugReport {
    std::string title{};
    std::string description{};
    std::string steps_to_reproduce{};
    std::string email{};
    bool include_system_info = true;
    bool include_show = false;
    DiagnosticInfo diagnostic_info{};
};

// Result of issue submission attempt
enum class IssueSubmissionStatus {
    Success, // Issue created successfully on GitHub
    NetworkError, // Network/connectivity issue
    ApiError, // GitHub API returned error (rate limit, auth, etc)
    InvalidInput, // User input validation failed
    UnknownError // Some other unexpected error
};

// Format a bug report as markdown for display/copying to clipboard
auto FormatBugReportAsMarkdown(BugReport const& report) -> std::string;

} // namespace CalChart
