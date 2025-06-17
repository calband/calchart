## Release notes for 3.8.0

Bugs addressed in this release:

* [#582](../../issues/582) Why does CalChart still say it's from an unknown developer when launch on Mac?
* [#593](../../issues/593) Are the draw colors for selected and reference inverted on the drawing setup
* [#596](../../issues/596) Did flip labels break?
* [#598](../../issues/598) ASAN crashes when drawing sprites
* [#599](../../issues/599) Selected marchers on field don't match animation
* [#600](../../issues/600) click on Sprites in animation view doesn't automatically update the view
* [#624](../../issues/624) Why are all shows showing there are collisions
* [#629](../../issues/629) Crash when using transition solver

Other changes:

* [#590](../../issues/590) Move things related to drawing a sheet to sheet so it can generate Draw commands
* [#602](../../issues/602) Have Background images use CalChart::Draw
* [#602](../../issues/602) Have Background images use CalChart::Draw
* [#608](../../issues/608) Add the ability to draw a curve in CalChart
* [#613](../../issues/613) Have a way to save Curves in the show
* [#615](../../issues/615) Update to latest wxWidgets
* [#617](../../issues/617) Don't have GetBackgroundImages return std::vector<ImageInfo> by reference
* [#619](../../issues/619) fix failing builds with png
* [#622](../../issues/622) change beats_t to Beats
* [#628](../../issues/628) Bump wxWidgets to 3.2.8 and wxUI to 0.2.0
* [#632](../../issues/632) Make PointPicker more generic
* [#634](../../issues/634) change ImportPrintableContinuity to return optional
* [#636](../../issues/636) introduce MarcherIndex to clarify what type an argument is
* [#638](../../issues/638) Move tests into CMake Tests
* [#640](../../issues/640) Clean up some of the CalChartAngles.h clang-tidy errors
* [#643](../../issues/643) Remove sheet as point friend
* [#645](../../issues/645) Adopt wxUI Menu widely
* [#649](../../issues/649) It is too hard to select curve control buttons.

