#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
#
# Wrap up script to compile just mrv2, do the documenting and packaging.
#
#

set +e

if [[ ! $RUNME ]]; then
    . etc/build_dir.sh
else
    . etc/functions.sh
fi

echo "BUILD_DIR=${BUILD_DIR}"

dir=$BUILD_DIR/mrv2/src/mrv2-build

if [[ ! -d $dir ]]; then
    echo "mrv2 build directory:"
    echo $dir
    echo "does not exist."
    echo "Please run:"
    echo " $ runme.sh [sameflags]"
    exit 1
fi

if [[ "$CMAKE_TARGET" == "package" ]]; then  
    # Needed to to force a relink and update build info.
    touch src/lib/mrvWidgets/mrvVersion.cpp
fi

if [[ "$CMAKE_TARGET" == "" ]]; then
    CMAKE_TARGET=install
fi


if [[ "$CMAKE_TARGET" == doc* ]]; then
    
    . ./etc/sphinx_install.sh # Install Sphinx python modules
    
    cd $dir

    run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t ${CMAKE_TARGET}
    export CMAKE_TARGET=install

    cd -
fi


if [[ "$CMAKE_TARGET" == "mo" ]]; then
    
    cd $dir

    run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE
    run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t mo

    sleep 5
    export CMAKE_TARGET=install

    cd -
fi

if [[ "$CMAKE_TARGET" == "package" ]]; then
    
    clean_mo_files
    
    cd $dir
	
    run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t pot
    run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t mo
    run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install

    cd -
fi

cd $dir

if [[ $CMAKE_TARGET == "package" && $KERNEL == *Darwin* ]]; then
    echo "Packaging with a single thread..."
    sync
    cmake --build . -j1 --config $CMAKE_BUILD_TYPE -t ${CMAKE_TARGET}
else
    cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t ${CMAKE_TARGET} 
fi

if [[ $? != 0 ]]; then
    echo "COMPILATION of mrv2 failed!"
    exit 1
fi


cd -

. etc/build_end.sh
