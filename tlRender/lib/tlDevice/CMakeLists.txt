set(HEADERS
    Init.h)
set(HEADERS_PRIVATE)

set(SOURCE
    Init.cpp)

set(INCLUDE_DIRS)
if (MRV2_BACKEND STREQUAL "VK")
    set(LIBRARIES tlTimelineVk)
else()
    set(LIBRARIES tlTimelineGL)
endif()
set(LIBRARIES_PRIVATE)

if(TLRENDER_NDI)
    list(APPEND HEADERS
	DevicesModel.h
	DeviceSystem.h
	IOutput.h
	OutputData.h)
    list(APPEND SOURCE
	DevicesModel.cpp
	DeviceSystem.cpp
	IOutput.cpp
	OutputData.cpp)
endif()

if (MRV2_BACKEND STREQUAL "GL")
    list(APPEND HEADERS
	GLUtil.h)
    list(APPEND SOURCE
	GLUtil.cpp)
endif()

if(TLRENDER_NDI AND TLRENDER_FFMPEG)
    list(APPEND HEADERS
	NDI/NDIOutputDevice.h
	NDI/NDISystem.h
	NDI/NDIUtil.h)
    list(APPEND SOURCE
	NDI/NDIOutputDevice.cpp
	NDI/NDISystem.cpp
	NDI/NDIUtil.cpp)
    list(APPEND LIBRARIES_PRIVATE NDI)
endif()

add_library(tlDevice ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_include_directories(tlDevice PUBLIC ${INCLUDE_DIRS})
target_link_libraries(tlDevice PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlDevice PROPERTIES FOLDER lib)
set_target_properties(tlDevice PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlDevice
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlDevice)
