set(HEADERS
    Cache.h
    Cineon.h
    DPX.h
    IO.h
    IOInline.h
    Init.h
    PPM.h
    Plugin.h
    PluginInline.h
    SGI.h
    SequenceIO.h
    STB.h
    System.h
    SystemInline.h)
set(HEADERS_PRIVATE
    SequenceIOReadPrivate.h)

set(SOURCE
    Cache.cpp
    CineonRead.cpp
    CineonWrite.cpp
    Cineon.cpp
    DPXRead.cpp
    DPXWrite.cpp
    DPX.cpp
    IO.cpp
    Init.cpp
    Normalize.cpp
    PPM.cpp
    PPMRead.cpp
    PPMWrite.cpp
    Plugin.cpp
    SGI.cpp
    SGIRead.cpp
    SGIWrite.cpp
    STB.cpp
    STBRead.cpp
    STBWrite.cpp
    SequenceIORead.cpp
    SequenceIOWrite.cpp
    System.cpp)

set(LIBRARIES)
set(LIBRARIES_PRIVATE)
if(TLRENDER_JPEG)
    list(APPEND HEADERS_PRIVATE JPEG.h)
    list(APPEND SOURCE JPEG.cpp JPEGRead.cpp JPEGWrite.cpp)
    list(APPEND LIBRARIES_PRIVATE libjpeg-turbo::turbojpeg-static)
endif()
if(TLRENDER_TIFF)
    list(APPEND HEADERS_PRIVATE TIFF.h)
    list(APPEND SOURCE TIFF.cpp TIFFRead.cpp TIFFWrite.cpp)
    list(APPEND LIBRARIES_PRIVATE TIFF)
endif()
if(TLRENDER_STB)
    list(APPEND HEADERS_PRIVATE STB.h)
    list(APPEND SOURCE STB.cpp STBRead.cpp STBWrite.cpp)
endif()
if(TLRENDER_PNG)
    list(APPEND HEADERS_PRIVATE PNG.h)
    list(APPEND SOURCE PNG.cpp PNGRead.cpp PNGWrite.cpp)
    list(APPEND LIBRARIES_PRIVATE PNG)
endif()
if(TLRENDER_EXR)
    list(APPEND HEADERS_PRIVATE OpenEXR.h Normalize.h)
    list(APPEND SOURCE OpenEXR.cpp OpenEXRRead.cpp
	               OpenEXRWrite.cpp)
    list(APPEND LIBRARIES_PRIVATE OpenEXR::OpenEXR)
endif()
if(TLRENDER_FFMPEG)
    list(APPEND HEADERS_PRIVATE FFmpeg.h FFmpegMacros.h FFmpegReadPrivate.h)
    list(APPEND SOURCE FFmpeg.cpp FFmpegRead.cpp FFmpegReadAudio.cpp
        FFmpegReadVideo.cpp FFmpegWrite.cpp)
    list(APPEND LIBRARIES_PRIVATE FFmpeg)
endif()
if(TLRENDER_USD)
    list(APPEND HEADERS_PRIVATE USD.h USDPrivate.h)
    list(APPEND SOURCE USD.cpp USDRead.cpp USDRender.cpp)
    list(APPEND LIBRARIES_PRIVATE tf usd usdGeom usdImagingGL usdAppUtils hd)
endif()
if(TLRENDER_RAW)
    list(APPEND HEADERS_PRIVATE RAW.h)
    list(APPEND SOURCE RAW.cpp RAWRead.cpp)
    list(APPEND LIBRARIES_PRIVATE LibRaw)
endif()
if(TLRENDER_NDI AND TLRENDER_FFMPEG)
    list(APPEND HEADERS_PRIVATE NDI.h)
    list(APPEND SOURCE NDI.cpp NDIRead.cpp NDIReadVideo.cpp NDIReadAudio.cpp )
    list(APPEND LIBRARIES_PRIVATE NDI)
endif()
list(APPEND LIBRARIES tlGL tlCore)

add_library(tlIO ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlIO PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlIO PROPERTIES FOLDER lib)
set_target_properties(tlIO PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlIO
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlIO)
