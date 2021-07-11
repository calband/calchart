# Steps for building on Windows

Install windows with at least 64 gigs space and 8GB mem.
Install all updates until it's happy

## Get the Tools

Get the latest cygwin (I used 2.10.0).  Set up with the standard instation, and add the following
packages:

* git
* cmake
* bison
* flex

## Get Visual Studio Community 2017
We tried Visual Studio Community 2017.
You'll need to install the "Visual C++" "Common Tools for Visual C++ 2017".


## Add environment variables
Go to Control Panel->System->Advanced System Settings->Environment Variables
Add the following User variables:

	CMAKE_PREFIX_PATH=$PATH_TO_BISON

## Get CMake
Read instructions at 
https://cmake.org/download/
https://cmake.org/runningcmake/


## Get calchart (you'll need to log in with your source forge account):
Open cygwin and run the following:

	$ git clone --recursive https://github.com/calband/calchart.git /cygdrive/c/calchart

(or cd to whereever you installed calchart)

## Configure build system with CMake
See
https://cmake.org/runningcmake/

Set the "Where is the source code:" to c:/calchart
Set the "Where to build the binaries:" to c:/calchart/build

Press Configure
Press Generate
Press Open Project

## Make Calchart
Build the target CalChart.  This will create the CalChart.exe.

You can also build the ALL_BUILD target.


