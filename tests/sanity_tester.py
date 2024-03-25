#!/usr/bin/env python3

import argparse
import os
import subprocess
import shutil
import tempfile
import zipfile
import multiprocessing

Description="""
Sanity Tester for CalChart.
Example: How to compare with a gold version:
sanity_tester.py -d shows -g gold.zip

Example: How to generate new gold results
sanity_tester.py -d shows -o gold
zip -r gold.zip gold
"""

Debug = False

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
    with open(output_name, 'w') as output:
        # we do this to filter out the dir name to allow location agnostic testing
        result = subprocess.run(command, capture_output=True, text=True)
        output.write(filter_output(result.stdout, dir_name))

def compare_files(file1, file2, custom_comparison_function):
    num_errors = 0
    line_num = 0
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        for line1, line2 in zip(f1, f2):
            line_num = line_num + 1
            # Apply custom comparison function to each line
            if not custom_comparison_function(line1, line2):
                print(f"Failed.  Line {line_num} \"{line1}\" \"{line2}\"")
                num_errors = num_errors + 1
    return num_errors

def extractValues(input_string):
    import re
    # input_string = "pt 98: (448, 672), dir=-168.7"
    # -> 
    # [98, 448, 672, -168.7]

    pattern = r"pt (\d+): \((-?\d+), (-?\d+)\), dir=(-?\d+(\.\d+)?([eE]-?\d+)?)"
    match = re.match(pattern, input_string)
    if match:
        return [ int(match.group(1)), int(match.group(2)), int(match.group(3)), float(match.group(4)) ]
    else:
        return None

def custom_comparison_function(line1, line2):
    value1 = extractValues(line1.strip())
    value2 = extractValues(line2.strip())
    if value1 is None or value2 is None:
        if Debug:
            print(f"comparing {line1.strip()} and {line2.strip()}")
        return line1.strip() == line2.strip()
    # Adjust tolerance as needed
    tolerance = 1e-2
    result = value1[0] == value2[0] and value1[1] == value2[1] and value1[2] == value2[2] and abs(value1[3] - value2[3]) < tolerance
    if Debug:
        print(f"comparing {value1} and {value2} -> {result}")
    return result

def compare_directories(dir1, dir2):
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
                return False

            # Compare files using custom comparison function
            if compare_files(file1, file2, custom_comparison_function) > 0:
                print(f"Differences found between {file1} and {file2}")
                return False
    return True

def check_against_gold(gold_zip, output_directory):
    gold_dir = tempfile.mkdtemp()

    # Unzip the gold output
    unzip_gold_output(gold_zip, gold_dir)

    # Compare generated results against gold output
    if not compare_directories(output_directory, gold_dir + "/gold"):
        import sys
        sys.exit(1)

    # Cleanup
    shutil.rmtree(gold_dir)

def main():
    args = parse_arguments()

    # Create a temporary directory for output if none is specified
    output_directory = args.output if args.output is not None else tempfile.mkdtemp()

    # what command to run
    calchart_cmd = args.calchart_cmd if args.calchart_cmd is not None else "./build/calchart_cmd/calchart_cmd"

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

                # Run the command with different combinations of flags asynchronously
                for [options, extension] in [["-p", "print"], ["-d", "dump"], ["-D", "dumpprintcont"], ["-c", "check"], ["-a", "animate"]]:
                    process = multiprocessing.Process(target=run_command, args=(calchart_cmd, file_path, output_file, options, extension))
                    process.start()
                    processes.append(process)

            # once we have started a bunhc of jobs, see if we should drain them before the next batch.
            # Limit the number of concurrent jobs to the number of CPU cores
            if len(processes) >= num_cores:
                for process in processes:
                    process.join()
                processes = []

    # Wait for remaining processes to finish
    for process in processes:
        process.join()

    if args.gold_zip is not None:
        check_against_gold(args.gold_zip, output_directory)

    # Cleanup
    if args.output is None:
        shutil.rmtree(output_directory)

if __name__ == "__main__":
    main()

