// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvPy/CmdsAux.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvApp/mrvApp.h"
#include "mrvApp/mrvSettingsObject.h"

#include <pybind11/pybind11.h>
namespace py = pybind11;



namespace mrv2
{
    namespace settings
    {
        using namespace tl;
        using namespace mrv;

        bool checkForUpdates()
        {
            return (bool)App::ui->uiPrefs->uiPrefsCheckForUpdates->value();
        }

        /**
         * @brief Returns the Gigabytes memory setting.
         *
         *
         * @return in gigabytes.
         */
        int memory()
        {
            return mrv::settings()->getValue<int>("Cache/GBytes");
        }

        /**
         * @brief Sets the Read Behind Cache setting.
         *
         * @param value in seconds.
         */
        void setMemory(const int value)
        {
            if (value < 0)
                throw std::invalid_argument(_("Value less than 0"));
            mrv::App* app = mrv::App::app;
            mrv::settings()->setValue("Cache/GBytes", value);
            app->cacheUpdate();
            if (panel::settingsPanel)
                panel::settingsPanel->refresh();
        }

        /**
         * @brief Returns the Read Ahead Cache setting.
         *
         *
         * @return in seconds.
         */
        double readAhead()
        {
            return mrv::settings()->getValue<double>("Cache/ReadAhead");
        }

        /**
         * @brief Returns the Read Behind Cache setting.
         *
         *
         * @return in seconds.
         */
        double readBehind()
        {
            return mrv::settings()->getValue<double>("Cache/ReadBehind");
        }

        /**
         * @brief Sets the Read Ahead Cache setting.
         *
         * @param value in seconds.
         */
        void setReadAhead(const double value)
        {
            mrv::App* app = mrv::App::app;
            mrv::settings()->setValue("Cache/ReadAhead", value);
            app->cacheUpdate();
            if (panel::settingsPanel)
                panel::settingsPanel->refresh();
        }

        /**
         * @brief Sets the Read Behind Cache setting.
         *
         * @param value in seconds.
         */
        void setReadBehind(const double value)
        {
            mrv::App* app = mrv::App::app;
            mrv::settings()->setValue("Cache/ReadBehind", value);
            app->cacheUpdate();
            if (panel::settingsPanel)
                panel::settingsPanel->refresh();
        }

        void setFileSequenceAudio(const timeline::FileSequenceAudio value)
        {
            mrv::settings()->setValue(
                "FileSequence/Audio", static_cast<int>(value));
            if (panel::settingsPanel)
                panel::settingsPanel->refresh();
        }

        timeline::FileSequenceAudio fileSequenceAudio()
        {
            return static_cast<timeline::FileSequenceAudio>(
                mrv::settings()->getValue<int>("FileSequence/Audio"));
        }

        /**
         * @brief Sets the file sequence audio file name.
         *
         * @param value a string.
         */
        void setFileSequenceAudioFileName(const std::string& value)
        {
            mrv::settings()->setValue("FileSequence/AudioFileName", value);
            if (panel::settingsPanel)
                panel::settingsPanel->refresh();
        }

        /**
         * @brief Returns the file sequence audio file name.
         *
         * @return a string
         */
        std::string fileSequenceAudioFileName()
        {
            return mrv::settings()->getValue<std::string>(
                "FileSequence/AudioFileName");
        }

        /**
         * @brief Set the file sequence audio directory.
         *
         * @param value a string
         */
        void setFileSequenceAudioDirectory(const std::string& value)
        {
            mrv::settings()->setValue("FileSequence/AudioDirectory", value);
            if (panel::settingsPanel)
                panel::settingsPanel->refresh();
        }

        /**
         * @brief Returns the file sequence audio directory.
         *
         *
         * @return a string
         */
        std::string fileSequenceAudioDirectory()
        {
            return mrv::settings()->getValue<std::string>(
                "FileSequence/AudioDirectory");
        }

        /**
         * Set the maximum file sequqnce digits.
         *
         * @param value a value of 1 or more.
         */
        void setMaxFileSequenceDigits(const int value)
        {
            if (value < 1)
                throw std::invalid_argument("Value less than 1.");
            mrv::settings()->setValue("Misc/MaxFileSequenceDigits", value);
            if (panel::settingsPanel)
                panel::settingsPanel->refresh();
        }

        /**
         * Returns the maximum file sequence digits.
         *
         * @return an interger of 1 or higher.
         */
        int maxFileSequenceDigits()
        {
            return mrv::settings()->getValue<int>("Misc/MaxFileSequenceDigits");
        }

        void setTimerMode(const timeline::TimerMode value)
        {
            mrv::settings()->setValue(
                "Performance/TimerMode", static_cast<int>(value));
            if (panel::settingsPanel)
                panel::settingsPanel->refresh();
        }

        timeline::TimerMode timerMode()
        {
            return static_cast<timeline::TimerMode>(
                mrv::settings()->getValue<int>("Performance/TimerMode"));
        }

        void setAudioBufferFrameCount(const int value)
        {
            mrv::settings()->setValue(
                "Performance/AudioBufferFrameCount", static_cast<int>(value));
            if (panel::settingsPanel)
                panel::settingsPanel->refresh();
        }

        int audioBufferFrameCount()
        {
            return mrv::settings()->getValue<int>(
                "Performance/AudioBufferFrameCount");
        }

        /**
         * @brief Set the video request count.
         *
         * @param value and integer > 0.
         */
        void setVideoRequests(const int value)
        {
            if (value < 1)
                throw std::invalid_argument("Invalid value less than 1");
            mrv::settings()->setValue("Performance/VideoRequestCount", value);
            if (panel::settingsPanel)
                panel::settingsPanel->refresh();
        }

        /**
         * @brief Returns the video request count.
         *
         * @return an integer > 0
         */
        int videoRequests()
        {
            return mrv::settings()->getValue<int>(
                "Performance/VideoRequestCount");
        }

        /**
         * @brief Set the audio request count.
         *
         * @param value and integer > 0.
         */
        void setAudioRequests(const int value)
        {
            if (value < 1)
                throw std::runtime_error("Invalid value less than 1");
            mrv::settings()->setValue("Performance/AudioRequestCount", value);
            if (panel::settingsPanel)
                panel::settingsPanel->refresh();
        }

        /**
         * @brief Returns the audio request count.
         *
         * @return an integer > 0
         */
        int audioRequests()
        {
            return mrv::settings()->getValue<int>(
                "Performance/AudioRequestCount");
        }

        /**
         * @brief Set the sequence thread count.
         *
         * @param value and integer > 0.
         */
        void setSequenceThreadCount(const int value)
        {
            if (value < 1)
                throw std::invalid_argument("Invalid value less than 1");
            mrv::settings()->setValue("Performance/SequenceThreadCount", value);
            if (panel::settingsPanel)
                panel::settingsPanel->refresh();
        }

        /**
         * @brief Returns the sequence thread count.
         *
         * @return an integer > 0
         */
        int sequenceThreadCount()
        {
            return mrv::settings()->getValue<int>(
                "Performance/SequenceThreadCount");
        }

        /**
         * @brief Sets the FFmpeg YUV to RGB conversion.
         *
         * @param value
         */
        void setFFmpegYUVToRGBConversion(const bool value)
        {
            mrv::settings()->setValue(
                "Performance/FFmpegYUVToRGBConversion", (int)value);
            if (panel::settingsPanel)
                panel::settingsPanel->refresh();
        }

        /**
         * @brief Returns the FFmpeg YUV to RGB conversion.
         *
         * @return a bool
         */
        bool FFmpegYUVToRGBConversion()
        {
            return mrv::settings()->getValue<bool>(
                "Performance/FFmpegYUVToRGBConversion");
        }

        /**
         * @brief Sets the FFmpeg YUV to RGB conversion.
         *
         * @param value
         */
        void setFFmpegColorAccuracy(const bool value)
        {
            mrv::settings()->setValue(
                "Performance/FFmpegColorAccuracy", (int)value);
            if (panel::settingsPanel)
                panel::settingsPanel->refresh();
        }

        /**
         * @brief Returns the FFmpeg's Color Accuracy.
         *
         * @return a bool
         */
        bool FFmpegColorAccuracy()
        {
            return mrv::settings()->getValue<bool>(
                "Performance/FFmpegColorAccuracy");
        }

        /**
         * @brief Set the FFmpeg thread count.
         *
         * @param value and integer > 0.
         */
        void setFFmpegThreadCount(const int value)
        {
            if (value < 0)
                throw std::invalid_argument("Invalid value less than 0");
            mrv::settings()->setValue("Performance/FFmpegThreadCount", value);
            if (panel::settingsPanel)
                panel::settingsPanel->refresh();
        }

        /**
         * @brief Returns the FFmpeg thread count.
         *
         * @return an integer >= 0
         */
        int FFmpegThreadCount()
        {
            return mrv::settings()->getValue<int>(
                "Performance/FFmpegThreadCount");
        }
    } // namespace settings

} // namespace mrv2

void mrv2_settings(py::module& m)
{
    using namespace tl;

    py::module settings = m.def_submodule("settings");
    settings.doc() = _(R"PYTHON(
Settings module.

Contains all settings functions.
)PYTHON");

    settings.def(
        "checkForUpdates", &mrv2::settings::checkForUpdates,
        _("Returns whether to check for updates at start up."));

    settings.def(
        "memory", &mrv2::settings::memory,
        _("Retrieve the cache memory setting in gigabytes."));

    settings.def(
        "setMemory", &mrv2::settings::setMemory,
        _("Set the cache memory setting in gigabytes."));

    settings.def(
        "readAhead", &mrv2::settings::readAhead,
        _("Retrieve Read Ahead cache in seconds."));

    settings.def(
        "setReadAhead", &mrv2::settings::setReadAhead,
        _("Set Read Ahead cache in seconds."));

    settings.def(
        "readBehind", &mrv2::settings::readBehind,
        _("Retrieve Read Behind cache in seconds."));

    settings.def(
        "setReadBehind", &mrv2::settings::setReadBehind,
        _("Set Read Behind cache in seconds."));

    py::module sequence = settings.def_submodule("sequence");

    sequence.def(
        "setAudio", &mrv2::settings::setFileSequenceAudio,
        _("Set file sequence audio."));

    sequence.def(
        "audio", &mrv2::settings::fileSequenceAudio,
        _("Get file sequence audio."));

    sequence.def(
        "setAudioFileName", &mrv2::settings::setFileSequenceAudioFileName,
        _("Set file sequence audio file name."));

    sequence.def(
        "audioFileName", &mrv2::settings::fileSequenceAudioFileName,
        _("Get file sequence audio file name."));

    sequence.def(
        "setAudioDirectory", &mrv2::settings::setFileSequenceAudioDirectory,
        _("Set file sequence audio directory."));

    sequence.def(
        "audioDirectory", &mrv2::settings::fileSequenceAudioDirectory,
        _("Get file sequence audio directory."));

    py::module misc = settings.def_submodule("misc");

    misc.def(
        "setMaxFileSequenceDigits", &mrv2::settings::setMaxFileSequenceDigits,
        _("Set maximum file sequence digits."));

    misc.def(
        "maxFileSequenceDigits", &mrv2::settings::maxFileSequenceDigits,
        _("Get maximum file sequence digits."));

    py::module performance = settings.def_submodule("performance");

    performance.def(
        "setTimerMode", &mrv2::settings::setTimerMode, _("Set Timer Mode."));

    performance.def(
        "timerMode", &mrv2::settings::timerMode, _("Get Timer Mode."));

    performance.def(
        "setAudioBufferFrameCount", &mrv2::settings::setAudioBufferFrameCount,
        _("Set Audio Buffer Frame Count."));

    performance.def(
        "audioBufferFrameCount", &mrv2::settings::audioBufferFrameCount,
        _("Get Audio Buffer Frame Count."));

    performance.def(
        "setVideoRequests", &mrv2::settings::setVideoRequests,
        _("Set Video Request Count."));

    performance.def(
        "videoRequests", &mrv2::settings::videoRequests,
        _("Get Video Request Count."));

    performance.def(
        "setAudioRequests", &mrv2::settings::setAudioRequests,
        _("Set Audio Request Count."));

    performance.def(
        "audioRequests", &mrv2::settings::audioRequests,
        _("Get Audio Request Count."));

    performance.def(
        "setSequenceThreadCount", &mrv2::settings::setSequenceThreadCount,
        _("Set Sequence Thread Count."));

    performance.def(
        "sequenceThreadCount", &mrv2::settings::sequenceThreadCount,
        _("Get Sequence Thread Count."));

    performance.def(
        "setFFmpegYUVToRGBConversion",
        &mrv2::settings::setFFmpegYUVToRGBConversion,
        _("Set FFmpeg YUV To RGB Conversion."));

    performance.def(
        "ffmpegYUVToRGBConversion", &mrv2::settings::FFmpegYUVToRGBConversion,
        _("Get FFmpeg YUV To RGB Conversion."));

    performance.def(
        "setFFmpegColorAccuracy", &mrv2::settings::setFFmpegColorAccuracy,
        _("Set FFmpeg Color Accuracy."));

    performance.def(
        "ffmpegColorAccuracy", &mrv2::settings::FFmpegColorAccuracy,
        _("Get FFmpeg Color Accuracy."));

    performance.def(
        "setFFmpegThreadCount", &mrv2::settings::setFFmpegThreadCount,
        _("Set FFmpeg Thread Count."));

    performance.def(
        "FFmpegThreadCount", &mrv2::settings::FFmpegThreadCount,
        _("Get FFmpeg Thread Count."));
}
