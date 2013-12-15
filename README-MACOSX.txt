My steps for building for MacOSX.  Updated for Mountain Lion (10.8)

These steps require having Mountain Lion 10.8 and XCode 4.6.3.  Both should be on the app store.

Most tools will already be on your system, but you will need the following projects from macports (or fink).  See http://www.macports.org/:
boost

Download 2.9.5 of wxWidgets to build the library:
$ svn co http://svn.wxwidgets.org/svn/wx/wxWidgets/tags/WX_2_9_5 ~/wxWidgets-2.9.5

$ cd wxWidgets-2.9.5

$ mkdir build-results
$ cd build-results

$ ../configure --with-cocoa --with-macosx-version-min=10.6 --with-macosx-sdk=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk --enable-debug --enable-debug_info --disable-shared
$ make

put into the /usr/local/bin (if you wanted to put somewhere different, use the --prefix option in configure but you're on your own)
$ sudo make install

Get calchart (you'll need to log in with your source forge account):
$ svn co https://calchart.svn.sourceforge.net/svnroot/calchart/trunk ~/calchart/trunk

Configure and build tex2rtf (this is only if you are planning on rebuilding the documentation):
CalChart uses the tex2rtf project.  Unfortunately, tex2rtf is no longer supported in wxWidgets.
Download tex2rtf from sourceforge:
$ svn co https://tex2rtf.svn.sourceforge.net/svnroot/tex2rtf/trunk ~/tex2rtf/trunk
$ cd ~/tex2rtf/trunk
Because there are slight issues with the version in trunk, you'll need to patch up the trunk
$ patch -p0 < ~/calchart/trunk/tex2rtf_changes.diff

$ autoreconf --install
$ mkdir build-results
$ cd build-results
$ ../configure
$ make
$ sudo install ./src/tex2rtf /usr/local/bin

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
GCC_PREFIX_HEADER = /usr/local/include/wx-2.9/wx/wxprec.h

