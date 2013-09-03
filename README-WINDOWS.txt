My steps for building on Windows

Install windows with at least 20 gigs space and 512 megs mem.
Install all updates until it's happy

Get the latest cygwin (I used 1.7.25).  Set up with the standard instation, and add the following
packages:

subversion
make
bison
flex
patch
autoconf

Get Boost:
Go to boost.org and get the latest (I used boost 1.54).
Put it at c:\boost_1_54_0

Get Visual C++ 2012 Express:
I use Visual C++ 2012 Express because it supports c99 and it is free.  If you have the Pro version, feel free to use that.
Go to microsoft, download and install Visual C++ 2012 Express.

Download 2.9.5 of wxWidgets
Go to wxwidgets.org and download the exe installer (which will put the source at c:\wxWidgets-2.9.5

Make wxWidgets:
(I'm following the instructions at wxWidgets.org)
Open Visual C++ 2012 Express.
Open the project wx at wxWidgets-2.9.5\build\msw.  It should prompt you to migrate the project.
Note that there may be a error during the migration about not being able to make a backup.  That seems to be benign.
Choose the Debug configuration and Win32 configuration
Build the solution

Add environment variables
Go to Control Panel->System->Advanced System Settings->Environment Variables
Add the following User variables:
WXWIN=c:\wxWidgets-2.9.5
BOOST_DIR=c:\boost_1_54_0

Get calchart (you'll need to log in with your source forge account):
Open cygwin and run the following:
$ svn co https://calchart.svn.sourceforge.net/svnroot/calchart/trunk /cygdrive/c/calchart/trunk

Make Calchart's generated files:
$ cd ~/calchart/trunk
$ make generate

Make Calchart
Open Visual C++ 2012 Express.
Open the solution file:
...\calchart\trunk\build-win\CalChart\CalChart.sln
Build.  This will create the CalChart.exe.


