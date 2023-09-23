#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


#
# This script checks the languages .po files for missing translations.
#
# You must run it from the root of the mrv2 project.
#

. etc/build_dir.sh

bin/python/missing_translate.py
