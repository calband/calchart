~=~=~=~=~=~=~=~=~=~=~
Checklist

 []  Did you run clang format on all the files? (clang-format -style="{BasedOnStyle: webkit}" -i src/*)
 []  Did it build on both windows and mac?
 []  Did you open and close some files?

~=~=~=~=~=~=~=~=~=~=~

How to release calchart:

The current calchart version is 3.6.3.  In all commands below, substitute that number for $CCVER (meaning when you read $CCVER, type 3.6.3)

0.1 LATEST_RELEASE_NOTES.md will get posted to the release by our CI bot, so update what bugs have been fixed.

0.2 copy LATEST_RELEASE_NOTES.md into master README.md for the master record via this command:

```
awk '//; /^# Release notes/{while(getline<"LATEST_RELEASE_NOTES.md"){print}}' README.md > tmp && mv tmp README.md
```

Because you sometimes run into build issues on one platform instead of another, do a build before you do a tag:
1. Prebuild the project to make sure its working

2. Tag the depot

```
$ git tag -a v3.6.3 -m "calchart-3.6.3"
$ git push origin v3.6.3
```

This should trigger the github actions

4. Go to the git Go to the sourceforge page at http://sourceforge.net/projects/calchart
You'll need to be logged in as Administrator to modify the files.

5. Go to the sourceforge page at http://sourceforge.net/projects/calchart
You'll need to be logged in as Administrator to modify the files.

6. Go to Project Admin -> File Manager

7. Make a new directory for this version (click on the * next to root)

8. Upload the README.txt to the new folder.  Click on README.txt and edit the file details to signify it's a release notes.

9. Upload the CalChart-$CCVER.dmg to the folder.  Click on dmg and edit the file details to signify its platform is Mac (OS X) and set the release_notes for the file.

10. Repeat for Install-CalChart-$CCVER.exe.  Click on zip and edit the file details to signify its platform is Windows and set the release_notes for the file.

11. Update https://github.com/calband/calchart/releases with Readme version updates.

12. Update https://github.com/calband/calchart/milestone by closing the shipped milestone and starting the next one.

13. Have a cookie.

