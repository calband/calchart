Updated on 7/2/11 by Kyle Barlow

Steps taken to get calchart to build on Ubuntu 10.04
Feel free to change directories to your liking

Packages to install:
$ sudo apt-get install subversion bison flex transfig

Packages for wxwidgets: (from http://wiki.wxwidgets.org/Installing_and_configuring_under_Ubuntu)
$ sudo apt-get install libwxgtk2.9.2-dev libwxgtk2.9.2-dbg

Boost libraries:
$ sudo apt-get install libboost-all-dev

If you don't have a C++ compiler yet, you can install one by installing this package:
$ sudo apt-get install build-essential

The libwx packages do not include tex2rtf, so you will need to download 2.8.10 of wxWidgets:
$ svn co http://svn.wxwidgets.org/svn/wx/wxWidgets/tags/WX_2_8_10 ~/wxWidgets-2.8.10

$ cd ~/wxWidgets-2.8.10/utils/tex2rtf/src
$ mv makefile.unx makefile
$ make
$ sudo mv tex2rtf /usr/bin

Get calchart (you'll need to log in with your source forge account when you commit):
$ svn co https://calchart.svn.sourceforge.net/svnroot/calchart/trunk ~/calchart/trunk

Build calchart
$ cd ~/calchart/trunk
$ make
