# Steps for building on Windows

Install windows with at least 20 gigs space and 512 megs mem.
Install all updates until it's happy

## Get the Tools

Get the latest cygwin (I used 1.7.25).  Set up with the standard instation, and add the following
packages:

* git
* make
* bison
* flex

## Get Visual Studio Community 2015
We tried Visual Studio Community 2015.

## Download 3.1 of wxWidgets
Go to wxwidgets.org.  Download and run the Windows Installer (which will put the source at c:\wxWidgets-3.1.0)

## Make wxWidgets
Open the project wx_vc14.sln at wxWidgets-3.1.0\build\msw.  It should prompt you to migrate the project.
Build the following solutions

       Debug   | Win32
       Release | Win32

(additional instructions and information can be found at wiki.wxWidgets.org/Install)

## Download Boost
Visit http://www.boost.org/ and download the latest version of Boost. 
Place the downloaded Boost directory somewhere on your computer:

	e.g. c:\boost

Remember where you place this Boost directory. In the next section, I will refer to the directory as $YOUR_BOOST_DIRECTORY. In the example above, $YOUR_BOOST_DIRECTORY = `c:\boost`

## Add environment variables
Go to Control Panel->System->Advanced System Settings->Environment Variables
Add the following User variables:

	WXWIN=c:\wxWidgets-3.1.0
	BOOST_DIR=$YOUR_BOOST_DIRECTORY

(see the `Download Boost` section above for the meaning of $YOUR_BOOST_DIRECTORY)

## Get calchart (you'll need to log in with your source forge account):
Open cygwin and run the following:

	$ git clone https://github.com/calband/calchart.git /cygdrive/c/calchart

(or cd to whereever you installed calchart)

Make Calchart's generated files:

	$ make generate

## Make Calchart
Open Visual C++ Express 2013 for Windows Desktop.
Open the solution file:
...\calchart\build-win\CalChart\CalChart.sln
Build.  This will create the CalChart.exe.


