#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
# Minimal build script for mrv2 with NDI support.
#

#
# Store the parameters for passing them later
#
params="$@"

. etc/build_dir.sh

#
# Clear the flags, as they will be set by runme_nolog.sh.
#
export FLAGS=""
export CMAKE_FLAGS=""

export TLRENDER_NDI=ON

if [[ $KERNEL == *Linux* ]]; then
    export TLRENDER_NDI_SDK="/home/gga/code/lib/NDI\ SDK\ for\ Linux/"
elif [[ $KERNEL == *Msys* ]]; then
    export TLRENDER_NDI_SDK="C:/Program\ Files/NDI/NDI\ 5\ SDK/"
else
    export TLRENDER_NDI_SDK="/Library/NDI\ SDK\ for\ Apple/"
fi

./runme.sh $params
