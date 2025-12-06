#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script needs to be run with Administrator rights.
#

. etc/build_dir.sh
. etc/vulkan_version.sh

if [[ $ARCH == *aarch64* || $ARCH == *arm64* ]]; then
    curl -L -o /tmp/vulkan-sdk.exe "https://sdk.lunarg.com/sdk/download/${VK_DOWNLOAD}/warm/vulkansdk-windows-ARM64-${VK_DOWNLOAD}.exe"
else
    curl -L -o /tmp/vulkan-sdk.exe "https://sdk.lunarg.com/sdk/download/${VK_DOWNLOAD}/windows/vulkan-sdk-windows-X64-${VK_DOWNLOAD}.exe"
fi

if [[ ! -e /tmp/vulkan-sdk.exe ]]; then
    echo "VK_DOWNLOAD ${VK_DOWNLOAD} version not found.  Downloading latest."
    if [[ $ARCH == *aarch64* || $ARCH == *arm64* ]]; then
        curl -L -o /tmp/vulkan-sdk.exe "https://sdk.lunarg.com/sdk/download/latest/warm/vulkan-sdk.exe"
    else
	curl -L -o /tmp/vulkan-sdk.exe "https://sdk.lunarg.com/sdk/download/latest/windows/vulkan-sdk.exe"
    fi
fi

exit

ls /tmp

export VULKAN_SDK="/C/VulkanSDK"
rm -rf $VULKAN_SDK

/tmp/vulkan-sdk.exe \
    --root "$VULKAN_SDK" \
    --accept-licenses \
    --default-answer \
    --confirm-command install

rm -f /tmp/vulkan-sdk.exe
