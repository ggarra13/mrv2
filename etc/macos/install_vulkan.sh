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


#
# Prefix of macOS installer
#
VULKAN_DOWNLOAD=InstallVulkan-


#
# Get SDK version from macOS installer
#
SDK_VERSION=$(ls -d ${VULKAN_DOWNLOAD}* | grep -o "$VULKAN_DOWNLOAD[0-9]*\..*"| sed -e "s#$VULKAN_DOWNLOAD/##" | sed -e "s#.app##")


#
# Go back to mrv2's root dir
#
cd -

mkdir -p VulkanSDK-${KERNEL}
export VULKAN_ROOT=VulkanSDK-${KERNEL}

/tmp/InstallVulkan-${SDK_VERSION}.app/Contents/MacOS/InstallVulkan-${SDK_VERSION} \
  --root ${VULKAN_ROOT}/${SDK_VERSION} \
  --accept-licenses \
  --default-answer \
  --confirm-command install

rm -rf /tmp/${VULKAN_DOWNLOAD}-*

    

export VULKAN_SDK=${VULKAN_ROOT}/${SDK_VERSION}/macOS
export VK_LAYER_PATH=$VULKAN_SDK/lib
export VK_ICD_FILENAMES=$VULKAN_SDK/share/vulkan/icd.d/MoltenVK_icd.json
export PATH=${VULKAN_SDK}/bin:$PATH

if [[ "$BUILD_VULKAN" == "ON" || "$BUILD_VULKAN" == "1" ]]; then
    . etc/common/build_vulkan.sh
else
    echo "-------------------------------"
    echo "   Using pre-compiled Vulkan   "
    echo "-------------------------------"
fi
