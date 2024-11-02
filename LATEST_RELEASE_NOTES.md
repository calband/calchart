## Release notes for 3.7.1

Bugs addressed in this release:

* [#443](../../issues/443) Dark Mode doesn't show status bar on Mac
* [#497](../../issues/497) Animate ctor should be explicit
* [#518](../../issues/518) Animation looks wrong

Other changes:

* [#492](../../issues/492) calchart_cmd should have an option for dumping json
* [#494](../../issues/494) Refine the calchart_cmd for dump_continuity_text
* [#498](../../issues/498) remove GetAnimation
* [#500](../../issues/500) Add JSON tests to sanity tests
* [#502](../../issues/502) remove get animation sheet iterator
* [#504](../../issues/504) Make Animation::NextBeat private
* [#506](../../issues/506) remove index from AnimateInfo
* [#510](../../issues/510) Switch the underlying Coord system to int32
* [#512](../../issues/512) Animation commands should contain the position, not the beat
* [#521](../../issues/521) Degree should have special constructors for common directions
* [#526](../../issues/526) We should have a sanity test for ps output
* [#528](../../issues/528) Clean up postscript files to be more value like
* [#530](../../issues/530) Tidy up CalChartShow.h
* [#532](../../issues/532) tidy up CalChartFileFormat
* [#534](../../issues/534) remove unused Token from Continuity parsing
* [#536](../../issues/536) Change the animation objects to be within the Animate namespace
* [#538](../../issues/538) Remove unused cont symbol from animate errors

