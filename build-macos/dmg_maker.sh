#!/bin/bash

PROJECT=CalChart
VERSION=3.1.5
APP=CalChart.app

echo Making a disk image...
hdiutil create $PROJECT-$VERSION.dmg -volname $PROJECT-$VERSION -type UDIF -megabytes 50 -fs HFS+

echo Mounting the disk image...
hdiutil attach $PROJECT-$VERSION.dmg
MYDEV=/Volumes/$PROJECT-$VERSION
echo Device is $MYDEV

echo Copying $PROJECT to the disk image...
ditto --rsrc build/Debug/$APP /Volumes/$PROJECT-$VERSION/$APP
touch /Volumes/$PROJECT-$VERSION/"DRAG CALCHART TO APPLICATIONS FOLDER"

echo Unmountin the disk image...
hdiutil detach $MYDEV

echo Compressing the disk image...
hdiutil convert $PROJECT-$VERSION.dmg -format UDZO -o $PROJECT-$VERSION-compressed.dmg

echo Internet enbling the disk image...
hdiutil internet-enable $PROJECT-$VERSION-compressed.dmg

echo Renaming compressed image...
rm -f $PROJECT-$VERSION.dmg
mv $PROJECT-$VERSION-compressed.dmg $PROJECT-$VERSION.dmg
