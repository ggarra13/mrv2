// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <vector>
#include <map>
#include <algorithm>

#include <tlCore/StringFormat.h>
#include <tlTimeline/TimelinePlayer.h>

#include "mrvFl/mrvIO.h"

#include "mrvApp/mrvSettingsObject.h"

namespace mrv
{
    namespace
    {
        const char* kModule = "SettingsObject";
        const int recentFilesMax = 10;
    } // namespace

    struct SettingsObject::Private
    {
        std::map<std::string, std_any> defaultValues;
        std::map<std::string, std_any> settings;
        std::vector<std::string> recentFiles;
        TimeObject* timeObject = nullptr;
    };

    SettingsObject::SettingsObject(TimeObject* timeObject) :
        _p(new Private)
    {
        TLRENDER_P();

        DBG;

        p.defaultValues["Timeline/Thumbnails"] = 1;
        p.defaultValues["Timeline/StopOnScrub"] = 0;
        p.defaultValues["Cache/ReadAhead"] =
            timeline::PlayerCacheOptions().readAhead.value();
        p.defaultValues["Cache/ReadBehind"] =
            1.0; // timeline::PlayerCacheOptions().readBehind.value();
        p.defaultValues["FileSequence/Audio"] =
            static_cast<int>(timeline::FileSequenceAudio::BaseName);
        p.defaultValues["FileSequence/AudioFileName"] = std::string();
        p.defaultValues["FileSequence/AudioDirectory"] = std::string();
        p.defaultValues["Performance/TimerMode"] =
            static_cast<int>(timeline::TimerMode::System);
        p.defaultValues["Performance/AudioBufferFrameCount"] =
            static_cast<int>(timeline::AudioBufferFrameCount::_256);
        p.defaultValues["Performance/VideoRequestCount"] = 16;
        p.defaultValues["Performance/AudioRequestCount"] = 16;
        p.defaultValues["Performance/SequenceThreadCount"] = 16;
        p.defaultValues["Performance/FFmpegThreadCount"] = 0;
        p.defaultValues["Performance/FFmpegYUVToRGBConversion"] = 0;
        p.defaultValues["Misc/MaxFileSequenceDigits"] = 9;
        p.defaultValues["Misc/ToolTipsEnabled"] = 1;
        p.defaultValues["EnvironmentMap/Sphere/SubdivisionX"] = 36;
        p.defaultValues["EnvironmentMap/Sphere/SubdivisionY"] = 36;
        p.defaultValues["EnvironmentMap/Spin"] = 1;

        p.defaultValues[kTextFont] = 0;
        p.defaultValues[kFontSize] = 52;

        p.defaultValues[kPenColorR] = 0;
        p.defaultValues[kPenColorG] = 255;
        p.defaultValues[kPenColorB] = 0;

        p.defaultValues[kPenSize] = 10;

        p.defaultValues[kGhostPrevious] = 15;
        p.defaultValues[kGhostNext] = 15;

        p.defaultValues[kAllFrames] = 0;

        p.timeObject = timeObject;
        p.defaultValues["TimeUnits"] = (int)p.timeObject->units();
        DBG;
    }

    SettingsObject::~SettingsObject() {}

    const std::vector<std::string> SettingsObject::keys() const
    {
        TLRENDER_P();
        std::vector< std::string > ret;
        ret.reserve(p.settings.size() + p.defaultValues.size());
        for (const auto& m : p.settings)
        {
            ret.push_back(m.first);
        }
        for (const auto& m : p.defaultValues)
        {
            if (std::find(ret.begin(), ret.end(), m.first) != ret.end())
                continue;
            ret.push_back(m.first);
        }
        return ret;
    }

    std_any SettingsObject::value(const std::string& name)
    {
        TLRENDER_P();
        std_any defaultValue;
        auto i = p.settings.find(name);
        if (i != p.settings.end())
        {
            return p.settings[name];
        }

        i = p.defaultValues.find(name);
        if (i != p.defaultValues.end())
        {
            defaultValue = i->second;
        }
        return p.settings[name] = defaultValue;
    }

    const std::vector<std::string>& SettingsObject::recentFiles() const
    {
        return _p->recentFiles;
    }

    void SettingsObject::setValue(const std::string& name, const std_any& value)
    {
        _p->settings[name] = value;
    }

    void SettingsObject::setDefaultValue(
        const std::string& name, const std_any& value)
    {
        _p->defaultValues[name] = value;
    }

    void SettingsObject::reset()
    {
        TLRENDER_P();
        p.settings.clear();
        for (auto i = p.defaultValues.begin(); i != p.defaultValues.end(); ++i)
        {
            p.settings[i->first] = i->second;
        }
        p.recentFiles.clear();
    }

    void SettingsObject::addRecentFile(const std::string& fileName)
    {
        TLRENDER_P();
        if (std::find(p.recentFiles.begin(), p.recentFiles.end(), fileName) !=
            p.recentFiles.end())
            return;
        p.recentFiles.insert(p.recentFiles.begin(), fileName);
        while (p.recentFiles.size() > recentFilesMax)
        {
            p.recentFiles.pop_back();
        }
    }

} // namespace mrv
