# Phase 3A: Threading Fix Guide - For Future Implementation

## Problem Statement

The `calchart_cmd` multi-threading implementation (Phase 3A) achieved ~30% performance improvement but produced incorrect output, indicating thread-safety issues in the CalChart core library. This guide provides a roadmap for fixing these issues.

## Identified Issues

### 1. Bison/Flex Parser Thread Safety

**Location**: `src/core/contgram.y`, `src/core/contscan.l`

**Issue**: Bison and Flex generate parsers with global state by default:
```cpp
// Generated code has globals like:
static int yystate = 0;        // parser state
static YY_BUFFER_STATE *yy_buffer_stack = nullptr;  // lexer buffers
// ... more globals
```

**Impact**: Multiple threads calling `CalChart::Show::Create()` concurrently leads to parser state corruption

**Solution**:
1. Rebuild Bison parser with `%pure-parser` or `%define api.pure full`
   - Add to `contgram.y`: `%define api.pure full`
   - Update CMake bison target with `BISON_FLAGS -y`

2. Rebuild Flex lexer with thread-safe mode
   - Add to `contscan.l`: `%option reentrant`
   - Update CMake flex target with `FLEX_FLAGS --reentrant`

3. Update Show::Create() to handle thread-local parser state:
   ```cpp
   // In CalChartShow.cpp
   thread_local yy_buffer_state* thread_buffer = nullptr;
   
   auto Show::Create(...) {
       // Use thread-local state
       yy_push_buffer_state(thread_buffer);
       // ... parse ...
       yy_pop_buffer_state();
   }
   ```

**Effort**: 3-5 hours (rebuild + testing)

**Risk**: Medium - Parser changes could affect all parsing operations

### 2. Animation Compilation Thread Safety

**Location**: `src/core/CalChartAnimationCompile.cpp`

**Issue**: Animation object compilation may use internal caches or global state

**Symptoms**: Output validation failures for `.output.check` files (DumpFileCheck output)

**Investigation Steps**:
1. Search for static variables in Animation classes:
   ```bash
   grep -n "static" src/core/CalChartAnimation*.cpp
   ```
   
2. Check for global caches or memoization:
   ```bash
   grep -n "cache\|memo\|static.*map\|static.*set" src/core/CalChartAnimation*.cpp
   ```

3. Profile concurrent Animation creation:
   - Use ThreadSanitizer: `cmake -DSANITIZER=thread ...`
   - Run with multiple threading levels

**Solution**:
- Convert any static caches to thread-local or per-object storage
- Implement mutex protection for shared state
- Or redesign to avoid sharing between Animation instances

**Effort**: 2-4 hours per issue found

**Risk**: High - Animation is core to CalChart functionality

### 3. Show Object Internal State

**Location**: `src/core/CalChartShow.cpp`

**Issue**: Sheet iteration and continuity parsing may have shared state

**Investigation Steps**:
```cpp
// Check for mutable state in Show
grep -n "mutable" src/core/CalChartShow.h
grep -n "static" src/core/CalChartShow.cpp

// Check for non-const methods called during iteration
for (auto i = show.GetSheetBegin(); i != show.GetSheetEnd(); ++i) {
    // If GetSheetBegin/End or iteration modifies state -> problem
}
```

**Solution**:
- Ensure all iteration methods are const
- Use read-write locks for mutable state if needed
- Consider copy-on-write for sheet data

**Effort**: 1-3 hours per issue

**Risk**: Medium - Could affect all Show operations

## Implementation Roadmap

### Step 1: Enable Thread Sanitizer (1 hour)
```bash
cmake -B build -S . \
  -DCMAKE_BUILD_TYPE=Debug \
  -DSANITIZER=thread

cmake --build build --config Debug
```

Then run with sample files:
```bash
./build/tools/calchart_cmd/calchart_cmd parse \
  --print_show \
  shows/Choc\ Cookie.shw shows/90s.radio.jamz.sexy.shw
```

Watch for ThreadSanitizer warnings.

### Step 2: Fix Bison/Flex (2-3 hours)
1. Modify `src/core/contgram.y`:
   ```bison
   %define api.pure full
   %define api.push-pull push
   ```

2. Modify `src/core/contscan.l`:
   ```flex
   %option reentrant
   %option bison-bridge
   ```

3. Update `src/core/CMakeLists.txt`:
   ```cmake
   BISON_TARGET(CalChartParser contgram.y ${CMAKE_CURRENT_BINARY_DIR}/contgram.cpp
     COMPILE_FLAGS "-y --defines=${CMAKE_CURRENT_BINARY_DIR}/contgram.h"
   )
   
   FLEX_TARGET(CalChartLexer contscan.l ${CMAKE_CURRENT_BINARY_DIR}/contscan.cpp
     COMPILE_FLAGS "--reentrant --bison-bridge"
   )
   ```

4. Rebuild and test:
   ```bash
   cmake --build build --config Debug
   ./test_threading_build.sh  # Run threading tests
   ```

### Step 3: Investigate & Fix Animation Issues (3-5 hours)
1. Profile with ThreadSanitizer to identify specific race conditions
2. Add mutex protection where needed
3. Consider redesigning for thread-local state if applicable

### Step 4: Fix Show Object Issues (1-3 hours)
1. Review sheet iteration for side effects
2. Add thread-local storage for iteration state if needed
3. Implement read-write locks if necessary

### Step 5: Re-Enable Threading in calchart_cmd (1 hour)
In `tools/calchart_cmd/calchart_cmd_parse.hpp`:
```cpp
namespace CalChartCmd {

constexpr auto Parse = [](auto args, auto& os) {
    auto list_of_files = args["<shows>"].asStringList();

    // Determine thread count
    size_t num_threads = std::thread::hardware_concurrency() / 2;
    if (num_threads == 0) num_threads = 2;

    // Use thread pool for parallel processing
    std::vector<std::pair<std::string, std::future<std::string>>> file_futures;
    file_futures.reserve(list_of_files.size());

    // Lambda for processing each file
    auto process_file = [&args](std::string_view file) -> std::string {
        std::ostringstream oss;
        try {
            auto show = OpenShow(file);  // NOW thread-safe!

            if (args["--print_show"].asBool()) {
                PrintShow(*show, oss);
            }
            // ... other operations ...
        } catch (const std::exception& e) {
            oss << "Error: " << e.what() << "\n";
        }
        return oss.str();
    };

    // Enqueue all tasks
    for (auto&& file : list_of_files) {
        file_futures.emplace_back(
            std::string(file),
            std::async(std::launch::async, process_file, std::string(file))
        );
    }

    // Collect results in order
    for (auto& [file, future] : file_futures) {
        os << future.get();
    }
};
}
```

### Step 6: Validation & Testing (2-3 hours)
```bash
# Run full sanity test suite
python3 resources/tests/sanity_tester.py \
  -c ./build/tools/calchart_cmd/calchart_cmd \
  -d shows \
  -g resources/tests/gold.zip

# Compare metrics before/after
# Expected improvement: ~30% (52s → ~36s in release)
```

## Testing Strategy

### Unit Tests
Add thread-specific tests to `src/core/tests/`:
```cpp
TEST_CASE("Concurrent Show Creation", "[threading]") {
    std::vector<std::future<std::unique_ptr<CalChart::Show>>> futures;
    
    for (int i = 0; i < 10; ++i) {
        futures.push_back(std::async(std::launch::async, [i]() {
            // Each thread creates its own show
            std::ifstream input(test_show_files[i % test_show_files.size()]);
            return CalChart::Show::Create(show_mode, input);
        }));
    }
    
    for (auto& future : futures) {
        auto show = future.get();
        CHECK(show != nullptr);
        CHECK(show->GetShowMode() == show_mode);
    }
}
```

### Integration Tests
```bash
# Test with increasing thread levels
for threads in 1 2 4 8; do
    export OMP_NUM_THREADS=$threads
    python3 resources/tests/sanity_tester.py \
      -c ./build/tools/calchart_cmd/calchart_cmd \
      -d shows \
      -g resources/tests/gold.zip
done
```

### Thread Sanitizer
```bash
# Run with ThreadSanitizer enabled
TSAN_OPTIONS=verbosity=1 ./build/tools/calchart_cmd/calchart_cmd parse \
  --print_show \
  shows/*.shw > /dev/null 2>&1

# Check for SUMMARY: ThreadSanitizer: data race
```

## Performance Expectations

After thread-safety fixes:

**Best Case Scenario** (perfect parallelism):
- Release: 52s → ~18s (3x speedup with 4 threads)
- Debug: 165s → ~55s (3x speedup)

**Realistic Scenario** (70% parallelism efficiency):
- Release: 52s → ~24s (2.2x speedup)
- Debug: 165s → ~75s (2.2x speedup)

**Worst Case** (10-20% overhead):
- Release: 52s → ~40s (1.3x speedup)
- Debug: 165s → ~127s (1.3x speedup)

## Common Pitfalls to Avoid

1. **Incomplete Bison/Flex Reentrance**: Just enabling might not be enough - need proper API changes
2. **Missing Lock Protection**: Static state might exist in template instantiations - use grep carefully
3. **Resource Cleanup**: Ensure destructors are thread-safe for concurrent cleanup
4. **Memory Model Issues**: Use `std::memory_order_seq_cst` for critical state if needed
5. **Testing Coverage**: Run tests with ThreadSanitizer and multiple thread levels

## References

### Bison Thread Safety
- `info bison` - Section "Parser C API" for reentrant parsers
- `https://www.gnu.org/software/bison/manual/html_node/Pure-Calling.html`

### Flex Thread Safety  
- `man flex` - Search for "reentrant"
- `https://westes.github.io/flex/manual/Options-for-Code-Generation.html#Threading`

### C++ Threading Best Practices
- `https://en.cppreference.com/w/cpp/thread/thread`
- `https://herbsutter.com/2013/02/11/you-dont-know-cpp-because-you-only-code-in-c/`

## Estimated Total Effort

- **Bison/Flex fixes**: 2-3 hours
- **Animation investigation**: 2-3 hours  
- **Show object fixes**: 1-2 hours
- **Testing & validation**: 2-3 hours
- **Buffer/rework**: 1-2 hours

**Total: 8-13 hours** (1-2 development days)

**Expected ROI**: 
- 2-3x faster sanity_tester execution
- Foundation for future parallel operations
- Better code quality (thread-safe code is more robust)

---

This roadmap provides a clear path forward for implementing thread-safe calchart_cmd. Following these steps should resolve the current issues while providing significant performance improvements.
