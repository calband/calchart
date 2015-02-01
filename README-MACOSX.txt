My steps for building for MacOSX.

These steps were written with Yosemite 10.10 and XCode 6.1.1.  Both should be on the app store.  You *can* do ealier builds, but you may have to modify your install steps.

You also need to install the command line tools for Xcode.  See https://developer.apple.com/library/ios/technotes/tn2339/_index.html

Note, these instructions assume you will build from source.  If you want to download from HomeBrew or some other package utility, you will have to modify xcode project to link to the appropriate place.

Download 3.0.2 of wxWidgets
Go to wxwidgets.org and download the "Source for Linux, OS X".  You will then need to move the package to a known place (I recommend your desktop).
Make sure you unzip and untar the bundle (usually involves just double clicking the tar file.

Note: All lines starting with ‘$’ mean you should run them from the terminal.  Do not include the ‘$’ with the command :)
To start the terminal, open Spotlight and search for terminal.

$ cd ~/Desktop/wxWidgets-3.0.2

$ mkdir build-results
$ cd build-results

http://trac.wxwidgets.org/ticket/16329

$ ../configure --with-cocoa --with-macosx-version-min=10.7 --with-macosx-sdk=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.10.sdk --enable-debug --enable-debug_info --disable-shared CXXFLAGS="-std=c++11 -stdlib=libc++" OBJCXXFLAGS="-stdlib=libc++ -std=c++11" LDFLAGS=-stdlib=libc++

(see http://forums.wxwidgets.org/viewtopic.php?f=19&t=37432 for more information about why you need to add the build options.)

$ make

put into the /usr/local/bin (if you wanted to put somewhere different, use the --prefix option in configure but you're on your own)
$ sudo make install

Get calchart (you'll need to log in with your source forge account):
$ git clone https://github.com/calband/calchart.git ~/calchart

If you need to rebuild the documentation, refer to the README-DOC.txt

Make Calchart's generated files:
$ cd ~/calchart
$ make generate

Now open the xcode project in build-macosx/CalChart.xcodeproj
Build and run the project

I modify the project to match up with the results of theses commands:
$ wx-config --cxxflags
$ wx-config --libs

Also, as the project uses precompiled headers, you may have to modify the
preconfigured header file path:
GCC_PREFIX_HEADER = /usr/local/include/wx-3.0/wx/wxprec.h

