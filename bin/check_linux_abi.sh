#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script checks the minimum Linux ABI version of an executable or a
# library.
#

readelf -V $1 | grep -oP '(GLIBC|GLIBCXX)_[0-9]+\.[0-9]+' | sort -V | uniq
