# CalChart Help System Architecture

## Overview

CalChart features a modern, maintainable help system that replaces the legacy wxHtmlHelpController with a Markdown-based approach. This document describes the architecture, design, implementation, and migration strategy.

**Status:** Phase 1 Foundation complete; Phase 2 content migration in progress.

## Goals & Benefits

1. **Easier Maintenance** — Plain Markdown is human-readable and version-control friendly
2. **Accuracy** — Consolidated, updated documentation (previous system was 10+ years outdated)
3. **wxWidgets Integration** — Displays via wxWebView with modern HTML/CSS rendering
4. **Extensibility** — Supports search, related topics, context-sensitive help, and future enhancements
5. **Cross-Platform** — Works seamlessly on macOS, Windows, and Linux

## Architecture

### Core Components

#### 1. HelpManager (`src/HelpManager.hpp` / `src/HelpManager.cpp`)

Loads and manages all help content from pre-built HTML files.

**Primary Mode:** Pre-built HTML files (generated at build time)
- Loads from `docs/html/` directory
- Highest performance (no runtime conversion)
- Required for distribution builds

**Key Responsibilities:**
- Load HTML content from disk
- Parse metadata from HTML files
- Build and cache search index
- Provide full-text search with relevance ranking
- Navigate topics and manage table of contents

**Key Public Methods:**
```cpp
[[nodiscard]] auto LoadHelpHtmlDirectory(const std::string& htmlPath) -> bool;
[[nodiscard]] auto GetHelpTopicAsHtml(const std::string& topicId) const -> std::string;
[[nodiscard]] auto SearchHelp(const std::string& query) const -> std::vector<HelpSearchResult>;
[[nodiscard]] auto GetTableOfContents() const -> std::vector<HelpTopic>;
[[nodiscard]] auto GetTopicMetadata(const std::string& topicId) const -> HelpTopic;
[[nodiscard]] auto GetRelatedTopics(const std::string& topicId) const -> std::vector<HelpTopic>;
[[nodiscard]] auto HasTopic(const std::string& topicId) const -> bool;
[[nodiscard]] auto GetIndexAsHtml() const -> std::string;
```

**Member Variables:**
```cpp
std::map<std::string, std::string> mTopicsHtml;          // Pre-built HTML content
std::map<std::string, std::string> mTopicsMarkdown;      // Markdown content (fallback)
std::map<std::string, HelpTopic> mTopicMetadata;         // Metadata for all topics
std::vector<HelpTopic>                                    // Root directory path
```

#### 2. HelpDialog (`src/HelpDialog.hpp` / `src/HelpDialog.cpp`)

wxFrame-based help viewer window with modern UI.

**Features:**
- HTML rendering via wxWebView (modern CSS-capable rendering)
- Navigation buttons: Back, Forward, Home, Table of Contents
- Search bar with live search results
- Internal link handling via `help://` protocol
- External links (http/https) open in default browser
- Modeless operation (doesn't block main window)
- Persistent window size and position

**Member Variables:**
```cpp
HelpManager& mHelpManager;
wxWebView* mWebView;
wxTextCtrl* mSearchBox;
wxButton* mBackButton, mForwardButton, mHomeButton, mTocButton;
std::stack<std::string> mBackStack;      // Navigation history
std::stack<std::string> mForwardStack;   // Forward navigation
std::string mCurrentTopicId;             // Currently displayed topic
```

#### 3. CalChartApp Integration

- `CalChartApp::InitAppAsServer()` — Initialize HelpManager with markdown/HTML docs
- Help menu items trigger `HelpDialog` display
- Ctrl+H keyboard shortcut opens Help
- `GetGlobalHelpManager()` provides access to help system throughout app

### Directory Structure

```
docs/
├── md/                               # Markdown source (primary)
│   ├── index.md                      # Help contents/index
│   ├── introduction.md               # Getting started
│   ├── installation-setup.md         # Installation guide
│   ├── reference/                    # Reference documentation
│   │   ├── overview.md               # Concepts and terminology
│   │   ├── basics.md                 # Basic concepts
│   │   ├── movement.md               # Point movement
│   │   ├── animation.md              # Animation features
│   │   └── [more to be added]
│   ├── tutorials/                    # Step-by-step guides
│   │   ├── creating-first-show.md
│   │   └── [more to be added]
│   └── _resources/                   # Images, styles, diagrams
│       └── images/
│
├── html/                             # Generated HTML (build artifact)
│   ├── index.html
│   ├── introduction.html
│   └── [all md files converted]
│
├── BugReporting.md                   # Bug reporting guide
└── [deprecated files to be removed]

DeveloperDocs/
├── HelpSystemArchitecture.md         # This file
├── HELP_CONTRIBUTOR_GUIDE.md         # Quick reference for adding content
└── HELP_MIGRATION_CHECKLIST.md       # Conversion checklist
```

## Content Format

### Markdown with YAML Frontmatter

All help pages use Markdown with optional YAML frontmatter for metadata:

```markdown
---
title: Moving Points
keywords: movement, translate, animation
related:
  - selecting-points
  - animation-commands
---

# Moving Points

Content here...

## How to Move Points

[detailed instructions]

## See Also
[Related content links]
```

**Frontmatter Fields:**
- `title` — Human-readable page title (used in search results, breadcrumbs)
- `keywords` — Comma-separated search keywords (boost search relevance)
- `related` — Related topic IDs (displayed as "See Also" section at end)

### Supported Markdown Elements

- Headers: `#`, `##`, `###`, etc.
- **Bold** and *italic* text
- `Inline code` and code blocks with syntax highlighting
- Bullet lists (`-` items)
- [Internal links](reference/topic.md) converted to `help://` protocol
- [External links](https://example.com) for http/https URLs
- Tables, blockquotes, horizontal rules
- HTML escape sequences for special characters

## Build-Time Conversion

### Markdown → HTML Pipeline

**When:** During CMake build (before compilation)

**Process:**
1. CMake target `build_help_html` runs Python conversion script
2. Script processes all `.md` files in `docs/md/`
3. Markdown converted to HTML with CSS styling
4. YAML frontmatter parsed and stored as metadata
5. Internal links `[text](file.md)` converted to `help://file` protocol
6. HTML files written to `docs/html/` in build directory

**Script:** `scripts/convert_markdown_to_html.py`

**Implementation:**
- Uses Python `markdown` library if available
- Falls back to built-in regex-based converter (no external dependencies)
- Applies professional CSS styling to all output
- Handles relative path resolution for internal links

**Output Locations:**

After `cmake --build build`:
```
build/
├── docs/
│   ├── html/              # Pre-converted HTML (from build_help_html target)
│   │   ├── index.html
│   │   ├── introduction.html
│   │   └── ...
│   └── md/                # Source Markdown (for fallback)
│       ├── index.md
│       └── ...
```

macOS app bundle:
```
CalChart.app/
└── Contents/
    └── Resources/
        └── docs/
            ├── html/      # HelpManager loads from here first
            │   └── ...
            └── md/        # Fallback if HTML not found
                └── ...
```

## Runtime Behavior

### Loading Process

1. **App Startup** → `CalChartApp::InitAppAsServer()` creates HelpManager
2. **Path Resolution** → Determines help directory based on platform/install location
3. **Primary Load** → `LoadHelpHtmlDirectory(htmlPath)` attempts to load pre-built HTML
4. **Fallback** → If HTML not found, `LoadHelpDirectory(markdownPath)` loads Markdown
5. **Index Build** → Scans loaded topics, builds search index in memory
6. **Load HTML** → `LoadHelpHtmlDirectory(htmlPath)` loads pre-built HTML files
4. **Index Build** → Scans loaded topics, builds search index in memory
5
Full-text search with relevance ranking:

1. Split query into individual words
2. For each topic, score matches:
   - Title matches: +10 points
   - Keyword matches: +5 points each
   - Content matches: +1 point each
3. Return results sorted by relevance (highest first)
4. Case-insensitive search

### Link Navigation

**Internal Links:**
- Format: `help://topic_id` (e.g., `help://reference/movement`)
- Handled by `HelpDialog::OnWebViewNavigating()`
- Navigates within help system, maintains back/forward history

**External Links:**
- Format: `http://` or `https://`
- Handled by wxWebView automatically
- Opens in system default browser

## Content Development Workflow

### Adding a New Help Page

1. **Create Markdown file** in appropriate location:
   ```bash
   docs/md/reference/new-feature.md
   ```

2. **Add frontmatter** with metadata:
   ```yaml
   ---
   title: New Feature Guide
   keywords: feature, configuration
   related:
     - reference/overview
   ---
   ```

3. **Write content** in Markdown:
   ```markdown
   # New Feature Guide

   ## Overview
   Explain the feature...

   ## How to Use
   Step-by-step instructions...
   ```

4. **Add internal links** to related topics:
   ```markdown
   See also [Point Movement](reference/movement.md)
   ```

5. **Link from index** or related pages in `docs/md/index.md`:
   ```markdown
   - [New Feature Guide](reference/new-feature.md)
   ```

6. **Test locally**:
   ```bash
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
   cmake --build build --config Debug
   ./build/src/CalChart
   # Press Ctrl+H to view changes --target build_help_html
   cmake --build build --config Debug
   ```

### Development Tips

- Changes to `docs/md/` take effect immediately when app reloads help
- No rebuild needed for require rebuilding the HTML files via the `build_help_html` CMake target
- Run `cmake --build build --target build_help_html` after changing markdown files
- Full rebuild requirs for internal links: `[text](../reference/topic.md)`
- Keep lines under 100 characters for version control readability
- Use consistent heading hierarchy (one `#` per page)

## Migration Strategy

For detailed tracking of migration progress and remaining work, see [HELP_MIGRATION_CHECKLIST.md](HELP_MIGRATION_CHECKLIST.md).

### Phase Overview

**Phase 1: Foundation** ✓ COMPLETE
- HelpManager infrastructure implemented
- HelpDialog UI created
- CalChartApp integrated
- Initial help content (5 pages)
- Build system updated

**Phase 2: Core Content Migration** 📋 IN PROGRESS
- High-priority reference topics
- Tutorial creation
- FAQ and troubleshooting guides
- See [HELP_MIGRATION_CHECKLIST.md](HELP_MIGRATION_CHECKLIST.md) for detailed task list

**Phase 3: Full Migration** 📋 FUTURE
- Convert remaining legacy HTML files (90+ pages)
- Comprehensive testing
- Update all outdated content
- Add images/diagrams to key topics

**Phase 4: Cleanup** 📋 FUTURE
- Remove legacy HTML files
- Remove wxHtmlHelpController dependency
- Final documentation cleanup

## Performance Considerations

### Build-Time Optimization
- **Markdown → HTML conversion** happens once at build time
- No runtime conversion overhead in distribution builds
- Smaller app footprint (pre-con

### Runtime Optimization
- **Lazy loading**: Topics loaded on-demand from files
- **In-memory caching**: Content loaded once at startup, cached in memory
- **Fast search**: Search index built from loaded topics at startup
- **No string conversion**: std::string used throughout (minimal wxString overhead)

### Memory Usage
- Typical help system: ~500KB-1MB in memory (depends on content volume)
- Search index: ~100KB
- Minimal for a modern application

## Future Enhancements

### Planned Improvements
- [ ] Context-sensitive help buttons in dialogs
- [ ] Video tutorial integration (links to external resources)
- [ ] Offline help bundling
- [ ] Dark mode support for help viewer
- [ ] Multi-language support
- [ ] Community contributions workflow
- [ ] Better Markdown parser (md4c library)

### Possible Additions
- Docstring extraction to generate API documentation
- Interactive tutorials within help system
- Help search suggestions/autocomplete
- Print/export help pages
- Help page analytics (most viewed topics)

## Testing Checklist

- [ ] Help system loads at app startup without errors
- [ ] Help dialog opens with Ctrl+H
- [ ] Help index displays correctly
- [ ] Search returns relevant results
- [ ] Navigation buttons (Back/Forward/Home) work correctly
- [ ] Links to other topics navigate correctly
- [ ] External links open in browser
- [ ] Help dialog can be resized and closed
- [ ] Multiple help dialogs can be open simultaneously
- [ ] New markdown changes appear after rebuild
- [ ] Search updates when new content added

## Key Design Decisions

### Why Markdown?
- Version control friendly (plain text)
- Easy for contributors to edit
- Wide tool support (editors, renderers, converters)
- Low barrier to entry for documentation

### Why Pre-built HTML?
- Build-time conversion eliminates runtime overhead
- Smaller distribution size (optimized HTML)
- Enables professional CSS styling
- No fallback complexity
- Offline distribution friendly

### Why wxWebView?
- Modern HTML/CSS rendering
- Cross-platform support
- Built into wxWidgets
- Automatic link handling
- Interactive web features possible

### Public API: std::string
- Decouples help system from wxWidgets
- Allows use in non-GUI contexts (e.g., calchart_cmd)
- Standard C++ approach
- No wxString dependency in public interface

## Common Questions

**QRebuild the HTML files with `cmake --build build --target build_help_html`, then rebuild CalChart and press Ctrl+H.

**Q: Can I include images in help?**
A: Yes, place images in `docs/md/_resources/images/` and reference as `![alt](_resources/images/file.png)`.

**Q: Does search index update automatically?**
A: Yes, every time HelpManager loads, it scans all topics and builds search index in memory
A: HelpManager automatically falls back to loading Markdown files from `docs/md/`.

**Q: Can help system be used without GUI?**
A: Yes, HelpManager uses standard C++ types (no wxWidgets dependency in public API).

---

**Related Documentation:**
- [HELP_CONTRIBUTOR_GUIDE.md](HELP_CONTRIBUTOR_GUIDE.md) — Quick reference for adding/editing help content
- [HELP_MIGRATION_CHECKLIST.md](HELP_MIGRATION_CHECKLIST.md) — Checklist of pages to convert from legacy system

**Document Version:** 2.0  
**Last Updated:** January 14, 2026  
**Next Review:** After Phase 2 content migration complete
