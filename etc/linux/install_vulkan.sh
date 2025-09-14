#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

. etc/functions.sh

get_kernel

# SDK docs from:
#
# https://vulkan.lunarg.com/doc/sdk/1.4.321.1/linux/getting_started.html
#
# Khronos Vulkan Headers: https://github.com/KhronosGroup/Vulkan-Headers
# Khronos Vulkan Loader source: https://github.com/KhronosGroup/Vulkan-Loader
# Khronos Vulkan Validation Layers source: https://github.com/KhronosGroup/Vulkan-ValidationLayers
# Khronos Vulkan Tools source: https://github.com/KhronosGroup/Vulkan-Tools
# LunarG Vulkan Tools source: https://github.com/LunarG/VulkanTools
# Khronos glslang source: https://github.com/KhronosGroup/glslang
# Google Shaderc source: https://github.com/google/shaderc
# Khronos SPIRV-Headers source: https://github.com/KhronosGroup/SPIRV-Headers
# Khronos SPIRV-Tools source: https://github.com/KhronosGroup/SPIRV-Tools
# Khronos SPIRV-Cross source: https://github.com/KhronosGroup/SPIRV-Cross
# LunarG GFXReconstruct source: https://github.com/LunarG/gfxreconstruct
# Khronos SPIRV-Reflect source: https://github.com/KhronosGroup/SPIRV-Reflect
# Khronos Vulkan Extension Layer source: https://github.com/KhronosGroup/Vulkan-ExtensionLayer
# Khronos Vulkan Profiles: https://github.com/KhronosGroup/Vulkan-Profiles
# Microsoft DirectX Shader Compiler: https://github.com/microsoft/DirectXShaderCompiler
# Volk Vulkan Meta Loader: https://github.com/zeux/volk
# Vulkan Memory Allocator Library: https://gpuopen.com/vulkan-memory-allocator/
# Vulkan Caps Viewer: https://github.com/SaschaWillems/VulkanCapsViewer
# Vulkan Utility Libraries: https://github.com/KhronosGroup/Vulkan-Utility-Libraries.git
# Crash Diagnostic Layer: https://github.com/LunarG/CrashDiagnosticLayer.git
# Vulkan Profiles: https://github.com/KhronosGroup/Vulkan-Profiles.git
# slang: https://github.com/shader-slang/slang.git


curl -L -o /tmp/vulkan-sdk.tar.gz "https://sdk.lunarg.com/sdk/download/latest/linux/vulkan-sdk.tar.gz"

echo "After downloading it..."
ls /tmp

mkdir -p VulkanSDK-Linux
cd VulkanSDK-Linux
tar -xvf /tmp/vulkan-sdk.tar.gz

cd ..

VULKAN_ROOT=$PWD/VulkanSDK-Linux
export SDK_VERSION=$(ls -d ${VULKAN_ROOT}/* | sort -r | grep -o "$VULKAN_ROOT/[0-9]*\..*"| sed -e "s#$VULKAN_ROOT/##" | head -1)
export COMPILE_VERSION=$(echo "$SDK_VERSION" | sed -E 's/^([0-9]+\.[0-9]+\.[0-9]+).*$/\1/')


export VULKAN_SDK=$VULKAN_ROOT/$SDK_VERSION/$UNAME_ARCH

    
if [[ $ARCH == *arm64* || $ARCH == *aarch64* ]]; then
    . etc/linux/build_vulkan.sh
else
    echo "-------------------------------"
    echo "   Using pre-compiled Vulkan   "
    echo "-------------------------------"
fi

export VK_LAYER_PATH=$VULKAN_SDK/lib
export PATH=${VULKAN_SDK}/bin:$PATH

ls $VULKAN_SDK

rm -f /tmp/vulkan-sdk.tar.gz
