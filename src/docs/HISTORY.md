v1.5.2
======
mrv2 and vmrv2 are open source professional players and review tools for VFX, animation and computer graphics for Windows, Linux and macOS.  You can choose to compile from source or get binaries.

If you are unsure what binary to choose, go to:

https://mrv2.sourceforge.io/downloads/mrv2-download-page.html

mrv2 ships now in two compiled versions:

   - mrv2 with OpenGL backend  (free and donationware)   
   - vmrv2 with Vulkan backend (free and donationware)

It also ships for many more architectures, so be careful to download the correct one.
It has NOT been tested on Windows aarch64 nor Linux aarch64 (beta testers wanted).

Prices for binaries
-------------------

Donationware prices of binary licenses through PayPal:

[![Donate](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=UJMHRRKYCPXYW)

I use the email information to contact you privately.  I don't sell your information, as I don't have access to it, except for your email, which I use to contact you if you run into issues.

- u$  25 for a Solo node-locked license for one year.
- u$  50 for a Standard node-locked standard license for one year.
- u$  75 for a Edit node-locked license with annotations and editing tools for one year.
- u$ 150 for a Pro node-locked license to own.
- u$ 300 for a Pro+ node-locked license to own (monthly).

License works for both mrv2 and vmrv2 (you can have both installed).
You need to have an internet connection for the license system.

The donationware version when running without a donation, does not have:

    - Annotations (available from Solo and later)
    - Python (available in Standard and later)
    - Editing (available in Edit and later)
    - Voice and Link Annotations (available in Pro)
    - Move license to a new machine (available in Pro monthly)

They do have, however:

    - Tone-mapping (OpenGL) and HDR (Vulkan)
    - OpenEXR layer switching
    - Saving Images and Movies with Audio

Prices might change (go up or down) depending on competition and new features.

ChangeLog
---------

- Made installer's smaller on Windows by getting rid of .pdb files.
- Made creating package installers on Windows faster.
- Fixed a regresion on FLTK Windows resizing.
- Consolidated tltimelineui for both Vulkan and OpenGL under a single code base.- Refactored all Options classes in mrvNetwork to mrvOptions.
- Made clicking twice on the Status Bar bring up the Logs panel to the front.


v1.5.1
======

ChangeLog
---------

- Added Edit/Video Gap/Insert to mimic the audio gaps.
- Added Edit/Remove Gap/Insert to mimic the audio gaps.
- Made it possible to insert gaps even when the other track also had a gap instead of a clip.
- Fixed Dissolves on both the OpenGL and Vulkan using wrong blending flags leading to mid-dark transitions.
- Fixed Overlay.otio and multiple tracks drawing on Vulkan backend.
- Made Roll and Trim work properly when no available ranges are set in the clip.
- Made Roll to work properly with transitions.
- Added Slip Edit tool to modify clips' start times in place.
- Created a nice web page for easier downloading as GitHub was collapsing all the files and leading users to not download the Vulkan vmrv2.
- Removed the annoying start up Donationware license_helper window at the start.
  Now, for unlocking features, you should go to:
  
       Help->Unlock Features.


v1.5.0
======

ChangeLog
---------

- Improved Vulkan's Text Editor.
- Besides test.0001.exr, added support for sequences as test.%04d.exr and test.####.exr.
- Improved Vulkan performance on macOS Intel and macOS M1+.
- Improved Vulkan performance on Linux and Windows.
- Removed the time parameter from Python's getVoiceAnnotations as it was not used.
- Fixed Pro's Voice Over preferences not getting saved.
- Added Python's getVoiceOverAnnotationsJSON to get voice over data and mouse moves as a simple JSON file.
- Improved performance of OTIO timeline, getting refreshed unneedlessly.
- Fixed OTIO saving from the UI which had gotten broken.  Python saving was fine.
- Added adding of transitions to the Edit edition of mrv2/vmrv2.
- Fixed macOS' vmrv2 macOS Intel and M1 installers not working.
- Fixed license_helper's buttons sometimes requiring two presses.
- Updated license_helper's information.
- For installation on each platform, please refer to:

	https://github.com/ggarra13/mrv2?tab=readme-ov-file#notes-on-installation
	

v1.4.9
======

ChangeLog
---------

- Improved performance of Vulkan playback of RGB_F16 and RGB_F32 images.
  Now you can achieve 60 FPS with any card, at the cost of some memory.
  If your card supports fast compute shaders, you can keep working with
  RGB_F16 and RGB_F32 images.  Just set:

             Preferences->HDR->Prefer RGB->Use RGB.
	  
- Added support for Darby's toucan:Dissolves.
- Fixed Roll and Trim being reversed.
- Added support for .webp images and image sequences.  Note that animated .webp pictures are NOT supported.
- Move, Trim and Roll commands on the Edit version of mrv2/vmrv2 are now fully
  functional.
- Fixed a mutex bug in Vulkan's vmrv2.
- Fixed progress bar on loading a movie file or sequence of frames.
- Fixed a memory leak when dealing with storage images and storage buffers on Vulkan.
- Made OpenUSD compile on Modern Linux distros like Ubuntu 25.10.
- Updated OpenUSD to v25.11 on the OpenGL backend.
- Updated Python to v3.12.12.
- Fixed OpenEXR layer switching on Standard and later licenses.

v1.4.8
======

ChangeLog
---------

A quick release to fix some problems of v1.4.7.

- Fixed thumbnail menu not displaying the current size correctly.
- Fixed OpenColorIO's for RGB32F textures on Vulkan build.


v1.4.7
======

ChangeLog
---------

- Re-release v1.4.7 on 12-16-2025 due to problems with OpenColorIO.
- Made blue pixel color of Pixel Color Bar more readable against the black
  background.
- Fixed compilation without libplacebo when BUILD_PYTHON is off.
- Added TLRENDER_SSL only compilation for MRV2_NETWORK license check, without
  full playing of clips from the network.
- Added OpenSSL version to the About window.
- Fixed Floating panels on (K)Ubuntu 25.10+.
- Added support for pen/tablet on macOS (tested).
- Fixed Vulkan HDR support on Apple's M1+ which was not working.
- Refactored Wayland's Always On Top hotkey code.
- Added libwayland's version to Help/About.
- Added Vulkan's GPU card and driver versions to Help/About (HW tab).
- Added Donate button on the license_helper, with simple donations.
- Verified vmrv2 and mrv2 work on Kwin.
- Fixed distributing libvulkan.so on Unix.  macOS and Windows
  needs distributing it..
- One feature of FLTK's python has changed.  You can no longer use:
      ```from fltk import Fl
        Fl.check()```
  as the Fl module is gone.  It is replaced by:
     ```import fltk
      import fltk as Fl
      Fl.check()```
- Fixed libjpeg-turbo compilation on aarch64 runners.
- Fixed OpenJPH compilation on aarch64 runners.
- Upgraded OpenJPH version to v0.25.3.
- Made Vulkan compilation faster skipping OpenGL's tlRender files.
- Added downloading only when updating mrv2 or vmrv2.
- Fixed UTF8toUTF32 and UTF32toUTF8 code from Darby.
- Print out Vulkan color space and format detected when not in HDR.
- Added Vulkan card name and drivers to Help->About->HW.
- Improved utf8 to utf32 conversion and viceversa.
- Keep the number for dropped frames after stopping playback.
- Fixed an issue on macOS M1 that would (possibly) make vrmv2 not work on M1+.
- Fixed Vulkan MAILBOX issue on Gnome49 / Kwin6.4.5 (Ubuntu 25.10).
- Added MoltenVK version to vmrv2 on macOS.
- Fixed ussage of old vulkan-1.dll on Windows' machines on beta (Github) builds.
- Fixed a number of issues with aarch64 compilations.  Previous builds seem to have been AMD64 stubs.
- Added shipping Python opentimelineio modules within mrv2/vmrv2.
- Fixed FLU file requester's on Linux putting the filename in the directory path after one save.
- Sped up Vulkan's playback of RGB_F16 and RGB_F32 sequences at 4K to 40 FPS,
  from 15FPS.  Still work to do to reach 60FPS, like OpenGL.  


v1.4.6
======

- By popular demand, the DEMO / BETA versions no longer log you off after a number of minutes.  The Vulkan port still supports HDR and the OpenGL port supports tone mapping.
- Fixed certs directory on macOS install.
- Fixed double clicking and Open With on macOS.
- Fixed URL Link requester's buttons going off screen.
- Fixed Progress Report crash when loading multiple small clips in a single call from the file requester.
- Cleaning up FLU's file requester (used on Linux).
- Fixed FLU's file requester not accepting double clicks to accept a file.
- Fixed AutoPlayback when loading multiple clips.
- Fixed Wayland's Kwin decorations to use libdecor to allow subwindows to be dragged outside the main window.
- Simplified MacOS launcher and made it work without "exec" Window.
- Started work on OTIO dissolves editing.  Lots of work to do.
- Minor speed up in Edit viewport on OpenGL backend.
- Improved speed of compilation of Vulkan backend.
- OTIO transitions (dissolves) can now be moved and trimmed.
- OTIO clips can be trimmed and rolled (name concepts popular to editors).
- Added new Editing tools (only trim and move icon).
- Fixed undo on moving clips.
- Upgraded OTIO to v0.18.0 from v0.17.0.
- Improved performance of 4K OpenEXR sequences.  With the new htjk256 and similar OpenEXR compressions, is possible to play back 4K sequences at 60 FPS on the Vulkan backend.  OpenGL backend plays at 30FPS.
- Fixed toggling Edit mode on and off.
- Made OTIO clips smaller on non-high DPI OSes.
- Improved window resizing on Vulkan backend.


v1.4.5
======

ChangeLog
---------

- Made Cache Read Ahead default to 5.0 seconds.
- Fixed some Vulkan validation errors that got through in the original v1.4.4 release.
- Fixed a FLTK Wayland crash when tooltips were shown and the mouse pointer was moved inside them.
- Upgraded OpenEXR to v3.4.3.
- Upgraded OpenJPH to v0.25.0.
- Tiny speed up in OpenUSD playback.
- Removed an old "lighting=1/0" message when opening the USD Panel in mrv2.
- Added enableSceneLighting and enableSceneMaterials as properties that the USD Panel will save on exit.
- Removed error on opening vmrv2 with the USD panel open.
- Made HUD less prominent as it was somewhat unreadable and making Vulkan skip frames too.
- Fixed OpenEXR's tile reading code.
- Fixed Vulkan's Data Window and Display Window.
- Fixed OpenGL's line size consistency on OpenEXR's Data/Display Windows.
- Fixed Wayland's Desktop Scaling under the Vulkan backend for vmrv2.
- Fixed Vulkan's playback slow down when mouse entered the title bar.
- Fixed file associations for mrv2 and vmrv2 on Linux.
- Fixed removal of icons on Linux on DEB and RPM uninstall.
- Fixed caching when loading a file and autoPlayback was on.
- Added Progress Report when caching files upon loading.
- Fixed exec_command on Windows platforms to properly use UTF-8.
- Fixed exec_command with pipes using threads to capture output.
- Fixed licensing issue on newer Windows using legacy wmic which is no longer present on newer Windows distros.
- Made all paths on Windows use forward slashes instead of backwards slashes or a mix of both.
- Made session file paths use forward slashes on Windows.
- Fixed macOS file associations.


v1.4.4
======

ChangeLog
---------

- Fixed some validation warnings on Vulkan backend that got past the v1.4.4 release.
- Made default Read Ahead be 4 seconds instead of 2.
- Frame/Seconds/Timecode widgets now handle focus by clicking on them instead of just entering them.
- Made Vulkan save out images and sequences with full color corrections (HDR + OCIO) if those are active.
- Added Link Annotations.  Title is shown as a tooltip when moving the mouse pointer near the Link drawing.
- Added Link Annotations to Session saving.
- Added path mapping to Link Annotations.
- Fixed a Linux Wayland warning about Always On Top when dragging a File item under Gnome.
- Made popup menus on viewport use a bigger font for better readability.
- Made docking faster when subwindow is dragged to it (open/close dock).
- Fixed Automatic color buffer type selected when YUV_420P_U12/16 and similar depth movies are loaded in both OpenGL and Vulkan.
- Fixed saving in Vulkan all RGBA image types supported by vmrv2.
- Improved precision of PNGs by saving RGB_U16 or RGBA_U16 when possible.
- Cleaned up saving movie/image code in the Vulkan backend.
- Improved performance of image saving code in the Vulkan backend.
- Improved performance of panel subwindows dragging on Wayland.
- Fixed saving elapsed time being not being in milliseconds in the progress report bar.
- Fixed showing Original (O) pixel values on the Vulkan backend as image was flipped in Y.
- Made thumbnail generation on both Vulkan and OpenGL backends faster.
- When saving out a temporary session file (temp.mrv2s or lang.mrv2s), the current session file is no longer updated to it.
- Fixed -p Stop command-line flag stopping after playback was started when AutoPlayback was on in the Preferences.
- Fixed thumbnail generation on Vulkan making the playback stutter.
- Improved the performance of video decoding.
- Improved the performance of sequence decoding (all formats).
- Fixed Linux directory selection in file requester, not allowing selection when travel single drawers was on.
- Fixed a movie decoding error sometimes skipping frames.
- Removed LOG_ERROR reporting when pixel values at mouse position could not be read (due to a window resize usually).


v1.4.3
======

ChangeLog
---------

- Upgraded to OpenColorIO v2.5.0.
- Upgraded to yaml-cpp v8.0.0.
- Upgraded to expat R_2_7_2.
- Upgraded to minizip-ng v4.0.10.
- Made Voice Over Annotations stop playing or recording when clicking on any other action tool or clicking on voice over annotation tool again.
- Fixed an incorrect dependency on OpenGL on tlDevice when compiling the Vulkan backend.
- Added guards on MRV2_NETWORK and TLRENDER_NET for network code.
- Added support for libaom for encoding AV1 instead of SvtAV1, as it is better supported and portable to aarch64.
- Removed a line printout debugging from video saving code.
- Fixed Edit timeline refreshing improperly under Wayland (needs nvidia-drivers-580).
- Fixed hiding of pixel toolbar on Wayland which had gotten broken.
- Added getting name of all temporary audio files from voice over annotations.
- Fixed a crash on A/B Comparison Overlays on the Vulkan backend.
- Fixed a redraw (trails) on A/B Comparison Overlays on the Vulkan backend when alpha was not solid.
- Fixed background color/checkers not showing on A/B comparisons on Vulkan backend.
- Fixed loading of hotkeys.  The user's home directory mrv2.keys.prefs takes precedence over the STUDIOPATH one.
- Made Audio mute's icon turn on (off actually) automatically if volume is too low.
- Added Mute Hotkey to turn on/off audio ('m' key by default).
- Added a hotkey to toggle the in/out points together (Shift + 'i' by default).
- Fixed scrubbing "jumping" when the previous action had been pressing on the timeline bar (like the Stop button).
- Made color of S (Start) and E (End) range buttons more prominent.
- Made the color of play buttons match the color of S and E.
- Fixed drawing annotations not following the cursor accurately.
- Fixed Text annotations on Vulkan backend which had gotten broken in v1.4.2 due to refactoring of code.
- Improved performance of draw cursor on Windows by using PRESENTATION_MAILBOX instead of PRESENTATION_FIFO.
- Fixed decoding of some movies where a frame would be missing with FFmpeg 8.0.
- Made Save and Reload Session close the original mrv2 and open a new one with the session.
- Added presentation mode used in Vulkan's viewport when displaying FPS.
- Fixed resource leak on Vulkan's vmrv2 exit.
- Improved performance of Vulkan under Wayland to support 4K OpenEXRs at 60FPS properly.
- Limited usage time of demo to about 9 minutes before exiting the program.
- Fixed dock panel location when the window was maximized.
- Made HUD draw an outline on the text to make it more readable.
- Fixed Linux drawing of timeline and viewport on OpenGL Wayland with buggy NVidia drivers.
- Vulkan's vmrv2 OpenEXR saving now saves out HDR Primaries as OpenEXR chromaticities.  Also, the inverse PQ transform is made to keep the OpenEXR as close to match the HDR video (with the use of OpenColorIO).
- Added OpenColorIO's ACES 2.0 .ocio files, besides using ocio:// so that artists can inspect and modify the original .ocio configs.
- Added "Video Primaries Name" to OpenEXR to more easily clasify the chromaticities attribute.
- Fixed starting up with the Files Panel as a Window under Wayland.  Previously, it would cut out the window's height.


v1.4.2
======


- Added Linux aarch64 builds, without NDI.
- Added Windows aarch64 builds, without NDI or SVT-AV1 encoder.
- Upgraded OpenJPH to v0.24.1 from OpenEXR's internal version.
- Fixed incorrect use of FLTK_USE_WAYLAND when not in __linux__.
- Fixed incorrect use of FLTK_USE_X11 when not in __linux__.
- Made volume slider automatically unmute the audio if it was muted.
- Added "File/Open/New Program Instance" to pop up a new mrv2.
- Made Erase tool support Alt + Drag to clear a rectangular area quickly.
- Fixed subwindow panels flickering/wobbling under Wayland and X11.
  Tested under:
  	* Wayland Ubuntu GNOME Shell 48.0 (older GNOMEs may still wobble)
- Fixed lookup of machine's UUID for licensing on Windows aarch64 builds.
- Created a web server for node-lock licenses so that hacking the node-lock
  license cannot happen.
- Added support for network licenses.


v1.4.1
======

mrv2 and vmrv2 are an open source professional players and review tools for vfx, animation and computer graphics for Windows, Linux and macOS.  You can choose to compile from source or get binaries.

This is the first donationware version of mrv2 and vmrv2.

It ships now in two compiled versions:
   - mrv2 with OpenGL backend  (free up to version 1.4.0 -
     	       	      	        donationware afterwards)
   - vmrv2 with Vulkan backend (donationware)

Difference between OpenGL and Vulkan
------------------------------------

Vulkan is a new open source API, compared to OpenGL that it might get deprecated on some platforms like macOS. It supports true HDR (High Dynamic Range), it is about 20% to 50% faster than the OpenGL version (on Windows, macOS M1+ and it is the same speed for Linux's GNOME 48 and later) but it does not support OpenUSD.

OpenGL's main benefit at this point is that it supports OpenUSD and works better on old CPUs (macOS Intel and older Wayland compositors).

Prices for binaries
-------------------

Donationware prices of binary licenses through PayPal:

https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=UJMHRRKYCPXYW

I use the email information to contact you privately.  I don't sell your information, as I don't have access to it, except for your email, which I use to contact you.

- u$  50 for a node-lock license for one year.
- u$ 150 for a node-lock license to own.
- The prices are cumulative.  If you donate, say u$10 in 5 months, you can access the node-lock license for one year.

License works for both mrv2 and vmrv2 (you can have both installed).

ChangeLog
---------

- Improved FPS reporting to not flicker so much or, on Wayland + Vulkan, be double its value on Presentation mode.
- Fixed starting of mrv2/vmrv2 on Windows when the installation directory has international characters like üí etc.
- Reverted to use FIFO instead of MAILBOX presentation mode in Vulkan.
- Some Unicode (UTF-8) fixes.
- Fixed shortcuts on Windows installer without having to modify the PATH variable.
- Fixed OpenGL backend with HDR to tonemap properly to 100nits.
- Fixed Vulkan SDR backend with HDR to tonemap properly to 100nits.
- Added missing Hable tonemap to Preferences->Render->Tonemap->Algorithm.
- Added VSync support to Vulkan backend (so that FIFO/MAILBOX can be chosen).
- Added Stereo 3D Scanlines, Columns and Checkerboard for Vulkan.
- Added Render->HDR->Gamut Mapping controls.
- Split Preferences->Render into Render and HDR.
- Added Preferences->HDR->Gamut Mapping controls.
- Fixed Stereo 3D Checkerboard on OpenGL due to deprecation of GL_POINTS.
- Fixed hiding of pixel bar on Vulkan backend after opening/closing Preference Window.
- Fixed some movies's play rate not being valid for timecode due to minor rounding errors.
- Made Stereo 3D Anaglyph work on MacOS.
- Updated Frame/Seconds Entry Widgets to allow calculations, so that you can
  type, for example:
  	  10 + 5 and get 15.
- Updated compile to work with cmake 4.1.1.
- Updated to OpenEXR v3.4 (from 3.3.5).
- Added reading and writing of colorInteropId attribute, but no interpretation
  of the parameter yet.
- Updated to use Imath v3.2.1 (from 3.1.9)	
- Updated to use cpptrace v1.0.4
- Updated to use FFmpeg v8.0 (from v7.0.1)
- Updated beta builds to build OpenUSD on Windows and macOS 13, as disk space is no longer an issue (Thank you Microsoft!).
- Refactored mrvGL/mrvTimelineViewport.cpp and mrvVk/mrvTimelineViewport.cpp to common classes in mrvCommonBackend/mrvTimelineViewport.cpp
- Refactored also mrvTimelineViewportEvents.cpp.
- Flipped Y coordinate of pixel toolbar on the Vulkan backend to make it consistent with the OpenGL backend and OpenEXR which also has 0, 0 at the bottom left corner.
- Fixed PDF thumbnail creation under Vulkan backend.
- Improved focus on Timecode widgets.  Now entering them will draw the cursor and leaving a timecode widget, will not throw focus.  This makes the whole
  UI much more friendly.
- Added .po translations to hdr utility.
- Fixed switching languages on Linux and macOS.
- Made demo mode pop up a license request every 5 minutes of use.
- Made demo mode not allow clicking twice on draw actions.
- Fixed dpkg and rpm uninstallers failing due to a syntax error on bash script.
- Removed deprecated is_valid_timecode_rate from OTIO pybind11's rationalTime.
- Fixed audio saving to also store the duration of each frame.
- Fixed HDR data also potentially being added to audio streams when saving a movie.
- Improved locate_python in etc/functions.sh.
- Added expiration date reporting on licenses.
- Put a work around on Wayland Panel Windows to avoid having them go off-screen.
  Note that they may still flicker and due to limitations of Wayland at 4k with
  60 FPS.
- Fixed current frame in timeline not updating on 4K videos sometimes on the
  OpenGL backend.
- Added Voice Over Annotations (new icon at the bottom of the toolbar).  How to use it:
  	* Select the microphone icon.
	* Click somewhere on the canvas. 
	  You are now recording your mouse positions, LMB presses, and your
	  voice on the default microphone (you can change it in the
	  Preferences->Voice Over section)
	* Click on the button again to stop recording.  It will change to a
	  stop button in yellow.
	* Click again on the button to replay the mouse moves and the audio.
- Voice Annotations can now be saved in the session.  Note that voice over
  audios are saved to the specified temp directory and should not be removed
  for playback.
- Voice Annotations are now saved and played in stereo.
- Added blinking recording button for Voice Annotation.
- Fixed incorrect resizing of window when being in fullscreen and loading a
  smaller clip than the window one.
- Added a -np (-noPython) flag for faster start ups.
- Fixed saving of single OpenEXR images failing on some cases on Vulkan backend.
- Fixed scaling of single OpenEXR images.
- Made UI more responsive when loading images (specially from the command-line).
- Added Vulkan install instructions to vmrv2 documentation for Ubuntu and
  Red Hat / Fedora 42 / Rocky Linux.

v1.4.0
======

- Added license_helper for easy licensing of vmrv2.
- Added backend used to -version flag.
- Added tabulation to -version flag's output.
- Made Vulkan's mrv2 be installed as vmrv2 to distinguish it from the OpenGL version and allow having both installed easily.
- Fixed a useless icons directory on macOS and a missing one on Windows.
- Turned off screen saver/suspending as it was not friendly to some users.
- Added Alt + LMB when in scrubbing mode to scrub slower.
- Added a Preference to set a multiplier on how slow with Alt + LMB.
- Added a Preference to control how scrubbing looping behavior works.  It can be set to Button (the UI timeline setting), Inactive or Active.
- Fixed Vulkan's DPI and scaling of images/videos within the viewport.
- Made audio stop playback when the window is minimized, except on older
  Wayland versions.
- Fixed YUV Original look up crashing the viewer.
- Fixed YUV Original pixel values as they were not matching the Full ones.
- Added Preferences->Behavior->Allow Screen Saver to control whether mrv2
  blocks the screen saver when running.
- Fixed blocking Suspend mode on Windows.
- Improved performance of Vulkan shaders' compilation for faster start ups.
- Added to title bar an (E) when non-saved Edit changes are present and
  (A) when annotations have been done.
- Fixed Vulkan macOS builds.
- Fixed Vulkan HDR support when Windows' HDR monitor was set to SDR.
- Improved scrubbing behavior.
- Fixed Wayland port on Linux distributing some incorrect Wayland libraries.
- Fixed Vulkan macOS installs, without having to set environment variables.  Now you can actually click on the icon (after you authorize the permissions to the application).  It DOES work now!
- Fixed macOS distro not installing a local python into the .app directory.
- Fixed Linux distro shipping two wayland DSOs by mistake.
- Made macOS Vulkan work with Retina.
- Fixed Vulkan crashing bug when going to presentation mode with NVidia 575
  drivers.
- Fixed hiding of Timeline in normal mode leaving a gap of 30 pixels.
- Fixed a lag when scrubbing with one or more of the thumbnail panels active.
- Fixed fullscreen mode in Vulkan backend.
- Fixed loading of some animated .gif files.
- Added a Preferences->Thumbnails->Refresh Thumbnails in Panels Manually to
  avoid auto refreshing of thumbnails.  You have to go to any thumbnail and
  RMB->Update->Thumbnails.
- Added thorough list of Wayland compositors with their version commands.
- Added Desktop Envionment report when using X11 or Wayland.
- Fixed finding all preferences with MRV2_STUDIOPATH and STUDIOPATH.
- Fixed framing when loading a session file.
- Improved startup times from the command-line and when loading the first clip.
- Made starting playback from command-line once the audio has been cached.
- Added reporting of Studio location if MRV2_STUDIOPATH or STUDIOPATH are set.
- Fixed saving of movies in Vulkan.
- Fixed saving of movies at half and quarter resolution in Vulkan.
- Fixed saving of single frames in Vulkan.
- Fixed saving of single frames at half and quarter resolution in Vulkan.
- Added Stereo3D Anaglyph support to Vulkan on Windows and Linux.
- Fixed hanging on Wayland with Vulkan's windows when they were going off-screen.
- Fixed saving of Prores and DNXHD with the correct profiles.
- Fixed saving of Half images on Vulkan backend.
- Fixed saving of images without annotations on Vulkan.
- Fixed Presentation mode autofit on OpenGL backend.
- Fixed toggling of Presentation and Fullscreen modes quickly in Vulkan backend.
- Made Vulkan text input widget for annotations support composed keys as those
  in international keyboards and CJK.
- Made starting playback from the command-line faster.
- Fixed crashes when switching from/to presentation mode when starting out as presentation mode.
- Fixed a potential crash when exiting the program in Presentation mode.
- Fixed OCIO and libplacebo Vulkan image validation errors.
- When a codec is not found, now mrv2 will report the actual name and long name for it.
- Added Python command cmd.getBackend() to return the backend of mrv2.
- Updated update-cmd.py plug-in to start faster, keep UI responsive and
  support both the OpenGL (mrv2) and Vulkan (vmrv2) backends.
- No longer we distribute libpng.so on Linux to avoid conflicts with GTK3 plugin.
- Fixed saving of movie files with and without annotations from OpenEXRs on Vulkan backend.
- Added saving HDR data into movie codecs that support it (VP9, AV1, etc).  Note that for saving the HDR data, Color TRC has to be set to a value different than BT709 in the Advanced Settings.
- Fixed saving movies from OpenEXRs with an alpha channel on Vulkan backend.
- Fixed Desktop Environment report when using -v (-version) flag.
- Added list of GPUs to Status bar on Vulkan backend.
- Made Playback->Auto Hide Pixel Bar a choice menu between "Inactive", "OpenGL only" and "OpenGL And Vulkan" instead of just a toggle.  The default is OpenGL only as Vulkan is so fast it does not slow down when reading back a single pixel information.
- Added "cursor" as one of the valid external editors for the Python console.
- Added a description of all valid environment variables for mrv2.
- Upgraded to FFmpeg 8.0, which speeds up decoding by 5-10% on 8K movies.
- Improved error checking on movie decoding, catching errors on some corrupt movies.


v1.3.9
======

- v1.3.8 had broken the short name of the buttons (F/O in the pixel bar) and
  T/S/F in the timecode selection button.
- Fixed Wayland's monitor names.
- Fixed opening of Secondary Window on Vulkan mrv2.
- Made all icons inlined in the C++ code.  This improves loading performance.
- Made movies without a valid audio codec still play the video.
- Fixed HDR movies that specify a transfer/color primaries function but no valid
  HDR metadata.
- Achieved consistently 60 FPS with 4K HDR movies on Vulkan build.
- Achieved consistently 40 FPS with 8K YUV420P_U8 movies.
- Fixed printing version information of GNOME in Ubuntu 25.04 LTS.
- Fixed installing of icons on Linux's Vulkan .deb/.rpm installers.
- Made Float on Top's default hotkey be <Meta>+w instead of <Ctrl>+w to avoid issues with standard Emacs hotkey.
- Added support for DTS (DCA) audio decoder.
- Dramatically improved the performance of movies decoding.
- Made the Vulkan backend not hide the pixel toolbar as, unlike OpenGL, does not make playback slower.  Hiding it also made the playback drop some frames.
- Fixed markers in timeline.
- Added Undo for Annotations->Clear All and Annotations->Clear current frame.
- Fixed BGRA animated gifs crashing.
- Fixed color corrections resetting when there was a change like flipping the
  image.
- Fixed incorrect use of hdrData even when the clip did not have any HDR info.
  This would make clips brighter when the playback was stopped.
- Added support for RGB_U10 in Vulkan builds.
- Fixed hiding of pixel bar when starting the playback from the command-line.
- Fixed compareDemo.py to work reliably.
- Fixed Python's cmd.setDisplayOptions() not updating the UI's top bar.
- Added default hotkeys for (you need to Reset the hotkeys for them):
  	* Playback/Annotation/Toggle Visible
	* Playback/Annotation/Clear
	* Playback/Annotation/Clear All
- Added an Alert message when using "Playback/Annotation/Clear All".
- Improved even more memory and decoding performance.  Now we can decode 8K YUV_420P videos with no popups at 60FPS, albeit audio may stutter or not play, specially on Vulkan when using HDR data.

  

v1.3.8
======

- Fixed a problem in first release of v1.3.8 regarding Input Color Space failing when first switched on.
- Fixed a bug with menus crashing on Wayland when Fullscreen was activated with F11 and a menu from the menubar was selected.
- Fixed OpenEXR's Data Window and Display Window display which was crashing when the coordinates had negative values.
- Added proper support for all possible texture types of OpenGL that libplacebo might use.
- Same for Vulkan's hdr utility.
- Flattened tlRender from a submodule to a directory to start working on porting it to Vulkan.
- Added mrvCore/mrvBackend.h to select which backend should mrv2 use.  By default, we still use opengl.
- Fixed hdr view utility to handle HDR10 and HLG properly on HDR10 monitors, as it would lead to a crash.
- Fixed hdr view utility sometimes crashing at start up on macOS.
- Fixed vkSetHdrMetadataEXT not being active.
- Fixed HDR monitor scoring selection to prefer HDR10 and HLG over P3_NONLINEAR.
- Improved runmeq.sh and runmet.sh scripts to exit early and with an error message if compilation fails.
- Fixed a potential reset of the X11 server when hiding the timeline bar.
- Fixed a crash with color area selection and switching to a clip of smaller size.
- Fixed a crash with color area selection switching from Full to Original Values.
- Allowed overriding pixel ratio on OpenEXR images from the Image Information Panel.  Note that once you override it, it will remain like that for all images.  To reverse them, you need to set it back to a value of 0 or less than 0.
- Fixed a memory leak in the Vectorscope.
- Fixed secondary viewport flickering when the timeline was hidden (at least on X11).
- Improved performance of text rendering on both OpenGL and Vulkan.
- Removed outline class from OpenGL as it was not needed.
- Added showing of clip name in the HUD for .otioz files.
- Improved performance of the thumbnails in the timeline for both OpenGL and Vulkan.
- Fixed a memory issue on libplacebo.
- Fixed a slowdown due to always trying to change cursor when in viewport with the media information window present.
- Fixed a Wayland bug not allowing to select the button menus (such as HSV) at the bottom of the screen.
- Fixed a memory leak on HDR processing on the OpenGL backend.
- Fixed crash on X11 when a monitor was connected but disabled.
- Made starting of playback not begin immediately but wait one second since
  showing the UI.  This allows the application to not skip frames at the 
  beginning.
- Started porting to Vulkan.

	 + Working:
		 * Viewport
		 * Pixel toolbar
		 * Timeline with tick bars, current frame number and labels.
		 * OTIO with dissolves
		 * OpenColorIO
		 * HDR Toggle (for HDR).
		 * HDR Tonemapping (for SDR).
		 * HUD
		 * Masking
		 * Safe Areas
		 * Data/Display Window
		 * Area selection
		 * Comparison Modes: 
			 - A
			 - B
			 - Difference
			 - Dissolve
			 - Wipe
			 - Horizontal
			 - Vertical
			 - Tile
		 * Annotation cursor (circle)
		 * Annotations
		 * Vectorscope
		 * Histogram
		 * Color Areas (Area color information)
		 * Environment mapping
		 * Thumbnails in Timeline (extremely slow).
		 * Thumbnails in Panels
		 * Missing Frames crosses
		 * Vulkan Text widget annotation
		 * Editing.
		 * NDI with annotations.
		 * NDI with undo/redo annotations properly.
		 * Re-editing text annotations once stamped.
		 * Added VMA support to FLTK.
		 * Added MoltenVK compilation on macOS to work around HDR bugs.
		 
	 + Missing to check/add:
		 * Saving of Movies/Pictures with Annotations 
		   (not easy and may not be possible)
		 * OpenUSD in Vulkan (Impossible to do it with Pixar's current API)
		 
	 + Problems:
	 	 * Performance of the timeline with clips is an even worse problem.
		   
	 + Improvements:
		 * Improved performance of drawing text in the HUD and timeline for
		   both Vulkan and OpenGL.
		 * Improved performance of drawing clips in the timeline for Vulkan.
		   Still slow, thou.
		 * Improved performance of Vulkan from 40FPS to 58/60FPS on 4K movies.
		   
- Fixed and simplified code for NDIView (hdr utility).
- Fixed hdr utility for macOS Intel trying to pass full HDR10 or HLG data, 
  which goes over those machines' nits.

v1.3.7
======

- Fixed Ghost Previous and Ghost Next not being in sync on the Secondary viewport.
- Made stepping with Next Annotation and Previous Annotation loop once they reach the final and first annotation respectively.
- Fixed opening Secondary Window not displaying the video when the video was stopped.
- Fixed Render->Minify Filter and Render->Magnify Filter toggling from the menu entries (Shift + F was working fine).
- Fixed Render->Minify and Magnify filter toggling.
- Fixed Render->Minify and Magnify saving in preferences.
- Added full support for Japanese language in the UI.
- Fixed some warnings when starting mrv2 with PYTHONHOME set on Linux.
- Removed the "export FLTK_BACKEND=x11" line as Wayland now is mature enough.
- Fixed tonemapping not turning off when switching from an HDR movie to aces SDR one.
- Fixed playing movies that have frames of 0 duration.
- Added Preferences->Render->HDR to control chromaticities and tonemapping.
- Fixed tags not getting saved when using save image or save movie.
- Made OpenEXRs keep the Data Window and Display Window when saving.
- Fixed OpenEXR saving when channel count was different than 4.
- Added the option in the Save Image Options for OpenEXR to allow saving the Data Window or flatten the image to save the full Display Window.
- Updated build to work with CMake 4.0.
- Added Playback->Annotation->Toggle Visible to toggle on or off the annotation without deleting them.
- Fixed a crash when saving a movie file and the container did not support it and the file was already present.
- Removed non-existant YUV440" pixel format from VP9.
- Revamped macOS install to support installing multiple applications.
- Updated ZLIB to v1.3.1.
- Made PNG link statically to avoid macOS issues.
- Updated Poco to 1.14.1.
- Added hotkey to toggle OCIO on and off.  By default, it is Alt + p (mnemonic pass through).
- Added hotkey entry for Compare None.  It was already present, but missing from Hotkey listing.  It is not set to any hotkey by default.
- Added a (currently in beta) hdr tool, to show the content of mrv2's viewport in full HDR (High Dynammic Range) by using the NDI Output's Best Format.  The way it works is that you stream your video through mrv2's NDI output and watch it in the hdr tool which works with Vulkan.  It is primarily made to wotk locally, albeit it can also work across the network.  NOTE: it has been tested only on masOS Intel and it is currently not available on macOS arm64.  LIMITATIONS:  it works well for FullHD content albeit 4K content tends to lag and it is mostly useful for single frames.
- Fixed the Windows uninstaller not deleting the hard-links it created.
- Added support for writing out DNxHD and DNxHR (needs documenting, but they are similar to ProRes). 

  
v1.3.6
======

- Fixed a serious OpenEXR crash on multipart files.
- Dramatically improved NDI input stream playback performance.  If playing with no audio, it is immediate.  If playing with audio, it will play with a 4 seconds delay.
- Fixed a hanging when playing NDI with audio.
- Fixed a zombie process when playing NDI upon program exit.
- Fixed locale (Internationalization) issues on Windows, which would make it impossible to switch to a different language if your Windows was not set up as English.
- Revamped locale on start up the first time you install mrv2 (or remove the preferences) on Windows and macOS to start with the System's locale.
- Moved building of dav1d, lcms2, vpx and SvtAV1 on Windows to the tlRender repository to avoid dealing with shell scripts.  This makes the build on Windows faster too and consistant on all platforms.
- Ported stack trace in debug and reldeb builds to use cpptrace on Windows and Linux.  The advantage is that it can give us stack traces of threads other than the main one.  Not as good as the traces of macOS, but it is a start.
- Fixed installation of NDI Advanced DLL being with the wrong name.
- Fixed builds when not building Python.
- Made building faster by using NPROCS.
- Made building faster by removing tlRender libraries we don't use, as Darby's code is diverging from the one used in mrv2.
- Fixed several locale issues.
- Fixed problem on Windows making the executable not run.
- Fixed command-line arguments not showing up when the application was used
  from Powershell or cmd.exe.
- Fixed python's cmd.getLanguage() call potentially crashing.  This would make
  the update-mrv2.py plugin fail.
- Fixed Window's _wexecv call with CreateProcessW to attach stderr/stdout
properly.
- Fixed Window's set_fonts() returning font names longer than 32 characters leading to problems of some users starting the program.


v1.3.5
======

- Updated to NDI's Advanced SDK.  The only limitation is that you can only stream content for a max. of 30 mins.  Note that NDI is currently not available on macOS arm64.
- Updated documentation to follow the NDI guidelines.

v1.3.4
======

- Updated default OpenColorIO configs to use their latest names instead of the versioned ones.
- Fixed disting libselinux on Linux.
- Added Ignore Chromaticities in Render/HDR menu.
- Fixed -v flag being used in -logLevel as well as version.  For backwards compatibility -v will refer to -version and -l to -logLevel.
- Added NDI Video Output to non-beta builds.
- Added NDI Audio Output to non-beta builds.
- Added NDI's HDR support to non-beta builds.  Albeit it is untested as I don't
  have an HDR monitor for it.  However HDR metadata is tonemapped correctly.
- Refactored Darby's broken BMD code to make it more generic to different devices.
- Refactored Background options to be controlled from MainControl.
- Major code clean up of audio code.
- Fixed antialiasing of text menus on Wayland.
- Fixed transparency on Wayland to be like X11.
- Fixed a Wayland crash when changing the scaling factor interactively from FLTK's <Ctrl>+ and <Ctrl>- shortcuts.
- Fixed a precision problem when drawing Text as an annotation.
- Fixed an issue with a bad optimization of annotations.
- Minor optimization of annotations drawing.
- Added annotations support to NDI video output.  The only thing that does not work yet is text annotations.
- Added NDI panels saving to sessions file.
- Made text annotations for NDI work on all platforms.
- Fixed annotations when set to all frames.
- Improved performance of annotations drawing.
- Made session files store the frame view, view position and view zoom settings.
- Fixed resizeWindow button not working any more as it should.
- Fixed a number of buggy OpenGL that were mostly responsible for mrv2's bad
  behavior under Wayland with NVidia graphics cards.
- NDI libraries can be loaded on demand instead of being shipped with mrv2.  However, Vizr is not shipping .h files compatible with NDI SDK 6.
- Made NDI sources be detected more cleanly by using Darby's observer class (observer pattern).
- Windows installer now opens the Firewall for mrv2 (and the version will show
  in the list of Firewall applications).
- Fixed a zombie process sometimes appearing when playing NDI.
- Improved log reporting by using TABs instead of spaces.
- Added support for HDR metadata for NDI Input reader.  Needs NDI's Advanced SDK.
- Sped up annotations drawing.
- Fixed a problem with annotations being kept with a slow macOS Intel.
- Added all tonemapping algorithms that libplacebo supports.  They are listed
  under Render/HDR/Tonemap.
- Fixed a potential installation issue on Windows.
- Updated to NDI 6.1.1.
- Fixed some potential libraries incompatibilities on macOS.


v1.3.3
======

- Fixed AC3 bitrate (saving audio using the AC3 codec).
- Improved saving audio bitrates for all formats.
- Fixed the sourceforge_defaults.sh script to work on all platforms.
- Fixed some potential rounding errors on saving frames or movies with
  annotations.
- Minor UI improvement.  Added small space between Gain/Saturation/Gamma
  sliders.
- Added upcoming (not yet released) HT256 OpenEXR compressor.
- Fixed update-mrv2.py script typo which would not allow updating mrv2.
- Updated OCIO default configs to v2.2 ones.  They are still based on ACES 1.3 though, as ACES 2 ones have yet to be released.
- Changed tlRender's use of exr::Compression to Imf::Compression to support current and future OpenEXR compressors.
- Changed SaveImageOptionsUI to use Imf::getCompressionNameFromId().
- Updated Python code to also use Imf::Compression instead of exr::Compression.
- Removed internal otioClipName and otioClipTime attributes from metadata.
- Added Annotation Frames Only to Save Image Options to save only the frames
  that have annotations when saving either annotations only or sequences with
  annotations.
- Added new Python command: cmd.saveMultipleAnnotationFrames() to save multiple
  annotation frames with either video or without video.
- Removed trailing newline from desktop information on status bar present on
Rocky Linux's **XDG_SESSION_TYPE** environment variable.
- Changed mrvTimelineViewport's resizeWindow() to use maximize() when the image will not fit on the screen.  This seems to fix some Wayland NVidia OpenGL issues.
- Made "Invalid EDID data" not appear when running under XWayland.
- Fixed Play buttons sometimes appearing in the middle of the viewport under
  Wayland.
- Made macOS OpenGL captures (Save Images/Movies with Annotations) use RGBA.
- Fixed macOS OpenGL captures with annotations reporting window is not in full screen when saving a single frame.
- Fixed resizing of main window under Wayland by using maximize instead of resize.
- Fixed resizing of main window when dealing with two monitors under X11.
- Added hotkeys to control the User Interface's transparency under Windows,
  macOS and Linux.  By default, they are assigned to **Ctrl + .** and
  **Ctrl + ,**.
- Added hotkey to control click (pass) through of window position and clicking.  Works on all platforms.  The hotkey by default is **Ctrl + t**.
- Added Wayland GNOME-Shell hotkey control for Float on Top.  It is Ctrl + w
  by default.  Note, however, that pass through will not automatically set
  Float on Top as in X11, macOS or Windows.  This is a limitation of Wayland.
- Fixed pip.sh script to call pip on the local mrv2 install.
- Added information about environment (desktop, os and kernel) of the machine that was used to build it.
- Made Build and Running log information tidier.
- Cleaned up log messages a bit and added a -logLevel flag.
- Added case matching to Hotkey searches.
- Hotkey names are now listed alphabetically in all languages.
- Hotkeys are now listed using GNOME's keyboard conventions in the Hotkey window, which are cleaner.
- Added Window/Toggle Click Through to menus.
- Added Window/More UI Transparency and Window/Less UI Transparency to menus.
- Added git branch and short hash of build to the About window.
- Improved build times on successive runme.sh runs.
- Improved translations.


v1.3.2
======

- Added support for Chromaticities attribute in OpenEXR files.
- Added support for Y, RY, BY OpenEXR images.
- Made YC OpenEXR conversion use file chromaticities.
- Hiding an audio track in an .otio timeline now turns off audio for that track.
- Added support for OpenEXR's ripmaps and mipmaps.
- Made Mipmap and Ripmaps' rounding mode show as "UP" or "DOWN".
- Made Line Order in Media Info Panel show as Increasing Y, Decreasing Y or Random instead of a number.
- Added Python functions: annotations.getTimes() to get the times where there are annotations.
- Added Python function: cmd.saveSingleFrame() to save the current frame.
- Added Python function: cmd.saveMultipleFrames() to save multiple frames from a list of times, like those returned from annotations.getTimes().
- Fixed instantiation of mrv2.io.SaveOptions().
- Made Page Up/Page Down change images always, instead of scrolling the panel
  sidebar.
- Fixed color refreshing in timeline panel when switching color themes.
- Fixed text annotations saving in PDF, movies or sequences on macOS Sonoma and Sequoia.
- Fixed zoom factor capture on high DPI displays on macOS Sonoma and Sequoia.
- Removed printing of profile when not saving a movie.
- Fixed opening a session file from the command-line when the Python panel was open.
- Improved Python Editor's highlighting of keywords.
- Fixed OpenGL capture of viewport on macOS, leading to offsets after some frames.


v1.3.1
======

- Updated docs.
- Fixed Image/Version/Next and Image/Version/Previous always going to the last
  clips.  The routines are also faster now.
- Fixed Image/Version menu not appearing when some directories had dashes and
  numbers in them.
- Made versioning regex get escaped, like Python's re.escape() function.
- Added support for version switching of clips in .otio timeline.   You need
  to be stopped at a certain clip with a proper version name to change it.
- Fixed showing of timelines that had the "enabled" set to false on them.
- Added 'm' hotkey to mark the in and out points of a clip in the .otio
  timeline.
- Added toggling visible state of tracks in Timeline Viewport.
- Upgraded to OpenEXR v3.3.2.
- Added OpenEXR's headers Compression, Compression Num. Scanlines,
  and Is Deep, Is Lossy.
- Added support for Flame's .otio files using OTIO's SerializableContainer in
  them.
- Updated libvpx compilation to MSVC2022.
- Updated to FLTK's release 1.4.1.
- Updated pyFTLK to official 1.4 release.

  ** COMPATIBILITY NOTE **

  Note that this pyFLTK update changes the namespace from fltk14 to fltk.
  If you are using:

``
  from fltk14 import *
``

  in your scripts, you will need to change it to:

``
  from fltk import *
``
  
- Improved build reporting swig version used.
- Made tlRender compile after FLTK so that preferences can be read from
  tlRender.
- Made Path Mappings to work on .otio files.  If building from source, you may
  need to do a:

       $ runme.sh clean
	   
- Added displaying of clip names in the otio files in the HUD.
- Improved performance of Data Window and Display Window when reading 
  multipart OpenEXRs.
- Fixed crashes of PlaylistButton when there were no tracks or stack.
- Updated AI translations.
- Fixed opening of sequences with Open Directory or dragging an actual
  directory to mrv2.
- Updated pyFLTK to use Git in sourceforge.
- Fixed Python compilation on new macOS that is no longer passing neither
  DYLD_LIBRARY_PATH nor DYLD_FALLBACK_LIBRARY_PATH to subshells.
- Fixed "Always Save on Exit" getting confused about the monitor where to open
  the window.
- Fixed libplacebo compilation linking in unneeded libshaderc.dylib.
- Added support for .m4a audio files used in Quicktime.
- Sped up building by removing yasm dependency.
- Added Preferences->Timeline->Video Start Frame to set the start frame of video
  files.  By default it is 0.  This setting does not effect sequences nor .otio
  timelines.
- Fixed thumbnails of movie files not showing the right frame when Video Start
  Frame was different than 0. 
- Added "File/Save Audio" to save only the audio track disregarding video.
- Constrained Save Audio Formats/Containers to those supported by LGPL mrv2.
- Fixed File/Save Audio for 48KHZ audios.
- Fixed pixel aspect ratio reading from movie files.
- Added changing pixel aspect ratio interactively from the Media Information
  Panel.
- Added comparison modes with their possible shortcuts to mrv2's View/Compare
  menu.
  

v1.3.0
======

One major new feature and one important bug fix.

- Added tone-mapping of HDR videos.  Note that FFmpeg seems to be buggy when
  reading the frame metadata, so we must rely on the stream metadata only.
  Note that to compile this on Windows, you must install MSVC's shipped clang
  compiler.
  
- Fixed tiling behavior (dragging of timeline bar not making view window
  smaller) which got broken in v1.2.9.
  

v1.2.9
======

- Handle **AV_DISPOSITION_ATTACHED_PIC** properly, instead of skipping it.
- Added PNG decoder so attached png pictures in .wav files are decoded properly.
- Added a File->Save Annotations Only.  This allows you to export the annotations as a movie or file sequence.
- Added a File->Save Annotations as JSON.  This allows you to export the annotations as a .json file.
- Fixed a crashing bug when drawing freehand.
- Fixed disappearing cursor when selecting a drawing tool.
- Added polygon drawing to the drawing tools.
- Polygon, Circle and Rectangle have two modes (outline or filled now). You select the mode by clicking on each triangle edge of the button.
- When saving a single frame, the file name is automatically filled on the file
  requester to save over the same file.
- Fixed Float on Top on start up on Linux X11.
- Added pip.sh for Unix systems to easily install pip libraries inside mrv2's 
  python library directory.
- Moved mrv2's ComfyUI directory to its own repository at:
	https://github.com/ggarra13/ComfyUI-mrv2
  so that it can be listed more easily in ComfyUI's Node Manager *AND* we
  make it more clear that that project is GPL compatible, not BSD one.
- Fixed selecting montior's Display/View to None from the Menus.
- Updated OpenColorIO to v2.3.4.
- Updated to OpenUSD v0.24.8.


v1.2.8
======

- Automated sourceforge OS automatic file selection for a release (default, the
  last git release).
- Gain, Saturation and Gamma now effect all videos in a comparison when in
  Compare Mode.
- A/B Wipe Comparison now properly follows the cursor when zoomed in.
- A/B Wipe Comparison now properly follows the cursor on Y (it was reversed
  previously).
- Fixed and simplified bakeOCIO.py demo script.  Now, you can bake OCIO
  from the command-line by just doing something like:
  ```
  mrv2 <INPUT_FILE> -ics ACEScg -od 'sRGB - Display' -pythonScript bakeOCIO.py -pythonArgs <BAKED.mov>
  ```
  Note that this script bakes without annotations support so if saving a float
  EXR to an 8 or 16-bit image, you will get banding.
- Fixed changing displayOptions through python not reflecting the changes in
  the UI.
- Fixed cmd.compare() to refresh the comparison immediately.
- Fixed Python Output showing icorrectly in panels when loading a session from
  the command-line.
- Fixed ICS pulldown not showing None when switching to a clip with no
  Input Color Space stored.
- Fixed viewport/image rescaling calculation in Save Move/Save Single Frame.
- Fixed Save Resolution setting being set to Half Size when Save Annotations
  was on.
- Made Status Bar error/warnings remain longer (8 seconds instead of 5).
- Fixed a precision issue on Windows with arbitrary scalings like 115%.
- Added Image/Previous Limited and Image/Next Limited to go from previous to
  next images, without looping.
- Added "Image/Go to/<Clip>" to switch from one clip to the next without having
  to bring up the Files Panel.
- Added "Image/Compare/<Clip>" and "Image/Compare Mode" to compare images 
  without bringing up the Compare Panel.
- Made UI at start up wider to account for new menus and new language 
  translations.
- Added HDR metadata to Media Information Panel.
- Very minor playback improvement for non HDR videos.
- Made "Advanced Settings" in Save Movie options store its settings.
- Made tlRender FFmpeg's write always print out the color space, color TRC
  and color primaries used when saving.
- Preferences->Render->Video Levels had Full Range and Legal Range reversed.
- Added first pass at Russian AI translation.
- Added Edit/Audio Clip/Insert.  It allows inserting an audio clip at the point
  in time for the video clip.
- Added Edit/Audio Clip/Remove.  It allows removing audio clip(s) at the point
  in time for the video clip.
- Fixed potential export issues when using Save Movie not exporting from the
  first frame.
- Clarified the name of TV (Legal Range) and PC (Full Range) in Advanced
  Options of Save Movie Options.
- tlRender now skips video streams with **AV_DISPOSITION_ATTACHED_PIC** or
  **AV_DISPOSITION_STILL_IMAGE**.
- Fixed Darby tlRender's BT.2020 coefficients. 


v1.2.7
======

- Better German, Portuguese, Chinese, French, Italian and Hindi translations.
- Added Selection of Page Size when saving out a PDF.
- Updated FAQ to cover X11 settings and FFmpeg Color Accuracy as a cause of
  slowness when playing YUV420P_U8 movies.
- Updated FAQ to cover X11 tearing due to bad graphics card configuration.
- Fixed session files with no panels open, leaving the dock group open if it
  was previously open.
- Fixed crash of HDR->Auto Normalize with OpenEXR files that had a smaller
  display window.
- Fixed FPS display not respecting the decimal separator on numeric locales
  that use commas.
- Fixed numeric locale for all languages.
- Made CTRL + r (Reset Colors) reset also saturation changes.
- Added Hotkeys display to all button and slider tooltips, which will change
  if you modify any shortcuts.
- Added hotkeys entries for saturation more and saturation less.  By default,
  they are assigned to '<' and '>'.
- Fixed tooltip in Gain slider not showing up.
- Fixed an OCIO bug when setting from the command-line the display/view with
  nuke-default's config.ocio.
- Fixed Menu Entry "Playback/Go to/Start" and "Playback/Go to/End" not showing
  their keyboard shortcuts.
- Fixed Darby's DPX reading code.
- Fixed dockgroup not getting highlited when dragging and the panel could dock.
- Allowed saving comparisons (Tiles, for example) to a movie file if Save
  Annotations is turned on.
- Improved dramatically the speed of packaging mrv2 on Linux and macOS.
- Made ICS, View and Look not translate "None" as that was causing trouble when
  switching languages in session files.
- Fixed "None" getting saved translated in several panels and OCIO settings,
  causing problems when loading the same session file on a different language.
- Dramatically improved the performance of playback of SolLevante Netflix demo
  clip by using YUV420P_U16 instead of RGB48.
- Improved hotkey selection/display when using shift on some international
  keyboards.
- Added explanation when FFmpeg Color Accuracy is on to explain slow conversion.
- Wayland crashing fix.
- Wayland now uses FLTK's own built-in libdecor to avoid warnings in GTK-3.
  If you compile from source, you can change this, albeit you will still get
  a minor warning due to a conflict between FLTK and GLFW calling GTK-3's
  functions twice.
  

v1.2.6
======

- Added Preferences->User Interface->View Window->OCIO In Top Bar to turn on
  OCIO at start of UI.
- Build fixes and directory clean-up.  If building from source, you should run
  a full rebuild with "runme.sh clean", as the whole directory structure of
  the mrv2 repository was changed.
- Added Portuguese, Italian and French translations done with AI.
- Better Chinese, German and Hindi translations.
- Added top bar OCIO / Color toggle (Hotkey 't' by default).
- Added lumma channel (Hotkey 'l' by default).
- Allowed saving .otio movie files as .mp4, .mov, etc. files.  This allows
  turning an .otio file into a new movie file.  Useful when concatenating
  movies with the Playlist panel or with the -edl command-line switch.
- Fixed cancelling of language switch.
- Added showing name of language that will be changed to.
- Allowed saving a picture with no audio and then a movie with audio as a
  standard movie file (not .otio).
- Fixed cursor when entering text.
- Improved port and monitor detection on Windows.
- Improved performance of redraws when using multiple monitors with OCIO.
- Fixed some update issues in Top Bar's OCIO View display when using multiple
  monitors and OCIO was changed from the menus.
- Fixed view getting reset if a monitor's view was selected previous to
  selecting an image ics.
- Added option to use default ocio/view saved in config.ocio.
- Added command-line option to just set the display only and it will use its
  default view for it.
- Added warning about variable frame rate movies, like Ubuntu's screen captures.
  They are played at 12 FPS.
- Added Darby's and my own Windows' sequence fix.
- Made python scripts run AFTER ocio settings, so that OCIO can be easily baked.
- Fixed and simplified bakeOCIO.py script.
- Made python script be searched in $STUDIOPATH/python/ directory and
  in mrv2's python/demos directory if it cannot be found directly.
- Sped up monitor look up on X11 which was slowing down channel switching or
  image mirroring.
- Made File->Save PDF save out international characters properly.  You can now
  save annotations written in Chinese, for example.
- Made File->Save PDF work properly on Wayland.
- Made File->Save PDF refresh properly, as it was buggy.
- Fixed drawn annotations disappearing when a text note in the same frame was 
  cleared.
- Removed LibHaru dependency, as PDF creation is now handled by FLTK.
- Added OCIO options and LUT options to timeline, using Darby's new code.
- Fixed page creation when creating PDFs.
- Fixed PDFs' time information going outside the page on macOS.
- Fixed PDF creation drawing annotations on the wrong frames on large movies.


v1.2.5
======

- RE-RELEASE: v1.2.5 in English had a serious bug in OCIO selection through the
              menus.
- SECOND RE-RELEASE: Added toggle between showing OCIO or COLOR information in
  	 	     topbar in OCIO menu.
- Input Color Space and Look setting are now stored with each clip.  However, 
  you cannot compare two clips (say in a wipe) with different OCIO settings.
- Made Image/Previous and Image/Next not appear when a single image or movie
  was loaded.
- Fixed typo when saving session files which would prevent them from being
  loaded which had gotten broken in v1.2.3.
- OCIO configuration can now be selected from the menus.
- OCIO Display/View can now be applied on a monitor per monitor basis.
- Removed OCIO from the Color Panel.
- Removed OCIO from the Top bar.  This change hopefully won't be controversial.
  Now OCIO is all managed through the OCIO menu which can be easily accessed
  with the Right Mouse Button or from the Main menu bar.
- Major refactoring of convoluted OCIO code.
- Allowed Chaging OCIO Input Color Space from the OCIO menu.
- Fixed OCIO ICS when set to None and changing languages. 
- Fixed annotation markers in timeline showing even when switching clips.
- Fixed Wayland PNG incompatibility on older Linux OSes.  Tested to work on
  Rocky Linux 8.10, Ubuntu 22.04.4 LTS and Ubuntu 24.04.4 LTS.
- Fixed saving annotations leading to black or stopping playback when there
  were no annotations on Windows.
- Added session pattern to Open Movie or Sequence.
- Moved View/OCIO Presets to OCIO/Presets.
- Added OCIO/Current File/Look to new OCIO menu.
- Added -otio or -edl command-line option to automatically create an .otio 
  timeline from a list of clips (movies or sequences) provided in the 
  command-line.  Note that FPS is taken to be that of the one with highest 
  FPS, so sequences may leave gaps if video clips bigger than their FPS are
  used.
- Made -otio or -edl command-line option work with relative paths.
- Fixed .otio, .otioz and -edl flags when concatenation videos with different
  rotations.
- Changed name of "Reproducción" to "Reproducir" in Spanish translation.
- Added Chinese (Simplified) Translation done with AI.
- Added German Translation done with AI.
- Added Hindi Translation done with AI.
- Made Languages display their English name in parenthesis to help if switching
  to an unknown language by mistake.


v1.2.4
======

- Fixed mrv::Tile behavior on Windows not dragging when the mouse was kept
  pressed.
- Made Edit button toggle active if you drag the timeline viewport separator.
- Made FFmpeg settings which were marked that the movie file had to be reloaded
  automatically reload upon a change.
- Made deprecated yuvj420p movie files decode fast by default.
- Changed mrv2 Windows icon to a bigger size.
  If you installed betas, you might need to refresh the cache.

  See:
  https://superuser.com/questions/499078/refresh-icon-cache-without-rebooting

- Fixed thumbnail creation on Files Panel upon redraws not respecting the
  OpenEXR layer.
- Made Desktop icon on Windows and Linux bigger.
- Fixed Loop behavior on new loaded movies to respect the Preferences setting.
- Updated to USD v24.08.
- Fixed gamma correction not ocurring in linear space, when reading
  log images.
- Fixed gain and color corrections not ocurring in linear space, when reading
  log images.
- Added documentation about mrv2's OCIO color pipeline in the Interface section
  of the on-line docs.
- Made Input Color Space and Display/View allowed to be applied individually
  by selecting None.
  This helps with checking the linear color space, as used in Nuke.
- Fixed some Display/Views in studio config that have parenthesis in them
  getting misinterpreted.
- Fixed cursor when in some drawing mode and leaving the view area to the color
  channel tool bar.
- Added Compare Time Mode to Compare Panel to select between Relative or
  Absolute timeline comparisons.
- Added media.setCompareTime() and media.getCompareTime() to get the
  comparison modes.
  

v1.2.3
======

- Fixed loading of movies with multiple audio tracks which had gotten
  broken.  Also it fixes an incorrect use of C++ that could lead to
  undefined behavior on compilers.
- Added "Render/HDR/Auto Normalize" to normalize HDR (OpenEXR and HDR images).
- Added "Render/HDR/Invalid Values" to show HDR images' invalid values.
- Added toggleSafeAreas, toggleDisplayWindow, toggleDataWindow,
  toggleIgnoreDisplayWindow, toggleAutoNormalize, toggleInvalidValues
  python commands in cmd module.
- Upgraded to NDI 6.0.
- Added support for NDI's PA216 format.
- Added NDI Panel's pulldown to support either fast or best format.
- Added NDI Panel's pulldown to support audio or not.
- Fixed a memory leak and slowdown in NDI streams.
- Fixed loading of the same frame several times from the command-line.
- Improved switching of clips.  There's no longer a flickering of black when 
  you switch clips.
- Slight improvements on playback performance.
- Fixed reverse playback seeking when cache was not filled on movies smaller
  than 4K.
- Fixed a memory leak on reverse playback seeking.
- Fixed area selection when switching to a clip of different size.
- Fixed thumbnail icons not showing the current layer in the Panels.
- Fixed reverse seeking not respecting the reverse playback action and 
  reverting to stopping.
- Re-arranging of View menu into submenus.
- Added small margin between thumbnail picture and labels on all Panel clips.
- Added credit for the icons used in mrv2 when possible.
- Changed mrv2 icon to a great new design of Thane5.
  On Windows, you might need to refresh the cache or manually set the file
  association again.

  See:
  https://superuser.com/questions/499078/refresh-icon-cache-without-rebooting
  
- Made icons on Linux follow the Freedesktop.org guidelines.
- Made UnReal movies with .avi and mjpeg codecs play properly in mrv2.
- Made mrv2 report "Unknown video codec" when a codec is unknown.
- Fixed pyFLTK compilation on newer FLTK branches as fltk-config would return
  includes quoted.
- Build fixes to remove relative paths on packaging.
- Code clean-up.


v1.2.2
======

- Updated FFmpeg build to support .webm muxer which was missing.
- Made OpenEXR tag for Compression show its actual name instead of a number.
- Added OpenEXR compression per layer instead of a global tag.
- Added Channel (Layer) stepping hotkeys ({ and } by default).
- Made exposure buttons (and the hotkeys) increment and decrement by 1/4 stop
  instead of 1/2 stop.
- Made single images not start playing even if auto playback is on.
- Improved hiding of the pixel bar to only do it when duration is not 1.
- Upgraded to FFmpeg 7.0.1.
- Fixed auto playback when started from the command-line with multiple videos.
  Now they all respect the auto playback setting instead of just the first
  video.
- Added Preferences->User Interface->Raise on Enter to set how the window
  should behave once the mouse enters the view window.  By default, now
  it is off, instead of on; meaning you will have to click on the window to
  bring it to the front and activate keyboard focus.
- Fixed overwriting of data in Darby's original FFmpegReadVideo code.
- Added support for HAP and HAP with Alpha decoders.
- Added support for HAP encoder.
- Fixed mrv2's custom file requester starting with no icons in the current
  directory.
- Fixed Open Separate Audio dialog to be non-modal.
- Fixed sequence loading on macOS sometimes returning a wrong sequence when
  sequence was like 0999-1001.
- Fixed loading of separate audio track for image sequences.
- Added a work-around to support loading audio tracks with embedded PNG files
  when FFmpeg was compiled as minimal.
- Fixed -playback command-line flag to start playback when set even if
  autoPlayback is off.
- Made reverse playback at start not leak an observable item pointer or be
  slower.
- FLTK fixes to OpenGL 3 for Debian 12+ and AMD/Mesa systems under X11.
- Added libsnappy version and copyright to About window.
- Made path mapping reject two identical remote paths.
- Updated SVT-AV1 library for MSVC 2019 and MSVC 2022 (v2.1.2).
- Moved Windows building of SVT-AV1 library to tlRender and removed the hack
  of creating a .pc manually.
- Fixed looping to stop at start/end when scrubbing and Loop is off.
- Fixed primary screen resizing on second monitor moving to first monitor.
  This might also fix crashes due to different monitor resolutions.
- Fixed secondary screen hiding timeline when going to presentation mode.
- Added Window->Preferences->User Interface->Maximized option.
- Fixed saving of OpenUSD settings.
- Added a ComfyUI directory with a mrv2 a custom node to allow saving
  EXR images in full half/float precision.  For those that believe in AI,
  see the instructions on how to install it in the ComfyUI directory.


v1.2.1
======

- Fixed codec support (libvpx, dav1d and Svt-Av1) which had gotten broken due
  to new way of building FFmpeg.
- Fixed scrubbing behavior to be smooth when there's no audio.  If there's
  audio the default behavior is to play the movie while scrubbing so audio
  can be heard, unless Preferences->Playback->Scrub With Audio is off.
- Fixed Settings->Gigabytes display resetting to the maximum memory even if the
  setting was actually using much less.
- Improved scrubbing behavior when audio is played and the mouse button is not
  released.
- Fixed Python ocioIcs(), ocioIcsList(), ocioView(), ocioViewList() to not
  return paths with '/' at the beginning.
- Made session (.mrv2s) files use OCIO names for default OCIO ics, view and
  look.
- Fixed HUD when saving images or movies not being correctly turned off.
- Added View->OCIO Presets to store and retrieve OCIO configurations.
- Fixed Background in OCIO Presets window on Windows 10.
- Fixed VP9 encoder to use .mp4 muxer instead of .webm as it was failing.
- Fixes STUDIOPATH variable so that it is used only with OCIO presets and
  Path Mapping.
- Fixed command-line seek value when starting a video.
- Fixed reverse playback when starting the video from the middle.
- Fixed rounding errors on reverse playback.
- Fixed .zip packaging on Windows being twice the size.
- Added Previous and Next image shortcuts (PgUp/PgDown) that were missing.
- Fixed crashing bug when saving Movie files.
  

v1.2.0
======

- Fixed Wayland support again on Linux which had broken in v1.1.9 due to
  Darby's update of glfw3.
- Fixed Wayland resizing due to a bad FLTK commit.
- Fixed libvpx compilation on Windows which had broken due to an upgrade in
  MSys2's yasm assembler.
- Made compiling FFmpeg on Windows be done in tlRender with MSys2, instead
  of the pre-flight script, so that we can use --enable-zlib and share all
  flags.
- Fixed Repeat Frame not respecting the pixel depth of the previous frame
  when Preferences->Loading->Missing Frames was set to Repeat or Scratched.
- Fixed Missing Frame when Autoplay was Off.
- Fixed Caching starting from behind when Autoplay was Off.
- Fixed Docking and Docking attempts to happen once the corner of the Window
  enters the docking area.
- Added support for VP8 decoding so that Ubuntu's screen capture videos can
  be played.
- Added support for macOS Sequoia (beta).


v1.1.9
======

- Made undocking of panels with the dragbar always position them close to the
  panel.
- Added a visual indicator when a panel would dock if released.
- Fixed a potential crashing bug with panels upon exit.
- Fixed Panel docking on Wayland which was broken.
- Made switching between single images (non-sequences) faster when showing the
  thumbnails for them.
- Added missing .CRW format to the list of supported RAW formats in mrv2's
  custom file requester.
- Fixed a random incorrect displaying of sequences as single frames on
  Flu_File_Chooser.
- Fixed Panel undocking with the Yellow undock button not respecting the saved
  Y coordinate of the panel and using X instead.
- Improved UI behavior when docking, undocking and showing/hiding the
  scrollbars.
- Fixed Playlist creation on Wayland and simplified code.
- Fixed Panel Windows under Wayland.  Now they respect their position, albeit
  they are parented to the main window.
- Fixed a random crash when opening the Flmm_ColorA_Chooser for the first time
  due to incomplete menu items (Windows compiler issue?).
- Fixed Background->Solid Color not getting read properly from the preferences.
- mrv2's custom file requester can now toggle between image previews with the
  preview button (Monalisa).
- Made building FFmpeg on Windows more solid by relying on any of its two
  repositories and if that fails, read it from a tar.gz file.
- Updated About->Update mrv2 to list the changes from the version in a text
  display.
- Improved -pythonScript functionality.  It is now possible to change OCIO
  settings, load clips and save out a new movie file with baked OCIO, all
  from a Python script.
- Fixed building mrv2 without TLRENDER_NET and with BUILD_PYTHON ON on macOS.
- Consolidated Python commands to all use "fileName" instead of "file" or
  "filename".
- Added bakeOCIO.py demo script with arguments.
- Added -pythonArgs command-line string to pass a string of separated arguments
  with spaces, like:

  ```
  mrv2 -pythonScript mrv2/python/demos/bakeOCIO.py -pythonArgs "/D/pictures/Mantaflow_v09/Fluid.0001.exr 'ACEScg' 'ACES 1.0 - SDR Video' test.mov"
  ```
  
- Fixed drag and drop of http:// and similar URLs to mrv2 for network
  playback.
- Added file::isNetwork().
- Added "on_open_file" method callback to Python plugin system that gets
  called when a file is opened.
- Fixed a bug when docking the Python Panel and exiting.  The Python Panel
  would get reset to just the menus' size.
- Fixed OCIO color LUTs not getting activated when selected.
- Added a check button to the Color Panel to turn on/off LUTs.
- Added OCIO color LUTs per clips.
- Fixed typo in LUTOptions "enable" instead of "enabled".
- Fixed drag and drop to work with URIs coming from Google's Chrome when they
  don't have https:// prefixed into them under X11. 
- It is now possible to update a single frame of a sequence instead of the
  full cache, by pressing SHIFT+u (Default hotkey).
- Fixed refreshing of thumbnails when cache refresh was requested.
- Fixed Text annotation rendering disappearing after it was entered.
- Consolidated Darby's ui::Style::ColorRole with FLTK's color schemes.  Now
  changing from one of the preset color schemes should result on a nicer look
  for the timeline viewport.
- Sped up Thumbnail creation in Panels.  Fixed Darby's .otio/.otioz thumbnail
  creation.
- Sped up Thumbnail creation above Timeline.
- Sped up Thumbnail creation on mrv2's custom file requester.
- Made mousewheel not have any effect on the view window, timeline viewport nor
  volume slider unless the mouse is directly above them.
- Fixed mousewheel not scrolling sometimes on panel dockbar.
- Fixed Media Info Panel's search box not resizing when docked and scrollbar
  appeared.
- Made Preferences' window non-modal as it was conflicting with OCIO Pick's
  File Requester.
- Fixed Wayland's Text tool showing the colors semi-transparent.
- Fixed compiler error under g++13.
- Fixed Linux and macOS binary distribution crashing on loading USD files.


v1.1.8
======

- Added the ability to load single images from File->Open->Single Image.  You
  can select multiple images but they won't be loaded as a sequence, only as
  stills.
- Added a command-line setting (--single or -s) to also load single images.
- Removed the 1 pixel padding that was added when first loading a clip full
  screen.
- Fixed cursor disappearing when going to the action tool dock while a drawing
  tool was active.
- Fixed libglib-2.0 being bundled in the distribution.
- Added step to check Wayland compatibility for the binary (.deb or .rpm)
  installers compiled under Rocky Linux 8.9 and fix configuration file if
  needed.
- Made CMake documentation target (-t doc) not fail if no Python is found.
- Fixed a potential crash when mrv2 was compiled without Python support and
  a python file was passed as a parameter to the viewer.
- Fixed a potential misdetection of Linux flavor when updating version with	
  Help->Update mrv2 on a system that had both rpm and dpkg installed.
- Added displaying of the icon on the Taskbar under Wayland.
- Made both Python and Log Window start in Windowed mode as defaults.
- Added missing RtAudio's information to About window which was missing.
- Added Preferences->Audio to allow selecting the Audio API to use as
  well as the Output Device to use.
- Fixed a minor redraw issue on the right side of the pixel bar.
- Fixed "Fixed Position" and "Fixed Size" not working when there was no image
  loaded (the usual on Window's and macOS' users).
- Fixed "Fixed Position" and "Fixed Size" rescaling the window bigger every
  time the preference was changed.
- Fixed Y pixel coordinate in Pixel Bar when showing environment maps.
- Fixed X and Y coordinates displaying large negative numbers when no image
  was loaded.
- Made Preferences->Playback->Auto Playback not activate playback when a clip
  is dragged for the first time to an EDL playlist.
- Color accuracy for YUV420P movies introduced a minor speed penalty which
  can make some movies' performance playback stall, so a new boolean check box
  setting was added to Panel->Settings to toggle that setting.  The default is
  to have it on.
- Fixed an additional bug in resizing window code when loading movies without
  fixed size.
- Improved resizing window code to try to use the less space possible based on
  panel bars open and size of image.
- Made binary distribution smaller on Linux and macOS by removing the useless
  python.a config library.
- Fixed Presentation mode on Wayland having colored (gray) borders instead of
  black ones.
- Worked around an FLTK bug in its CMakeLists.txt files creating problems when 
  the API changed and the install/include/FL directory was not cleared.


v1.1.7
======

- Made mrv2's custom file requester create thumbnails on demand when they are
  on view.  This helps for large files like many big USD files on disk or with
  RAW files which are also slow to load.
- Fixed mrv2's custom file requester thumbnails being too high.
- Made mrv2's custom file requester display the name of the files below the
  pictures, except on WideList view.
- Changed Music icon in custom file requester to an .svg icon.
- Added custom RAW and several image missing formats to mrv2's custom file
  requester.
- Started cleaning up FLU's code (mrv2 custom file requester).
- mrv2's v1.1.6 was not respecting the hidden pixel bar on the Preferences
  Window, always showing it stopping.
- Fixed session files starting playback automatically even when the original
  timeline was stopped.
- Fixed a color discrepancy on YUV420 movies being too saturated.
- Rewrote the algorithm for the scaling of the window when the first image is
  loaded (or run from the command-line).
- Fixed a terrible bug on Linux X11 which would sometimes would make the X11
  server reboot on some Linux distros like Rocky Linux 8.9.
- Added OpenRV's license to mrv2 as I took inspiration for their reverse
  playback.  I was doing the same on mrViewer, but I had forgotten about it.
- Update tlRender to use OTIO v0.16.0.
- Added what each contributor did so far for the mrv2 project.
  If you don't want to be listed as contributor, not list your email address,
  or something else, please let me know.
- Updated update_mrv2.py script.  Tested on macOS, Windows, Rocky Linux 8.9
  and Ubuntu 22.04.4 LTS.  From now on, you can rely on automatic upgrades
  if you go to Window->Preferences->Behavior->Upgrades and set them at Startup.
  When you start mrv2, it will check to see if there is an upgrade after 5 days
  of the actual release (so any portential early bugs are squashed).
- Improved translation CMake scripts.
- Added documentation on how to get Wayland working almost flawlessly on Ubuntu
  22.04.4 LTS.
- Added 120 FPS, which drops frames but it is still nice to quickly browse a
  movie.
- Darby's Rec709 coefficients and YUV shader were incorrect leading to subtle
  color shifts.  Now they are fixed.
- Fixed shifting clips with annotations when transitions were present.
  

v1.1.6
======

This is a release full of goodies and bug fixes.

- **ATTENTION**:
  This version introduces a change in the way Cache Settings are used for
  movie files.
  The Settings->Gigabytes and the pair of "Settings->Read Ahead" and
  "Settings->Read Behind" are now decoupled.
  In the case of sequences, the Read Ahea and Read Behind will be automatically
  calculated from the Gigabytes settings as before.
  However, for movie files the timeline will only display the settings for
  Read Ahead/Behind, not the actual Gigabytes cache (which can be bigger than
  the read ahead/behind setting).  The small Read Ahead and Read Behind is to
  allow playing 4K movies backwards.
  The Gigabytes setting of 0 is **NO LONGER USED** and will revert to
  4 Gb if set to 0.

- Fixed slow seeking on 4K movies.  Now we beat OpenRV on **all** movies,
  seeking and reverse playback too.
- Improved performance of dragging Tile like the main divider between the
  view and timeline viewport, particularly on macOS.
- Made switching languages keep the UI preferences.
- Fixed cross cursor when entering the view area and coming from the draw
  tools or the drag bar.
- Made drag bar change color besides changing cursor to indicate you can
  drag it.
- Fixed NDI playback with audio which had gotten broken on v1.1.1.
- Improved .githook to run dos2unix to avoid dummy commits due to Windows'
  CR returns.
- Fixed window resizing calculation that had two ugly bugs in it (thanks
  to ManoloFLTK for having dealt with my ugly code!!!).  Now resizing of
  the main window takes the dock properly into account.
- Fixed auto hiding of the pixel bar from the preferences switched is changed
  and okay'ed while the movie is still playing.
- Fixed seeking on the timeline on Linux sometimes getting the event
  incorrectly on X.
- Fixed starting a movie with loop on and playing stopped and then start
  playing it backwards.
- Refactored playback code and fixed playback buttons sometimes getting out of
  sync compared to the command-line flags passed.
- Created Preferences->User Interface/Thumbnails to select the places
  where thumbnails appear.  Currently can be above the Timeline and in the
  Panels.
- Made the thumbnail above the thumbnail a tad smaller and more polished.
- Fixed a potentially nullptr pointer de-referencing when OpenGL accuracy was
  set to Automatic.
- Worked around an UI redraw issue on Wayland.
- Fixed timeline interaction (dragging clips in .otio timeline) in 1.1.5
  re-release.
- Fixed macOS issues when showing and hiding the timeline bar.
- Made View Window / Timeline Viewport divider highlight in a grayish color
  when it can be dragged.
- Made Panel divider highlight in a grayish color when it can be dragged.
- Fixed cursor when dragging on the Panel divider bar.
- Fixed cursor sometimes switching to the Arrow instead of the Cross cursor.
  This was maily a Linux issue.
- Fixed background transparent color being lighter on Wayland.
- Added Cache Use and Cache Percentage to HUD's Cache display.
- Made Edit button turn on/off automatically according to the size of the
  timeline viewport.
- Added default sensible Window settings for UI panels when in window mode.
- Made Window settings for UI panels get saved even if they were in panel mode.
- Fixed .otio markers display in timeline viewport.
- Fixed size of Timeline Viewport when in Edit mode on macOS.  Now it resizes
  properly.


v1.1.5
======

This is a critical update for Windows and more importantly Linux users.
It is a bug fix release for the regressions in v1.1.4 which was rushed:

- Added a FAQ on the documentation about Linux XWayland and Wayland on
  why it could make playback slow if your OS was not configured properly.
- This is the first version that supports Wayland and XWayland.
  For instructions on how to set XWayland and Wayland on Ubuntu 22.04.4 LTS
  with NVidia drivers, please see Help->Documentation the FAQ section.
- Fixed annotations not showing when Blit Viewports was on.
- Fixed hiding of pixel bar sometimes failing even when:
      Playback->Auto Hide Pixel Bar
  and:
      Playback->Auto Playback was on.
- Fixed not hiding of the pixel bar when the image was open from the
  recent menus or from the command-line.
- Fixed color buffers not taking full advantage of OpenGL improvements on
  Windows mainly.
- Fixed incorrect positioning and resizing of the images on loading
  which would crash Wayland.
- mrv2 can now be built with the pre-installed python, without having to
  build python itself.  Of course, in that case, mrv2 cannot be re-distributed.
- Added a python translating system for plug-ins.
- Translated all update-mrv2.py strings into Spanish.
- If using blit for the viewports, automatically switches to shaders if
  Background is transparent and image has transparency, as glBlitFrameBuffers
  does not support alpha blending.
- Panel->Settings->Default Settings now warns the user about it and asks
  for confirmation.
- Moved Panel->Settings->Default Hotkeys to Window->Hotkeys.
- Added Window->Preferences->Default to reset the user preferences to their
  default.
- Cleaned up source code of passing ui pointer to several classes, using
  App::ui instead.
- Removed .po python plug-in files from package distribution.
- Improved missing_translate.py script and added searching for plug-in
  translations too.
- Removed unused FLU XPM icons from the code, as most have been replaced by
  .svg icons.
- Fixed Edit Viewport Clip Info and Device Info not respecting the device
  pixel ratio when the Edit button was pressed.
- Fixed macOS crashes with preview thumbnail.
- Respect last configuration of Timeline/ClipInfo and Timeline/TrackInfo.


v1.1.4
======

This is a major release with big speed improvements and fixes to the color pipeline.

- Simplified build scripts.
- Fixed macOS build issues (fixed in v1.1.3 re-release actually).
- Prevented a macOS python plug-in issue with Python 3.11.
- Fixed Wayland support on Ubuntu 22.04.3 LTS and later.
- Improved performance of OpenGL drawing between 20% and 150%. macOS has seen the most benefit going from playing videos at 4K at 15FPS to 60FPS.
- Added Preferences->OpenGL->Color Buffer Accuracy to select between automatic,
  float or 8-bit buffers for speeding up movies or getting more accurate color
  precision.
- Added option to select blitting of main viewports for potentially
  faster OpenGL drawing.
- Added Windows' stack traces with line numbers for easy debugging.
- Added Windows' stack traces to be saved into tmppath().  That is:
     %TMP%/mrv2.crash.log.
- Allowed pyFLTK be able to compile in Debug.
- Fixed floating values not respecting float values below 0 or higher than 1.
- Improved the performance of OpenGL by avoiding OpenGL context switching when
  not needed (you don't have a text input widget, nor a text annotations
  showing).
- Added Linux's stack traces with line numbers by relying on libbacktrace.
- Improved performance of OpenGL dramatically, particularly on macOS.
- Improved performance of drawing OpenGL annotations (drawings).
- Renamed skipped frames as drop frames.
- Fixed Preview thumbnails above the Timeline crashing the viewer on all
  platforms.
- Hide Preview thumbnails when going fullscreen or presentation.
- Fixed Playlist creation showing an empty view for the first clip loaded.
  (regression of v1.1.2).
- Several fixes to Presentation Mode.
- On a re-release of v1.1.4 improved OpenGL performance as soon as a clip is
  loaded and starts playing, when
  Window->Preferences->Playback->Auto Playback and
  Window->Preferences->Playback->Auto Hide Pixel Bar are active.

v1.1.3
======

- pyFLTK build improvements (removal of warnings).
- Made Windows .zip file smaller by removing unit tests.
- Added Linux Desktop name to About Window.
- Fixed update-mrv2.py plug-in for Windows.  The install location is now
  retrieved from the registry.  Only if not found it looks in hard-coded
  paths.
- Install update-mrv2.py always, instead of removing it when python demos
  was not installed.
- Added instructions on Windows on how to retrieve the Explorer thumbnails.
- Darby has been busy!  He added automatic scrolling of the timeline when
  playing and ability to toggle track and clip info.
- Added Darby's new options to Network connections.
- New action icons, which should display properly at high resolutions.
- Sped up comparison wipes with Alt and Shift dragging.
- Sped up color corrections from the Color Panel.
- Fixed libdecor incompatibility with older libdecor plug-ins.
- Fixed + and - signs rotating the image when using one of the draw tools and
  not just the pencil tool.  With all of them now the pencil size is changed.


v1.1.2
======

- Windows version of mrv2 is now compiled with MSVC 2022.
- Fixed opening directories from within the command-line.
- Fixed stopping of playback when playing and selecting an area with
  Shift + Left Mouse Button drag or panning with Ctrl + Drag.
- Made status bar report "Everything OK." after 10 seconds without further
  errors.
- Removed -debug flag from command-line as it only works on debug builds.
- Made Metadata tab not appear if all metadata is shown in video/audio 
- Fixed a compilation issue between FLTK's JPEG libraries and those of
  libjpeg-turbo on Linux.
- Fixed Right Mouse Button toggling off the audio when there had been a drag
  action with some other button before.
- Fixed a precision problem when shifting annotations at the beginning of a
  clip due to a following clip being dragged to the same place as the
  annotation.
- Improved performance of timeline drawing when annotations are present.
- Fixed refreshing of image sequences
  (ie. Files Panel->Select Image->RMB->File/Refresh Cache) when there were
  missing frames and later they were added.  It used to work in v1.0.0 but a
  change in tlRender broke it.
- Updated mrv2 to Python 3.11 to conform to the VFX 2024 Platform.
- Python 3.11 on Windows is now built with optimizations on.
- Updated mrv2's glfw3 to 3.4.
- Updated mrv2's FLTK build to use the system libdecor
  (needs v0.2.2 at least).


v1.1.1
======

- Added keyboard shortcuts for Render/Video Levels and Render/Alpha Blend.
- Added support for multiple audio tracks.  You can switch from one to the
  other by selecting the Audio button (the speaker icon) with the
  Right Mouse Button and selecting a track.
- Fixed audio metadata in audio section of Media Info Panel for each audio
  stream.
- Added missing video metadata in video section of Media Info Panel.
- Improved speed of refreshing of a clip (when switching audio tracks or using
  View->Ignore Display Window).
- Made movies longer than 30 minutes with multiple audio tracks, use a short
  read ahead / read behind or switching audio would be too slow.
- Fixed saving movies with annotations and with an alpha channel.
- Fixed saving movies with annotations at half or quarter resolution.
- Fixed an ugly bug on Linux when saving movies with annotations repeating
  every other frame (ie. saving on two's).
- Fixed line error reporting in Python's Output (pybind11 bug).
- Added Editor/Jump to Error to Python Panel.  You can select the line of the
  error in the Python output and the Editor will jump to the offending line.
  If you don't select anything in the Python output, it will jump to the last
  error counting from the bottom.
  It will skip errors from files that are <frozen ...>, like importlib.
  If the error is on a file and not from the Python Editor, mrv2 will try to
  open an external editor and jump to the line of it.
  The editor to use is taken from the EDITOR environment variable first.
  If not set, mrv2 will try to use one of the popular editors.  Its defaults
  supports the syntax for emacs, gvim, vim, VS Code, notepad++ and nano.
  The preferred editor is checked at start up based on what you have installed
  on your PATH and whether you are running interactively or also have a
  terminal.
  Visual editors are given preference over terminal editors.
  Of course, you can edit the editor with Python Panel->Editor/External Editor.
- Fixed Python error reporting in the Python Output of the Python Panel for
  mrv2 python plug-ins.
- Fixed message that said it was saving a movie without audio when saving
  a sequence of frames.
- Fixed reading of FPS from OpenEXR images as they are now rational instead of
  a double.
- Fixed thumbnail of image sequences in mrv2's custom file requester when the
  sequence did not start at 0.
- Fixed a minor bug using snprintf of int64_t with an int specifier.
- Improved the main web page for mrv2.
- Fixed translation of "Color" in Panels.  Of course, since English and
  Spanish use the same word, this still makes no difference :D!
- Fixed the name of the video stream in the Media Info Panel to be like the
  layer name listed in the Panels and in the Color Channel pull down.
- Added back truehd decoder and encoder which were removed from
  TLRENDER_FFMPEG_MINIMAL compilations.
- Sped up redrawing when the Media Info Panel was open, as we now refresh
  only the tabs that change.
- Fixed changing the first and last frames from the Media Info Panel.
- Modified int sliders to use HorSlider instead to make it consistant.
- Fixed (very slow) changing speed from the Media Info Panel.
- Fixed HorSlider resizing improperly when values were bigger than 10000 or
  smaller than -10000.
- Improved precision when accepting or editing text with a zoom factor or
  pixels_per_unit different than 1.
 


v1.1.0
======

- Build fixes.
- Made mrv2's custom file requester list presumed sequences of a single frame
  as a file instead of a custom sequence name display.
- Matched NDI colors.
- Made NDI streams not display thumbnails in any of the panels, as it would
  look like mrv2 had hanged.
- Made installed size and installer smaller, by removing some unneeded python
  libraries used for documenting mrv2.
- Made installed size and installer smaller, by removing Python's unittests.
- Made default Windows' installer not install Python's Tk libraries by default.
- Made default Windows' installer not install IDLE nor turtledemo.
- Overall, the Windows installed size is now almost 150Mb smaller, now at 325Mb.
- Made Windows installer create a registry entry backup for the file
  associations, instead of using an .ini file which was worse.  On uninstall,
  the original file association is restored or the registry entry is deleted.
- Added a friendlier name to Windows' Open With RMB menu.  Now the version is
  listed, instead of just "mrv2.exe".  Also the name in English is provided.
- Added a latest version to Windows' Open With RMB menu.  If used, it will
  associate the latest version installed with the file.  Upgrading won't
  require you to reset file associations like before.
- Fixed rotations not framing the image even when View->Auto Frame was on.
- Added symbolic links on Windows .exe installer, so that it works more like
  Unix.
  You can use mrv2-v1.1.0.exe to call a specific version of the viewer for
  example.
- Split Timeline into "Timeline" and "Edit" Preferences in Preferences Window.
- Added Default View to "Edit" to select what gets displayed when you hit the
  Edit button ("Video Only" or "Video and Audio").
- Fixed Edit Viewport size when transitions were present.
- Fixed a potential crash when switching to nuke's ocio config.
- Made Python's setOcioConfig accept ocio::// configurations.
- Fixed colors on Python commands being orange instead of dark yellow as before.
- Fixed a redraw issue on Python's Panel output, at least on Linux.
- Made View pull-down menu display the full name of the display / view.
- Made UI topbar resize a tad nicer.
- Fixed Python Output in Python Panel which was not refreshing at all due to
  my misuse of the FLTK API since v0.9.3.
- Fixed a stray menu bar on Python Panel on closing it.
- Improved performance of package creation using multithreading when possible
  with cpack's generators.
- Added support for NDI's BT601 and BT2020.
- Added support for NDI's alpha channel.
- Added support for saving PNG and TIFF with alpha channel when saving a half
  or float OpenEXR image.  TIFFs will get saved in float.  PNG in 16-bits.
- Eliminated duplicated error and warning messages from the logs.
- Improved start up time and sync issues with NDI streams.
- Simplified all parameters in NDI stream to just selecting the source.
- Allowed NDI streams to change size, pixel aspect or color space.
- You currently cannot mix two NDI streams, one with audio and one without.
  Only the first one information will be used.
- You also cannot stream two clips through NDI with different audio
  frequencies or channels.
- Streaming two clips through the same connection after stopping for a while
  can lead to audio stuttering.  Either stream them one after the other or
  open a new connection.


v1.0.9
======

- Added a preference setting on Preferences->Errors to control what to do
  on FFmpeg errors.
- Made the panel drag bar a tad more grey, for a nicer look and more clear
  distinction when several panels are stacked on the dockbar.
- Fixed a thumbnail refresh issue on mrv2's custom file requester which would
  show when we save over a previous file.  tlRender would cache the file and
  would display the previous thumbnail before saving.  Now it is fixed.
- Removed mrvSequence.cpp which was empty.
- Added Global USD Render Option parameters which were missing.
- Fixed overlapping of audio labels in Edit window.
- Fixed rotation inverting the coordinates of the annotations.
- Fixed the pixel coordinate information when flipped or rotated.
- Fixed the pixel color bar display and information when rotated.
- Fixed text tool incorrect positioning when image was rotated.
- Made arrow head size be proportional to the line size.
- Made drawing cursor be drawn with float coordinates.
- Made circle's center also be drawn with float coordinates.
- Added back .wmv/a decoders and asf demuxer in TLRENDER_FFMPEG_MINIMAL.
- Added Video Color Primaries, Color TRC and Color Space FFmpeg information
  to Media Info Panel's video.
- Fixed FFmpeg reader to take color coefficients.  Now the gray ramp of
  Sam Richards is the same.
- Fixed a bug in session loading when there was no Input Color Space specified
  in the image, which got introduced in v1.0.8.
- Made pen size be able to be 1 pixel making the cursor one line only.
- Made prores_ks (FFmpeg's native encoder) match Apple's ProRes one at
  4444p10le.  It is now possible to take ProRes4444 encoded files to video
  editors.
- Upgraded to FFmpeg v6.1.1.
- Improved rotation speed of movies from cell phones, by doing it in OpenGL
  with metadata instead of with a C++ function.
- Added Creation, Modified Date and Disk Space to Media Info Panel.
- Removed Frame information from Start/End Time in Image Tab of Media Info
  Panel.
- Made Start/End Time in Image Tab of Media Info Panel not use scientific
  notation for seconds.
- Fixed changing frame rate from the Timeline toolbar not updating the FPS
  indicator in the Image Tab of the Media Info Panel.
- Improved the performance of Media Info Panel when playing a sequence and
  the Image tab was closed.
- Removed Default Speed information from Video Tab and moved it to Image Tab.
- Improved colors of deactivated enums showing too dark to read the black
  text by default.
- It is now possible to use OpenColorIO's built-in configs.  That is, those
  stating with ocio://.
- Added a Choice menu in Preferences->OCIO to select any of the three built-in
  OCIO configs.
- Set ocio default config to the built-in:
      ocio://default
- Fixed a focus issue on Linux when using a draw tool, then typing into one
  input widget in the toolbar and going back to the view window not ending the
  draw tool, but continue its trace.
- Fixed Text annotations when image was rotated jumping to the wrong location
  on confirmation.
- Entering a Text annotation on the viewport can be confirmed by doing
  SHIFT + Enter.
- Added FFmpeg's actual version information, not just each library.
- Fixed listing of codecs in the About Window.
- Added missing .gif decoder and encoder.
- Added mrv::TextBrowser to allow copying the text in them with a RMB context
  menu. This is useful, for example, in the About Window for the GPU or codec
  information, as the full lines get copied, unlike a Fl_Text_Display in which
  a line can be cut in half.
- Fixed gcc version information in mrv2's About Window.  It was showing the
  __GLIBCXX__ instead.
- Darby simplified and fixed the A/B comparison sometimes getting out of sync.
- Improved the look of HorSlider widget.  Now the X to reset to defaults is
  a tad to the right.
- Fixed selection of Wipe mode which would not work after scrolling in the
  docker (FLTK bug?).
- Made Compare options automatically frame the view if View->Auto Frame is
  on.
- Improved the playback performance of the viewer a tad.
- Fixed an OpenGL flickering of the timeline and sometimes of the main viewport
  when the secondary window was opened and then closed.
- Fixed cursor not showing in panels when drawing was enabled.
- Improved the performance of adding clips to an EDL Playlist.  There's no more
  flickering.
- Fixed thumbnails in the Files and Stereo panels resetting to their
  start time, instead of showing their current time.


v1.0.8
======

- Added rotation of the images +90 and -90 with automatic framing if
  "View->Auto Frame" is on.
- I have enabled TLRENDER_FFMPEG_MINIMAL as a default to compile FFmpeg with
  less muxers, demuxers, parsers and protocols, for smaller file sizes and
  a faster compilation and program start up.
- Made Help->Documentation point to the online docs if the local docs are not
  installed.
- Made H264 entry in Save Movie Options panel not show when FFmpeg was built
  as LGPL.
- Updated OpenUSD to v24.03.
- Fixed <| (go to start) and |> (go to end) buttons not refreshing the button
  status on playback.
 - Added -ics (-ocioInput), -od (ocioDisplay), -ov (-ocioView) and
   -ol (-ocioLook) command-line flags to override the default settings.
   Note that -ocioDisplay and -ocioView must be used together.
 - Made OCIO Input Color Space be stored with the image, except when doing
   comparisons in which the first image's Input Color Space is used.
 - Made OCIO Input Color Space for each image be stored in session files.
 - Made FFmpeg's repeated warnings and errors only show up once in the status
   bar and the logs.
 - Added a Resolution parameter to the Save Movie options.  You can save:
   	 * Same Size
	 * Half Size
	 * Quarter Size
   

v1.0.7
======

- Slow code clean up.
- Faster windows minimal compilations for quickly checking on GitHub.
- Added optional component installation on Windows's .exe installer.  Currently,
  you can install:
  
      * mrv2 application (obviously)
      * mrv2 documentation
      * mrv2 Python TK libraries
      * mrv2 Python demos

- Changed name of install directory of mrv2 on Windows from "mrv2 vX.X.X" to
  "mrv2-vX.X.X" to avoid spaces for easier scripting.
- Fixed image sequence detection that could happen randomly depending on how
  the filesystem returned the order of files.
- Added listing the OS and distro versions at the start and in About of mrv2
  for debugging purposes.
- Made panels be listed alphabetically, regardless of Natural Language.
- Automated version update in docs.
- Removed view sources from html docs to save space on disk.
- Cleaned up docs directory before building docs.
- Sped up creation of docs in build system.
- Updated Python API with FFmpeg and Background changes.
- Fixed reading permissions on files as they were broken!  I did not notice
  as I was using an NTFS drive which sets umask 0022 by default.
- Made reading session more robust to handle missing files or wrong settings.
- Made checking for readable files faster.
- Fixed saving large images/movies with annotations on work properly.
- Improved memory consumption of saving movies, particularly large ones like
  4K.
- Improved logging system logging the tlRender warnings.
- Improved logging system adding a Status mode to report information right
  away.
- FFmpeg's logging now prints out the codec/module where the error was
  generated.
- Improved movie save reporting of parameters.
- Fixed saving a movie with annotations when the movie is bigger than the
  viewport.
- Added GBR8/9/10/12 reading support for VPX.
- Fixed start and end timeline buttons not refreshing thumbnails in the Panels.
- Fixed seeking not updating the thumbnails in the Panels.
- Fixed image panel not refreshing its information when changing images and
  the playback was stopped.
- Fixed Preferences->Positioning->Position/Size when both were used.
- Saving of .otio files as movies when the first clip did not start at 0 now
  works properly.
- Upgraded to RtAudio v5.2.0 on all platforms.
- Added a "View/Auto Frame" to turn off auto framing of the view when changing
  from one clip to another.
- Added User Interface->View->Auto Frame to preferences. 
- Made saving a VP9 or AV1 with the wrong extension not fail.  Instead, they
  are renamed to .mp4.
- Made mrv2's file requester favorites listing automatically remove
  non-existent directories.
- When saving a sequence of OpenEXR, we encode the speed in the actual
  image file, as taken from the playback tool bar.  This value will take
  precedence over the Sequence Default speed as set in Preferences->Playback.
- Made warnings also show up in the status bar, but with an orange background.
- Improved HUD Attributes.  They are now listed alphabetically and they are not
  repeated.  Also, they refresh properly.
- Fixed Media Information panel not refreshing properly when changing images.
- mrv2's tlRender library now reads the video and audio stream metadata.
- When there's no audio metadata there's no longer the titles of
  Attribute/Value used at the end of the Metadata tab in the Media Info Panel.
- Metadata in Media Info Panel is now sorted and stripped of repeated data.
- Media Info Panel's tabs now remember whether they were opened and closed.
- Added GoPro Cineform codec to the list of profiles you can use to encode.
  The biggest benefit is that it can encode at GBRP_12LE and GBRAP_12LE (ie.
  RGB and RGBA at 12 bits with alpha) for a replacement to Apple's proprietary
  ProRes4444 codec.
- Removed the GoPro prefix name from Cineform when saving.
- Fixed saving of EXR movies to movie files being just RGBA_U8 instead of
  RGBA_U16.


v1.0.6
======

- Darby fixed Render->Alpha Blend modes which were partially broken.
- Darby fixed macOS checkers drawing.
- Darby added gradient background and moved it to draw video.
- Made Background colors use Flmm's color chooser with alpha (alpha is
  current discarded, thou).
- Added printing out of audio and video codec names when saving with FFmpeg.
- Fixed presets names.  Instead of using '_' we now use '-'.
- Added last successful saved movie to recent files for easy checking it back.
- Fixed pixel format selection in VP9 codec not working properly due to
  profile being set to 0, which would prevent saving with 10 or 12 bits.
- Fixed Prores color shifting due to bad unneeded use of libswscale API.
- Added options for color_range, colorspace, color_primaries and color_trc in
  tlRender's source code.  mrv2 can set them if you open the window called
  Advanced Settings in the Save Movie dialog.
- Made mrv2 remember the save movie and image settings for easy saving of
  multiple movie or image files.
- Made FFmpeg errors be reported to the console immediately.
- Fixed macOS VideoToolbox's hardware encoding not working.
- Fixed Windows' presets not being read due to spaces in the installed
  directory.
- Fixed a long standing bug of gamma (when changed in the top bar slider)
  not being reapplied when changing from one clip to another.


v1.0.5
======

- Code clean up.
- Build system clean up and consolidation with environment variables.
- Fixed building pyFLTK on Linux which could fail if LD_LIBRARY_PATH was not
  set.
- Improved building speed on all platforms.
- Fixed Windows' Python compilation screwing up if some other Python version
  was installed.
- Fixed pyFLTK compilation on macOS and Linux having swig not use the right
  path includes.
- Fixed NDI compilation which had gotten broken.
- Fixed listing of movie files when they were named as sequences. 
- Allowed saving movie files with the speed (FPS) as set in the playback
  toolbar as long as you are not saving audio.
- Updated version in web page docs.
- Added FFmpeg presets for saving codecs.  Currently we ship mjpeg (none),
  h264, vp9 and av1 presets, but you are free to create your own.
- Switched to building with gcc-12 on Rocky Linux 8.9 (not in Dockerfile or
  Github builds).
- Fixed encoding of movie files' YUV conversion.  Now the movie file is much
  more accurate.
- Added pixel formats currently supported for each codec.
- Added saving alpha channel in ProRes_4444 and ProRes_XQ when YUVA_4444P_16LE
  pixel format is selected.
- Added saving alpha channel in VP9 when YUVA_420P is selected and the container
  is a Matroska file.
- Added HISTORY.md file to the Web docs.
- Added reporting of FFmpeg module that raised the callback.


v1.0.4
======

- Code clean up.
- Made Environment Map spin and rotation take into account zoom speed for a
  more controlable rotation.
- Made spin and rotation of environment maps much more controlable, regardless
  of zoom speed.
- Fixed spin/rotation of environment map not stop playback of video on middle
  button release.
- Mostly avoided gimbal lock on environment map rotations.
- Added VP9 Profile for saving in FFmpeg.  The VP9 codec is supported by most
  browsers nowadays and offers a better compression ratio than H264, without
  any patents or license issues.
- Improved remaining time calculation when saving movie files.
- Fixed message in saving movie of sequences without audio reporting that audio
  was getting saved.
- Added support for AV1 codec decoding (libdav1d codec).
- Added AV1 Profile for saving with AV1 codec in FFmpeg (SVT-AV1 encoder).
- Fixed a number of movies which would not play in mrv2 due to the number of
  threads and codec.


v1.0.3
======

- Added Darwin arm64 beta builds (ie. M1 architecture) but without NDI® support.
- Fixed a problem when saving OpenEXR not setting the format to RGB_F16, but
  trying to use RGB_U8 instead.
- Added the options to show OpenEXRs with data windows bigger than their display windows.  You activate it with View->Ignore Display Window and it will reload the exr image or sequence.
- Improved drawing of Data and Display Window at high resolutions.
- Fixed zooming when Media Information was active on an image sequence.
- Fixed a refresh of mrv2's custom file requester when saving a single image
  over a previous image file.  The icon would not get refreshed previously.


v1.0.2
======

- Added a Gigabytes setting to NDI® Panel to allow reproducing 4K and higher
  movie files with synced audio.
- Fixed NDI® streams playing with audio sometimes hanging at start up.
- Added a "No Source" to disconnect from NDI®.
- Made "Remote Connection" not be a valid NDI® connection.
- Fixed closing and reopening panel not showing the selected source.
- Fixed hang ups when switching sources in NDI® panel with videos with audio.
  Videos without audio were fine.
- Removed references to NDI/Source Index.
- Improved startup times of NDI® Sources with Audio.


v1.0.1
======

- Fixed relative paths function on Windows returning an empty path when the
  path could not be made relative.
- Made ALT + RMB zooming a tad less sensitive.
- Fixed ALT + RMB zooming sliding incorrectly.
- Improved speed on exiting the application.
- Made zooming with RMB + Alt slower for users with tablets.  
- Added an options to Preferences->Playback called Auto Scrub Playback, which
  when turned off will turn off the audio playback while scrubbing, like
  on previous versions of mrv2 (v0.8.2 and earlier).
- Added NDI® support in source code.  You need to compile mrv2 against the
  NDI® SDK for now.


v1.0.0
======

- Improved performance of scrubbing when audio is turned off in the timeline
  section of the main UI.  Helps in scrubbing 4K movies.
- Made readBehind cache expand freely when using Gigabytes in Settings
  to improve the performance of scrubbing 4K movies.
- Fixed a crash when showing an .otio timeline with markers at start or when
  dragging a clip with .otio markers.
- Added display of transitions (Dissolves) to Timeline Viewport.  If you edit
  a clip that has transitions, those will be removed before the move.
  Currently, there's no way to add transitions again (you need to edit the
  .otio file manually, or convert it from a Non-Linear Editor format).
- You can also not currently move or scale the transitions.
- Fixed default versioning regex for '_v' to match UNC paths on Windows.
- Fixed version of USD which showed MaterialX version instead.
- Updated USD to v23.11.


v0.9.4
======

v1.0.0 of mrv2 will be released on January 1st, 2024.  Therefore, it is of
utmost importance that you report any bugs you find before that.

- Improved Help/Update mrv2 Python script to work fine at start up on beta
  builds.
- Added the option to Update mrv2 even if you are already using the same
  version.  Useful to upgrade from a beta build to a release build.
- Improved performance of Python Panel by not having it wrap at bounds.  This
  allows faster resizing of the panel on Linux mainly.
- Fixed cursor shape in Python Panel when entering the divider between the
  output and the editor.
- Prevented resizing of the Python Panel to very small sizes when in window
  mode.
- Added $HOME/.local/lib/python${PYTHON_VERSION}/site-packages to the default
  search path of PYTHONPATH in environment.sh.
- Fixed Pixel Bar showing up when set to auto hide pixel bar on playback and
  in presentation mode.
- Made the command-line -p (-playback) switch override the default Auto
  Playback preference setting.
- Switched the default hotkeys of the Pixel Bar (was F2, now F3) and timeline
  (was F3, now F2) to make them consistent to the order on the view window.
  For that new hotkey assignment to take effect, you need to reset the hotkeys
  in Windows->Hotkeys or remove $HOME/.filmaura/mrv2.prefs.
  Note that that means you will loose any custom hotkeys you may have.
- Added support for .ts movies.
- Fixed OCIO support on network connections, changing both the pulldown menus
  as the OCIO color panel, which had gotten broken on the OCIO optimization and
  the OCIO looks addition.
- Added option to File->Save->Frames to Folder.  First, you will be asked
  to save an image and settings as usual.  However, after you save the first
  image, every time you invoke this function, it will save the image with the
  name and frame number without asking for confirmation.
  If you want to reset saving the image name, go to File/Save/Single Frame
  as usual.
- Some minor speed optimizations from Darby Johnston.


v0.9.3
======

v1.0.0 of mrv2 will be released on January 1st, 2024.  Therefore it is of most
importance you report any bugs you find before that.

- Made calculation of Actual Frame Rate more robust by averaging it over
  multiple frames.
- Added OCIO looks to the GUI, to Python API and to OpenGL display.
- Added OCIO to Color Panel, so that it becomes clearer what you have selected.
- Removed deprecated OCIO scene_linear space.
- Corrected popup-menu pulldown changing label even when enable label was
  disabled.
- Fixed OCIO view pull-down menu value being selected from the last display
  instead of the actual selected and used display at start-up.
- Made pixel bar not show up after going to Preferences if video clip is
  playing and Auto Hide Pixel Bar is on.
- Added OCIO Looks loading/saving to mrv2.prefs file.
- Added rotation and flipping for YUV formats to handle movie files taken from
  a phone like an IPhone.
- Removed setting OCIO ICS, View and Look by index as they were not taking into
  account the submenus.
- Solidified OCIO ICS return and set functions to support submenus.
- Made Panels appear in the same order that they were when exiting the
  application, instead of showing alphabetically.
- Documentation is now online at:
  		English: https://mrv2.sourceforge.io/docs/en
		Spanish: https://mrv2.sourceforge.io/docs/es

v0.9.2
======

- Added .otioz to Windows file associations installer and uninstaller.
- Fixed Hotkeys not working.
- Fixed dead hotkeys Shift 1 to 9 and 0.
- Made Hotkey entry work on just pressing a key, without having to type or
  having to select a special key from the awkward Special pull down menu.
- Fixed Meta (Windows) hotkey shortcuts on Linux.
- Added Zoom Minimum and Zoom Maximum to hotkeys, so you can turn them off.
- Caught Escape hotkey on Window callback to prevent it from exiting the
  application if it is not set to do so.
- Increased performance of playback on Linux when the timeline is visible at
  high frame rates (60 FPS).
- Increased performance of playback on Windows when the timeline is visible.
- Actual Frame Rate display in the HUD when FPS is selected (it shows
  Skipped Frames, Actual Frame Rate and Target Frame Rate).
- Made cursor disappear on Presentation mode after three seconds of inactivity.
- Added OpenGL controls for blitting the timeline or using shaders.  Blitting
  the timeline can improve performance on some graphic cards and OS.  In my
  tests Windows and Linux benefit from blitting while macOS benefits from
  using shaders.
- Added OpenGL control for Vsync.  Currently it works on Linux and macOS.
- Fixed default value of Minify / Magnify filters from the preferences to
  be Linear instead of Nearest.
- For programmers using VSCode, added .vscode directory with tasks to:
    * Build All mrv2 Project with all Dependencies (main compile -- runme.sh)
    * Build tlRender, FLTK and mrv2 (runmet.sh)
    * Build mrv2 only (runmeq.sh)


v0.9.1
======

- Fixed In / Out ranges when loading session files for clips that were not
  the one in playback.
- Fixed editing clips that had a timecode in them.
- Fixed an OpenGL issue mainly on Windows which would flip the video on Y when
  dragging it to create a playlist.  It could also lead to a crash.
- If OCIO config cannot be found, like when it is loaded from a session file,
  defaults to previous config.  Previously it could crash mrv2.
- Made internal checks for files that are not found and for replacing paths in
  path mapping deal with empty filenames properly.
- Fixed a random crash on Windows when loading a session file with
  Auto Playback set to on.
- Fixed an OpenGL flipping/flickering when Timeline Viewport was open and the
  user switched media items.
- Fixed Python's setOcioView() and ocioView() just returning the view name.
  Now both the display and view name are returned.
- Added ocioViewList() Python function to list all Displays / Views available.
- Added a setting for Display / View to Preferences->OCIO Defaults.  It will
  get used whenever the application opens or the user access the Preferences,
  overriding the setting set in the OCIO config file.
- Made panning work with CTRL + Left Mouse Button, besides the Middle Mouse
  Button on both the view and timeline viewport.
- Added Preferences->Playback->Single Click Playback to turn off playing by
  clicking on the main viewport, which was very annoying.
- Added Preferences->User Interface->Render->Minify and Magnify Filters.
  Moved Video Levels and Alpha Blend to this new preferences panel.
- Fixed color display (in both pixel bar and area color panel) not updating
  properly when single stepping through a file.
- Corrected handling of wstring characters on command-line and file associations
  on Windows start up.
- Improved Skipped Frame HUD display (SF:) when FPS is set to active so that it
  does not get confused when scrubbing.
- Fixed Skipped Frame HUD display not resetting itself when going to the first
  or last frame of the movie.
- Added Preferences->Playback->Auto Hide Pixel Bar, which when set, hides the
  pixel bar when playback is started.  This is to prevent slow-downs and
  skipped frames of clips at high resolutions.


v0.9.0
======

- Fixed issues with python library dependencies not copying the dependant DSOs
  on Linux.  This would lead to issues with libssl and libcrypto, for example.
- Fixed cmake's function get_runtime_dependencies() and
  get_macos_runtime_dependencies() only working for one element instead of a
  list.
- Created a python plug-in to automatically check the latest released version
  of mrv2 on github, allow to download it and install it, asking for a password
  if sudo permissions are needed.
  The plug-in is installed by default.  In order to have mrv2 automatically
  check for updates on start-up, you must set Preferences->Behavior and select
  Check for Updates at start up.
- Fixed a minor memory leak when saving movies with audio.
- Made Saving Audio only pop up its own file requester window.
- Thanks to the great Darby Johnston, it is now possible to edit the video and
  audio clips of an otio timeline in the timeline viewport.
- Added Preferences->Timeline the options to start in editing mode and start
  with Edit Associated Clips.
- Added the code and callback to edit the annotations when editing the clips.
- Added Edit/Audio Gap/Insert and Edit/Audio Gap/Remove to insert or remove
  audio gaps matching the length of the video clip at the current time.
- Fixed adding a clip without audio to a timeline with audio that ends before
  the video.  Now a gap is added before the new clip.
- Fixed changing of Timeline->Markers or Timeline->Transitions leaving too
  little or too much space.
- mrv2 now supports audio fading of clips.  Previously, only video would
  dissolve and audio would suddenly stop/start.
- Thanks to Darby Johnston, we now support playing back non-streaming movies
  directly from the http:// and https::// protocols.
- Fixed redraw issues under Wayland.
- Fixed muting of audio not working.
- It is now possible to change the font of the menus in:
     Preferences->User Interface->Fonts.
- Fixed Wayland support on modern platforms like Ubuntu 22.04.3.  Under Rocky
  Linux 8.1 builds, running with more modern distros under Wayland you may
  encounter an error about missing "antialising".

  To fix it:
    
```
$ sudo cp /usr/share/glib-2.0/schemas/org.gnome.settings-daemon.plugins.xsettings.gschema.xml /usr/share/glib-2.0/schemas/org.gnome.settings-daemon.plugins.xsettings.gschema.xml.bad
$ sudo nano /usr/share/glib-2.0/schemas/org.gnome.settings-daemon.plugins.xsettings.gschema.xml
       	    (remove lines 19 and 20)
	 <   </schema>
	 <   <schema id="org.gnome.settings-daemon.plugins.xsettings.deprecated">
$ sudo glib-compile-schemas /usr/share/glib-2.0/schemas
```


v0.8.3
======

- Fixed saving of L_U16, LA_U16, RGB_U16 and RGBA_U16 movies which were
  flipped on Y.
- Sorted Panels in the Panel menu alphabetically instead of by shortcut.
- Fixed Alpha Channel saving images on Annotations when movie was RGBA_U16.
- Now you can concatenate .otio clips into the EDL Playlist.  Just drag the
  .otio clip to the Playlist Panel where you have the temporary EDL
  or to the Timeline Viewport while selecting the EDL and it will
  be added to any previous clips in the timeline.
  The .otio clips support in and out points and annotations.  Transitions are
  supported, but you cannot cut a transition in half with the in and out
  points.
- Fixed a crash when trying to load an inexistent clip from the command-line.
- Made dragging of a clip show the dragged clip in cyan in the Files Panel, to
  distinguish it from the selected one.
- Fixed the dragged clip being also selected after an unsuccessful drag when
  there was a clip selected.
- Made Panel shortcut keys in the menu be more separated from the actual name
  of panel.  Small UI improvement.
- Added shortcuts to Environment Map Panel (Ctrl + e) and
  Playlist Panel (Ctrl + p).
- Improved .otio Playlist creation. Now the audio channel is created only when
  needed.
- Fixed creation of .otio Playlist with a sequence that had the audio on disk
  with the same base name.
- Fixed frame stepping when there were in/out points in the timeline.
- Fixed text input color when creating a new folder in mrv2's custom file
  requester.
- Fixed scrollbar when creating a new directory in mrv2's custom file
  requester.
- When creating a directory in mrv2's custom file directory, the entry widget
  is placed at the end of all directories instead of at the end of all files.
- After creating a new directory, it is sorted back into the list of
  directories.
- Fixed Saving remaining time progress being incorrect.
- Made mrv2's file chooser recognize .otio and .otioz as OpenTimelineIO EDLs.
- Refactored and simplified code.
- Fixed clearing of cache resulting in cache starting again from the beginning.
- Fixed seek and timeline thumbnail preview being incorrect after an Edit/Cut
  or Edit/Insert on a movie with timecode.
- Improved interactivity of editing tools (cut, insert, slice and remove)
  due to cache no longer starting from the beginning but from the current
  position.
- Added license to all fltk demos.
- Made Save->Movie files optionally save with audio.  Note that while saving
  audio is not heard.
- Fix saving of additional frames when video is shorter than audio.
- Split Saving options between saving movies and saving images.
- Added saving of player's In/Out Ranges to session files.
- Fixed an OpenGL issue on Linux when saving a movie and dragging the window
  partially outside the screen.
- Split the Save Options popup into a Save Movie Options and a Save Image
  Options.
- Added a Cancel button to both Save Movie Options and Save Image Options.
- Turned off audio (as it would stutter) while saving movies with annotations.
- Added a different message to the Save Movie Progress Report to indicate
  whether you are saving with audio or not.
- Added a check when saving audio and there's no audio in the current clip.

  
v0.8.2
======

- Python commands to set the ocio config have been added to the image module.
  image.setOcioConfig() and image.ocioConfig().
- Python documentation has been updated.
- Python's Editor bug that would concatenate the last two lines together
  incorrectly has been fixed.
- Fixed playlist of adding a sequence when it had no audio at the beginning of
  the EDL.
- Fixed adding a sequence with audio to the EDL Playlist.
- Fixed parsing command-line audio files.  Now only the first sequence added in
  the command-line receives the audio file, instead of all files.
- Fixed most issues when mixing clips of different frame rates and different
  audio sample rates in an EDL.  Some precision issues seem unavoidable, thou.
- Synchronized menu items to python changes as some were not being taken into
  account.
- Added imageOptions to session loading and saving.
- Fixed swallowing of last character in Python Editor.
- Fixed nested parenthesis in last expression in Python Editor.
- Fixed Python Editor sometimes not running a multi-line expression.
- Added keyword constructors to all Python classes.
- Added support for YUVA formats in FFmpeg through RGBA conversion.
  This means both ProRes444 and webm (vpx) videos now support alpha channels.
- Fixed Preferences for Safe Areas not doing anything.
- Added Video Levels and Alpha Blend to:
     Preferences->User Interface->View Window.
- Made scrubbing automatically switch to playback with audio.
- Fixed coloring of Python functions when they were typed at the start of a
  line.
- There's a new 'session' module to handle everything related to sessions,
  including saving, loading and setting the metadata.
- The metadata for sessions has changed API.  Now it is a Python dict.
- Added libvpx on Linux and macOS which was missing.
- Added support for decoding webm (vpx) videos with alpha channel.
- Fixed crashing issues on macOS at start up due to brew libraries being
  loaded instead of the mrv2 shipped libraries.
- Fixed sequence detection when the sequence would reach the number of padded
  digits.
- Fixed Auto Playback working only for the first clip loaded.
- Fixed playback buttons when switching clips not showing playback.
- Fixed a random OpenGL error when creating the color texture in the main
  viewport.
- Fixed EDL creation for movies that did not have audio.
- Fixed selecting the wrong clip when loading a session from the command-line.
- Added Background panel to change solid color, checker size and checker colors.
- Made session files try to store relative paths to clips and OCIO config so
  as to be able to use them on different platforms.
- Made routine for relative paths return the original path if the path could not
  be translated into a relative path.
- Fixed the annoying macOS bug where the timeline viewport elements would not
  get drawn sometimes.
- Fixed adding a movie with no audio, which created an audio gap of sample
  rate of 1, leading to precision issues.
- Fixed Annotations shifting when moving clips around in the Timeline Viewport
  (feature of v0.9.0 not yet released by Darby).
- Added support for HDR Radiance (.hdr) format, both loading and saving.
- Fixed a crash when deleting the last clip from the Files Panel.
- Added a Go to/Previous Clip and Go to/Next Clip for .otio files, using
  Ctrl + Right Arrow and Ctrl + Left Arrow.


v0.8.1
======

- This is a quick bug fix release to the Playlist creation which got broken.

v0.8.0
======

- Fixed mrv2's file requester not selecting files with [] in them.
- Fixed mrv2's file requester not changing directories if you had typed the
  full name of the directory in the filename field and pressed Enter,
- Fixed a crash when loading a session with no files.
- Added session name to the window's title bar.
- Fixed a typo in Python's binding to session (oepenSession instead of
  openSession).
- Made Save Session not save temporary EDLs in the session file.
- Added a '\_\_divider\_\_' tuple entry to Plug-in menus to add a divider line
  between menu entries.
- Made Python's output and errors automatically be sent to the Python editor,
  instead of waiting until the commands finish, like in v0.7.9 and previous
  ones.
- Added a cmd.getVersion() to get the version of mrv2 from Python.
- Made playback play with audio when changing frame rate (slower or faster).
- Made audio play when stepping through frames.  It is currently a hack and
  not a proper fix yet.  Also, the stepping buttons are not updated properly.
- Fixed a locale change when using the FPS pull-down and there were thumbnails
  present.
- Fixed macOS menu bar font size when switching from macOS menus back to
  normal ones.
- Made saving of .otio files also work from File/Save/Movie or Sequence if the
  extension given is .otio.
- Added user metadata to save in the session file as "metadata".  This can be
  set with the Python commands setSessionMetadata and retrieved with
  sessionMetadata.
- Added a warning check when saving a session with temporary EDLs in it.
- Added timeline.speed(), timeline.defaultSpeed() and timeline.setSpeed() to
  retrieve and manipulate the FPS.
- Added image.ocioIcs() and image.setOcioIcs() and image.ocioIcsList() to
  Python to set the input color space of the image.
- Added image.ocioView(), image.setOcioView() and image.ocioViewList() to
  Python to set the Display/View color space and to retrieve a list of all
  Display/Views.
- Fixed reading of OCIO file name in network connections.
- Color channels (layers) are now kept with the file so that switching between
  media will not revert to the rgba channel if there isn't an equivalent one.
- USD Panel is now interactive.  You can change the parameters and it will
  show the change.  The only parameter not recommended to change (except for
  very simple scenes) is the complexity.
- USD Panel visibility is now saved in the Preferences.
- Refreshing of cache is now done in seconds, without re-loading and
  switching an image as before.
- Creating a timeline in the Playlist Panel is also done in seconds.
- Fixed a crash when creating an empty timeline or a timeline from a clip in
  the Playlist Panel.
- Fixed missing frames (Repeat Last and Repeat Scratched) when the user was
  reading a different layer and he was playing backwards or stepping through
  the frames.
- Added drawing background as transparent, solid or checkers.
- Made dragging a clip from the Files Panel not loose the selection.
- Fixed a network error (harmless) about edit mode.
- Fixed Creation of EDL Playlist with image sequences.
- Fixed annotations copying from source clip to EDL Playlist when adding the
  clip to the playlist.
- Adding a clip to an EDL playlist will now positiong the current time at the
  frame of the new clip, instead of resetting it to 0.
- Session files also save and load the timeline viewport options (ie. Edit mode,
  size of thumbnails, transitions and markers).
- Thumbnail above the timeline no longer appears when there's no clip loaded
  after it was shown once.
- Fixed pixel zooming and panning on timeline viewport on network connections
  when the pixels per unit was not 1 (like macOS Retina).
- Added a Reset Hotkeys to default in the Hotkeys Window.  This will reset
  *all* your hotkeys to the default values of mrv2 (no need to mess with
  mrv2.keys.prefs).
- Added a Reload Hotkeys to the Hotkeys Window.  This will reload the last saved
  hotkeys (ie. $HOME/.filmaura/mrv2.keys.prefs).
- Fixed Annotations Clear Frame and Annotations Clear All Frames hotkeys being
  the same.
- Hotkeys are now compared properly when they are uppercase and shift was not
  used while another hotkey had shift and a the same lowercase letter.
- Some UI fixes and improvements:
    * The Zoom factor in the Pixel Toolbar keeps its value when selecting
      it from the pulldown.
    * All buttons and displays have the same size on both the timeline and
      pixel toolbar.
    * Cursor in all input fields is now red for easier reading.
    * The FPS input widget now displays the FPS with different number of
      digits to fit the value as best it can on the limited width.



v0.7.9
======

- Fixed sequence of images detection when there was an image with the same
  basename, directory and prefix but no number.
- Fixed the RAW image reader to handle images that are smaller once decoded.
- Fixed RAW reader not supporting LCMS2 nor jasper.  Now they are supported on
  all platforms.
- Added Sigma .X3F RAW support to RAW Reader.
- The RAW Reader properly reads all files but two from:
      https://www.rawsamples.ch
- Fixed an OpenGL refresh/redraw issue when going to the Preferences
  Window and returning with new settings.  It could also provoke a crash on
  Windows.
- Made the Log Panel not save its visibility upon program exit.  This prevents
  the Log Panel Window from opening on a new start up of mrv2 as it is usually
  not wanted, but happens once there had been an error in the previous instance.
- Linux uninstall with DEB and RPM has been improved.  Now, instead of just
  removing the symlink of /usr/bin/mrv2, the symlink is changed to point to the
  latest version installed if any.
- You can now open only one instance of the viewer and new images opened will
  be sent to the already opened viewer, instead of opening multiple instances
  of the viewer.  You set the behavior in:
```  
Preferences->User Interface->Single Instance.
```	
- Removed all warnings and errors from the Sphinx documentation generation.
- Fixed positioning of text editing when re-editing a text annotaion.
  Previously, it could be offset quite a bit if the zoom was not 1.
- A lot of source code clean up from mrViewer's bad and old source code.
- Windows installer now will remove the file association first before replacing
  it with the one of mrv2.
- Made executable smaller on all platforms by using dead-code elimination.
- Added pyFLTK bindings to the distribution.  It is now possible to create
  FLTK windows with Python and control and access mrv2's windows with it.
- Added Find and Replace to Python Editor in the Python Panel.
- Added Comment and Uncomment region to Python Editor in the Python Panel.
- Made focus selection on the main view window not work upon just entering.
  This solves the issues with losing focus on the Frame, Start Frame and End
  Frame widgets.  It also fixes problems when showing a Python window which
  would otherwise not stay on top.
- Added Save/OTIO EDL Timeline to the menus.
- Allowed Saving OTIO Timelines of single clips and other .otio files.
- Build system changes and fixes:
   	* Renamed runme.sh script to runme_nolog.sh.
   	* Added a new runme.sh script that calls runme_nolog.sh but saves the compile log into BUILD-KERNEL-ARCH/CMAKE_BUILD_TYPE/compile.log.
   	* Updated windows build script to work with MSVC 2022, not just 2019.
   	* The Windows compilation takes advantage of Msys to install the dependencies of libintl, libiconv and gettext.
   	* Made all compile options work when off.  You can customize mrv2 to build it with either all the bells and whistles or pretty barebones.
   	* Added --help flag to runme.sh to list all the possible settings.
   	* All bash commands are run with run_cmd which prints them and times them.
   	* Added optional build support for all the optional TLRENDER_* settings and for the MRV2_* settings.  It is now possible to build a light version of mrv2 or one with all the features.
   	* Fixed a problem with the embedded python (pybind11) locating the system Python installation instead of the local one instead.  This created havok on my Ubuntu 22.04 when there was an upgrade.
   	* Made parsing of -D options like -D TLRENDER_USD=OFF work with or without a space.
	* Got rid of compiling the slow Gettext on macOS, replacing it with
	  a prebuilt dylib.
	* Made the build system automatically release beta versions of the software after each successful compilation.  You can now download the latest beta binaries from:
     
         https://sourceforge.net/projects/mrv2/files/beta/


v0.7.8
======

- Added a hotkey selection to switch pen colors.
- Fixed hotkey search highlight redraw showing the wrong hotkey.
- Made pen size be adjusted to match the resolution of the image.
- Made eraser pen size be twice and a half the pen size by default for easier
  drawing and erasing without having to bring the annotation panel.
- Made saved color presets in the color picker be sorted from top to bottom,
  where the top color is the last one used.
- Added saving the old color to the preferences.
- Fixed saving current EDL through Python API.
- Added a command-line switch (-resetHotkeys) to reset hotkeys to their default.
- Added a button in Settings to reset hotkeys to their defaults.
- Python API now supports Playlists again with a new API.
  You can add new clips to an EDL playlist, list all playlists,
  select a playlist based on its index, its name or its item
  and save the selected playlist.
- Made FileItem paths read-only in the Python API.
- Fixed the beginning of English playback documentation which was in Spanish.
- Improved the User Documentation.
- Fixed name of some hotkeys in Spanish locale.
- Made playback buttons change background color when playing or stopping.
- Added support for Camera RAW formats through LibRaw.
- Fixed File->Save Image and File->Save Movie resetting the UI.
- Improved Windows build system dramatically.  Now there are bash scripts to
  compile all the GNU-like dependencies (FFmpeg, libx264, libvpx and liblcms2).
- Added x264 to the Linux and macOS builds but it is turned OFF by default.
- Made right mouse button text size match that of the main menu bar.
- Fixed Preferences->Playback->FPS not doing anything.  Removed
  Preferences->Playback->Override FPS.
- Added a way to edit baked Text Annotations from a frame.
  If you click with the Right Mouse Button on the Text tool, a pop up Window
  will appear displaying a pull down with all your text shapes for the
  current frame.  Selecting one, and clicking on Edit Text will take you to
  the Text widget where you will be able to reposition it, re-edit it, etc.
  This also works in network connections.
- Made text widget font size be dependant on the render size of the image.
- Improved scripts in bin/ directory with help messages.
- Moved python scripts in bin/ directory to bin/python.
- Improved runme.sh script to accept a -gpl flag to compile FFmpeg with libx264
  support in GPL mode on all platforms.  The default is still to build a LGPL
  FFmpeg without libx264 saving support as that complies with the BSD license
  of mrv2's source code.
- Improved README.md build instructions to document the optional building of
  FFmpeg.
- Fixed saving movies when saving without annotations leading to bad redraws
  later on in the timeline.
- Fixed resizing of window when an image is loaded or the Fit Window to Image
  button is pressed.  Now it will correctly zoom to 1 if it fits it.
- Made tiling slider of Timeline Viewport darker so it is more visible.
- Fixed order of clips loaded from the command-line being in reverse order.
- Fixed Settings Panel FFmpeg I/O Threads not accepting 0.
- Simplified hotkeys loading and saving and now hotkeys are saved on exit.
- Made command-line support any number of files instead of just three.
- Fixed session saving which got partially broken in v0.7.7.
- File Panel thumbnails now update for the selected stereo and the compare
  media too.
- Compare Panel thumbnails also update for the A and B images.
- Stereo Panel thumbnails also update for the A and Stereo images.
- Command-line -b <image> for the compare image now properly selects the
  image in the compare panel.
- Fixed a precision issue with annotations which could make some of them
  disappear.
- Fixed go to next and previous annotations when several clips of different
  rates were present in the timeline.
- Fixed floating timeline thumbnail not updating properly when showing an EDL.
- Added shifting of annotations when tlRender's darby routines drag an item
  into new place.
- Made Fit ('f' key) in timeline viewport work on network connections.
- Made Panning (Middle mouse drag) in timeline viewport work on network
  connections.
- Fixed Edit button on network connections.
- Fixed seeking on network connections when the viewport was zoomed in and
  the windows' size in the local and remote machines were different.

v0.7.7
======

- Fixed adding a file to playlist when the path was empty (ie. the file was
  loaded from the current directory).
- Fixed adding audio to a playlist when there was an empty audio track and a
  video clip (ie. a sequence of images and then a video clip).
- Fixed Text annotations having been turned off by mistake in v0.7.5.
- Fixed drag and drop on Windows only allowing to load 4 clips before not
  allowing drag and drop to work anymore.
- Fixed file requester hanging when reading .py files in a directory.
- Updated to OpenEXR 3.2, OpenColorIO 2.3.0, etc.
- Fixed Frame/Timecode/Seconds display in the PDF Exporter which was showing
  always 0.
- Fixed PDF export to save out annotations in increasing time order.
- Added User Documentation in English and Spanish, roughly based on xStudio
  documentation.
- Added 7 saved color presets to the color picker, like Krita, so you can
  easily choose from them.
- Added two colors to drawing tooldock.  You can switch between them with the
  arrows that point to them.
- Wipe Comparison is now fixed which had gotten broken in v0.7.5.  Thanks to
  Darby Johnston.
- Fixed Fullscreen Mode (F11 hotkey) on Windows which got broken in v0.7.5.


v0.7.6
======

This is mainly a bug fix release to Edit features and general issues found
with v0.7.5.

- Fixed hotkey check when entering a hotkey of the first 5 entries (a legacy
  from mrViewer).
- Fixed Load/Save hotkey file requester on Windows that would redraw
  incorrectly.
- Fixed message about corruption in hotkeys, when the reason was a new
  forced hotkey.
- Fixed Windows installer not installing the icon on Windows 11's Settings->
  Apps->Installed Apps.
- Fixed a memory corruption when pasting or inserting one frame of audio and
  playing back in reverse.
- Fixed log window showing up when there was a corruption on hotkeys even when
  the Preferences->Errors->Do Nothing was set.
- Fixed copying frames from one video to another even when they have different
  frame rates.
- Added an option in Preferences->Timeline to remove the EDLs from the
  temporary directory once the application exits.
- Improved focus handling of current frame, start frame and end frame widgets,
  which would loose it once the cursor was moved to the timeline.
- Fixed Edit/Frame/Insert when the movie had timecode in it and did not start
  at 0.
- Improved quality of Windows' icon.


v0.7.5
======

Playlist and Editing
--------------------

This is the first version that supports some basic editing and improves upon
the playlist panel by making it interactive.

- The Playlist panel's functionality has been simplified. It is there only to
  create an empty track or start a new EDL with one clip from the Files Panel.
- Added an Edit/Frame/Cut, Edit/Frame/Copy, Edit/Frame/Paste and
  Edit/Frame/Insert to cut, copy, paste and insert one frame (video and audio)
  of any media.  Currently, it does not support transitions, that are removed.
  As soon as one of these commands is used, a new EDL is created.
- Added an Edit/Slice to cut a clip in half at the current time location.
- Added an Edit/Remove to remove the clips that intersect the current time
  location.
- Added Drag and Drop functionality to the Files Panel into the Timeline
  Window as well as to the Playlist panel to add clips and create an EDL.
- Currently, there's still no support for trimming the Timeline clips yet.
  
- Documented Python USD module.
- Fixed DWA compression on non English locales (with commas as decimal
  separators)
- Allowed saving movies as EXR frames if Annotations is turned on.
- Fixed Media Information Depth display for floating point lumma and lumma with
  alpha images.
- Added pixel type saving to OpenEXR saving.  It can be Half or Float when
  Annotations is on.
- Added all libraries and their versions (when possible) to the About window.
- Fixed tlRender's version macro.
- Changed OpenEXR saving to use multipart api to allow future support to save
  all layers of an exr.
- Fixed edit viewport leaving room when show transitions was active but there
  were no transitions in the timeline.
- Added Timeline Preferences to show thumbnails, transitions and markers.
- Fixed pixel aspect ratio of saved OpenEXR images when they were not 1.
- Made Window resizing take into account Editing Viewport at start up.
- Fixed Log Panel when an error was shown to resize to the size of the window
  and not smaller.
- Log Panel will no longer open when the file requester is open. 
- Fixed Undo/Redo of annotations, which was incorrect.
- Fixed keyboard (menu) shortcuts not working in the Files Panel.
- Annotations are now kept with RationalTime instead of frames to be more
  precise.  Note, however, that old session files that use annotations will be
  incompatible.
- Fixed Network connections which had gotten broken on v0.7.1.
- Fixed Network connections on client startup, leading it to change the
  selected file on the server.
- Added Edit mode to the sessions file.
- Added Edit mode to the network connection (it will load as timeline or full).
- Laser annotations are no longer added to the draw undo/redo queue.
- Laser annotations now work properly on Network connections.
- Fixed Recent Files with entries with backslashes (ie. '\').
- Added new controls to Playlist panel.  Added a new Save icon.
- Fixed an annoying repositioning of window when loading new clips.
- Added support for Markers in timeline viewport.
- Made FPS display show only three decimal digits to simplify.
- Added File->Save->Single Frame to save a single frame only.
- Sped up Python compilation on Windows.
- Fixed OpenColorIO Active Displays and Active Views when they were set to an
  empty string which would turn off the View menu.
- The OCIO Defaults now has an option to use or ignore active_views and
  active_displays on the OCIO .config file.  The default is now to ignore them,
  as it was suggested using them in production was usually not the hassle.
- Fixed Image Information Panel size when it was saved as a window with the
  tabs open.
- Made Network connections more solid.  In case of wrong data sent through the
  network, it will discard it.
- Fixed Environment Mapping editing of the subdivisions no longer changing the
  sphere.
- Fixed menus still showing the panels open when they were closed from the
  Close button in network connections.
- Fixed a crash when selecting a new clip with the <- or -> arrows in the
  Files Panel.
- Fixed changing of volume and muting on network connections not showing the
  change on the remote client's interface.
- Added "Save/Single Frame" to save the current frame as an image.
- Fixed Timeline redraw issues on Windows.

v0.7.1
------
- Made Secondary Window respond to menu shortcuts, like F12.
- Made Secondary Window resize to Presentation mode or Fullscreen if it is
  present, instead of the normal viewport.
- Fixed Timeline redraw when playing the movie and the Secondary window was
  closed.
- Made default pen color (if not saved in preferences) be yellow to avoid
  conflicts with green screens.
- Fixed a random crash on Linux when using the -h or -v flags due to forcing
  an exit (NVidia driver would crash).
- Added usd python module and usd.setRenderOptions method.
- Fixed default values of USD stageCacheCount and diskCacheByteCount.
- Added laser drawing to annotations.  This allows the shape to not be
  permanent and disappear after a second.

v0.7.0
------
- Added Edit view (OpenTimelineIO) with thumbnails and audio waveforms, courtesy
  of Darby Johnston.
- First pass at USD OpenGL support courtesy of the great Darby Johnston.
- Added USD panel and -usd* command-line switches to control the quality and
  behavior of the USD display.
- Fixed pixel aspect ratio of OpenEXR, Cineon and DPX images when run on a
  locale that uses commas as decimal separator.
- Added Zip Compression support to saving OpenEXR images.
- Fixed Video Levels radio menus being toggle menus instead.
- Made menu items and pulldown labels smaller so they fit when mrv2 is sized
  to its minimum size.
- Fixed all overlapping widgets which could cause problems with FLTK.
- Signed the Windows installer with a self-certificate.  It does not prevent
  Windows and Chrome from complaining but it gives Publisher info.
- Fixed a minor memory leak when opening menus in the Python Editor.
- Added a Right Mouse Button menu to Log Panel to allow to copy text more
  easily.
- Fixed an incorrect use of OpenGL's GL_LINE_LOOP in a VAO.
- Fixed a flickering OpenGL issue when the Secondary Window was opened with a
  selection and then closed.
- Fixed incorrect use of OpenGL resources being shared with Secondary view
  leading to display issues.
- Made Secondary Window also display the name, type and audio of the
  video/image being played.
- Fixed Wayland and XWayland off-screen framebuffers.  Wayland support *must*
  be compiled with a recent Linux version like Ubuntu 22.04 LTS.  The binaries
  we distribute are compiled with a very old version of Wayland.
- Fixed a potential OpenGL redraw issue when drawing both soft and hard lines.
- Made draw cursor be a white/black shape for easier display.
- Fixed RPM package to install to /usr/local/mrv2-v${VERSION]-Linux-64 without library conflicts.  You can now install it just with something like:

```
  $ sudo rpm -i mrv2-v0.7.0-Linux-amd64.rpm
```

- Made Linux .deb and .rpm installers set the mrv2 desktop icon to Allow
  Launching by default.
- Made Linux .deb and .rpm installers set xdg-mime file associations properly
  for video, image, otio and USD files.
- Added mrv2.io.Options class to Python bindings.  With it, you can set the
  options when running cmd.save() to, for example, save annotations.
- Added a Always Save on Exit to Positioning preferences to always save the
  positioning and size of the window upon exiting the program.
- Added support for .otioz (Open Timeline IO .zip files).
- Added annotations Python module to allow adding (add function) notes to a
  certain time, frame or seconds.
- Fixed timeline thumbnail caching the last thumbnails of the movie shown when
  switching or closing movies.
- Fixed missing frame scratch display when playing a gap in an .otio file.
- Added Right Mouse Button menu option to File Panel to copy the name of the
  file to the clipboard and to open the location of the file in your file
  browser.
- Session files now also store information from the color panel (color
  adjustments).
- RPM and DEB packages have the version number in them to allow installing
  multiple versions of mrv2.  Besides /usr/bin/mrv2 pointing to the last
  installed version of mrv2, symlinks with the version number in them are
  also creaetd, like:
       /usr/bin/mrv2-v0.7.0
  

v0.6.4
------
- Improved Python plug-in API.  Now plug-ins are defined with a base class,
  and menus with a dict (without tuples) like:

```
      class HelloPlugin(mrv2.plugin.Plugin):
          def hello(self):
              print("Hello from plug-in!")

          def menus(self):
              menus = { "New Menu/Hello" : self.hello }
              return menus
```

- You can have multiple plug-ins in a single .py and have the class be named
  whatever you like, as long as you derive from mrv2.plugin.Plugin.
- Improved the look of Gamma, Gain and Volume sliders.
- Fixed Window on Top check mark when run from the Context menu.
- Fixed Presentation mode not returning to its previous state when switched off.
- Fixed an internal OpenGL error.
- Fixed Playback menu status at the beginning when Auto Playback was checked.
- Fixed pixel color look-up when loading a single frame.


v0.6.3
------
- Added a python plug-in system which is now documented in the
  Help->Documentation.  The environment variable used to look up plug-ins is:

  	MRV2_PYTHON_PLUGINS

  It is a list of colon (Linux or macOS) or semi-colon (Windows) paths.
  Plug-ins are defined, like:

```
      class Plugin(mrv2.plugin.Plugin):
          def hello(self):
              print("Hello from plug-in!")

          def menus(self):
              menus = { "New Menu/Hello" : self.hello }
              return menus
```
     	

- Added a mrv2_hello.py plug-in for demo purposes.
- Fixed a bug in the log panel appearing compressed on start up when docked.
- Allowed creation of .otio files of a single clip in Playlist Panel.
- Fixed scratched frames showing up on .otio files with gaps in them.
- Fixed preferences not hiding the different bars anymore (regression in
  v0.6.2).
- Fixed Layer menu popup displaying "Default" for movies instead of "Color".
- Fixed Layer menu popup allowing you to change the layer when a single layer
  was available.


v0.6.2
------
- Fixed the Media Info Panel crashing on start-up when the panel was open and
  the media was an OpenEXR with multiple layers.
- Made timeline cursor be white for easier reading on the eyes.
- Fixed timeline cursor not ending in last frame when dealing with sequences.
- Fixed Auto Refit Image preference working only after a restart of the
  application.
- Fixed Media Info Panel showing up with scrollbars when mrv2 was started
  command-line and with a movie.
- Fixed Save Movie or Sequence and Save PDF Document allowing to be selected
  even when no movie was loaded.
- Fixed mrv2's File requester saving always overwriting the file that was
  selected instead of using the filename in the filename input widget.
- Improved file requester selecting a file or directory when typing.
- Added a ffmpeg_windows_gpl.sh script to compile a GPL version of FFmpeg with
  libx264 and libvpx suport with MSVC.
- Made mrv2's GL window swallow Left Alt key presses when pressed alone to
  avoid Windows' taking over the Window.
- Fixed some typos in English installer (thanks to BigRoy!).


v0.6.1
------
- Split the Save Session menu entry in two.  There's now a Save Session As and a
  Save Session.  The session filename is kept if the session file was loaded, so
  you can just use Save Session to overwrite it.
- Fixed creating of playlists with file sequences with absolute and relative
  paths.
- Fixed creating of playlists with different layers as it is not possible in
  .otio files to specify the layer to load.
- Fixed a refresh issue on color lookups that would show the previous frame
  values (or previous redraw values).
- Added video and audio codec names to the HUD Attributes and the Media Info
  Panel.
- Made all tabs in all panels adjust the packing of the other panels. Tabs
  open/close are also now stored in the preferences.
- Added nuke-default ocio config once again.
- Added studio ocio config to distribution.
- Added TGA, BMP and PSD 8 and 16 bit readers.
- Added TGA and BMP 8 bit writers.
- Added a Scripts/Add to Script List to Python Panel.  It allows you to store
  up to 10 scripts in the list and run them just by accessing the menu.
  The script list is saved in the preferences.
- Fixed window size on starting mrv2 when Dock Group was open.
- Fixed PDF thumbnail creation when the clip was taller than its width.
- Fixed annotations not keeping the soft parameter in session or network
  connection.

v0.6.0
------
- Added the options for missing frames on the Preferences.  You can now:
  	* Display black
	* Repeat last frame
	* Repeat last frame scratched
	
- Made loading of session files use Path Mapping for files and OCIO config
  so that if a session file is loaded from different OSes the files will be
  found.
- Fixed loading session from the command-line not showing the opened panels that
  were also open in the preferences file.
- Added the name of the layer to the thumnail description in the files, compare,
  playlist and stereo panels.
- Added anaglyph, scanline, columns and checkered stereo 3D.
- Added a new Stereo 3D Panel to control the stereo.
   * To use it, you load a clip with left and right views (usually a v2
     multipart openexr).  Then, open the Files Panel and select the clip and
     layer to use.
   * Open the Stereo Panel and select the Input to "Image".  That will clone
     the clip and select the opposite view (ie. right if you selected left).
   * Choose the Output for the Stereo 3D (Anaglyph, Checkered, etc).

- You can also use the Stereo 3D Panel with two clips (movies or sequences),
  but you need to set it manually.
   * Open the Files Panel, load the two clips. Select one of them.
   * Open the Stereo 3D Panel, select the other clip.  Then select Input as
     "Image".
   * Choose the Output for the Stereo 3D (Anaglyph, Checkered, etc).
  
- Fixed loading of multiple clips from a session messing up the video layers.
- Made movie's default layer be labeled "Color" to be consistant with images.
- Fixed OpenEXR's v2 multipart images with view (stereo) parameter.
- Fixed OpenEXR's v2 multipart images with changing data windows between frames.
- Fixed mrv2's native file chooser on Windows not cd'ing to the file path
  when the location input field was manually edited.
- Fixed playback starting when session was loaded command line and the session
  was not originally playing.
- Fixed thumbnail display in Files, Compare, Stereo 3D and Playlist panels.
- Fixed order of panels when loaded from a session file.
- Improved performance of exiting the application.
- Made HUD Attributes display the (sometimes changing) frame attributes.
- Added a Data and Display Window display option to the menus and to the
  view window display.
- Added compare and stereo options sent when a client syncs to the server.
- Made File/Clone (Right Mouse Button on Files Panel clip) respect the frame
  and playback state of the original clip.
- Added a File/Refresh Cache (Right Mouse Button on Files Panel clip) to
  refresh the cache.  This is useful when viewing a partially rendered
  sequence.
- Made thumbnails in Files, Compare, Stereo 3D and Playlist panels show the
  actual layer (color channel).
- Made timeline thumbnail reflect the actual layer (color channel).
- Allowed saving annotations to a PDF.  Both picture thumbnails as well as
  text notes are saved.
- Made Media Info Panel refresh every frame when there's a Data Window present.
- Fixed safe areas partially disappearing when zooming out.


v0.5.4
------
- Made Playlist thumbnail reflect the current or in times.
- Changed extension of Session files to be .mrv2s to distinguish them from
  .m2s video/audio files.
- Fixed copying of colors from the Color Area Panel.
- Fixed refreshing of timeline when Close All was executed.
- Fixed sending and receiving notes through the network.
- Fixed saving of annotations in session files that were on the timeline.
- Fixed loading of annotations from a session file.
- Made clicking twice on area selection open/close the color area panel.
- Fixed annotations' ghosting which was not fading in/out correctly.
- Allowed loading a session file from the command-line.  Just do:

    $ mrv2 test.mrv2s

- Added accidentally missing licenses of Python and pybind11 to docs/Legal.
- Added a File/Clone right mouse button menu option to Files Panel.  This is
  useful when creating a playlist of the same element but different in/out
  points.
- Added support for OCIO settings in session file.
- Added support for Color Channel (Layers) settings in session file.
- Added session files to the list of recent files.
- Fixed channel (layer) shown in the color channel pulldown when switching
  files.
- Fixed macOS start-up script not passing the command-line arguments.
- Made session file store and restore the current time.


v0.5.3
------
- Made area selection allow it to select 1 pixel easier by a single click.
  To disable it, you just need to switch to a new action mode (drawing, etc).
- Some users on older macOS versions reported problems with the Privacy
  mechanism of the OS on Documents, Desktop and Download directories.
  The problem is not there if we use the native file chooser.  I've switched
  the default on macOS to use the native file chooser.
- Added a soft brush for annotations on all shapes.  You access it from the
  Annotation panel which can be opened from the menus or by clicking twice on
  any of the draw tools.  The algorithm for smooth brushes is not yet perfect,
  as it can lead to an overlapping triangle on self intersections.
- Allowed splatting a brush stroke if clicking only once.
- Made Pen size in annotations go as low as 2 pixels.  One pixel tends to
  vanish and have issues when panels are open.
- Added license and code attribution to the Polyline2D.h code which was missing
  and I had lost where I downloaded it from.  I have further modified it to
  support UV mapping and indexed triangles.
- Fixed flickering of timeline thumbnail if switched to on first and then
  later set it to off in the preferences.
- Added a session file to store a mrv2 session (.m2s files)
  All files loaded, ui elements, panel values, etc. are saved and restored.
- Fixed a potential crash when using One Panel Only.
- Added Notes to Annotation Panel.  This allows you to add comments on a frame,
  without having to draw anything (or in addition to the drawn elements).
- Made view take the focus upon entering except when typing in the text tool.
- Fixed search in the Hotkey window which was missing the last character of
  the function.
- Fixed search repeatedly in the Hotkey window which was searching from the
  topline instead of from the last selected item.
- Allowed annotation drawing outside of the canvas once again.
- Fixed precision issues on annotation drawings.
- Made annotations respond to R, G, B, A channels changing.
- Removed ngrok documentation as it was incorrect for internet access.
- Fixed resizing of viewport not taking into account the status bar, leading
  to zoom factors of 1/1.04 instead of 1.

v0.5.2
------
- TCP Control Network port number is now saved in the preferences.
- Volume control is now saved in the preferences.
- Mute control is now saved in the preferences.
- Moved TCP volume and mute control to App.
- Fixed a bug in selection of items in Files Panel when two or more images
  had the same path.
- Fixed a bug in selection of items in Compare Panel which would show unselected
  files as selected.
- Added volume/setVolume to python cmds module.
- Added isMuted/setMute to python cmds module.
- Fixed resizing of log window when an error appears not remembering the user
  size settings.
- Fixed a horrible math bug in the calculation of zooming with Rig ht
  Mouse Button + ALT key.
- Made paths sent through network connections be garbled with a simple cypher
  scheme.
- Fixed bundle identifier on macOS having the same ID as the old mrViewer.
- Added -server, -client and -port command-line flags to start a network
  connection.
- Added documentation on how to establish a server-client connection on the
  internet using the free ngrok service.  This allows a single mrv2 server and
  a single mrv2 client to connect for free albeit for non-commercial projects.
  For multiple clients or commercial ventures, you need to pay for one of
  ngrok's plans or use another server of your choosing that will allow you to
  open a network port or remote ssh connection.
- Added parsing of hostname to extract tcp:// and :port from it.
- Upped the network protocol version used.  Now it is 2.  You can no longer
  use v0.5.1 with v0.5.2 or else the paths will get garbled.
- Fixed drawing and erasing of shapes getting drawn in different order.
- Annotations now can only be drawn inside the image instead of everywhere in
  the viewport.
- Fixed annotations ghosting not being drawn transparent in some areas and more
  solid in others.
- Made volume slider knob more attractive.
- Hotkey editor now has a close button on Windows.
- Fixed toggling of magnify texture filtering.
- Added hotkey entry for toggling minify texture filtering.
- Added magnify texture filtering to the list of hotkeys as it was missing.
- Added opacity (alpha) to drawing tools.
- Fixed a major memory leak when switching images which would show up mostly
  on Linux.


v0.5.1
------
- Made Path Mappings get saved to a different file (mrv2.paths.prefs) instead
  of the main preferences file.
- Fixed a Windows input of accented (foreign) characters in Text tool.
- Fixed on Windows opening files with spaces on them when the language was
  not the same as the language of the OS.
- Improved the Save Options file requester with FFmpeg and OpenEXR options
  (not yet functional in tlRender).
- Made double clicking on any of the annotation tools in the action dock
  panel toggle the Annotation Panel.
- Added Send and Accept Media to send and receive media files opening,
  closing and syncing.
- Improved drawing overlaps of multiple annotations.  Only when the erase tool
  is used does the drawing get reversed.
- Made cursor re-appear if drawing and using the right mouse button menu.
- Fixed saving of annotations in EXR images when they were big. 

v0.5.0
------
- Added networking to mrv2.  You can have a server and one or more clients and
  they will all colaborate with UI, pan and zoom, color transformations,
  playback, audio and annotations.  They can all be set to send or accept any
  item individually, from either the Preferences or the Sync menu.
  The server should contain the media to be reviewed.  Upon a connection by any
  client, the client will attempt to synchronize with the server.
  The sever and client are on a LAN and if both the client and server use the
  same paths to the media, the client will get all of its media loaded
  automatically.
  If they don't have the same paths, each file will be to the list of path
  mappings set in the Preferences.
  Finally, if that fails, the files will be compared on its base name
  and if matched, it will get accepted as the same clip, with a warning.
  If none of this is true, an error will appear, but the connection will
  continue.  However, syncing among multiple clips may show the wrong clip.
- Added Path Mapping to deal with paths being different on each platform, client
  or server.
- Fixed dragging of the timeline outside of the in-out range.  Now it will
  clamp the slider.
- Fixed a subtle bug in translations of Preferences' tree view which could lead
  to the wizard panel not show.
- Fixed a potential crash on log panel opening (when it was already opened).
- Fixed a bug on Windows and macOS that would size the panels beyond the bottom
  of the window.
- Added Environment Map options to python API.
- Fixed Luminance label spilling into the black areas of the pixel bar.
- Fixed Luminance tooltip flickering on macOS.
- Fixed Media Info Panel not showing up when the dockgroup was created for the
  first time.
- Fixed cursor disappearing on the action tool bar when a draw mode was
  selected.  Now it only disappears when it is in one of the views.
- Added saving of annotations when saving movie files or sequence of images.
  


v0.4.0
------
- Added Search on Hotkeys for functions and hotkeys.
- Updated all Python on every OS to 3.10.9, which is the sanctioned Python
  for VFX Platform 2023.
- Fixed resizing of dock and close button on macOS.
- Exposed all Python symbols on Linux when linked statically in mrv2 executable.
  This prevented on Linux from loading some external symbols on some libraries.
- Fixed PYTHONPATH on Linux and macOS to point to the mrv2 directory, whcih
  was preventing loading some modules.
- Improved Docker building by not cloning the git repository in the Dockerfile.
  The cloning now happens in the etc/entrypoint.sh script.
- Fixed mrv2.sh permissions on .tar.gz files.
- Added Reverse playback with audio!!!!
- Made input widgets in the timeline (current frame, fps, start frame and end
  frame), return the focus to the main window once you press return.
- Updated cmd.update() to return the number of seconds (usually milliseconds)
  the UI took to update.
- Updated the timelineDemo.py to play the clip for 5 seconds instead of a
  random number.
- Made Text input tool (widget) not loose focus when it is dragged somewhere
  else.
- Fixed loop mode at start not showing the appropiate loop mode.
- Made default loop mode be Loop.
- Updated to newer tlRender (new OpenColorIO 2.1, FFmpeg 6.0, etc).
- Due to changes in OpenColorIO, support for Windows 8.1 is no longer
  provided.
- Fixed Panel/Logs not showing as a toggle menu entry.
- We are also dropping support for 32-bit Windows machines, as it waa
  causing a lot of confusion with users downloading the wrong version
  from sourceforge.net when the amd64 (64-bits) version was not tagged as
  default or that it was called amd (and not Intel :)
- Fixed a random crash when invoking panels from hotkeys.
- Fixed hotkeys in menu bar not working when the menubar was hidden.
- Added all python libs to Linux distribution.
- Fixed a redrawing issue when the Media Information Panel was put as a window.
- Fixed zombie process on exit on Windows.
- Fixed Hotkeys window that had gotten broken in v0.4.0.
- Improved the performance of dragging panels as Windows (mainly on Linux).
- Added remembering of which tabs where open/closed in Media Information Panel.
- Fixed Spanish translations on Color Panel.
  

v0.3.8
------
- Changed language handling in preferences.  Now the locale code is stored.
- Removed all languages except for English and Spanish.  Note that on Windows,
  if you had Spanish selected, it will revert to English.  You will need to
  change it once again.
- Added reporting of memory use to HUD.
- Added Cache in Gigabytes to Settings Panel.  When this is non-zero the
  Read Ahead and the Read Behind are calculated automatically based on
  the Gigabytes number set here.  It divides it by image size, pixel type,
  fps and number of active movies.  It also takes into account audio, but
  poorly.
- Fixed a resizing issue on Python Panel, not resizing the tile group.
- Documented Python API in both English and Spanish, with Search browser.
- Fixed sorting of recent files so that they don't change order.
- Fixed reccent files to not list files that cannot be found on disk.
- Made recent files list the files in order of how they were loaded, with last
  loaded first.
- Fixed original pixel lookups on clips that have a pixel aspect ratio != 1.0.
- Fixed original pixel lookups on YUV420P_U16, YUV444P_U16 format.
  Missing testing YUV422P_U16, but it should work.
- Made audio volume and audio mute / track selection not active if the clip
  has no audio.
- Added number of Cache Ahead and Behind Video and Audio frames to HUD.
  If Ahead Video cache becomes 0 when playing forwards, playback will stop.
- Fixed Text tool input on Wayland.
- Removed libharfbuzz from the Linux distribution as it was causing trouble
  with some newer Linux distros.
- Added a Render->Black Background option to quickly switch from a gray
  background to a black background on images or movies that have an alpha
  channel.

v0.3.7
------
- Added a half OCIO default to handle OpenEXR half images.
- Added timeRange and inOutRange to timeline module.
- Added setIn() and setOut() to timeline module to set the in and out
  time/frame/seconds.
- Fixed timeRange conversion to string (__str__) and repr (__repr__).
- Fixed Presentation toggle from the menus and from the right mouse button menu.
- Added media.firstVersion(), media.previousVersion(), media.nextVersion(),
  and media.lastVersion() to move from one version of the clip to the next.
- Allowed saving of sequences if you use something like bunny.0001.exr.
- Allowed saving of .otio files with relative paths.
- Flushed the cout buffer.
- Added creating playlists from python.
- Fixed audio slider which would jump from 0 to 1 abruptly.
- Fixed resizing of panel windows when they were created first as windows,
  not from undocked.
- Panel windows now remember their undocked state even after being docked once.
- Added help text to viewport.  Now it will report when you click once on the
  viewport to Play or Stop the playback.
- Switching languages on Windows now works properly, both from the command-line
  and from the GUI.
- Fixed a crash on exiting the application.
- Made upgrading mrv2 more painless, as it will now update the OCIO config
  automatically to the new version, unless the path does not contain mrv2.
- Removed the outdated nuke-default OCIO config, replacing it with OCIO2's
  cgstudio config.
- Added Cut/Copy/Paste to Python editor (it was possible before, but just
  from the keyboard shortcuts).
- Renamed Python Editor's Python menu to File.
- Added a hint when playback is started or stopped by single clicking on
  the viewport.


v0.3.6
------
- Fixed Python Editor crashes (memory trashing).
- Made Python Editor remember its text when closed and reopened.
- Fixed Python Editor's coloring sometimes getting mixed up.
- Improved Python Editor's tabulation when a colon ends the line
  (to handle for, def, class, etc).
- Added a contactSheet.py demo for showing all the layers of an OpenEXR
  in Compare tile mode.
- Made cmd.compare() use the item index instead of item itself to avoid
  confusion when the same file was loaded more than once.
- Made CompareMode be part of the media module instead of the timeline module.
- Made mrv2 exit cleanly once the process calls _wexecv on windows.
- Fixed Compare Panel selection when paths were the same on two clips.
- Fixed a zombie process being left on Windows exit.

v0.3.5
------

- Bug fixed default OCIO input color spaces not being applied.
- Bug fixed an issue with scrubbing forwards not scrubbing smoothly.
- Bug fixed OCIO ICS when the color space had slashes (/) in it.
- Fixed printing of command-line arguments when run from cmd.exe or similar
  on Windows.
- Added a --version switch to command-line arguments to report version number.
- Made Drag and Drop in Linux work with other file requesters other than
  Nautilus (nemo, thunar, etc).
- Fixed sliders not appearing in Compare Panel.
- Fixed some missing libraries from Linux distribution.
- Fixed language switching on Windows when paths had spaces in them.
- Added Python bindings and a Python Panel with an editor and output window
  to run code interactively.
  There's not any documentation yet for it, but there are some sample scripts
  in the python/demos directory.
  Currently, you can:
     * Open images, videos and otio timelines.
     * Control the timeline.
     * Change colors and LUT config.
     * Compare two images and change the compare settings.
     * Change the layer of the image.
     * Change the R, G, B, A channels of the image.
     * Change the foreground (A) and compare (B) images either by index
       or by file media item.
     * Use libraries from the python standard library, except threads.

     The modules are:
     	 import mrv2
	 from mrv2 import cmd, math, imaging, media, timeline

v0.3.4
------

- Bug fixed a crash that would happen when the OCIO config was not found.
  This would happen mostly on Linux, when switching versions.
- Added popping the log panel when an error occurs if the preference is
  set that way.
- Fixed audio problems on Linux.
- Fixed a crash that would happen when the movie entered command-line was
  not found.
- Fixed a thumbnail exiting when the file was not being found.
- Added logging to all messages from the start of mrv2 on.  They can now
  be viewed in the Logs panel/window.
- Fixed log window popping up when errors are presented.
- Improved Pulse Audio complaining about devices in use on Linux.
- Fixed Spanish translation of main UI's tooltips and Preferences Window.
- Fixed threading hang up race condition which would mostly be seen on
  Linux.

v0.3.3
------

- Added a spin option to Environment Maps to instead of panning around with
  middle mouse, it allows you to push and spin in one direction.
- Added all licenses to docs/Legal.
- Fixed file attachments on Linux.
- Fixed unistaller on Linux to remove icon and desktop file from
  /usr/share/*.
- Fixed installer on Windows to not popup the file association panel if not
  requested to do so (it asks now, instead of listing as one the things to
  insall).  This is better as it allows us to translate into other natural
  languages that part of the installer.
- Added Natural Language translations (.mo files).  Currently only Spanish is
  provided.
- Added comprehensive documentation on how to translate mrv2 to other natural
  languages.
- Fixed a bug in thumbnails changing the group that it was attached.  This
  would effect the FilesPanel, ComparePanel, and PlaylistPanel.
- Fixed several crashes in the Prefereneces window.
- Fixed a race condition in the mrv2 File requester when creating thumbnails.
  This was most noticeable on Windows, where the thumbnails would get corrupted.
- Fixed a thread crashing on Linux when creating thumbnails.
- Fixed the logic in the OCIO file preferences which would prevent from
  selecting a new .ocio oonfig file.
- Made File/Open and Open button in the Files Panel open the movie and then
  play it if the Preferences' autoplay button is on.
- Fixed favorites directory in custom file requester not getting saved on Linux.
- Fixed xcb_ and _XRead multithread errors on custom file requester on Linux.
- Added stacktrace and signal handler routines on Linux and Windows.
- Fixed log window/dock to pop up when an error occurs.
- Mostly fixed audio problems on Linux when switching clips.  There can still
  be issues, but it is a matter of switching the clip again to make it work.
- Fixed text tool not working in v0.3.2.

v0.3.2
------

- Updated the build to rely on media-autobuild_suite exclusively on Windows.
- Fixed menu bar hiding not showing in the view menu properly
  (it was always on).
- Fixed Spherical environment mapping (not using a shader anymore).
- Added Cubic environment maps with the OpenEXR distribution.
- Fixed repositioning of text input field when clicking inside the text input.
- Fixed locating libintl.h on Windows.
- Added this HISTORY.md file to docs/ directory in distribution.
- Improved build instructions.
- Fixed mouse rotation of environment maps.
- Fixed middle mouse button click starting playback, like left mouse button.


v0.3.1
------

*******************************************************************************
- Linux Binary releases that work on Rocky Linux 8, RedHat 8 and Ubuntu 20.04.
*******************************************************************************

*******************************************************************************
- Added a Dockerfile for easy building and disting on all Linux distros.
  The base distro it builds on is Rocky Linux 8.
*******************************************************************************

*******************************************************************************
- Updated manual building documentation for Rocky Linux, Ubuntu, macOS and
  Windows separately to make it clearer.
*******************************************************************************

- The main executable is mrv2.exe (Windows) or mrv2.sh (Linux / macOS ).
- Fixed the build system to use mrv2 everywhere instead of mrv2 or mrViewer2.
- Added getting all .so dependencies in CMake to distribute the executable
  appropiately.
- Changed hard-coded file extensions to use Darby's IO plugin system.
- Fixed crash on Linux GNOME when using native file requester.
- Added tooltips to Read Ahead/Read Behind caches to clarify they are in
  seconds.
- Added single click playback and stop on the view window, like RV.
- Added Doxygen documentation (very incomplete).
- Added displaying of spherical environment maps in a virtual sphere
  ( courtesy of an open source OpenRV shader from The Mill ).
- Updated building documentation for Rocky Linux, Ubuntu, macOS and Windows.
- Added pen size change thru hotkeys.
- Fixed thumbnail creation on Windows.
- Removed memory leak of thumbnail creation.

v0.3.0
------

- Improved UI: menus, status bar, functionality.
- Moved status bar and status tool to bottom of the screen.
- Added preferences and menu toggle for status bar.
- Added a Panel menu to hold all dockable panels/windows.
- Added a One Panel Only toggle to show one panel at a time instead of packing
  all panels one after the other.  Floating windows are not effecte by this
  setting.
- Fixed video layer (channels) displayed when switching from one clip version
  to the next.
- Added a gamma switch to switch between 1 and the previous value.
- Added CONTRIBUTORS.md list.
- Automated version bumps in C++ code by looking at cmake/version.cmake.
- Fixed a refresh bug in FPS display when selecting Default FPS.
- I finally fixed a horrible FLTK crashing bug on thumbnail on timeline slider.
- Fixed a  crash when setting loop mode with no media loaded.
- Fixed playback of clips where fps did not match tbr.
- Fixed autoplayback when setting is set in the preferences.


v0.2.0
------

- Added support for multipart OpenEXR files.
- Fixed crashes on Windows due to time slider thumbnail.
- Made time slider thumbnail appear.
- Improved redrawing of thumbnails.
- Fixed crash on too long attributes when displayed in the HUD.
- Moved all tools into their own library (mrvTools).
- Fixed cursor drawing and slow performance of drawing tools.
- Fixed default gamma keyboard shortcuts not working.
- Added a rather rudimentary OTIO Playlist.  You select clips in the file
  window, change their in/out points and add them to the Playlist.
  When the playlist is done, you click OT Playlist and the clips are
  assembled in an otio file that is saved in $TEMP.
  Currently, you cannot nest OTIO files within another OTIO file.
- Added menu entry for Presentation mode.
- Added menu entries for deleting an annotation and all annotations
  from the movie.
- Made annotation menus appear as soon as a drawing is made.
