--- CalChart README.txt ---
Updated on 2/15/2014 by Richard Powell
---

CalChart version 3.4.0

---
What's new in this version?
---

Changed save file format.  Files saved in this version and beyond cannot be
opened by earlier versions of calchart.

Added a Print Continuity Editor.  This allows you to preview and edit what the
bottom part of the printed page.

Non-user facing changes:
  Moved to GitHub
  Updating to wxWidgets 3.0

Bugs and features fixed in this release:
#17 can not import a Mac continuity file
#27 Add a printed contiuity editor.

---
What is CalChart?
---

CalChart is the open source project created and maintained by members and
alumni of the California Marching Band.  We use the software to chart our 
marching shows.  This program allows us to plot how we want marchers to move,
and how formations to flow.  It is used to animate a show, and to print out
continuity (poop sheets).

Calchart uses several open source tools for building an executable for a target
system.  There are different UI windowing systems (like Cocoa for Mac, or GNOME
for linux, or QT for cross platform), but wxWidgets was chosen as a good
balance of features, cross-platform, and support.  This allows one version to
be built on Windows/MacOS/Linux, but also means that proprietary UI Kits like
iOS would be difficult to support, requiring a complete rewrite.  Currently,
Calchart uses wxWidgets version 3.0.

Calchart also has an internal compiling language for interpreting the
continuity commands into movement.  This language description is converted into
source code using bison and flex.  The Calchart documentation is done with a
group of tex files that are converted into HTML by some unsupported tools.  The
tex files can be re-edited and used to regenerate the documentation, but the
conversion tools are unsupported.  The generated HTML results are also
distributed in the developer package.

---
Helping Develop CalChart:
---

Visit the CalChart GitHub page:

CalChart is hosted on github.com in the Calband/CalChart section.  You can
think of this as the "developer" site of CalChart.

https://github.com/calband/calchart

note:
The CalChart sourceforge.net will be used for hosting CalChart files, but
principle development will be on GitHub.
http://sourceforge.net/projects/calchart/

----
Reporting a bug or requesting a new feature.
----

CalChart uses an issues page on the github site for tracking bugs and features.
To post issues, you will need to create a GitHub account at https://github.com.

From the CalChart project page (https://github.net/calband/calchart/),
click on Issues.  This should list all of the issues logged against CalChart.
Your bug may already be here.  If it is, then add any information to the bug.
If not, click on "New Issue" to add a new bug.

This will bring up the new Issue page.  Please enter a summary and a description
of the issue.  If you have any reproduction steps that will cause a bug to
happen, please enter them.  The more details, the more likely it is to fix.  If
you have a file that causes this to happen, or if you have any screen shots of
the problem, attach them to the bug.  If you are requesting a feature, add
anything that you think will help create the feature, such as describing the
desired behavior.

---
Build CalChart
---

For building on MacOSX platforms, see the README-MACOSX.txt.  For building on
Windows platforms, see the README-WINDOWS.txt.

If you need to regenerate the help documentation, see the README-docs.txt

----------

Release notes for 3.3.5:

Bugs fixed in this release:
#74: 3.3.4: Hang when opening Preferences

----------

Release notes for 3.3.4:

Non-user facing changes:
  General clean-up to separate core code from wxWidgets code
  Updating to wxWidgets 2.9.5
  Changes to Do/Undo system where we snapshot the show.  Increase memory usage, but simplifies the code.
  Updated for Mavericks and Window 7

Bugs fixed in this release:
3524581: Printing with continuity: scale font to fit

----------

Release notes for 3.3.3:

Bugs fixed in this release:

CalChart-3.3.2 corruption fixed
3547345: continuity files do not import correctly

----------

Release notes for 3.3.2:

Bugs and features fixed in this release:

3524205: printing to ps doesn't work on mac, file is corrupted
3524585: Rotate in omniview with q causes it to go upside-down	
3524583: Fix camera angel 3.
3524582: Adjust picture should be easier, seems to have issues.
3538572: tempo is 10 in Viewer on windows

feature:
3524594: add error as toolbars.
3524589: Change animation, comma, period for next/prev beat.
3524584: Should be able to have custom camera angles
3524592: Put Reset reference points back in
3524587: selection should behave same in animation and field view
3524588: Add short-cut for OmniView.
3322611: Export to MOV, AVI or something home viewable
3544446: continue marcher letters after 'Z' with  'AA' , 'BB'

----------

Release notes for 3.3.0:

What's new:

* CalChart Omniview.  'nuff said.

Bugs and features fixed in this release:

2965282: Integrate Omniviewer

----------

Release notes for 3.2.2:

What's new:

* Now you can put a background picture on the field.  Neat-o!
* Marchers are selectable in the animation screen.
* Updated icon.  Now less ugly!

Updated wxWidgets to use 2.9.3.

Bugs and features fixed in this release:

3315322: Show animation path
3018574: Need a better icon
3315313: Field view should have scrollbars
3018579: Able to set background
3315321: Marchers in animation window should be selectable
3442097: Put background image in middle of view
2943425: Need Installation steps for windows
2981575: Icon should show up on windows executable

----------

Release notes for 3.2.1:

Bugs and features fixed in this release

3315271: When the show gets closed and opened, it keeps growing?
3315318: Don't allow Edit Continuity to close without saving.
3315319: Show collision by default
3315320: full screen by default for animation window 
3315323: Autosave Path doesn't matter anymore, remove
3315337: Add show open history.

----------

Release notes for 3.2.0:

Major changes under the hood.  Switching to the wxDocument/wxView paradigm.

Features added:
3018591: move from config file to internal settings

Bugs Fixed in this release

3288634: Make sure go to pages works for all views
3218824: Titles are blank
3024169: Leaking CC_show with ever open and new shows
3018566: Ugly grey on the bottom of animation window
3018571: Dialog appears: Autosave error
3022228: Leaking CC_show with ever open and new shows

----------

Release notes for 3.1.5:

3023650: Does not prompt to save when you quit
fixing crash when pressing next on the Animation window.

----------

Release notes for 3.1.4:

Bugs Fixed in this release

2975951, Add help menu
3018568, Animation continuity input box to small
3018576, next/previous field hot-keys.
3018567, arrow keys not work in animation window (windows only)
3018565: stretching animation needs to refresh window
3018573, window legacy print is screwed up on windows

----------

Release notes for 3.1.3:

Features added:
2965268, selectable color palettes
2960367, Dot Count Feature
2965265, Make animation screen scalable
2947102, Print directly to Printers, PDF

Bugs Fixed in this release

3006744, Window flickers when drawing on Windows.
2977400, adding some hot-keys
2965264, make it so when you select something again, it unselects it

----------

Release notes for 3.1.2:

Beautified and reorganized code.  Fixed up lasso and polygon selector.

Bugs Fixed in this release

2977400, adding some hot-keys
2965264, make it so when you select something again, it unselects it.
2981576, Adding an icon for MAC (not a very good one...)
2964628, support for Leopard

Build tested by quickly animating guns2.shw on mac and windows.

----------

Release notes for 3.1.1:

Bugs Fixed in this release

2981574 hold-shift moving mouse scrolling scale is off
2981973 Animation different on windows/mac
2977399 Animation doesn't work
2976306 Adding the ability to open old mas files on mac.

Build tested by quickly animating guns2.shw on mac and windows.

