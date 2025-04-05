#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

curl -L -o /tmp/vulkan-sdk.exe "https://sdk.lunarg.com/sdk/download/latest/windows/vulkan-sdk.exe"

ls /tmp

export VULKAN_SDK="/C/VulkanSDK"

/tmp/vulkan-sdk.exe \
    --root "$VULKAN_SDK" \
    --accept-licenses \
    --default-answer \
    --confirm-command install

rm -f /tmp/vulkan-sdk.exe
