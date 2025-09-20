
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
- [Packaging](#packaging)
- [Translating](#translating)
   - [If you compiled mrv2](#if-you-compiled-mrv2)
   - [If you did not compile mrv2](#if-you-did-not-compile-mrv2)
   - [Translating with AI](#translating-with-ai)
   - [Translating on Windows](#translating-on-windows)
- [Developing](#developing)


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

# Running mrv2

## macOS and Linux

If you have a bin directory in your $HOME (ie. ~/bin ), the build scripts will
create a symlink there.  So you should add ~/bin to your PATH in your .bashrc
or .zshrc.

Assuming you complied mrv2 with the ~/bin directory already created, then to
start mrv2 then you'd do:

```
export PATH=~/bin:$PATH  # no need if you add this line to your .bashrc
mrv2
```

and to run the debug build.

```
export PATH=~/bin:$PATH  # no need if you add this line to your .bashrc
mrv2-dbg
```

If you compiled mrv2 without bin directory in your HOME directory, you can
start it from the BUILD directory with the mrv2.sh script, like:

```
BUILD-Linux-amd64/Release/install/bin/mrv2.sh
```

## Windows

On Windows, we cannot create symbolic links, so in Msys you need to type the whole path to the install.  That is, for example:


```
BUILD-Msys-amd64/Release/install/bin/mrv2.exe
```

If you like to work command line, you should add the whole path to the mrv2.exe to your path.  In Msys, you can add it to the .bashrc like shown on macOS and Linux.

For cmd.exe or PowerShell, on the Windows taskbar, right-click the Windows icon and select System. In the Settings window, under Related Settings, click Advanced system settings. On the Advanced tab, click Environment Variables. Find the PATH environment variable and add the full path to mrv2.exe.

For working with a GUI, after the build is done, you should do:

```
cd BUILD-Msys-amd64/Release/install/bin/  # or similar
explorer .
```

And in the explorer directory that it will open, you should create a shortcut
with the RMB to the mrv2.exe.  Once that is done, you can drag and rename the
shortcut to your Desktop to have it handy.
Note that if you will not be developing mrv2, you should instead proceed to [Packaging](#packaging).


# Packaging

Once you build mrv2 and tested that it runs, you might want to create a package
for distribution.  On macOS, this is a .dmg file.  On Linux it is a RPM, DEB or
TGZ file.  On Windows it is a ZIP or an NSIS EXE installer.

To do so, from the main dir of mrv2, you have to do:

```
./runmeq.sh -t package
```
For all architectures, the installers will be stored in:

```
packages/
```

That is the root directory of mrv2.

# Documenting

Currently, the documentation is generated automatically from the translations.
To do so, you must run:

```
./runmeq.sh -t doc
```

# Translating

mrv2 can support multiple natural language translations.  Currently, 
Chinese, English, French, German, Hindi, Italian, Portuguese, Russian and
Spanish are supported.  The translation system used is gettext so
familiarity with it is desired (albeit not essential).
The translations reside in src/po and follow internationalization language
code files, like es.po (for Spanish) or de.po (for German).

First, you should create a branch to submit a pull request.

There are GitHub docs, e.g.
https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/creating-a-pull-request

In a nutshell:

- fork the mrv2 repo to e.g. your-username/mrv2
- create a feature branch in you fork (important although not required [1])
- push one or more commits to your feature branch
- request a PR on your repo/feature-branch

To create such a file for a new language, open the file
cmake/translations.cmake
and add a language international code to this line:

```
set( LANGUAGES es ) # add a new language code inside the parenthesis, like "de".
```

Then, run:

```
./runmeq.sh -t po
```

If there's no .po file for that language yet, gettext's msginit command
will be run for you.  You may be asked for your email address as part of
the process.

Go to mrv2/po/{lang}.po where lang is the language you added.

and edit the text.  Make sure to change the charset to UTF-8.

Note that you should use an editor that can write in Unicode (UTF-8) to
write non-Occidental languages.

You need to edit "msgstr" strings and leave "msgid" untouched as a reference.
If the comment has a "fuzzy" string it means gettext tried to guess the
translation, but it will not use it.  Remove the fuzzy qualifier and change the
"msgstr" string.  Note that if the "msgid" has new-lines you need to match them
too.  Refer to the gettext manual for further information.

Once you are ready to test your translation, run:

```
./runmeq.sh -t mo
```

That will create the .mo files for your language. 

## If you compiled mrv2

To test the translation, you can just run:

```
./runmeq.sh -t install
```

or just:

```
./runmeq.sh
```

and that will place the .mo files in the:
$BUILD-$OS-$ARCH/$BUILD_TYPE/install/share/locale directory.

If you add or remove strings as part of your code changes, you may want to
regenerate the .pot files after a while, before calling -t mo.  To do so:

```
./runmeq.sh -t pot
```

Note that this change is dramatic as your commits of the code changes will
get mangled with all the .pot/.po comments, preventing a clean PR
(Pull Request) on github.com.

## Translating with AI

There's an AI script that uses the transformers python library for AI
translation in:

	bin/po/po_translate.sh
	
It works by sending a language to it, like:

	bin/po/po_translate.sh all     # for all languages supported by the script.
	bin/po/po_translate.sh zh-CN   # for Chinese Simplified

## If you did not compile mrv2

Create a pull request on GitHub:

Then, create a new .po file for your the main translations.  For example:

```
cp mrv2/po/en.po mrv2/po/it.po   # For Italian
````

Then add the file for a new language, open the file
cmake/translations.cmake
and add a language international code to this line:

```
set( LANGUAGES en es it ) # add a new language code inside the parenthesis, like "it".
```

Translate that new .po file manually and then do:

```
git add mrv2/po/it.po
git commit
git push
```

submit a GitHub PR with that new file.  The mrv2 developers will try to merge your changes later.


## Translating on Windows

On Windows, besides the text of mrv2, you also need to translate the text for
the NSIS .exe installer.

You can do it by editing the cmake/nsis/mrv2_translations.nsh file.
Just follow the examples in that file.

# Developing

If you want to become a developer, first familiarize yourself with the build process.  Then clone the repository to your github account and send PRs.  If you become an avid developer, you can then request access to the main repository.

One additional thing that you will need for making commits to the repository, is:

```
clang-format
```

This is part of the LLVM project, you can download it from your usual repositories (apt, brew, etc.), or from:

[LLVM Main Download Page](https://releases.llvm.org/download.html)

This utility verifies previous to a commit that all the C++ formatting follows the standard used in mrv2.


You might also want to get Doxygen so as to get the source code documentation in docs/Doxygen.
