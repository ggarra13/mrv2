// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/App.h"

#include "mrvPy/CmdsAux.h"

namespace mrv
{
    namespace settings
    {

        double readAhead()
        {
            auto settings = settingsObject();
            std_any any = settings->value("Cache/ReadAhead");
            return std_any_cast<double>(any);
        }

        double readBehind()
        {
            auto settings = settingsObject();
            std_any any = settings->value("Cache/ReadBehind");
            return std_any_cast<double>(any);
        }

        void setReadAhead(const double value)
        {
            mrv::App* app = mrv::App::application();
            auto settings = settingsObject();
            settings->setValue("Cache/ReadAhead", value);
            app->_cacheUpdate();
            if (settingsPanel)
                settingsPanel->refresh();
        }

        void setReadBehind(const double value)
        {
            mrv::App* app = mrv::App::application();
            auto settings = settingsObject();
            settings->setValue("Cache/ReadBehind", value);
            app->_cacheUpdate();
            if (settingsPanel)
                settingsPanel->refresh();
        }

        void setFileSequenceAudio(const timeline::FileSequenceAudio value)
        {
            auto settings = settingsObject();
            settings->setValue("FileSequence/Audio", static_cast<int>(value));
            if (settingsPanel)
                settingsPanel->refresh();
        }

        timeline::FileSequenceAudio fileSequenceAudio()
        {
            auto settings = settingsObject();
            std_any any = settings->value("FileSequence/Audio");
            const timeline::FileSequenceAudio value =
                static_cast<timeline::FileSequenceAudio>(
                    std_any_cast<int>(any));
            return value;
        }

        void setFileSequenceAudioFileName(const std::string& value)
        {
            auto settings = settingsObject();
            settings->setValue("FileSequence/AudioFileName", value);
            if (settingsPanel)
                settingsPanel->refresh();
        }

        std::string fileSequenceAudioFileName()
        {
            auto settings = settingsObject();
            std_any any = settings->value("FileSequence/AudioFileName");
            std::string value = std_any_cast<std::string>(any);
            return value;
        }

        void setFileSequenceAudioDirectory(const std::string& value)
        {
            auto settings = settingsObject();
            settings->setValue("FileSequence/AudioDirectory", value);
            if (settingsPanel)
                settingsPanel->refresh();
        }

        std::string fileSequenceAudioDirectory()
        {
            auto settings = settingsObject();
            std_any any = settings->value("FileSequence/AudioDirectory");
            std::string value = std_any_cast<std::string>(any);
            return value;
        }

        void setMaxFileSequenceDigits(const int value)
        {
            auto settings = settingsObject();
            settings->setValue("Misc/MaxFileSequenceDigits", value);
            if (settingsPanel)
                settingsPanel->refresh();
        }

        int maxFileSequenceDigits()
        {
            auto settings = settingsObject();
            std_any any = settings->value("Misc/MaxFileSequenceDigits");
            int value = std_any_cast<int>(any);
            return value;
        }

        void setTimerMode(const timeline::TimerMode value)
        {
            auto settings = settingsObject();
            settings->setValue(
                "Performance/TimerMode", static_cast<int>(value));
            if (settingsPanel)
                settingsPanel->refresh();
        }

        timeline::TimerMode timerMode()
        {
            auto settings = settingsObject();
            std_any any = settings->value("Performance/TimerMode");
            timeline::TimerMode value =
                static_cast<timeline::TimerMode>(std_any_cast<int>(any));
            return value;
        }

        void
        setAudioBufferFrameCount(const timeline::AudioBufferFrameCount value)
        {
            auto settings = settingsObject();
            settings->setValue(
                "Performance/AudioBufferFrameCount", static_cast<int>(value));
            if (settingsPanel)
                settingsPanel->refresh();
        }

        timeline::AudioBufferFrameCount audioBufferFrameCount()
        {
            auto settings = settingsObject();
            std_any any = settings->value("Performance/AudioBufferFrameCount");
            timeline::AudioBufferFrameCount value =
                static_cast<timeline::AudioBufferFrameCount>(
                    std_any_cast<int>(any));
            return value;
        }

        void setVideoRequests(const int value)
        {
            auto settings = settingsObject();
            settings->setValue("Performance/VideoRequestCount", value);
            if (settingsPanel)
                settingsPanel->refresh();
        }

        int videoRequests()
        {
            auto settings = settingsObject();
            std_any any = settings->value("Performance/VideoRequestCount");
            int value = std_any_cast<int>(any);
            return value;
        }

        void setAudioRequests(const int value)
        {
            auto settings = settingsObject();
            settings->setValue("Performance/AudioRequestCount", value);
            if (settingsPanel)
                settingsPanel->refresh();
        }

        int audioRequests()
        {
            auto settings = settingsObject();
            std_any any = settings->value("Performance/AudioRequestCount");
            int value = std_any_cast<int>(any);
            return value;
        }

        void setSequenceThreadCount(const int value)
        {
            auto settings = settingsObject();
            settings->setValue("Performance/SequenceThreadCount", value);
            if (settingsPanel)
                settingsPanel->refresh();
        }

        int sequenceThreadCount()
        {
            auto settings = settingsObject();
            std_any any = settings->value("Performance/SequenceThreadCount");
            int value = std_any_cast<int>(any);
            return value;
        }

        void setFFmpegYUVToRGBConversion(const bool value)
        {
            auto settings = settingsObject();
            settings->setValue(
                "Performance/FFmpegYUVToRGBConversion", (int)value);
            if (settingsPanel)
                settingsPanel->refresh();
        }

        bool FFmpegYUVToRGBConversion()
        {
            auto settings = settingsObject();
            std_any any =
                settings->value("Performance/FFmpegYUVToRGBConversion");
            bool value = static_cast<bool>(std_any_cast<int>(any));
            return value;
        }

        void setFFmpegThreadCount(const int value)
        {
            auto settings = settingsObject();
            settings->setValue("Performance/FFmpegThreadCount", value);
            if (settingsPanel)
                settingsPanel->refresh();
        }

        int FFmpegThreadCount()
        {
            auto settings = settingsObject();
            std_any any = settings->value("Performance/FFmpegThreadCount");
            int value = std_any_cast<int>(any);
            return value;
        }
    } // namespace settings

} // namespace mrv

void mrv2_settings(py::module& m)
{
    using namespace tl;

    py::module settings = m.def_submodule("settings");
    settings.doc() = _(R"PYTHON(
Settings module.

Contains all settings functions.
)PYTHON");

    settings.def(
        "readAhead", &mrv::settings::readAhead,
        _("Retrieve Read Ahead cache in seconds."));

    settings.def(
        "setReadAhead", &mrv::settings::setReadAhead,
        _("Set Read Ahead cache in seconds."));

    settings.def(
        "readBehind", &mrv::settings::readBehind,
        _("Retrieve Read Behind cache in seconds."));

    settings.def(
        "setReadBehind", &mrv::settings::setReadBehind,
        _("Set Read Behind cache in seconds."));

    py::module sequence = settings.def_submodule("sequence");

    sequence.def(
        "setAudio", &mrv::settings::setFileSequenceAudio,
        _("Set file sequence audio."));

    sequence.def(
        "audio", &mrv::settings::fileSequenceAudio,
        _("Get file sequence audio."));

    sequence.def(
        "setAudioFileName", &mrv::settings::setFileSequenceAudioFileName,
        _("Set file sequence audio file name."));

    sequence.def(
        "audioFileName", &mrv::settings::fileSequenceAudioFileName,
        _("Get file sequence audio file name."));

    sequence.def(
        "setAudioDirectory", &mrv::settings::setFileSequenceAudioDirectory,
        _("Set file sequence audio directory."));

    sequence.def(
        "audioDirectory", &mrv::settings::fileSequenceAudioDirectory,
        _("Get file sequence audio directory."));

    py::module misc = settings.def_submodule("misc");

    misc.def(
        "setMaxFileSequenceDigits", &mrv::settings::setMaxFileSequenceDigits,
        _("Set maximum file sequence digits."));

    misc.def(
        "maxFileSequenceDigits", &mrv::settings::maxFileSequenceDigits,
        _("Get maximum file sequence digits."));

    py::module performance = settings.def_submodule("performance");

    performance.def(
        "setTimerMode", &mrv::settings::setTimerMode, _("Set Timer Mode."));

    performance.def(
        "timerMode", &mrv::settings::timerMode, _("Get Timer Mode."));

    performance.def(
        "setAudioBufferFrameCount", &mrv::settings::setAudioBufferFrameCount,
        _("Set Audio Buffer Frame Count."));

    performance.def(
        "audioBufferFrameCount", &mrv::settings::audioBufferFrameCount,
        _("Get Audio Buffer Frame Count."));

    performance.def(
        "setVideoRequests", &mrv::settings::setVideoRequests,
        _("Set Video Request Count."));

    performance.def(
        "videoRequests", &mrv::settings::videoRequests,
        _("Get Video Request Count."));

    performance.def(
        "setAudioRequests", &mrv::settings::setAudioRequests,
        _("Set Audio Request Count."));

    performance.def(
        "audioRequests", &mrv::settings::audioRequests,
        _("Get Audio Request Count."));

    performance.def(
        "setSequenceThreadCount", &mrv::settings::setSequenceThreadCount,
        _("Set Sequence Thread Count."));

    performance.def(
        "sequenceThreadCount", &mrv::settings::sequenceThreadCount,
        _("Get Sequence Thread Count."));

    performance.def(
        "setFFmpegYUVToRGBConversion",
        &mrv::settings::setFFmpegYUVToRGBConversion,
        _("Set FFmpeg YUV To RGB Conversion."));

    performance.def(
        "FFmpegYUVToRGBConversion", &mrv::settings::FFmpegYUVToRGBConversion,
        _("Get FFmpeg YUV To RGB Conversion."));

    performance.def(
        "setFFmpegThreadCount", &mrv::settings::setFFmpegThreadCount,
        _("Set FFmpeg Thread Count."));

    performance.def(
        "FFmpegThreadCount", &mrv::settings::FFmpegThreadCount,
        _("Get FFmpeg Thread Count."));
}
