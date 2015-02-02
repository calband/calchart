Updated on 7/2/11 by Kyle Barlow
(note, these instructions are way out of date.)

Steps taken to get calchart to build on Ubuntu 10.04
Feel free to change directories to your liking

Packages to install:
$ sudo apt-get install subversion bison flex transfig

Packages for wxwidgets: (from http://wiki.wxwidgets.org/Installing_and_configuring_under_Ubuntu)
$ sudo apt-get install libwxgtk2.9.2-dev libwxgtk2.9.2-dbg

If you don't have a C++ compiler yet, you can install one by installing this package:
$ sudo apt-get install build-essential

Get calchart (you'll need to log in with your source forge account when you commit):
$ git clone https://github.com/calband/calchart.git ~/calchart

Build calchart
$ cd ~/calchart/trunk
$ make

