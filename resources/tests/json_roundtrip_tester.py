#!/usr/bin/env python3

import argparse
import os
import subprocess
import tempfile
import multiprocessing
import sys

Description="""
JSON Round-trip Tester for CalChart.
Verifies that shows can be written to JSON and read back without data loss.

Example:
json_roundtrip_tester.py -d shows -c ./build/tools/calchart_cmd/calchart_cmd
"""

Debug = False

def parse_arguments():
    parser = argparse.ArgumentParser(description=Description, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("-d", "--target-directory", required=True, help="The directory of show files to process")
    parser.add_argument("-c", "--calchart_cmd", help="The path for calchart_cmd (optional, defaults to ./build/tools/calchart_cmd/calchart_cmd)")
    parser.add_argument("-s", "--show-schema", help="Path to the show schema (optional, defaults to resources/common/show_schema_v1.json)")
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose output")
    return parser.parse_args()

def test_json_roundtrip(command_location, file_path, schema_path):
    """
    Test that a show can be converted to JSON and back without loss.
    Returns (status, error_message) tuple where status is:
        'success': Test passed
        'unopenable': File cannot be opened (this is okay - file is invalid)
        'json_export_failed': File can be opened but JSON export failed (BUG)
        'json_invalid': JSON export succeeded but output is not valid JSON (BUG)
        'roundtrip_failed': Valid JSON but roundtrip comparison failed (BUG)
    """
    try:
        # First, check if the file can be opened at all with a simple parse
        # Use --check_flag as a lightweight way to verify the file is valid
        check_command = [command_location, "parse", file_path, "--check_flag"]
        if Debug:
            print(f"Running: {' '.join(check_command)}")
        
        check_result = subprocess.run(check_command, capture_output=True, text=True)
        if check_result.returncode != 0:
            # File cannot be opened - this is okay, it's an invalid show file
            return ('unopenable', f"Cannot open file (invalid show): {check_result.stderr[:200]}")
        
        # File can be opened, now try to export to JSON
        # Create temporary files for the two JSON outputs
        with tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as tmp1, \
             tempfile.NamedTemporaryFile(mode='w', suffix='.json', delete=False) as tmp2:
            tmp1_path = tmp1.name
            tmp2_path = tmp2.name

        # First conversion: .shw -> JSON
        command1 = [command_location, "parse", file_path, "--showjson", "--showschema", schema_path, "--jsonwidth=0"]
        if Debug:
            print(f"Running: {' '.join(command1)}")
        
        result1 = subprocess.run(command1, capture_output=True, text=True)
        if result1.returncode != 0:
            return ('json_export_failed', f"JSON export failed: {result1.stderr[:200]}")
        
        # Verify the first output is valid JSON
        try:
            import json
            json1 = json.loads(result1.stdout)
        except json.JSONDecodeError as e:
            return ('json_invalid', f"toJSON() did not produce valid JSON: {e}")
        
        with open(tmp1_path, 'w') as f:
            f.write(result1.stdout)

        # Second conversion: JSON -> JSON
        command2 = [command_location, "parse", tmp1_path, "--showjson", "--showschema", schema_path, "--jsonwidth=0"]
        if Debug:
            print(f"Running: {' '.join(command2)}")
        
        result2 = subprocess.run(command2, capture_output=True, text=True)
        if result2.returncode != 0:
            return ('json_export_failed', f"Second JSON export failed: {result2.stderr[:200]}")
        
        # Verify the second output is valid JSON
        try:
            json2 = json.loads(result2.stdout)
        except json.JSONDecodeError as e:
            return ('json_invalid', f"Second conversion did not produce valid JSON: {e}")
        
        with open(tmp2_path, 'w') as f:
            f.write(result2.stdout)

        # Compare the two JSON objects (not just text)
        if json1 != json2:
            # Try to identify specific differences
            def find_diff(obj1, obj2, path=""):
                """Recursively find differences between two JSON objects"""
                if type(obj1) != type(obj2):
                    return f"Type mismatch at {path}: {type(obj1).__name__} vs {type(obj2).__name__}"
                
                if isinstance(obj1, dict):
                    keys1 = set(obj1.keys())
                    keys2 = set(obj2.keys())
                    if keys1 != keys2:
                        missing = keys1 - keys2
                        extra = keys2 - keys1
                        msg = []
                        if missing:
                            msg.append(f"Missing keys in second: {missing}")
                        if extra:
                            msg.append(f"Extra keys in second: {extra}")
                        return f"Key mismatch at {path}: {', '.join(msg)}"
                    
                    for key in keys1:
                        diff = find_diff(obj1[key], obj2[key], f"{path}.{key}" if path else key)
                        if diff:
                            return diff
                
                elif isinstance(obj1, list):
                    if len(obj1) != len(obj2):
                        return f"Array length mismatch at {path}: {len(obj1)} vs {len(obj2)}"
                    
                    for i, (item1, item2) in enumerate(zip(obj1, obj2)):
                        diff = find_diff(item1, item2, f"{path}[{i}]")
                        if diff:
                            return diff
                
                elif obj1 != obj2:
                    # Truncate long values for readability
                    val1_str = str(obj1)[:100]
                    val2_str = str(obj2)[:100]
                    return f"Value mismatch at {path}: {val1_str} vs {val2_str}"
                
                return None
            
            diff_msg = find_diff(json1, json2)
            return ('roundtrip_failed', f"JSON objects differ: {diff_msg}")

        # Clean up temporary files
        os.unlink(tmp1_path)
        os.unlink(tmp2_path)
        
        return ('success', None)

    except Exception as e:
        return ('error', f"Exception: {str(e)}")

def process_file(args_tuple):
    """
    Process a single file. Used for multiprocessing.
    Returns (file_path, status, error_message) tuple.
    """
    command_location, file_path, schema_path = args_tuple
    status, error = test_json_roundtrip(command_location, file_path, schema_path)
    return (file_path, status, error)

def main():
    global Debug
    args = parse_arguments()
    
    Debug = args.verbose

    # Set up calchart_cmd path
    calchart_cmd = args.calchart_cmd if args.calchart_cmd else "./build/tools/calchart_cmd/calchart_cmd"
    
    # Verify calchart_cmd exists
    if not os.path.exists(calchart_cmd):
        print(f"Error: calchart_cmd not found at {calchart_cmd}")
        sys.exit(1)

    # Set up schema path
    schema_path = args.show_schema if args.show_schema else "resources/common/show_schema_v1.json"
    
    # Verify schema exists
    if not os.path.exists(schema_path):
        print(f"Error: Schema not found at {schema_path}")
        sys.exit(1)

    # Collect all .shw files
    shw_files = []
    for root, _, files in os.walk(args.target_directory):
        for file in files:
            if file.endswith('.shw'):
                file_path = os.path.join(root, file)
                shw_files.append(file_path)

    if not shw_files:
        print(f"No .shw files found in {args.target_directory}")
        sys.exit(1)

    print(f"Found {len(shw_files)} show files to test")

    # Process files in parallel
    num_cores = multiprocessing.cpu_count()
    pool_args = [(calchart_cmd, file_path, schema_path) for file_path in shw_files]
    
    with multiprocessing.Pool(processes=num_cores) as pool:
        results = pool.map(process_file, pool_args)

    # Analyze results
    results_by_status = {
        'success': [],
        'unopenable': [],
        'json_export_failed': [],
        'json_invalid': [],
        'roundtrip_failed': [],
        'error': []
    }
    
    for file_path, status, error in results:
        results_by_status[status].append((file_path, error))
        
        if status == 'success':
            if args.verbose:
                print(f"✓ {file_path}")
        elif status == 'unopenable':
            if args.verbose:
                print(f"⊘ {file_path} (invalid show file - okay)")
        else:
            # All other statuses are actual failures
            print(f"✗ {file_path}")
            if error:
                print(f"  [{status}] {error}")

    # Print summary
    print(f"\n{'='*60}")
    print(f"Tested {len(shw_files)} files")
    print(f"\nResults:")
    print(f"  ✓ Passed:               {len(results_by_status['success'])}")
    print(f"  ⊘ Unopenable (okay):    {len(results_by_status['unopenable'])}")
    
    # Count actual failures (bugs)
    bugs = (len(results_by_status['json_export_failed']) + 
            len(results_by_status['json_invalid']) + 
            len(results_by_status['roundtrip_failed']) +
            len(results_by_status['error']))
    
    print(f"\n  ✗ BUGS FOUND:           {bugs}")
    if results_by_status['json_export_failed']:
        print(f"    - JSON export failed: {len(results_by_status['json_export_failed'])}")
    if results_by_status['json_invalid']:
        print(f"    - Invalid JSON:       {len(results_by_status['json_invalid'])}")
    if results_by_status['roundtrip_failed']:
        print(f"    - Roundtrip failed:   {len(results_by_status['roundtrip_failed'])}")
    if results_by_status['error']:
        print(f"    - Other errors:       {len(results_by_status['error'])}")
    
    # List files with bugs
    if bugs > 0:
        print(f"\nFiles with bugs:")
        for status in ['json_export_failed', 'json_invalid', 'roundtrip_failed', 'error']:
            if results_by_status[status]:
                print(f"\n  {status.replace('_', ' ').title()}:")
                for file_path, _ in results_by_status[status]:
                    print(f"    - {file_path}")
        sys.exit(1)
    else:
        print(f"\n✓ All openable files passed JSON roundtrip test!")
        sys.exit(0)

if __name__ == "__main__":
    main()
