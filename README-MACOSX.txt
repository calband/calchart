My steps for building for MacOSX.  Updated for Lion (10.7)

These steps require having Lion 10.7 and XCode 4.1.  Both should be on the app store.

Most tools will already be on your system, but you will need the following projects from macports (or fink).  See http://www.macports.org/:
boost
transfig

Download 2.9.2 of wxWidgets to build the library:
$ svn co http://svn.wxwidgets.org/svn/wx/wxWidgets/tags/WX_2_9_2 ~/wxWidgets-2.9.2

$ cd wxWidgets-2.9.2
$ mkdir build-release
$ cd build-release

$ ../configure --with-cocoa --with-macosx-version-min=10.5 --with-macosx-sdk=/Developer/SDKs/MacOSX10.6.sdk --enable-debug --enable-debug_info --disable-shared
$ make

put into the /usr/local/bin (if you wanted to put somewhere different, use the --prefix option in configure and you're on your own)
$ make install

CalChart uses the tex2rtf project that comes with wxWidgets:
Need to build the utilities too!
$ cd build-debug/utils
$ make tex2rtf
$ sudo install tex2rtf/src/tex2rtf.exe /usr/local/bin

Get calchart (you'll need to log in with your source forge account):
$ svn co https://calchart.svn.sourceforge.net/svnroot/calchart/trunk ~/calchart/trunk

Make Calchart's generated files.  I just do a preliminary version of make to
get it work
$ cd ~/calchart/trunk
$ make


Now open the xcode project in build-macosx/CalChart.xcodeproj
Build and run the project

I modify the project to match up with the results of theses commands:
$ wx-config --cxxflags
$ wx-config --libs


