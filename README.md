# What is CalChart?

CalChart is the open source project created and maintained by members and
alumni of the California Marching Band.  We use the software to chart our 
marching shows.  This program allows us to plot how we want marchers to move,
and how formations to flow.  It is used to animate a show, and to print out
continuity (poop sheets).

Calchart also has an internal compiling language for interpreting the
continuity commands into movement.  This language description is converted into
source code using bison and flex.  The Calchart documentation is done with a
group of tex files that are converted into HTML by some unsupported tools.  The
tex files can be re-edited and used to regenerate the documentation, but the
conversion tools are unsupported.  The generated HTML results are also
distributed in the developer package.

## Helping Develop CalChart:

CalChart is hosted on github.com in the Calband/CalChart section.  You can
think of this as the "developer" site of CalChart.

https://github.com/calband/calchart

For information on the CalChart Architecture, please refer to [Architecture Guide](README-architecture.md).

note:
The CalChart sourceforge.net will be used for hosting CalChart deliverables files, but
principle development will be on GitHub.

http://sourceforge.net/projects/calchart/

## Reporting a bug or requesting a new feature.

CalChart uses an issues page on the github site for tracking bugs and features.
To post issues, you will need to create a GitHub account at https://github.com.

From the CalChart project page (https://github.com/calband/calchart/),
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


## Building CalChart

Please check out the [Getting Started](GETTING_STARTED.md) guide.

If you need to regenerate the help documentation, see the [README-docs.txt](README-docs.txt).


# Release notes

## Release notes for 3.6.2

Bugs addressed in this release:

* [#333](../../issues/333) Field thumbnail seems to Pop
* [#335](../../issues/335) not able to change beats on step drills.

## Release notes for 3.6.1

Features addressed in this release:

* [#326](../../issues/326) Improving click and drag by not unselecting groups.
* [#327](../../issues/327) Adding sprites to Animation screen.
* [#331](../../issues/331) Adding ability to label Instruments

Bugs addressed in this release:

* [#323](../../issues/323) cannot open old calchart files when they have images.
* [#324](../../issues/324) Transition solver doesn't seem to add groups on Windows.
* [#328](../../issues/328) Point picker crashes if you open, close then quit a show

Other changes:

* [#322](../../issues/322) Increment wxWidgets to 3.1.4

## Release notes for 3.6.0

Features addressed in this release:

* General clean-up the UI.
* [#28](../../issues/28) set of color palettes
* [#177](../../issues/177) Should we get rid of "mode"
* [#196](../../issues/196) Could mode be per show, per stunt sheet.
* [#201](../../issues/201) Add Better Continuity editor, get away from text
* [#291](../../issues/291) CalChart should top-level several color choices
* [#297](../../issues/297) Look to use wxAuiManager
* [#301](../../issues/301) Add a better starting page

Bugs addressed in this release:

* [#279](../../issues/279) pressing on arrow keys doesn't seem to update thumbnails
* [#235](../../issues/235) Rotate is broken
* [#293](../../issues/293) Get rid of unused EPS printing.
* [#294](../../issues/294) Get rid of Spring Show mode to simplify
* [#307](../../issues/307) When importing a file with bad continuity, offer some way to correct it.
* [#308](../../issues/308) have calchart remember the size, position, and layout

Other changes:

* [#287](../../issues/287) Increment wxWidgets to 3.1.3
* [#303](../../issues/303) Switch to c++17
* [#309](../../issues/309) Animation view flickers sometimes

## Release notes for 3.5.4

* [#](../../issues/) Switch to CMake.

## Release notes for 3.5.3

* [#271](../../issues/271) Unreliable viewer file generation

## Release notes for 3.5.2

* [#248](../../issues/248) Unused continuities are not automatically deleted
* [#252](../../issues/252) Rotate Tool Snaps To Grid, Causing Strange Effects
* [#254](../../issues/254) put back cal chart viewer at top level menu
* [#255](../../issues/255) field preview scroll is slow on windows
* [#256](../../issues/256) Collapsing everything to a point, then spreading the point doesn't work
* [#258](../../issues/258) When printing, lines labels are "green"
* [#263](../../issues/263) Remove ContinuityEditor
* [#265](../../issues/265) Don't need a pop-up to tell me if I've cancelled printing
* [#267](../../issues/267) copy/pasting stuntsheets doesn't work (Windows)

## Release notes for 3.5.1

Features addressed in this release:

* General clean-up the UI.
* [#179](../../issues/179) Reset Ref points should obey selected
* [#187](../../issues/187) Have yard markers "stick" to the top edge
* [#236](../../issues/236) Relabel requires positions to be exact, makes it hard to use for script cal
* [#239](../../issues/239) See if we can preserve the order of dots when creating a line
* [#250](../../issues/250) Draw path should be in view.
* [#251](../../issues/251) Make edit menu have sub-section

Bugs addressed in this release:

* [#222](../../issues/222) Don't display the "there aren't enough labels" dialog
* [#235](../../issues/235) Rotate is broken
* [#249](../../issues/249) Genius move doesn't work


## Release notes for 3.5.0

Features addressed in this release:

* [#120](../../issues/120), [#121](../../issues/121) Save current_sheet and selection list with the show when saving
* [#209](../../issues/209) Delete one point
* [#198](../../issues/198) Shape Palette, and able to draw curves

Bugs addressed in this release:

* [#199](../../issues/199) Snap to grid when moving points
* [#203](../../issues/203) Removing unused Description Field
* [#11](../../issues/11) Background images should be undoable
* [#105](../../issues/105), [#104](../../issues/104) Background images should be part of the show

Build infrastructure issues addressed in this release:

* [#128](../../issues/128) clang-format all code


## Release notes for 3.4.4

Features addressed in this release:

* [#117](../../issues/117), [#139](../../issues/139), [#149](../../issues/149) CalChart Online Viewer File Exporter
* [#200](../../issues/200) Have keys move selected dot
* [#183](../../issues/183) Pinch to zoom
* [#185](../../issues/185) We should try out treating Next/previous and Selection as undoable commands
* [#178](../../issues/178) There should be a select all marchers command

Bugs addressed in this release:

* [#202](../../issues/202) Have new show column default to 8
* [#189](../../issues/189) Make sure to add scroll direction preference
* [#190](../../issues/190) Add scrolling to preference pane
* [#153](../../issues/153) Field doesn't redraw if the mode has changed
* [#206](../../issues/206) Viewer File Exporter: Format an Empty Array Where Manual Input is Required


Build infrastructure issues addressed in this release:

* [#184](../../issues/184) Revamp the command_do and undo
* [#174](../../issues/174) update to wxWidgets 3.1
* [#148](../../issues/148) Bring Back Boost
* [#155](../../issues/155) Update authors

## Release notes for 3.4.3

Features addressed in this release:

* [#103](../../issues/103) Preview where dots should go on "moves"
* [#109](../../issues/109) Improved Preferences and config
* [#114](../../issues/114) Auto-zoom on Animation
* [#116](../../issues/116) use wxWidgets 3.0.2

Bugs addressed in this release:

* [#19](../../issues/19)  Preferences - Reset-All should not require a close and open
* [#32](../../issues/32)  Make sure different versions play nicely
* [#92](../../issues/92)  Group Select Doesn't Properly Report Number of Selected Dots
* [#94](../../issues/94)  Inconsistent Use of Animation::GotoSheet(...)
* [#96](../../issues/96)  Occasional Unhandled Exception when Changing Dot Type in Continuity Editor
* [#98](../../issues/98)  Mac: cmd-c (copy) in Continuity editor closes instead of copies
* [#111](../../issues/111) "Edit print continuity" should allow tab entry
* [#115](../../issues/115) Preference color selection on Windows is screwed up

Build infrastructure issues addressed in this release:

* [#42](../../issues/42)  Use precompiled headers
* [#90](../../issues/90)  Build warnings with Windows
* [#122](../../issues/122) Build Release for distribution
* [#129](../../issues/129) use pragma once instead of ifdef header guards


## Release notes for 3.4.2

Bugs fixed and features added in this release:

* [#24](../../issues/24) Dot swap feature: You can swap two points on a stunt sheet
* [#29](../../issues/29) Add a ghost image of previous/next screen
* [#40](../../issues/40) Show Setup dialog: grey out letter labels when on numbers
* [#68](../../issues/68) Option to turn off point labels 
* [#77](../../issues/77) Omniview shows purple dots even when they're not colliding 
* [#84](../../issues/84) Copy and paste for pages in field view


## Release notes for 3.4.1

Add ability to insert stunt sheets from other shows.
Updated VisualStudio runtime to VC2013.

Bugs and features fixed in this release:

* [#20](../../issues/20) MAC: Crash when quitting from the help menu 
* [#23](../../issues/23) Add ability to insert stunt sheets from other shows
* [#66](../../issues/66) Ctrl + Click Scrolling
* [#67](../../issues/67) Include the most recent Visual C++ Redistribution with CalChart
* [#69](../../issues/69) Continuity editor saves to incorrect dot types
* [#71](../../issues/71) Saving/Loading shows with descriptions is broken


## Release notes for 3.4.0:

Changed save file format.  Files saved in this version and beyond cannot be
opened by earlier versions of calchart.

Added a Print Continuity Editor.  This allows you to preview and edit what the
bottom part of the printed page.

Non-user facing changes:

*  Moved to GitHub
*  Updating to wxWidgets 3.0

Bugs and features fixed in this release:

* [#7](../../issues/7) Make Point-Picker better
* [#15](../../issues/15) Windows: Always propted to save changes after edit cont
* [#16](../../issues/16) keyb buttons (right-left arrow) are ignored in view on win
* [#17](../../issues/17) can not import a Mac continuity file
* [#31](../../issues/31) Add a printed continuity editor.
* [#34](../../issues/34) Adding printed continuity to be saved with the file
* [#43](../../issues/43) Updated docs for GitHub
* [#53](../../issues/53) Removed Boost and used std lib wherever possible
* [#54](../../issues/54) Auto launching CalChart on startup on Windows.
* [#56](../../issues/56) Keyboard shortcut for Reanimate


## Release notes for 3.3.5:

Bugs fixed in this release:

* [#74](../../issues/74): 3.3.4: Hang when opening Preferences


## Release notes for 3.3.4:

Non-user facing changes:

*  General clean-up to separate core code from wxWidgets code
*  Updating to wxWidgets 2.9.5
*  Changes to Do/Undo system where we snapshot the show.  Increase memory usage, but simplifies the code.
*  Updated for Mavericks and Window 7

Bugs fixed in this release:

* 3524581: Printing with continuity: scale font to fit


## Release notes for 3.3.3:

Bugs fixed in this release:

* CalChart-3.3.2 corruption fixed
* 3547345: continuity files do not import correctly


## Release notes for 3.3.2:

Bugs and features fixed in this release:

* 3524205: printing to ps doesn't work on mac, file is corrupted
* 3524585: Rotate in omniview with q causes it to go upside-down	
* 3524583: Fix camera angel 3.
* 3524582: Adjust picture should be easier, seems to have issues.
* 3538572: tempo is 10 in Viewer on windows

feature:

* 3524594: add error as toolbars.
* 3524589: Change animation, comma, period for next/prev beat.
* 3524584: Should be able to have custom camera angles
* 3524592: Put Reset reference points back in
* 3524587: selection should behave same in animation and field view
* 3524588: Add short-cut for OmniView.
* 3322611: Export to MOV, AVI or something home viewable
* 3544446: continue marcher letters after 'Z' with  'AA' , 'BB'


## Release notes for 3.3.0:

What's new:

* CalChart Omniview.  'nuff said.

Bugs and features fixed in this release:

* 2965282: Integrate Omniviewer


## Release notes for 3.2.2:

What's new:

* Now you can put a background picture on the field.  Neat-o!
* Marchers are selectable in the animation screen.
* Updated icon.  Now less ugly!
* Updated wxWidgets to use 2.9.3.

Bugs and features fixed in this release:

* 3315322: Show animation path
* 3018574: Need a better icon
* 3315313: Field view should have scrollbars
* 3018579: Able to set background
* 3315321: Marchers in animation window should be selectable
* 3442097: Put background image in middle of view
* 2943425: Need Installation steps for windows
* 2981575: Icon should show up on windows executable


## Release notes for 3.2.1:

Bugs and features fixed in this release

* 3315271: When the show gets closed and opened, it keeps growing?
* 3315318: Don't allow Edit Continuity to close without saving.
* 3315319: Show collision by default
* 3315320: full screen by default for animation window 
* 3315323: Autosave Path doesn't matter anymore, remove
* 3315337: Add show open history.


## Release notes for 3.2.0:

Major changes under the hood:

* Switching to the wxDocument/wxView paradigm.

Features added:

* 3018591: move from config file to internal settings

Bugs Fixed in this release

* 3288634: Make sure go to pages works for all views
* 3218824: Titles are blank
* 3024169: Leaking CC_show with ever open and new shows
* 3018566: Ugly grey on the bottom of animation window
* 3018571: Dialog appears: Autosave error
* 3022228: Leaking CC_show with ever open and new shows


##  Release notes for 3.1.5:

* 3023650: Does not prompt to save when you quit
fixing crash when pressing next on the Animation window.


## Release notes for 3.1.4:

Bugs Fixed in this release

* 2975951, Add help menu
* 3018568, Animation continuity input box to small
* 3018576, next/previous field hot-keys.
* 3018567, arrow keys not work in animation window (windows only)
* 3018565: stretching animation needs to refresh window
* 3018573, window legacy print is screwed up on windows


## Release notes for 3.1.3:

Features added:

* 2965268, selectable color palettes
* 2960367, Dot Count Feature
* 2965265, Make animation screen scalable
* 2947102, Print directly to Printers, PDF

Bugs Fixed in this release

* 3006744, Window flickers when drawing on Windows.
* 2977400, adding some hot-keys
* 2965264, make it so when you select something again, it unselects it


## Release notes for 3.1.2:

Beautified and reorganized code.  Fixed up lasso and polygon selector.

Bugs Fixed in this release

* 2977400, adding some hot-keys
* 2965264, make it so when you select something again, it unselects it.
* 2981576, Adding an icon for MAC (not a very good one...)
* 2964628, support for Leopard

Build tested by quickly animating guns2.shw on mac and windows.


## Release notes for 3.1.1:

Bugs Fixed in this release

* 2981574 hold-shift moving mouse scrolling scale is off
* 2981973 Animation different on windows/mac
* 2977399 Animation doesn't work
* 2976306 Adding the ability to open old mas files on mac.

Build tested by quickly animating guns2.shw on mac and windows.
