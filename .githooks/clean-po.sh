#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

filename="$PWD/$1"
sed -e '/^#: /d' "$filename" > /tmp/no_comments.po
sed -e 's/^"POT-Creation-Date.*//' /tmp/no_comments.po > /tmp/no_dates.po
cat /tmp/no_dates.po
