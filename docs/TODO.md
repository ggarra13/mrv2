TODO
====

# Bug fixes
-----------

## Third-part
- Fix .webm and TopGun.mp4 movies showing black in tlRender.


## Easy

- Add Mute Audio to preferences.
- Remove unused icons from icons/ directory.
- Fix TLRENDER_VERSION macro which is set to "".
- Added HUD message display, like when playback starts/stops.


## Medium

- Add comments to all source code (and clean up the code more).
- Sort Tools alphabetically in mrvPanelsCallbacks (but avoid include conflicts).


## Difficult

- Add saving with audio.
- Verify the build scripts work for a macOS M1 machine
  (how? I only have an intel machine and I would need to reinstall all
   brew packages for arm64)
- Add Remote syncing/connections.
