Since the documentation for the calchart project is build from tex files, if you change them you must rebuild them.

These steps require having Lion 10.7 and XCode 4.2.1.  Both should be on the app store.

Follow the instructions for README-MACOSX.txt

Making the help document uses the tex2rtf project.  Unfortunately, tex2rtf is no longer supported in wxWidgets.
Download tex2rtf from sourceforge:
$ svn co https://tex2rtf.svn.sourceforge.net/svnroot/tex2rtf/trunk ~/tex2rtf/trunk
$ cd ~/tex2rtf/trunk
Because there are slight issues with the version in trunk, you'll need to patch up the trunk

$ patch -p0 < ~/calchart/trunk/tex2rtf_changes.diff

Configure and build tex2rtf
$ autoreconf --install
$ mkdir build-results
$ cd build-results
$ ../configure
$ make
$ sudo install ./src/tex2rtf /usr/local/bin

Make Calchart's help files.
$ cd ~/calchart/trunk
$ make html


