# CalChart Sanity Tester Performance Optimization - Complete Guide Index

## 📋 Overview

The CalChart sanity validation tests are slow, especially in debug mode. This guide provides a **data-driven, phased approach** to identify and fix the bottlenecks.

**Key insight**: Don't guess where time is spent—measure first, then optimize strategically.

---

## 🚀 Quick Start (TL;DR)

**If you have 2 minutes:**
→ Read [QUICK_REFERENCE.md](QUICK_REFERENCE.md)

**If you have 5 minutes:**
→ Read [PERFORMANCE_OPTIMIZATION_SUMMARY.md](PERFORMANCE_OPTIMIZATION_SUMMARY.md)

**If you have 30 minutes:**
→ Read [VISUAL_WORKFLOW.md](VISUAL_WORKFLOW.md)

---

## 📚 Complete Documentation

### 1. [QUICK_REFERENCE.md](QUICK_REFERENCE.md) — The 5-Minute Overview
**What it covers:**
- Problem statement (tests are slow)
- Solution approach (3 phases)
- Quick decision tree
- FAQ

**Best for**: Getting oriented quickly, understanding the approach

---

### 2. [PERFORMANCE_OPTIMIZATION_SUMMARY.md](PERFORMANCE_OPTIMIZATION_SUMMARY.md) — Executive Summary
**What it covers:**
- Problem statement with context
- Three-phase solution overview
- Timeline and effort estimates
- Success metrics
- Contact information

**Best for**: Planning the work, estimating time/effort

---

### 3. [VISUAL_WORKFLOW.md](VISUAL_WORKFLOW.md) — Visual Guides & Diagrams
**What it covers:**
- Decision tree flowchart
- Phase-by-phase visual comparisons
- Before/after timelines
- Testing strategy diagrams
- Code change summaries

**Best for**: Understanding the workflow visually, discussing with team

---

### 4. [PERFORMANCE_IMPROVEMENT_PLAN.md](PERFORMANCE_IMPROVEMENT_PLAN.md) — Detailed Strategy
**What it covers:**
- Current state analysis
- Four phases: Instrumentation, Analysis, and three optimization options
- Detailed implementation approach for each phase
- Integration points and dependencies
- Files to modify and examples

**Best for**: Technical deep dive, planning implementation

---

### 5. [PHASE_1_IMPLEMENTATION.md](PHASE_1_IMPLEMENTATION.md) — Step-by-Step Code Changes
**What it covers:**
- Exact Python code to add to `sanity_tester.py`
- Metrics data structures (dataclasses)
- Timing instrumentation code
- Report generation functions
- Testing instructions

**Best for**: Implementing Phase 1 (instrumentation)

---

## 🎯 Three-Phase Approach

```
Phase 1: MEASURE
↓ Add timing instrumentation (1-2 hours)
↓ Identify actual bottleneck

Phase 2: ANALYZE
↓ Run instrumented tests (30 min)
↓ Determine which phase to optimize

Phase 3: OPTIMIZE
↓ Choose from Phase 3A, 3B, or 3C based on findings
↓ Implement, test, measure improvement
↓ Repeat Phase 3 if needed for additional gains
```

---

## 📈 Optimization Options

After Phase 1 & 2, choose from:

| Option | If... | Impact | Effort | File(s) |
|--------|-------|--------|--------|---------|
| **Phase 3A** | calchart_cmd execution is 80%+ of time | 2-4x | Medium | `tools/calchart_cmd/` |
| **Phase 3B** | File comparison is 20%+ of time | 2-3x | Low | `resources/tests/` |
| **Phase 3C** | Python overhead is 15%+ of time | 1.2-1.5x | Low | `resources/tests/` |

---

## 📋 Implementation Checklist

```
Phase 1: Instrumentation
 ☐ Read PHASE_1_IMPLEMENTATION.md
 ☐ Add timing imports to sanity_tester.py
 ☐ Create metrics data structures
 ☐ Add TimedBlock context manager
 ☐ Instrument run_command() and run_command_ps()
 ☐ Instrument comparison functions
 ☐ Add report generation
 ☐ Test instrumentation
 ☐ Commit: "Add performance instrumentation"

Phase 2: Analysis
 ☐ Run: python3 resources/tests/sanity_tester.py -d shows ...
 ☐ Note: total time and phase breakdown
 ☐ Identify bottleneck (exec%, comp%, overhead%)
 ☐ Choose Phase 3A/3B/3C based on findings

Phase 3A (if execution is bottleneck):
 ☐ Read PERFORMANCE_IMPROVEMENT_PLAN.md section "Phase 3A"
 ☐ Profile calchart_cmd execution
 ☐ Implement C++20 threading in calchart_cmd
 ☐ Rebuild and test
 ☐ Re-run Phase 1 to measure improvement
 ☐ Commit: "Phase 3A: Multi-thread calchart_cmd"

Phase 3B (if comparison is bottleneck):
 ☐ Read PERFORMANCE_IMPROVEMENT_PLAN.md section "Phase 3B"
 ☐ Pre-compile regex patterns
 ☐ Add fast-path checks
 ☐ Optimize extractValues() and compare_files()
 ☐ Re-run Phase 1 to measure improvement
 ☐ Commit: "Phase 3B: Optimize string comparison"

Phase 3C (if overhead is bottleneck):
 ☐ Read PERFORMANCE_IMPROVEMENT_PLAN.md section "Phase 3C"
 ☐ Replace manual Process with ProcessPoolExecutor
 ☐ Optimize batch sizes and pool configuration
 ☐ Re-run Phase 1 to measure improvement
 ☐ Commit: "Phase 3C: Optimize test harness"

Validation:
 ☐ All tests pass
 ☐ Metrics show 2-3x speedup in debug mode
 ☐ Metrics show 1.5-2x speedup in release mode
 ☐ Re-run full test suite to ensure correctness
```

---

## 🔍 Key Files Modified

### Phase 1 (Always Required)
- **`resources/tests/sanity_tester.py`** ← Add instrumentation

### Phase 3A (If calchart_cmd is bottleneck)
- **`tools/calchart_cmd/main.cpp`** ← Add threading
- **`tools/calchart_cmd/calchart_cmd_parse.hpp`** ← Parallelize execution
- **`src/core/*`** ← Potentially add parallelization points

### Phase 3B (If comparison is bottleneck)
- **`resources/tests/sanity_tester.py`** ← Optimize regex

### Phase 3C (If overhead is bottleneck)
- **`resources/tests/sanity_tester.py`** ← Optimize process management

---

## 🧪 Testing & Validation

### Before Optimization
```bash
# Configure & build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug

# Run instrumented tests (baseline)
python3 resources/tests/sanity_tester.py \
    -d shows \
    -c ./build/tools/calchart_cmd/calchart_cmd
```

**Save**: Total time and metrics breakdown

### After Phase 3X Implementation
```bash
# Rebuild with changes
cmake --build build --config Debug

# Re-run instrumented tests
python3 resources/tests/sanity_tester.py \
    -d shows \
    -c ./build/tools/calchart_cmd/calchart_cmd
```

**Compare**: Before vs. after metrics. Look for speedup in the optimized phase.

---

## 💡 Success Criteria

After completing Phase 1 + selected Phase 3X:

✓ **Debug mode**: 2-3x faster (example: 60s → 20-30s)
✓ **Release mode**: 1.5-2x faster
✓ **All tests pass** with same accuracy
✓ **No regressions** in validation results
✓ **Metrics remain visible** for future optimization work

---

## 📞 Questions & Troubleshooting

### Q: Where do I start?
**A**: Read [QUICK_REFERENCE.md](QUICK_REFERENCE.md), then [PHASE_1_IMPLEMENTATION.md](PHASE_1_IMPLEMENTATION.md)

### Q: How long will this take?
**A**: 
- Phase 1: 1-2 hours (instrumentation)
- Phase 2: 30 minutes (analysis)
- Phase 3X: 30 minutes - 4 hours (depends on bottleneck)
- **Total: 2-7 hours** depending on findings

### Q: Should I do all phases?
**A**: No. Do Phase 1, measure, do Phase 2, pick one Phase 3, optimize, measure again. Only add more phases if still too slow.

### Q: What if I'm wrong about the bottleneck?
**A**: Phase 1 tells you exactly. Trust the metrics, not guesses.

### Q: Can I test locally without full test suite?
**A**: Yes. Use a subset of show files with `python3 resources/tests/sanity_tester.py -d <your_test_dir>`

---

## 📁 Complete Document Map

```
/private/tmp/dev/calchart.git/
├── QUICK_REFERENCE.md ......................... Start here (5 min)
├── PERFORMANCE_OPTIMIZATION_SUMMARY.md ....... Executive overview (10 min)
├── VISUAL_WORKFLOW.md ........................ Visual guides & diagrams (15 min)
├── PERFORMANCE_IMPROVEMENT_PLAN.md ........... Detailed strategy (30 min)
└── PHASE_1_IMPLEMENTATION.md ................. Step-by-step code (implement)

Plus existing project files:
├── resources/tests/sanity_tester.py ......... File to modify
├── tools/calchart_cmd/main.cpp .............. File to modify (Phase 3A)
├── tools/calchart_cmd/calchart_cmd_parse.hpp  File to modify (Phase 3A)
└── src/core/ ............................... Potentially modified (Phase 3A)
```

---

## 🔄 Workflow Summary

1. **Understand** → Read [QUICK_REFERENCE.md](QUICK_REFERENCE.md)
2. **Plan** → Read [PERFORMANCE_OPTIMIZATION_SUMMARY.md](PERFORMANCE_OPTIMIZATION_SUMMARY.md)
3. **Visualize** → Review [VISUAL_WORKFLOW.md](VISUAL_WORKFLOW.md) diagrams
4. **Implement Phase 1** → Follow [PHASE_1_IMPLEMENTATION.md](PHASE_1_IMPLEMENTATION.md)
5. **Measure & Analyze** → Run tests, read metrics, decide Phase 3X
6. **Deep Dive** → Read [PERFORMANCE_IMPROVEMENT_PLAN.md](PERFORMANCE_IMPROVEMENT_PLAN.md) Phase 3X section
7. **Implement Phase 3X** → Code changes based on plan
8. **Validate** → Re-measure, confirm speedup

---

## 🎓 Key Principles

1. **Measure before optimizing** — Avoid wasting effort on non-bottlenecks
2. **One phase at a time** — Understand impact of each change
3. **Keep instrumentation** — Metrics are valuable for future work
4. **Data-driven decisions** — Trust Phase 1 output, not guesses
5. **Iterative improvement** — Phase 1 + 3A is better than Phase 1 + 3A + 3B + 3C if 3A solves it

---

## ✅ Next Steps

**Right now:**
1. Read [QUICK_REFERENCE.md](QUICK_REFERENCE.md) (5 minutes)

**Then:**
2. Read [PHASE_1_IMPLEMENTATION.md](PHASE_1_IMPLEMENTATION.md) (10 minutes)

**Then:**
3. Implement Phase 1 instrumentation in `resources/tests/sanity_tester.py` (1-2 hours)

**Then:**
4. Run tests and analyze metrics (30 minutes)

**Then:**
5. Implement Phase 3X based on findings (30 min - 4 hours)

---

**Questions?** Refer to the specific guide sections above. This is a complete, self-contained optimization plan with all the information needed to succeed.

Good luck! 🚀

