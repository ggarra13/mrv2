#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
# This script checks the download count of mrv2 releases in github.com
#
# You must run it from the root of the mrv2 project.
#

. etc/build_dir.sh

if [[ $KERNEL == *Msys* ]]; then
    export PATH=$BUILD_DIR/install/bin:$PATH
    python -m pip install requests
fi

export TAG=$1

if [[ "$TAG" == "" ]]; then
    export TAG=`git ls-remote --tags --refs | tail -n1 | cut -d/ -f3`
fi

bin/github-download-count.py ggarra13 mrv2 $TAG
