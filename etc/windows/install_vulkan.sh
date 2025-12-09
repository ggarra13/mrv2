#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

#
# This script needs to be run with Administrator rights.
#

. etc/build_dir.sh
. etc/vulkan_version.sh

EXE=/tmp/vulkan-sdk.exe
rm -rf $EXE

needs_download() {
    # return 0 (true) if file does not exist OR is smaller than 8192 bytes
    [[ ! -e "$EXE" ]] || (( $(wc -c <"$EXE") < 8192 ))
}

# 1) Try full version (unless "latest")
if [[ "$VK_DOWNLOAD" != "latest" ]] && needs_download; then
    if [[ $ARCH == *aarch64* || $ARCH == *arm64* ]]; then
	curl -L -o $EXE "https://sdk.lunarg.com/sdk/download/${VK_DOWNLOAD}/warm/vulkansdk-windows-ARM64-${VK_DOWNLOAD}.exe"
    else
	curl -L -o $EXE "https://sdk.lunarg.com/sdk/download/${VK_DOWNLOAD}/windows/vulkan-sdk-windows-X64-${VK_DOWNLOAD}.exe"
    fi
fi

# 2) Try stripped version if still too small
if needs_download && [[ -n "$VK_STRIPPED" ]]; then
    if [[ $ARCH == *aarch64* || $ARCH == *arm64* ]]; then
	curl -L -o $EXE "https://sdk.lunarg.com/sdk/download/${VK_DOWNLOAD}/warm/vulkansdk-windows-ARM64-${VK_STRIPPED}.exe"
    else
	curl -L -o $EXE "https://sdk.lunarg.com/sdk/download/${VK_DOWNLOAD}/windows/vulkan-sdk-windows-X64-${VK_STRIPPED}.exe"
    fi
fi

# 3) Fallback to latest
if needs_download; then
    echo "VK_DOWNLOAD ${VK_DOWNLOAD} version not found.  Downloading latest."
    if [[ $ARCH == *aarch64* || $ARCH == *arm64* ]]; then
        curl -L -o $EXE "https://sdk.lunarg.com/sdk/download/latest/warm/vulkan-sdk.exe"
    else
	curl -L -o $EXE "https://sdk.lunarg.com/sdk/download/latest/windows/vulkan-sdk.exe"
    fi
fi

ls /tmp

#
# Do not use the system VULKAN_SDK, which may be old.
#
export VULKAN_ROOT="$PWD/VulkanSDK-${KERNEL}"
export WINDOWS_VULKAN_ROOT=`cygpath -w "${VULKAN_ROOT}"`

$EXE \
    --root "$VULKAN_ROOT" \
    --accept-messages \
    --accept-licenses \
    -da \
    --confirm-command install

rm -f $EXE

if [[ "$VULKAN_COMPILE" == "ON" ]]; then
    . etc/common/build_vulkan.sh
else
    echo "---------------------------------------------"
    echo "   Using pre-compiled Vulkan for ${KERNEL}   "
    echo "---------------------------------------------"
    export VULKAN_SDK=$VULKAN_ROOT
    export WINDOWS_VULKAN_SDK=$WINDOWS_VULKAN_ROOT
fi


echo "VULKAN_SDK=$VULKAN_SDK"
echo "----------------------"
echo
echo "Contains..."
echo
ls $VULKAN_SDK
