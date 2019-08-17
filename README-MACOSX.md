# Steps for building for MacOSX.

These steps were written with MacOS 10.14 and XCode 10.2.  Both should be on the app store.  You *can* do earlier builds, but you may have to modify your install steps.

## Dependencies.

### Xcode

We use Xcode to build CalChart on Mac.  It's available on the App store.

You also need to install the command line tools for Xcode.  See https://developer.apple.com/library/ios/technotes/tn2339/_index.html

### CMake

CalChart uses CMake to build its projects.  I recommend you read through this tutorial to help you make sure you've got CMake set up for you system.
https://cgold.readthedocs.io/en/latest/first-step.html

### Boost

CalChart uses Boost.  See boost.org for more information about what Boost is.  You will need to make sure it is installed on your system somewhere common so CMake can find it.  I recommend using macports to install it.  See http://www.macports.org/.

## Building CalChart:

CalChart uses cmake to configure and run the platform specific build system.  For Mac we use Xcode.

Get CalChart (you'll need to log in with your GitHub account:

	$ git clone --recursive https://github.com/calband/calchart.git ~/calchart

If you need to rebuild the documentation, refer to the README-DOC.txt

First use cmake to generate the Xcode project file:

	$ cd ~/calchart
	$ mkdir build
        $ cd build
        $ cmake -G Xcode ..

Now open the xcode project in build/CalChart.xcodeproj and select the CalChart target.

Build and run!  I find I need to build several times in order to have it finish building.

