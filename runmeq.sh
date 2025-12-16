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
echo 

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

if [[ "$CMAKE_TARGET" == "package" ]]; then
    
    clean_mo_files
    
    echo
    echo ".mo files not found."
    echo
    
    cd $dir
    
    run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t pot
    run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t mo
    run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install

    cd -
fi



if [[ "$CMAKE_TARGET" == "package" ]]; then
    
    cd $dir
	
    run_cmd cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t install

    cd -
fi

cd $dir

MAX_RETRIES=3
if [[ $CMAKE_TARGET != "package" ]]; then
    MAX_RETRIES=1
fi
COUNT=1

while (( COUNT <= MAX_RETRIES )); do
    echo "Build attempt $COUNT of $MAX_RETRIES..."

    cmake --build . $FLAGS --config $CMAKE_BUILD_TYPE -t "${CMAKE_TARGET}"
    STATUS=$?
    
    if [[ $STATUS -eq 0 ]]; then
        echo "Build succeeded on attempt $COUNT"

	cd -

	. etc/build_end.sh

        exit 0
    fi

    echo "Build failed (attempt $COUNT)."
    (( COUNT++ ))
done



