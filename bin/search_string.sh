#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/usr/bin/env bash

#
# This script quickly searches for a string in all .sh, .cmake and mrv2
# source code.
#

echo "Searching for $1..."
grep -rn --exclude-dir='BUILD-*' --exclude-dir='.git' --exclude-dir='tlRender' $1
