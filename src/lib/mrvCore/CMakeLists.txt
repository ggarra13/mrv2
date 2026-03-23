# SPDX-License-Identifier: BSD-3-Clause
# mrv2
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    mrvActionMode.h
    mrvBackend.h
    mrvColor.h
    mrvColorAreaInfo.h
    mrvColorSpaces.h
    mrvFile.h
    mrvFonts.h
    mrvGPU.h
    mrvHome.h
    mrvHotkey.h
    mrvLicensing.h
    mrvLocale.h
    mrvMedia.h
    mrvMesh.h
    mrvOrderedMap.h
    mrvPathMapping.h
    mrvSequence.h
    mrvTimeObject.h
    mrvUtil.h
)

set(SOURCES
    mrvColor.cpp
    mrvColorSpaces.cpp
    mrvFile.cpp
    mrvFonts.cpp
    mrvGPU.cpp
    mrvHome.cpp
    mrvHotkey.cpp
    mrvLicensing.cpp
    mrvLocale.cpp
    mrvMedia.cpp
    mrvMesh.cpp
    mrvPathMapping.cpp
    mrvTimeObject.cpp
    mrvUtil.cpp
)

if (MRV2_NETWORK)
    find_package(Poco REQUIRED Net NetSSL JSON Foundation)
endif()
if (TLRENDER_NET)
    find_package(OpenSSL REQUIRED)
endif()

set(LIBRARIES mrvOptions)
set(LIBRARIES tlRender::tlIO tlRender::tlCore OpenEXR::OpenEXR Imath::Imath mrvOS ${FLTK_LIBRARIES})

if (MRV2_NETWORK)
    set(LIBRARIES_PRIVATE
	Poco::Net
	Poco::NetSSL
	Poco::JSON
	Poco::Foundation
    )
endif()
if (TLRENDER_NET)
    list(APPEND LIBRARIES_PRIVATE
	OpenSSL::SSL
	OpenSSL::Crypto)
endif()

add_library(mrvCore ${SOURCES} ${HEADERS})

files_to_absolute_paths()

if (WIN32)
   target_compile_definitions(mrvCore PUBLIC -DFL_DLL)
 endif()
target_link_libraries(mrvCore PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE} )
set_target_properties(mrvCore PROPERTIES FOLDER lib)


install(TARGETS mrvCore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrvCore
    COMPONENT libraries )
