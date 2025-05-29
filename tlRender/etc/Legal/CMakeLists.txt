set(LEGAL
    LICENSE_BlackmagicDesign.txt
    LICENSE_CMake.txt
    LICENSE_FFmpeg.txt
    LICENSE_FreeType.txt
    LICENSE_GLFW.md
    LICENSE_MaterialX.txt
    LICENSE_OFL.txt
    LICENSE_OpenColorIO.txt
    LICENSE_OpenEXR.txt
    LICENSE_OpenSubdiv.txt
    LICENSE_OpenTimelineIO.txt
    LICENSE_OpenUSD.txt
    LICENSE_Qt.txt
    LICENSE_RtAudio.txt
    LICENSE_boost.txt
    LICENSE_expat.txt
    LICENSE_glad.txt
    LICENSE_libjpeg-turbo.txt
    LICENSE_libjpeg.txt
    LICENSE_libpng.txt
    LICENSE_libsamplerate.txt
    LICENSE_libtiff.txt
    LICENSE_minizip-ng.txt
    LICENSE_nativefiledialog-extended.txt
    LICENSE_nlohmann_json.txt
    LICENSE_oneTBB.txt
    LICENSE_pystring.txt
    LICENSE_stb.txt
    LICENSE_yaml-cpp.txt
    LICENSE_zlib.txt)

install(
    FILES ${LEGAL}
    DESTINATION etc/Legal)
