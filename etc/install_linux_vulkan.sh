#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

. etc/functions.sh

get_kernel

curl -L -o /tmp/vulkan-sdk.tar.gz "https://sdk.lunarg.com/sdk/download/latest/linux/vulkan-sdk.tar.gz"


echo "After downloading it..."
ls /tmp

mkdir -p VulkanSDK
cd VulkanSDK
tar -xvf /tmp/vulkan-sdk.tar.gz

cd ..

VULKAN_ROOT=$PWD/VulkanSDK
SDK_VERSION=$(ls -d ${VULKAN_ROOT}/* | sort -r | grep -o "$VULKAN_ROOT/[0-9]*\..*"| sed -e "s#$VULKAN_ROOT/##" | head -1)
export VULKAN_SDK=$VULKAN_ROOT/$SDK_VERSION/$UNAME_ARCH

echo "Contents of SDK:"
ls $VULKAN_SDK

rm -f /tmp/vulkan-sdk.tar.gz
