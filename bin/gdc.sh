#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script counts the downloads of the latest github release of mrv2.
#
export TAG=`git ls-remote --tags --refs | tail -n1 | cut -d/ -f3`
bin/github-download-count.py ggarra13 mrv2 $TAG
