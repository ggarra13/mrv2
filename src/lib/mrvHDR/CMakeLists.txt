# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvNDIView.h
)

set(SOURCES
    mrvNDIView.cpp
)

find_package(Intl REQUIRED)
find_package(Vulkan REQUIRED)
find_package(libplacebo REQUIRED)
find_package(NDI REQUIRED)

set(LIBRARIES ${FLTK_LIBRARIES}
    Vulkan::Vulkan
    libplacebo
    NDI
    ${Intl_LIBRARIES}
)


add_library(mrvHDR ${SOURCES} ${HEADERS})

files_to_absolute_paths()
target_link_libraries(mrvHDR PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvHDR PROPERTIES FOLDER lib)


install(TARGETS mrvHDR
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries )
