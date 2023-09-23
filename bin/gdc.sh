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
    requests=$BUILD_DIR/install/bin/Lib/site-packages/requests
    if [[ ! -e $requests ]]; then
	python -m pip install requests
    fi
else    
    extract_python_version
    requests=$BUILD_DIR/install/lib/python$PYTHON_VERSION/site-packages/requests
    if [[ ! -e $requests ]]; then
	python$PYTHON_VERSION -m pip install requests
    fi
fi


export TAG=$1

if [[ "$TAG" == "" ]]; then
    export TAG=`git ls-remote --tags --refs | tail -n1 | cut -d/ -f3`
fi

bin/python/github-download-count.py ggarra13 mrv2 $TAG
