
My log of steps for building for windows

Install windows xp with at least 20 gigs space and 512 megs mem.
Install all updates until it's happy

Get cygwin 1.7.1.  Set up with the standard instation, and add the following
packages:

gcc-core
gcc-g++
gdb
subversion
make
cvs
bison
flex
transfig

Download 2.8.10 of wxWidgets (I used the svn version by open cygwin and running
the command):
$ svn co http://svn.wxwidgets.org/svn/wx/wxWidgets/tags/WX_2_8_10 /cygdrive/c/wxWidgets-2.8.10

Following these instructions (from http://wiki.wxwidgets.org/Cygwin):

To remove all traces of cygwin-dependence, I had to do the following (assuming you are building under bash)
$ export CFLAGS=-mno-cygwin
$ export CPPFLAGS=-mno-cygwin
$ export CXXFLAGS=-mno-cygwin
$ export LDFLAGS="-mno-cygwin -mwindows"
$ ./configure --with-msw --enable-debug --enable-debug_gdb --disable-shared
$ make

Add wxWidgets to your path:
$ export PATH=/cygdrive/c/wxWidgets-2.8.10:$PATH

CalChart uses the tex2rtf project that comes with wxWidgets:
Need to build the utilities too!
$ cd /cygdrive/c/wxWidget-2.8.10/utils
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

