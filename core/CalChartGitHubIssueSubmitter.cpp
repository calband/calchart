// CalChartGitHubIssueSubmitter.cpp
// Core functionality for GitHub API integration - markup formatting only
// (see src/GitHubIssueSubmitter.cpp for wxWidgets-dependent submission code)

#include "CalChartGitHubIssueSubmitter.hpp"
#include <format>

namespace CalChart {

auto FormatBugReportAsMarkdown(BugReport const& report) -> std::string
{
    std::string result = std::format("# {}\n\n## Description\n{}\n\n", report.title, report.description);

    if (!report.steps_to_reproduce.empty()) {
        result += std::format("## Steps to Reproduce\n{}\n\n", report.steps_to_reproduce);
    }

    if (report.include_system_info) {
        result += std::format("## System Information\n```\n{}\n```\n\n", report.diagnostic_info.toString());
    }

    if (!report.email.empty()) {
        result += std::format("## Reporter Email\n{}\n", report.email);
    }

    return result;
}

} // namespace CalChart
