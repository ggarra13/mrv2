# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


set(HEADERS
    USDCollectTextures.h
    USDGetMaterials.h
    USDRender.h
    USDRenderEngine.h
    USDRenderShadersBinary.h
    USDResolveTexture.h

    usd/material.h
    usd/primvars.h

    Vk/VAOPool.h
)
set(SOURCES
    USDCollectTextures.cpp
    USDGetMaterials.cpp
    USDRender.cpp
    USDRenderEngine.cpp
    USDRenderPrims.cpp
    USDRenderShaders.cpp
    USDRenderVulkan.cpp
    USDResolveTexture.cpp

    usd/primvars.cpp
    
    Vk/VAOPool.cpp
    
    main.cpp)

find_package(Vulkan REQUIRED)

find_package(pxr REQUIRED)
add_definitions(-DTLRENDER_USD)

set(LIBRARIES )
list(APPEND LIBRARIES OpenEXR::OpenEXR tlRender::tlVk tlRender::tlCore tf usd usdUtils usdGeom usdShade usdSkel hio hd)
list(APPEND LIBRARIES ${FLTK_vk_LIBRARY} ${FLTK_LIBRARIES})
list(APPEND LIBRARIES Vulkan::Vulkan)


if( APPLE )
    set(OSX_FRAMEWORKS "-framework IOKit")
    list(APPEND LIBRARIES ${OSX_FRAMEWORKS})
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;/usr/local/lib")
endif()

add_executable(usdviewer WIN32 ${SOURCES} ${HEADERS})

target_include_directories(usdviewer BEFORE PRIVATE .)

target_link_libraries(usdviewer PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories( usdviewer BEFORE PUBLIC ${CMAKE_INSTALL_PREFIX}/lib /usr/local/lib )

install(TARGETS usdviewer
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION lib COMPONENT libraries
    ARCHIVE DESTINATION lib COMPONENT libraries )



