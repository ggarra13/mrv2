#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#!/usr/bin/env bash

#
# Flushes the disk caches to measure performance of vmrv2 on a cold start.
#

# Write any dirty pages to disk
sync

sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
