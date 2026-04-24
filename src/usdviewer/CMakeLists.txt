# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.


set(HEADERS
    USDCollectTextures.h
    USDGetMaterials.h
    USDRenderEngine.h

    USDRender/ShadersBinary.h
    USDRender/Private.h
    USDRender/Render.h

    USDEnginePrimitive.h
    USDResolveTexture.h

    engine/options.h

    usdgeom/pointBased.h
    usdgeom/prim.h
    usdgeom/mesh.h
    
    usd/material.h
    usd/primvars.h
    usd/subsetsSplit.h

    Vk/VAOPool.h
)
set(SOURCES
    
    USDRender/Render.cpp
    USDRender/Prims.cpp
    USDRender/Shaders.cpp
    USDRender/Vulkan.cpp
    
    USDCollectTextures.cpp
    USDGetMaterials.cpp
    USDRenderEngine.cpp
    USDResolveTexture.cpp
    USDSceneTraversal.cpp

    usdgeom/pointBased.cpp
    usdgeom/prim.cpp
    usdgeom/mesh.cpp
    
    usd/primvars.cpp
    usd/subsetsSplit.cpp
    
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
if(UNIX AND NOT APPLE)
    target_compile_options(usdviewer PUBLIC -Wno-deprecated)
endif()

target_link_libraries(usdviewer PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
target_link_directories( usdviewer BEFORE PUBLIC ${CMAKE_INSTALL_PREFIX}/lib /usr/local/lib )

install(TARGETS usdviewer
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION lib COMPONENT libraries
    ARCHIVE DESTINATION lib COMPONENT libraries )



