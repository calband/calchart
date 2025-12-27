# Phase 1: Sanity Tester Instrumentation - Implementation Guide

## Objectives
1. Add precise timing measurements to identify performance bottlenecks
2. Generate a detailed report showing time distribution
3. Measure in both debug and release modes
4. Support optional detailed metrics export

## Implementation Details

### Changes to `resources/tests/sanity_tester.py`

#### 1. Import Additional Modules
```python
import time
import sys
import json
from dataclasses import dataclass, asdict, field
from typing import Dict, List
from datetime import datetime
```

#### 2. Add Metrics Data Structures
```python
@dataclass
class CommandMetrics:
    """Metrics for a single command execution"""
    command_type: str  # 'print', 'dump', 'check', 'animate', 'json', 'ps'
    file_name: str
    execution_time: float
    
@dataclass
class ComparisonMetrics:
    """Metrics for file comparison"""
    file_pair: str  # "file1 vs file2"
    comparison_time: float
    regex_match_count: int
    regex_execution_time: float
    whole_line_comparison_count: int
    tolerance_check_count: int
    errors_found: int

@dataclass
class PhaseMetrics:
    """Aggregate metrics by phase"""
    phase_name: str  # 'execution', 'comparison', 'overhead'
    total_time: float
    percentage: float
    operation_count: int
    avg_time_per_op: float

@dataclass
class TestRunMetrics:
    """Complete metrics for one test run"""
    start_time: str
    total_execution_time: float
    command_executions: List[CommandMetrics] = field(default_factory=list)
    comparisons: List[ComparisonMetrics] = field(default_factory=list)
    phase_summary: List[PhaseMetrics] = field(default_factory=list)
    files_processed: int = 0
    errors_encountered: int = 0
```

#### 3. Add Timing Context Manager
```python
class TimedBlock:
    """Context manager for timing code blocks"""
    def __init__(self, label: str = ""):
        self.label = label
        self.start_time = None
        self.elapsed = 0
        
    def __enter__(self):
        self.start_time = time.perf_counter()
        return self
        
    def __exit__(self, *args):
        self.elapsed = time.perf_counter() - self.start_time
        if Debug:
            print(f"[TIMING] {self.label}: {self.elapsed:.3f}s")
```

#### 4. Modify `run_command()` with Timing
```python
def run_command(command_location, file, output_file, option, extension, metrics):
    """Run command with timing instrumentation"""
    command = [command_location, "parse", option, file]
    dir_name = os.path.dirname(file) + "/"
    output_name = output_file + "." + extension
    
    if Debug:
        print("{} > {}".format(command, output_name))
    
    with TimedBlock(f"run_command {extension}") as timer:
        with open(output_name, 'w') as output:
            result = subprocess.run(command, capture_output=True, text=True)
            output.write(filter_output(result.stdout, dir_name))
    
    metrics.command_executions.append(CommandMetrics(
        command_type=extension,
        file_name=os.path.basename(file),
        execution_time=timer.elapsed
    ))
```

#### 5. Modify `run_command_ps()` with Timing
```python
def run_command_ps(command_location, file, output_file, option, extension, metrics):
    """Run PostScript command with timing instrumentation"""
    dir_name = os.path.dirname(file) + "/"
    output_name = output_file + "." + extension
    command = [command_location, "print_to_postscript", option, file, output_name]
    
    if Debug:
        print("{} > {}".format(command, output_name))
    
    with TimedBlock(f"run_command_ps {extension}") as timer:
        subprocess.run(command, capture_output=True, text=True)
    
    metrics.command_executions.append(CommandMetrics(
        command_type=extension,
        file_name=os.path.basename(file),
        execution_time=timer.elapsed
    ))
```

#### 6. Enhance Comparison Functions with Metrics
```python
def compare_files(file1, file2, custom_comparison_function, metrics=None):
    """Compare files with detailed metrics"""
    num_errors = 0
    line_num = 0
    regex_match_attempts = 0
    whole_line_comparisons = 0
    tolerance_checks = 0
    
    with TimedBlock(f"compare {os.path.basename(file1)} vs {os.path.basename(file2)}") as timer:
        with open(file1, 'r') as f1, open(file2, 'r') as f2:
            line_count1 = sum(1 for line in f1)
            line_count2 = sum(1 for line in f2)
            
            if (line_count1 != line_count2):
                print(f"Failed.  Lines {file1}: {line_count1}, {file2}: {line_count2}")
                num_errors = 1
            else:
                f1.seek(0)
                f2.seek(0)
                for line1, line2 in zip(f1, f2):
                    line_num = line_num + 1
                    # Track which comparison path is used
                    line1_stripped = line1.strip()
                    if line1_stripped.startswith("pt "):
                        regex_match_attempts += 1
                        tolerance_checks += 1
                    else:
                        whole_line_comparisons += 1
                    
                    if not custom_comparison_function(line1, line2):
                        print(f"Failed.  Line {line_num} \"{line1}\" \"{line2}\"")
                        num_errors = num_errors + 1
    
    if metrics:
        metrics.comparisons.append(ComparisonMetrics(
            file_pair=f"{os.path.basename(file1)} vs {os.path.basename(file2)}",
            comparison_time=timer.elapsed,
            regex_match_count=regex_match_attempts,
            regex_execution_time=0,  # Could be refined further
            whole_line_comparison_count=whole_line_comparisons,
            tolerance_check_count=tolerance_checks,
            errors_found=num_errors
        ))
    
    return num_errors
```

#### 7. Update `compare_directories()` to Pass Metrics
```python
def compare_directories(dir1, dir2, metrics=None):
    """Compare directories with metrics tracking"""
    if Debug:
        print(f"comparing dirs {dir1} with {dir2}")
    for root, _, files in os.walk(dir1):
        for file in files:
            file1 = os.path.join(root, file)
            file2 = os.path.join(dir2, os.path.relpath(file1, dir1))
            if Debug:
                print(f"comparing files {file1} with {file2}")
            if not os.path.exists(file2):
                print(f"File {file2} does not exist in {dir2}")
                if metrics:
                    metrics.errors_encountered += 1
                return False

            # Compare files using custom comparison function
            if compare_files(file1, file2, custom_comparison_function, metrics) > 0:
                print(f"Differences found between {file1} and {file2}")
                if metrics:
                    metrics.errors_encountered += 1
                return False
    return True
```

#### 8. Add Report Generation Function
```python
def generate_metrics_report(metrics: TestRunMetrics) -> str:
    """Generate human-readable metrics report"""
    report = []
    report.append("\n" + "="*70)
    report.append("SANITY TESTER PERFORMANCE METRICS")
    report.append("="*70)
    report.append(f"Started: {metrics.start_time}")
    report.append(f"Total Time: {metrics.total_execution_time:.2f}s")
    report.append(f"Files Processed: {metrics.files_processed}")
    report.append("")
    
    # Execution statistics
    if metrics.command_executions:
        total_exec_time = sum(cmd.execution_time for cmd in metrics.command_executions)
        report.append("COMMAND EXECUTION SUMMARY:")
        report.append(f"  Total commands run: {len(metrics.command_executions)}")
        report.append(f"  Total execution time: {total_exec_time:.2f}s ({100*total_exec_time/metrics.total_execution_time:.1f}% of total)")
        report.append(f"  Average per command: {total_exec_time/len(metrics.command_executions):.3f}s")
        
        # Group by command type
        cmd_by_type = {}
        for cmd in metrics.command_executions:
            if cmd.command_type not in cmd_by_type:
                cmd_by_type[cmd.command_type] = []
            cmd_by_type[cmd.command_type].append(cmd.execution_time)
        
        for cmd_type in sorted(cmd_by_type.keys()):
            times = cmd_by_type[cmd_type]
            report.append(f"  {cmd_type:15} x{len(times):3}: {sum(times):7.2f}s (avg {sum(times)/len(times):.3f}s)")
        report.append("")
    
    # Comparison statistics
    if metrics.comparisons:
        total_comp_time = sum(cmp.comparison_time for cmp in metrics.comparisons)
        report.append("FILE COMPARISON SUMMARY:")
        report.append(f"  Total comparisons: {len(metrics.comparisons)}")
        report.append(f"  Total comparison time: {total_comp_time:.2f}s ({100*total_comp_time/metrics.total_execution_time:.1f}% of total)")
        report.append(f"  Average per comparison: {total_comp_time/len(metrics.comparisons):.3f}s")
        
        total_regex = sum(cmp.regex_match_count for cmp in metrics.comparisons)
        total_whole = sum(cmp.whole_line_comparison_count for cmp in metrics.comparisons)
        report.append(f"  Regex-based comparisons: {total_regex}")
        report.append(f"  Direct line comparisons: {total_whole}")
        report.append("")
    
    # Phase breakdown (estimated)
    execution_time = sum(cmd.execution_time for cmd in metrics.command_executions)
    comparison_time = sum(cmp.comparison_time for cmp in metrics.comparisons)
    overhead_time = metrics.total_execution_time - execution_time - comparison_time
    
    report.append("TIME BREAKDOWN:")
    report.append(f"  Command execution: {execution_time:7.2f}s ({100*execution_time/metrics.total_execution_time:5.1f}%)")
    report.append(f"  File comparison:   {comparison_time:7.2f}s ({100*comparison_time/metrics.total_execution_time:5.1f}%)")
    report.append(f"  Overhead (I/O etc): {overhead_time:7.2f}s ({100*overhead_time/metrics.total_execution_time:5.1f}%)")
    
    if metrics.errors_encountered > 0:
        report.append("")
        report.append(f"ERRORS: {metrics.errors_encountered}")
    
    report.append("="*70 + "\n")
    return "\n".join(report)

def export_metrics_json(metrics: TestRunMetrics, filename: str):
    """Export metrics to JSON for further analysis"""
    data = {
        'start_time': metrics.start_time,
        'total_execution_time': metrics.total_execution_time,
        'files_processed': metrics.files_processed,
        'errors_encountered': metrics.errors_encountered,
        'command_executions': [asdict(cmd) for cmd in metrics.command_executions],
        'comparisons': [asdict(cmp) for cmp in metrics.comparisons],
    }
    with open(filename, 'w') as f:
        json.dump(data, f, indent=2)
    print(f"Metrics exported to {filename}")
```

#### 9. Update `main()` to Use Instrumentation
```python
def main():
    args = parse_arguments()
    
    # Initialize metrics
    metrics = TestRunMetrics(
        start_time=datetime.now().isoformat(),
        total_execution_time=0
    )
    
    overall_start = time.perf_counter()
    
    # Create a temporary directory for output if none is specified
    output_directory = args.output if args.output is not None else tempfile.mkdtemp()

    # what command to run
    calchart_cmd = args.calchart_cmd if args.calchart_cmd is not None else "./build/tools/calchart_cmd/calchart_cmd"

    # Number of CPU cores
    num_cores = multiprocessing.cpu_count() * 2

    # Process each file
    processes = []
    for root, _, files in os.walk(args.target_directory):
        for file in files:
            if file.endswith('.shw'):
                file_path = os.path.join(root, file)
                output_dir = os.path.join(output_directory, os.path.relpath(root, args.target_directory))
                os.makedirs(output_dir, exist_ok=True)
                output_file = os.path.join(output_dir, os.path.splitext(file)[0] + '.output')
                
                metrics.files_processed += 1

                # Run the command with different combinations of flags asynchronously
                for [options, extension] in [["--print_show", "print"], ["--dump_continuity", "dump"], ["--check_flag", "check"], ["--animate_show", "animate"], ["--json", "json"]]:
                    process = multiprocessing.Process(target=run_command, args=(calchart_cmd, file_path, output_file, options, extension, metrics))
                    process.start()
                    processes.append(process)

                # PS version Run the command with different combinations of flags asynchronously
                for [options, extension] in [["", "normal.ps"], ["--landscape", "landscape.ps"], ["--cont", "cont.ps"], ["--contsheet", "contsheet.ps"], ["--overview", "overview.ps"]]:
                    process = multiprocessing.Process(target=run_command_ps, args=(calchart_cmd, file_path, output_file, options, extension, metrics))
                    process.start()
                    processes.append(process)

            # once we have started a bunch of jobs, see if we should drain them before the next batch.
            # Limit the number of concurrent jobs to the number of CPU cores
            if len(processes) >= num_cores:
                for process in processes:
                    process.join()
                processes = []

    # Wait for remaining processes to finish
    for process in processes:
        process.join()

    if args.gold_zip is not None:
        with TimedBlock("Compare against gold"):
            check_against_gold(args.gold_zip, output_directory, metrics)

    metrics.total_execution_time = time.perf_counter() - overall_start
    
    # Print and optionally export metrics
    print(generate_metrics_report(metrics))
    
    # Export JSON if verbose/debug mode
    if Debug or os.environ.get('EXPORT_METRICS'):
        export_metrics_json(metrics, "sanity_test_metrics.json")

    # Cleanup
    if args.output is None:
        shutil.rmtree(output_directory)
```

### Notes on Implementation

1. **Multiprocessing challenge**: The metrics object cannot be directly shared between processes. Options:
   - Use `multiprocessing.Manager()` for shared data structures (slightly slower)
   - Collect metrics in-process and merge at end (simpler, good for now)
   - Use a queue to collect results from worker processes

2. **Recommendation**: Start with in-process timing (easier to implement), defer multiprocessing metrics collection for Phase 2 if needed.

3. **Simple alternative** if multiprocessing.Manager is complex:
   - Collect metrics in each process to a temporary file
   - Main process reads and aggregates after all workers complete

### Testing the Instrumentation

```bash
# Build in debug mode
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug

# Run with metrics (enables debug output and JSON export)
DEBUG=1 EXPORT_METRICS=1 python3 resources/tests/sanity_tester.py \
    -d shows \
    -c ./build/tools/calchart_cmd/calchart_cmd

# Review output: look for the metrics report and sanity_test_metrics.json
```

### Expected Output Example
```
======================================================================
SANITY TESTER PERFORMANCE METRICS
======================================================================
Started: 2025-12-26T10:30:45.123456
Total Time: 45.67s
Files Processed: 12

COMMAND EXECUTION SUMMARY:
  Total commands run: 120
  Total execution time: 40.23s (88.1% of total)
  Average per command: 0.335s
  print          x 12: 4.82s (avg 0.402s)
  dump           x 12: 5.21s (avg 0.434s)
  check          x 12: 4.56s (avg 0.380s)
  animate        x 12: 8.34s (avg 0.695s)
  json           x 12: 6.12s (avg 0.510s)
  normal.ps      x 12: 4.87s (avg 0.406s)
  ...

FILE COMPARISON SUMMARY:
  Total comparisons: 120
  Total comparison time: 3.21s (7.0% of total)
  Average per comparison: 0.027s
  Regex-based comparisons: 2450
  Direct line comparisons: 8920

TIME BREAKDOWN:
  Command execution: 40.23s (88.1%)
  File comparison:    3.21s (7.0%)
  Overhead (I/O etc): 2.23s (4.9%)
======================================================================
```

This gives you clear visibility into where time is spent, enabling informed decisions for Phase 3 optimizations.
