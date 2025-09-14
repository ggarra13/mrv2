#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

. etc/build_dir.sh

if [[ $ARCH == *aarch64* || $ARCH == *arm64* ]]; then
    # curl -L -o /tmp/vulkan-sdk.exe "https://sdk.lunarg.com/sdk/download/1.4.321.1/latest/windows/vulkansdk-windows-ARM64-1.4.321.1.exe"
    curl -L -o /tmp/vulkan-sdk.exe "https://sdk.lunarg.com/sdk/download/1.4.321.1/warm/vulkansdk-windows-ARM64-1.4.321.1.exe"
else
    curl -L -o /tmp/vulkan-sdk.exe "https://sdk.lunarg.com/sdk/download/latest/windows/vulkan-sdk.exe"
fi

ls /tmp

export VULKAN_SDK="/C/VulkanSDK"
rm -rf $VULKAN_SDK

/tmp/vulkan-sdk.exe \
    --root "$VULKAN_SDK" \
    --accept-licenses \
    --default-answer \
    --confirm-command install

rm -f /tmp/vulkan-sdk.exe
