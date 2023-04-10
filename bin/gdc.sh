#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

. etc/build_dir.sh

if [[ $KERNEL == *Msys* ]]; then
    export PATH=$BUILD_DIR/install/bin:$PATH
    python -m pip install requests
fi

#
# This script counts the downloads of the latest github release of mrv2.
#
export TAG=`git ls-remote --tags --refs | tail -n1 | cut -d/ -f3`
bin/github-download-count.py ggarra13 mrv2 $TAG
