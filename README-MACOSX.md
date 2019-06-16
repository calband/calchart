# Steps for building for MacOSX.

These steps were written with 10.14 and XCode 10.2.  Both should be on the app store.  You *can* do earlier builds, but you may have to modify your install steps.

You also need to install the command line tools for Xcode.  See https://developer.apple.com/library/ios/technotes/tn2339/_index.html

## Boost
CalChart uses boost.  Most tools will already be on your system, but you will need the following projects from macports (or fink).  See http://www.macports.org/:

* boost

We assume that boost library is located at /opt/local/include.  If it is located somewhere else, you'll need to modify the Xcode project.

## wxWidgets
Note: These instructions assume you will build from source.  If you want to download from HomeBrew or some other package utility, you will have to modify xcode project to link to the appropriate place.

Note: All lines starting with ‘$’ mean you should run them from the terminal.  Do not include the ‘$’ with the command :)

## Download and build 3.1 of wxWidgets
Go to wxwidgets.org and download the "Source for Linux, OS X".  You will then need to move the package to a known place (I recommend your desktop).
Make sure you unzip and untar the bundle (usually involves just double clicking the tar file.

To start the terminal, open Spotlight and search for terminal.

	$ cd ~/Downloads/wxWidgets-3.1

Now you can build.  You'll first need to create a work directory, then configure the project for your system, and finally make it.  Note, it may take some time and many resources to build, so you'll want to leave your computer alone for some time.

	$ mkdir build-results
	$ cd build-results
	$ ../configure --with-cocoa --with-macosx-version-min=10.10 --with-macosx-sdk=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk --enable-debug --enable-debug_info --disable-shared --enable-cxx11 --disable-mediactrl CC=clang CXX=clang
	$ make -j4

(see http://forums.wxwidgets.org/viewtopic.php?f=19&t=37432 for more information about why you need to add the build options.  Also, we are disabling mediactrl because of issue http://trac.wxwidgets.org/ticket/17639.  Because of this we will not have media playback until we move to a wxWidgets that supports it.)

Finally, install wxWidgets libraries on you system.  The install step puts them into the /usr/local/bin (if you wanted to put somewhere different, use the --prefix option in configure but you're on your own).  You'll probably need to have admin rights to modify /usr/local/bin, so sudo is in order:

	$ sudo make install

Get calchart (you'll need to log in with your source forge account):

	$ git clone --recursive https://github.com/calband/calchart.git ~/calchart

If you need to rebuild the documentation, refer to the README-DOC.txt

Now open the xcode project in build-macosx/CalChart.xcodeproj.  If you installed to a different directory, or used different build options, you're settings may be slightly different than what CalChart.xcodeproj is expecting.  You can check to see the options that wx-config needs by running:

	$ wx-config --cxxflags
	$ wx-config --libs

Also, as the project uses precompiled headers, you may have to modify the
preconfigured header file path:
GCC_PREFIX_HEADER = /usr/local/include/wx-3.1/wx/wxprec.h

Build and run!
