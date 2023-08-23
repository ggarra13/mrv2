// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef __linux__

#    include <sys/types.h>
#    include <sys/sysinfo.h>
#    include <stdlib.h>
#    include <stdio.h>
#    include <string.h>

#    include <FL/platform.H>
#    undef None

#endif

#ifdef __APPLE__

#    define GL_SILENCE_DEPRECATION 1

#    include <sys/sysctl.h>
#    include <sys/param.h>
#    include <sys/mount.h>
#    include <mach/mach.h>
#    include <OpenGL/OpenGL.h>

#endif

#ifdef FLTK_USE_WAYLAND
#    include <wayland-client.h>
#    include <wayland-server.h>
#    include <wayland-client-protocol.h>
#    include <wayland-egl.h> // Wayland EGL MUST be included before EGL headers
#    include <EGL/egl.h>
#    include <EGL/eglplatform.h>
#endif

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include <expat.h>
#include <Imath/ImathConfig.h>
#include <hpdf_version.h>
#include <jconfig.h>
#include <libpng16/png.h>
#include <mz.h>
#include <stb/stb_image.h>

#ifdef TLRENDER_USD
#    include <boost/version.hpp>
#    include <tbb/tbb_stddef.h>
#    include <MaterialXCore/Util.h>
#endif

#ifdef TLRENDER_GL
#    include <tlGL/Init.h>
#endif

#include <nlohmann/json.hpp>
#include <Poco/Version.h>
#include <pybind11/pybind11.h>
#include <opentime/version.h>
#include <opentimelineio/version.h>

#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;

#include <rtaudio/RtAudio.h>
#include <tiffvers.h>
#include <zlib.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/version.h>
#include <libswscale/version.h>
#include <libswresample/version.h>
}

#ifdef _WIN32
#    pragma warning(disable : 4275)
#endif

#include "mrvCore/mrvOS.h"
#undef snprintf

#include "mrvCore/mrvCPU.h"

#include "mrvWidgets/mrvVersion.h"

#include "mrvGL/mrvGLErrors.h" // defines glGetString and GL_VERSION

#include "mrViewer.h"
#undef snprintf

#ifdef _WIN32
#    include <windows.h>
#    include <psapi.h>
#endif

#include "mrvFl/mrvIO.h"

namespace mrv
{
    namespace
    {
        void
        semantic_versioning(std::ostringstream& o, const unsigned version_hex)
        {
            unsigned int major = (version_hex >> 24) & 0xFF;
            unsigned int minor = (version_hex >> 16) & 0xFF;
            unsigned int micro = (version_hex >> 8) & 0xFF;
            unsigned int release_level = version_hex & 0xFF;

            o << major << "." << minor << "." << micro << " ";

            if (release_level == 0x00)
                o << "Release level: Final";
            else if (release_level & 0xAF)
                o << "Release level: Alpha";
            else if (release_level & 0xBF)
                o << "Release level: Beta";
            else if (release_level & 0xDF)
                o << "Release level: Development";
            else
                o << "Release level: Unknown";
            o << " (" << std::hex << release_level << ")" << std::dec;
        }
    } // namespace

    static const char* kVersion = MRV2_VERSION;
    static const char* kBuild = "- Built " __DATE__ " " __TIME__;

#if INTPTR_MAX == INT64_MAX
    static const char* kArch = "64";
#elif INTPTR_MAX == INT32_MAX
    static const char* kArch = "32";
#else
#    error Unknown pointer size or missing size macros!
#endif

    struct FormatInfo
    {
        bool encode;
        bool decode;
        bool blob;
        std::string name;
        std::string module;
        std::string description;

        FormatInfo(
            bool dec, bool enc, bool blo, const char* n, const char* mod,
            const char* desc) :
            encode(enc),
            decode(dec),
            blob(blo),
            name(n),
            module(mod),
            description(desc)
        {
        }

        bool operator<(const FormatInfo& b) const
        {
            return (strcasecmp(name.c_str(), b.name.c_str()) < 0);
        }
    };

    struct SortFormatsFunctor
    {
        bool operator()(const FormatInfo* a, const FormatInfo* b) const
        {
            return *a < *b;
        }
    };

    typedef std::vector< FormatInfo* > FormatList;

    const char* version()
    {
        return kVersion;
    }

    const char* build_date()
    {
        return kBuild;
    }

    void ffmpeg_formats(mrv::Browser& browser)
    {
        using namespace std;

        const AVInputFormat* ifmt = NULL;
        const AVOutputFormat* ofmt = NULL;
        const char* last_name = NULL;

        FormatList formats;
        FormatInfo* f = NULL;

        f = new FormatInfo(true, true, false, "EXR", "tlRender", "ILM OpenEXR");
        formats.push_back(f);

        f = new FormatInfo(true, true, false, "JPEG", "tlRender", "JPEG");
        formats.push_back(f);

        f = new FormatInfo(true, true, false, "PNG", "tlRender", "PNG");
        formats.push_back(f);

        f = new FormatInfo(true, true, false, "TIFF", "tlRender", "TIFF");
        formats.push_back(f);

        f = new FormatInfo(true, true, false, "Cineon", "tlRender", "Cineon");
        formats.push_back(f);

        last_name = "000";
        for (;;)
        {
            bool decode = false;
            bool encode = false;
            const char* name = NULL;
            const char* long_name = NULL;

            void* opaque = NULL;
            while ((ofmt = av_muxer_iterate(&opaque)))
            {
                if ((name == NULL || strcmp(ofmt->name, name) < 0) &&
                    strcmp(ofmt->name, last_name) > 0)
                {
                    name = ofmt->name;
                    long_name = ofmt->long_name;
                    encode = true;
                }
            }
            opaque = NULL;
            while ((ifmt = av_demuxer_iterate(&opaque)))
            {
                if ((name == NULL || strcmp(ifmt->name, name) < 0) &&
                    strcmp(ifmt->name, last_name) > 0)
                {
                    name = ifmt->name;
                    long_name = ifmt->long_name;
                    encode = true;
                }
                if (name && strcmp(ifmt->name, name) == 0)
                    decode = true;
            }

            if (name == NULL)
                break;
            last_name = name;

            f = new FormatInfo(
                decode, encode, false, name, "FFMPEG", long_name);
            formats.push_back(f);
        }

        // Sort formats alphabetically
        std::sort(formats.begin(), formats.end(), SortFormatsFunctor());

        // Now concatenate all the stuff into a string
        {
            FormatList::const_iterator i = formats.begin();
            FormatList::const_iterator e = formats.end();
            for (; i != e; ++i)
            {
                f = *i;
                std::ostringstream o;
                o << (f->decode ? "D\t" : " \t") << (f->encode ? "E\t" : " \t")
                  << (f->blob ? "B\t" : " \t") << f->name << "\t" << f->module
                  << "\t" << f->description;
                browser.add(o.str().c_str());
                delete f;
            }
        }
    }

    static void ffmpeg_codecs(mrv::Browser& browser, int type)
    {
        using namespace std;

        const AVCodec* p;
        const AVCodec* p2;
        const char* last_name;

        std::ostringstream o;
        last_name = "000";
        for (;;)
        {
            int decode = 0;
            int encode = 0;
            int cap = 0;

            p2 = NULL;
            void* opaque = NULL;
            while ((p = av_codec_iterate(&opaque)))
            {
                if ((p2 == NULL || strcmp(p->name, p2->name) < 0) &&
                    strcmp(p->name, last_name) > 0)
                {
                    p2 = p;
                    decode = encode = cap = 0;
                }
                if (p2 && strcmp(p->name, p2->name) == 0)
                {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 33, 100)
                    encode = av_codec_is_encoder(p);
                    decode = av_codec_is_decoder(p);
#else
                    if (p->decode)
                        decode = 1;
                    if (p->encode2)
                        encode = 1;
#endif
                    cap |= p->capabilities;
                }
            }

            if (p2 == NULL)
                break;
            last_name = p2->name;

            if (p2->type != type)
                continue;

            std::ostringstream o;
            o << (decode ? "D\t" : " \t") << (encode ? "E\t" : " \t") << "\t"
              << (cap & AV_CODEC_CAP_DRAW_HORIZ_BAND ? "S\t" : " \t")
              << (cap & AV_CODEC_CAP_DR1 ? "D\t" : " \t") << p2->name;

            browser.add(o.str().c_str());

            /* if(p2->decoder && decode==0)
               printf(" use %s for decoding", p2->decoder->name);*/
        }
    }

    void ffmpeg_audio_codecs(mrv::Browser& browser)
    {
        return ffmpeg_codecs(browser, AVMEDIA_TYPE_AUDIO);
    }

    void ffmpeg_video_codecs(mrv::Browser& browser)
    {
        return ffmpeg_codecs(browser, AVMEDIA_TYPE_VIDEO);
    }

    void ffmpeg_subtitle_codecs(mrv::Browser& browser)
    {
        return ffmpeg_codecs(browser, AVMEDIA_TYPE_SUBTITLE);
    }

    std::string ffmpeg_protocols()
    {
        std::ostringstream o;
#if LIBAVUTIL_VERSION_MAJOR > 50
        void* opaque = NULL;
        const char* up;
        for (up = avio_enum_protocols(&opaque, 0); up;
             up = avio_enum_protocols(&opaque, 0))
        {
            o << " " << up << ":";
        }
#else
        URLProtocol* up;
        for (up = av_protocol_next(NULL); up; up = av_protocol_next(up))
        {
            o << " " << up->name << ":";
        }
#endif
        return o.str();
    }

    void ffmpeg_motion_estimation_methods(mrv::Browser* b)
    {
        static const char* motion_str[] = {
            "zero", "esa",   "tss",  "tdls", "ntss", "fss",
            "ds",   "hexds", "epzs", "umh",  NULL,
        };

        const char** pp = motion_str;
        while (*pp)
        {
            b->add(*pp);
            pp++;
        }
    }

    std::string about_message()
    {
        using namespace std;

        avformat_network_init();

        std::ostringstream o;

        o << "mrv2 " << kArch << " bits - v" << kVersion << " " << kBuild
          << endl
#ifdef __GLIBCXX__
          << "With gcc " << __GLIBCXX__ << endl
#elif __clang__
          << "With clang " << __clang__ << " " << __llvm__ << endl
#else
          << "With msvc " << _MSC_VER << endl
#endif
          << "(C) 2022-present Film Aura, LLC." << endl
          << "Gonzalo Garramuño & others" << endl
          << endl
          << "mrv2 depends on:" << endl
          << endl;

        const auto expat = XML_ExpatVersionInfo();
#ifdef TLRENDER_USD
        unsigned int boost_major = BOOST_VERSION / 100000;
        unsigned int boost_minor = BOOST_VERSION / 100 % 1000;
        unsigned int boost_patch = BOOST_VERSION % 100;
        o << "Boost v" << boost_major << "." << boost_minor << "."
          << boost_patch << endl
          << "Copyright (c) 2016-present Contributors to the Boost Project"
          << endl
          << endl;
#endif
        o << "expat v" << expat.major << "." << expat.minor << "."
          << expat.micro << endl
          << "Copyright (c) 1998-2000 Thai Open Source Software Center Ltd and "
             "Clark Cooper"
          << endl
          << "Copyright (c) 2001-2022 Expat maintainers" << endl
          << endl
          << "FFmpeg" << endl
          << "libavutil          v" << AV_STRINGIFY(LIBAVUTIL_VERSION) << endl
          << "libavcodec      v" << AV_STRINGIFY(LIBAVCODEC_VERSION) << endl
          << "libavformat     v" << AV_STRINGIFY(LIBAVFORMAT_VERSION) << endl
          << "libavfilter        v" << AV_STRINGIFY(LIBAVFILTER_VERSION) << endl
          << "libswresample v" << AV_STRINGIFY(LIBSWRESAMPLE_VERSION) << endl
          << "libswscale       v" << AV_STRINGIFY(LIBSWSCALE_VERSION) << endl
          << "http://ffmpeg.mplayerhq.hu/" << endl
          << "License: " << avcodec_license() << endl
          << "Copyright (c) 2000-present Fabrice Bellard, et al." << endl
          << "Configuration: " << avcodec_configuration() << endl
          << endl
          << "FLTK v1.4" << endl
          << "http://www.fltk.org/" << endl
          << "Copyright (c) 2000-present Bill Spitzak & others" << endl
          << endl
          << "Modified FLU - FLTK Utility Widgets" << endl
          << "Copyright (c) 2002 Ohio Supercomputer Center, Ohio State "
             "University"
          << endl
          << endl
          << "FreeType" << endl
          << "Copyright (c) 1996-2002, 2006 by David Turner, Robert Wilhelm, "
             "and "
             "Werner Lemberg"
          << endl
          << endl
          << "glad v" << GLAD_GENERATOR_VERSION << endl
          << "Copyright (c) 2013-2020 David Herberth" << endl
          << endl
          << "GLM" << endl
          << "Copyright (c) 2005 - G-Truc Creation" << endl
          << endl
          << "Imath v" << IMATH_VERSION_STRING << endl
          << "Copyright Contributors to the OpenEXR Project" << endl
          << endl
          << "libharu v" << HPDF_VERSION_TEXT << endl
          << "Copyright (c) 1999-2006 Takeshi Kanno" << endl
          << "Copyright (c) 2007-2009 Antony Dovgal" << endl
          << endl
          << "libjpeg-turbo v" << AV_STRINGIFY(LIBJPEG_TURBO_VERSION) << endl
          << "Copyright (c) 2009-2020 D. R. Commander.  All Rights Reserved."
          << "Copyright (c) 2015 Viktor Szathmáry.  All Rights Reserved."
          << endl
          << endl
          << "libjpeg" << endl
          << "Copyright (C) 1991-2016, Thomas G. Lane, Guido Vollbeding."
          << endl
          << endl
          << PNG_HEADER_VERSION_STRING
          << "Copyright (c) 1995-2019 The PNG Reference Library Authors."
          << endl
          << "Copyright (c) 2018-2019 Cosmin Truta." << endl
          << "Copyright (c) 2000-2002, 2004, 2006-2018 Glenn Randers-Pehrson."
          << endl
          << "Copyright (c) 1996-1997 Andreas Dilger." << endl
          << "Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc." << endl
          << endl
          << "libsamplerate" << endl
          << "Copyright (c) 2012-2016, Erik de Castro Lopo "
             "<erikd@mega-nerd.com>"
          << endl
          << endl
          << TIFFLIB_VERSION_STR << endl
          << endl
          << "LibVPX" << endl
          << "Copyright (c) 2010, The WebM Project authors. All rights "
             "reserved."
          << endl
          << endl
#ifdef TLRENDER_USD
          << "MaterialX v" << MaterialX::getVersionString() << endl
          << "Copyright Contributors to the MaterialX Project" << endl
          << endl
#endif
          << "minizip-ng v" << MZ_VERSION << endl
          << "Copyright (C) Nathan Moinvaziri" << endl
          << endl
          << "nlohmann_json v" << NLOHMANN_JSON_VERSION_MAJOR << "."
          << NLOHMANN_JSON_VERSION_MINOR << "." << NLOHMANN_JSON_VERSION_PATCH
          << endl
          << "Copyright (c) 2013-present Niels Lohmann" << endl
          << endl
          << "OFL" << endl
          << "Copyright (c) 26 February 2007" << endl
          << endl
          << "OpenColorIO v" << OCIO::GetVersion() << endl
          << "http://www.opencolorio.org/" << endl
          << "Copyright Contributors to the OpenColorIO Project." << endl
          << endl
          << "OpenEXR v" << OPENEXR_VERSION_STRING << endl
          << "http://www.openexr.org/" << endl
          << "(C) 2005-2023 Industrial Light & Magic" << endl
          << endl
          << "OpenTimelineIO" << endl
          << "opentime " << AV_STRINGIFY(OPENTIME_VERSION) << endl
          << "opentimelineio " << AV_STRINGIFY(OPENTIMELINEIO_VERSION) << endl
          << "Copyright Contributors to the OpenTimelineIO project" << endl
          << endl;

        o << "Poco v";
        semantic_versioning(o, POCO_VERSION);
        o << endl
          << "Copyright (c) 2012, Applied Informatics Software Engineering "
             "GmbH. and Contributors."
          << endl
          << endl
          << "Polyline2D (modified)" << endl
          << "Copyright © 2019 Marius Metzger (CrushedPixel)" << endl
          << endl
          << "pybind11 v" << PYBIND11_VERSION_MAJOR << "."
          << PYBIND11_VERSION_MINOR << "." << PYBIND11_VERSION_PATCH << endl
          << "Copyright (c) 2016 Wenzel Jakob <wenzel.jakob@epfl.ch>, All "
             "rights reserved"
          << endl
          << endl
          << "pystring" << endl
          << "Copyright (c) 2008-present Contributors to the Pystring project."
          << endl
          << "All Rights Reserved." << endl
          << endl
          << "Python v" << PY_VERSION << " - ";
        switch (PY_RELEASE_LEVEL)
        {
        case PY_RELEASE_LEVEL_ALPHA:
            o << "Alpha ";
            break;
        case PY_RELEASE_LEVEL_BETA:
            o << "Beta ";
            break;
        case PY_RELEASE_LEVEL_GAMMA:
            o << "Gamma ";
            break;
        case PY_RELEASE_LEVEL_FINAL:
            o << "Final";
            break;
        }
        if (PY_RELEASE_SERIAL)
            o << PY_RELEASE_SERIAL;
        o << endl
          << "Copyright (c) 2001-present Python Software Foundation." << endl
          << "All Rights Reserved." << endl
          << endl
          << "RtAudio v" << RTAUDIO_VERSION << endl
          << "Copyright (c) 2001-2019 Gary P. Scavone" << endl
          << endl
          << "stb v" << STBI_VERSION << endl
          << "Copyright (c) 2017 Sean Barrett" << endl
          << endl
#ifdef TLRENDER_USD
          << "tbb v" << TBB_VERSION_MAJOR << " " << TBB_VERSION_MINOR << endl
          << "Copyright (c) 2005-2019 Intel Corporation" << endl
          << endl
#endif
          << "tlRender v" << TLRENDER_VERSION << " (modified)." << endl
          << "Original at: " << endl
          << "https://www.github.com/darbyjohnston/tlRender" << endl
          << "(C) 2021-2023 Darby Johnston." << endl
          << endl
#ifdef TLRENDER_USD
          << "USD v" << MaterialX::getVersionString() << endl
          << "(C) 2016-present Pixar" << endl
          << endl
#endif
          << "yaml-cpp" << endl
          << "Copyright (c) 2008-2015 Jesse Beder." << endl
          << endl
          << "zlib v" << ZLIB_VERSION << endl
          << "(C) 2008-2023 Jean-loup Gailly and Mark Adler" << endl
          << endl
          << endl
          << "A big thank you goes to Greg Ercolano who helped with" << endl
          << "the fltk1.4 porting and with the color schemes.";

        return o.str();
    }

    std::string cpu_information()
    {
        return GetCpuCaps(&gCpuCaps);
    }

    std::string gpu_information(ViewerUI* ui)
    {
        using std::endl;
        std::ostringstream o;

        int num_monitors = Fl::screen_count();
        o << "Monitors:\t" << num_monitors << endl << endl;

        tl::gl::initGLAD();

        // Get OpenGL information
        char* vendorString = (char*)glGetString(GL_VENDOR);
        if (!vendorString)
            vendorString = (char*)_("Unknown");

        char* rendererString = (char*)glGetString(GL_RENDERER);
        if (!rendererString)
            rendererString = (char*)_("Unknown");

        char* versionString = (char*)glGetString(GL_VERSION);
        if (!versionString)
            versionString = (char*)_("Unknown");

        o << _("Vendor:\t") << vendorString << endl
          << _("Renderer:\t") << rendererString << endl
          << _("Version:\t") << versionString << endl
          << endl;

        // Get maximum texture resolution for gfx card
        GLint glMaxTexDim;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTexDim);
        o << _("Max. Texture Size:\t") << glMaxTexDim << " x " << glMaxTexDim
          << endl
          << endl;

#ifdef FLTK_USE_WAYLAND
        wl_display* wld = fl_wl_display();
        if (wld)
        {
            EGLDisplay display = eglGetDisplay((EGLNativeDisplayType)wld);
            const char* client_apis = eglQueryString(display, EGL_CLIENT_APIS);
            const char* egl_version = eglQueryString(display, EGL_VERSION);
            const char* egl_vendor = eglQueryString(display, EGL_VENDOR);
            const char* no_display_extensions =
                eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
            const char* extensions = eglQueryString(display, EGL_EXTENSIONS);
            o << "EGL client apis:\t" << client_apis << std::endl
              << "EGL version:\t" << egl_version << std::endl
              << "EGL vendor:\t" << egl_vendor << std::endl
              << "No display extensions:\t" << no_display_extensions
              << std::endl
              << "Extensions:\t" << extensions << std::endl
              << std::endl;
        }
#endif

        o << "HW Stereo:\t" << (ui->uiView->can_do(FL_STEREO) ? "Yes" : "No")
          << endl
          << "HW Overlay:\t" << (ui->uiView->can_do_overlay() ? "Yes" : "No")
          << endl;

        return o.str();
    }

} // namespace mrv
