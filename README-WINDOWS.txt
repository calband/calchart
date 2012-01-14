My steps for building on Windows

Install windows with at least 20 gigs space and 512 megs mem.
Install all updates until it's happy

Get the latest cygwin (I used 1.7.9-1).  Set up with the standard instation, and add the following
packages:

subversion
make
bison
flex
patch
autoconf

Get Boost:
Go to boost.org and get the latest (I used boost 1.48).
Put it at c:\boost_1_48_0

Get Visual C++ 2010 Express:
I use Visual C++ 2010 Express because it supports c99 and it is free.  If you have the Pro version, feel free to use that.
Go to microsoft, download and install Visual C++ 2010 Express.

Download 2.9.3 of wxWidgets
Go to wxwidgets.org and download the exe installer (which will put the source at c:\wxWidgets-2.9.3

Make wxWidgets:
One issue is that Visual C++ 2010 Express doesn't support the dsw projects supplied with wxWidgets.  I use Visual Studio Command Prompt to build wxWidgets instead.
Open Visual C++ 2010 Express.
Go To Tools->Visual Studio Command Prompt
From the prompt, go to the  wxWidgets directory, and invoke nmake:
c:> cd \wxWidgets-2.9.3\build\msw
c:\wxWidgets-2.9.3\build\msw> nmake -f makefile.vc BUILD=debug MONOLITHIC=0 SHARED=0 UNICODE=1

Add environment variables
Go to Control Panel->System->Advanced System Settings->Environment Variables
Add the following User variables:
WXWIN=c:\wxWidgets-2.9.3
BOOST_DIR=c:\boost_1_48_0

Get calchart (you'll need to log in with your source forge account):
Open cygwin and run the following:
$ svn co https://calchart.svn.sourceforge.net/svnroot/calchart/trunk /cygdrive/c/calchart/trunk

Make Calchart
Open Visual C++ 2010 Express.
Open the solution file:
...\calchart\trunk\build-win\CalChart\CalChart.sln
Build.  This will create the CalChart.exe.


