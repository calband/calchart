My steps for building for MacOSX.  Updated for Lion (10.7)

These steps require having Lion 10.7 and XCode 4.2.1.  Both should be on the app store.

Most tools will already be on your system, but you will need the following projects from macports (or fink).  See http://www.macports.org/:
boost
transfig

Download 2.9.3 of wxWidgets to build the library:
$ svn co http://svn.wxwidgets.org/svn/wx/wxWidgets/tags/WX_2_9_3 ~/wxWidgets-2.9.3

$ cd wxWidgets-2.9.3

Apply patch for issue:
[wxOSX-Cocoa] wxHtmlHelpWindow asserts about unsupported wxCB_SORT (issue 12419)
Go to http://trac.wxwidgets.org/ticket/12419
Download the patch (there is a link to the patch, and download in the "Original Format")
Patch via the patch command:
$ patch -p0 < ~/Downloads/helpwnd_combobox.patch 
(update to the location of the downloaded patch file)


$ mkdir build-results
$ cd build-results

$ ../configure --with-cocoa --with-macosx-version-min=10.5 --with-macosx-sdk=/Developer/SDKs/MacOSX10.6.sdk --enable-debug --enable-debug_info --disable-shared
$ make

put into the /usr/local/bin (if you wanted to put somewhere different, use the --prefix option in configure but you're on your own)
$ sudo make install

Get calchart (you'll need to log in with your source forge account):
$ svn co https://calchart.svn.sourceforge.net/svnroot/calchart/trunk ~/calchart/trunk

CalChart uses the tex2rtf project.  Unfortunately, tex2rtf is no longer supported in wxWidgets.
Download tex2rtf from sourceforge:
$ svn co https://tex2rtf.svn.sourceforge.net/svnroot/tex2rtf/trunk ~/tex2rtf/trunk
$ cd ~/tex2rtf/trunk
Because there are slight issues with the version in trunk, you'll need to patch up the trunk

$ patch -p0 < ~/calchart/trunk/tex2rtf_changes.diff

Configure and build tex2rtf (this is only if you are planning on rebuilding the documentation):
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


