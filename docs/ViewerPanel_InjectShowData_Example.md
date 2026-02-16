# ViewerPanel::InjectShowData() - Usage Examples

## Overview

The `ViewerPanel` now supports two methods for loading show data:

1. **`UpdateShowData()`** - Fetches data from `/api/show` endpoint (uses current document)
2. **`InjectShowData(jsonData)`** - Directly injects JSON string (bypasses server)

## Method 1: Using Existing Document (Current Approach)

```cpp
// This is what happens now - uses mDoc to generate JSON via server
viewerPanel->UpdateShowData();

// Behind the scenes:
// 1. JavaScript calls /api/show
// 2. ViewerServer calls mCurrentDoc->toViewerJSON()
// 3. JSON returned to viewer
// 4. ShowUtils.fromJSON() creates Show object
```

## Method 2: Direct JSON Injection (New Feature)

### Example 1: Inject from Current Document
```cpp
void SomeCalChartFrame::OnSomeEvent()
{
    if (mDoc && mViewerPanel) {
        // Get JSON from current document
        nlohmann::json showJson = mDoc->toViewerJSON();
        
        // Inject directly into viewer (bypasses server)
        mViewerPanel->InjectShowData(showJson.dump());
    }
}
```

### Example 2: Inject from File
```cpp
void SomeCalChartFrame::OnLoadViewerFile()
{
    wxFileDialog dialog(
        this,
        "Choose a viewer file",
        "",
        "",
        "Viewer files (*.viewer)|*.viewer|JSON files (*.json)|*.json",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST
    );

    if (dialog.ShowModal() == wxID_OK) {
        std::string filepath = dialog.GetPath().ToStdString();
        
        // Read JSON file
        std::ifstream file(filepath);
        std::string jsonData((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
        
        // Inject into viewer
        mViewerPanel->InjectShowData(jsonData);
    }
}
```

### Example 3: Inject Custom/Test Data
```cpp
void SomeCalChartFrame::OnLoadTestShow()
{
    // Create a simple test show
    nlohmann::json testShow = {
        {"meta", {{"version", "1.0.0"}}},
        {"show", {
            {"title", "Test Show"},
            {"year", "2026"},
            {"description", "A simple test"},
            {"dotLabels", {"A1", "A2", "B1", "B2"}},
            {"sheets", {
                {
                    {"name", "Opener"},
                    {"duration", 16},
                    {"dots", {
                        {"A1", {
                            {{"type", "stand"}, {"beats", 16}, {"x", 0}, {"y", 0}}
                        }},
                        {"A2", {
                            {{"type", "stand"}, {"beats", 16}, {"x", 10}, {"y", 0}}
                        }},
                        // ... more dots
                    }}
                }
            }}
        }}
    };
    
    mViewerPanel->InjectShowData(testShow.dump());
}
```

## When to Use Each Method

### Use `UpdateShowData()` when:
- You want to display the current document
- The document has been modified and you want to refresh the viewer
- You're working within the normal CalChart workflow

### Use `InjectShowData()` when:
- You want to preview a show without modifying the current document
- You're loading a .viewer file directly
- You're testing with synthetic data
- You want to compare multiple shows (by injecting different data)
- You're implementing a "preview before open" feature

## Technical Details

### How InjectShowData() Works

1. **Receives JSON string** - Must be valid viewer format JSON
2. **Creates JavaScript variable** - `window.__calchart_injected_data = {...}`
3. **Calls ShowUtils.fromJSON()** - Parses JSON into Show object
4. **Calls setShow()** - Updates ApplicationController with new show
5. **Cleans up** - Deletes temporary variable

### Error Handling

The method includes error handling in JavaScript:
- Catches parse errors
- Logs to browser console
- Cleans up temporary variables even on error

### Format Requirements

The JSON must match the viewer file format (version 1.0.0):
```json
{
  "meta": {
    "version": "1.0.0"
  },
  "show": {
    "title": "...",
    "year": "...",
    "description": "...",
    "dotLabels": [...],
    "sheets": [...]
  }
}
```

## Debugging

To see what's happening:
1. Open Chrome DevTools in the embedded viewer (if available)
2. Look for console messages:
   - "Show data injected successfully via InjectShowData()"
   - "Error injecting show data: ..."

## Example Integration in CalChartFrame

```cpp
// In CalChartFrame.h
class CalChartFrame : public wxFrame {
    // ...
    ViewerPanel* mViewerPanel;
    
    void OnViewerInjectTestData(wxCommandEvent& event);
};

// In CalChartFrame.cpp
void CalChartFrame::OnViewerInjectTestData(wxCommandEvent& event)
{
    if (!mViewerPanel) {
        wxMessageBox("Viewer not open", "Error");
        return;
    }
    
    // Get JSON from current doc
    if (mDoc) {
        auto json = mDoc->toViewerJSON();
        mViewerPanel->InjectShowData(json.dump());
        wxLogMessage("Injected current document into viewer");
    } else {
        wxMessageBox("No document loaded", "Error");
    }
}
```

## Performance Notes

- **InjectShowData()** is faster than `UpdateShowData()` because it bypasses the HTTP request
- Both methods avoid full page reload (no `RefreshViewer()` call)
- The viewer's state (selected dot, current sheet/beat) is reset when new data loads
