[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

mrv2
====

mrv2 is an open source playback and review tool for visual effects, film and animation.

Contents:
* [Pre-built Binaries](#pre-built-binaries)
* [Building](#building)
    * [Building on Linux with a Dockerfile](#building-on-linux-with-a-dockerfile)
    * [Building mrv2](#building-mrv2)
    * [Debug builds](#debug-builds)
    * [Building on Windows](#building-on-windows)
    * [Building FFmpeg on Windows](#building-ffmpeg-on-windows)
* [Running mrv2](#running-mrv2)
    * [macOS and Linux](#macos-and-linux)
    * [Windows](#windows)
* [Packaging](#packaging)

# Pre-built binaries

If you are looking for pre-built binaries for Windows, Linux or macOS (Intel), they can be found in:

https://sourceforge.net/projects/mrv2/files/

# Building

## Dependencies

Required dependencies:
* [CMake] (https://cmake.org/download/)
* [ninja-build] (https://github.com/ninja-build/ninja.git)
* [tlRender] (https://github.com/ggarra13/tlRender)
* [FLTK 1.4] (https://github.com/fltk/fltk)
* [Imath] (https://github.com/AcademySoftwareFoundation/Imath.git)
* [Boost] (https://www.boost.org/)
* [gettext/libintl] (https://savannah.gnu.org/projects/gettext/)
		    (https://github.com/mlocati/gettext-iconv-windows)
* [libiconv] (https://savannah.gnu.org/projects/libiconv/)
	     (https://github.com/mlocati/gettext-iconv-windows)

Optional dependencies:
* [Pango (Linux only)] (https://github.com/GNOME/pango)

## Building mrv2

If you have docker installed with your user as part of the docker group,
you can just build mrv2 with:
```
docker.sh
```


Download and install cmake from either your OS repository or from the
web page above.  Note that on Windows you don't need to download it as
it comes with Visual Studio.

Install Ninja
```
sudo apt install ninja-build  # Ubuntu

dnf install epel-release      # RedHat / CentOS
dnf install ninja-build

brew install ninja            # macOS

# Install Visual Studio       # Windows (comes bundled)
```

Install Pango (Linux only) for foreign fonts (Japanese, Chinese, etc):

```
sudo apt install libpango1.0-dev  # Ubuntu
dnf install pango-devel           # RedHat / CentOS
```


Install the X11 / X.org development packages (Linux only):
```
sudo apt install xorg-dev libx11-dev libxcursor-dev libxinerama-dev  # Ubuntu
dnf group install "X Software Development"                   # RedHat / CentOS
```

Install gettext (libintl / iconv)
```
apt install gettext   # Ubuntu
brew install gettext  # macOS
		      # Windows (bundled with mrv2)
```


Clone the repository:
```
cd some_dir
git clone https://github.com/ggarra13/mrv2.git
cd mrv2
./runme.sh
```

The script is a superbuild script that will download all needed dependencies
required.  It will create a build and a BUILD-KERNEL-ARCH/BUILDTYPE/install
directory where all files shall reside.

The default is to build it with 4 cores, but if you want more cores
pass another number to any of the runme*.sh scripts.  For example, to build
with 8 cores, you can do:

```
./runme.sh -j 8
```


Later, if you just want to build mrv2 quickly without running
through all the dependencies, run:

```
./runmeq.sh
```

Later, to just build FLTK, tlRender and mrv2, run;

```
./runmet.sh
```

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
There are two .bat files included in the distribution (in windows/bat),
which need to be arranged to the path of Visual Studio (2019 by default),
the optional Windows SDK (none by default) and your copy of Msys.


## Building FFmpeg on Windows

The windows compilation does not compile the ffmpeg, libintl nor
the libiconv libraries.  These have to be compiled manually using
gcc or clang instead of Visual Studio's CL.exe.

As a convernience for Windows users, DLLs, includes and .lib files
for those libraries are provided in mrv2's windows/win32
and windows/win64 directories.

libintl and libiconv are not updated often.

However ffmpeg is, so it is suggested you learn how to compile it.
A very good way is to use the media autobuild suite, which runs on a
separate and fresh MSys (yes, you need to keep two Msys copies) and
downloads and compiles all dependencies based on simple questions.

As a bonus, you can replace the LGPL ffmpeg DLL that ships with mrv2 with a
more complete GPL one.

The media autobuild suite can be obtained with:

```
cd your_favorite_directory
git clone https://github.com/m-ab-s/media-autobuild_suite m-ab-s
```

Then from Windows explorer (not from Msys as it won't run .bat files)
run:

media-autobuild_suite.bat

# Running mrv2

## macOS and Linux

If you have a bin directory in your $HOME (ie. ~/bin ), the build scripts will create a symlink there.  So you should add ~/bin to your PATH in your .bashrc or .zahrc.

Assuming you complied mrv2 with the ~/bin directory already created, then to
start mrv2 then you'd do:

```
export PATH=~/bin:$PATH  # not need if you add this line to your .bashrc
mrv2
```

and to run the debug build.

```
export PATH=~/bin:$PATH  # not need if you add this line to your .bashrc
mrv2-dbg
```

If you compiled mrv2 without bin directory in your HOME directory, you can start it from the BUILD directory with the mrv2.sh script, like:

```
BUILD-Linux-64/Release/install/bin/mrv2.sh
```

## Windows

On Windows, we cannot create symbolic links, so in Msys you need to type the whole path to the install.  That is, for example:


```
BUILD-Msys-64/Release/install/bin/mrv2.exe
```

If you like to work command line, you should add the whole path to the mrv2.exe to your path.  In Msys, you can add it to the .bashrc like shown on macOS and Linux.

For cmd.exe or PowerShell, on the Windows taskbar, right-click the Windows icon and select System. In the Settings window, under Related Settings, click Advanced system settings. On the Advanced tab, click Environment Variables. Find the PATH environment variable and add the full path to mrv2.exe.

For working with a GUI, after the build is done, you should do:

```
explorer BUILD-Msys-64/Release/install/bin/
```

And in the explorer directory that it will open, you should create a shortcut with the RMB to the mrv2.exe.  Once that is done, you can drag and rename the shortcut to your Desktop to have it handy.

# Packaging

Once you build mrv2 and tested that it runs, you might want to create a package for distribution.  On macOS, this is a .dmg file.  On Linux it is a RPM, DEB or TGZ file.  On Windows it is a ZIP or EXE installer.

To do so, from the main dir of mrv2, you have to do:

```
runmeq.sh -t package
```

The resulting files will be placed in:

BUILD-$KERNEL-$ARCH/Release/mrv2/src/mrv2-build
