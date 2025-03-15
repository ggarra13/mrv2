#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
# This script checks the languages .po files for missing translations.
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

echo $PYTHON_SITEDIR/polib.py 
if [[ ! -e $PYTHON_SITEDIR/polib.py && ! -e $PYTHON_USER_SITEDIR/polib.py ]];
then
    $PYTHON -m pip install polib
fi

$PYTHON bin/python/copy_po.py $*
