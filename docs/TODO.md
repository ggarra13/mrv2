TODO
====

Easy
----
- Fix color scheme on Loading entries and OCIO entries.
- Add Mute Audio to preferences.
- Remove unused icons from icons/ directory.
- Fix the .desktop file not showing in Ubuntu 22.04 LTS.

Medium
------
- Add comments to all source code (and clean up the code more).
- Sort Tools alphabetically in mrvToolsCallbacks (but avoid include conflicts).
- Rename mrvTools to mrvPanels and every Tool file.


Difficult
---------

- Add saving with audio.
- Add a Session file to store the images loaded.
- Verify the build scripts work for an M1 machine (how?  I only have an intel
  machine and I would need to reinstall all brew packages for arm64)
- Improve the genereation of thumbnails in the timeline.  We need to match
  the speed that YouTube has.  Also we still crash with pbuffers on Linux.
