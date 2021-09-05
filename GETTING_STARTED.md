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

## Getting the CalChart3 source code -- estimated time: 5 minutes

The CalChart3 source code lives on [Github](https://github.com/calband/calchart).  We use submodules to package several projects sources together for the final project.  To get the source code to your machine, you will need to clone the repository to a local copy.  You should choose a target directory that you can find easily on your machine.  In this guide, we use the default directory location.

```
git clone --recursive https://github.com/calband/calchart.git ./calchart
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
Submodule path 'submodules/wxWidgets/3rdparty/catch': checked out 'ee4acb6ae6e32a02bc012d197aa82b1ca7a493ab'
Submodule path 'submodules/wxWidgets/src/expat': checked out '7532d85708929ebdb148308ca998268d3aaf3527'
Submodule path 'submodules/wxWidgets/src/jpeg': checked out '852493611506076fd9ad931d35bf1c3cc5b5a9c3'
Submodule path 'submodules/wxWidgets/src/png': checked out '3ffeff7877598d3236cc09a6d3f478073eb33f35'
Submodule path 'submodules/wxWidgets/src/tiff': checked out '9f657ff8a7411c95ffe83ec39e3e881c3fec6bb0'
Submodule path 'submodules/wxWidgets/src/zlib': checked out '5888671274cde770edbe683b435f052de2b03681'
```

## Setting up clang-format git-hook

If you are not able to set up your IDE for clang-format, then set up a git-hook with the following command:

```
cp submodules/githook-clang-format/clang-format.hook .git/hooks/pre-commit       
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




