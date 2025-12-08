#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

. etc/vulkan_version.sh

rm -rf /mtp/vulkan-sdk.zip

if [[ "$VK_DOWNLOAD" != "latest" ]]; then
    curl -L -o /tmp/vulkan-sdk.zip "https://sdk.lunarg.com/sdk/download/${VK_DOWNLOAD}/mac/vulkansdk-macos-${VK_DOWNLOAD}.zip"
fi

#
# If download failed, version is prolaly not available.  Use latest.
#
if [[ ! -e /tmp/vulkan-sdk.zip ]]; then
    echo "Vulkan version ${VK_DOWNLOAD} not found! Downloading latest"
    curl -L -o /tmp/vulkan-sdk.zip "https://sdk.lunarg.com/sdk/download/latest/mac/vulkan-sdk.zip"
fi

cd /tmp
unzip vulkan-sdk.zip
rm -f vulkan-sdk.zip


VULKANROOT=InstallVulkan-

SDK_VERSION=$(ls -d $VULKANROOT* | grep -o "$VULKAN_ROOT[0-9]*\..*"| sed -e "s#$VULKAN_ROOT/##" | sed -e "s#.app##")

/tmp/InstallVulkan-${SDK_VERSION}.app/Contents/MacOS/InstallVulkan-${SDK_VERSION} \
  --root ~/VulkanSDK/${SDK_VERSION} \
  --accept-licenses \
  --default-answer \
  --confirm-command install

rm -rf /tmp/InstallVulkan-*


export VULKAN_SDK=~/VulkanSDK/${SDK_VERSION}/macOS
export VK_LAYER_PATH=$VULKAN_SDK/lib
export VK_ICD_FILENAMES=$VULKAN_SDK/share/vulkan/icd.d/MoltenVK_icd.json
export PATH=${VULKAN_SDK}/bin:$PATH
