
my log of steps for building for MacOSX.
I've built with Snowleopard (10.6).

You need to have xcode installed (the developer DVD).

You will need the following projects from macports (or fink)
subversion
cvs
bison
flex
transfig

MacOSX should come with a version of wxWidgets built in.  10.6 comes with 
version 2.8.

But you will need the project, at least for tex2rtf which calchart uses
Download 2.8.10 of wxWidgets (I used the svn version by open cygwin and running
the command):
$ svn co http://svn.wxwidgets.org/svn/wx/wxWidgets/tags/WX_2_8_10 ~/wxWidgets-2.8.10

$ cd wxWidgets-2.8.10
$ ./configure --with-msw --enable-debug --enable-debug_gdb --disable-shared

CalChart uses the tex2rtf project that comes with wxWidgets:
Need to build the utilities too!
$ cd /cygdrive/c/wxWidget-2.8.10/utils
$ make tex2rtf
$ cp tex2rtf/src/tex2rtf.exe /usr/local/bin

Get calchart (you'll need to log in with your source forge account):
$ svn co https://calchart.svn.sourceforge.net/svnroot/calchart/trunk ~/calchart/trunk

Make Calchart's generated files.  I just do a preliminary version of make to
get it work
$ cd ~/calchart/trunk
$ make


Now open the xcode project in build-macosx/CalChart.xcodeproj
Build and run the project

