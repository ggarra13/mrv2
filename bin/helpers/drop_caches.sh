#!/usr/bin/env bash

#
# Flushes the disk caches to measure performance of vmrv2 on a cold start.
#

# Write any dirty pages to disk
sync

sudo sh -c 'echo 3 > /proc/sys/vm/drop_caches'
