# My steps for building for MacOSX.

These steps were written with Yosemite 10.10 and XCode 6.1.1.  Both should be on the app store.  You *can* do ealier builds, but you may have to modify your install steps.

You also need to install the command line tools for Xcode.  See https://developer.apple.com/library/ios/technotes/tn2339/_index.html

Note: These instructions assume you will build from source.  If you want to download from HomeBrew or some other package utility, you will have to modify xcode project to link to the appropriate place.
Note: All lines starting with ‘$’ mean you should run them from the terminal.  Do not include the ‘$’ with the command :)

## Download and build 3.0.2 of wxWidgets
Go to wxwidgets.org and download the "Source for Linux, OS X".  You will then need to move the package to a known place (I recommend your desktop).
Make sure you unzip and untar the bundle (usually involves just double clicking the tar file.

To start the terminal, open Spotlight and search for terminal.

	$ cd ~/Downloads/wxWidgets-3.0.2

There is currently a known build issue for wxWidgets and 10.10 (see http://trac.wxwidgets.org/ticket/16329).  To address, download the patch (below I use curl to get the file directly from the web), and then apply the the patch:

	$ curl http://trac.wxwidgets.org/raw-attachment/ticket/16329/wx_webview.patch > wx_webview.patch
	$ patch -p0 < wx_webview.patch 

Now you can build.  You'll first need to create a work directory, then configure the project for your system, and finally make it.  Note, it may take some time and many resources to build, so you'll want to leave your computer alone for some time.

	$ mkdir build-results
	$ cd build-results
	$ ../configure --with-cocoa --with-macosx-version-min=10.7 --with-macosx-sdk=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.10.sdk --enable-debug --enable-debug_info --disable-shared CXXFLAGS="-std=c++11 -stdlib=libc++" OBJCXXFLAGS="-stdlib=libc++ -std=c++11" LDFLAGS=-stdlib=libc++
	$ make -j4

(see http://forums.wxwidgets.org/viewtopic.php?f=19&t=37432 for more information about why you need to add the build options.)

Finally, install wxWidgets libraries on you system.  The install step puts them into the /usr/local/bin (if you wanted to put somewhere different, use the --prefix option in configure but you're on your own).  You'll probably need to have admin rights to modify /usr/local/bin, so sudo is in order:

	$ sudo make install

Get calchart (you'll need to log in with your source forge account):

	$ git clone https://github.com/calband/calchart.git ~/calchart

If you need to rebuild the documentation, refer to the README-DOC.txt

## Make Calchart's generated files:

	$ cd ~/calchart
	$ make generate

Now open the xcode project in build-macosx/CalChart.xcodeproj.  If you installed to a different directory, or used different build options, you're settings may be slightly different than what CalChart.xcodeproj is expecting.  You can check to see the options that wx-config needs by running:

	$ wx-config --cxxflags
	$ wx-config --libs

Also, as the project uses precompiled headers, you may have to modify the
preconfigured header file path:
GCC_PREFIX_HEADER = /usr/local/include/wx-3.0/wx/wxprec.h

## Build and run the project.

