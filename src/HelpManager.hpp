/*
 * HelpManager.hpp
 * Manages loading pre-built HTML help documentation
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

#pragma once

#include <map>
#include <string>
#include <vector>
#include <wx/string.h>

// Structure representing a help topic
struct HelpTopic {
    std::string id; // Unique identifier (filename without .md)
    std::string title; // Human-readable title
    std::string path; // File path relative to docs/md
    std::vector<std::string> keywords; // Search keywords from frontmatter
    std::vector<std::string> relatedTopics; // Related topic IDs
};

// Structure for search results
struct HelpSearchResult {
    std::string topicId;
    std::string title;
    std::string excerpt; // Short excerpt showing match context
    int relevanceScore; // Higher = better match
};

/**
 * HelpManager
 *
 * Manages loading pre-built HTML help files and providing search/navigation functionality.
 * HTML files should be pre-generated from Markdown at build time using the conversion script.
 *
 * Usage:
 *   HelpManager mgr;
 *   if (mgr.LoadHelpHtmlDirectory("/path/to/docs/html")) {
 *       std::string html = mgr.GetHelpTopicAsHtml("basics");
 *       auto results = mgr.SearchHelp("movement");
 *   }
 */
class HelpManager {
public:
    /**
     * Load pre-built HTML help content from a directory
     * @param htmlPath Path to docs/html directory (pre-converted HTML files)
     * @return true if HTML files were successfully loaded
     */
    [[nodiscard]] auto LoadHelpHtmlDirectory(const std::string& htmlPath) -> bool;

    /**
     * Get a help topic as HTML
     * @param topicId Topic identifier (filename without .md or path)
     * @return HTML content, or empty string if not found
     */
    [[nodiscard]] auto GetHelpTopicAsHtml(const std::string& topicId) const -> std::string;

    /**
     * Get the file path for a help topic
     * @param topicId Topic identifier
     * @return Full file path to the HTML file, or empty if not found
     */
    [[nodiscard]] auto GetHelpTopicPath(const std::string& topicId) const -> std::string;

    /**
     * Search help content
     * @param query Search query (words separated by spaces)
     * @return Vector of matching topics, sorted by relevance
     */
    [[nodiscard]] auto SearchHelp(const std::string& query) const -> std::vector<HelpSearchResult>;

    /**
     * Get table of contents
     * @return Vector of topics from index.md, in order
     */
    [[nodiscard]] auto GetTableOfContents() const -> std::vector<HelpTopic>;

    /**
     * Get a specific help topic's metadata
     * @param topicId Topic identifier
     * @return HelpTopic struct, or default if not found
     */
    [[nodiscard]] auto GetTopicMetadata(const std::string& topicId) const -> HelpTopic;

    /**
     * Get related topics for a given topic
     * @param topicId Topic identifier
     * @return Vector of related HelpTopic structs
     */
    [[nodiscard]] auto GetRelatedTopics(const std::string& topicId) const -> std::vector<HelpTopic>;

    /**
     * Check if a help topic exists
     * @param topicId Topic identifier
     * @return true if topic is available
     */
    [[nodiscard]] auto HasTopic(const std::string& topicId) const -> bool;

    /**
     * Get the main index topic (usually "index")
     * @return HTML content of the help index
     */
    [[nodiscard]] auto GetIndexAsHtml() const -> std::string;

    /**
     * Get the help path (base directory for HTML files)
     * @return Path to the help HTML directory
     */
    [[nodiscard]] auto GetHelpPath() const -> std::string { return mHelpPath; }

private:
    /**
     * Load a single HTML file
     * @param filePath Full path to .html file
     * @param id Topic ID (derived from filename)
     * @return true if loaded successfully
     */
    [[nodiscard]] auto LoadHtmlFile(const wxString& filePath, const wxString& id) -> bool;

    // Member variables
    std::map<std::string, std::string> mTopicsHtml; // HTML content
    std::map<std::string, HelpTopic> mTopicMetadata; // Metadata for each topic
    std::vector<HelpTopic> mTableOfContents; // Index order

    std::string mHelpPath; // Root path to help directory
};
