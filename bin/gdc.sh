#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
# This script checks the download count of mrv2 releases in github.com
#

if [[ ! -e etc/build_dir.sh ]]; then
    echo "You must run this script from the root of mrv2 directory like:"
    echo
    script=`basename $0`
    echo "> bin/$script"
    exit 1
fi

. etc/build_dir.sh

$PYTHONEXE -m pip install requests

export TAG=$1

if [[ "$TAG" == "" ]]; then
    export TAG=`git ls-remote --tags --refs | tail -n1 | cut -d/ -f3`
fi

$PYTHONEXE bin/python/github-download-count.py ggarra13 mrv2 $TAG
