#include "HelpManager.hpp"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <wx/filename.h>
#include <wx/log.h>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace {

class HelpDirectory {
public:
    HelpDirectory()
    {
        auto temporaryFile = wxFileName::CreateTempFileName("calchart-help-test");
        if (temporaryFile.empty() || !wxRemoveFile(temporaryFile) || !wxMkdir(temporaryFile)) {
            throw std::runtime_error("Could not create temporary help directory");
        }
        path = temporaryFile.ToStdString();
    }

    ~HelpDirectory()
    {
        std::error_code error;
        std::filesystem::permissions(path / "unreadable.html", std::filesystem::perms::owner_all, error);
        std::filesystem::remove(path / "unreadable.html", error);
        std::filesystem::remove(path / "index.html", error);
        std::filesystem::remove(path, error);
    }

    void Write(const char* name, const char* content) const
    {
        auto file = std::ofstream(path / name);
        file << content;
        if (!file) {
            throw std::runtime_error("Could not write temporary help file");
        }
    }

    std::filesystem::path path;
};

}

TEST_CASE("HelpManager loads HTML files", "[help]")
{
    auto directory = HelpDirectory{};
    directory.Write("index.html", "<html>Help index</html>");
    auto manager = HelpManager{};
    wxLogNull suppressTestLogging;

    REQUIRE(manager.LoadHelpHtmlDirectory(directory.path.string()));
    CHECK(manager.HasTopic("index"));
    CHECK(manager.GetIndexAsHtml() == "<html>Help index</html>");
    CHECK(manager.GetHelpTopicPath("index") == (directory.path / "index.html").string());
}

TEST_CASE("HelpManager reports a missing help directory", "[help]")
{
    auto directory = HelpDirectory{};
    auto manager = HelpManager{};
    wxLogNull suppressExpectedWarning;

    CHECK_FALSE(manager.LoadHelpHtmlDirectory((directory.path / "missing").string()));
    CHECK_FALSE(manager.HasTopic("index"));
}

#ifndef _WIN32
TEST_CASE("HelpManager throws on an unreadable HTML file without retaining partial content", "[help]")
{
    auto directory = HelpDirectory{};
    directory.Write("index.html", "<html>Help index</html>");
    auto manager = HelpManager{};
    REQUIRE(manager.LoadHelpHtmlDirectory(directory.path.string()));

    directory.Write("unreadable.html", "unreadable");
    auto const unreadable = directory.path / "unreadable.html";
    std::filesystem::permissions(unreadable, std::filesystem::perms::none);
    if (access(unreadable.c_str(), R_OK) == 0) {
        SKIP("The current user can read files without read permissions");
    }

    try {
        (void)manager.LoadHelpHtmlDirectory(directory.path.string());
        FAIL("Expected an error for an unreadable help file");
    } catch (HelpFileError const& error) {
        CHECK(std::string(error.what()).find(unreadable.string()) != std::string::npos);
    }
    CHECK_FALSE(manager.HasTopic("index"));
    CHECK_FALSE(manager.HasTopic("unreadable"));
    std::filesystem::permissions(unreadable, std::filesystem::perms::owner_read);
    REQUIRE(manager.LoadHelpHtmlDirectory(directory.path.string()));
    CHECK(manager.HasTopic("index"));
    CHECK(manager.HasTopic("unreadable"));
}
#endif
