/*
 * HelpManager.cpp
 * Implementation of help system management with pre-built HTML
 */

/*
   Copyright (C) 1995-2025  Garrick Brian Meeker, Richard Michael Powell

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "HelpManager.hpp"

#include <algorithm>
#include <regex>
#include <sstream>
#include <wx/dir.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/log.h>

auto HelpManager::LoadHelpHtmlDirectory(const std::string& htmlPath) -> bool
{
    wxString wxHtmlPath(htmlPath);
    mHelpPath = htmlPath;
    mTopicsHtml.clear();
    mTopicMetadata.clear();
    mTableOfContents.clear();

    wxDir dir(wxHtmlPath);
    if (!dir.IsOpened()) {
        wxLogWarning("HTML help directory not found or inaccessible: %s", wxHtmlPath);
        return false;
    }

    wxString filename;
    bool found = dir.GetFirst(&filename, "*.html", wxDIR_FILES | wxDIR_DIRS);

    if (!found) {
        wxLogWarning("No HTML files found in help directory: %s", wxHtmlPath);
        return false;
    }

    // Recursively load all HTML files
    std::function<void(const wxString&)> loadDir = [&](const wxString& dirPath) {
        wxDir subDir(dirPath);
        wxString fn;
        bool found = subDir.GetFirst(&fn, "*", wxDIR_FILES | wxDIR_DIRS);

        while (found) {
            if (fn.GetChar(0) != '.') { // Skip hidden files and . / ..
                wxString fullPath = dirPath;
                fullPath.Append(wxFILE_SEP_PATH).Append(fn);

                if (wxDir::Exists(fullPath)) {
                    // Recurse into subdirectories
                    loadDir(fullPath);
                } else if (fullPath.EndsWith(".html")) {
                    LoadHtmlFile(fullPath, "");
                }
            }
            found = subDir.GetNext(&fn);
        }
    };

    loadDir(htmlPath);

    return !mTopicsHtml.empty();
}

auto HelpManager::LoadHtmlFile(const wxString& filePath, const wxString&) -> bool
{
    wxFile file(filePath);
    if (!file.IsOpened()) {
        return false;
    }

    wxString content;
    file.ReadAll(&content);

    // Generate topic ID from relative path
    wxString relativePath = filePath;
    wxString wxHelpPath(mHelpPath);
    relativePath.Replace(wxHelpPath + wxFILE_SEP_PATH, "");
    relativePath.Replace(wxFILE_SEP_PATH, "/");
    relativePath.Replace(".html", "");

    std::string topicId(relativePath.mb_str());
    mTopicsHtml[topicId] = std::string(content.mb_str());

    // Store in metadata map for TOC
    HelpTopic topic;
    topic.id = topicId;
    topic.title = topicId;
    topic.path = std::string(filePath.mb_str());
    mTopicMetadata[topicId] = topic;

    return true;
}

auto HelpManager::GetHelpTopicAsHtml(const std::string& topicId) const -> std::string
{
    auto it = mTopicsHtml.find(topicId);
    if (it != mTopicsHtml.end()) {
        wxLogInfo("Found help topic '%s', content length: %zu", topicId, it->second.length());
        return it->second;
    }

    // Topic not found - log available topics for debugging
    wxLogWarning("Help topic not found: '%s'", topicId);
    if (!mTopicsHtml.empty()) {
        wxString available;
        int count = 0;
        for (const auto& [id, _] : mTopicsHtml) {
            if (count++ < 5) { // Show first 5 topics
                available += wxString::Format("'%s' ", id);
            }
        }
        wxLogInfo("Available topics (%zu total): %s...", mTopicsHtml.size(), available);
    }
    return std::string();
}

auto HelpManager::GetHelpTopicPath(const std::string& topicId) const -> std::string
{
    auto it = mTopicMetadata.find(topicId);
    if (it != mTopicMetadata.end()) {
        return it->second.path;
    }
    return std::string();
}

auto HelpManager::HasTopic(const std::string& topicId) const -> bool
{
    return mTopicsHtml.find(topicId) != mTopicsHtml.end();
}

auto HelpManager::GetTopicMetadata(const std::string& topicId) const -> HelpTopic
{
    auto it = mTopicMetadata.find(topicId);
    if (it == mTopicMetadata.end()) {
        return HelpTopic{};
    }
    return it->second;
}

auto HelpManager::GetRelatedTopics(const std::string& topicId) const -> std::vector<HelpTopic>
{
    std::vector<HelpTopic> related;
    auto metaIt = mTopicMetadata.find(topicId);
    if (metaIt == mTopicMetadata.end()) {
        return related;
    }

    for (const auto& relatedId : metaIt->second.relatedTopics) {
        auto it = mTopicMetadata.find(relatedId);
        if (it != mTopicMetadata.end()) {
            related.push_back(it->second);
        }
    }

    return related;
}

auto HelpManager::GetTableOfContents() const -> std::vector<HelpTopic>
{
    std::vector<HelpTopic> toc;

    // Parse the index.html to extract TOC
    auto indexIt = mTopicsHtml.find("index");
    if (indexIt == mTopicsHtml.end()) {
        return toc;
    }

    // Simple extraction: look for markdown links in the index
    const std::string& indexContent = indexIt->second;
    std::regex linkRegex("\\[([^\\]]+)\\]\\(([^\\)]+)\\)");
    std::smatch match;
    std::string::const_iterator searchStart(indexContent.cbegin());

    while (std::regex_search(searchStart, indexContent.cend(), match, linkRegex)) {
        std::string linkTarget = match[2].str();

        // Remove .md extension
        if (linkTarget.find(".md") != std::string::npos) {
            linkTarget.erase(linkTarget.find(".md"));
        }

        // Handle relative paths (e.g., "../file.md" -> "file", "./local.md" -> "local")
        size_t lastSlash = linkTarget.find_last_of("/");
        if (lastSlash != std::string::npos) {
            linkTarget = linkTarget.substr(lastSlash + 1);
        }

        auto metaIt = mTopicMetadata.find(linkTarget);
        if (metaIt != mTopicMetadata.end()) {
            toc.push_back(metaIt->second);
        }

        searchStart = match.suffix().first;
    }

    return toc;
}

auto HelpManager::SearchHelp(const std::string& queryStr) const -> std::vector<HelpSearchResult>
{
    std::vector<HelpSearchResult> results;
    std::string query(queryStr);

    // Convert to lowercase for case-insensitive search
    std::transform(query.begin(), query.end(), query.begin(), ::tolower);

    // Split query into individual words
    std::istringstream iss(query);
    std::vector<std::string> queryWords;
    std::string word;
    while (iss >> word) {
        queryWords.push_back(word);
    }

    // Search through all topics
    for (const auto& [topicId, html] : mTopicsHtml) {
        int relevanceScore = 0;

        // Get metadata
        auto metaIt = mTopicMetadata.find(topicId);
        if (metaIt == mTopicMetadata.end()) {
            continue;
        }

        const HelpTopic& topic = metaIt->second;
        std::string topicText(html);
        std::transform(topicText.begin(), topicText.end(), topicText.begin(), ::tolower);
        std::string titleLower(topic.title);
        std::transform(titleLower.begin(), titleLower.end(), titleLower.begin(), ::tolower);

        // Check each query word
        for (const auto& w : queryWords) {
            // Match in title (higher weight)
            if (titleLower.find(w) != std::string::npos) {
                relevanceScore += 10;
            }

            // Match in keywords
            for (const auto& kw : topic.keywords) {
                std::string kwLower(kw);
                std::transform(kwLower.begin(), kwLower.end(), kwLower.begin(), ::tolower);
                if (kwLower.find(w) != std::string::npos) {
                    relevanceScore += 5;
                }
            }

            // Match in content
            if (topicText.find(w) != std::string::npos) {
                relevanceScore += 1;
            }
        }

        if (relevanceScore > 0) {
            // Extract excerpt (first 100 chars of HTML content)
            std::string excerpt = html.substr(0, 100);
            size_t nlPos = excerpt.find('\n');
            if (nlPos != std::string::npos) {
                excerpt = excerpt.substr(0, nlPos);
            }
            if (excerpt.length() > 100) {
                excerpt = excerpt.substr(0, 97) + "...";
            }

            results.push_back({ topicId, topic.title, excerpt,
                relevanceScore });
        }
    }

    // Sort by relevance score (highest first)
    std::sort(results.begin(), results.end(),
        [](const HelpSearchResult& a, const HelpSearchResult& b) {
            return a.relevanceScore > b.relevanceScore;
        });

    return results;
}

auto HelpManager::GetIndexAsHtml() const -> std::string
{
    return GetHelpTopicAsHtml("index");
}
