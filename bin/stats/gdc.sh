#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
# This script checks the download count of mrv2 releases in github.com and
# sourceforge.
#

if [[ ! -e etc/build_dir.sh ]]; then
    echo "You must run this script from the root of mrv2 directory like:"
    echo
    script=`basename $0`
    echo "> bin/$script"
    exit 1
fi

NOARGS=1
. etc/build_dir.sh

locate_python

export LD_LIBRARY_PATH=$BUILD_DIR/install/lib64:$BUILD_DIR/install/lib:$OLD_LIBRRARY_PATH

if [[ ! -d $PYTHON_SITEDIR/requests || ! -d $PYTHON_SITEDIR/pytz ]];
then
    $PYTHON -m pip install requests pytz
fi

export TAG=$1

if [[ "$TAG" == "" ]]; then
    export TAG=`git ls-remote --tags --refs 2> /dev/null | grep -E 'v[0-9]+\.[0-9]+\.[0-9]+$' | tail -n1 | cut -d/ -f3`
fi

# Get all tags sorted by version
all_tags=$(git ls-remote --tags --refs 2> /dev/null | grep -E 'v[0-9]+\.[0-9]+\.[0-9]+$' | cut -d/ -f3 | sort -V)

# Find the next tag after the current one
next_tag=$(echo "$all_tags" | grep -A1 "^$TAG$" | tail -n1)

# If the next tag is the same as the current tag, there is no next tag
if [[ "$next_tag" == "$TAG" || -z "$next_tag" ]]; then
    next_tag_date=$(date +'%F %T')
else
    next_tag_date=$(git for-each-ref --format="%(creatordate:iso)" refs/tags/$next_tag)
fi

date_created=$(git for-each-ref --format="%(creatordate:iso)" refs/tags/$TAG)
if [[ "$date_created" == "" ]]; then
    git pull
    date_created=$(git for-each-ref --format="%(creatordate:iso)" refs/tags/$TAG)
fi

if [[ "$date_created" == "" ]]; then
    echo "No tag ${TAG} found!"
    exit 1
fi


$PYTHON bin/python/github-download-count.py ggarra13 mrv2 $TAG "$date_created" "$next_tag_date"
