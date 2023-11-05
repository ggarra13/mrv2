// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <set>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

#include <tlCore/StringFormat.h>
#include <tlIO/System.h>

#include <FL/Fl_Progress.H>
#include <FL/Fl_Output.H>
#include <FL/Enumerations.H>
#include <FL/fl_ask.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Native_File_Chooser.H>

#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvString.h"

#include "mrvFl/mrvHotkey.h"
#include "mrvFl/mrvPreferences.h"

#include "mrvFLU/Flu_File_Chooser.h"

#include "mrvApp/App.h"

#include "mrvFl/mrvIO.h"

namespace
{

    const char* kModule = "filereq";

#ifdef _WIN32
#    define kSeparator ";"
#else
#    define kSeparator ":"
#endif

    // File extension patterns

    static const std::string kReelPattern = "otio,otioz";

    std::string
    getMoviePattern(const std::shared_ptr<tl::system::Context>& context)
    {
        std::string result;
        const auto ioSystem = context->getSystem<io::System>();
        const auto plugins = ioSystem->getPlugins();
        for (auto plugin : plugins)
        {
            auto extensions =
                plugin->getExtensions(static_cast<int>(io::FileType::Movie));
            for (auto& extension : extensions)
            {
                if (!result.empty())
                    result += ',';
                result += extension.substr(1, extension.size());
            }
        }
        return result;
    }

    std::string
    getImagePattern(const std::shared_ptr<tl::system::Context>& context)
    {
        std::string result;
        const auto ioSystem = context->getSystem<io::System>();
        const auto plugins = ioSystem->getPlugins();
        for (auto plugin : plugins)
        {
            auto extensions =
                plugin->getExtensions(static_cast<int>(io::FileType::Sequence));
            for (auto& extension : extensions)
            {
                if (!result.empty())
                    result += ',';
                result += extension.substr(1, extension.size());
            }
        }
        return result;
    }

    std::string
    getAudioPattern(const std::shared_ptr<tl::system::Context>& context)
    {
        std::string result;
        const auto ioSystem = context->getSystem<io::System>();
        const auto plugins = ioSystem->getPlugins();
        for (auto plugin : plugins)
        {
            auto extensions =
                plugin->getExtensions(static_cast<int>(io::FileType::Audio));
            for (auto& extension : extensions)
            {
                if (!result.empty())
                    result += ',';
                result += extension.substr(1, extension.size());
            }
        }
        return result;
    }

    static const std::string kSubtitlePattern = "srt,sub,ass,vtt";

    static const std::string kOCIOPattern = "ocio";

    static const std::string kSessionPattern = "mrv2s";

} // namespace

namespace mrv
{

    // WINDOWS crashes if pattern is sent as is.  We need to sanitize the
    // pattern here.  From:
    // All (*.{png,ogg})\t
    // to:
    // All\t{*.png,ogg}\n
    std::string pattern_to_native(std::string pattern)
    {
        std::string ret;
        size_t pos = 0, pos2 = 0;
        while (pos != std::string::npos)
        {
            pos = pattern.find(' ');
            if (pos == std::string::npos)
                break;
            ret += pattern.substr(0, pos) + '\t';
            size_t pos3 = pattern.find('(', pos);
            size_t pos2 = pattern.find(')', pos);
            ret += pattern.substr(pos3 + 1, pos2 - pos3 - 1) + '\n';
            if (pos2 + 2 < pattern.size())
                pattern = pattern.substr(pos2 + 2, pattern.size());
            else
                pattern.clear();
        }
        return ret;
    }

    const std::string file_save_single_requester(
        const char* title, const char* pattern, const char* startfile,
        const bool compact_images)
    {
        std::string file;
        try
        {
            bool native = mrv::Preferences::native_file_chooser;
            if (native)
            {
                Fl_Native_File_Chooser native;
                native.title(title);
                native.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
                std::string native_pattern = pattern_to_native(pattern);
                native.filter(native_pattern.c_str());
                native.preset_file(startfile);
                // Show native chooser
                switch (native.show())
                {
                case -1:
                    break; // ERROR
                case 1:
                    break; // CANCEL
                default:   // PICKED FILE
                    if (native.filename())
                    {
                        file = native.filename();
                    }
                }
            }
            else
            {
                const auto context = App::app->getContext();
                const char* f = flu_save_chooser(
                    context, title, pattern, startfile, compact_images);
                if (!f)
                {
                    return "";
                }
                file = f;
            }
        }
        catch (const std::exception& e)
        {
        }

        return file;
    }

    std::vector<std::string> file_multi_requester(
        const char* title, const char* pattern, const char* startfile,
        const bool compact_images)
    {
        std::vector<std::string> filelist;

        try
        {
            if (!startfile)
                startfile = "";
            bool native = mrv::Preferences::native_file_chooser;
            if (native)
            {
                Fl_Native_File_Chooser native;
                native.title(title);
                native.type(Fl_Native_File_Chooser::BROWSE_MULTI_FILE);
                std::string native_pattern = pattern_to_native(pattern);
                native.filter(native_pattern.c_str());
                native.preset_file(startfile);
                // Show native chooser
                switch (native.show())
                {
                case -1:
                    break; // ERROR
                case 1:
                    break; // CANCEL
                default:   // PICKED FILE
                    if (native.count() > 0)
                    {
                        for (int i = 0; i < native.count(); ++i)
                        {
                            filelist.push_back(native.filename(i));
                        }
                    }
                }
            }
            else
            {
                const auto context = App::app->getContext();
                flu_multi_file_chooser(
                    context, title, pattern, startfile, filelist,
                    compact_images);
            }
        }
        catch (const std::exception& e)
        {
        }
        catch (...)
        {
        }
        return filelist;
    }

    std::string file_single_requester(
        const char* title, const char* pattern, const char* startfile,
        const bool compact_files)
    {
        std::string file;
        try
        {
            if (!startfile)
                startfile = "";
            bool native = mrv::Preferences::native_file_chooser;
            if (native)
            {
                Fl_Native_File_Chooser native;
                native.title(title);
                native.type(Fl_Native_File_Chooser::BROWSE_FILE);
                std::string native_pattern = pattern_to_native(pattern);
                native.filter(native_pattern.c_str());
                native.preset_file(startfile);
                // Show native chooser
                switch (native.show())
                {
                case -1:
                    break; // ERROR
                case 1:
                    break; // CANCEL
                default:   // PICKED FILE
                    if (native.count() > 0)
                    {
                        file = native.filename();
                    }
                }
            }
            else
            {
                const auto context = App::app->getContext();
                const char* f = flu_file_chooser(
                    context, title, pattern, startfile, compact_files);
                if (f)
                    file = f;
            }
        }
        catch (const std::exception& e)
        {
        }
        catch (...)
        {
        }

        return file;
    }

    /**
     * Opens a file requester to load a directory of image files
     *
     * @param startfile  start filename (directory)
     *
     * @return A directory to be opened or NULL
     */
    std::string open_directory(const char* startfile)
    {
        std::string dir;
        std::string title = _("Load Directory");
        bool native = mrv::Preferences::native_file_chooser;
        if (native)
        {
            Fl_Native_File_Chooser native;
            native.title(title.c_str());
            native.directory(startfile);
            native.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
            // Show native chooser
            switch (native.show())
            {
            case -1:
                break; // ERROR
            case 1:
                break; // CANCEL
            default:   // PICKED DIR
                if (native.filename())
                {
                    dir = native.filename();
                }
                break;
            }
        }
        else
        {
            const auto context = App::app->getContext();
            const char* d = flu_dir_chooser(context, title.c_str(), startfile);
            if (d)
                dir = d;
        }
        fs::path path = dir;
        return path.generic_string();
    }

    /**
     * Opens a file requester to load image files
     *
     * @param startfile  start filename (directory)
     *
     * @return Each file to be opened
     */
    std::vector<std::string>
    open_image_file(const char* startfile, const bool compact_images)
    {
        auto context = App::app->getContext();
        const std::string kREEL_PATTERN =
            _("Reels (*.{") + kReelPattern + "})\t";
        const std::string kAUDIO_PATTERN =
            ("Audios (*.{") + getAudioPattern(context) + "})\t";
        const std::string kIMAGE_PATTERN =
            _("Images (*.{") + getImagePattern(context) + "})\t";
        const std::string kALL_PATTERN =
            _("All (*.{") + getImagePattern(context) + "," +
            getMoviePattern(context) + "," + kReelPattern + "," +
            getAudioPattern(context) + "," + kReelPattern + "})\t" +
            kIMAGE_PATTERN + kAUDIO_PATTERN + _("Movies (*.{") +
            getMoviePattern(context) + "})\t" + kREEL_PATTERN;

        std::string pattern = kIMAGE_PATTERN + kAUDIO_PATTERN;
        std::string title = _("Load Image");
        if (compact_images)
        {
            title = _("Load Movie or Sequence");
            pattern = kALL_PATTERN;
        }

        return file_multi_requester(
            title.c_str(), pattern.c_str(), startfile, compact_images);
    }

    std::string save_otio(const char* startfile)
    {
        std::string kOTIO_PATTERN = "OTIO (*.otio)\t";

        std::string title = _("Save OTIO timeline");

        std::string file = file_save_single_requester(
            title.c_str(), kOTIO_PATTERN.c_str(), startfile, false);
        if (file.empty())
            return file;

        if (file.substr(file.size() - 5, file.size()) != ".otio")
            file += ".otio";

        return file;
    }

    /**
     * Opens a file requester to save a .py file.
     *
     * @param startfile  start filename (directory)
     *
     * @return  opened .py file or empty
     */
    std::string save_python_file(const char* startfile)
    {
        std::string kPYTHON_PATTERN = "Python (*.py)\t";

        std::string title = _("Save Python Script");

        return file_save_single_requester(
            title.c_str(), kPYTHON_PATTERN.c_str(), startfile, false);
    }

    /**
     * Opens a file requester to load a .py file.
     *
     * @param startfile  start filename (directory)
     *
     * @return  opened python file or empty
     */
    std::string open_python_file(const char* startfile)
    {
        std::string kPYTHON_PATTERN = "Python (*.py)\t";

        std::string title = _("Load Python Script");

        return file_single_requester(
            title.c_str(), kPYTHON_PATTERN.c_str(), startfile, false);
    }

    /**
     * Opens a file requester to load a lut file.
     *
     * @param startfile  start filename (directory)
     *
     * @return  opened lut file or empty
     */
    std::string open_lut_file(const char* startfile)
    {
        std::string lut_pattern;
        for (const auto& i : tl::timeline::getLUTFormatExtensions())
        {
            if (!lut_pattern.empty())
                lut_pattern += ",";
            lut_pattern += i.substr(1, i.size()); // no period
        }
        std::string kLUT_PATTERN = _("LUTS (*.{") + lut_pattern + "})\t";

        std::string title = _("Load LUT");

        return file_single_requester(
            title.c_str(), kLUT_PATTERN.c_str(), startfile, false);
    }

    /**
     * Opens a file requester to load subtitle files
     *
     * @param startfile  start filename (directory)
     *
     * @return  opened subtitle file or empty
     */
    std::string open_subtitle_file(const char* startfile)
    {
        std::string kSUBTITLE_PATTERN =
            _("Subtitles (*.{") + kSubtitlePattern + "})\n";

        std::string title = _("Load Subtitle");

        return file_single_requester(
            title.c_str(), kSUBTITLE_PATTERN.c_str(), startfile, false);
    }

    /**
     * Opens a file requester to load audio files
     *
     * @param startfile  start filename (directory)
     *
     * @return  opened audio file or null
     */
    std::string open_audio_file(const char* startfile)
    {
        auto context = App::app->getContext();
        std::string kAUDIO_PATTERN =
            _("Audios (*.{") + getAudioPattern(context) + "})";

        std::string title = _("Load Audio");

        return file_single_requester(
            title.c_str(), kAUDIO_PATTERN.c_str(), startfile, true);
    }

    std::string save_single_image(const char* startdir)
    {
        auto context = App::app->getContext();
        const std::string kIMAGE_PATTERN =
            _("Images (*.{") + getImagePattern(context) + "})";
        const std::string kALL_PATTERN = kIMAGE_PATTERN;

        std::string title = _("Save Single Frame");

        if (!startdir)
            startdir = "";

        const std::string& file = file_save_single_requester(
            title.c_str(), kALL_PATTERN.c_str(), startdir, true);

        return file;
    }
    std::string save_movie_or_sequence_file(const char* startdir)
    {
        auto context = App::app->getContext();
        // const std::string kAUDIO_PATTERN =
        //     ("Audios (*.{") + getAudioPattern(context) + "})\t";
        const std::string kIMAGE_PATTERN =
            _("Images (*.{") + getImagePattern(context) + "})\t";
        const std::string kMOVIE_PATTERN =
            _("Movies (*.{") + getMoviePattern(context) + "})\t";
        const std::string kAUDIO_PATTERN =
            _("Audios (*.{") + getAudioPattern(context) + "})\t";
        const std::string kSESSION_PATTERN =
            _("Sessions (*.{") + kSessionPattern + "})";
        const std::string kALL_PATTERN =
            _("All (*.{") + getImagePattern(context) + "," +
            getMoviePattern(context) + "," + kSessionPattern + "})\t" +
            kIMAGE_PATTERN + kMOVIE_PATTERN + kAUDIO_PATTERN + kSESSION_PATTERN;

        std::string title = _("Save Movie or Sequence");

        if (!startdir)
            startdir = "";

        const std::string& file = file_save_single_requester(
            title.c_str(), kALL_PATTERN.c_str(), startdir, true);

        return file;
    }

    std::string save_pdf(const char* startdir)
    {
        const std::string kPDF_PATTERN = _("Acrobat PDF (*.{pdf})");
        const std::string kALL_PATTERN = kPDF_PATTERN;

        std::string title = _("Save Annotations to PDF");

        if (!startdir)
            startdir = "";

        std::string file = file_save_single_requester(
            title.c_str(), kALL_PATTERN.c_str(), startdir, true);

        if (file.empty())
            return file;

        if (file.substr(file.size() - 4, file.size()) != ".pdf")
        {
            file += ".pdf";
        }

        return file;
    }

    std::string open_session_file(const char* startdir)
    {
        const std::string kSESSION = _("Session");
        const std::string kSESSION_PATTERN = kSESSION + " (*.{mrv2s})";
        const std::string kALL_PATTERN = kSESSION_PATTERN;

        std::string title = _("Open Session");

        if (!startdir)
            startdir = "";

        const std::string& file = file_single_requester(
            title.c_str(), kALL_PATTERN.c_str(), startdir, true);

        return file;
    }

    std::string save_session_file(const char* startdir)
    {
        const std::string kSESSION = _("Session");
        const std::string kSESSION_PATTERN = kSESSION + " (*.{mrv2s})";
        const std::string kALL_PATTERN = kSESSION_PATTERN;

        std::string title = _("Save Session");

        if (!startdir)
            startdir = "";

        std::string file = file_save_single_requester(
            title.c_str(), kALL_PATTERN.c_str(), startdir, true);
        if (file.empty())
            return file;

        if (file.substr(file.size() - 6, file.size()) != ".mrv2s")
        {
            file += ".mrv2s";
        }

        return file;
    }

    std::string open_ocio_config(const char* startfile)
    {
        std::string kOCIO_PATTERN = _("OCIO config (*.{") + kOCIOPattern + "})";
        std::string title = _("Load OCIO Config");

        std::string file = file_single_requester(
            title.c_str(), kOCIO_PATTERN.c_str(), startfile, false);
        return file;
    }

} // namespace mrv
