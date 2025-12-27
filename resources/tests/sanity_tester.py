#!/usr/bin/env python3

import argparse
import os
import subprocess
import shutil
import tempfile
import zipfile
import multiprocessing
import time
import json
from dataclasses import dataclass, asdict, field
from datetime import datetime
from typing import List

Description="""
Sanity Tester for CalChart.
Example: How to compare with a gold version:
sanity_tester.py -d shows -g gold.zip

Example: How to generate new gold results
sanity_tester.py -d shows -o gold
zip -r gold.zip gold
"""

Debug = False

# ============================================================================
# METRICS COLLECTION DATA STRUCTURES
# ============================================================================

@dataclass
class CommandMetrics:
    """Metrics for a single command execution"""
    command_type: str
    file_name: str
    execution_time: float

@dataclass
class ComparisonMetrics:
    """Metrics for file comparison"""
    file_pair: str
    comparison_time: float
    regex_match_count: int
    whole_line_comparison_count: int
    errors_found: int

@dataclass
class TestRunMetrics:
    """Complete metrics for one test run"""
    start_time: str
    total_execution_time: float
    command_executions: List[CommandMetrics] = field(default_factory=list)
    comparisons: List[ComparisonMetrics] = field(default_factory=list)
    files_processed: int = 0
    errors_encountered: int = 0

# ============================================================================
# TIMING UTILITIES
# ============================================================================

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

def generate_metrics_report(metrics: TestRunMetrics) -> str:
    """Generate human-readable metrics report"""
    report = []
    report.append("\n" + "="*70)
    report.append("SANITY TESTER PERFORMANCE METRICS")
    report.append("="*70)
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
        'total_execution_time': metrics.total_execution_time,
        'files_processed': metrics.files_processed,
        'errors_encountered': metrics.errors_encountered,
        'command_executions': [asdict(cmd) for cmd in metrics.command_executions],
        'comparisons': [asdict(cmp) for cmp in metrics.comparisons],
    }
    with open(filename, 'w') as f:
        json.dump(data, f, indent=2)
    print(f"Metrics exported to {filename}")


def parse_arguments():
    parser = argparse.ArgumentParser(description=Description)
    parser.add_argument("-d", "--target-directory", required=True, help="The directory of show files to process")
    parser.add_argument("-o", "--output", help="Location to generate results.  If omited, tmp dir is used then deleted at the end.")
    parser.add_argument("-c", "--calchart_cmd", help="The path for calchart_cmd (optional)")
    parser.add_argument("-g", "--gold-zip", help="Specify the location of the gold zip file (optional)")
    return parser.parse_args()

def unzip_gold_output(gold_zip, temp_dir):
    with zipfile.ZipFile(gold_zip, 'r') as zip_ref:
        zip_ref.extractall(temp_dir)

def filter_output(output, string_to_remove):
    return output.replace(string_to_remove, '')

def run_command(command_location, file, output_file, option, extension):
    command = [command_location, "parse", option, file]
    dir_name = os.path.dirname(file) + "/"
    output_name = output_file + "." + extension
    if Debug:
        print("{} > {}".format(command, output_name))
    
    with TimedBlock(f"run_command {extension}") as timer:
        with open(output_name, 'w') as output:
            # we do this to filter out the dir name to allow location agnostic testing
            result = subprocess.run(command, capture_output=True, text=True)
            output.write(filter_output(result.stdout, dir_name))

def run_command_ps(command_location, file, output_file, option, extension):
    dir_name = os.path.dirname(file) + "/"
    output_name = output_file + "." + extension
    command = [command_location, "print_to_postscript", option, file, output_name]
    if Debug:
        print("{} > {}".format(command, output_name))
    
    with TimedBlock(f"run_command_ps {extension}") as timer:
        subprocess.run(command, capture_output=True, text=True)

def compare_files(file1, file2, custom_comparison_function, metrics=None):
    num_errors = 0
    line_num = 0
    regex_match_attempts = 0
    whole_line_comparisons = 0
    
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
                    else:
                        whole_line_comparisons += 1
                    
                    if not custom_comparison_function(line1, line2):
                        print(f"Failed.  Line {line_num} \"{line1}\" \"{line2}\"")
                        num_errors = num_errors + 1
    
    if metrics is not None:
        metrics.comparisons.append(ComparisonMetrics(
            file_pair=f"{os.path.basename(file1)} vs {os.path.basename(file2)}",
            comparison_time=timer.elapsed,
            regex_match_count=regex_match_attempts,
            whole_line_comparison_count=whole_line_comparisons,
            errors_found=num_errors
        ))
    
    return num_errors

def extractTrailing(input_string):
    import re
    # input_string = ".*$" -> 0
    # input_string = ".*, collision!$" -> 1
    pattern = r".*, collision!"
    match = re.match(pattern, input_string)
    if match:
        return 1
    else:
        return 0

def extractValues(input_string):
    import re
    # input_string = "pt 98: (448, 672), dir=-168.7"
    # -> 
    # [98, 448, 672, -168.7, 0]
    # input_string = "pt 98: (448, 672), dir=-168.7, collision!"
    # -> 
    # [98, 448, 672, -168.7, 1]

    pattern = r"pt (\d+): \((-?\d+), (-?\d+)\), dir=(-?\d+(\.\d+)?([eE]-?\d+)?)"
    match = re.match(pattern, input_string)
    if match:
        return [ int(match.group(1)), int(match.group(2)), int(match.group(3)), float(match.group(4)), extractTrailing(input_string) ]
    else:
        return None

def compare_whole_lines(line1, line2):
    if line1.startswith("%%CreationDate: "):
        return line2.startswith("%%CreationDate: ")
    return line1.strip() == line2.strip()

def custom_comparison_function(line1, line2):
    value1 = extractValues(line1.strip())
    value2 = extractValues(line2.strip())
    if value1 is None or value2 is None:
        if Debug:
            print(f"comparing {line1.strip()} and {line2.strip()}")
        return compare_whole_lines(line1, line2)
    # Adjust tolerance as needed
    tolerance = 1e-2
    result = value1[0] == value2[0] and value1[1] == value2[1] and value1[2] == value2[2] and abs(value1[3] - value2[3]) < tolerance and value1[4] == value2[4]
    if Debug:
        print(f"comparing {value1} and {value2} -> {result}")
    return result

def compare_directories(dir1, dir2, metrics=None):
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

def check_against_gold(gold_zip, output_directory, metrics=None):
    gold_dir = tempfile.mkdtemp()

    # Unzip the gold output
    unzip_gold_output(gold_zip, gold_dir)

    # Compare generated results against gold output
    if not compare_directories(output_directory, gold_dir + "/gold", metrics):
        import sys
        sys.exit(1)

    # Cleanup
    shutil.rmtree(gold_dir)

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

    # Time execution phase
    exec_start = time.perf_counter()
    
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
                    process = multiprocessing.Process(target=run_command, args=(calchart_cmd, file_path, output_file, options, extension))
                    process.start()
                    processes.append(process)

                # PS version Run the command with different combinations of flags asynchronously
                for [options, extension] in [["", "normal.ps"], ["--landscape", "landscape.ps"], ["--cont", "cont.ps"], ["--contsheet", "contsheet.ps"], ["--overview", "overview.ps"]]:
                    process = multiprocessing.Process(target=run_command_ps, args=(calchart_cmd, file_path, output_file, options, extension))
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
    
    exec_time = time.perf_counter() - exec_start

    # Time comparison phase
    if args.gold_zip is not None:
        comp_start = time.perf_counter()
        check_against_gold(args.gold_zip, output_directory, metrics)
        comp_time = time.perf_counter() - comp_start
    else:
        comp_time = 0

    metrics.total_execution_time = time.perf_counter() - overall_start
    
    # Calculate and add timing metrics
    overhead_time = metrics.total_execution_time - exec_time - comp_time
    
    # Create a dummy metric for execution tracking
    metrics.command_executions.append(CommandMetrics(
        command_type="[total_execution]",
        file_name="",
        execution_time=exec_time
    ))
    metrics.comparisons.append(ComparisonMetrics(
        file_pair="[total_comparison]",
        comparison_time=comp_time,
        regex_match_count=0,
        whole_line_comparison_count=0,
        errors_found=0
    ))
    
    # Print and optionally export metrics
    print(generate_metrics_report(metrics))
    
    # Export JSON if verbose/debug mode
    if Debug or os.environ.get('EXPORT_METRICS'):
        export_metrics_json(metrics, "sanity_test_metrics.json")

    # Cleanup
    if args.output is None:
        shutil.rmtree(output_directory)

if __name__ == "__main__":
    main()

