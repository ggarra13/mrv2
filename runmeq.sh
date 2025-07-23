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
max_retries=1
if [[ "${CMAKE_TARGET}" == "package" ]]; then
    max_retries=3
fi
retry_delay=5
success=0

for ((i=1; i<=max_retries; i++)); do
    echo "Attempt $i of $max_retries to build mrv2..."
    cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t ${CMAKE_TARGET}
    
    if [[ $? == 0 ]]; then
        echo "Build succeeded on attempt $i."
        success=1
        break
    else
        echo "Build failed on attempt $i."
        if [[ $i -lt $max_retries ]]; then
            echo "Retrying after $retry_delay seconds..."
            sleep $retry_delay
        fi
    fi
done

if [[ $success -ne 1 ]]; then
    echo "COMPILATION of mrv2 failed after $max_retries attempts!"
    exit 1
fi


cd -

. etc/build_end.sh
