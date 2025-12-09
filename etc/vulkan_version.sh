#!/usr/bin/env bash

# export VK_DOWNLOAD=latest

#
# Verified on vmrv2 v1.4.6 on Windows
#
# Use latest to use latest download
#
export VK_DOWNLOAD=1.4.321.1
export VK_STRIPPED=$VK_DOWNLOAD

if [[ $VK_DOWNLOAD =~ ^([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+)$ ]]; then
    # Remove the last ".NUMBER"
    VK_STRIPPED=${VK_DOWNLOAD%.*}.0
else
    # Leave it unchanged
    VK_STRIPPED=$VK_DOWNLOAD
fi

echo ""
echo "Vulkan version to try to download ${VK_DOWNLOAD}"
echo ""
