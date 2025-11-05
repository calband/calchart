# Getting Started

This guide is to help you get CalChart3 building.  This guide is written to help you build for macOS, Windows, and Linux.  The estimated time that this should take is about 60-90 minutes, depending on your system performance.

CalChart3 utilizes [CMake](https://cmake.org) to allow several different build platforms and environments.  This tutorial is helpful and may serve as a good reference if you get stuck: https://cgold.readthedocs.io/en/latest/first-step.html.

It's highly recommended that you use clang-format to help keep the coding style consistent.  More information can be found [here](https://github.com/andrewseidl/githook-clang-format) for how to incorporate clang-format into your IDE.  If that's not possible, then please set up the clang-format git-hook as outlined.

## Setting up your dependencies and Packages -- estimated time: 10 minutes

The CalChart3 code tries to be self contained, but it has several external dependencies.

### macOS Steps

Next, you will need to install a package manager to install `cmake`. I recommend using [brew](https://brew.sh) to install the tools that may be missing on your Mac.

First, go to https://brew.sh and follow the installation instructions.  Next, open the Terminal Applicaton and enter the following command:

```
brew install cmake clang-format
```

Note:
Avoid using macport version of cmake due to this issue:
https://trac.macports.org/ticket/58450

### Windows Steps

We recommend using [chocolatey](https://chocolatey.org) for the package manager for Windows.  

First, go to https://chocolatey.org/install and follow the installation instructions.  Once installed, open powershell.exe and install `cmake`, `git`, and `winbisonflex`:

```
choco install cmake git winflexbison clang-format
```

### Linux Steps

We recommend using apt-get to install `git`, `cmake`, `bison`, `flex`, and the gtk:

```
sudo apt-get update && sudo apt-get install build-essential libgtk-3-dev git cmake bison flex clang-format
```

## Dependency modes and CI behavior

CalChart now supports two dependency modes to speed up developer and CI builds while still allowing fully reproducible release builds:

- USE_SYSTEM_DEPENDENCIES (default ON): prefer system-installed libraries via package managers (brew/apt/vcpkg). This gives much faster configure/build cycles for local development and pull-request CI runs.
- FORCE_VENDOR_DEPENDENCIES (default OFF): force building dependencies from source using CMake's FetchContent. This is slower but produces a self-contained, reproducible build (used for releases).

CI behavior
- Pull request and branch builds (default) use system dependencies so CI is fast. The workflow will not set `FORCE_VENDOR_DEPENDENCIES`.
- Tag/release builds explicitly enable `FORCE_VENDOR_DEPENDENCIES=ON` when configuring CMake so releases are built from vendor sources (this ensures reproducibility of release artifacts).

Fastest local build (recommended)
1. Install dependencies with your package manager. On macOS (Homebrew):

```bash
brew install cmake clang-format wxwidgets nlohmann-json catch2
```

On Ubuntu/Debian (APT):

```bash
sudo apt-get update
sudo apt-get install build-essential libgtk-3-dev libwxgtk3.0-gtk3-dev libcurl4-openssl-dev cmake bison flex clang-format
```

2. Configure and build using system dependencies (fast):

```bash
cmake -B build -S . -DUSE_SYSTEM_DEPENDENCIES=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug -j$(nproc)
```

Notes about wxWidgets and Homebrew
- Homebrew installs `wx-config` and the wx libraries, but on some Homebrew packages a CMake package config (which provides `wx::core`, `wx::net`, `wx::gl` imported targets) is not installed. CalChart includes a small compatibility shim that creates lightweight `wx::` imported targets from the legacy `wxWidgets_*` variables when the modern targets are not present — this lets you use the Homebrew install for fast local builds.
- If you do have a wxWidgets install that provides a CMake config/targets (in `/opt/homebrew/.../lib/cmake` or similar) you can point CMake to it with `-DCMAKE_PREFIX_PATH=/opt/homebrew/opt/wxwidgets/lib/cmake`.

Reproducible release build (slow)
- To reproduce a release (or when building on CI for a tagged release), build with vendor dependencies enabled. This downloads and builds all vendor sources (including wxWidgets) and can take substantially longer.

```bash
cmake -B build -S . -DUSE_SYSTEM_DEPENDENCIES=OFF -DFORCE_VENDOR_DEPENDENCIES=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(nproc)
```

Forcing vendor deps in CI
- The GitHub Actions workflow has been updated so that when a workflow run is for a tag (releases), the configure step will pass `-DFORCE_VENDOR_DEPENDENCIES=ON` automatically. You don't need to change CI by hand unless you want a different policy.

Troubleshooting
- If CMake fails saying a package is missing, either install the system package for your platform (brew/apt/vcpkg) or re-run CMake with `-DFORCE_VENDOR_DEPENDENCIES=ON` to let CMake fetch the source automatically (useful offline or in release automation).

CI packages installed for fast builds
-----------------------------------

The CI workflow installs system packages for PR/branch builds to keep CI fast. If you want your local environment to match CI, install the same set of packages listed below.

macOS (Homebrew) packages installed in CI:

```bash
brew update
brew install cmake wxwidgets nlohmann-json catch2 pkg-config
```

Ubuntu (apt) packages installed in CI:

```bash
sudo apt-get update
sudo apt-get install -y build-essential libgtk-3-dev libcurl4-openssl-dev \
	libwxgtk3.0-gtk3-dev libnlohmann-json-dev catch2 pkg-config
```

These are used by the CI job to satisfy `find_package` calls and avoid long vendor builds during PRs and branch testing.


## Getting the CalChart3 source code -- estimated time: 5 minutes

The CalChart3 source code lives on [Github](https://github.com/calband/calchart).  We use submodules to package several projects sources together for the final project.  To get the source code to your machine, you will need to clone the repository to a local copy.  You should choose a target directory that you can find easily on your machine.  In this guide, we use the default directory location.

```
git clone https://github.com/calband/calchart.git ./calchart
```

You should see the project being downloaded, and it should appear to be similar to:
```
Cloning into './calchart'...
remote: Enumerating objects: 12166, done.
remote: Counting objects: 100% (1214/1214), done.
remote: Compressing objects: 100% (321/321), done.
remote: Total 12166 (delta 896), reused 1143 (delta 848), pack-reused 10952
...
Receiving objects: 100% (5068/5068), 2.70 MiB | 11.27 MiB/s, done.
Resolving deltas: 100% (3559/3559), done.
```

## Setting up clang-format git-hook (optional)

If you are not able to set up your IDE for clang-format, then set up a git-hook with the following command:

```
git clone https://github.com/andrewseidl/githook-clang-format.git /tmp/githook-clang-format.git
cp /tmp/githook-clang-format.git/clang-format.hook .git/hooks/pre-commit       
```

## Setting up your IDE -- estimated time: 10 minutes

To build and debug CalChart you'll need a Integrated Development Environment ([IDE](https://en.wikipedia.org/wiki/Integrated_development_environment)).  For this guide we assume you will be using `Visual Studio Code`, but other environments, like `Xcode`, `Visual Studio Explorer`, `emacs`, etc, work if you are more comfortable with them.

First, install install [Visual Studio Code](https://code.visualstudio.com).

Once installed, you'll need to add 2 additional extensions.  Go to `View` -> `Extensions` to open up the Extensions Browser and install the `CMake Tools` and the `C/C++`.


## Opening and configuring the project -- estimated time: 5 minutes

In `Visual Studio Code`, go to `File` -> `Open...` and then find the directory where you clone the git repository.  Once open, `Visual Studio Code` should recognize the CMake file, and prompt if you want to configure the project for CMake.  Say yes, and a toolbar at the bottom of the screen should appear, and display the CMake configuration options.

You will first need to specify the "Kit" for CMake to use.  Look to the bottom of the Visual Studio Code window and you should see a "No Kit Select" in the blue bar.  Click the bottom and choose `[Unspecified]` to allow CMake to determine the Kit to use, or specify one from the list that you would like to select.

## Build the project -- estimated time: 30 minutes

Finally is time to build the project.  Simply click the `Build` gear located in the blue bar at the botom at the bottom of the toolbar and let 'er rip!  This process should take about 30 minutes, but may be longer depending on your machine.
 
## Running the project

If everything went as expected you should be able to press the play icon in the blue bar at the bottom of the screen in order to run CalChart.  Hurrah!




