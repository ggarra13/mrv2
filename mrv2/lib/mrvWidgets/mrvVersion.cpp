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

#include <tlGlad/gl.h> // defines glGetString and GL_VERSION

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

#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;

#include "mrvCore/mrvOS.h"
#undef snprintf

#include "mrvCore/mrvCPU.h"

#include "mrvWidgets/mrvVersion.h"

#include "ImathInt64.h"
#include "mrViewer.h"
#undef snprintf

#ifdef _WIN32
#    include <windows.h>
#    include <psapi.h>
#endif

#include "mrvFl/mrvIO.h"

namespace mrv
{

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
          << "(C) 2007-2023 Film Aura, LLC." << endl
          << "              Gonzalo Garramuño & others" << endl;

        o << endl
          << "mrv2 depends on:" << endl
          << endl
          << endl
          << "Modified tlRender.  Original at: "
          << "https://www.github.com/darbyjohnston/tlRender" << endl
          << "(C) 2021-2023 Darby Johnston." << endl
          << endl
          << "FLTK v1.4" << endl
          << "http://www.fltk.org/" << endl
          << "(C) 2000-2023 Bill Spitzak & others" << endl
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
          << "(C) 2000-2023 Fabrice Bellard, et al." << endl
          << "Configuration: " << avcodec_configuration() << endl
          << endl
          << "ILM OpenEXR v" << OPENEXR_VERSION_STRING << endl
          << "http://www.openexr.org/" << endl
          << "(C) 2005-2023 Industrial Light & Magic" << endl
          << endl
          << "OpenColorIO v" << OCIO::GetVersion() << endl
          << "http://www.opencolorio.org/" << endl
          << "(C) 2005-2023 Sony Pictures Imageworks" << endl
          << endl;

        o << endl
          << "AMPAS ACES v1.0 or later" << endl
          << "https://github.com/ampas/aces-dev" << endl
          << "(C) 2019-2023 AMPAS" << endl
          << endl
          << "zlib v" << ZLIB_VERSION
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
        ;
        o << "Monitors:\t" << num_monitors << endl << endl;

        char* vendorString = (char*)glGetString(GL_VENDOR);
        if (!vendorString)
            vendorString = (char*)_("Unknown");

        char* rendererString = (char*)glGetString(GL_RENDERER);
        if (!rendererString)
            rendererString = (char*)_("Unknown");

        char* versionString = (char*)glGetString(GL_VERSION);
        if (!versionString)
            versionString = (char*)_("Unknown");

        // Get maximum texture resolution for gfx card
        GLint glMaxTexDim;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glMaxTexDim);

        o << _("Vendor:\t") << vendorString << endl
          << _("Renderer:\t") << rendererString << endl
          << _("Version:\t") << versionString << endl
          << endl

          // << _("PBO Textures:\t") << (_pboTextures   ? _("Yes") : _("No")) <<
          // endl
          // << _("Float Pixels:\t") << (_floatPixels  ? _("Yes") : _("No")) <<
          // endl
          // << _("Half Pixels:\t") << (_halfPixels  ? _("Yes") : _("No")) <<
          // endl
          // << _("Float Textures:\t") << (_floatTextures ? _("Yes") : _("No"))
          // << endl
          // << _("Non-POT Textures:\t")
          // << (_pow2Textures  ? _("No")  : _("Yes")) << endl
          << _("Max. Texture Size:\t") << glMaxTexDim << " x " << glMaxTexDim
          << endl
          // << _("Texture Units:\t") << _maxTexUnits << endl
          // << _("YUV  Support:\t") << (supports_yuv() ? _("Yes") : _("No")) <<
          // endl
          // << _("YUVA Support:\t") << (supports_yuva() ? _("Yes") : _("No"))
          // << endl
          // << _("SDI Output:\t") << (_sdiOutput ? _("Yes") : _("No"))
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
