// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>

#ifdef __linux__

#    include <sys/types.h>
#    include <sys/sysinfo.h>
#    include <stdlib.h>
#    include <stdio.h>
#    include <string.h>

#    include <FL/platform.H>
#    undef Status
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

#ifdef TLRENDER_OCIO
#    include <expat.h>
#endif

#ifdef MRV2_PDF
#    include <hpdf_version.h>
#endif

#include <Imath/ImathConfig.h>
#include <jconfig.h>
#include <libintl.h>
#include <libpng16/png.h>
#include <mz.h>
#include <nlohmann/json.hpp>
#include <opentime/version.h>
#include <opentimelineio/version.h>

#ifdef TLRENDER_NDI
#    include <Processing.NDI.Lib.h>
#endif

#ifdef TLRENDER_AUDIO
#    include <rtaudio/RtAudio.h>
#endif

#ifdef TLRENDER_GL
#    include <tlGL/Init.h>
#endif

#ifdef TLRENDER_RAW
#    include <jasper/jas_version.h>
#    include <lcms2.h>
#    include <libraw/libraw_version.h>
#endif

#ifdef TLRENDER_STB
#    include <stb/stb_image.h>
#endif

#ifdef TLRENDER_TIFF
#    include <tiffvers.h>
#endif

#ifdef TLRENDER_USD
#    include <boost/version.hpp>
#    include <tbb/tbb_stddef.h>
#    include <MaterialXCore/Util.h>
#    include <pxr/pxr.h>
#endif

#ifdef TLRENDER_NET
#    include <curl/curl.h>
#endif

#ifdef MRV2_NETWORK
#    include <Poco/Version.h>
#endif

#ifdef MRV2_PYBIND11
#    include <pybind11/pybind11.h>
#endif

#ifdef TLRENDER_OCIO
#    include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;
#endif

#include <zlib.h>

#ifdef TLRENDER_FFMPEG
extern "C"
{
#    include <libavcodec/avcodec.h>
#    include <libavformat/avformat.h>
#    include <libswscale/version.h>
#    include <libswresample/version.h>
}
#else
#    define AV_STRINGIFY(s) AV_TOSTRING(s)
#    define AV_TOSTRING(s) #s
#endif

#ifdef _WIN32
#    pragma warning(disable : 4275)
#endif

#include <tlCore/String.h>

#include "mrvCore/mrvOS.h"

#include "mrvCore/mrvCPU.h"

#include "mrvWidgets/mrvVersion.h"

#include "mrvGL/mrvGLErrors.h" // defines glGetString and GL_VERSION

#include "mrViewer.h"
#undef snprintf

#ifdef _WIN32
#    include <windows.h>
#    include <psapi.h>
#endif

#include "mrvCore/mrvOS.h"
#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvIO.h"

namespace mrv
{
    namespace
    {
        void
        semantic_versioning(std::stringstream& o, const unsigned version_hex)
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
        

#ifdef TLRENDER_FFMPEG
        const AVCodec* next_codec_for_id(enum AVCodecID id, void **iter,
                                         int encoder)
        {
            const AVCodec* c = nullptr;
            while ((c = av_codec_iterate(iter)))
            {
                if (c->id == id &&
                    (encoder ? av_codec_is_encoder(c) : av_codec_is_decoder(c)))
                    return c;
            }
            return NULL;
        }
        
        void print_codecs_for_id(std::ostringstream& o,
                                 enum AVCodecID id, int encoder)
        {
            void *iter = NULL;
            const AVCodec *codec;

            o << " (" << (encoder ? "encoders" : "decoders") << ":";
        
            while ((codec = next_codec_for_id(id, &iter, encoder)))
                o << ' ' << codec->name;

            o << ')';
        }

        int compare_codec_desc(const void *a, const void *b)
        {
            const AVCodecDescriptor * const *da = (const AVCodecDescriptor* const *) a;
            const AVCodecDescriptor * const *db = (const AVCodecDescriptor* const *) b;

            return (
                (*da)->type != (*db)->type
                    ? FFDIFFSIGN((*da)->type, (*db)->type)
                    : strcmp((*da)->name, (*db)->name));
        }

        int get_codecs_sorted(const AVCodecDescriptor ***rcodecs)
        {
            const AVCodecDescriptor *desc = NULL;
            const AVCodecDescriptor **codecs;
            unsigned nb_codecs = 0, i = 0;

            while ((desc = avcodec_descriptor_next(desc)))
                nb_codecs++;
            if (!(codecs = (const AVCodecDescriptor**)av_calloc(
                      nb_codecs, sizeof(*codecs))))
                return AVERROR(ENOMEM);
            desc = NULL;
            while ((desc = avcodec_descriptor_next(desc)))
                codecs[i++] = desc;
            qsort(codecs, nb_codecs, sizeof(*codecs), compare_codec_desc);
            *rcodecs = codecs;
            return nb_codecs;
        }
#endif
    
    } // namespace

    const char* kVersion = MRV2_VERSION;
    const char* kBuild = "- Built " __DATE__ " " __TIME__;
    const char* kArch = "64";

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
            name(string::toLower(n)),
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

    static int ffmpeg_format_widths[] = { 20, 20, 20, 130, 80, 150, 0 };
    
    void ffmpeg_formats(mrv::TextBrowser* b)
    {
        using namespace std;
#ifdef TLRENDER_FFMPEG
        const AVInputFormat* ifmt = NULL;
        const AVOutputFormat* ofmt = NULL;
#endif
        const char* last_name = NULL;

        FormatList formats;
        FormatInfo* f = NULL;

#ifdef TLRENDER_EXR
        f = new FormatInfo(true, true, false, "EXR", "mrv2", "ILM OpenEXR");
        formats.push_back(f);
        f = new FormatInfo(
            true, true, false, "SXR", "mrv2", "ILM Stereo OpenEXR");
        formats.push_back(f);
#endif

#ifdef TLRENDER_JPEG
        f = new FormatInfo(
            true, true, false, "JPEG", "mrv2",
            "Joint Photographic Experts Group");
        formats.push_back(f);
#endif

#ifdef TLRENDER_PNG
        f = new FormatInfo(true, true, false, "PNG", "mrv2", "PNG");
        formats.push_back(f);
#endif

#ifdef TLRENDER_STB
        f = new FormatInfo(
            true, false, false, "PSD", "mrv2", "Photoshop Document");
        formats.push_back(f);

        f = new FormatInfo(
            true, true, false, "TGA", "mrv2", "Truevision Graphics Adapter");
        formats.push_back(f);

        f = new FormatInfo(
            true, true, false, "BMP", "mrv2", "Bitmap Image File");
        formats.push_back(f);
#endif

#ifdef TLRENDER_TIFF
        f = new FormatInfo(
            true, true, false, "TIFF", "mrv2", "Tagged Image File Format");
        formats.push_back(f);
        f = new FormatInfo(
            true, true, false, "TIF", "mrv2", "Tagged Image File Format");
        formats.push_back(f);
#endif

        f = new FormatInfo(true, true, false, "Cineon", "mrv2", "Cineon");
        formats.push_back(f);
        f = new FormatInfo(true, true, false, "DPX", "mrv2", "DPX");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "OTIO", "mrv2", "OpenTimelineIO");
        formats.push_back(f);
        f = new FormatInfo(true, true, false, "PPM", "mrv2", "Portable Pixmap");
        formats.push_back(f);
        f = new FormatInfo(
            true, true, false, "RGB", "mrv2", "Silicon Graphics RGB Picture");
        formats.push_back(f);
        f = new FormatInfo(
            true, true, false, "RGBA", "mrv2", "Silicon Graphics RGBA Picture");
        formats.push_back(f);
        f = new FormatInfo(
            true, true, false, "SGI", "mrv2", "Silicon Graphics Image");
        formats.push_back(f);

#ifdef TLRENDER_RAW
        f = new FormatInfo(
            true, false, false, "3FR", "mrv2", "Hasselblad RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "ARW", "mrv2", "Sony RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "BAY", "mrv2", "Phase One RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(true, false, false, "BMQ", "mrv2", "RAW Image File");
        formats.push_back(f);
        f = new FormatInfo(true, false, false, "CAP", "mrv2", "RAW Image File");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "CINE", "mrv2",
            "Vision Research's Phantom RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "CR2", "mrv2", "Canon RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "CR3", "mrv2", "Canon RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "CRW", "mrv2", "Canon RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "CS1", "mrv2", "CaptureShop 1-shot RAW Image");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "DC2", "mrv2", "Kodak RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "DCR", "mrv2", "Kodak RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "DNG", "mrv2", "Digital Negative Camera");
        formats.push_back(f);
        f = new FormatInfo(true, false, false, "DRF", "mrv2", "RAW Image File");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "DSC", "mrv2", "Digitial Still RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "ERF", "mrv2", "Epson RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "FFF", "mrv2", "Hasselblad RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(true, false, false, "IA", "mrv2", "RAW Image File");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "IIQ", "mrv2", "Phase One RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "KDC", "mrv2", "Kodak RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "MDC", "mrv2", "Minolta RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "MEF", "mrv2", "Mamiya Electronic");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "MOS", "mrv2", "Leaf RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "MRW", "mrv2", "Minolta RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "MRW", "mrv2", "Minolta RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "NEF", "mrv2", "Nikon Electronic");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "NRW", "mrv2", "Nikon RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "ORF", "mrv2", "Olympus RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "PEF", "mrv2",
            "Pentax Electronic File (Ricoh)");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "PTX", "mrv2", "Pentax RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "PXN", "mrv2",
            "Logitech Fotoman Pixtura Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "QTK", "mrv2", "Apple QuickTake Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "RAF", "mrv2", "Fujifilm RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "RDC", "mrv2", "Red Digital Clip");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "RW2", "mrv2", "Panasonic RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "RWL", "mrv2", "Leica RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(true, false, false, "RWZ", "mrv2", "RAW Image File");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "SR2", "mrv2", "Sony RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "SRF", "mrv2", "Sony RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "SRW", "mrv2", "Samsung RAW Camera");
        formats.push_back(f);
        f = new FormatInfo(true, false, false, "STI", "mrv2", "RAW Image File");
        formats.push_back(f);
        f = new FormatInfo(
            true, false, false, "X3F", "mrv2", "Sigma (Foveon) RAW Camera");
        formats.push_back(f);
#endif

        last_name = "000";
#ifdef TLRENDER_FFMPEG
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
                if (name && strcmp(ofmt->name, name) == 0)
                    encode = true;
            }
            opaque = NULL;
            while ((ifmt = av_demuxer_iterate(&opaque)))
            {
                if ((name == NULL || strcmp(ifmt->name, name) < 0) &&
                    strcmp(ifmt->name, last_name) > 0)
                {
                    name = ifmt->name;
                    long_name = ifmt->long_name;
                    decode = true;
                }
                if (name && strcmp(ifmt->name, name) == 0)
                    decode = true;
            }

            if (name == NULL)
                break;
            last_name = name;

            f = new FormatInfo(
                decode, encode, false, name, "FFmpeg", long_name);
            formats.push_back(f);
        }
#endif
        // Sort formats alphabetically
        std::sort(formats.begin(), formats.end(), SortFormatsFunctor());

        // Now concatenate all the stuff into a string
        {
            FormatList::const_iterator i = formats.begin();
            FormatList::const_iterator e = formats.end();
            b->column_widths(ffmpeg_format_widths);
            for (; i != e; ++i)
            {
                f = *i;
                std::ostringstream o;
                o << (f->decode ? "D\t" : " \t") << (f->encode ? "E\t" : " \t")
                  << (f->blob ? "B\t" : " \t") << f->name << "\t" << f->module
                  << "\t" << f->description;
                b->add(o.str().c_str());
                delete f;
            }
        }
    }

    static int ffmpeg_codec_widths[] = { 20, 20, 20, 20, 20, 150, 0 };
    
    void ffmpeg_codec_information(mrv::TextBrowser* b)
    {
#ifdef TLRENDER_FFMPEG
        b->column_widths(ffmpeg_codec_widths);
        b->add(_("D\t\t\t\t\t=\tDecoding supported"));
        b->add(_("\tE\t\t\t\t=\tEncoding supported"));
        b->add(_("\t\tI\t\t\t=\tIntra frame-only codec"));
        b->add(_("\t\t\tL\t\t=\tLossy compression")); 
        b->add(_("\t\t\t\tS\t=\tLossless compression"));
#endif
    }
    
    static void ffmpeg_codecs(mrv::TextBrowser* b, int type)
    {
#ifdef TLRENDER_FFMPEG
        b->column_widths(ffmpeg_codec_widths);
        const AVCodecDescriptor **codecs;
        unsigned i;
        int nb_codecs = get_codecs_sorted(&codecs);

        if (nb_codecs < 0)
            return;

        for (i = 0; i < nb_codecs; i++) {
            const AVCodecDescriptor *desc = codecs[i];
            const AVCodec *codec;
            void *iter = NULL;

            if (desc->type != type)
                continue;
        
            if (strstr(desc->name, "_deprecated"))
                continue;

            bool decode = avcodec_find_decoder(desc->id);
            bool encode = avcodec_find_encoder(desc->id);
            if (!decode && !encode)
                continue;

            std::ostringstream o;
            o << (decode ? 'D' : ' ')
              << '\t'
              << (encode ? 'E' : ' ')
              << '\t'
              << ((desc->props & AV_CODEC_PROP_INTRA_ONLY) ? 'I' : ' ')
              << '\t'
              << ((desc->props & AV_CODEC_PROP_LOSSY)      ? 'L' : ' ')
              << '\t'
              << ((desc->props & AV_CODEC_PROP_LOSSLESS)   ? 'S' : ' ')
              << '\t'
              << desc->name
              << '\t'
              << (desc->long_name ? desc->long_name : "");

            /* print decoders/encoders when there's more than one or their
             * names are different from codec name */
            while ((codec = next_codec_for_id(desc->id, &iter, 0))) {
                if (strcmp(codec->name, desc->name)) {
                    print_codecs_for_id(o, desc->id, 0);
                    break;
                }
            }
            iter = NULL;
            while ((codec = next_codec_for_id(desc->id, &iter, 1))) {
                if (strcmp(codec->name, desc->name)) {
                    print_codecs_for_id(o, desc->id, 1);
                    break;
                }
            }
        
        
            b->add(o.str().c_str());
        }
    
        av_free(codecs);
#endif
    }

    void ffmpeg_audio_codecs(mrv::TextBrowser* browser)
    {
#ifdef TLRENDER_FFMPEG
        return ffmpeg_codecs(browser, AVMEDIA_TYPE_AUDIO);
#endif
    }

    void ffmpeg_video_codecs(mrv::TextBrowser* browser)
    {
#ifdef TLRENDER_FFMPEG
        return ffmpeg_codecs(browser, AVMEDIA_TYPE_VIDEO);
#endif
    }

    void ffmpeg_subtitle_codecs(mrv::TextBrowser* browser)
    {
#ifdef TLRENDER_FFMPEG
        return ffmpeg_codecs(browser, AVMEDIA_TYPE_SUBTITLE);
#endif
    }

    void ffmpeg_protocols(mrv::TextBrowser* b)
    {
#ifdef TLRENDER_FFMPEG
        void* opaque = NULL;
        const char* up;
        for (up = avio_enum_protocols(&opaque, 0); up;
             up = avio_enum_protocols(&opaque, 0))
        {
            std::ostringstream o;
            o << " " << up << ":";
            b->add(o.str().c_str());
        }
#endif
    }

    void about_message(mrv::TextBrowser* b)
    {
        using namespace std;

#ifdef TLRENDER_FFMPEG
        avformat_network_init();
#endif

        std::stringstream o;

        o << "mrv2 " << kArch << " bits - v" << kVersion << " " << kBuild
          << endl
#ifdef __GLIBCXX__
          << _("With gcc ") << __GNUC__ << endl
#elif __clang__
          << _("With clang ") << __clang__ << " " << __llvm__ << endl
#else
          << _("With msvc ") << _MSC_VER << endl
#endif
          << "(C) 2022-Present" << endl
          << "Gonzalo Garramuño & others" << endl
          << endl
          << mrv::get_os_version() << endl
          << endl
          << _("mrv2 depends on:") << endl
          << endl;

#ifdef TLRENDER_USD
        unsigned int boost_major = BOOST_VERSION / 100000;
        unsigned int boost_minor = BOOST_VERSION / 100 % 1000;
        unsigned int boost_patch = BOOST_VERSION % 100;
        o << "Boost v" << boost_major << "." << boost_minor << "."
          << boost_patch << endl
          << "Copyright (c) 2016-Present Contributors to the Boost Project"
          << endl
          << endl;
#endif
#ifdef TLRENDER_OCIO
        const auto expat = XML_ExpatVersionInfo();
        o << "expat v" << expat.major << "." << expat.minor << "."
          << expat.micro << endl
          << "Copyright (c) 1998-2000 Thai Open Source Software Center Ltd and "
             "Clark Cooper"
          << endl
          << "Copyright (c) 2001-2022 Expat maintainers" << endl
          << endl;
#endif
#ifdef TLRENDER_FFMPEG
        o << "FFmpeg " << av_version_info() << endl
          << "libavutil          v" << AV_STRINGIFY(LIBAVUTIL_VERSION) << endl
          << "libavcodec      v" << AV_STRINGIFY(LIBAVCODEC_VERSION) << endl
          << "libavformat     v" << AV_STRINGIFY(LIBAVFORMAT_VERSION) << endl
          << "libswresample v" << AV_STRINGIFY(LIBSWRESAMPLE_VERSION) << endl
          << "libswscale       v" << AV_STRINGIFY(LIBSWSCALE_VERSION) << endl
          << "http://ffmpeg.mplayerhq.hu/" << endl
          << "License: " << avcodec_license() << endl
          << "Copyright (c) 2000-Present Fabrice Bellard, et al." << endl
          << "Configuration: " << endl
          << "\tavutil: " << avutil_configuration() << endl
          << "\tavcodec: " << avcodec_configuration() << endl
          << "\tavformat: " << avformat_configuration() << endl
          << endl;
#endif
        o << "Flmm Color Picker (modified)" << endl
          << "Copyright (c) 2002 - 2004 Matthias Melcher" << endl
          << endl;
        o << "FLTK v1.4" << endl
          << "http://www.fltk.org/" << endl
          << "Copyright (c) 2000-Present Bill Spitzak & others" << endl
          << endl;
        o << "Modified FLU - FLTK Utility Widgets" << endl
          << "Copyright (c) 2002 Ohio Supercomputer Center, Ohio State "
             "University"
          << endl
          << endl;
        o << "FreeType" << endl
          << "Copyright (c) 1996-2002, 2006 by David Turner, Robert Wilhelm, "
             "and Werner Lemberg"
          << endl
          << endl;
        o << "glad v" << GLAD_GENERATOR_VERSION << endl
          << "Copyright (c) 2013-2020 David Herberth" << endl
          << endl;
        o << "Imath v" << IMATH_VERSION_STRING << endl
          << "Copyright Contributors to the OpenEXR Project" << endl
          << endl;
#ifdef TLRENDER_RAW
        o << "Jasper v" << jas_getversion() << endl
          << JAS_COPYRIGHT << endl
          << "Little Color Management System v" << (LCMS_VERSION / 1000.0)
          << endl
          << "Copyright (c) 1998-Present Marti Maria Saguer" << endl
          << endl;
#endif
#ifdef TLRENDER_NET
        o << curl_version() << endl
          << "(C) Daniel Stenberg, <daniel@haxx.se>, et al." << endl
          << endl;
#endif
#ifdef MRV2_PDF
        o << "libharu v" << HPDF_VERSION_TEXT << endl
          << "Copyright (c) 1999-2006 Takeshi Kanno" << endl
          << "Copyright (c) 2007-2009 Antony Dovgal" << endl
          << endl;
#endif
        o << "libintl/gettext v0.22.3" << endl
          << endl
          << "libjpeg-turbo v" << AV_STRINGIFY(LIBJPEG_TURBO_VERSION) << endl
          << "Copyright (c) 2009-2020 D. R. Commander.  All Rights Reserved."
          << "Copyright (c) 2015 Viktor Szathmáry.  All Rights Reserved."
          << endl
          << endl;
#ifdef TLRENDER_RAW
        o << "LibRaw " << LIBRAW_VERSION_STR << endl
          << "Copyright (C) 2008-2021 LibRaw LLC (info@libraw.org)" << endl
          << "The library includes source code from" << endl
          << "dcraw.c, Dave Coffin's raw photo decoder" << endl
          << "Copyright 1997-2016 by Dave Coffin, dcoffin a cybercom o net"
          << endl
          << endl;
#endif
#ifdef TLRENDER_PNG
        o << PNG_HEADER_VERSION_STRING
          << "Copyright (c) 1995-2019 The PNG Reference Library Authors."
          << endl
          << "Copyright (c) 2018-2019 Cosmin Truta." << endl
          << "Copyright (c) 2000-2002, 2004, 2006-2018 Glenn Randers-Pehrson."
          << endl
          << "Copyright (c) 1996-1997 Andreas Dilger." << endl
          << "Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc." << endl
          << endl;
#endif
        o << "libsamplerate" << endl
          << "Copyright (c) 2012-2016, Erik de Castro Lopo "
             "<erikd@mega-nerd.com>"
          << endl
          << endl;
#ifdef TLRENDER_TIFF
        o << TIFFLIB_VERSION_STR << endl << endl;
#endif
        o << "LibVPX" << endl
          << "Copyright (c) 2010, The WebM Project authors. All rights "
             "reserved."
          << endl
          << endl;
#ifdef TLRENDER_USD
        o << "MaterialX v" << MaterialX::getVersionString() << endl
          << "Copyright Contributors to the MaterialX Project" << endl
          << endl;
#endif
        o << "minizip-ng v" << MZ_VERSION << endl
          << "Copyright (C) Nathan Moinvaziri" << endl
          << endl;
#ifdef TLRENDER_NDI
        o << "NDI® " << NDIlib_version() << endl
          << "Copyright (C) 2021-Present Vizrt NDI AB" << endl
          << endl;
#endif
        o << "nlohmann_json v" << NLOHMANN_JSON_VERSION_MAJOR << "."
          << NLOHMANN_JSON_VERSION_MINOR << "." << NLOHMANN_JSON_VERSION_PATCH
          << endl
          << "Copyright (c) 2013-Present Niels Lohmann" << endl
          << endl;
        o << "OFL (Open Font License)" << endl
          << "Copyright (c) 26 February 2007" << endl
          << endl;
#ifdef TLRENDER_OCIO
        o << "OpenColorIO v" << OCIO::GetVersion() << endl
          << "http://www.opencolorio.org/" << endl
          << "Copyright Contributors to the OpenColorIO Project." << endl
          << endl;
#endif
#ifdef TLRENDER_EXR
        o << "OpenEXR v" << OPENEXR_VERSION_STRING << endl
          << "http://www.openexr.org/" << endl
          << "(C) 2005-Present Industrial Light & Magic" << endl
          << endl;
#endif
        o << "OpenTimelineIO" << endl
          << "opentime " << AV_STRINGIFY(OPENTIME_VERSION) << endl
          << "opentimelineio " << AV_STRINGIFY(OPENTIMELINEIO_VERSION) << endl
          << "Copyright Contributors to the OpenTimelineIO project" << endl
          << endl;
#ifdef MRV2_NETWORK
        o << "Poco v";
        semantic_versioning(o, POCO_VERSION);
        o << endl;
        o << "Copyright (c) 2012, Applied Informatics Software Engineering "
             "GmbH. and Contributors."
          << endl
          << endl;
#endif
        o << "Polyline2D (modified)" << endl
          << "Copyright © 2019 Marius Metzger (CrushedPixel)" << endl
          << endl;
#ifdef MRV2_PYBIND11
        o << "pybind11 v" << PYBIND11_VERSION_MAJOR << "."
          << PYBIND11_VERSION_MINOR << "." << PYBIND11_VERSION_PATCH << endl
          << "Copyright (c) 2016 Wenzel Jakob <wenzel.jakob@epfl.ch>, All "
             "rights reserved"
          << endl
          << endl;
#endif
#ifdef MRV2_PYFLTK
        o << "pyFLTK v1.4" << endl
          << "Copyright 2003-Present by Andreas Held and others." << endl
          << endl;
#endif
        o << "pystring" << endl
          << "Copyright (c) 2008-Present Contributors to the Pystring project."
          << endl
          << "All Rights Reserved." << endl
          << endl;
#ifdef MRV2_PYBIND11
        o << "Python v" << PY_VERSION << " - ";
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
          << "Copyright (c) 2001-Present Python Software Foundation." << endl
          << "All Rights Reserved." << endl
          << endl;
#endif
#ifdef TLRENDER_AUDIO
        o << "RtAudio v" << RTAUDIO_VERSION << endl
          << "Copyright (c) 2001-Present Gary P. Scavone" << endl
          << endl;
#endif
#ifdef TLRENDER_STB
        o << "stb v" << STBI_VERSION << endl
          << "Copyright (c) 2017 Sean Barrett" << endl
          << endl;
#endif
#ifdef TLRENDER_USD
        o << "tbb v" << TBB_VERSION_MAJOR << " " << TBB_VERSION_MINOR << endl
          << "Copyright (c) 2005-2019 Intel Corporation" << endl
          << endl;
#endif
        o << "tlRender v" << TLRENDER_VERSION << " (modified)." << endl
          << "Original at: " << endl
          << "https://www.github.com/darbyjohnston/tlRender" << endl
          << "(C) 2021-Present Darby Johnston." << endl
          << endl;
#ifdef TLRENDER_USD
        o << "USD v" << PXR_MAJOR_VERSION << "." << PXR_MINOR_VERSION << "."
          << PXR_PATCH_VERSION << endl
          << "(C) 2016-Present Pixar" << endl
          << endl;
#endif
        o << "yaml-cpp" << endl
          << "Copyright (c) 2008-2015 Jesse Beder." << endl
          << endl;
        o << "zlib v" << ZLIB_VERSION << endl
          << "(C) 2008-Present Jean-loup Gailly and Mark Adler" << endl
          << endl
          << endl;
        o << "A big thank you goes to Greg Ercolano who helped with" << endl
          << "the fltk1.4 porting and with the color schemes.";

        std::string line;
        while (std::getline(o, line, '\n'))
        {
            b->add(line.c_str());
        }
    }

    void cpu_information(mrv::TextBrowser* b)
    {
        const std::string& lines = GetCpuCaps(&gCpuCaps);

        std::stringstream o(lines);
        
        std::string line;
        while (std::getline(o, line, '\n'))
        {
            b->add(line.c_str());
        }
    }

    std::string gpu_information(ViewerUI* ui)
    {
        using std::endl;
        std::stringstream o;

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
