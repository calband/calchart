# Capturing wxLog Lines for Bug Reports

## Overview
This document outlines how to capture the last N log lines from wxLog into bug reports using wxWidgets' logging system.

## How wxLog Works
wxWidgets provides a flexible logging system with several key components:

1. **wxLog**: Base logging class - the foundation of the system
2. **wxLogTextCtrl**: Logs to a wxTextCtrl 
3. **wxLogStderr**: Logs to stderr (default in non-GUI apps)
4. **wxLogGui**: Shows messages in message boxes (default in GUI apps)
5. **wxLogChain**: Chains multiple loggers together

The key insight: **You can install a custom wxLog instance that captures log messages while also chaining to the default behavior**.

## Architecture: Custom wxLog Target

### High-level Strategy

1. **Create a custom `CircularLogBuffer` class** that:
   - Inherits from `wxLog` (or implements a custom log target)
   - Maintains a circular buffer of the last N log messages
   - Stores: timestamp, log level, message
   - Can be queried to retrieve all captured messages

2. **Install it during app initialization** in `CalChartApp::OnInit()`:
   - Create the circular buffer with a configurable size (e.g., 100 lines)
   - Use `wxLog::SetActiveTarget()` or chain it with the existing logger
   - Keep a global reference for bug report access

3. **Query it when building bug reports** in `BugReportDialog`:
   - Add a method to `DiagnosticInfo` to include log lines
   - Format as a markdown code block in the bug report

## Implementation Approach

### 1. Core Logger Class (Platform-independent)

**File**: `core/CircularLogBuffer.hpp` / `core/CircularLogBuffer.cpp`

```cpp
namespace CalChart {

struct LogMessage {
    std::string timestamp;     // ISO 8601 format
    std::string level;         // "Info", "Warning", "Error", etc.
    std::string message;
};

class CircularLogBuffer {
public:
    explicit CircularLogBuffer(size_t capacity = 100);
    
    // Add a message to the buffer
    void AddMessage(std::string level, std::string message);
    
    // Get all messages in chronological order
    [[nodiscard]] std::vector<LogMessage> GetMessages() const;
    
    // Clear the buffer
    void Clear();
    
private:
    std::vector<LogMessage> buffer_;
    size_t capacity_;
    size_t current_index_;
    mutable std::mutex lock_;
};

} // namespace CalChart
```

**Why platform-independent?**
- The log buffer logic is pure C++
- Doesn't depend on wxWidgets or UI framework
- Can be tested without GUI
- Makes it reusable

### 2. wxWidgets Integration

**File**: `src/CalChartLogTarget.h` / `src/CalChartLogTarget.cpp`

This creates a bridge between wxWidgets' logging and our circular buffer:

```cpp
class CalChartLogTarget : public wxLog {
public:
    explicit CalChartLogTarget(CalChart::CircularLogBuffer& buffer);
    
protected:
    void DoLogText(const wxLogRecordInfo& info, const wxString& msg) override;
    
private:
    CalChart::CircularLogBuffer& buffer_;
};
```

### 3. Integration Points

**In CalChartApp** (`src/CalChartApp.h/cpp`):
- Create a global `CircularLogBuffer` instance
- Install `CalChartLogTarget` during `OnInit()`
- Provide accessor method: `GetLogBuffer()`
- Use `wxLog::SetActiveTarget()` or chain loggers

**In DiagnosticInfo** (`src/DiagnosticInfo.cpp`):
- Query the log buffer when collecting diagnostic info
- Add methods:
  - `CollectLogLines()` - gets recent log messages
  - Format them for the bug report

**In BugReportDialog** (`src/BugReportDialog.cpp`):
- Include log lines in the diagnostic info section
- Display them in a collapsible section or code block
- Allow users to see what happened before the report

## Key Design Decisions

### Thread Safety
- Use `std::mutex` in the circular buffer for thread-safe access
- wxLog operations are usually on the main thread, but it's good practice

### Circular Buffer Implementation
Option 1: Pre-allocated vector with index wrapping (simple, efficient)
Option 2: Using `std::deque` with size checking (simpler code)

```cpp
// Simple circular approach - pre-allocated
void CircularLogBuffer::AddMessage(std::string level, std::string message) {
    std::lock_guard<std::mutex> lock(lock_);
    
    if (current_index_ >= capacity_) {
        current_index_ = 0;  // Wrap around
    }
    
    if (buffer_.size() < capacity_) {
        buffer_.push_back({timestamp, level, message});
    } else {
        buffer_[current_index_] = {timestamp, level, message};
    }
    current_index_++;
}
```

### Log Level Mapping
wxLog provides different log levels:
- `wxLOG_FatalError` → "Fatal Error"
- `wxLOG_Error` → "Error"
- `wxLOG_Warning` → "Warning"
- `wxLOG_Message` → "Info"
- `wxLOG_Debug` → "Debug"
- `wxLOG_Trace` → "Trace"

You can configure which levels to capture (e.g., skip debug/trace in Release builds).

## Integration with Existing Bug Report System

The existing structure already has:
- `CalChart::DiagnosticInfo` - stores diagnostic data
- `CalChart::DiagnosticInfo::toString()` - formats as markdown
- `BugReportDialog` - collects and files reports
- `BugReport` struct - holds all report data

### Minimal Changes:
1. Add `log_lines` field to `CalChart::DiagnosticInfo`:
   ```cpp
   struct DiagnosticInfo {
       // ... existing fields ...
       std::vector<CalChart::LogMessage> recent_logs;
   };
   ```

2. Update `toString()` to include logs in markdown:
   ```markdown
   ## Recent Log Messages
   \`\`\`
   [timestamp] [level] message
   [timestamp] [level] message
   \`\`\`
   ```

3. In `src/DiagnosticInfo.cpp`, populate logs when collecting info:
   ```cpp
   auto wxCalChart::DiagnosticInfo::CollectDiagnosticInfo(CalChartDoc const* doc)
   {
       auto info = CalChart::DiagnosticInfo::Create();
       // ... existing code ...
       info.recent_logs = wxCalChart::GetGlobalApp().GetLogBuffer().GetMessages();
       return info;
   }
   ```

## Configuration Options

Consider these as CMake or runtime options:
- `CALCHART_LOG_BUFFER_SIZE` - number of lines to keep (default: 100)
- `CALCHART_LOG_LEVELS_TO_CAPTURE` - which levels to capture (default: Warning, Error, FatalError)
- `CALCHART_LOG_INCLUDE_IN_REPORTS` - whether to include logs (default: true)

## Privacy Considerations

**Important**: Log messages could contain sensitive information:
- File paths (may reveal user directory structure)
- User data embedded in parsed content
- Performance metrics that could identify a user

### Recommendations:
1. **User control**: Let users see what's being captured
   - Already done in `BugReportDialog` - shows diagnostic info before submit
   - Users can review and edit if needed

2. **Log levels**: Don't capture `wxLOG_Debug` / `wxLOG_Trace` in Release builds
   - These are typically more verbose

3. **Documentation**: Note in Help/Documentation that logs are captured
   - Similar to other diagnostic info (OS, version, etc.)

## Testing Strategy

### Unit Tests (`core/tests/CircularLogBufferTests.cpp`):
- Test adding messages at capacity
- Test wrapping behavior
- Test formatting output
- Test thread safety (manual verification)

### Integration Tests (`src/tests/DiagnosticInfoTests.cpp`):
- Test log collection during app operation
- Test formatting in bug report context
- Test with real wxLog messages

### Manual Testing:
1. Trigger some errors/warnings in CalChart
2. Open bug report dialog
3. Verify recent logs appear in diagnostic info
4. Verify formatting looks correct in markdown

## Example Output

When formatted in a bug report:

```markdown
## System Information
...

## Recent Log Messages
[2025-12-27 14:30:45] Warning: Could not load image from file.png
[2025-12-27 14:30:46] Error: Parser encountered unexpected token
[2025-12-27 14:30:47] Message: File saved successfully
```

## Next Steps

1. ✅ Implement `CircularLogBuffer` in core - **COMPLETED**
   - Created `core/CircularLogBuffer.hpp` and `core/CircularLogBuffer.cpp`
   - Implements circular buffer with thread-safe operations
   - Supports timestamps, log levels, and formatting

2. ✅ Create `CalChartLogTarget` wxWidgets adapter - **COMPLETED**
   - Created `src/CalChartLogTarget.h` and `src/CalChartLogTarget.cpp`
   - Inherits from `wxLog` and bridges to `CircularLogBuffer`
   - Maps wxLog levels to string representations

3. ✅ Install in `CalChartApp::OnInit()` - **COMPLETED**
   - Modified `src/CalChartApp.h` to add log buffer, target, and chain members
   - Modified `src/CalChartApp.cpp` to initialize buffer and chain in `InitAppAsServer()`
   - Added `GetLogBuffer()` accessor method
   - Uses `wxLogChain` to properly chain loggers so messages flow through our capture AND the original system
   - Messages are captured but then forwarded to the original logger

4. ✅ Extend `DiagnosticInfo` to collect logs - **COMPLETED**
   - Added `recent_logs` field to `CalChart::DiagnosticInfo` struct
   - Updated `toString()` method to include log messages in markdown format
   - Logs are formatted as a code block with timestamp, level, and message

5. ✅ Collect logs in `wxCalChart::DiagnosticInfo` - **COMPLETED**
   - Modified `src/DiagnosticInfo.cpp::CollectDiagnosticInfo()`
   - Queries the global app's log buffer
   - Populates `info.recent_logs` with captured messages

6. ✅ Write unit tests - **COMPLETED**
   - Created `core/tests/CircularLogBufferTests.cpp`
   - Tests cover: adding/retrieving messages, circular wrapping, formatting, clearing, timestamps
   - Tests verify thread safety and chronological ordering

7. ✅ Update CMakeLists files - **COMPLETED**
   - Added `CircularLogBuffer.cpp/hpp` to `core/CMakeLists.txt`
   - Added `CalChartLogTarget.cpp/h` to `src/CMakeLists.txt`
   - Added `CircularLogBufferTests.cpp` to `core/tests/CMakeLists.txt`

8. ✅ Update documentation - **IN PROGRESS**
   - Marking completed steps in this document

## Testing the Implementation

To verify the implementation works:

1. Build the project:
   ```bash
   cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
   cmake --build build --config Debug
   ```

2. Run unit tests:
   ```bash
   ctest --test-dir build --output-on-failure
   ```

3. Manual testing:
   - Launch CalChart
   - Trigger some errors/warnings (e.g., try to load a bad file)
   - Open the bug report dialog (Help → Report a Bug or Ctrl+Shift+B)
   - Verify that recent log messages appear in the diagnostic info
   - Check that the logs are formatted correctly with timestamps and levels

## Implementation Summary

The log capture system is now fully integrated and ready to use:

- **Auto-capture**: All wxLog messages are automatically captured without code changes
- **Circular buffer**: The last 100 messages are retained (configurable)
- **Thread-safe**: All operations use mutex protection
- **Privacy-aware**: Users see logs before submitting, can review/edit
- **Formatted output**: Logs appear as a markdown code block in bug reports
- **Well-tested**: Unit tests verify core functionality

The system integrates seamlessly with the existing bug reporting infrastructure.

---

## References

- [wxLog Documentation](https://docs.wxwidgets.org/3.2/overview_log.html)
- [wxLog::SetActiveTarget()](https://docs.wxwidgets.org/3.2/classwx_log.html#a59f3e77c25cec7dd98a5d9f937f00c65)
- Existing code: `src/DiagnosticInfo.h/cpp`, `core/CalChartDiagnosticInfo.hpp/cpp`
