My steps for building for MacOSX.  Updated for Mavericks (10.9)

These steps require having Mavericks 10.9 and XCode 5.0.2.  Both should be on the app store.

Download 3.0.0 of wxWidgets
Go to wxwidgets.org and download the "Source for Linux, OS X".  You will then need to move the package to a known place (I recommend your desktop).
Make sure you unzip and untar the bundle (usually involves just double clicking the tar file.

Note: All lines starting with ‘$’ mean you should run them from the terminal.  Do not include the ‘$’ with the command :)
To start the terminal, open Spotlight and search for terminal.

$ cd ~/Desktop/wxWidgets-3.0.0

$ mkdir build-results
$ cd build-results

$ ../configure --with-cocoa --with-macosx-version-min=10.7 --with-macosx-sdk=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk --enable-debug --enable-debug_info --disable-shared CXXFLAGS="-std=c++11 -stdlib=libc++" OBJCXXFLAGS="-stdlib=libc++ -std=c++11" LDFLAGS=-stdlib=libc++

(see http://forums.wxwidgets.org/viewtopic.php?f=19&t=37432 for more information about why you need to add the build options.)

$ make

put into the /usr/local/bin (if you wanted to put somewhere different, use the --prefix option in configure but you're on your own)
$ sudo make install

Get calchart (you'll need to log in with your source forge account):
$ git clone https://github.com/calband/calchart.git ~/calchart

If you need to rebuild the documentation, refer to the README-DOC.txt

Make Calchart's generated files:
$ cd ~/calchart/trunk
$ make generate

Now open the xcode project in build-macosx/CalChart.xcodeproj
Build and run the project
Do the same for CalChartViewer

I modify the project to match up with the results of theses commands:
$ wx-config --cxxflags
$ wx-config --libs

Also, as the project uses precompiled headers, you may have to modify the
preconfigured header file path:
GCC_PREFIX_HEADER = /usr/local/include/wx-3.0/wx/wxprec.h

