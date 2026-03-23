# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvConvertImage.h
    mrvOperations.h
    mrvPixelConverter.h
)

set(SOURCES
    mrvConvertImage.cpp
    mrvOperations.cpp
)

set(LIBRARIES tlRender::tlCore OpenEXR::OpenEXR Imath::Imath)
set(LIBRARIES_PRIVATE )

add_library(mrvImage ${SOURCES} ${HEADERS})

files_to_absolute_paths()

target_link_libraries(mrvImage PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvImage PROPERTIES FOLDER lib)


install(TARGETS mrvImage
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrvImage
    COMPONENT libraries )
