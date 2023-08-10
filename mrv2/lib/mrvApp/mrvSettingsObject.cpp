// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <vector>
#include <map>
#include <unordered_set>
#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

#if defined(TLRENDER_USD)
#    include <tlIO/USD.h>
#endif // TLRENDER_USD

#include <tlCore/StringFormat.h>
#include <tlTimeline/Player.h>

#include "mrvCore/mrvMemory.h"

#include "mrvFl/mrvIO.h"

#include "mrvApp/mrvSettingsObject.h"

namespace mrv
{
    namespace
    {
        const char* kModule = "SettingsObject";
        const int kRecentFilesMax = 10;
        const int kRecentHostsMax = 10;
        const int kRecentPythonScriptsMax = 10;
    } // namespace

    struct SettingsObject::Private
    {
        std::map<std::string, std_any> defaultValues;
        std::map<std::string, std_any> settings;
        std::vector<std::string> recentFiles;
        std::vector<std::string> recentHosts;
        std::vector<std::string> pythonScripts;
    };

    SettingsObject::SettingsObject() :
        _p(new Private)
    {
        TLRENDER_P();

        uint64_t totalVirtualMem = 0;
        uint64_t virtualMemUsed = 0;
        uint64_t virtualMemUsedByMe = 0;
        uint64_t totalPhysMem = 0;
        uint64_t physMemUsed = 0;
        uint64_t physMemUsedByMe = 0;
        memory_information(
            totalVirtualMem, virtualMemUsed, virtualMemUsedByMe, totalPhysMem,
            physMemUsed, physMemUsedByMe);
        totalPhysMem /= 1024;

        p.defaultValues["Timeline/Thumbnails"] = 1;
        p.defaultValues["Timeline/StopOnScrub"] = 0;
        p.defaultValues["Audio/Volume"] = 1.0F;
        p.defaultValues["Audio/Mute"] = false;
        p.defaultValues["Cache/GBytes"] = static_cast<int>(totalPhysMem / 2);
        p.defaultValues["Cache/ReadAhead"] =
            timeline::PlayerCacheOptions().readAhead.value();
        p.defaultValues["Cache/ReadBehind"] =
            timeline::PlayerCacheOptions().readBehind.value();
        p.defaultValues["FileSequence/Audio"] =
            static_cast<int>(timeline::FileSequenceAudio::BaseName);
        p.defaultValues["FileSequence/AudioFileName"] = std::string();
        p.defaultValues["FileSequence/AudioDirectory"] = std::string();
        const timeline::PlayerOptions playerOptions;
        p.defaultValues["Performance/TimerMode"] =
            static_cast<int>(playerOptions.timerMode);
        p.defaultValues["Performance/AudioBufferFrameCount"] =
            static_cast<int>(playerOptions.audioBufferFrameCount);
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
        p.defaultValues["TCP/Control/Port"] = std::string("55150");
#if defined(TLRENDER_USD)
        p.defaultValues["usd/renderWidth"] =
            static_cast<int>(usd::RenderOptions().renderWidth);
        p.defaultValues["usd/complexity"] =
            static_cast<float>(usd::RenderOptions().complexity);
        p.defaultValues["usd/drawMode"] =
            static_cast<int>(usd::RenderOptions().drawMode);
        p.defaultValues["usd/enableLighting"] =
            static_cast<int>(usd::RenderOptions().enableLighting);
        p.defaultValues["usd/stageCacheCount"] =
            static_cast<int>(usd::RenderOptions().stageCacheCount);
        p.defaultValues["usd/diskCacheByteCount"] = static_cast<int>(
            usd::RenderOptions().diskCacheByteCount / memory::gigabyte);
#endif
        p.defaultValues[kTextFont] = 0;
        p.defaultValues[kFontSize] = 52;

        p.defaultValues[kPenColorR] = 255;
        p.defaultValues[kPenColorG] = 255;
        p.defaultValues[kPenColorB] = 0;
        p.defaultValues[kPenColorA] = 255;

        p.defaultValues[kLaser] = 0;
        p.defaultValues[kPenSize] = 10;
        p.defaultValues[kSoftBrush] = 0;

        p.defaultValues[kGhostPrevious] = 15;
        p.defaultValues[kGhostNext] = 15;

        p.defaultValues[kAllFrames] = 0;
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

    const std::vector<std::string>& SettingsObject::recentHosts() const
    {
        return _p->recentHosts;
    }

    const std::vector<std::string>& SettingsObject::pythonScripts() const
    {
        return _p->pythonScripts;
    }

    const std::string SettingsObject::pythonScript(size_t index) const
    {
        if (index >= _p->pythonScripts.size())
            return "";
        return _p->pythonScripts[index];
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

        std::unordered_set<std::string> set;
        std::vector< std::string > result;

        p.recentFiles.insert(p.recentFiles.begin(), fileName);

        for (const auto& str : p.recentFiles)
        {
            auto path = fs::canonical(fs::path(str));
            if (set.find(path.generic_string()) == set.end())
            {
                set.insert(path.generic_string());
                result.push_back(str);
            }
        }

        while (result.size() > kRecentFilesMax)
        {
            result.pop_back();
        }

        p.recentFiles = result;
    }

    void SettingsObject::addRecentHost(const std::string& fileName)
    {
        TLRENDER_P();

        std::unordered_set<std::string> set;
        std::vector< std::string > result;

        p.recentHosts.insert(p.recentHosts.begin(), fileName);

        for (const auto& str : p.recentHosts)
        {
            if (set.find(str) == set.end())
            {
                set.insert(str);
                result.push_back(str);
            }
        }

        while (result.size() > kRecentHostsMax)
        {
            result.pop_back();
        }

        p.recentHosts = result;
    }

    void SettingsObject::addPythonScript(const std::string& fileName)
    {
        TLRENDER_P();

        std::unordered_set<std::string> set;
        std::vector< std::string > result;

        p.pythonScripts.insert(p.pythonScripts.begin(), fileName);

        for (const auto& str : p.pythonScripts)
        {
            if (set.find(str) == set.end())
            {
                set.insert(str);
                result.push_back(str);
            }
        }

        while (result.size() > kRecentPythonScriptsMax)
        {
            result.pop_back();
        }

        p.pythonScripts = result;
    }
} // namespace mrv
