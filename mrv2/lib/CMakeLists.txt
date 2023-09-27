# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

add_subdirectory(mrvCore)
add_subdirectory(mrvDraw)
add_subdirectory(mrvFLU)
add_subdirectory(mrvFlmm)
add_subdirectory(mrvWidgets)
add_subdirectory(mrvUI)
if(TLRENDER_GL)
    add_subdirectory(mrvGL)
endif()
if(MRV2_PDF)
    add_subdirectory(mrvPDF)
endif()
add_subdirectory(mrvFl)
if(MRV2_NETWORK)
    add_subdirectory(mrvNetwork)
endif()
add_subdirectory(mrvEdit)
add_subdirectory(mrvPanels)
add_subdirectory(mrvApp)
if(MRV2_PYBIND11)
    add_subdirectory(mrvPy)
endif()

set( PO_SOURCES ${PO_SOURCES} PARENT_SCOPE)
set( PO_ABS_SOURCES ${PO_ABS_SOURCES} PARENT_SCOPE)
