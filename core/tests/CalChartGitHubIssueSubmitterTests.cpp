/*
 * CalChartGitHubIssueSubmitterTests.cpp
 * Unit tests for GitHub issue submission functionality
 */

#include "CalChartDiagnosticInfo.hpp"
#include "CalChartGitHubIssueSubmitter.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace CalChart;

TEST_CASE("FormatBugReportAsMarkdown", "[GitHub][BugReport]")
{
    // Create a minimal diagnostic info
    DiagnosticInfo diag;
    diag.calchart_version = "3.8.0";
    diag.build_type = "Debug";

    SECTION("Basic report formatting")
    {
        BugReport report;
        report.title = "Test Bug";
        report.description = "This is a test bug";
        report.email = "test@example.com";
        report.include_system_info = false;

        auto markdown = FormatBugReportAsMarkdown(report);

        CHECK(markdown.find("# Test Bug") != std::string::npos);
        CHECK(markdown.find("This is a test bug") != std::string::npos);
        CHECK(markdown.find("test@example.com") != std::string::npos);
    }

    SECTION("Report with all fields")
    {
        BugReport report;
        report.title = "Critical Bug";
        report.description = "Application crashes on startup";
        report.steps_to_reproduce = "1. Launch CalChart\n2. Click File > Open";
        report.email = "user@example.com";
        report.diagnostic_info = diag;
        report.include_system_info = true;

        auto markdown = FormatBugReportAsMarkdown(report);

        CHECK(markdown.find("# Critical Bug") != std::string::npos);
        CHECK(markdown.find("## Steps to Reproduce") != std::string::npos);
        CHECK(markdown.find("## System Information") != std::string::npos);
        CHECK(markdown.find("user@example.com") != std::string::npos);
    }

    SECTION("Report without optional fields")
    {
        BugReport report;
        report.title = "Simple Bug";
        report.description = "Something doesn't work";
        report.include_system_info = false;

        auto markdown = FormatBugReportAsMarkdown(report);

        // Should not include sections we didn't fill out
        CHECK(markdown.find("## Steps to Reproduce") == std::string::npos);
        CHECK(markdown.find("## Reporter Email") == std::string::npos);
    }

    SECTION("System information included")
    {
        BugReport report;
        report.title = "Bug with Diagnostics";
        report.description = "Test";
        report.diagnostic_info = diag;
        report.include_system_info = true;

        auto markdown = FormatBugReportAsMarkdown(report);

        CHECK(markdown.find("## System Information") != std::string::npos);
        CHECK(markdown.find("```") != std::string::npos); // Code block
        CHECK(markdown.find("3.8.0") != std::string::npos); // Version in output
    }

    SECTION("System information excluded")
    {
        BugReport report;
        report.title = "Bug without Diagnostics";
        report.description = "Test";
        report.diagnostic_info = diag;
        report.include_system_info = false;

        auto markdown = FormatBugReportAsMarkdown(report);

        // Should not include the system information section
        CHECK(markdown.find("## System Information") == std::string::npos);
    }
}

TEST_CASE("BugReport struct", "[GitHub][BugReport]")
{
    SECTION("Default initialization")
    {
        BugReport report;

        CHECK(report.title.empty());
        CHECK(report.description.empty());
        CHECK(report.steps_to_reproduce.empty());
        CHECK(report.email.empty());
        CHECK(report.include_system_info == true);
        CHECK(report.include_show == false);
    }

    SECTION("Field assignment")
    {
        BugReport report;
        report.title = "Test";
        report.description = "Description";
        report.email = "test@example.com";
        report.include_system_info = false;
        report.include_show = true;

        CHECK(report.title == "Test");
        CHECK(report.description == "Description");
        CHECK(report.email == "test@example.com");
        CHECK(report.include_system_info == false);
        CHECK(report.include_show == true);
    }
}

TEST_CASE("IssueSubmissionStatus enum", "[GitHub][Status]")
{
    SECTION("Enum values exist")
    {
        // Just verify the enum values are defined and can be used
        CHECK(IssueSubmissionStatus::Success == IssueSubmissionStatus::Success);
        CHECK(IssueSubmissionStatus::NetworkError == IssueSubmissionStatus::NetworkError);
        CHECK(IssueSubmissionStatus::ApiError == IssueSubmissionStatus::ApiError);
        CHECK(IssueSubmissionStatus::InvalidInput == IssueSubmissionStatus::InvalidInput);
        CHECK(IssueSubmissionStatus::UnknownError == IssueSubmissionStatus::UnknownError);
    }
}
