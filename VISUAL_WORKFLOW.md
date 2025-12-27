# CalChart Performance Optimization - Visual Workflow

## Decision Tree

```
                        START: Sanity tests are slow
                               /
                              /
                    Phase 1: Instrument Code
                    (Add timing measurements)
                             |
                             ↓
                    Run: python3 resources/tests/sanity_tester.py
                             |
                  ┌──────────┴──────────┐
                  ↓                     ↓
        Look at metrics output   Look for breakdown:
        
        Command execution: X.XXs (YY.Y%)
        File comparison:   X.XXs (YY.Y%)
        Overhead (I/O):    X.XXs (YY.Y%)
        
                  |
        ┌─────────┼─────────┐
        ↓         ↓         ↓
    
    80%+?     20%+?     15%+?
    Exec      Comp      Overhead
    
        ↓         ↓         ↓
    Phase 3A   Phase 3B   Phase 3C
    Multi-     Optimize   Optimize
    thread     Regex      Harness
    cmd                   
    
        ↓         ↓         ↓
    
  Implement  Implement  Implement
  C++ code   Python     Python
  changes    changes    changes
  
        ↓         ↓         ↓
        └─────────┼─────────┘
                  ↓
        Re-run instrumented tests
        Compare before/after metrics
                  ↓
        ┌─────────┴──────────────┐
        ↓                        ↓
    Speedup                  Still slow?
    achieved?          Try next Phase 3
    
    YES↓                    option↓
    ✓ SUCCESS             Loop back &
    Done!                 measure
```

---

## Time Distribution (Current State - Unknown)

We need to measure this with Phase 1:

```
Total sanity_tester runtime: ~45s (example)

Current state (unknown distribution):
█████████████████████████████████████████████░ 45s

Scenarios after Phase 1:

Scenario A (80% exec):
█████████████████████████████████ ██ █ 45s
Command Execution (36s)
File Comparison (2s)
Overhead (7s)
→ Focus: Phase 3A (Multi-thread calchart_cmd)

Scenario B (20% comparison):
█████████████████████ █ ███████████ █ 45s
Command Execution (32s)
File Comparison (9s)
Overhead (4s)
→ Focus: Phase 3B (Optimize regex)

Scenario C (15% overhead):
█████████████████████████ █ ████████ █ 45s
Command Execution (32s)
File Comparison (7s)
Overhead (6s)
→ Focus: Phase 3C (Optimize harness)
```

---

## Phase 3A: Multi-thread calchart_cmd

```
Current (Sequential):
┌────────────┐
│ calchart   │  Sheet 1  ← 4s
│ _cmd       │  Sheet 2  ← 4s
│ _cmd       │  Sheet 3  ← 4s
│ (process)  │  Sheet 4  ← 4s
│            │  ─────────
│            │  Total: 16s
└────────────┘

With Threading (Parallel):
┌────────────────────────┐
│ calchart_cmd (process) │
│  ┌────────────────────┐│
│  │ Thread Pool (4)    ││
│  │ ┌──┬──┬──┬──────┐ ││
│  │ │S1│S2│S3│ wait ││
│  │ └──┴──┴──┴──────┘ ││
│  └────────────────────┘│
│  Total: 4s             │
└────────────────────────┘

Speedup: 4x (with 4 cores)
```

**Key implementation points**:
- Add thread pool to calchart_cmd
- Parallelize sheet processing
- Potentially parallelize output format generation

---

## Phase 3B: Optimize Regex

```
Current (Naive):
for each line:
    result = subprocess.run(complex_regex)  ← 1000x per file
    compare result

Optimized:
compiled_regex = re.compile(pattern)       ← Once at start

for each line:
    if line.startswith("pt "):             ← Fast check first
        value = compiled_regex.match(line)  ← Reuse compiled regex
        compare value
    else:
        direct_string_compare(line)         ← Skip regex for others

Speedup: 2-3x in comparison phase
```

---

## Phase 3C: Optimize Harness

```
Current (Manual Process Management):
processes = []
for file in all_files:
    for variant in 10_variants:
        process = Process(...)
        process.start()
        processes.append(process)
        
        if len(processes) >= num_cores:
            join_all()

Optimized (ProcessPoolExecutor):
with ProcessPoolExecutor(max_workers=num_cores) as executor:
    futures = []
    for file in all_files:
        for variant in 10_variants:
            future = executor.submit(run_command, ...)
            futures.append(future)
    
    wait_all(futures)

Benefits:
- Automatic worker reuse (no per-process startup)
- Better load balancing
- Cleaner code
```

---

## Before & After Timeline

```
Before (Debug Mode):
Build:  5s  ┐
Test:  60s  ├─→ Total: ~70s per iteration (slow!)
           ┘

After optimization (assumed 2.5x speedup):
Build:  5s  ┐
Test:  24s  ├─→ Total: ~30s per iteration (fast!)
           ┘

Time saved per 10-iteration dev session: ~400s = 6-7 minutes! 👍
```

---

## Code Change Summary

```
Phase 1: sanity_tester.py modifications
├── Add imports (time, json, dataclasses)
├── Add metrics data structures (CommandMetrics, ComparisonMetrics, etc.)
├── Add TimedBlock context manager
├── Wrap subprocess.run() calls with timing
├── Wrap compare_files() with metrics collection
├── Add report generation function
└── Modify main() to aggregate & print metrics

Lines of code: ~200-300 lines of instrumentation code

Phase 3A: calchart_cmd modifications (IF NEEDED)
├── tools/calchart_cmd/main.cpp
├── tools/calchart_cmd/calchart_cmd_parse.hpp
└── Potentially src/core/* files for parallelization

Lines of code: ~100-200 lines of C++ threading code

Phase 3B: sanity_tester.py regex optimizations (IF NEEDED)
├── Compile regex at module level
├── Optimize extractValues() with pre-compiled regex
└── Add fast-path checks

Lines of code: ~20-30 lines

Phase 3C: sanity_tester.py harness optimization (IF NEEDED)
├── Replace manual Process with ProcessPoolExecutor
└── Simplify process management

Lines of code: ~20-50 lines
```

---

## Testing Strategy

```
Step 1: Baseline (Before Optimization)
┌──────────────────────────────────────┐
│ Run: python3 resources/tests/       │
│      sanity_tester.py -d shows      │
│      -c ./build/tools/calchart_cmd/ │
│      calchart_cmd                   │
└──────────────────────────────────────┘
        ↓
   Review metrics output
   Note: total time, phase breakdown
   Example: 45.67s total
        ↓
   Save output (metrics.json created)


Step 2: Implement Phase 3X
┌──────────────────────────────────────┐
│ Edit code based on Phase 3 choice    │
│ Rebuild: cmake --build build         │
│ Run tests to verify no regressions   │
└──────────────────────────────────────┘
        ↓


Step 3: Post-Optimization
┌──────────────────────────────────────┐
│ Run: python3 resources/tests/       │
│      sanity_tester.py -d shows      │
│      -c ./build/tools/calchart_cmd/ │
│      calchart_cmd                   │
└──────────────────────────────────────┘
        ↓
   Review metrics output
   Compare: 45.67s → 22.3s? (2.05x)
        ↓
   Success! Or run Phase 3Y if needed


Step 4: Repeat if Necessary
If still too slow:
        ↓
   Identify next bottleneck
   Implement Phase 3Y
   Re-measure
```

---

## Expected Outcomes by Scenario

```
If Phase 1 shows: exec=88%, comp=7%, overhead=5%
→ Phase 3A: Multi-thread calchart_cmd
→ Expected: 45s → 20s (2.25x) ✓ Meets goal!

If Phase 1 shows: exec=60%, comp=25%, overhead=15%
→ Phase 3B: Optimize regex (improves 25% to 8%)
→ Result: 45s → 30s (1.5x)
→ If needed, also do Phase 3A or 3C

If Phase 1 shows: exec=65%, comp=10%, overhead=25%
→ Phase 3C: Optimize harness (reduces overhead)
→ Result: 45s → 35s (1.3x)
→ Likely also need Phase 3A
```

---

## Success Criteria Checklist

```
Before:          Phase 1      Phase 3X      Final
─────────────────────────────────────────────────
Slow (60s)   →  Measured   →  Optimized  →  FAST (20-30s)
                (where?)       (how much?)
                
Tests pass ✓     ✓            ✓ (verify!)     ✓
Metrics         Visible in     (repeat        Still visible
              console & JSON   Phase 1)       in output
                
                              Speedup: 2-3x ✓
```

---

## Development Commit Pattern

```
Commit 1: "Add performance instrumentation to sanity_tester"
─ Phase 1 implementation
─ Shows timing and metrics output

Commit 2: "Phase 3X: [Optimization description]"
─ Specific Phase 3A/3B/3C changes
─ References metrics showing X% speedup

Commit 3 (if needed): "Phase 3Y: [Additional optimization]"
─ If multiple phases needed

Branch: feature/sanity-tester-performance
PR description: 
  - Before: X seconds
  - After: Y seconds (Z% speedup)
  - Metrics output showing breakdown
```

---

## Resources

- [QUICK_REFERENCE.md](QUICK_REFERENCE.md) — TL;DR guide
- [PERFORMANCE_IMPROVEMENT_PLAN.md](PERFORMANCE_IMPROVEMENT_PLAN.md) — Full strategy
- [PHASE_1_IMPLEMENTATION.md](PHASE_1_IMPLEMENTATION.md) — Step-by-step code changes
- [PERFORMANCE_OPTIMIZATION_SUMMARY.md](PERFORMANCE_OPTIMIZATION_SUMMARY.md) — Executive summary

Start here: [QUICK_REFERENCE.md](QUICK_REFERENCE.md)

