#!/usr/bin/env bash

. etc/functions.sh

locate_python

if [[ -z "$UNAME_ARCH" ]]; then
    UNAME_ARCH=x86_64
fi

if [[ -z "$BUILD_WERROR" ]]; then
    BUILD_WERROR=OFF
fi

if [[ -z "$BUILD_VULKAN_HEADERS" ]]; then
    BUILD_VULKAN_HEADERS=ON
fi

if [[ -z "$BUILD_VULKAN_LOADER" ]]; then
    BUILD_VULKAN_LOADER=ON
fi

if [[ -z "$BUILD_SPIRV_TOOLS" ]]; then
    BUILD_SPIRV_TOOLS=ON
fi

if [[ -z "$BUILD_GLSLANG" ]]; then
    BUILD_GLSLANG=ON
fi

if [[ -z "$BUILD_SPIRV_HEADERS" ]]; then
    BUILD_SPIRV_HEADERS=ON
fi

if [[ -z "$BUILD_VULKAN_EXTENSIONLAYER" ]]; then
    BUILD_VULKAN_EXTENSIONLAYER=OFF  # broken on Linux
fi

if [[ -z "$BUILD_VULKAN_PROFILES" ]]; then
    BUILD_VULKAN_PROFILES=ON
fi

if [[ -z "$BUILD_VULKAN_VALIDATIONLAYERS" ]]; then
    BUILD_VULKAN_PROFILES=ON
fi

if [[ -z "$BUILD_SHADERC" ]]; then
    BUILD_SHADERC=ON
fi


#
# Function used to try to checkout the correct SDK version or as close to it
# as possible.  If it fails, it will compile from HEAD.
#
try_checkout()
{
    checkouts="v$SDK_VERSION vulkan-sdk-v$SDK_VERSION sdk-$SDK_VERSION v$COMPILE_VERSION vulkan-sdk-v$COMPILE_VERSION sdk-$COMPILE_VERSION main master"
    for checkout in $checkouts; do
	cmd=`git checkout $checkout &> /dev/null`
	if [[ $? == 0 ]]; then
	    return
	fi
    done
}

try_build()
{
    cmake --build build
    if [[ $? != 0 ]]; then
	echo "************ COMPILATION FAILED *****************"
	exit 1
    fi
    cmake --build build -t install
}


if [[ ! -d VulkanSDK-${KERNEL} ]]; then
    echo "No VulkanSDK downloaded for Kernel ${KERNEL}"
    echo "Will not compile it."
    exit 1
fi

if [[ ! -d $VULKAN_ROOT ]]; then
    VULKAN_ROOT=$PWD/VulkanSDK-${KERNEL}
fi
export SDK_VERSION=$(ls -d ${VULKAN_ROOT}/* | sort -r | grep -o "$VULKAN_ROOT/[0-9]*\..*"| sed -e "s#$VULKAN_ROOT/##" | head -1)
export COMPILE_VERSION=$(echo "$SDK_VERSION" | sed -E 's/^([0-9]+\.[0-9]+\.[0-9]+).*$/\1/')

if [[ -z $VULKAN_SDK ]]; then
    export VULKAN_SDK=$VULKAN_ROOT/$SDK_VERSION/$UNAME_ARCH
fi

echo "--------------------------------------------------------"
echo "Compiling Vulkan SDK ${SDK_VERSION} on $KERNEL          "
echo "--------------------------------------------------------"

mkdir -p compile
cd compile

export CMAKE_INSTALL_PREFIX=${VULKAN_SDK}
export CMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_PREFIX}/lib
export CMAKE_PREFIX_PATH=${VULKAN_SDK}


# https://vulkan.lunarg.com/doc/sdk/1.4.321.1/linux/getting_started.html
#
##########################
# Vulkan Headers         #
##########################
if [[ "$BUILD_VULKAN_HEADERS" == "ON" || "$BUILD_VULKAN_HEADERS" == "1" ]]; then
    if [ ! -d Vulkan-Headers ]; then
	git clone https://github.com/KhronosGroup/Vulkan-Headers
    fi

    cd Vulkan-Headers
    try_checkout
    cmake -S . -B build -D CMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX}
    cmake --install build --prefix ${CMAKE_INSTALL_PREFIX}
    cd .. 
fi

##########################
# Vulkan Loader          #
##########################
if [[ "$BUILD_VULKAN_LOADER" == "ON" || "$BUILD_VULKAN_LOADER" == "1" ]]; then
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
	  -D CMAKE_INSTALL_PREFIX=$CMAKE_INSTALL_PREFIX
    try_build

    # Alternatively, using Python
    # mkdir build
    # cd build
    # python ../scripts/update_deps.py
    # cmake -C helper.cmake ..
    # cmake --build .
    cd ..
fi

# Khronos Vulkan Tools source: https://github.com/KhronosGroup/Vulkan-Tools
# LunarG Vulkan Tools source: https://github.com/LunarG/VulkanTools

###################################
# SPIRV-Tools and SPIRV-Tools-opt #
###################################
if [[ "$BUILD_SPIRV_TOOLS" == "ON" || "$BUILD_SPIRV_TOOLS" == "1" ]]; then
    if [[ ! -d SPIRV-Tools ]]; then
	git clone https://github.com/KhronosGroup/SPIRV-Tools
    fi

    cd SPIRV-Tools
    try_checkout
    ${PYTHON} utils/git-sync-deps
    cmake -G Ninja -B build \
	  -D SPIRV_SKIP_EXECUTABLES=ON \
	  -D SPIRV_SKIP_TESTS=ON \
	  -D CMAKE_BUILD_TYPE=Release \
	  -D CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} \
	  -D CMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
	  -D CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    try_build

    cd ..
fi



###############
# SPIRV-Cross #
###############
if [[ "$BUILD_SPIRV_CROSS" == "ON" || "$BUILD_SPIRV_CROSS" == "1" ]]; then
    if [[ ! -d SPIRV-Cross ]]; then
	git clone https://github.com/KhronosGroup/SPIRV-Cross
    fi
    cd SPIRV-Cross
    try_checkout
    cmake -G Ninja -B build \
	  -D CMAKE_BUILD_TYPE=Release \
	  -D CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} \
	  -D CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} \
	  -D SPIRV_CROSS_ENABLE_C_API=OFF \
	  -D SPIRV_CROSS_CLI=OFF \
	  -D SPIRV_CROSS_ENABLE_HLSL=OFF \
	  -D SPIRV_CROSS_ENABLE_TESTS=OFF \
	  -D SPIRV_CROSS_FORCE_PIC=ON  # it was off by default
    try_build
    cd ..
fi

##########################
# glslang                #
##########################
if [[ "$BUILD_GLSLANG" == "ON" || "$BUILD_GLSLANG" == "1" ]]; then
    if [[ ! -d glslang ]]; then
	git clone https://github.com/KhronosGroup/glslang
    fi

    cd glslang
    try_checkout
    ${PYTHON} ./update_glslang_sources.py
    cmake -G Ninja -B build \
	  -D CMAKE_BUILD_TYPE=Release \
	  -D CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} \
	  -D CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} \
	  -D ALLOW_EXTERNAL_SPIRV_TOOLS=ON \
	  -D BUILD_EXTERNAL=OFF
    try_build
    cd ..
fi

###########################
# SPIRV-Headers (needed?) #
###########################
# if [[ "$BUILD_SPIRV_HEADERS" == "ON" || "$BUILD_SPIRV_HEADERS" == "1" ]]; then
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
if [[ "$BUILD_VULKAN_EXTENSIONLAYER" == "ON" || \
	  "$BUILD_VULKAN_EXTENSIONLAYER" == "1" ]]; then
    if [[ ! -d Vulkan-ExtensionLayer ]]; then
	git clone https://github.com/KhronosGroup/Vulkan-ExtensionLayer
    fi

    cd Vulkan-ExtensionLayer
    try_checkout
    cmake -S . -B build \
	  -D UPDATE_DEPS=ON \
	  -D BUILD_WERROR=${BUILD_WERROR} \
	  -D BUILD_TESTS=OFF \
	  -D CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} \
	  -D CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} \
	  -D CMAKE_BUILD_TYPE=Release
    try_build
    cd ..
fi

##############################
# Vulkan-Profiles            #
##############################
if [[ "$BUILD_VULKAN_PROFILES" == "ON" || \
	  "$BUILD_VULKAN_PROFILES" == "1" ]]; then
    if [[ ! -d Vulkan-Profiles ]]; then
	git clone https://github.com/KhronosGroup/Vulkan-Profiles
    fi

    cd Vulkan-Profiles
    try_checkout
    cmake -S . -B build \
	  -D UPDATE_DEPS=ON \
	  -D BUILD_WERROR=${BUILD_WERROR} \
	  -D BUILD_TESTS=OFF \
	  -D CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} \
	  -D CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} \
	  -D CMAKE_BUILD_TYPE=Release
    try_build
    cd ..
fi

######################################################
# Vulkan-ValidationLayers (cannot use prev. install) #
######################################################
if [[ "$BUILD_VULKAN_VALIDATIONLAYERS" == "ON" || \
	  "$BUILD_VULKAN_VALIDATIONLAYERS" == "1" ]]; then
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
	  -D CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} \
	  -D CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}

    try_build

    cd ..
fi

##############################
# shaderc                    #
##############################
if [[ "$BUILD_SHADERC" == "ON" || "$BUILD_SHADERC" == "1" ]]; then
    if [[ ! -d shaderc ]]; then
	git clone "https://github.com/google/shaderc.git"
    fi
    cd shaderc
    try_checkout
    ./utils/git-sync-deps
    cmake -S . -B build \
	  -G Ninja \
	  -D UPDATE_DEPS=ON \
	  -D BUILD_TESTS=OFF \
	  -D SHADERC_ENABLE_WERROR_COMPILE=${BUILD_WERROR} \
	  -D SHADERC_SKIP_TESTS=ON \
	  -D SHADERC_SKIP_EXAMPLES=ON \
	  -D SHADERC_SKIP_EXECUTABLES=ON \
	  -D CMAKE_BUILD_TYPE=Release \
	  -D CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} \
	  -D CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}

    try_build

    cd ..
fi

cd ..

#
# Clean up disk space
#
rm -rf compile
