#!/usr/bin/env python3

import argparse
import json
import math
import os
import subprocess
import shutil
import tempfile
import zipfile
import multiprocessing
from dataclasses import dataclass
from typing import List, Optional

Description="""
Sanity Tester for CalChart.

Examples:
  # Compare with gold version
  sanity_tester.py -d shows -g gold.zip

  # Generate new gold results
  sanity_tester.py -d shows -o gold && cd gold && zip -r ../gold.zip . && cd ..

  # Run only specific test types
  sanity_tester.py -d shows -g gold.zip --filter-test print_show --filter-test animate_show

  # Run tests for a specific show file
  sanity_tester.py -d shows -g gold.zip --filter-show example.shw

  # Run serially (easier for debugging)
  sanity_tester.py -d shows -g gold.zip --serial
"""

Debug = False

# Global variables for tracking failures (initialized in main())
failures_lock = None
failures_list = None

@dataclass
class FailureInfo:
    """Information about a test failure"""
    test_type: str
    show_file: str
    command: List[str]
    error_message: str
    stderr_output: Optional[str] = None
    
    def format_reproduction_command(self) -> str:
        """Format the command for easy reproduction"""
        return ' '.join(f'"{arg}"' if ' ' in arg else arg for arg in self.command)

def parse_arguments():
    parser = argparse.ArgumentParser(description=Description, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-d", "--target-directory", required=True, help="The directory of show files to process")
    parser.add_argument("-o", "--output", help="Location to generate results. If omitted, tmp dir is used then deleted at the end.")
    parser.add_argument("-c", "--calchart_cmd", help="The path for calchart_cmd (optional)")
    parser.add_argument("-g", "--gold-zip", help="Specify the location of the gold zip file (optional)")
    parser.add_argument("--max-diff-lines", type=int, default=10, help="Maximum number of different lines to show per file (default: 10)")
    
    # Filtering options for focused testing
    parser.add_argument("--filter-test", action="append", dest="filter_tests",
                        choices=['print_show', 'dump_continuity', 'animate_show', 'json',
                                'ps_normal', 'ps_landscape', 'ps_cont', 'ps_contsheet', 'ps_overview'],
                        help="Run only specific test types (can be specified multiple times)")
    parser.add_argument("--filter-show", action="append", dest="filter_shows",
                        help="Run only tests for specific show files (can be specified multiple times, matches basename)")
    parser.add_argument("--serial", action="store_true",
                        help="Run tests serially instead of in parallel (easier for debugging)")
    
    return parser.parse_args()

def flatten(lst):
    result = []
    for item in lst:
        if isinstance(item, list):
            result.extend(flatten(item))
        else:
            result.append(item)
    return result

def should_run_test(test_id, show_file, filter_tests, filter_shows):
    """Check if a test should run based on filters"""
    # If no filters specified, run all tests
    if not filter_tests and not filter_shows:
        return True
    
    # Check test type filter
    if filter_tests and test_id not in filter_tests:
        return False
    
    # Check show file filter
    if filter_shows:
        show_basename = os.path.basename(show_file)
        if not any(filter_show in show_basename for filter_show in filter_shows):
            return False
    
    return True

def unzip_gold_output(gold_zip, temp_dir):
    with zipfile.ZipFile(gold_zip, 'r') as zip_ref:
        zip_ref.extractall(temp_dir)

def run_command(command_location, file, option, failures_lock, failures_list):
    command = [command_location, "parse", option, file]
    command = flatten(command)

    dir_name = os.path.dirname(file) + "/"
    if Debug:
        print("{}".format(command))
    try:
        # we do this to filter out the dir name to allow location agnostic testing
        result = subprocess.run(command)
            
        # If command failed, track the error
        if result.returncode != 0:
            with failures_lock:
                failures_list.append({
                    'test_type': f"parse {option}",
                    'show_file': file,
                    'command': command,
                    'error_message': f"Command exited with code {result.returncode}",
                    'stderr_output': result.stderr if result.stderr else None
                })
    except Exception as e:
        with failures_lock:
            failures_list.append({
                'test_type': f"parse {option}",
                'show_file': file,
                'command': command,
                'error_message': f"Exception running command: {str(e)}",
                'stderr_output': None
            })

def try_load_json(path):
    try:
        with open(path) as f:
            return json.load(f)
    except (json.JSONDecodeError, UnicodeDecodeError, OSError):
        return None

class JsonCompareError(AssertionError):
    """Raised when two JSON structures differ beyond tolerance."""
    pass


def compare_json(a, b, path="root", rel_tol=1e-9, abs_tol=1e-9,
                  exclude_keys=None, errors=None):
    """
    Recursively compare two JSON-loaded structures.
    Collects all differences (rather than stopping at the first) into `errors`.
    Returns the list of error strings.
    """
    if errors is None:
        errors = []
    exclude_keys = exclude_keys or set()

    if isinstance(a, dict) and isinstance(b, dict):
        keys_a, keys_b = set(a.keys()), set(b.keys())
        only_a = keys_a - keys_b
        only_b = keys_b - keys_a
        if only_a:
            errors.append(f"{path}: keys only in first file: {sorted(only_a)}")
        if only_b:
            errors.append(f"{path}: keys only in second file: {sorted(only_b)}")
        for key in sorted(keys_a & keys_b):
            if key in exclude_keys:
                continue
            compare_json(a[key], b[key], f"{path}.{key}", rel_tol, abs_tol,
                         exclude_keys, errors)

    elif isinstance(a, list) and isinstance(b, list):
        if len(a) != len(b):
            errors.append(f"{path}: length mismatch ({len(a)} vs {len(b)})")
        else:
            for i, (x, y) in enumerate(zip(a, b)):
                compare_json(x, y, f"{path}[{i}]", rel_tol, abs_tol,
                             exclude_keys, errors)

    elif isinstance(a, (int, float)) and isinstance(b, (int, float)):
        if isinstance(a, bool) != isinstance(b, bool):
            # avoid True == 1 comparing as numerically equal when types should match
            errors.append(f"{path}: type mismatch: {a!r} vs {b!r}")
        elif not math.isclose(a, b, rel_tol=rel_tol, abs_tol=abs_tol):
            errors.append(f"{path}: value mismatch: {a} != {b}")

    else:
        if type(a) != type(b):
            errors.append(f"{path}: type mismatch: {type(a).__name__} vs {type(b).__name__}")
        elif a != b:
            errors.append(f"{path}: value mismatch: {a!r} != {b!r}")

    return errors


def assert_json_files_equal(path_a, path_b, rel_tol=1e-9, abs_tol=1e-9, exclude_keys=None):
    a = load_json(path_a)
    b = load_json(path_b)
    errors = compare_json(a, b, rel_tol=rel_tol, abs_tol=abs_tol, exclude_keys=exclude_keys)
    if errors:
        raise JsonCompareError(
            f"JSON mismatch between {path_a} and {path_b}:\n" + "\n".join(errors)
        )
        
def compare_json_files(json1, json2, rel_tol=1e-9, abs_tol=1e-9, exclude_keys=None):
    num_errors = 0
    error_details = []
    errors = compare_json(json1, json2, rel_tol=rel_tol, abs_tol=abs_tol, exclude_keys=exclude_keys)
    if errors:
        error_details.append(f"Error comparing files: {errors}")
        return (1, error_details)
    return (num_errors, error_details)

def compare_files(file1, file2, custom_comparison_function, max_diff_lines=10):
    """Compare two files and return a tuple of (num_errors, error_details)"""
    num_errors = 0
    line_num = 0
    error_details = []
    
    json_a = try_load_json(file1)
    json_b = try_load_json(file2)

    if (json_a is None) != (json_b is None):
        error_details.append(f"One file is valid JSON and the other isn't")
        return (1, error_details)

    if json_a is not None:
        return compare_json_files(json_a, json_b)

    try:
        with open(file1, 'r') as f1, open(file2, 'r') as f2:
            f1_lines = f1.readlines()
            f2_lines = f2.readlines()
            line_count1 = len(f1_lines)
            line_count2 = len(f2_lines)
            
            if line_count1 != line_count2:
                error_details.append(f"Line count mismatch: new={line_count1}, gold={line_count2}")
                return (1, error_details)
            
            for line1, line2 in zip(f1_lines, f2_lines):
                line_num = line_num + 1
                # Apply custom comparison function to each line
                if not custom_comparison_function(line1, line2):
                    if num_errors < max_diff_lines:
                        error_details.append(f"Line {line_num}:")
                        error_details.append(f"  New:  {line1.rstrip()}")
                        error_details.append(f"  Gold: {line2.rstrip()}")
                    num_errors = num_errors + 1
            
            if num_errors > max_diff_lines:
                error_details.append(f"... and {num_errors - max_diff_lines} more differences (use --max-diff-lines to see more)")
    except Exception as e:
        error_details.append(f"Error comparing files: {str(e)}")
        return (1, error_details)
    
    return (num_errors, error_details)

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

def extract_test_type_from_filename(filename):
    """Extract the test type from the output filename"""
    if filename.endswith('.print'):
        return 'parse --print_show'
    elif filename.endswith('.dump'):
        return 'parse --dump_continuity'
    elif filename.endswith('.check'):
        return 'parse --check_flag'
    elif filename.endswith('.animate'):
        return 'parse --animate_show'
    elif filename.endswith('.json'):
        return 'parse --json'
    elif filename.endswith('.landscape.ps'):
        return 'print_to_postscript --landscape'
    elif filename.endswith('.cont.ps'):
        return 'print_to_postscript --cont'
    elif filename.endswith('.contsheet.ps'):
        return 'print_to_postscript --contsheet'
    elif filename.endswith('.overview.ps'):
        return 'print_to_postscript --overview'
    else:
        return 'unknown'

def extract_show_file_from_path(rel_path):
    """Extract the original .shw filename from the output path"""
    # Path is like: some/path/showname.output.extension
    # We want to get back to: some/path/showname.shw
    base = os.path.dirname(rel_path)
    filename = os.path.basename(rel_path)
    # Remove .output.extension to get showname
    showname = filename.split('.output.')[0] if '.output.' in filename else filename.split('.')[0]
    return os.path.join(base, showname + '.shw') if base else showname + '.shw'

def get_filter_arg_for_test(test_type, show_file):
    """Generate the --filter-test argument for a specific test"""
    # Map test_type to filter arg
    test_map = {
        'parse --print_show': 'print_show',
        'parse --dump_continuity': 'dump_continuity',
        'parse --animate_show': 'animate_show',
        'parse --json': 'json',
        'print_to_postscript --landscape': 'ps_landscape',
        'print_to_postscript --cont': 'ps_cont',
        'print_to_postscript --contsheet': 'ps_contsheet',
        'print_to_postscript --overview': 'ps_overview',
    }
    
    filter_test = test_map.get(test_type, None)
    show_basename = os.path.basename(show_file)
    
    return filter_test, show_basename

def compare_directories(dir1, dir2, calchart_cmd, failures_lock, failures_list, max_diff_lines=10):
    """Compare two directories and track all failures with detailed info"""
    if Debug:
        print(f"comparing dirs {dir1} with {dir2}")
    
    all_passed = True
    for root, _, files in os.walk(dir1):
        for file in files:
            file1 = os.path.join(root, file)
            file2 = os.path.join(dir2, os.path.relpath(file1, dir1))
            rel_path = os.path.relpath(file1, dir1)
            
            if Debug:
                print(f"comparing files {file1} with {file2}")
            
            if not os.path.exists(file2):
                # Extract test info from filename
                test_type = extract_test_type_from_filename(file)
                show_file = extract_show_file_from_path(rel_path)
                
                with failures_lock:
                    failures_list.append({
                        'test_type': test_type,
                        'show_file': show_file,
                        'command': [],  # We don't have the command here
                        'error_message': f"Gold file missing: {file2}",
                        'stderr_output': None
                    })
                all_passed = False
                continue

            # Compare files using custom comparison function
            num_errors, error_details = compare_files(file1, file2, custom_comparison_function, max_diff_lines)
            if num_errors > 0:
                test_type = extract_test_type_from_filename(file)
                show_file = extract_show_file_from_path(rel_path)
                
                # Reconstruct the command that would generate this file
                command = reconstruct_command(calchart_cmd, show_file, test_type, file1)
                
                error_msg = f"{num_errors} line(s) differ:\n" + "\n".join(error_details)
                
                with failures_lock:
                    failures_list.append({
                        'test_type': test_type,
                        'show_file': show_file,
                        'command': command,
                        'error_message': error_msg,
                        'stderr_output': None
                    })
                all_passed = False
    
    return all_passed

def reconstruct_command(calchart_cmd, show_file, test_type, output_file):
    """Reconstruct the command that would produce a given output file"""
    if test_type.startswith('parse'):
        option = test_type.replace('parse ', '').strip()
        return [calchart_cmd, "parse", option, show_file]
    elif test_type.startswith('print_to_postscript'):
        parts = test_type.split()
        if len(parts) > 1 and parts[1] != 'normal':
            option = parts[1]
            return [calchart_cmd, "print_to_postscript", option, show_file, output_file]
        else:
            return [calchart_cmd, "print_to_postscript", show_file, output_file]
    else:
        return []

def check_against_gold(gold_zip, output_directory, calchart_cmd, failures_lock, failures_list, max_diff_lines=10):
    gold_dir = tempfile.mkdtemp()

    # Unzip the gold output
    unzip_gold_output(gold_zip, gold_dir)

    # Compare generated results against gold output
    all_passed = compare_directories(output_directory, gold_dir + "/gold", calchart_cmd, failures_lock, failures_list, max_diff_lines)

    # Cleanup
    shutil.rmtree(gold_dir)
    
    return all_passed

def print_failure_summary(script_path, show_dir, gold_zip, calchart_cmd):
    """Print a detailed summary of all failures"""
    if not failures_list:
        return
    
    print("\n" + "="*80)
    print(f"SANITY TEST FAILURES: {len(failures_list)} test(s) failed")
    print("="*80 + "\n")
    
    for i, failure_dict in enumerate(failures_list, 1):
        print(f"Failure {i}/{len(failures_list)}:")
        print(f"  Test Type: {failure_dict['test_type']}")
        print(f"  Show File: {failure_dict['show_file']}")
        
        if failure_dict['command']:
            cmd_str = ' '.join(f'"{arg}"' if ' ' in str(arg) else str(arg) for arg in failure_dict['command'])
            print(f"  Command:   {cmd_str}")
        
        # Print error message with proper indentation if it's multiline
        error_msg = failure_dict['error_message']
        if '\n' in error_msg:
            print(f"  Error:")
            for line in error_msg.split('\n'):
                print(f"    {line}")
        else:
            print(f"  Error:     {error_msg}")
        
        if failure_dict['stderr_output']:
            print(f"  Stderr:")
            for line in failure_dict['stderr_output'].split('\n'):
                print(f"    {line}")
        
        # Generate focused re-run command
        filter_test, show_basename = get_filter_arg_for_test(failure_dict['test_type'], failure_dict['show_file'])
        if filter_test:
            rerun_cmd = f"python3 {script_path} -d {show_dir} -g {gold_zip}"
            if calchart_cmd != "./build/tools/calchart_cmd/calchart_cmd":
                rerun_cmd += f" -c {calchart_cmd}"
            rerun_cmd += f" --filter-test {filter_test} --filter-show {show_basename} --serial"
            print(f"  Re-run:    {rerun_cmd}")
        
        print()
    
    print("="*80)
    print("To reproduce a specific failure:")
    print("  1. Run the direct command shown above, OR")
    print("  2. Use the 'Re-run' command to run just that test through the test harness")
    print()
    print("To regenerate gold files: python3 resources/tests/sanity_tester.py -d shows -o gold")
    print("="*80 + "\n")

def main():
    global failures_lock, failures_list
    
    args = parse_arguments()
    
    # Initialize multiprocessing manager for failure tracking
    manager = multiprocessing.Manager()
    failures_lock = manager.Lock()
    failures_list = manager.list()

    # Create a temporary directory for output if none is specified
    output_directory = args.output if args.output is not None else tempfile.mkdtemp()

    # what command to run
    calchart_cmd = args.calchart_cmd if args.calchart_cmd is not None else "./build/tools/calchart_cmd/calchart_cmd"

    # Number of CPU cores
    num_cores = multiprocessing.cpu_count() * 2

    # Map test identifiers to actual test configurations
    parse_tests = [
        ("print_show", "--print_show_out", "print"),
        ("dump_continuity", "--dump_cont_out", "dump"),
        ("animate_show", "--animate_show_out", "animate"),
        ("json", "--json_out", "json"),
        ("ps_landscape", "--landscape_out", "landscape.ps"),
        ("ps_cont", "--cont_out", "cont.ps"),
        ("ps_contsheet", "--contsheet_out", "contsheet.ps"),
        ("ps_overview", "--overview_out", "overview.ps"),
    ]

    # Process each file
    processes = []
    tests_scheduled = 0
    
    for root, _, files in os.walk(args.target_directory):
        for file in files:
            if file.endswith('.shw'):
                file_path = os.path.join(root, file)
                output_dir = os.path.join(output_directory, os.path.relpath(root, args.target_directory))
                os.makedirs(output_dir, exist_ok=True)
                output_file = os.path.join(output_dir, os.path.splitext(file)[0] + '.output')

                # Run the command with different combinations of flags
                list_of_options = [b + "=" + output_file + "." + c for a, b, c in parse_tests if should_run_test(a, file_path, args.filter_tests, args.filter_shows)]

                if list_of_options:
                    process = multiprocessing.Process(target=run_command, args=(calchart_cmd, file_path, list_of_options, failures_lock, failures_list))
                    if args.serial:
                        # Run serially - start and wait immediately
                        process.start()
                        process.join()
                        tests_scheduled += 1
                        print(f"Completed: {list_of_options} for {file} ({tests_scheduled} tests run {failures_list})")
                    else:
                        # Parallel mode
                        process.start()
                        processes.append(process)
                        tests_scheduled += 1

            # once we have started a bunch of jobs, see if we should drain them before the next batch.
            # Limit the number of concurrent jobs to the number of CPU cores (only in parallel mode)
            if not args.serial and len(processes) >= num_cores:
                for process in processes:
                    process.join()
                processes = []
    
    if tests_scheduled == 0:
        print("Warning: No tests matched the specified filters!")
        print(f"  Filters: tests={args.filter_tests}, shows={args.filter_shows}")

    # Wait for remaining processes to finish
    for process in processes:
        process.join()

    if args.gold_zip is not None:
        all_passed = check_against_gold(args.gold_zip, output_directory, calchart_cmd, failures_lock, failures_list, args.max_diff_lines)
        
        # Print detailed failure summary
        if not all_passed or failures_list:
            script_path = os.path.relpath(__file__)
            print_failure_summary(script_path, args.target_directory, args.gold_zip, calchart_cmd)
            import sys
            sys.exit(1)
        else:
            print(f"✓ All {tests_scheduled} sanity tests passed!")

    # Cleanup
    if args.output is None:
        shutil.rmtree(output_directory)

if __name__ == "__main__":
    main()
