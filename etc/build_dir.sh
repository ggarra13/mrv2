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
# Determine OS Kernel, OS CPU architecture
#
. etc/parse_args.sh


#
# Build a build directory with that information
#
if [[ -z $BUILD_ROOT ]]; then
    export BUILD_DIR=$default_build_root/${BUILD_TYPE_DIR}
else
    export BUILD_DIR=$BUILD_ROOT/${BUILD_TYPE_DIR}
fi

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
# Recreate the build directory
#
if [[ ! -d $BUILD_DIR ]]; then
    run_cmd mkdir -p $BUILD_DIR
fi


#
# Get the number of CPU cores for maximum efficiency
#
. etc/build_cores.sh

#
# For Darwin, when building amd64, we make it compatible with macOS 11.0
#
export CMAKE_OSX_DEPLOYMENT_TARGET=11.0

if [[ $KERNEL == *Darwin* ]]; then
    export PATH="/usr/local/opt/gnu-sed/libexec/gnubin:${PATH}"
    if [[ $ARCH == arm64 || $ARCH == aarch64 ]]; then
	export CMAKE_OSX_ARCHITECTURES=$ARCH
	export CMAKE_OSX_DEPLOYMENT_TARGET=11.3
    else
	export CMAKE_OSX_ARCHITECTURES="x86_64"
    fi
fi

if [[ $FLAGS == "" ]]; then
    export FLAGS="-j ${CPU_CORES}"
fi
export FLAGS="${FLAGS} $*"

#
# Store old LD_LIBRARY_PATH
#
if [ -z "$OLD_PATH" ]; then
    export OLD_PATH="$PATH"
fi


#
# We set both lib64 and lib to handle differences in Linux RH and Ubuntu
#
export LD_LIBRARY_PATH="$PWD/${BUILD_DIR}/install/lib64:$PWD/${BUILD_DIR}/install/lib:${LD_LIBRARY_PATH}"
export DYLD_LIBRARY_PATH="$PWD/${BUILD_DIR}/install/lib:${DYLD_LIBRARY_PATH}"
export DYLD_FALLBACK_LIBRARY_PATH="$PWD/${BUILD_DIR}/install/lib:${DYLD_FALLBACK_LIBRARY_PATH}"

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
if [[ ! $RUNME || ${BUILD_PYTHON} == OFF || ${BUILD_PYTHON} == 0 ]]; then
    locate_python
    #
    # Set PYTHONPATH
    #
    export PYTHONPATH="${PYTHON_LIBDIR}:${PYTHON_SITEDIR}:${PYTHONPATH}"
else
    if [[ $KERNEL == *Windows* ]]; then
	export PYTHONEXE="$PWD/${BUILD_DIR}/install/bin/python.exe"
    else
	export PYTHONEXE="$PWD/${BUILD_DIR}/install/bin/python${PYTHON_VERSION}"
    fi
    #
    # Set PYTHONPATH
    #
    export PYTHONPATH="$PWD/${BUILD_DIR}/install/lib/python${PYTHON_VERSION}:$PWD/${BUILD_DIR}/install/lib/python${PYTHON_VERSION}/site-packages:${PYTHONPATH}"
fi
 
