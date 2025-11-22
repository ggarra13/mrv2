#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/usr/bin/env bash

apps="mrv2 vmrv2 hdr"
for app in $apps; do
    sudo xattr -rd com.apple.quarantine /Applications/${app}.app/
done
