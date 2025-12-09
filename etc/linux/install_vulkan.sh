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


. etc/vulkan_version.sh

#curl -L -o /tmp/vulkan-sdk.tar.gz "https://sdk.lunarg.com/sdk/download/latest/linux/vulkan-sdk.tar.gz"
#curl -L -o /tmp/vulkan-sdk.tar.gz "https://sdk.lunarg.com/sdk/download/1.4.328.1/linux/vulkansdk-linux-x86_64-1.4.328.1.tar.xz"

# curl -L -o /tmp/vulkan-sdk.tar.gz "https://sdk.lunarg.com/sdk/download/${VK_DOWNLOAD}/linux/vulkansdk-linux-x86_64-1.4.328.1.tar.xz"

TAR_XZ=/tmp/vulkan-sdk.tar.xz
rm -rf $TAR_XZ

needs_download() {
    # return 0 (true) if file does not exist OR is smaller than 8192 bytes
    [[ ! -e "$TAR_XZ" ]] || (( $(wc -c <"$TAR_XZ") < 8192 ))
}

# 1) Try full version (unless "latest")
if [[ "$VK_DOWNLOAD" != "latest" ]] && needs_download; then
    curl -L -o "$TAR_XZ" "https://sdk.lunarg.com/sdk/download/${VK_DOWNLOAD}/linux/vulkan-sdk-linux-x86_64-${VK_DOWNLOAD}.tar.xz"   
fi

# 2) Try stripped version if still too small
if needs_download && [[ -n "$VK_STRIPPED" ]]; then
    curl -L -o "$TAR_XZ" "https://sdk.lunarg.com/sdk/download/${VK_DOWNLOAD}/linux/vulkan-sdk-linux-x86_64-${VK_STRIPPED}.tar.xz"  
fi

# 3) Fallback to latest
if needs_download; then
    echo "Vulkan version ${VK_DOWNLOAD} not found! Downloading latest"
    curl -L -o "$TAR_XZ" "https://sdk.lunarg.com/sdk/download/latest/linux/vulkan-sdk.tar.xz"xs
fi

#
#
#
if [[ $ARCH == *arm64* || $ARCH == *aarch64* ]]; then
    if [ -z $VULKAN_COMPILE ]; then
	export VULKAN_COMPILE=ON
    fi
fi


echo "After downloading it..."
ls /tmp

mkdir -p VulkanSDK-${KERNEL}
cd VulkanSDK-${KERNEL}
tar -xvf $TAR_XZ

cd ..

export VULKAN_ROOT=$PWD/VulkanSDK-${KERNEL}
export SDK_VERSION=$(ls -d ${VULKAN_ROOT}/* | sort -r | grep -o "$VULKAN_ROOT/[0-9]*\..*"| sed -e "s#$VULKAN_ROOT/##" | head -1)
export COMPILE_VERSION=$(echo "$SDK_VERSION" | sed -E 's/^([0-9]+\.[0-9]+\.[0-9]+).*$/\1/')

if [[ $ARCH == *arm64* || $ARCH == *aarch64* ]]; then
    if [ -z $BUILD_VULKAN ]; then
	export BUILD_VULKAN=ON
    fi

    if [[ -d $VULKAN_ROOT/$SDK_VERSION/x86_64 ]]; then
	cd $VULKAN_ROOT/$SDK_VERSION
	mv x86_64 $UNAME_ARCH
    fi
fi

export VULKAN_SDK=$VULKAN_ROOT/$SDK_VERSION/$UNAME_ARCH
export VK_LAYER_PATH=$VULKAN_SDK/lib
export PATH=${VULKAN_SDK}/bin:$PATH

rm -f $TAR_XZ

if [[ "$BUILD_VULKAN" == "ON" || "$BUILD_VULKAN" == "1" ]]; then
    . etc/common/build_vulkan.sh
else
    echo "-------------------------------"
    echo "   Using pre-compiled Vulkan   "
    echo "-------------------------------"
fi

echo "VULKAN_SDK=$VULKAN_SDK"
echo "----------------------"
echo
echo "Contains..."
echo
ls $VULKAN_SDK
