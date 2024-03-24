#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
#
# Wrap up script to compile just mrv2, do the documenting and packaging.
#
#


if [[ ! $RUNME ]]; then
    . etc/build_dir.sh
else
    . etc/functions.sh
fi


dir=$BUILD_DIR/mrv2/src/mrv2-build

if [[ ! -d $dir ]]; then
    echo "mrv2 build directory does not exist."
    echo "Please run:"
    echo " $ runme.sh [sameflags]"
    exit 1
fi

if [[ "$CMAKE_TARGET" == "package" ]]; then  
    # Needed to to force a relink and update build info.
    touch mrv2/lib/mrvWidgets/mrvVersion.cpp
fi

if [[ "$CMAKE_TARGET" == "" ]]; then
    CMAKE_TARGET=install
fi

if [[ $CMAKE_TARGET == doc* || $CMAKE_TARGET == "package" ]]; then
    #
    # First, generate the translations and install them
    #
    cd $dir
    run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t mo
    run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install
    cd -
fi

if [[ $CMAKE_TARGET == doc* ]]; then
    
    . ./etc/sphinx_install.sh # Install Sphinx python modules

    #
    # Second, generate the documentation and install them
    #
    cd $dir
    run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t doc
    export CMAKE_TARGET=install
    cd -
fi


cd $dir

run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t ${CMAKE_TARGET}

cd -

. etc/build_end.sh
