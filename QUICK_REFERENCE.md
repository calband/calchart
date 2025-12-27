# CalChart Sanity Tester Performance Optimization - Quick Reference

## Current Problem
```
Sanity tests are slow:
  Debug mode:   🐢🐢🐢 (very slow - wants 2-3x speedup)
  Release mode: 🐢🐢   (slow - wants 1.5-2x speedup)
```

## Root Cause: Unknown
We don't know where time is spent. Three suspects:
1. **calchart_cmd execution** (parsing & generating show data)
2. **String comparison** (regex matching & tolerance checks)
3. **Python overhead** (I/O, process management)

## Solution: Data-Driven Optimization

### Step 1: MEASURE (Phase 1)
Add instrumentation to track where time goes.

**Modified file**: `resources/tests/sanity_tester.py`

```python
# New instrumentation tracks:
- Time per calchart_cmd execution
- Time per file comparison
- Count of regex vs. direct comparisons
- Total breakdown by phase
```

**Output example**:
```
Command execution: 40.23s (88.1%)  ← Time spent in calchart_cmd
File comparison:    3.21s (7.0%)   ← Time spent comparing outputs
Overhead (I/O etc): 2.23s (4.9%)   ← Python & misc overhead
```

If you see 88% in execution, that's where to focus!

---

### Step 2: ANALYZE (Phase 2)
Look at Phase 1 output and identify the bottleneck.

```
Does it look like this?     Then do this next:

Exec: 88%  → Phase 3A: Multi-thread calchart_cmd
Exec: 20%  → Phase 3B: Optimize string comparison
Exec: 15%  → Phase 3C: Optimize test harness
```

---

### Step 3: OPTIMIZE (Phase 3)
Based on Phase 2 findings, pick the highest-impact optimization.

#### **Phase 3A: Multi-thread calchart_cmd** (if execution >> 80%)
**Problem**: calchart_cmd processes sheets sequentially

**Solution**: Parallelize with C++20 threads
- Parse multiple sheets in parallel
- Generate multiple output formats in parallel
- Use thread pool to avoid overhead

**Expected speedup**: 2-4x

**Complexity**: Medium (C++ threading)

**Files**: 
- `tools/calchart_cmd/main.cpp`
- `tools/calchart_cmd/calchart_cmd_parse.hpp`

---

#### **Phase 3B: Optimize Regex** (if comparison >> 20%)
**Problem**: Expensive regex matching in Python

**Solution**: Quick wins
1. Compile regex once, reuse many times
2. Check `startswith("pt ")` before regex
3. Skip regex for non-matching lines

**Expected speedup**: 2-3x in comparison phase

**Complexity**: Low (Python optimization)

**Files**: 
- `resources/tests/sanity_tester.py` → `extractValues()`, `compare_files()`

---

#### **Phase 3C: Optimize Test Harness** (if overhead >> 15%)
**Problem**: Multiprocessing overhead

**Solution**: 
1. Use ProcessPoolExecutor (simpler, more efficient)
2. Batch operations to reduce per-operation overhead
3. Tune pool size based on core count

**Expected speedup**: 1.2-1.5x

**Complexity**: Low (Python refactoring)

**Files**: 
- `resources/tests/sanity_tester.py` → `main()` function

---

## Workflow

```
1. Implement Phase 1
   └─→ Add timing instrumentation
       └─→ Commit: "Add performance metrics to sanity_tester"

2. Run Phase 1
   └─→ python3 resources/tests/sanity_tester.py -d shows -c ./build/tools/calchart_cmd/calchart_cmd
       └─→ Review time breakdown in output
           └─→ Note: sanity_test_metrics.json also created

3. Analyze Phase 1 Results
   └─→ Identify bottleneck (88% execution? 20% comparison? etc.)
       └─→ Pick Phase 3A, 3B, or 3C

4. Implement Phase 3X
   └─→ Make code changes
       └─→ Test and validate
           └─→ Commit: "Optimize sanity_tester with Phase 3X changes"

5. Re-run Phase 1
   └─→ Compare before/after metrics
       └─→ Confirm speedup
           └─→ If not fast enough, pick next Phase 3 option
```

---

## Timeline Estimate

| Phase | Time | Notes |
|-------|------|-------|
| Phase 1 (Measure) | 1-2 hours | Add instrumentation |
| Phase 2 (Analyze) | 30 min | Run tests, read output |
| Phase 3X (Optimize) | 30 min - 4 hours | Depends on which phase needed |
| **Total** | **2-7 hours** | Actual time depends on findings |

---

## Key Files

```
resources/tests/sanity_tester.py      ← Phase 1 & 3B/3C modifications
tools/calchart_cmd/main.cpp           ← Phase 3A modifications
tools/calchart_cmd/calchart_cmd_parse.hpp  ← Phase 3A modifications
```

---

## Success Criteria

After all optimizations:
```
✓ Debug mode:   2-3x faster
✓ Release mode: 1.5-2x faster
✓ All tests pass with same accuracy
✓ No false positives/negatives
✓ Metrics remain visible for future optimization
```

---

## Important Principles

1. **Measure first**: Don't optimize blindly. Phase 1 prevents wasting time on non-bottlenecks.

2. **One phase at a time**: Implement Phase 1, measure, implement Phase 3X, measure again. This lets you see the impact of each change.

3. **Keep instrumentation**: The metrics code is low-overhead. Keep it in the codebase for continuous visibility into performance.

4. **Be conservative with C++**: Phase 3A adds C++ threading complexity. Only do it if Phase 1 shows execution is the bottleneck.

---

## FAQ

**Q: Should I do all three phases?**
A: No. Do Phase 1, measure, and pick the highest-impact phase. After implementing it, re-measure. Only add more phases if needed.

**Q: What if multiple are bottlenecks?**
A: Unlikely, but if so, do them in this order: 3B (easiest) → 3C → 3A (hardest).

**Q: Will multiprocessing in Python interfere with C++ threading?**
A: No, they operate independently. Each calchart_cmd process (spawned by Python) can internally use threads. They don't interfere.

**Q: How do I know if Phase 3X actually helped?**
A: Re-run the instrumented tests. Look at the before/after metrics. The bottleneck phase should show significant improvement.

---

## Next Action

→ Read [PHASE_1_IMPLEMENTATION.md](PHASE_1_IMPLEMENTATION.md) for step-by-step code changes to add instrumentation.

