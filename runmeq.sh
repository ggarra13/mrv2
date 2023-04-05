#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.



. $PWD/etc/build_dir.sh


dir=$BUILD_DIR/mrv2/src/mrv2-build

if [[ ! -d $dir ]]; then
    echo "mrv2 build directory does not exist."
    echo "Please run:"
    echo " $ runme.sh [sameflags]"
    exit 1
fi

if [[ "$CMAKE_TARGET" == "" ]]; then
    CMAKE_TARGET=install
fi

#
# This is needed on macOS which spits out linking errors otherwise, like:
#
# error: install_name_tool: no LC_RPATH load command with path: install/lib found in: install/bin/mrv2 (for architecture x86_64), required for specified option "-delete_rpath install/lib"
if [[ $KERNEL == *Darwin* && $CMAKE_TARGET == "install" ]]; then
    rm -f $BUILD_DIR/install/bin/mrv2
fi


if [[ $CMAKE_TARGET == doc* || $CMAKE_TARGET == "install" ||
	  $CMAKE_TARGET == "package" ]]; then
    #
    # First, generate the translations and install them
    #
    cd $dir
    cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t mo
    cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install
    cd -
fi

if [[ $CMAKE_TARGET == doc* ]]; then
    
    . ./etc/sphinx_install.sh # Install Sphinx python modules

    #
    # Second, generate the documentation and install them
    #
    cd $dir
    cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t doc
    cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install
    cd -
fi

cd $dir

cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t ${CMAKE_TARGET}

cd -

. $PWD/etc/build_end.sh
