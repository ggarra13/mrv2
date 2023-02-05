TODO
====

# Bug fixes
-----------

## Third-part
- Fix .webm and TopGun.mp4 movies showing black in tlRender.


## Easy

- Fix color scheme on Loading preferences and OCIO entry preferences.
- Add Mute Audio to preferences.
- Remove unused icons from icons/ directory.
- Fix TLRENDER_VERSION macro which is set to "".
- Make the text tool not move wh en clicking inside the multilinewidget.
- Added HUD message display, like when playback starts/steops.


## Medium

- Add comments to all source code (and clean up the code more).
- Sort Tools alphabetically in mrvPanelsCallbacks (but avoid include conflicts).
- Improve the main UI to make it more streamlined?
- Change .prefs files to use JSON, instead of custom Windows .ini files?


## Difficult

- Add saving with audio.
- Verify the build scripts work for a macOS M1 machine
  (how? I only have an intel machine and I would need to reinstall all
   brew packages for arm64)
- Add Remote syncing/connections.
- Fix crash with pbuffers on Linux (I/O error or xcb_ error).  Fixed?
- Support annotations when in environment map mode.
