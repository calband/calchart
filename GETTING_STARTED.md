# Getting Started

This guide is to help you get CalChart3 building.  This guide is written to help you build for macOS, Windows, and Linux.  The estimated time that this should take is about 60-90 minutes, depending on your system performance.

CalChart3 utilizes [CMake](https://cmake.org) to allow several different build platforms and environments.  This tutorial is helpful and may serve as a good reference if you get stuck: https://cgold.readthedocs.io/en/latest/first-step.html.

It's highly recommended that you use clang-format to help keep the coding style consistent.  More information can be found [here](https://github.com/andrewseidl/githook-clang-format) for how to incorporate clang-format into your IDE.  If that's not possible, then please set up the clang-format git-hook as outlined.

## Setting up your dependencies and Packages -- estimated time: 10 minutes

The CalChart3 code tries to be self contained, but it has several external dependencies.

- **Python 3**: Required for building the help system documentation
- **Pandoc**: Required for converting help documentation from Markdown to HTML
- **CMake**: Build system generator
- **vcpkg**: C++ dependency manager

### macOS Steps

Install the required build tools using [brew](https://brew.sh):

First, go to https://brew.sh and follow the installation instructions.  Next, open the Terminal Application and enter the following command:

```
brew install cmake clang-format python3 pandoc
```

**Note:** Pandoc is required for converting help documentation from Markdown to HTML during the build process.

Note:
Avoid using macport version of cmake due to this issue:
https://trac.macports.org/ticket/58450

#### Installing vcpkg

CalChart uses [vcpkg](https://vcpkg.io) to manage C++ dependencies. Install vcpkg and set up the environment variable:

```bash
# Clone vcpkg to a location of your choice (e.g., ~/vcpkg)
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg
./bootstrap-vcpkg.sh

# Set the VCPKG_ROOT environment variable (add to ~/.zshrc or ~/.bash_profile)
echo 'export VCPKG_ROOT="$HOME/vcpkg"' >> ~/.zshrc
source ~/.zshrc
```

After setting the environment variable, restart your terminal for the changes to take effect.

### Windows Steps

We recommend using [chocolatey](https://chocolatey.org) for the package manager for Windows.  

First, go to https://chocolatey.org/install and follow the installation instructions. You'll need to run PowerShell as Administrator:
1. Press Windows key, type "PowerShell"
2. Right-click on "Windows PowerShell" and select "Run as administrator"
3. Follow the Chocolatey installation instructions from the website

Once Chocolatey is installed, install the required dependencies (in an admin PowerShell):

```
choco install cmake git winflexbison ninja clang-format python3 pandoc
```

**Note**: `ninja` is required for building with CMake presets. Pandoc is required for converting help documentation from Markdown to HTML. If you encounter issues with Ninja not being found, close and reopen your terminal after installation.

#### Installing vcpkg

CalChart uses [vcpkg](https://vcpkg.io) to manage C++ dependencies. Install vcpkg and set up the environment variable:

```powershell
# Clone vcpkg to a location of your choice (e.g., C:\vcpkg)
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat

# Set the VCPKG_ROOT environment variable (required for CMake to find vcpkg)
[System.Environment]::SetEnvironmentVariable('VCPKG_ROOT', 'C:\vcpkg', [System.EnvironmentVariableTarget]::User)
```

After setting the environment variable, close and reopen your terminal/VS Code for the changes to take effect.

### Linux Steps

We recommend using apt-get to install the required build tools:

```
sudo apt-get update && sudo apt-get install build-essential libgtk-3-dev git cmake bison flex clang-format ninja-build python3 pandoc
```

**Note:** Pandoc is required for converting help documentation from Markdown to HTML during the build process.

#### Installing vcpkg

CalChart uses [vcpkg](https://vcpkg.io) to manage C++ dependencies. Install vcpkg and set up the environment variable:

```bash
# Clone vcpkg to a location of your choice (e.g., ~/vcpkg)
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
cd ~/vcpkg
./bootstrap-vcpkg.sh

# Set the VCPKG_ROOT environment variable (add to ~/.bashrc)
echo 'export VCPKG_ROOT="$HOME/vcpkg"' >> ~/.bashrc
source ~/.bashrc
```

After setting the environment variable, restart your terminal for the changes to take effect.

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




