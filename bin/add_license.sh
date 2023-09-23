#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
# This script adds the licensing information to all source files.
#
# You must run it from the root of the mrv2 project.
#

. etc/build_dir.sh

bin/python/add_license.py
