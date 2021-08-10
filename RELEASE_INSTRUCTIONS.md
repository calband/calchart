---

Checklist

 *  Did you run clang format on all the files? (`clang-format -style="{BasedOnStyle: webkit}" -i src/*`)
 *  Did it build on both windows and mac?
 *  Did you open and close some files?
 *  Did you open and close some files with images in them?

---

# How to release calchart:

CalChart uses CMake and Github actions to automate the release process.  When you push a tag for the repository, github will build and package the release.

The current calchart version is 3.6.4.  In all commands below, substitute that number for `$CCVER` (meaning when you read `$CCVER`, type 3.6.4)

 1. Prebuild the project to make sure its working

 2. Update LATEST_RELEASE_NOTES.md with all the changes (it will get posted to the release by our CI bot)

 3. copy LATEST_RELEASE_NOTES.md into master README.md for the master record via this command:

```
awk '//; /^# Release notes/{while(getline<"LATEST_RELEASE_NOTES.md"){print}}' README.md > tmp && mv tmp README.md
```

 4. Tag the depot

```
$ git tag -a v3.6.4 -m "calchart-3.6.4"
$ git push origin v3.6.4
```

This should trigger the github action, which should publish release notes in Draft form.

 5. Once the Release information looks good, Press the Publish Release button.

 6. Download the Release artifacts to your machine.

 7. Go to the sourceforge page at http://sourceforge.net/projects/calchart.  You'll need to be logged in as Administrator to modify the files.

 8. Go to Project Admin -> File Manager

 9. Make a new directory for this version (click on the * next to root)

 10. Upload the `README.md` to the new folder.  Click on `README.md` and edit the file details to signify it's a release notes.

 11. Upload the `CalChart-$CCVER.dmg` to the folder.  Click on dmg and edit the file details to signify its platform is Mac (OS X) and set the release_notes for the file.

 12. Repeat for `Install-CalChart-$CCVER.exe`.  Click on zip and edit the file details to signify its platform is Windows and set the release_notes for the file.

 13. Update https://github.com/calband/calchart/milestone by closing the shipped milestone and starting the next one.

 14. Notify the world about the new version of CalChart.

 15. Have a cookie.

