#pragma once

#include "../core/CalChartGitHubIssueSubmitter.hpp"
#include <functional>
#include <string>

namespace CalChart {

// Callback signature for issue submission result
// Parameters:
// - status: whether submission succeeded or why it failed
// - issue_url: if successful, the GitHub URL of the created issue (e.g., https://github.com/calband/calchart/issues/123)
// - error_message: if failed, a user-friendly error message explaining what went wrong
using IssueSubmissionCallback = std::function<void(IssueSubmissionStatus status, const std::string& issue_url, const std::string& error_message)>;

// Start a background submission of a bug report to the GitHub calband/calchart repository.
// This function is fire-and-forget and will call the callback on the main thread (via wxApp::CallAfter)
// when the submission completes (success or failure).
//
// The GitHub API token is read from the CALCHART_GITHUB_TOKEN environment variable if not provided.
// If no token is available, the issue is formatted as markdown and copied to the clipboard instead.
//
// Parameters:
// - report: the bug report data to submit
// - on_complete: callback to invoke on the main/UI thread when submission completes
// - token: optional GitHub PAT - if provided, uses this instead of environment variable
void StartBackgroundIssueSubmission(const BugReport& report, IssueSubmissionCallback on_complete, const std::string& token = "");

} // namespace CalChart
