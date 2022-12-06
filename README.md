[![License](https://img.shields.io/badge/License-BSD%203--Clause-blue.svg)](https://opensource.org/licenses/BSD-3-Clause)

mrv2
====

mrViewer 2 ( aka mrv2 ) is an open source playback and review tool for
visual effects, film and animation.

Contents:
* [Libraries](#libraries)
* [Building](#building)
    * [Dependencies](#dependencies)
    * [Building mrv2](#building-mrv2)
    * [Debug builds](#debug-builds)
    * [Building on Windows](#building-on-windows)
    * [Building FFmpeg on Windows](#building-ffmpeg-on-windows)
* [Running mrv2](#running-mrv2)
    * [macOS and Linux](#macos-and-linux)
    * [Windows](#windows)

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

## Building mrv2

Download and install cmake from either your OS repository or from the
web page above.  Note that on Windows you don't need to download it as
it comes with Visual Studio.

Install Ninja
```
sudo apt install ninja-build  # Ubuntu
dnf install ninja-build       # RedHat / CentOS
brew install ninja            # macOS
# Install Visual Studio       # Windows (comes bundled)
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


Later, if you just want to build mrViewer quickly without running
through all the dependencies, run:

```
./runmeq.sh
```

Later, to just build FLTK, tlRender and mrViewer, run;

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
the optional Windows SDK (8.1 by default) and your copy of Msys.
Also once you start Msys with the bat files, the first thing you need
to do is:

```
mv /usr/bin/link.exe /usr/bin/link_msys.exe
```

as there's a conflict in the link.exe executable that ships with Visual
Studio and the link.exe utility that ships with Msys.


## Building FFmpeg on Windows

The windows compilation does not compile the ffmpeg, libintl nor
the libiconv libraries.  These have to be compiled manually using
gcc or clang instead of Visual Studio's CL.exe.

As a convernience for Windows users, DLLs, includes and .lib files
for those libraries are provided in mrViewer's windows/win32
and windows/win64 directories.

libintl and libiconv are not updated often.

However ffmpeg is, so it is suggested you learn how to compile it.
A very good way is to use the media autobuild suite, which runs on a
separate and fresh MSys (yes, you need to keep two Msys copies) and
downloads and compiles all dependencies based on simple questions.

As a bonus, you can replace the LGPL ffmpeg that ships with mrv2 with a more
complete GPL one.

The media autobuild suite can be obtained with:

```
cd your_favorite_directory
git clone https://github.com/m-ab-s/media-autobuild_suite m-ab-s
```

Then from Windows explorer (not from Msys as it won't run .bat files)
run;

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

## Windows

On Windows, we cannot create symbolic links, so in Mdys you need to type the whole path to the install.  That is, for example:


```
BUILD-Msys-64/Release/install/bin/mrViewer.exe
```

If you like to work command line, you should add the whole path to the mrViewer.exe ro your path in the System->Advanced Settings->Environment Variables.
For working with a GUI, after the build is done, you should do:

```
explorer BUILD-Msys-64/Release/install/bin/
```

And in the explorer directory that it will open, you should create a shortcut with the RMB to the mrViewer.exe.  Once that is done, you can drag and rename the shortcut to your Desktop to have it handy.
