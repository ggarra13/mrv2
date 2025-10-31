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

if [[ ! -d $PYTHON_SITEDIR/requests || ! -d $PYTHON_SITEDIR/bs4 ]];
then
    if [[ $PYTHON != *$BUILD_DIR* ]]; then
	if [ ! -d .venv ]; then
	    $PYTHON -m venv venv
	fi
	. venv/bin/activate
	export PYTHON=python
    fi
    $PYTHON -m pip install requests bs4
fi

$PYTHON bin/python/download_assets.py

if [[ -e venv ]]; then
    if [[ $KERNEL != *Windows* ]]; then
	. venv/bin/deactivate
    fi
fi

rm -rf venv

