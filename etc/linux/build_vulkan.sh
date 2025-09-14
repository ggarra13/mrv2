#!/usr/bin/env bash

if [[ -z "$UNAME_ARCH" ]]; then
    UNAME_ARCH=x86_64
fi

if [[ -z "$BUILD_WERROR" ]]; then
    BUILD_WERROR=OFF
fi

#
# Function used to try to checkout the correct SDK version or as close to it
# as possible.  If it fails, it will compile from HEAD.
#
try_checkout()
{
    checkouts="v$SDK_VERSION vulkan-sdk-v$SDK_VERSION sdk-$SDK_VERSION v$COMPILE_VERSION vulkan-sdk-v$COMPILE_VERSION sdk-$COMPILE_VERSION"
    for checkout in $checkouts; do
	cmd=`git checkout $checkout &> /dev/null`
	if [[ $? == 0 ]]; then
	    return
	fi
    done
}



if [[ ! -d VulkanSDK-Linux ]]; then
    curl -L -o /tmp/vulkan-sdk.tar.gz "https://sdk.lunarg.com/sdk/download/latest/linux/vulkan-sdk.tar.gz"

    echo "After downloading it..."
    ls /tmp

    mkdir -p VulkanSDK-Linux
    cd VulkanSDK-Linux
    tar -xvf /tmp/vulkan-sdk.tar.gz
    cd ..
fi

VULKAN_ROOT=$PWD/VulkanSDK-Linux
export SDK_VERSION=$(ls -d ${VULKAN_ROOT}/* | sort -r | grep -o "$VULKAN_ROOT/[0-9]*\..*"| sed -e "s#$VULKAN_ROOT/##" | head -1)
export COMPILE_VERSION=$(echo "$SDK_VERSION" | sed -E 's/^([0-9]+\.[0-9]+\.[0-9]+).*$/\1/')


export VULKAN_SDK=$VULKAN_ROOT/$SDK_VERSION/$UNAME_ARCH
if [[ -d ${VULKAN_SDK} && ! -d ${VULKAN_SDK}_orig ]]; then
    mv ${VULKAN_SDK} ${VULKAN_SDK}_orig
fi

echo "--------------------------------------------------------"
echo "Compiling Vulkan SDK ${SDK_VERSION} on Linux "
echo "--------------------------------------------------------"

mkdir -p compile
cd compile

# https://vulkan.lunarg.com/doc/sdk/1.4.321.1/linux/getting_started.html
#
# git clone https://github.com/KhronosGroup/Vulkan-Headers # Not needed, shipped i

##########################
# Vulkan Loader          #
##########################
if [ ! -d Vulkan-Loader ]; then
    git clone https://github.com/KhronosGroup/Vulkan-Loader
fi

cd Vulkan-Loader
try_checkout
# Using CMake.
cmake -S . -B build \
      -G Ninja \
      -D UPDATE_DEPS=On \
      -D BUILD_TESTS=OFF \
      -D BUILD_WSI_XCB_SUPPORT=OFF \
      -D CMAKE_BUILD_TYPE=Release \
      -D CMAKE_INSTALL_PREFIX=$VULKAN_SDK
cmake --build build
cmake --build build -t install
cd ..

# Alternatively, using Python
# mkdir build
# cd build
# python ../scripts/update_deps.py
# cmake -C helper.cmake ..
# cmake --build .


# Khronos Vulkan Tools source: https://github.com/KhronosGroup/Vulkan-Tools
# LunarG Vulkan Tools source: https://github.com/LunarG/VulkanTools

###################################
# SPIRV-Tools and SPIRV-Tools-opt #
###################################
if [[ ! -d SPIRV-Tools ]]; then
    git clone https://github.com/KhronosGroup/SPIRV-Tools
fi

cd SPIRV-Tools
try_checkout
python3 utils/git-sync-deps
cmake -G Ninja -B build \
      -D CMAKE_BUILD_TYPE=Release \
      -D CMAKE_PREFIX_PATH=${VULKAN_SDK} \
      -D CMAKE_INSTALL_PREFIX=${VULKAN_SDK}
cmake --build build
cmake --build build -t install

cd ..



###############
# SPIRV-Cross #
###############
if [[ ! -d SPIRV-Cross ]]; then
    git clone https://github.com/KhronosGroup/SPIRV-Cross
fi
cd SPIRV-Cross
try_checkout
cmake -G Ninja -B build \
      -D CMAKE_BUILD_TYPE=Release \
      -D CMAKE_PREFIX_PATH=${VULKAN_SDK} \
      -D CMAKE_INSTALL_PREFIX=${VULKAN_SDK} \
      -D SPIRV_CROSS_ENABLE_C_API=OFF \
      -D SPIRV_CROSS_CLI=OFF \
      -D SPIRV_CROSS_ENABLE_HLSL=OFF \
      -D SPIRV_CROSS_ENABLE_TESTS=OFF \
      -D SPIRV_CROSS_FORCE_PIC=ON  # it was off by default
cmake --build build
cmake --build build -t install
cd ..



##########################
# glslang                #
##########################
if [[ ! -d glslang ]]; then
    git clone https://github.com/KhronosGroup/glslang
fi

cd glslang
try_checkout
./update_glslang_sources.py
cmake -G Ninja -B build \
      -D CMAKE_BUILD_TYPE=Release \
      -D CMAKE_PREFIX_PATH=${VULKAN_SDK} \
      -D CMAKE_INSTALL_PREFIX=${VULKAN_SDK} \
      -D ALLOW_EXTERNAL_SPIRV_TOOLS=ON \
      -D BUILD_EXTERNAL=OFF
cmake --build build
cmake --build build -t install
cd ..

# Google Shaderc source: https://github.com/google/shaderc # Compiled by CMake

###########################
# SPIRV-Headers (needed?) #
###########################
# if [[ ! -d SPIRV-Headers ]]; then
#     git clone https://github.com/KhronosGroup/SPIRV-Headers
# fi

# cd SPIRV-Headers
# try_checkout
# cd ..



# LunarG GFXReconstruct source: https://github.com/LunarG/gfxreconstruct
# Khronos SPIRV-Reflect source: https://github.com/KhronosGroup/SPIRV-Reflect


#####################################
# Vulkan-ExternsionLayer (needed?)  #
#####################################
if [[ ! -d Vulkan-ExtensionLayer ]]; then
    git clone https://github.com/KhronosGroup/Vulkan-ExtensionLayer
fi

cd Vulkan-ExtensionLayer
try_checkout
cmake -S . -B build \
      -D UPDATE_DEPS=ON \
      -D BUILD_WERROR=${BUILD_WERROR} \
      -D BUILD_TESTS=OFF \
      -D CMAKE_PREFIX_PATH=${VULKAN_SDK} \
      -D CMAKE_INSTALL_PREFIX=${VULKAN_SDK} \
      -D CMAKE_BUILD_TYPE=Release
cmake --build build
cmake --build build -t install
cd ..

##############################
# Vulkan-Profiles            #
##############################
if [[ ! -d Vulkan-Profiles ]]; then
    git clone https://github.com/KhronosGroup/Vulkan-Profiles
fi

cd Vulkan-Profiles
try_checkout
cmake -S . -B build \
      -D UPDATE_DEPS=ON \
      -D BUILD_WERROR=${BUILD_WERROR} \
      -D BUILD_TESTS=OFF \
      -D CMAKE_PREFIX_PATH=${VULKAN_SDK} \
      -D CMAKE_INSTALL_PREFIX=${VULKAN_SDK} \
      -D CMAKE_BUILD_TYPE=Release
cmake --build build
cmake --build build -t install
cd ..

######################################################
# Vulkan-ValidationLayers (cannot use prev. install) #
#####################################################
if [[ ! -d Vulkan-ValidationLayers ]]; then
    git clone https://github.com/KhronosGroup/Vulkan-ValidationLayers
fi

cd Vulkan-ValidationLayers
try_checkout
cmake -S . -B build \
      -G Ninja \
      -D UPDATE_DEPS=ON \
      -D BUILD_WERROR=${BUILD_WERROR} \
      -D BUILD_TESTS=OFF \
      -D BUILD_WSI_XCB_SUPPORT=OFF \
      -D CMAKE_BUILD_TYPE=Release \
      -D CMAKE_PREFIX_PATH=$VULKAN_SDK \
      -D CMAKE_INSTALL_PREFIX=$VULKAN_SDK

cmake --build build
cmake --build build -t install

cd ..

##############################
# shaderc                    #
##############################
git clone "https://github.com/google/shaderc.git"
cd shaderc
try_checkout
cmake -S . -B build \
      -G Ninja \
      -D UPDATE_DEPS=ON \
      -D BUILD_TESTS=OFF \
      -D SHADERC_ENABLE_WERROR_COMPILE=${BUILD_WERROR} \
      -D SHADERC_SKIP_TESTS=ON \
      -D SHADERC_SKIP_EXAMPLES=ON \
      -D SHADERC_SKIP_EXECUTABLES=ON \
      -D CMAKE_BUILD_TYPE=Release \
      -D CMAKE_PREFIX_PATH=$VULKAN_SDK \
      -D CMAKE_INSTALL_PREFIX=$VULKAN_SDK

cmake --build build
cmake --build build -t install

cd ..

