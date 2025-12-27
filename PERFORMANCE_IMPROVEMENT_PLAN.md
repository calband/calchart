# CalChart Sanity Tester Performance Improvement Plan

## Current State Analysis

### Overview
The `sanity_tester.py` script validates CalChart outputs by:
1. Running `calchart_cmd` on show files with multiple output options (10 variants per file)
2. Comparing generated outputs against gold standard files
3. Using custom regex-based comparisons with tolerance for floating-point differences

### Identified Bottlenecks & Optimization Opportunities

---

## Phase 1: Instrumentation & Measurement
**Goal:** Quantify time spent in each operation to identify actual bottlenecks

### 1.1 Add Timing Instrumentation to `sanity_tester.py`

Track the following:
- **calchart_cmd execution time**: Measure subprocess.run() calls (most likely bottleneck)
  - Per-file execution time
  - Per-command-variant execution time
  - Total execution time across all files

- **File I/O time**: Reading/writing output files
  - File write time from subprocess
  - File read time during comparison

- **Comparison time**: Breakdown of string comparison phases
  - Time spent in `compare_files()`
  - Time spent in `extractValues()` regex parsing
  - Time spent in floating-point tolerance checks

- **Python processing overhead**: ZIP extraction, file walking, etc.

### Implementation Strategy:
- Add timing decorators/context managers for major function blocks
- Use `time.perf_counter()` for high-resolution measurements
- Generate a summary report showing:
  - Total time per file
  - Time breakdown by phase (exec vs. comparison vs. overhead)
  - Percentage time in each component
- Optional: CSV export for analysis

**Instrumented code locations:**
- `run_command()` and `run_command_ps()` — wrap subprocess.run() calls
- `compare_files()` and `custom_comparison_function()` — wrap each comparison phase
- `main()` — overall timing and per-file summaries

---

## Phase 2: Performance Analysis (Post-Instrumentation)

Once instrumentation is in place, run sanity_tester with timing output and analyze:

### Expected Findings & Responses:

#### **Scenario A: calchart_cmd dominates** (likely in debug mode)
If > 80% of time is in subprocess execution:
- Move to Phase 3A: **Multi-threaded calchart_cmd optimization**
- Profile parsing, compilation, and generation phases in `calchart_cmd`
- Evaluate thread pools for parallel sheet processing

#### **Scenario B: String comparison dominates**
If > 20% of time is in file comparison:
- Move to Phase 3B: **Optimize regex and comparison logic**
- Compile regex patterns once, reuse
- Consider binary comparison for files with no special rules
- Optimize `extractValues()` with faster parsing

#### **Scenario C: I/O and Python overhead dominates**
If multiprocessing pool efficiency is poor:
- Move to Phase 3C: **Optimize test harness parallelization**
- Tune process pool size and batching strategy
- Measure subprocess startup overhead

---

## Phase 3: Optimization Implementations

### Phase 3A: Multi-threaded calchart_cmd (if calchart_cmd is the bottleneck)

**Approach 1: Sheet-level Parallelism**
- Identify phases in show processing that can be parallelized:
  - Sheet parsing (each sheet independently)
  - Continuity compilation (per-sheet or per-animation)
  - PostScript generation (per-sheet or per-output-format)
- Use C++20 thread pool (or std::async) to parallelize these phases
- Minimize lock contention on shared data structures

**Approach 2: Output Format Parallelism**
- Currently all 10 output variants are generated sequentially from one parsed show
- Could potentially generate multiple output formats in parallel

**Key files to analyze:**
- [tools/calchart_cmd/main.cpp](tools/calchart_cmd/main.cpp) — PrintToPS() and Parse() functions
- [tools/calchart_cmd/calchart_cmd_parse.hpp](tools/calchart_cmd/calchart_cmd_parse.hpp) — Parse() implementation
- Core library parsing/compilation logic in `src/core/`

---

### Phase 3B: Optimize String Comparison (if comparison dominates)

**Quick Wins:**
1. **Pre-compile regex patterns**: Move `re.compile()` outside loops
   - Pattern: `pt \d+: \(-?\d+\), dir=(-?\d+(\.\d+)?([eE]-?\d+)?)`
   - Compile once at module load, reuse in `extractValues()`

2. **Lazy evaluation**: Only parse lines matching expected patterns
   - Use startswith() checks before expensive regex matching
   - Skip regex if line doesn't start with "pt "

3. **Fast-path for non-numeric lines**: 
   - Check if line contains "pt" before regex attempt
   - Use simple string equality for non-point data

4. **Micro-optimize extractValues()**: 
   - Use `re.search()` instead of `re.match()` with boundary checking
   - Cache tolerance value

5. **Binary comparison where possible**: 
   - For non-PostScript formats (.print, .dump, .animate, .json), use direct byte comparison after filtering
   - Only use custom logic for PostScript files (where date filtering and line tolerance matter)

**Code locations:**
- [resources/tests/sanity_tester.py](resources/tests/sanity_tester.py) — `extractValues()`, `custom_comparison_function()`, `compare_files()`

---

### Phase 3C: Optimize Test Harness Parallelization (if overhead dominates)

**Current state:** 
- Uses `multiprocessing.Process` with simple pool management
- Limits concurrent jobs to `num_cores * 2`
- Creates new process for every command variant

**Potential improvements:**
1. **Use ProcessPoolExecutor**: Better resource management than manual process handling
2. **Batch per-file operations**: Reduce per-file Python overhead
3. **Reduce process count**: Process creation is expensive; batch larger units
4. **Measure process startup vs. execution time**: If startup > 10% of execution, reduce variant counts per batch

---

## Phase 4: Implementation Roadmap

### Step 1: Instrument sanity_tester.py (1-2 hours)
- Add timing measurements
- Generate summary report
- Run and analyze on real test suite

### Step 2: Based on Phase 2 findings, prioritize:
- If calchart_cmd > 60% → Phase 3A (likely 2-4 hours of profiling + coding)
- If comparison > 20% → Phase 3B (30 min - 1 hour, high impact)
- If overhead > 15% → Phase 3C (1-2 hours)

### Step 3: Implement selected optimizations
- Start with Phase 3B (highest ROI, lowest risk)
- Then Phase 3A if needed (requires C++ threading knowledge)
- Phase 3C is last (lower impact typically)

### Step 4: Validate improvements
- Re-run instrumented sanity_tester
- Compare before/after timing
- Ensure all tests still pass

---

## Success Criteria

- **Debug mode**: 2-3x speedup (target: < 30 seconds for standard test suite)
- **Release mode**: 1.5-2x speedup (target: < 10 seconds)
- All tests pass with same accuracy
- No false positives/negatives in comparisons

---

## Files to Modify

### Primary:
1. [resources/tests/sanity_tester.py](resources/tests/sanity_tester.py) — Add instrumentation & optimizations
2. [tools/calchart_cmd/main.cpp](tools/calchart_cmd/main.cpp) — If threading required
3. [tools/calchart_cmd/calchart_cmd_parse.hpp](tools/calchart_cmd/calchart_cmd_parse.hpp) — If threading required

### Potentially:
- Core library files in `src/core/` (if major refactoring of parsing pipeline needed)

---

## Next Steps

**Immediate action**: Implement Phase 1 instrumentation (see detailed steps in Phase 1 section). Once timing data is collected, revisit this plan and prioritize Phase 3 implementations.

