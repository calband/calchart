# CalChart Sanity Tester Performance Optimization - Executive Summary

## Problem Statement
The CalChart `sanity_tester.py` validation tests are slow, especially in debug mode. A typical test run can take significant time, making iterative development slower. We need to:
1. Identify the actual bottlenecks (not guesses)
2. Optimize systematically based on data
3. Achieve 2-3x speedup in debug mode, 1.5-2x in release mode

## Solution Overview

### Three-Phase Approach

#### **Phase 1: Instrumentation & Measurement** (Immediate - 1-2 hours)
**Goal**: Add timing instrumentation to pinpoint bottlenecks

Add detailed metrics to `resources/tests/sanity_tester.py`:
- Time each `calchart_cmd` subprocess execution
- Time each file comparison operation
- Measure string comparison phases (regex vs. direct comparison)
- Generate a summary report showing time breakdown

**Deliverables**:
- Modified `sanity_tester.py` with timing instrumentation
- Human-readable metrics report on each test run
- Optional JSON export for detailed analysis
- Clear visibility into: command execution time vs. file comparison vs. overhead

**Why first?**: Without data, optimization is guesswork. Phase 1 tells us exactly where to focus effort.

---

#### **Phase 2: Analysis** (After Phase 1 - 30 min)
**Goal**: Identify the actual bottleneck from metrics data

Run instrumented sanity_tester and analyze output:

**Likely Scenarios & Next Actions**:

| Scenario | Signal | Next Phase |
|----------|--------|-----------|
| **calchart_cmd dominates** | > 80% of time | Phase 3A: Multi-thread calchart_cmd |
| **String comparison heavy** | > 20% of time | Phase 3B: Optimize regex & comparison |
| **I/O & Python overhead** | > 15% of time | Phase 3C: Optimize test harness |

**Probability Assessment**:
- **Phase 3A (calchart_cmd)**: Likely ~70% — parsing, compiling, and generating show data in C++ may not be parallelized
- **Phase 3B (comparison)**: Likely ~20% — regex operations can be optimized
- **Phase 3C (overhead)**: Likely ~10% — multiprocessing startup is minimal relative to work

---

#### **Phase 3: Optimization Implementations**
Implement optimizations in priority order (Phase 2 determines priority):

##### **Phase 3A: Multi-thread calchart_cmd** (if execution dominates)
Target: Parallelize parsing, compilation, and generation phases within `calchart_cmd`

**Key opportunities**:
- Sheet-level parallelism: Parse & process sheets independently with thread pool
- Output-format parallelism: Generate multiple output types (print, dump, JSON, PostScript) in parallel
- Minimize lock contention on shared show data

**Impact**: Potential 2-4x speedup (depends on number of sheets and cores)

**Complexity**: Medium-High (requires C++20 threading, careful synchronization)

**Files**:
- `tools/calchart_cmd/main.cpp` — PrintToPS() and Parse() functions
- `tools/calchart_cmd/calchart_cmd_parse.hpp` — Parse() implementation
- Core library in `src/core/` — parallelization points in parsing/compilation

---

##### **Phase 3B: Optimize String Comparison** (if comparison dominates)
Target: Speed up regex matching and tolerance comparisons

**Quick wins** (high ROI, low effort):
1. Pre-compile regex patterns (one-time cost, reuse in loops)
2. Fast-path for non-numeric lines (check startswith("pt ") before regex)
3. Skip regex if line doesn't match expected pattern
4. Cache tolerance value (avoid repeated computation)

**Expected impact**: 2-3x speedup in comparison phase (if comparison is bottleneck)

**Complexity**: Low (Python string/regex optimization)

**Files**:
- `resources/tests/sanity_tester.py` — `extractValues()`, `custom_comparison_function()`, `compare_files()`

---

##### **Phase 3C: Optimize Test Harness** (if overhead dominates)
Target: Reduce Python overhead and process management costs

**Improvements**:
1. Use `ProcessPoolExecutor` instead of manual process management
2. Batch per-file operations to reduce Python overhead per operation
3. Tune process pool size based on Phase 2 measurements
4. Reduce number of variant commands per file if startup overhead is high

**Expected impact**: 1.2-1.5x speedup in overhead phase

**Complexity**: Low (Python refactoring)

**Files**:
- `resources/tests/sanity_tester.py` — `main()` function

---

## Implementation Timeline

| Phase | Activity | Effort | Dependencies |
|-------|----------|--------|--------------|
| **Phase 1** | Add instrumentation to sanity_tester.py | 1-2 hrs | None |
| **Phase 2** | Run instrumented tests, analyze metrics | 30 min | Phase 1 complete |
| **Phase 3A** | Multi-thread calchart_cmd (if needed) | 2-4 hrs | Phase 2 analysis |
| **Phase 3B** | Optimize string comparison (if needed) | 30 min - 1 hr | Phase 2 analysis |
| **Phase 3C** | Optimize test harness (if needed) | 1-2 hrs | Phase 2 analysis |

**Total**: 3-9 hours depending on findings

---

## Success Metrics

- ✓ Sanity tests run 2-3x faster in debug mode
- ✓ Sanity tests run 1.5-2x faster in release mode
- ✓ All test validations pass (same accuracy)
- ✓ No false positives/negatives in comparisons
- ✓ Metrics are clearly visible for future optimization work

---

## Document References

For detailed implementation guidance, see:

1. **[PERFORMANCE_IMPROVEMENT_PLAN.md](PERFORMANCE_IMPROVEMENT_PLAN.md)**
   - Complete breakdown of bottlenecks and optimization strategies
   - Detailed implementation approach for each phase
   - Success criteria and validation methods

2. **[PHASE_1_IMPLEMENTATION.md](PHASE_1_IMPLEMENTATION.md)**
   - Step-by-step code changes for Phase 1 instrumentation
   - Data structures for metrics collection
   - Report generation examples
   - Testing instructions

---

## Recommended Next Steps

1. **Start with Phase 1** (today): Implement instrumentation in `resources/tests/sanity_tester.py`
2. **Run instrumented tests**: Execute on your typical test suite
3. **Analyze metrics**: Review the output to identify bottleneck
4. **Prioritize Phase 3**: Based on analysis, tackle the highest-impact phase first
5. **Validate & iterate**: Re-run tests, measure improvement, identify next bottleneck

This data-driven approach ensures that optimization effort is focused on the actual limiting factor, not on guesses.

---

## Questions & Considerations

### Q: Should we parallelize everything at once?
**A**: No. Start with Phase 1 to measure. Phase 3 options may have conflicting benefits (e.g., more Python multiprocessing might hurt if calchart_cmd is already slow). Data guides decisions.

### Q: Why not just multi-thread calchart_cmd immediately?
**A**: C++ threading adds complexity and potential bugs. If calchart_cmd is only 20% of the time, threading won't help. If it's 90%, we should do it. Phase 1 tells us.

### Q: Can we test on different machines?
**A**: Yes! Run Phase 1 metrics on both:
- Your local dev machine (important for iteration speed)
- CI environment (important for overall test pipeline)
The bottlenecks might differ, so both perspectives are valuable.

### Q: How do we prevent regression?
**A**: Keep Phase 1 instrumentation in the code. It's low-overhead and provides continuous visibility into test performance. Flag any optimization that significantly increases any category's time.

---

## Contact & Questions

Refer to the detailed plan documents for specific implementation guidance. This is a living plan—adjust priorities based on Phase 2 findings.

