# wxWebView Demo in CalChart

This demonstrates the integration of wxWebView into CalChart to show webview functionality is working.

## How to Use

1. Build CalChart with webview support using one of the provided CMake presets:
   - **Mac (System wxWidgets)**: "Mac - Release (System wxWidgets)"
   - **Mac (vcpkg with overlay)**: "Mac - Release (vcpkg with overlay)"
   - **Windows**: Standard Windows preset (vcpkg includes webview support)

2. Launch CalChart

3. From the menu, go to **Debug → wxWebView Demo**

4. A dialog will open showing a simple web browser:
   - Enter a URL in the text field (e.g., `https://example.com`)
   - Click the "Go" button to navigate

## What This Shows

- wxWebView is available and working on your platform
- On macOS: Uses native WebKit
- On Windows: Uses WebView2 (Chromium/Edge)
- Cross-platform API compatibility

## Implementation Details

- [WebViewDemoDialog.h](WebViewDemoDialog.h) - Header
- [WebViewDemoDialog.cpp](WebViewDemoDialog.cpp) - Implementation
- Menu integration in [CalChartSplash.cpp](CalChartSplash.cpp)

## Building with WebView Support

### macOS - System wxWidgets
```bash
cmake --preset=mac-release-system
cmake --build build/mac-release-system
```

### macOS - vcpkg with WebView Fix
```bash
cmake --preset=mac-release-vcpkg-overlay
cmake --build build/mac-release-vcpkg-overlay
```

### Windows
```bash
cmake --preset=windows
cmake --build build/windows
```

## Platform Notes

- **macOS**: wxWidgets uses native WebKit (requires macOS 10.10+)
- **Windows**: Uses WebView2 (available in Windows 10+)
- **Linux**: Uses GTK WebKit
