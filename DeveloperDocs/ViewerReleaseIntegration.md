# CalChart Viewer - Release Build Fix

## Problem

The CalChart Viewer was not working in release builds (CI-built installers) - it showed only static HTML without functionality. The JavaScript and CSS were not loading.

## Root Cause

### Issue #1: Static Assets Not Served in Release Builds

In [src/ViewerServer.cpp](../src/ViewerServer.cpp), static asset serving was conditional on `CMAKE_VIEWER_SOURCE_DIR`:

```cpp
#ifdef CMAKE_VIEWER_SOURCE_DIR
    // Serve static files from filesystem
    mServer->Get(R"(.+\.(css|js|...))", [...]);
#endif
```

This define is ONLY set in Debug builds (see [src/CMakeLists.txt#L220-227](../src/CMakeLists.txt#L220-227)):

```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(CalChart PRIVATE CMAKE_VIEWER_SOURCE_DIR="...")
endif()
```

**Result:** In Release builds, ViewerServer had NO route handlers for static assets. The HTML loaded but referenced `build/js/application.js` which returned 404.

### Issue #2: CI Builds Missing Viewer Assets

The GitHub Actions workflow ([.github/workflows/cmake.yml](../.github/workflows/cmake.yml)) had no Node.js setup step. 

[viewer/CMakeLists.txt](../viewer/CMakeLists.txt) requires Node.js and npm:

```cmake
find_program(NODE_EXECUTABLE NAMES node nodejs)
if(NOT NODE_EXECUTABLE)
    message(WARNING "Node.js not found. Viewer will not be built.")
    return()
endif()
```

**Result:** CI builds skipped building viewer assets entirely. No `viewer/build/` directory was created.

### Issue #3: Viewer Assets Not Bundled with App

Even if viewer assets were built, they weren't copied into the application bundle/installer. The CMakeLists.txt only copied docs and resources, not viewer assets.

## Solution Applied

### 1. Added Node.js to CI Workflow

Added Node.js setup immediately after checkout in [.github/workflows/cmake.yml](../.github/workflows/cmake.yml):

```yaml
- name: Setup Node.js
  uses: actions/setup-node@v4
  with:
    node-version: '20'
    cache: 'npm'
    cache-dependency-path: viewer/package.json
```

This ensures viewer assets are built during CI.

### 2. Made Static Asset Serving Work in Release Builds

Modified [src/ViewerServer.cpp](../src/ViewerServer.cpp):

**Added helper function** to find viewer assets in both debug and release:

```cpp
std::string GetViewerAssetsPath()
{
#ifdef CMAKE_VIEWER_SOURCE_DIR
    // Debug: source directory
    return std::string(CMAKE_VIEWER_SOURCE_DIR);
#else
    // Release: bundled Resources
    #ifdef __APPLE__
        wxString resourcesDir = wxStandardPaths::Get().GetResourcesDir();
        return (resourcesDir + wxFILE_SEP_PATH + "viewer").ToStdString();
    #else
        // Linux/Windows: relative to executable
        wxFileName exePath(wxStandardPaths::Get().GetExecutablePath());
        return (exePath.GetPath() + wxFILE_SEP_PATH + "viewer").ToStdString();
    #endif
#endif
}
```

**Removed conditional compilation** for static asset routes - now they work in both debug and release using the helper function.

### 3. Bundle Viewer Assets with Application

Added CMake commands to copy viewer assets to app bundle/executable directory:

**macOS** ([src/CMakeLists.txt#L310-335](../src/CMakeLists.txt)):
- Copies to `CalChart.app/Contents/Resources/viewer/`
- Includes `viewer/build/`, `viewer/js/`, `viewer/css/`, `viewer/img/`, `viewer/index.html`

**Windows** ([src/CMakeLists.txt#L357-379](../src/CMakeLists.txt)):
- Copies to `CalChart.exe/../viewer/`

**Linux** ([src/CMakeLists.txt#L402-424](../src/CMakeLists.txt)):
- Copies to executable directory `./viewer/`

## Testing

### Local Testing

Debug builds continue to work as before - serving from `viewer/` source directory.

### Release Build Testing

To test a release-like build locally:

```bash
# Build in Release mode
cmake -B build/release -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build/release --config Release

# Run the app
./build/release/src/CalChart.app/Contents/MacOS/CalChart  # macOS
./build/release/src/CalChart                               # Linux
```

**Verify:**
1. Open CalChart
2. Open a show file
3. View > Preview in Viewer (experimental)...
4. Viewer should load with full functionality (not just static HTML)
5. Check browser console (if available) - no 404 errors for CSS/JS

### CI Testing

The next CI build should:
1. Install Node.js dependencies via npm
2. Build viewer assets with grunt
3. Bundle them with the installer
4. Produce a working viewer in installed releases

## Files Changed

- [.github/workflows/cmake.yml](../.github/workflows/cmake.yml) - Added Node.js setup
- [src/ViewerServer.cpp](../src/ViewerServer.cpp) - Universal asset serving
- [src/CMakeLists.txt](../src/CMakeLists.txt) - Bundle viewer assets for all platforms

## Why the Local/Installed Experiment Worked Differently

The user's observation that launching the installed version then the local version "worked" was likely coincidental or related to:
- Browser caching from previous sessions
- The local version actually running (not the installed one)
- Port conflicts causing fallback behavior

Each CalChart instance runs its own in-process ViewerServer on port 1868. They cannot share servers across processes.

## Future Considerations

### Alternative: Embed All Assets in Binary

For a true single-binary deployment with no external files, we could:
1. Generate C++ headers for all CSS/JS/images (similar to how we embed HTML)
2. Serve entirely from memory

**Tradeoff:** Larger binary, more complex build, but no risk of missing files.

### Alternative: Single HTML with Inlined Assets

Build a single `viewer.html` with all CSS/JS/images inlined as data URLs.

**Tradeoff:** Very large HTML string, but simplest deployment.

The current solution (bundling built assets) balances simplicity, build time, and maintainability.
