
Contents:

- [Building](#building)
    - [Dependencies](#dependencies)
      - [RedHat](#redhat)
      - [Ubuntu](#ubuntu)
      - [macOS](#macos)
      - [Windows](#windows)
    - [Building mrv2](#building-mrv2)
    - [Debug builds](#debug-builds)
    - [Building on Windows](#building-on-windows)
    - [CMake build options](#cmake-build-options) 
    - [Building FFmpeg as GPL or LGPL](#building-ffmpeg-as-gpl-or-lgpl)


# Building

## Dependencies

### RedHat

```
#
# Repositories
#
sudo dnf -y install dnf-plugins-core
sudo dnf -y install epel-release
sudo dnf config-manager --set-enabled powertools

#
# Update dnf database
#
sudo dnf makecache --refresh

#
# Install bundles
#
sudo dnf -y groupinstall "Development Tools"
sudo dnf -y install perl perl-CPAN

# Install IPC::Cmd non-interactively
sudo cpan App::cpanminus && cpanm --notest IPC::Cmd

#
# Install dependencies
#
sudo dnf -y install git wget curl cmake pango-devel gettext ninja-build \
	       libglvnd-devel alsa-lib-devel pulseaudio-libs-devel \
	       dpkg \
	       autoconf wayland-devel wayland-protocols-devel cairo-devel \
	       libxkbcommon-devel dbus-devel mesa-libGLU-devel gtk3-devel \
	       libffi-devel openssl-devel tk-devel tcl-devel libXt-devel \
	       swig

sudo dnf install centos-release-scl

sudo dnf install gcc-toolset-13

scl enable gcc-toolset-13 bash

```

### Ubuntu

```
#
# Update apt database
#
sudo apt update

#
# Install dependencies
#
sudo apt -y install curl build-essential perl git cmake ninja-build \
                    libpango1.0-dev libglu1-mesa-dev \
		    xorg-dev libx11-dev libxcursor-dev libxinerama-dev \
		    gettext libasound2-dev \
		    libpulse-dev libssl-dev libffi-dev \
		    libwayland-dev wayland-protocols libdbus-1-dev \
		    libxkbcommon-dev libegl-dev libgtk-3-dev rpm \
                    doxygen tk-dev libxt-dev swig

# Install cpanminus and IPC::Cmd non-interactively
sudo cpan App::cpanminus && cpanm --notest IPC::Cmd

# For Vulkan builds
sudo apt -y install libvulkan-dev glslang-dev libshaderc-dev


```

### macOS

```
#
# Install
#
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

#
# Install development tools
#
xcode-select --install

#
# Install dependencies
#
brew install git gnu-sed swig python cmake ninja gettext openssl readline sqlite3 xz zlib

```

### Windows

- [Visual Studio 2022 or later Community/Enterprise](https://visualstudio.microsoft.com/en/free-developer-offers) (with clang compiler)
- [MSYS2](https://www.msys2.org/)
- [Git](https://git-scm.com/downloads)
- [CMake 3.26.2 or later](https://cmake.org/download/)
- [Python 3.10 or later](https://www.python.org/downloads/)
- [NSIS Installer for Packaging](https://nsis.sourceforge.io/Download) - Optional

Additional dependencies are downloaded and built automatically by the CMake
superbuild script.  For a list of non-system libraries that mrv2 depends on
and their licenses, please refer to src/docs/Legal.

The only special requirement is installing a new copy of cmake than the
one that ships with MSVC2022.
If building the NSIS installer, you need to place the root of mrv2 in a path
that has less than 20 characters, like:

```
     /D/code/applications
```



## Building mrv2

Clone the repository:
```
cd some_dir

#
# Clone the latest cutting-edge build (might be unstable).
#
git clone https://github.com/ggarra13/mrv2.git

cd mrv2
./runme.sh
```

The script is a superbuild script that will download all needed dependencies
required.  It will create a build and a:
```
BUILD-KERNEL-ARCH/BUILDTYPE/install
````
directory where all files shall reside.  

Make sure you meet the basic dependencies for your platform.  See [Dependencies](#dependencies).

The runme.sh script will output its progress to the terminal and also save it
in:
````
BUILD-KERNEL-ARCH/BUILDTYPE/compile.log.
````
The default is to build with all cores in all the Operating Systems.
Currently, the build with all settings on takes about 39 minutes on 16 cores.

If you want more or less cores pass another number to any of
the runme*.sh scripts.  For example, to build with 4 cores, you can do:

```
./runme.sh -j 4
```


Later, if you just want to build mrv2 quickly (runme quick mnemonic)
without running through all the dependencies, run:

```
./runmeq.sh
```

Later, to just build FLTK, tlRender and mrv2  (runme three mnemonic), run;

```
./runmet.sh
```

Also, look in the bin/ directory for other runme.sh scripts which compile  a quicker version of mrv2 without features like USD, python or networking.

## Debug builds


All runme.sh scripts support two additional parameters.
For a debug build, you would do:

```
./runme.sh debug
```

To clean up the directory, run a debug build with 8 cores, run:

```
./runme.sh clean debug -j 8
```



## Building on Windows

For windows, in addition to Visual Studio, you will need a new and
fresh copy of Msys.
There is a .bat file included in the distribution (in helpers/windows/bat),
which needs to be modified to the path of Visual Studio (2019 by default),
the optional Windows SDK (none by default) and your copy of Msys.
You run the .bat file first, which will set the Visual Studio paths and 
fire up a Msys console.  From then on, all commands described are run in 
the Msys console.

FFmpeg and liblcms2 are now compiled as part of the pre-flight cmake build.  libssh and libcrypto are taken from Msys64 repositories when building FFmpeg as well as swig.

The libintl and libiconv libraries are taken from the MSys64 repositories as pre-flight check with the bin/install_libintl_window.sh script (part of runme.sh).

## CMake build options

The main runme.sh script supports passing CMake flags to it and allows turning on or off some options of mrv2.  You must pass them like:

-D TLRENDER_USD=OFF

The flags are listed when you start the runme.sh script.  If you want to make some change to the flags permanent, you should change them
in runme_nolog.sh or create a wrapper script that calls runme.sh.


## Building FFmpeg as GPL or LGPL

If you pass -gpl or -lpgl to the runme.sh script, like:

```
./runme.sh -gpl
```

The build system will compile FFmpeg as GPL or LGPL on all platforms.  The default is to build a LGPL version of FFmpeg as that complies with the BSD binary distribution license.  The LGPL version of FFmpeg, however, does not come with libx264, which means you cannot save movie files with the H264 codec on Windows and Linux.  It also does not support GoPro Cineform files with alpha.

The GPL version of FFmpeg does not have that restriction and it will compile libx264 on all platforms and work GoPro Cineform with alpha.

