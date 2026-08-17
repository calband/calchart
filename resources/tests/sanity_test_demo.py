#!/usr/bin/env python3
"""
Demo script showing the improved SanityTest error output.
This demonstrates what you'll see when tests fail.
"""

print("""
================================================================================
                          OLD SANITY TEST OUTPUT
================================================================================

Failed.  Lines file1: 120, file2: 118
Failed.  Line 42 "pt 5: (100, 200), dir=90.0" "pt 5: (100, 200), dir=89.5"
Failed.  Line 43 "pt 6: (150, 200), dir=180.0" "pt 6: (150, 200), dir=179.8"
Differences found between /tmp/abc123/shows/example.output.animate and gold/shows/example.output.animate
File gold/shows/test2.output.print does not exist in gold

[Test exits with code 1]

❌ Problems:
  - Which test type failed? (print_show? animate_show? PostScript?)
  - How do I reproduce this?
  - Which show file caused it?
  - What command was run?

================================================================================
                          NEW SANITY TEST OUTPUT
================================================================================

================================================================================
SANITY TEST FAILURES: 3 test(s) failed
================================================================================

Failure 1/3:
  Test Type: parse --animate_show
  Show File: shows/example.shw
  Command:   ./build/mac-debug/calchart_cmd/calchart_cmd parse --animate_show shows/example.shw
  Error:     2 line(s) differ:
    Line 42:
      New:  pt 5: (100, 200), dir=90.0
      Gold: pt 5: (100, 200), dir=89.5
    Line 43:
      New:  pt 6: (150, 200), dir=180.0
      Gold: pt 6: (150, 200), dir=179.8

Failure 2/3:
  Test Type: parse --print_show
  Show File: shows/test2.shw
  Error:     Gold file missing: gold/shows/test2.output.print

Failure 3/3:
  Test Type: print_to_postscript --landscape
  Show File: shows/halftime.shw
  Command:   ./build/mac-debug/calchart_cmd/calchart_cmd print_to_postscript --landscape shows/halftime.shw /tmp/out.landscape.ps
  Error:     Command exited with code 1
  Stderr:
    Error: Failed to parse show file
    Invalid continuity on sheet 5: unknown command "SPIRAL"

================================================================================
To reproduce a specific failure, run the command shown above.
To regenerate gold files: python3 resources/tests/sanity_tester.py -d shows -o gold
================================================================================

✅ Benefits:
  - Clear test type identification
  - Exact reproduction commands
  - Organized summary of all failures
  - Captures command stderr output
  - Shows which show file caused each failure
  - Actionable next steps

================================================================================
                              QUICK START
================================================================================

# Compare your changes against gold
python3 resources/tests/sanity_tester.py -d shows -g resources/tests/gold.zip -c ./build/mac-debug/calchart_cmd/calchart_cmd

# If you get failures, copy the command from the output and run it:
./build/mac-debug/calchart_cmd/calchart_cmd parse --animate_show shows/example.shw

# If the new behavior is correct, regenerate gold:
python3 resources/tests/sanity_tester.py -d shows -o gold -c ./build/mac-debug/calchart_cmd/calchart_cmd
cd gold && zip -r ../resources/tests/gold.zip . && cd ..

# Verify all tests pass with the new gold:
python3 resources/tests/sanity_tester.py -d shows -g resources/tests/gold.zip -c ./build/mac-debug/calchart_cmd/calchart_cmd

================================================================================
""")
