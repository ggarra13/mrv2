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

locate_python

export LD_LIBRARY_PATH=$OLD_LD_LIBRARY_PATH:$BUILD_DIR/install/lib64:$BUILD_DIR/install/lib

if [[ ! -d $PYTHON_SITEDIR/requests ]]; then
    $PYTHON -m pip install requests
fi

export TAG=$1

if [[ "$TAG" == "" ]]; then
    export TAG=`git ls-remote --tags --refs | grep -E 'v[0-9]+\.[0-9]+\.[0-9]+$' | tail -n1 | cut -d/ -f3`
fi

date_created=`git for-each-ref --format="%(creatordate:iso)" refs/tags/$TAG`
today=`date +'%F %T'`
echo "DATE CREATED: ${date_created}"
echo "NOW         : ${today}"
$PYTHON bin/python/github-download-count.py ggarra13 mrv2 $TAG
