# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvPDFCreator.h
    mrvSavePDF.h
)

set(SOURCES
    mrvPDFCreator.cpp
    mrvSavePDF.cpp
)


add_library(mrvPDF ${SOURCES} ${HEADERS})
files_to_absolute_paths()

set( LIBRARIES mrvWidgets )

target_link_libraries(mrvPDF PUBLIC ${LIBRARIES} )
set_target_properties(mrvPDF PROPERTIES FOLDER lib)

install(TARGETS mrvPDF
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    COMPONENT libraries)
