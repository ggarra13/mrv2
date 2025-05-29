set(HEADERS
    CineonTest.h
    DPXTest.h
    IOTest.h
    PPMTest.h
    SGITest.h
    STBTest.h)

set(SOURCE
    CineonTest.cpp
    DPXTest.cpp
    IOTest.cpp
    PPMTest.cpp
    SGITest.cpp
    STBTest.cpp)

if(TLRENDER_FFMPEG)
    list(APPEND HEADERS FFmpegTest.h)
    list(APPEND SOURCE FFmpegTest.cpp)
endif()
if(TLRENDER_JPEG)
    list(APPEND HEADERS JPEGTest.h)
    list(APPEND SOURCE JPEGTest.cpp)
endif()
if(TLRENDER_PNG)
    list(APPEND HEADERS PNGTest.h)
    list(APPEND SOURCE PNGTest.cpp)
endif()
if(TLRENDER_EXR)
    list(APPEND HEADERS OpenEXRTest.h)
    list(APPEND SOURCE OpenEXRTest.cpp)
endif()
if(TLRENDER_TIFF)
    list(APPEND HEADERS TIFFTest.h)
    list(APPEND SOURCE TIFFTest.cpp)
endif()
if(TLRENDER_STB)
    list(APPEND HEADERS STBTest.h)
    list(APPEND SOURCE STBTest.cpp)
endif()

add_library(tlIOTest ${SOURCE} ${HEADERS})
target_link_libraries(tlIOTest tlTestLib tlIO)
set_target_properties(tlIOTest PROPERTIES FOLDER tests)
