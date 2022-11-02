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
        int toolTipsEnabled = 1;
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

        p.defaultValues["Timeline/Thumbnails"] = 1;
        p.defaultValues["Timeline/StopOnScrub"] = 0;
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
        p.defaultValues["Performance/FFmpegYUVToRGBConversion"] = 0;
        p.defaultValues["Misc/MaxFileSequenceDigits"] = 9;
        p.defaultValues["Misc/ToolTipsEnabled"] = 1;
        
        p.timeObject = timeObject;
        p.defaultValues["TimeUnits"] = (int)p.timeObject->units();
        DBG;

        // int size = p.settings.beginReadArray("RecentFiles"));
        // for (int i = 0; i < size; ++i)
        // {
        //     p.settings.setArrayIndex(i);
        //     p.recentFiles.push_back(p.settings.value("File").toString().toUtf8().data());
        // }
        // p.settings.endArray();
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
    }

    const std::vector<std::string> SettingsObject::keys() const
    {
        TLRENDER_P();
        std::vector< std::string > ret;
        ret.reserve( p.settings.size() + p.defaultValues.size() );
        for ( const auto& m : p.settings )
        {
            ret.push_back( m.first );
        }
        for ( const auto& m : p.defaultValues )
        {
            if ( std::find( ret.begin(), ret.end(), m.first ) != ret.end() )
                continue;
            ret.push_back( m.first );
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
        p.toolTipsEnabled = 1;
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
        p.toolTipsEnabled = (int)value;
    }

}
