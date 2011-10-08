
My log of steps for building for windows

Install windows xp with at least 20 gigs space and 512 megs mem.
Install all updates until it's happy

Get cygwin 1.7.9-1.  Set up with the standard instation, and add the following
packages:

gcc-core
gcc-g++
mingw64-i686-gcc
mingw64-i686-gcc-core
mingw64-i686-gcc-g++ 
gdb
subversion
make
bison
flex
transfig
boost
libboost

Download 2.9.2 of wxWidgets (I used the svn version by open cygwin and running
the command):
$ svn co http://svn.wxwidgets.org/svn/wx/wxWidgets/tags/WX_2_9_2 /cygdrive/c/wxWidgets-2.9.2

Make a sub-directory for building wxWidgets:
$ cd /cygdrive/c/wxWidgets-2.9.2
$ mkdir build-debug
$ cd build-debug

Configure wxWidgets.  I use mingw64-i686-gcc family (from http://wxwidgets.blogspot.com/2011_06_01_archive.html) to build without requiring cygwin.dll:
$ export LDFLAGS="-static-libgcc -static-libstdc++"
$ ../configure --host=i686-w64-mingw32 --build=i686-pc-cygwin --with-msw --enable-debug --enable-debug_info --disable-shared

Make wxWidgets:
$ make

Add wxWidgets to your path:
$ export PATH=$PATH:/cygdrive/c/wxWidgets-2.9.2/build-debug

CalChart uses the tex2rtf project that comes with wxWidgets:
Need to build the utilities too!
$ cd /cygdrive/c/wxWidget-2.9.2/utils
$ make tex2rtf
$ cp tex2rtf/src/tex2rtf.exe /usr/local/bin

Get calchart (you'll need to log in with your source forge account):
$ svn co https://calchart.svn.sourceforge.net/svnroot/calchart/trunk /cygdrive/c/calchart/trunk

Make Calchart
Note: You must have the CFLAGS export as above (-mno-cygwin).  The calchart 
makefiles uses that ENV variable
$ cd /cygdrive/c/calchart/trunk
$ make

You should now have an calchart.exe and a runtime direct.  These must be in the
same executable path.

