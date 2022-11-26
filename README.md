#
# Instructions to build mrViewer 2 ( mrv2 )
#

To build it with 4 cores (the default), run:

$ runme.sh -j 4

It will create a build and a BUILD-KERNEL-ARCH/BUILDTYPE/install directory
where çall files shall reside.

Later, if you just want to build mrViewer without running through all the
dependencies, run:

$ runmeq.sh

Later, to just build FLTK, tlRender and mrViewer, run;

$ runmet.sh


WINDOWS NOTE
============

For windows, in addition to Visual Studio, you will need a new copy of Msys which cannot be the one of media-autobuild-suite.  There are two .bat files included in the distribution (in windows/bat), which need to be arranged to the path of Visual Studio and your copy of Msys.  Also if you fire that Msys with the bat files, the first thing you need to do is:

$ mv /usr/bin/link.exe /usr/bin/link_msys.exe


The windows compilation does not compile the ffmpeg, libintl nor the libiconv
libraries.  These have to be compiled manually using gcc or clang instead of Visual Studio's CL.exe.

As a convernience for Windows users, DLLs, includes and .lib files for those
libraries are provided in mrViewer's windows/win32 and windows/win64
directories.

libint and libicon do not update often.

However ffmpeg does, so it is suggested you learn how to compile it.
A very good way is to use the media autobuild suite, which runs undexr MSys.

It can be obtained from:

https://github.com/m-ab-s/media-autobuild_suite

As a bonus, you can replace the LGPL ffmpeg that ships with mrv2 with a more
complete GPL one.
