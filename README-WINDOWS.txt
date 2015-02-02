My steps for building on Windows

Install windows with at least 20 gigs space and 512 megs mem.
Install all updates until it's happy

Get the latest cygwin (I used 1.7.25).  Set up with the standard instation, and add the following
packages:

git
subversion
make
bison
flex
patch
autoconf

Get Visual C++ Express 2013 for Windows Desktop:
I use Visual C++ Express 2013 for Windows Desktop because it supports c99 and it is free.  If you have the Pro version, feel free to use that.
Go to microsoft, download and install Visual C++ Express 2013 for Windows Desktop.

Download 3.0.2 of wxWidgets
Go to wxwidgets.org and download the exe installer (which will put the source at c:\wxWidgets-3.0.2

Make wxWidgets:
(I'm following the instructions at wiki.wxWidgets.org/Install)
Open Visual C++ Express 2013 for Windows Desktop.
Open the project wx_vc10.sln at wxWidgets-3.0.2\build\msw.  It should prompt you to migrate the project.
Build the following solutions
Debug   | Win32
Release | Win32

Add environment variables
Go to Control Panel->System->Advanced System Settings->Environment Variables
Add the following User variables:
WXWIN=c:\wxWidgets-3.0.2

Get calchart (you'll need to log in with your source forge account):
Open cygwin and run the following:
$ git clone https://github.com/calband/calband.git /cygdrive/c/calchart

Make Calchart's generated files:
$ cd /cygdrive/c/calchart
$ make generate

Make Calchart
Open Visual C++ Express 2013 for Windows Desktop.
Open the solution file:
...\calchart\build-win\CalChart\CalChart.sln
Build.  This will create the CalChart.exe.


