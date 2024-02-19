#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



#
# Turn on exit on error
#
set -o pipefail -e

#
# Get the auxiliary functions
#
. etc/functions.sh

#
# Extract mrv2 version from cmake/version.cmake
#
extract_version

#
# Extract python version
#
extract_python_version

#
# Extract cmake version
#
get_cmake_version

#
# Determine OS Kernel, OS CPU architecture
#
. etc/parse_args.sh

#
# Build a build directory with that information
#
export BUILD_DIR=$BUILD_ROOT/${CMAKE_BUILD_TYPE}

#
# Clean the directory if we were asked to.
#
if [[ $CLEAN_DIR == 1 && $RUNME == 1 ]]; then
    if [[ -d ${BUILD_DIR} ]]; then
	echo "Cleaning ${BUILD_DIR}.  Please wait..."
	run_cmd rm -rf $BUILD_DIR
    fi
fi

#
# Get the number of CPU cores for maximum efficiency
#
. etc/build_cores.sh

#
# Extract compiler version
#
get_compiler_version

#
# For Darwin, when building amd64, we make it compatible with macOS 11.0
#
export OSX_TARGET=11.0


if [[ $KERNEL == *Darwin* ]]; then
    export PATH="/usr/local/opt/gnu-sed/libexec/gnubin:${PATH}"
    if [[ $ARCH == arm64 ]]; then
	export CMAKE_OSX_ARCHITECTURES=$ARCH
    else
	export CMAKE_FLAGS="-DCMAKE_OSX_DEPLOYMENT_TARGET=${OSX_TARGET} ${CMAKE_FLAGS}"
    fi
fi

if [[ $FLAGS == "" ]]; then
    export FLAGS="-j ${CPU_CORES}"
fi
export FLAGS="${FLAGS} $*"

#
# Store old LD_LIBRARY_PATH
#
if [ -z "$OLD_LD_LIBRARY_PATH" ]; then
    export OLD_LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/usr/lib/x86_64-linux-gnu/:/lib/x86_64-linux-gnu/:/usr/lib64/:/usr/lib:/lib64:/lib:

fi

if [ -z "$OLD_DYLD_LIBRARY_PATH" ]; then
    # On macOS, sqlite which is used by subversion does not get linked into
    # /usr/local to not shadow the one in /usr/lib
    export OLD_DYLD_LIBRARY_PATH="/usr/local/opt/sqlite/lib:${DYLD_LIBRARY_PATH}"
fi

#
# Set environment variables to point to install directory
#

export PATH="$PWD/${BUILD_DIR}/install/bin:$PWD/$BUILD_DIR/install/bin/Scripts:${PATH}"

#
# We set both lib64 and lib to handle differences in Linux RH and Ubuntu
#
export LD_LIBRARY_PATH="$PWD/${BUILD_DIR}/install/lib64:$PWD/${BUILD_DIR}/install/lib:${LD_LIBRARY_PATH}"

export DYLD_LIBRARY_PATH="$PWD/${BUILD_DIR}/install/lib:${DYLD_LIBRARY_PATH}"

#
# We set both lib64 and lib to handle differences in Linux RH and Ubuntu
#
export PKG_CONFIG_PATH="$PWD/${BUILD_DIR}/install/lib64/pkgconfig:$PWD/${BUILD_DIR}/install/lib/pkgconfig:${PKG_CONFIG_PATH}"

##########
# Python #
##########

#
# Set the name of python executable we build
#
if [[ $KERNEL == *Msys* ]]; then
    export PYTHONEXE="$PWD/${BUILD_DIR}/install/bin/python.exe"
else
    export PYTHONEXE="$PWD/${BUILD_DIR}/install/bin/python${PYTHON_VERSION}"
fi

#
# Set PYTHONPATH
#
export PYTHONPATH="$PWD/${BUILD_DIR}/install/lib/python${PYTHON_VERSION}:$PWD/${BUILD_DIR}/install/lib/python${PYTHON_VERSION}/site-packages:${PYTHONPATH}"
