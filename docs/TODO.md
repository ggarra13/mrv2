TODO
====

# Bug fixes
-----------

## Third-part
- Fix .webm and TopGun.mp4 movies showing black in tlRender.
- Fix crash on Windows when loading webm (vpx) movies.
- Improve the generation of thumbnails in the timeline.  We need to match
  the speed that YouTube has.


## Easy

- Fix color scheme on Loading entries and OCIO entries.
- Add Mute Audio to preferences.
- Remove unused icons from icons/ directory.
- Add file associations on Windows installer.
- Fix TLRENDER_VERSION macro which is set to "".
- Make the text tool not move when clicking inside the multilinewidget.


## Medium

- Add comments to all source code (and clean up the code more).
- Sort Tools alphabetically in mrvToolsCallbacks (but avoid include conflicts).
- Rename mrvTools to mrvPanels and every Tool file.
- Improve the main UI to make it more streamlined?


## Difficult

- Add saving with audio.
- Verify the build scripts work for a macOS M1 machine
  (how? I only have an intel machine and I would need to reinstall all
   brew packages for arm64)
- Add Remote syncing/connections.
- Fix crash with pbuffers on Linux (I/O error or xcb_ error).
