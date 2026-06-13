#!/usr/bin/env bash

. etc/functions.sh

get_kernel

if [[ "$CMAKE_INSTALL_PREFIX" == "" ]]; then
    echo "Please set the CMAKE_INSTALL_PREFIX environment variable"
    exit 1
fi

swig_version=v4.3.0

cd /tmp
git clone https://github.com/swig/swig.git
cd swig
git checkout ${swig_version}

if [[ $KERNEL == *Windows* ]]; then
    
# SWIG can be built using CMake and Visual Studio rather than autotools. As with the other approaches to building SWIG the dependencies need to be installed. The steps below are one of a number of ways of installing the dependencies without requiring Cygwin or MinGW. For fully working build steps always check the Continuous Integration (CI) setups currently detailed in the GitHub Actions YAML file.

# Install Nuget from https://www.nuget.org/downloads (v6.0.0 is used in this example, and installed to C:\Tools). Nuget is the package manager for .NET, but allows us to easily install CMake and other dependencies required by SWIG.
# Install CMake-win64 Nuget package using the following command:
# C:\Tools\nuget install CMake-win64 -Version 3.15.5 -OutputDirectory C:\Tools\CMake
# Using PowerShell the equivalent syntax is:
# & "C:\Tools\nuget" install CMake-win64 -Version 3.15.5 -OutputDirectory C:\Tools\CMake
# Alternatively you can download CMake from https://cmake.org/download/.
# Install the Bison Nuget package using the following command:
# C:\Tools\nuget install Bison -Version 3.7.4 -OutputDirectory C:\Tools\bison
# Alternatively download Bison from https://sourceforge.net/projects/winflexbison/files/ (Bison 3.7.4 is used in this example) and save to a folder e.g. C:\Tools\Bison
# Install the PCRE2 Nuget package using the following command:
# C:\Tools\nuget install PCRE2 -Version 10.39 -OutputDirectory C:\Tools\pcre2

# Note this is a x64 build, if this is not suitable PCRE2 can be built from source using https://github.com/PhilipHazel/pcre2/. Alternatively, set WITH_PCRE=OFF to disable PCRE2 support if you are sure you do not require it.

# We will also need the SWIG source code. Either download a zipped archive from GitHub, or if git is installed clone the latest codebase using:
# git clone https://github.com/swig/swig.git
# In this example we are assuming the source code is available at C:\swig

# Now we have all the required dependencies we can build SWIG using PowerShell and the commands below. We are assuming Visual Studio 2019 is installed. For other versions of Visual Studio change "Visual Studio 16 2019 -A x64" to the relevant Visual Studio Generator and architecture. We add the required build tools to the system PATH, and then build a Release version of SWIG. If all runs successfully a new swig.exe should be generated in the C:/swig/install2/bin folder.

# cd C:\swig

# $env:PATH="C:\Tools\CMake\CMake-win64.3.15.5\bin;C:\Tools\bison\Bison.3.7.4\bin;" + $env:PATH
# $PCRE_ROOT="C:\Tools\pcre2\PCRE2.10.39.0"
# $PCRE_PLATFORM="x64"

# cmake -G "Visual Studio 16 2019" -A "x64" `
# -DCMAKE_INSTALL_PREFIX="C:/swig/install2" `
# -DCMAKE_C_FLAGS="/DPCRE2_STATIC" `
# -DCMAKE_CXX_FLAGS="/DPCRE2_STATIC" `
# -DPCRE2_INCLUDE_DIR="$PCRE_ROOT/include" `
# -DPCRE2_LIBRARY="$PCRE_ROOT/lib/pcre2-8-static.lib" `
# -S . -B build

# cmake --build build --config Release
# cmake --install build --config Release

# # to test the exe built correctly
# cd install2/bin
# ./swig.exe -version
# ./swig.exe -help
# In addition to Release builds you can create a Debug build using:

# cmake --build build --config Debug
# A Visual Studio solution file should be generated named swig.sln. This can be opened and debugged by running the swig project and setting Properties > Debugging > Command Arguments. For example to debug one of the test-suite .i files included with the SWIG source use the following:

# -python -c++ -o C:\Temp\doxygen_parsing.cpp C:\swig\Examples\test-suite\doxygen_parsing.i

    echo "Currently you cannot build swig from source on Windows"
    exit 0
fi

if [[ $KERNEL == *Linux* ]]; then
    sudo apt install build-essential libpcre2-dev libpcre3-dev autoconf automake libtool bison git libboost-dev golang-go guile-2.2-dev nodejs node-gyp libwebkit2gtk-4.1-dev lua5.3 liblua5.3-dev mono-devel octave-dev default-jdk-headless php-cli php-dev python3-dev r-base ruby ruby-dev tcl-dev scilab libxml2-dev
fi

if [[ $KERNEL == *Darwin* ]]; then
    brew install pcre2
fi

#
# Build it
#
./configure
make
make install --prefix=${CMAKE_INSTALL_PREFIX}
