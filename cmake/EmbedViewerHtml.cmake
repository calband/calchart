# CMake script to embed viewer HTML into a C++ header file
#
# This script reads the viewer/index.html file and generates a C++ header
# that contains the HTML as a string constant.
#
# In DEBUG builds, the generated header will include logic to optionally
# serve files directly from disk for live editing.

function(embed_viewer_html TARGET_NAME VIEWER_SOURCE_DIR OUTPUT_HEADER)
    set(VIEWER_HTML_FILE "${VIEWER_SOURCE_DIR}/index.html")
    
    # Read the HTML file
    if(EXISTS "${VIEWER_HTML_FILE}")
        file(READ "${VIEWER_HTML_FILE}" VIEWER_HTML_CONTENT)
        
        # Escape special characters for C++ string literal
        string(REPLACE "\\" "\\\\" VIEWER_HTML_CONTENT "${VIEWER_HTML_CONTENT}")
        string(REPLACE "\"" "\\\"" VIEWER_HTML_CONTENT "${VIEWER_HTML_CONTENT}")
        string(REPLACE "\n" "\\n\"\n\"" VIEWER_HTML_CONTENT "${VIEWER_HTML_CONTENT}")
        
        # Generate the header file content
        set(HEADER_CONTENT "// Auto-generated file - do not edit manually
// Generated from ${VIEWER_HTML_FILE}

#pragma once

#include <string>
#include <fstream>
#include <sstream>

namespace CalChart {
namespace ViewerHtml {

// Path to viewer source directory (only available in debug builds)
#ifdef CMAKE_VIEWER_SOURCE_DIR
constexpr const char* kViewerSourceDir = CMAKE_VIEWER_SOURCE_DIR;
constexpr bool kUseFileSystem = true;
#else
constexpr const char* kViewerSourceDir = nullptr;
constexpr bool kUseFileSystem = false;
#endif

// Embedded HTML content
inline const char* GetEmbeddedHtml() {
    return \"${VIEWER_HTML_CONTENT}\";
}

// Get viewer HTML - from filesystem in debug, embedded in release
inline std::string GetViewerHtml() {
    if constexpr (kUseFileSystem) {
        if (kViewerSourceDir != nullptr) {
            std::string path = std::string(kViewerSourceDir) + \"/index.html\";
            std::ifstream file(path);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                return buffer.str();
            }
            // Fall through to embedded if file can't be read
        }
    }
    return GetEmbeddedHtml();
}

} // namespace ViewerHtml
} // namespace CalChart
")
        
        # Write the header file
        file(WRITE "${OUTPUT_HEADER}" "${HEADER_CONTENT}")
        
        message(STATUS "Generated viewer HTML header: ${OUTPUT_HEADER}")
        message(STATUS "  Source: ${VIEWER_HTML_FILE}")
        
    else()
        message(WARNING "Viewer HTML file not found: ${VIEWER_HTML_FILE}")
        message(WARNING "Using fallback embedded HTML")
        
        # Create a minimal fallback header
        set(HEADER_CONTENT "// Auto-generated fallback - viewer source not found
#pragma once
#include <string>
namespace CalChart {
namespace ViewerHtml {
inline std::string GetViewerHtml() {
    return \"<html><body><h1>Viewer not available</h1><p>viewer/index.html not found</p></body></html>\";
}
}
}
")
        file(WRITE "${OUTPUT_HEADER}" "${HEADER_CONTENT}")
    endif()
endfunction()
