// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <map>

#include <tlCore/StringFormat.h>
#include <tlTimeline/TimelinePlayer.h>

#include "mrvFl/mrvIO.h"

#include <mrvPlayApp/mrvSettingsObject.h>

namespace mrv
{
    namespace
    {
        const char* kModule = "SettingsObject";
        const int recentFilesMax = 10;
    }

    struct SettingsObject::Private
    {
        std::map<std::string, std_any> defaultValues;
        std::map<std::string, std_any> settings;
        std::vector<std::string> recentFiles;
        TimeObject*              timeObject = nullptr;
        bool toolTipsEnabled = true;
    };

    SettingsObject::SettingsObject( bool reset,
                                    TimeObject* timeObject ) :
        _p(new Private)
    {
        TLRENDER_P();


        
        if (reset)
        {
            p.settings.clear();
        }

        DBG;

        p.defaultValues["Timeline/Thumbnails"] = true;
        p.defaultValues["Timeline/StopOnScrub"] = false;
        p.defaultValues["Cache/ReadAhead"] = 4.0;
        p.defaultValues["Cache/ReadBehind"] = 0.4;
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
        p.defaultValues["Performance/FFmpegYUVToRGBConversion"] = false;
        p.defaultValues["Misc/MaxFileSequenceDigits"] = 9;
        DBG;

        // int size = p.settings.beginReadArray("RecentFiles"));
        // for (int i = 0; i < size; ++i)
        // {
        //     p.settings.setArrayIndex(i);
        //     p.recentFiles.push_back(p.settings.value("File").toString().toUtf8().data());
        // }
        // p.settings.endArray();

        DBG;
        std_any value = p.settings["Misc/ToolTipsEnabled"];
        if ( value.empty() ) value = true;
        p.toolTipsEnabled = std_any_cast<bool>(value);
        DBG;

        p.timeObject = timeObject;
        value = p.settings[ "TimeUnits" ];
        if ( value.empty() )
            value = p.timeObject->units();
        
        p.timeObject->setUnits(std_any_cast<mrv::TimeUnits>(value));
    }

    SettingsObject::~SettingsObject()
    {
        TLRENDER_P();

        // p.settings.beginWriteArray("RecentFiles"));
        // for (size_t i = 0; i < p.recentFiles.size(); ++i)
        // {
        //     p.settings.setArrayIndex(i);
        //     p.settings.setValue("File", p.recentFiles[i]);
        // }
        // p.settings.endArray();

        p.settings["Misc/ToolTipsEnabled"] = p.toolTipsEnabled;
        p.settings["TimeUnits"] = p.timeObject->units();
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

    bool SettingsObject::hasToolTipsEnabled() const
    {
        return _p->toolTipsEnabled;
    }

    void SettingsObject::setValue(const std::string& name, const std_any& value)
    {
        _p->settings[name] = value;
    }

    void SettingsObject::setDefaultValue(const std::string& name, const std_any& value)
    {
        _p->defaultValues[name] = value;
    }

    void SettingsObject::reset()
    {
        TLRENDER_P();
        for (auto i = p.defaultValues.begin(); i != p.defaultValues.end(); ++i)
        {
            p.settings[i->first] = i->second;
        }
        p.recentFiles.clear();
        p.toolTipsEnabled = true;
    }

    void SettingsObject::addRecentFile(const std::string& fileName)
    {
        TLRENDER_P();
        p.recentFiles.insert(p.recentFiles.begin(), fileName);
        while (p.recentFiles.size() > recentFilesMax)
        {
            p.recentFiles.pop_back();
        }
    }

    void SettingsObject::setToolTipsEnabled(bool value)
    {
        TLRENDER_P();
        if (value == p.toolTipsEnabled)
            return;
        p.toolTipsEnabled = value;
    }

}
