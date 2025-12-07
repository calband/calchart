<!-- Copilot instructions for repository contributors and AI agents -->
# CalChart — Copilot guidelines

Focus: be practical and code-aware. The repository is a native C++ project using CMake, wxWidgets UI, and a core library that is reused by the `CalChart` GUI and a command-line tool `calchart_cmd`.

Quick facts
- Build: CMake (top-level `CMakeLists.txt`). Core library in `src/core` (target `calchart_core`). GUI target `CalChart` in `src/`. CLI tool `calchart_cmd` lives in `calchart_cmd/` and is built on non-MSVC platforms.
- Languages: C++ (C++20), Bison/Flex are used to generate parser code from `src/core/contgram.y` / `src/core/contscan.l`.
- Tests: CTest-based tests in `tests/` and a `tests/sanity_tester.py` harness that invokes `calchart_cmd`.
- CI: GitHub Actions workflow at `.github/workflows/cmake.yml` runs cmake/configure/build/test and packages with CPack.

What to change and why
- Prefer modifying the CMake file within the directory you change (project follows per-directory CMakeLists). When adding source files, update the CMakeLists in the same folder (see top comment in `CMakeLists.txt`).
- Parser changes must update the grammar `contgram.y` or lexer `contscan.l` in `src/core` — those are processed by BISON/FLEX via CMake targets (see `src/core/CMakeLists.txt` where BISON_TARGET/FLEX_TARGET are used).

Dev workflows (concrete commands)
- Configure + build (out-of-source):
  - `cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug`
  - `cmake --build build --config Debug`
- Run tests: `ctest --test-dir build --output-on-failure` or from project root: `cmake --build build --target test`.
- Run sanity harness: `python3 tests/sanity_tester.py -c ./build/calchart_cmd/calchart_cmd -d shows -g tests/gold.zip` (the script will default to `./build/calchart_cmd/calchart_cmd` if `-c` omitted).
- macOS notes: useful setup script `scripts/osx-setup.sh` automates installing wxWidgets used historically. CI uses FetchContent to obtain dependencies; local dev may prefer system-installed wxWidgets.

Debugging notes
- GUI uses wxWidgets; run the `CalChart` target binary from `build/src/CalChart` (macOS bundle: `build/src/CalChart.app`).
- `calchart_cmd` is a lightweight CLI useful for unit test data generation and JSON/PS dumps. Example usage in `calchart_cmd/main.cpp` (see `--version`, `parse`, `print_to_postscript`).

Project conventions and patterns
- Core vs UI separation: `src/core` contains logic, parser-generated sources, and the `calchart_core` static library. The `src` folder contains the wxWidgets GUI which uses the core library via `calchart_core` target.
- Modern CMake: the project uses helper functions in `cmake/compiler.cmake` and `cmake/dependencies.cmake`; prefer using `SetupCompilerForTarget(...)` when adding targets and declare dependencies with `target_link_libraries`.
- AddressSanitizer: enabled for Debug by default via `cmake/compiler.cmake` (option `ENABLE_ASAN`). Useful for memory bug detection.
- JSON uses `nlohmann_json` (declared in `src/CMakeLists.txt` and linked into `calchart_core` and GUI).

## Bug Reporting Feature

CalChart includes built-in bug reporting via GitHub issues (new as of v3.8.4).

**Architecture**:
- Core library (`src/core/CalChartGitHubIssueSubmitter.cpp`) provides pure formatting: `FormatBugReportAsMarkdown(report)`
- UI layer (`src/GitHubIssueSubmitter.cpp`) handles wxWidgets integration: `StartBackgroundIssueSubmission(report, callback)`
  - Uses libcurl for HTTPS requests to GitHub REST API
  - Background thread with wxApp::CallAfter() for UI thread callbacks
  - Reads CALCHART_GITHUB_TOKEN env var for authentication
  - Clipboard fallback when token is not configured
- Dialog (`src/BugReportDialog.h/cpp`) collects user input with privacy controls
- User entry: Help menu → "Report a Bug..." (Ctrl+Shift+B)
- Crash handler: Global exception handler offers to file bug on crash

**When modifying bug reporting code**:
- Keep diagnostic collection in `src/core/CalChartDiagnosticInfo.*` (platform-agnostic)
- Keep GitHub API code in `src/GitHubIssueSubmitter.cpp` (wxWidgets-dependent)
- Keep dialog UI in `src/BugReportDialog.cpp` (wxUI builder pattern)
- Add tests in `core/tests/CalChartGitHubIssueSubmitterTests.cpp` for formatting
- See `docs/BugReporting.md` for user documentation and token setup instructions

**Testing**:
- Unit tests: `ctest --test-dir build --output-on-failure | grep CalChartCoreTests`
- Full suite: `ctest --test-dir build --output-on-failure`
- Manual: Open CalChart, press Ctrl+Shift+B, verify dialog appears and privacy notice is clear

Integration points and external dependencies
- wxWidgets: UI framework — code that adapts draw primitives to wx lives in `src/CalChartDrawPrimativesHelper.h` and many UI files (e.g. `CalChartApp.*`, `CalChartFrame.*`).
- Bison/Flex: grammar files in `src/core/contgram.y` and `src/core/contscan.l` produce `contgram.cpp` and `contscan.cpp` into the build tree.
- External libraries fetched in CMake: `docopt` (used by `calchart_cmd`), `nlohmann_json`, and optionally `wxUI` (wrapper). See `cmake/dependencies.cmake`.

Examples to reference when implementing changes
- Add a new CLI flag: follow pattern in `calchart_cmd/main.cpp` using `docopt` and implement behavior in `calchart_cmd_parse.hpp`.
- Modify parsing behavior: update `src/core/contgram.y` and run a CMake rebuild — generated files appear under `build/src/core/` (Bison/Flex outputs are wired in `src/core/CMakeLists.txt`).

Edits and PR tips for reviewers
- Keep CMake edits local to directories and avoid monolithic top-level edits. Run `cmake --build build` after editing CMake lists to ensure generated sources (Bison/Flex) are present.
- Run `ctest -C Debug --output-on-failure` locally if you touched core logic.
- Prefer small, focused PRs: core logic changes (in `src/core`) and UI updates (in `src/`) are commonly reviewed separately.

Submitting changes / Pull requests
- Always work on a feature branch (do not commit directly to `main`). Example:
  - `git checkout -b fix/copilot-instructions-short-branch-name`
- Keep commits small and messages short (one-line summary). Use multiple commits for logical steps.
  - Good: `Add copilot instructions for build/test workflow`
  - Avoid long multi-paragraph commit messages for small changes; use the PR description for details.
- Commit messages should reference the issue when applicable and follow the form `Issue #<number>: <Short summary>`.
  - The issue number and the exact issue title/summary should be supplied by the author creating the commit/PR.
  - Example: `Issue #513: Add Copilot instructions for build/test and PR workflow`
- Push branch and create a PR. Example (GitHub CLI):
  - `git push -u origin fix/copilot-instructions-short-branch-name`
  - `gh pr create --fill --base main` (or open a PR via GitHub web UI)
- Before creating a PR run the basic validation locally:
  - Configure & build: `cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug && cmake --build build --config Debug`
  - Run unit tests: `ctest --test-dir build --output-on-failure`
  - Run sanity harness (optional for changes touching parsing or output): `python3 tests/sanity_tester.py -c ./build/calchart_cmd/calchart_cmd -d shows -g tests/gold.zip`
- In PRs, point reviewers to representative files changed (e.g., `src/core/contgram.y`, `src/CMakeLists.txt`, or `calchart_cmd/main.cpp`) and any commands to reproduce the change.

If anything above is ambiguous or you need command examples for a platform, ask and I will expand with exact commands and small reproduction steps.
