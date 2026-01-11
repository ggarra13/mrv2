# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvCompareOptions.h
    mrvDisplayOptions.h
    mrvEnums.h
    mrvEnvironmentMapOptions.h
    mrvEnvironmentMapOptionsInline.h
    mrvFilesPanelOptions.h
    mrvFilesPanelOptionsInline.h
    mrvImageOptions.h
    mrvLUTOptions.h
    mrvStereo3DOptions.h
    mrvStereo3DOptionsInline.h
    mrvTimelineItemOptions.h
    mrvUSD.h
)

set(SOURCES
    mrvCompareOptions.cpp
    mrvDisplayOptions.cpp
    mrvEnvironmentMapOptions.cpp
    mrvFilesPanelOptions.cpp
    mrvImageOptions.cpp
    mrvLUTOptions.cpp
    mrvStereo3DOptions.cpp
    mrvTimelineItemOptions.cpp
    mrvUSD.cpp
    mrvUSDInline.cpp
)

add_library(mrvOptions ${SOURCES} ${HEADERS})
files_to_absolute_paths()

set(LIBRARIES mrvCore )
set(LIBRARIES_PRIVATE )

if(TLRENDER_USD)
    list(APPEND LIBRARIES_PRIVATE tf usd usdGeom)
    if (TLRENDER_GL)
	list(APPEND LIBRARIES_PRIVATE usdImagingGL usdAppUtils hd)
    endif()
endif()

if(TLRENDER_GL AND MRV2_BACKEND STREQUAL "GL")
    list(APPEND LIBRARIES ${FLTK_gl_LIBRARY})
endif()

if(TLRENDER_VK AND MRV2_BACKEND STREQUAL "VK")
    list(APPEND LIBRARIES ${FLTK_vk_LIBRARY} Vulkan::Vulkan)
endif()


target_link_libraries(mrvOptions
    PUBLIC ${LIBRARIES}
    PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvOptions PROPERTIES FOLDER lib)


install(TARGETS mrvOptions
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries )
