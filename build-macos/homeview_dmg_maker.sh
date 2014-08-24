#!/bin/bash

PROJECT=CalChartHomeView
VERSION=3.4.2
APP=CalChartHomeView.app
CONFIG=Debug
WC_DMG=wc.dmg
WC_DIR=wc

echo Creating a working copy for our DMG
bunzip2 -k calcharthomeview_template.dmg.bz2
cp calcharthomeview_template.dmg $WC_DMG
rm calcharthomeview_template.dmg

echo Mounting the disk image...
mkdir -p $WC_DIR
hdiutil attach "$WC_DMG" -noautoopen -quiet -mountpoint "$WC_DIR"

echo Copying $PROJECT to the disk image...
rm -rf "$WC_DIR/$APP";
ditto --rsrc "build/$CONFIG/$APP" "$WC_DIR/$APP"
hdiutil detach $WC_DIR -quiet -force
rm -rf $WC_DIR

echo Compressing the disk image...
rm -f $PROJECT-$VERSION.dmg
hdiutil convert $WC_DMG -format UDZO -imagekey zlib-level=9 -o $PROJECT-$VERSION.dmg
rm -rf $WC_DMG

echo Internet enbling the disk image...
hdiutil internet-enable $PROJECT-$VERSION.dmg

