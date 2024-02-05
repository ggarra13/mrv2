# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
  mrvEnvironmentMapOptions.h
  mrvEnvironmentMapOptionsInline.h
  mrvFilesPanelOptions.h
  mrvFilesPanelOptionsInline.h
  mrvStereo3DOptions.h
  mrvStereo3DOptionsInline.h
  mrvUSD.h
  )

set(SOURCES
  mrvEnvironmentMapOptions.cpp
  mrvFilesPanelOptions.cpp
  mrvStereo3DOptions.cpp
  mrvUSD.cpp
  mrvUSDInline.cpp
  )

add_library(mrvOptions ${SOURCES} ${HEADERS})

set(LIBRARIES mrvCore )
set(LIBRARIES_PRIVATE )

if(TLRENDER_USD)
    list(APPEND LIBRARIES_PRIVATE tf usd usdGeom usdImagingGL usdAppUtils hd)
endif()

files_to_absolute_paths()

target_link_libraries(mrvOptions
    PUBLIC ${LIBRARIES}
    PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvOptions PROPERTIES FOLDER lib)


install(TARGETS mrvOptions
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries )
