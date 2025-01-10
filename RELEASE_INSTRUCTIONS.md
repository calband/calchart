---

Checklist

 *  Did you run clang format on all the files? (`clang-format -style="{BasedOnStyle: webkit}" -i src/*`)
 *  Did it build on both windows and mac?
 *  Did you open and close some files?
 *  Did you open and close some files with images in them?

---

# How to release calchart:

CalChart uses CMake and Github actions to automate the release process.  When you push a tag for the repository, github will build and package the release.

The current calchart version is 3.7.2.  In all commands below, substitute that number for `$CCVER` (meaning when you read `$CCVER`, type 3.7.2).  When incrementing, be sure to consider if it's time to bump the MINOR version.  Only do that if there's incompatibilities, such as a new feature that won't work on a previous version of CalChart.

 1. Create a git branch to prep the changes: `git checkout -b dev/prep_$CCVER`

 2. Prebuild the project to make sure its working

 3. Update LATEST_RELEASE_NOTES.md with all the changes (it will get posted to the release by our CI bot)

 4. copy LATEST_RELEASE_NOTES.md into master README.md for the master record via this command:

```
awk '//; /^# Release notes/{while(getline<"LATEST_RELEASE_NOTES.md"){print}}' README.md > tmp && mv tmp README.md
```

 5. Merge the branch into main

 6. Tag the depot

```
$ git tag -a v3.7.2 -m "calchart-3.7.2"
$ git push origin v3.7.2
```

This should trigger the github action, which should publish release notes in Draft form.

 7. Once the Release information looks good, Press the Publish Release button.

 8. Download the Release artifacts to your machine.

 9. Go to the sourceforge page at http://sourceforge.net/projects/calchart.  You'll need to be logged in as Administrator to modify the files.

 10. Go to Project Admin -> File Manager

 11. Make a new directory for this version (click on the * next to root)

 12. Upload the `README.md` to the new folder.  Click on `README.md` and edit the file details to signify it's a release notes.

 13. Upload the `CalChart-$CCVER.dmg` to the folder.  Click on dmg and edit the file details to signify its platform is Mac (OS X) and set the release_notes for the file.

 14. Repeat for `Install-CalChart-$CCVER.exe`.  Click on zip and edit the file details to signify its platform is Windows and set the release_notes for the file.

 15. Update https://github.com/calband/calchart/milestones by closing the shipped milestone and starting the next one.

 16. Create a git branch to pre-prep the next version: `git checkout -b dev/pre_prep_$CCVER+1`

 17. Clear out LATEST_RELEASE_NOTES.md for next development effort. Update $CCVER+1 in RELEASE_INSTRUCTIONS.md.

 18. Merge dev/pre_prep

 19. Notify the world about the new version of CalChart.

 20. Have a cookie.

