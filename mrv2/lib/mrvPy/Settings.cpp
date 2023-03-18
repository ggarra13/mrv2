// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/App.h"


namespace mrv
{
    namespace settings
    {

        double readAhead()
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            std_any any = settingsObject->value("Cache/ReadAhead");
            return std_any_cast<double>(any);
        }

        double readBehind()
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            std_any any = settingsObject->value("Cache/ReadBehind");
            return std_any_cast<double>(any);
        }
        
        void setReadAhead(const double value)
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            settingsObject->setValue("Cache/ReadAhead", value);
            app->_cacheUpdate();
            if ( settingsPanel ) settingsPanel->refresh();
        }

        void setReadBehind(const double value)
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            settingsObject->setValue("Cache/ReadBehind", value);
            app->_cacheUpdate();
            if ( settingsPanel ) settingsPanel->refresh();
        }

        void setFileSequenceAudio(const timeline::FileSequenceAudio value)
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            settingsObject->setValue("FileSequence/Audio",
                                     static_cast<int>(value));
            if ( settingsPanel ) settingsPanel->refresh();
        }

        void setFileSequenceAudioFileName(const std::string& value)
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            settingsObject->setValue("FileSequence/AudioFileName", value);
            if ( settingsPanel ) settingsPanel->refresh();
        }

        void setFileSequenceAudioDirectory(const std::string& value)
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            settingsObject->setValue("FileSequence/AudioDirectory", value);
            if ( settingsPanel ) settingsPanel->refresh();
        }
        

        void setMaxFileSequenceDigits(const int value)
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            settingsObject->setValue("Misc/MaxFileSequenceDigits", value);
            if ( settingsPanel ) settingsPanel->refresh();
        }

        void setTimerMode(const timeline::TimerMode value)
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            settingsObject->setValue("Performance/TimerMode",
                                     static_cast<int>(value));
            if ( settingsPanel ) settingsPanel->refresh();
        }

        void setAudioBufferFrameCount(
            const timeline::AudioBufferFrameCount value)
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            settingsObject->setValue("Performance/AudioBufferFrameCount",
                                     static_cast<int>(value));
            if ( settingsPanel ) settingsPanel->refresh();
        }

        void setVideoRequests(const int value)
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            settingsObject->setValue("Performance/VideoRequestCount", value);
            if ( settingsPanel ) settingsPanel->refresh();
        }
        
        void setAudioRequests(const int value)
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            settingsObject->setValue("Performance/AudioRequestCount", value);
            if ( settingsPanel ) settingsPanel->refresh();
        }
        
        void setSequenceThreadCount(const int value)
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            settingsObject->setValue("Performance/SequenceThreadCount", value);
            if ( settingsPanel ) settingsPanel->refresh();
        }
        
        void setFFmpegYUVToRGBConversion(const bool value)
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            settingsObject->setValue("Performance/FFmpegYUVToRGBConversion",
                                     (int)value);
            if ( settingsPanel ) settingsPanel->refresh();
        }
        
        void setFFmpegThreadCount(const int value)
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            settingsObject->setValue("Performance/FFmpegThreadCount", value);
            if ( settingsPanel ) settingsPanel->refresh();
        }
    }
    
}



void mrv2_settings(py::module& m)
{
    using namespace tl;

    py::module settings = m.def_submodule("settings");
    
    settings.def("readAhead", &mrv::settings::readAhead,
                 _("Retrieve Read Ahead cache in seconds."));
    
    settings.def("setReadAhead", &mrv::settings::setReadAhead,
                 _("Set Read Ahead cache in seconds."));
    
    settings.def("readBehind", &mrv::settings::readBehind,
                 _("Retrieve Read Behind cache in seconds."));
    
    settings.def("setReadBehind", &mrv::settings::setReadBehind,
                 _("Set Read Behind cache in seconds."));
    
    py::module sequence = settings.def_submodule("sequence");
    
    sequence.def("setAudio", &mrv::settings::setFileSequenceAudio,
                 _("Set file sequence audio."));
    
    sequence.def("setAudioFileName",
                 &mrv::settings::setFileSequenceAudioFileName,
                 _("Set file sequence audio file name."));

    sequence.def("setAudioDirectory",
                 &mrv::settings::setFileSequenceAudioDirectory,
                 _("Set file sequence audio directory."));

    py::module misc = settings.def_submodule("misc");
    
    misc.def("setMaxFileSequenceDigits",
                 &mrv::settings::setMaxFileSequenceDigits,
                 _("Set maximum file sequence digits."));
    
    py::module performance = settings.def_submodule("sequence");
    
    performance.def("setTimerMode",
                 &mrv::settings::setTimerMode,
                 _("Set Timer Mode."));
    
    performance.def("setAudioBufferFrameCount",
                 &mrv::settings::setAudioBufferFrameCount,
                 _("Set Audio Buffer Frame Count."));
    
    performance.def("setVideoRequests",
                 &mrv::settings::setVideoRequests,
                 _("Set Video Request Count."));
    
    performance.def("setAudioRequests",
                 &mrv::settings::setAudioRequests,
                 _("Set Audio Request Count."));
    
    performance.def("setSequenceThreadCount",
                 &mrv::settings::setSequenceThreadCount,
                 _("Set Sequence Thread Count."));
    
    performance.def("setFFmpegYUVToRGBConversion",
                 &mrv::settings::setFFmpegYUVToRGBConversion,
                 _("Set FFmpeg YUV To RGB Conversion."));
    
    performance.def("setFFmpegThreadCount",
                 &mrv::settings::setFFmpegThreadCount,
                 _("Set FFmpeg Thread Count."));
    
}
