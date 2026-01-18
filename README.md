[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)
[![Build Status](https://github.com/ggarra13/mrv2/actions/workflows/full_linux.yml/badge.svg)](https://github.com/ggarra13/mrv2/actions/workflows/full_linux.yml)
[![Build Status](https://github.com/ggarra13/mrv2/actions/workflows/full_linux_arm64.yml/badge.svg)](https://github.com/ggarra13/mrv2/actions/workflows/full_linux_arm64.yml)
[![Build Status](https://github.com/ggarra13/mrv2/actions/workflows/full_macos13.yml/badge.svg)](https://github.com/ggarra13/mrv2/actions/workflows/full_macos13.yml)
[![Build Status](https://github.com/ggarra13/mrv2/actions/workflows/full_win64.yml/badge.svg)](https://github.com/ggarra13/mrv2/actions/workflows/full_win64.yml)
[![Build Status](https://github.com/ggarra13/mrv2/actions/workflows/full_macos14.yml/badge.svg)](https://github.com/ggarra13/mrv2/actions/workflows/full_macos14.yml)
[![Build Status](https://github.com/ggarra13/mrv2/actions/workflows/vulkan_linux.yml/badge.svg)](https://github.com/ggarra13/mrv2/actions/workflows/vulkan_linux.yml)
[![Build Status](https://github.com/ggarra13/mrv2/actions/workflows/vulkan_linux_arm64.yml/badge.svg)](https://github.com/ggarra13/mrv2/actions/workflows/vulkan_linux_arm64.yml)
[![Build Status](https://github.com/ggarra13/mrv2/actions/workflows/vulkan_macos13.yml/badge.svg)](https://github.com/ggarra13/mrv2/actions/workflows/vulkan_macos13.yml)
[![Build Status](https://github.com/ggarra13/mrv2/actions/workflows/vulkan_win64.yml/badge.svg)](https://github.com/ggarra13/mrv2/actions/workflows/vulkan_win64.yml)
[![Build Status](https://github.com/ggarra13/mrv2/actions/workflows/vulkan_macos14.yml/badge.svg)](https://github.com/ggarra13/mrv2/actions/workflows/vulkan_macos14.yml)

[![Donate](https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=UJMHRRKYCPXYW)
[![Sponsor](https://img.shields.io/badge/Sponsor-ggarra13-blue.svg)](https://github.com/sponsors/ggarra13)

![banner](https://github.com/ggarra13/mrv2/blob/main/assets/images/banner.png)

mrv2
====

mrv2 is an open source professional player and review tool for VFX, animation and computer graphics.

![banner](https://github.com/ggarra13/mrv2/blob/main/docs/www/files/Main_UI.png)

Contents:

- [Pre-built Binaries](#pre-built-binaries)
    - [Compatibility](#compatibility) 
    - [Notes on Installation](#notes-on-installation)
- [Features](#features)
- [Running mrv2](#running-mrv2)
    - [macOS and Linux](#macos-and-linux)
    - [Windows](#windows)
- [Tutorials](#tutorials)
- [Documenting](#documenting)

# Pre-built binaries

If you are looking for pre-built binaries for Windows, Linux or macOS, they can be found in:

[GitHub](https://github.com/ggarra13/mrv2/releases)

or in its mirror site at:

[SourceForge](https://sourceforge.net/projects/mrv2/files/)

The source forge site also hosts beta builds (nightly builds with the latest changes):

[SourceForge Betas](https://sourceforge.net/projects/mrv2/files/beta)

The Linux releases are built on Rocky Linux 8.10 using SCL.
The Linux beta builds are built on Ubuntu 24.04.  The preferred OS for
installation on Linux, however is one supporting GNOME 48+.

## Compatibility

mrv2 binaries run on Windows 10+, RedHat 10+ or Ubuntu 24.04+, and macOS 11.0+.

## Notes on installation

- On macOS you install it by opening the .dmg file, and dragging the mrv2
  icon to the Applications directory.  If there's already an mrv2 version,
  we recommend you overwrite it.
  The macOS application is currently not notarized, so when you launch it you
  will not be able to run it as macOS will warn you that the file is not secure
  as it was downloaded from internet.
  To avoid that, you need to open the Apple Logo->Settings->Privacy and Security
  and go to Security and allow "Opening Anyway".
  Alternatively, you can do it from the Terminal, by:
  
```
  sudo xattr -rd com.apple.quarantine /Applications/mrv2.app/
```

  If you are running macOS Tahoe 26.2 or similar, you may run into an
  OpenGL bug where subwindows "leak" outside the main window.
  To fix it you need to:
     System Settings->Accesibility->Motion and turn on Reduce Motion.

- Windows and Chrome, like macOS, also protect you from installing files
  from the Internet.  When you first download it with Chrome it may warn
  you that it is not an usual archive to be downloaded.  Make sure to click
  on the right up arrow menu to Save it anyway.
  You cannot open the .exe from Chrome directly.  You will need to open
  Windows Explorer and go to the Downloads directory.  You should then
  run it from there.
  Then Windows will popup a Blue box telling you Windows SmartScreen
  prevented the start of an unknown application and that you can place your
  PC at risk.
  Click on the More Information text and a Button that says Run anyway or
  similar should appear.  Click on it and follow the standard instructions
  to any Windows installer.
  One note about the Windows install.  When asked if you want to add mrv2 to
  your PATH, it is recommended to answer No to it, as it avoids DLLs conflicts
  with other applications that use common libraries like FFmpeg or OpenUSD.


- On Linux, in order to install the .rpm or .deb packages requires your user to
  have sudo permissions.

  On Debian (Ubuntu, etc) systems, you would install with:

```
  sudo dpkg -i mrv2-v1.0.0-Linux-amd64.deb
```

  On Red Hat (Rocky Linux, etc), you would install it with:
  
```
  sudo rpm -i mrv2-v1.0.0-Linux-amd64.rpm
```

  Once you install it, you can run mrv2 by just typing mrv2 in the shell, as
  a symlink to the executable is placed in /usr/bin.  The installers will also
  associate file extensions and install an icon for easy starting up in the
  Desktop icon of the user that installed it.  For running mrv2 with the icon,
  you need to select it and use the right mouse button to open the menu and
  choose Allow Launch.
  
  If you lack sudo permissions in your organization, you should download the
  .tar.gz file and you can uncompress it with:
  
```
  tar -xf mrv2-v1.0.0-Linux-amd64.tar.gz
```

  That will create a folder in the directory you uncompress it from.  You can
  then run mrv2 by using the mrv2.sh shell script in the bin/ subdirectory.

# Features

The source code is written in C++20 and uses CMake for the build system, with some bash scripts for auxiliary tasks.  
The core of the playback engine is a custom version of tlRender (www.github.com/darbyjohnston/tlRender.git).

Currently supported:

- Movie files (H264, MP4, VPX, WEBM, AV1, etc.)
- Image file sequences (Cineon, DPX, JPEG, OpenEXR, PNG, PPM, TIFF, TGA, BMP,
  	     	       	PSD)
- RAW Camera Formats (CR2, CR3, X3F, etc).
- Multi-channel audio
- Color management
- HDR support
- A/B comparison
- Native OpenTimelineIO with dissolves
- .otioz file bundles
- Creation of OpenTimelineIO playlists
- OpenEXR multichannel, multiview, YC, tiled and multipart support
- Environment mapping (Spherical and Cubic)
- Python3 API and Plugin system
- Network connections
- Stereo 3D (Anaglyph, Scanlines, Columns, Checkered, Side by Side)
- PDF Exporting of Annotations and Notes
- Linux Wayland support
- Internationalization (Translations) support
- Editing
- OpenGL and Vulkan backends
- Linux and Windows aarch64 (untested)

# Tutorials

Besides the basic API documentation included, there is a special channel on youtube.com where you can
find some tutorials on its basic use:

[Video Tutorials](https://www.youtube.com/watch?v=8JViz-pPCrg&list=PLxJ9NNBdNfRmd8AQ41AJYmb7WhN99G5C-)
