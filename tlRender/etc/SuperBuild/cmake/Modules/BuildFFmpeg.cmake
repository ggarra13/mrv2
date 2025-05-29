include(ExternalProject)

include(ProcessorCount)
ProcessorCount(NPROCS)

if(WIN32)
    # Build FFmpeg with MSYS2 on Windows.
    find_package(Msys REQUIRED)
endif()


set(FFmpeg_DEPS ZLIB)
if(TLRENDER_NET AND NOT WIN32)
    list(APPEND FFmpeg_DEPS OpenSSL)
endif()
if(NOT WIN32)
    list(APPEND FFmpeg_DEPS NASM)
else()
    include(functions/Msys2)
endif()

set(INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
if(WIN32)
    convert_path_for_msys2("${CMAKE_INSTALL_PREFIX}" INSTALL_PREFIX)
endif()

set(FFmpeg_SHARED_LIBS ON)
set(FFmpeg_DEBUG OFF)
set(FFmpeg_LDFLAGS)
    set(FFmpeg_CFLAGS "--extra-cflags=-I${INSTALL_PREFIX}/include")
    set(FFmpeg_CXXFLAGS "--extra-cxxflags=-I${INSTALL_PREFIX}/include")
    set(FFmpeg_OBJCFLAGS "--extra-objcflags=-I${INSTALL_PREFIX}/include")
if(WIN32)
    list(APPEND FFmpeg_LDFLAGS "--extra-ldflags=-LIBPATH:${INSTALL_PREFIX}/lib/")
    list(APPEND FFmpeg_CFLAGS "--extra-cflags='-wd4828 -wd4101 -wd4267 -wd4334 -wd4090'")
    if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
        list(APPEND FFmpeg_CFLAGS "--extra-cflags=-MDd")
        list(APPEND FFmpeg_CXXFLAGS "--extra-cxxflags=-MDd")
    else()
        list(APPEND FFmpeg_CFLAGS "--extra-cflags=-MD")
        list(APPEND FFmpeg_CXXFLAGS "--extra-cxxflags=-MD")
    endif()
elseif(APPLE)
    list(APPEND FFmpeg_LDFLAGS "--extra-ldflags=-L${INSTALL_PREFIX}/lib")
else()
    list(APPEND FFmpeg_LDFLAGS "--extra-ldflags=-L${INSTALL_PREFIX}/lib")
    list(APPEND FFmpeg_LDFLAGS "--extra-ldflags=-L${INSTALL_PREFIX}/lib64")
endif()
if(APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
    list(APPEND FFmpeg_CFLAGS "--extra-cflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    list(APPEND FFmpeg_CXXFLAGS "--extra-cxxflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    list(APPEND FFmpeg_OBJCFLAGS "--extra-objcflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    list(APPEND FFmpeg_LDFLAGS "--extra-ldflags=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()
if(FFmpeg_DEBUG)
    list(APPEND FFmpeg_CFLAGS "--extra-cflags=-g")
    list(APPEND FFmpeg_CXXFLAGS "--extra-cxxflags=-g")
    list(APPEND FFmpeg_OBJCFLAGS "--extra-objcflags=-g")
    list(APPEND FFmpeg_LDFLAGS "--extra-ldflags=-g")
endif()

set(FFmpeg_CONFIGURE_ARGS
    --prefix=${INSTALL_PREFIX}
    --enable-pic
    --disable-programs
    --disable-doc
    --disable-postproc
    --disable-avfilter
    --disable-hwaccels
    --disable-devices
    --disable-filters
    --disable-alsa
    --disable-appkit
    --disable-avfoundation
    --disable-bzlib
    --disable-coreimage
    --disable-iconv
    --disable-libxcb
    --disable-libxcb-shm
    --disable-libxcb-xfixes
    --disable-libxcb-shape
    --disable-lzma
    --disable-metal
    --disable-sndio
    --disable-schannel
    --disable-sdl2
    --disable-securetransport
    --disable-vulkan
    --disable-xlib
    --enable-zlib
    --disable-amf
    --disable-cuda-llvm
    --disable-cuvid
    --disable-d3d11va
    --disable-dxva2
    --disable-ffnvcodec
    --disable-nvdec
    --disable-nvenc
    --disable-v4l2-m2m
    --disable-vaapi
    --disable-vdpau
    --disable-large-tests
    --pkg-config-flags=--static
    ${FFmpeg_CFLAGS}
    ${FFmpeg_CXXFLAGS}
    ${FFmpeg_OBJCFLAGS}
    ${FFmpeg_LDFLAGS})
if(NOT APPLE)
    list(APPEND FFmpeg_CONFIGURE_ARGS
	--disable-videotoolbox
	--disable-audiotoolbox)
else()
    list(APPEND FFmpeg_CONFIGURE_ARGS
	--enable-videotoolbox
	--enable-hwaccel=h264_videotoolbox
	--enable-hwaccel=hevc_videotoolbox
	--enable-hwaccel=prores_videotoolbox
	--enable-hwaccel=vp9_videotoolbox
	--enable-audiotoolbox)
endif()

if(TLRENDER_FFMPEG_MINIMAL)
    list(APPEND FFmpeg_CONFIGURE_ARGS
	--disable-decoders
	    
        --enable-decoder=aac
        --enable-decoder=ac3
        --enable-decoder=av1
        --enable-decoder=cfhd
        --enable-decoder=dnxhd
        --enable-decoder=eac3
        --enable-decoder=flac
        --enable-decoder=gif
        --enable-decoder=h264
        --enable-decoder=hevc
        --enable-decoder=mjpeg
        --enable-decoder=mp3
        --enable-decoder=mpeg2video
        --enable-decoder=mpeg4
        --enable-decoder=opus
        --enable-decoder=pcm_alaw
        --enable-decoder=pcm_bluray
        --enable-decoder=pcm_dvd
        --enable-decoder=pcm_f16le
        --enable-decoder=pcm_f24le
        --enable-decoder=pcm_f32be
        --enable-decoder=pcm_f32le
        --enable-decoder=pcm_f64be
        --enable-decoder=pcm_f64le
        --enable-decoder=pcm_lxf
        --enable-decoder=pcm_mulaw
        --enable-decoder=pcm_s16be
        --enable-decoder=pcm_s16be_planar
        --enable-decoder=pcm_s16le
        --enable-decoder=pcm_s16le_planar
        --enable-decoder=pcm_s24be
        --enable-decoder=pcm_s24daud
        --enable-decoder=pcm_s24le
        --enable-decoder=pcm_s24le_planar
        --enable-decoder=pcm_s32be
        --enable-decoder=pcm_s32le
        --enable-decoder=pcm_s32le_planar
        --enable-decoder=pcm_s64be
        --enable-decoder=pcm_s64le
        --enable-decoder=pcm_s8
        --enable-decoder=pcm_s8_planar
        --enable-decoder=pcm_sga
        --enable-decoder=pcm_u16be
        --enable-decoder=pcm_u16le
        --enable-decoder=pcm_u24be
        --enable-decoder=pcm_u24le
        --enable-decoder=pcm_u32be
        --enable-decoder=pcm_u32le
        --enable-decoder=pcm_u8
        --enable-decoder=pcm_vidc
	--enable-decoder=png
        --enable-decoder=prores
        --enable-decoder=rawvideo
	--enable-decoder=truehd
        --enable-decoder=v210
        --enable-decoder=v210x
        --enable-decoder=v308
        --enable-decoder=v408
        --enable-decoder=v410
        --enable-decoder=vorbis
        --enable-decoder=vp8
        --enable-decoder=vp9
        --enable-decoder=yuv4
        --enable-decoder=wmalossless
        --enable-decoder=wmapro
        --enable-decoder=wmav1
        --enable-decoder=wmav2
        --enable-decoder=wmavoice
        --enable-decoder=wmv1
        --enable-decoder=wmv2
        --enable-decoder=wmv3
        --enable-decoder=wmv3image
	
        --disable-encoders
        --enable-encoder=aac
        --enable-encoder=ac3
        --enable-encoder=cfhd
        --enable-encoder=dnxhd
        --enable-encoder=eac3
        --enable-encoder=gif
        --enable-encoder=mjpeg
        --enable-encoder=mpeg2video
        --enable-encoder=mpeg4
        --enable-encoder=opus
        --enable-encoder=pcm_alaw
        --enable-encoder=pcm_bluray
        --enable-encoder=pcm_dvd
        --enable-encoder=pcm_f32be
        --enable-encoder=pcm_f32le
        --enable-encoder=pcm_f64be
        --enable-encoder=pcm_f64le
        --enable-encoder=pcm_mulaw
        --enable-encoder=pcm_s16be
        --enable-encoder=pcm_s16be_planar
        --enable-encoder=pcm_s16le
        --enable-encoder=pcm_s16le_planar
        --enable-encoder=pcm_s24be
        --enable-encoder=pcm_s24daud
        --enable-encoder=pcm_s24le
        --enable-encoder=pcm_s24le_planar
        --enable-encoder=pcm_s32be
        --enable-encoder=pcm_s32le
        --enable-encoder=pcm_s32le_planar
        --enable-encoder=pcm_s64be
        --enable-encoder=pcm_s64le
        --enable-encoder=pcm_s8
        --enable-encoder=pcm_s8_planar
        --enable-encoder=pcm_u16be
        --enable-encoder=pcm_u16le
        --enable-encoder=pcm_u24be
        --enable-encoder=pcm_u24le
        --enable-encoder=pcm_u32be
        --enable-encoder=pcm_u32le
        --enable-encoder=pcm_u8
        --enable-encoder=pcm_vidc
        --enable-encoder=prores
        --enable-encoder=prores_ks
        --enable-encoder=rawvideo
	--enable-encoder=truehd
        --enable-encoder=v210
        --enable-encoder=v308
        --enable-encoder=v408
        --enable-encoder=v410
        --enable-encoder=yuv4
        --enable-encoder=vorbis
        --enable-encoder=wmav1
        --enable-encoder=wmav2
        --enable-encoder=wmv1
        --enable-encoder=wmv2
	
        --disable-demuxers
        --enable-demuxer=aac
        --enable-demuxer=ac3
        --enable-demuxer=aiff
        --enable-demuxer=asf
        --enable-demuxer=av1
	--enable-demuxer=avi
        --enable-demuxer=dnxhd
        --enable-demuxer=dts
        --enable-demuxer=dtshd
        --enable-demuxer=eac3
        --enable-demuxer=flac
        --enable-demuxer=gif
        --enable-demuxer=h264
        --enable-demuxer=hevc
        --enable-demuxer=m4v
        --enable-demuxer=matroska
        --enable-demuxer=mjpeg
        --enable-demuxer=mov
        --enable-demuxer=mp3
        --enable-demuxer=mxf
        --enable-demuxer=ogg
        --enable-demuxer=pcm_alaw
        --enable-demuxer=pcm_f32be
        --enable-demuxer=pcm_f32le
        --enable-demuxer=pcm_f64be
        --enable-demuxer=pcm_f64le
        --enable-demuxer=pcm_mulaw
        --enable-demuxer=pcm_s16be
        --enable-demuxer=pcm_s16le
        --enable-demuxer=pcm_s24be
        --enable-demuxer=pcm_s24le
        --enable-demuxer=pcm_s32be
        --enable-demuxer=pcm_s32le
        --enable-demuxer=pcm_s8
        --enable-demuxer=pcm_u16be
        --enable-demuxer=pcm_u16le
        --enable-demuxer=pcm_u24be
        --enable-demuxer=pcm_u24le
        --enable-demuxer=pcm_u32be
        --enable-demuxer=pcm_u32le
        --enable-demuxer=pcm_u8
        --enable-demuxer=pcm_vidc
        --enable-demuxer=rawvideo
        --enable-demuxer=truehd
        --enable-demuxer=v210
        --enable-demuxer=v210x
        --enable-demuxer=wav
        --enable-demuxer=yuv4mpegpipe

        --disable-muxers
        --enable-muxer=ac3
        --enable-muxer=aiff
        --enable-muxer=asf
	--enable-muxer=avi
        --enable-muxer=dnxhd
        --enable-muxer=dts
        --enable-muxer=eac3
        --enable-muxer=flac
        --enable-muxer=gif
        --enable-muxer=h264
        --enable-muxer=hevc
        --enable-muxer=m4v
        --enable-muxer=matroska
        --enable-muxer=mjpeg
        --enable-muxer=mov
        --enable-muxer=mp4
        --enable-muxer=mpeg2video
        --enable-muxer=mxf
        --enable-muxer=ogg
        --enable-muxer=opus
        --enable-muxer=pcm_alaw
        --enable-muxer=pcm_f32be
        --enable-muxer=pcm_f32le
        --enable-muxer=pcm_f64be
        --enable-muxer=pcm_f64le
        --enable-muxer=pcm_mulaw
        --enable-muxer=pcm_s16be
        --enable-muxer=pcm_s16le
        --enable-muxer=pcm_s24be
        --enable-muxer=pcm_s24le
        --enable-muxer=pcm_s32be
        --enable-muxer=pcm_s32le
        --enable-muxer=pcm_s8
        --enable-muxer=pcm_u16be
        --enable-muxer=pcm_u16le
        --enable-muxer=pcm_u24be
        --enable-muxer=pcm_u24le
        --enable-muxer=pcm_u32be
        --enable-muxer=pcm_u32le
        --enable-muxer=pcm_u8
        --enable-muxer=pcm_vidc
        --enable-muxer=rawvideo
        --enable-muxer=truehd
        --enable-muxer=wav
	--enable-muxer=webm
        --enable-muxer=yuv4mpegpipe

        --disable-parsers
        --enable-parser=aac
        --enable-parser=ac3
        --enable-parser=av1
        --enable-parser=dnxhd
        --enable-parser=dolby_e
        --enable-parser=flac
        --enable-parser=gif
        --enable-parser=h264
        --enable-parser=hevc
        --enable-parser=mjpeg
        --enable-parser=mpeg4video
        --enable-parser=mpegaudio
        --enable-parser=mpegvideo
        --enable-parser=opus
        --enable-parser=vorbis
        --enable-parser=vp8
        --enable-parser=vp9
	
        --disable-protocols
        --enable-protocol=crypto
        --enable-protocol=file
        --enable-protocol=ftp
        --enable-protocol=http
        --enable-protocol=httpproxy
        --enable-protocol=https
        --enable-protocol=md5)
    if(APPLE)
	list(APPEND FFmpeg_CONFIGURE_ARGS
	    --enable-encoder=pcm_alaw_at
	    --enable-decoder=pcm_alaw_at
	    --enable-encoder=pcm_mulaw_at
	    --enable-decoder=pcm_mulaw_at)
    endif()
endif()

if(TLRENDER_VPX)
    list(APPEND FFmpeg_DEPS VPX)
    list(APPEND FFmpeg_CONFIGURE_ARGS
        --enable-decoder=libvpx_vp8
        --enable-decoder=libvpx_vp9
        --enable-encoder=libvpx_vp8
        --enable-encoder=libvpx_vp9
	--enable-libvpx)
    if(WIN32)
	list(APPEND FFmpeg_CONFIGURE_ARGS
            --extra-libs=vpx.lib
	    --extra-libs=kernel32.lib
	    --extra-libs=user32.lib
	    --extra-libs=gdi32.lib
	    --extra-libs=winspool.lib
	    --extra-libs=shell32.lib
	    --extra-libs=ole32.lib
	    --extra-libs=oleaut32.lib
	    --extra-libs=uuid.lib
	    --extra-libs=comdlg32.lib
	    --extra-libs=advapi32.lib)
    else()
	list(APPEND FFmpeg_CONFIGURE_ARGS
	    --extra-ldflags="${INSTALL_PREFIX}/lib/libvpx.a")
    endif()
endif()
if(TLRENDER_AV1)
    list(APPEND FFmpeg_DEPS dav1d)
    list(APPEND FFmpeg_CONFIGURE_ARGS
	--enable-libdav1d
	--enable-decoder=libdav1d)
    if(UNIX)
	list(APPEND FFmpeg_CONFIGURE_ARGS
	    --extra-ldflags="${INSTALL_PREFIX}/lib/libdav1d.a")
	if (NOT APPLE)
	    list(APPEND FFmpeg_CONFIGURE_ARGS
		--extra-libs=-lm
		--extra-libs=-lpthread)
	endif()
    endif()
endif()
if(TLRENDER_HAP)
    list(APPEND FFmpeg_CONFIGURE_ARGS
        --enable-encoder=hap
        --enable-decoder=hap
	--enable-libsnappy)
    list(APPEND FFmpeg_DEPS libsnappy)
endif()
if(TLRENDER_X264)
    list(APPEND FFmpeg_CONFIGURE_ARGS
        --enable-encoder=libx264
        --enable-decoder=libx264
	--enable-libx264
	--enable-gpl)
    if(TLRENDER_NET)
	list(APPEND FFmpeg_CONFIGURE_ARGS
	    --enable-version3)
    endif()
    # if(UNIX)
    # 	list(APPEND FFmpeg_CONFIGURE_ARGS
    # 	    --extra-ldflags="${INSTALL_PREFIX}/lib/libx264.a")
    # 	list(APPEND FFmpeg_DEPS X264)
    # endif()
endif()

if(TLRENDER_SVTAV1)
    list(APPEND FFmpeg_DEPS SvtAV1)
    list(APPEND FFmpeg_CONFIGURE_ARGS
	--enable-libsvtav1
        --enable-encoder=libsvtav1)
    if(UNIX)
	list(APPEND FFmpeg_CONFIGURE_ARGS
	    --extra-ldflags="${INSTALL_PREFIX}/lib/libSvtAv1Enc.a")
	if (NOT APPLE)
	    list(APPEND FFmpeg_CONFIGURE_ARGS
		--extra-libs=-lm
		--extra-libs=-lpthread)
	endif()
    endif()
endif()

if(NOT WIN32)
    list(APPEND FFmpeg_CONFIGURE_ARGS
	--x86asmexe=${CMAKE_INSTALL_PREFIX}/bin/nasm)
endif()
if(TLRENDER_NET)
    list(APPEND FFmpeg_CONFIGURE_ARGS
        --enable-openssl)
    if(WIN32)
	list(APPEND FFmpeg_CONFIGURE_ARGS
            --extra-libs=crypto.lib --enable-version3)
    endif()
endif()
if(FFmpeg_SHARED_LIBS)
    list(APPEND FFmpeg_CONFIGURE_ARGS
        --disable-static
        --enable-shared)
endif()
if(FFmpeg_DEBUG)
    list(APPEND FFmpeg_CONFIGURE_ARGS
        --disable-optimizations
        --disable-stripping
        --enable-debug=3
        --assert-level=2)
endif()
if(WIN32)
    list(APPEND FFmpeg_CONFIGURE_ARGS
        --arch=x86_64
        --toolchain=msvc)
    set(FFmpeg_MSYS2
        ${MSYS_CMD}
        -use-full-path
        -defterm
        -no-start
        -here)

    # \bug Copy libssl.lib to ssl.lib and libcrypto.lib to crypto.lib so the
    # FFmpeg configure script can find them.
    # Not done here, but in pre-flight script of mrv2
    set(FFmpeg_OPENSSL_COPY)

    if(WIN32)
	convert_path_for_msys2("${CMAKE_CURRENT_BINARY_DIR}" BINARY_DIR)
        set(FFmpeg_OPENSSL_COPY
            "cp ${BINARY_DIR}/ffmpeg_configure.sh.in ${BINARY_DIR}/FFmpeg/src/FFmpeg/ffmpeg_configure.sh &&")
    endif()

    set(PKG_CONFIG_PATH_MSys2 "${INSTALL_PREFIX}/lib/pkgconfig")
    
    # Ensure PKG_CONFIG_PATH is set within the MSYS2 shell command
    set(PKG_CONFIG_PATH_CMD "export PKG_CONFIG_PATH=${PKG_CONFIG_PATH_MSys2}:\$PKG_CONFIG_PATH &&")
    
    list(JOIN FFmpeg_CONFIGURE_ARGS " \\\n" FFmpeg_CONFIGURE_ARGS_TMP)

    set(FFmpeg_CONFIGURE ${FFmpeg_MSYS2}
        -c "pacman -S diffutils make nasm pkg-config --noconfirm && \
        ${FFmpeg_OPENSSL_COPY} ${PKG_CONFIG_PATH_CMD} \
        ./ffmpeg_configure.sh")
    set(FFmpeg_BUILD ${FFmpeg_MSYS2} -c "make -j ${NPROCS}")
    set(FFmpeg_INSTALL ${FFmpeg_MSYS2} -c "make install"
        COMMAND ${FFmpeg_MSYS2} -c "mv ${INSTALL_PREFIX}/bin/avcodec.lib ${INSTALL_PREFIX}/lib"
        COMMAND ${FFmpeg_MSYS2} -c "mv ${INSTALL_PREFIX}/bin/avdevice.lib ${INSTALL_PREFIX}/lib"
        COMMAND ${FFmpeg_MSYS2} -c "mv ${INSTALL_PREFIX}/bin/avformat.lib ${INSTALL_PREFIX}/lib"
        COMMAND ${FFmpeg_MSYS2} -c "mv ${INSTALL_PREFIX}/bin/avutil.lib ${INSTALL_PREFIX}/lib"
        COMMAND ${FFmpeg_MSYS2} -c "mv ${INSTALL_PREFIX}/bin/swresample.lib ${INSTALL_PREFIX}/lib"
        COMMAND ${FFmpeg_MSYS2} -c "mv ${INSTALL_PREFIX}/bin/swscale.lib ${INSTALL_PREFIX}/lib")
else()
    set(FFmpeg_CONFIGURE ./configure ${FFmpeg_CONFIGURE_ARGS})
    set(FFmpeg_BUILD make -j ${NPROCS})
    set(FFmpeg_INSTALL make install)
endif()

if (WIN32)
    set(FFmpeg_CONFIGURE_CONTENTS "#!/usr/bin/env bash\n./configure ${FFmpeg_CONFIGURE_ARGS_TMP}\n")
    # Ensure the directory exists
    message(STATUS "Creating directory ${CMAKE_CURRENT_BINARY_DIR}/")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/")
    message(STATUS "Creating ffmpeg_configure.sh.in ${CMAKE_CURRENT_BINARY_DIR}/")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg_configure.sh.in
	${FFmpeg_CONFIGURE_CONTENTS}
    )
endif()

ExternalProject_Add(
    FFmpeg
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/FFmpeg
    DEPENDS ${FFmpeg_DEPS}
    URL https://ffmpeg.org/releases/ffmpeg-7.0.1.tar.bz2
    CONFIGURE_COMMAND ${FFmpeg_CONFIGURE}
    BUILD_COMMAND ${FFmpeg_BUILD}
    INSTALL_COMMAND ${FFmpeg_INSTALL}
    BUILD_IN_SOURCE 1)


