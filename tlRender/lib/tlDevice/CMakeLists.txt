set(HEADERS
    Init.h)
set(HEADERS_PRIVATE)

set(SOURCE
    Init.cpp)

set(INCLUDE_DIRS)
set(LIBRARIES tlTimelineGL)
set(LIBRARIES_PRIVATE)

if(TLRENDER_BMD OR TLRENDER_NDI)
    set(HEADERS
	DevicesModel.h
	DeviceSystem.h
	GLUtil.h
	Init.h
	IOutput.h
	OutputData.h)
    set(SOURCE
	DevicesModel.cpp
	DeviceSystem.cpp
	GLUtil.cpp
	Init.cpp
	IOutput.cpp
	OutputData.cpp
	)
endif()

if(TLRENDER_NDI AND TLRENDER_FFMPEG)
    list(APPEND HEADERS
	NDI/NDIOptions.h
	NDI/NDIOutputDevice.h
	NDI/NDISystem.h
	NDI/NDIUtil.h)
    list(APPEND SOURCE
	NDI/NDIOptions.cpp
	NDI/NDIOutputDevice.cpp
	NDI/NDISystem.cpp
	NDI/NDIUtil.cpp)
    list(APPEND LIBRARIES_PRIVATE NDI)
endif()

if(TLRENDER_BMD)
    list(APPEND HEADERS
        BMD/BMDOutputDevice.h
        BMD/BMDUtil.h)
    list(APPEND HEADERS_PRIVATE
        BMD/BMDOutputPrivate.h)
    list(APPEND SOURCE
        BMD/BMDOutputCallback.cpp
        BMD/BMDHDRVideoFrame.cpp
        BMD/BMDOutputDevice.cpp
        BMD/BMDUtil.cpp)

    if(WIN32)
        list(APPEND HEADERS_PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI.h
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/Win/platform.h)
        list(APPEND SOURCE
            ${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI_i.c
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/Win/platform.cpp)
        list(APPEND INCLUDE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/Win
            ${CMAKE_CURRENT_BINARY_DIR})
        list(APPEND LIBRARIES_PRIVATE comsuppw.lib)

        set(MIDL_OUTPUT_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI_i.c")
        set(MIDL_OUTPUT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI.h")
        set(MIDL_OUTPUT_TLB "${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI.tlb")
        set(MIDL_OUTPUT
            ${MIDL_OUTPUT_SOURCE}
            ${MIDL_OUTPUT_HEADER}
            ${MIDL_OUTPUT_TLB})
        set(DECKLINK_API_IDL "${TLRENDER_BMD_SDK}/Win/include/DeckLinkAPI.idl")
        set(MIDL_MACHINE amd64)
        add_custom_command(
            OUTPUT ${MIDL_OUTPUT}
            DEPENDS ${DECKLINK_API_IDL}
            COMMAND midl /env ${MIDL_MACHINE} /header ${MIDL_OUTPUT_HEADER} ${DECKLINK_API_IDL} /tlb ${MIDL_OUTPUT_TLB}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
        set_source_files_properties(${MIDL_OUTPUT} PROPERTIES GENERATED TRUE)
        add_custom_target(MIDL_Target DEPENDS ${MIDL_OUTPUT})
    elseif(APPLE)
        list(APPEND HEADERS_PRIVATE
            ${TLRENDER_BMD_SDK}/Mac/include/DeckLinkAPI.h
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/macOS/platform.h)
        list(APPEND SOURCE
            ${TLRENDER_BMD_SDK}/Mac/include/DeckLinkAPIDispatch.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/macOS/platform.cpp)
        list(APPEND INCLUDE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/macOS
            ${TLRENDER_BMD_SDK}/Mac/include)
    elseif(UNIX)
        list(APPEND HEADERS_PRIVATE
            ${TLRENDER_BMD_SDK}/Linux/include/DeckLinkAPI.h
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/Linux/platform.h)
        list(APPEND SOURCE
            ${TLRENDER_BMD_SDK}/Linux/include/DeckLinkAPIDispatch.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/Linux/platform.cpp)
        list(APPEND INCLUDE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}/BMD/Linux
            ${TLRENDER_BMD_SDK}/Linux/include)
    endif()
endif()

add_library(tlDevice ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_include_directories(tlDevice PUBLIC ${INCLUDE_DIRS})
target_link_libraries(tlDevice PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlDevice PROPERTIES FOLDER lib)
set_target_properties(tlDevice PROPERTIES PUBLIC_HEADER "${HEADERS}")
if(TLRENDER_BMD)
    if(WIN32)
        add_dependencies(tlDevice MIDL_Target)
    endif()
endif()

install(TARGETS tlDevice
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlDevice)
