#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

curl -L -o /tmp/vulkan-sdk.tar.gz "https://sdk.lunarg.com/sdk/download/latest/linux/vulkan-sdk.tar.gz"


echo "After downloading it..."
ls /tmp

tar -xvf /tmp/vulkan-sdk.tar.gz

echo "After decompressing it..."
ls /tmp

export VULKAN_SDK="/tmp/"

rm -f /tmp/vulkan-sdk.tar.gz
